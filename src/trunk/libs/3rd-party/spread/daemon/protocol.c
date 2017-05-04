/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

#define ext_prot_body

#include <string.h>

#include "prot_body.h"
#include "spread_params.h"
#include "membership.h"
#include "flow_control.h"
#include "spu_events.h"
#include "session.h"
#include "network.h"
#include "net_types.h"
#include "status.h"
#include "log.h"
#include "spu_objects.h"
#include "spu_memory.h"
#include "spu_alarm.h"
#include "sess_types.h"

typedef struct queue_link
{
        sys_scatter       *packet;
        struct queue_link *next;

} queue_link;

typedef struct
{
        int         num_packets;
        queue_link *first;
        queue_link *last;

} packet_queue;

typedef struct
{
        int           num_messages;
        message_link *first;
        message_link *last;

} message_queue;

/* Prot variables */
static  proc            My;
static  int             My_index;

static  int32           Prev_proc_id; /* predecessor in ring (i.e. process that sends me the token) */
static  bool            Token_has_priority; /* true when token channels have higher priority than bcast channels */
static  int             Token_counter;

/* Used ONLY in Prot_handle_bcast, inited in Prot_init */
static  sys_scatter     New_pack;

/* Used ONLY in Prot_handle_token and grurot, inited in Prot_init */
static  sys_scatter     New_token;
static  token_header   *Token;
static  packet_queue    Send_pack_queue;

static  packet_header  *Hurry_head;
static  sys_scatter     Hurry_pack;
static  sp_time         Zero_timeout    = {  0, 0};

/* Used to indicate a need to reload configuration at end of current membership */
static  bool            Prot_Need_Conf_Reload  = FALSE;
static  down_queue      Protocol_down_queue[2]; /* only used in spread3 */

/* Used to enforce a minimum delivery ordering semantic */
static  int             Prot_delivery_threshold = BLOCK_REGULAR_DELIVERY;

static  void    Prot_handle_bcast( int fd, int dmy, void *dmy_ptr );
static  void    Prot_handle_token( int fd, int dmy, void *dmy_ptr );
static  int     Answer_retrans( int *ret_new_ptr, int32 *proc_id, int16 *seg_index );
static  int     Send_new_packets( int num_allowed );
static  int     Prot_queue_bcast( sys_scatter *send_pack_ptr, packet_queue *pack_queue );
static  int     Prot_flush_bcast( packet_queue *pack_queue );
static  int     Is_token_hold();
static  int     To_hold_token();
static  void    Handle_hurry( packet_header *pack_ptr );
static  void    Deliver_packet( int pack_entry, int to_copy );
static  void    Flip_token_body( char *buf, token_header *token_ptr );
static  void    Flip_frag( fragment_header *frag_ptr );

static  void    Deliver_all_reliable_packets_event( int dmy, void *dmy_ptr );
static  void    Deliver_reliable_packets( int32 start_seq, int num_packets );

static  void    Deliver_agreed_packets_event( int dmy, void *dmy_ptr );
static  void    Deliver_agreed_packets();

static  void    Prot_handle_conf_reload( sys_scatter *scat );

void Prot_init( void )
{
        int      i, num_bcast, num_token;
        channel *bcast_channels;
        channel *token_channels;

        Mem_init_object( PACK_HEAD_OBJ, "pack_head", sizeof( packet_header ), MAX_PACKETS_IN_STRUCT, 0 );
        Mem_init_object( PACKET_BODY, "packet_body", sizeof( packet_body ), MAX_PACKETS_IN_STRUCT, 0 );
        Mem_init_object( TOKEN_HEAD_OBJ, "token_head", sizeof( token_header ), 10, 0 );
        Mem_init_object( TOKEN_BODY_OBJ, "token_body", sizeof( token_body ), 10, 0 );
        Mem_init_object( SCATTER, "scatter", sizeof( scatter ), 200+MAX_PROCS_RING, 0 );
        Mem_init_object( SYS_SCATTER, "sys_scatter", sizeof( sys_scatter ), 200, 0 );
        Mem_init_object( QUEUE_LINK, "queue_link", sizeof( queue_link ), 200, 0 );

        My = Conf_my();
        My_index = Conf_proc_by_id( My.id, &My );
        GlobalStatus.my_id = My.id;
        GlobalStatus.packet_delivered = 0;
        Prev_proc_id = 0;

        for( i=0; i < MAX_PROCS_RING+1; i++ )
                Up_queue[i].exist = 0;

        for( i=0; i < MAX_PACKETS_IN_STRUCT; i++ )
                Packets[i].exist = 0;

        if ( Conf_debug_initial_sequence() ) {
                Highest_seq      = INITIAL_SEQUENCE_NEAR_WRAP;
                Highest_fifo_seq = INITIAL_SEQUENCE_NEAR_WRAP;
                My_aru           = INITIAL_SEQUENCE_NEAR_WRAP;
                Aru              = INITIAL_SEQUENCE_NEAR_WRAP;
                Last_discarded   = INITIAL_SEQUENCE_NEAR_WRAP;
                Last_delivered   = INITIAL_SEQUENCE_NEAR_WRAP;
        } else {
                Highest_seq      = 0;
                Highest_fifo_seq = 0;
                My_aru           = 0;
                Aru              = 0;
                Last_discarded   = 0;
                Last_delivered   = 0;
        }

        Send_pack_queue.num_packets = 0;
        Send_pack_queue.first = NULL;
        Send_pack_queue.last = NULL;

        New_pack.num_elements = 2;
        New_pack.elements[0].len = sizeof(packet_header);
        New_pack.elements[0].buf = (char *) new(PACK_HEAD_OBJ);
        New_pack.elements[1].len = sizeof(packet_body);
        New_pack.elements[1].buf = (char *) new(PACKET_BODY);

        New_token.num_elements  = 2;
        New_token.elements[0].len = sizeof(token_header);
        New_token.elements[0].buf = (char *) new(TOKEN_HEAD_OBJ);
        New_token.elements[1].len = sizeof(token_body);
        New_token.elements[1].buf = (char *) new(TOKEN_BODY_OBJ);

        Token = (token_header *)New_token.elements[0].buf;
        Last_token = new(TOKEN_HEAD_OBJ);
        Last_token->type = 0; 
        Last_token->seq = 0;
        Last_token->aru = 0;
        Last_token->flow_control = 0;
        Last_token->conf_hash = 0;

        Hurry_pack.num_elements = 1;
        Hurry_pack.elements[0].len = sizeof(packet_header);
        Hurry_pack.elements[0].buf = (char *) new(PACKET_BODY);
        Hurry_head = (packet_header *)Hurry_pack.elements[0].buf;
        Hurry_head->proc_id = My.id;
        Hurry_head->type = HURRY_TYPE;

        Net_init();

        bcast_channels = Net_bcast_channel();
        token_channels = Net_token_channel();
        Net_num_channels( &num_bcast, &num_token);
        for ( i = 0; i < num_bcast; i++) {
                E_attach_fd( *bcast_channels, READ_FD, Prot_handle_bcast, 0, NULL, HIGH_PRIORITY );
                bcast_channels++;
        }
        for ( i = 0; i < num_token; i++) {
                E_attach_fd( *token_channels, READ_FD, Prot_handle_token, 0, NULL, MEDIUM_PRIORITY );
                token_channels++;
        }
        Token_has_priority = FALSE;

        FC_init( );
        Memb_init();

        Net_set_membership( Reg_membership );
}

void Prot_init_down_queues( void )
{
        Protocol_down_queue[NORMAL_DOWNQUEUE].num_mess  = 0 ;
        Protocol_down_queue[NORMAL_DOWNQUEUE].cur_element = 0;
        Protocol_down_queue[GROUPS_DOWNQUEUE].num_mess  = 0 ;
        Protocol_down_queue[GROUPS_DOWNQUEUE].cur_element = 0;
}

void Prot_set_down_queue( int queue_type )
{
        switch ( queue_type )
        {
        case NORMAL_DOWNQUEUE:
                Down_queue_ptr = &Protocol_down_queue[NORMAL_DOWNQUEUE];
                break;

        case GROUPS_DOWNQUEUE:
                Down_queue_ptr = &Protocol_down_queue[GROUPS_DOWNQUEUE];
                break;

        default:
                Alarmp(SPLOG_FATAL, PROTOCOL, "Prot_set_down_queue: Illegal queue_type (%d)\n", queue_type);
        }
}

void Prot_Create_Local_Session( session *new_sess )
{
        return;
}

void Prot_Destroy_Local_Session( session *old_sess )
{
        return;
}

void Prot_kill_session( message_obj *msg )
{
        return;
}

down_link *Prot_Create_Down_Link( message_obj *msg, int type, int mbox, int cur_element )
{
        down_link      *down_ptr;
        message_header *head_ptr;

        if ( ( down_ptr = new( DOWN_LINK ) ) == NULL )
        {
                Alarm(EXIT, "Prot_Create_Down_Link: Failure to allocate a Down_link\n");
        }

        if ( -1 == (down_ptr->type = type) )
        {
                head_ptr = (message_header *)msg->elements[0].buf;
                dispose( down_ptr );
                Alarm( PROTOCOL, "Prot_Create_Down_Link: Illegal message type %d\n", head_ptr->type );
                return(NULL);
        }

        return(down_ptr);
}

static void Prot_handle_bcast( channel fd, int dummy, void *dummy_p )
{
        packet_header   *pack_ptr;
        packet_body     *pack_body_ptr;
        fragment_header *frag_ptr;
        int             pack_entry;
        proc            p;
        int             received_bytes;
        int             processed_bytes;
        int             padding_bytes;
        int             i, ret;
        /* int          r1,r2; */
        int             num_bcast, num_token;
        channel         *bcast_channels;
        channel         *token_channels;

        received_bytes = Net_recv( fd, &New_pack );

        /* problem in receiving */
        if ( received_bytes <= 0 ) return;

        pack_ptr = (packet_header *) New_pack.elements[0].buf;

        if ( Is_status( pack_ptr->type ) )
        {
                Stat_handle_message( &New_pack );
                return;
        }

        if ( Is_fc( pack_ptr->type ) )
        {
                FC_handle_message( &New_pack );
                return;
        }

        if ( Is_conf_reload( pack_ptr->type ) )
        {
                Prot_handle_conf_reload( &New_pack );
                return;
        }

        /* delete random  
           r1 = ((-My.id)%17)+3;
           r2 = get_rand() % (r1+3 );
           if ( r2 == 0 ) return; */

        if ( Is_membership( pack_ptr->type ) )
        {
                Memb_handle_message( &New_pack );
                return;
        }

        if ( Is_hurry( pack_ptr->type ) )
        {
                Handle_hurry( pack_ptr );
                return;
        }

        if ( !Is_regular( pack_ptr->type ) )
        {
                Alarm( PROTOCOL, "Prot_handle_bcast: Unknown packet type %d\n",
                       pack_ptr->type );
                return;
        }

        if ( !Memb_is_equal( Memb_id(), pack_ptr->memb_id ) )
        {
                /* Foreign message */
                Memb_handle_message( &New_pack );
                return;
        }

        if (Memb_token_alive() ) {
                E_queue( Memb_token_loss_event, 0, NULL, Token_timeout );
                if ( Conf_leader( Memb_active_ptr() ) == My.id ) 
                {
                        E_queue( Prot_token_hurry_event, 0, NULL, Hurry_timeout );
                }
        }

        /* do we have this packet */
        if ( pack_ptr->seq <= Last_discarded )
        {
                Alarm( PROTOCOL, "Prot_handle_bcast: delayed packet %d already delivered (Last_discarded %d)\n", pack_ptr->seq, Last_discarded );
                return;
        }

        pack_entry = pack_ptr->seq & PACKET_MASK;
        if ( Packets[pack_entry].exist ) 
        {
                Alarm( PROTOCOL, "Prot_handle_bcast: packet %d already exist\n", pack_ptr->seq );
                return;
        }

        Packets[pack_entry].proc_index = Conf_proc_by_id( pack_ptr->proc_id, &p );
        if ( Packets[pack_entry].proc_index < 0 )
        {
                Alarm( PROTOCOL, "Prot_handle_bcast: unknown proc %d\n", pack_ptr->proc_id );
                return;
        }

        pack_body_ptr = (packet_body *)New_pack.elements[1].buf;
        frag_ptr = &(pack_ptr->first_frag_header);
        if ( !Same_endian( pack_ptr->type ) ) 
        {
                Flip_frag( frag_ptr );
                processed_bytes = frag_ptr->fragment_len;
                while( processed_bytes < pack_ptr->data_len )
                {
                        padding_bytes = 0; 
                        switch (processed_bytes % 4)
                        {
                        case 1:
                                padding_bytes++;
                        case 2:
                                padding_bytes++;
                        case 3:
                                padding_bytes++;
                        case 0:
                                /* already aligned */
                                break;
                        }
                        processed_bytes += padding_bytes;
                        
                        /* Sanity check for packet validity */
                        if ( processed_bytes + (int) sizeof(fragment_header) > pack_ptr->data_len )
                        {
                                Alarm( PRINT, "Prot_handle_bcast: invalid packet with seq %d from %d, fragments exceed data_len %d %d\n", 
                                       pack_ptr->transmiter_id, pack_ptr->seq, processed_bytes, pack_ptr->data_len );
                                break;
                        }
                        frag_ptr = (fragment_header *) &pack_body_ptr[processed_bytes];
                        Flip_frag(frag_ptr);
                        processed_bytes += sizeof(fragment_header) + frag_ptr->fragment_len;
                }
                if ( processed_bytes != pack_ptr->data_len )
                {
                        Alarm( PRINT, "Prot_handle_bcast: invalid packet with seq %d from %d, processed bytes not equal data_len %d %d\n", 
                               pack_ptr->transmiter_id, pack_ptr->seq, processed_bytes, pack_ptr->data_len );
                        /*  
                         * This is a malformed packet, but we decide to keep it instead of throwing it away. 
                         * Note that a packet with the same endianess will not even get here 
                         */
                }
        }

        /* insert new packet */
        Packets[pack_entry].head  = pack_ptr;
        Packets[pack_entry].body  = (packet_body *)New_pack.elements[1].buf;
        Packets[pack_entry].exist = 1;

        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_bcast: inserting packet %d\n", pack_ptr->seq );

        /* If this packet was from my predecessor in the ring
         * (pack_ptr->transmiter_id == Prev_proc_id), and this packet is
         * marked with a token round greater than the last round in which I
         * received a token (pack_ptr->token_round > Received_token_rounds),
         * then I can conclude that I've processed all bcast packets from the
         * previous round, so it is safe to process the next token.
         * My predecessor has already sent all its packets from the previous
         * round, which means everyone else has too.  I should try to process
         * the token now -- it won't cause me to request unnecessary 
         * retransmissions and I want to keep the token moving quickly.
         */

        if ( FC_accelerated_ring() && !Token_has_priority && 
             pack_ptr->token_round > Received_token_rounds &&
             pack_ptr->transmiter_id == Prev_proc_id )
        {
                bcast_channels = Net_bcast_channel();
                token_channels = Net_token_channel();
                Net_num_channels( &num_bcast, &num_token);
                for ( i = 0; i < num_bcast; i++ ) {
                        ret = E_detach_fd_priority( *bcast_channels, READ_FD, HIGH_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_bcast: bcast_channel being detached was not found\n");
                        }
                        ret = E_attach_fd( *bcast_channels, READ_FD, Prot_handle_bcast, 0, NULL, MEDIUM_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_bcast: bcast_channel could not be attached\n");
                        }
                        bcast_channels++;
                }
                for ( i = 0; i < num_token; i++ ) {
                        ret = E_detach_fd_priority( *token_channels, READ_FD, MEDIUM_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_bcast: token_channel being detached was not found\n");
                        }
                        E_attach_fd( *token_channels, READ_FD, Prot_handle_token, 0, NULL, HIGH_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_bcast: token_channel could not be attached\n");
                        }
                        token_channels++;
                }
                Token_has_priority = TRUE;
        }

        /* update variables */
        if ( Highest_seq < pack_ptr->seq ) Highest_seq = pack_ptr->seq;
        if ( pack_ptr->seq == My_aru+1 )
        {
                for( i=pack_ptr->seq; i <= Highest_seq; i++ )
                {
                        if ( ! Packets[i & PACKET_MASK].exist ) break;
                        My_aru++;
                }
                Deliver_agreed_packets();
        }
        else
        {
                Deliver_reliable_packets( pack_ptr->seq, 1 );
        }

        GlobalStatus.packet_recv++;
        GlobalStatus.my_aru = My_aru;
        GlobalStatus.highest_seq = Highest_seq;

        /* prepare New_pack for next packet */
        New_pack.elements[0].buf = (char *) new( PACK_HEAD_OBJ );
        New_pack.elements[1].buf = (char *) new( PACKET_BODY );
}

void Prot_handle_token( channel fd, int dummy, void *dummy_p )
{
        int             new_ptr;
        int             num_retrans, num_allowed, num_sent;
        int             flow_control;
        char            *new_rtr;
        ring_rtr        *ring_rtr_ptr;
        int32           rtr_proc_id;
        int16           rtr_seg_index;
        int32           val;
        int             retrans_allowed; /* how many of my retrans are allowed on token */
        int             max_rtr_seq;
        int             i, ret;
        int             num_bcast, num_token;
        channel         *bcast_channels;
        channel         *token_channels;
        membership_id   memb_id;

        /*      int             r1,r2;*/

        ret = Net_recv_token( fd, &New_token );

        if ( ret <= 0 ) return;

        /* delete random
           r1 = ((-My.id)%17)+3;
           r2 = get_rand() % (r1+3 );
           if ( r2 == 0 ) return; */

        if ( Is_form( Token->type ) )
        {
                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: it is a Form Token.\n" );
                Memb_handle_token( &New_token );
                goto END;
        }

        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: type = 0x%08X; transmitter = 0x%08X; seq = %d; proc_id = 0x%08X; aru = %d; aru_last_id = 0x%08X;\n", 
                Token->type, Token->transmiter_id, Token->seq, Token->proc_id, Token->aru, Token->aru_last_id );

        /* The Veto property for tokens - swallow this token */
        if ( ! Memb_token_alive() ) 
        {
                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: Veto Property. Memb not alive.\n" );
                goto END;
        }

        if ( !Memb_is_equal( Token->memb_id, ( memb_id = Memb_active_id() ) ) ) 
        {
                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: received token for wrong memb id (%d, %d)! Should be (%d, %d)\n",
                        Token->memb_id.proc_id, Token->memb_id.time, memb_id.proc_id, memb_id.time );
                goto END;
        }

        if ( Token->transmiter_id != (rtr_proc_id = Conf_previous( Memb_active_ptr() ) ) ) 
        {
                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: Received token from unexpected transmitter! Should be 0x%08X\n", rtr_proc_id );
                goto END;
        }

        if ( ret != sizeof(token_header) + Token->rtr_len )
        {
                Alarmp( SPLOG_WARNING, PROTOCOL, "Prot_handle_token: recv token len is %d, should be %d\n", ret, sizeof(token_header) + Token->rtr_len );
                goto END;
        }

        if ( !Same_endian( Token->type ) )
        {
                Flip_token_body( New_token.elements[1].buf, Token );
        }

        /* Deal with wrapping seq values (2^32) by triggering a membership by dropping the token */

        if ( Memb_state() != EVS && Token->seq > MAX_WRAP_SEQUENCE_VALUE )
        {
                Alarmp( SPLOG_WARNING, PROTOCOL, "Prot_handle_token: seq (%d) rollover; swallowing token to trigger membership\n", Token->seq );
                goto END;
        }

        if ( Conf_leader( Memb_active_ptr() ) == My.id )
        {
                if ( Get_arq( Token->type ) != Get_arq( Last_token->type ) )
                {
                        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: leader swallowing repeated token; type = %d; retrans = %d; arq = %d\n", 
                                Get_arq( Token->type ), Get_retrans( Token->type ), Get_arq( Last_token->type ) );
                        goto END;
                } 
                else if ( Memb_Just_Installed ) 
                {
                        Memb_Just_Installed  = FALSE;
                        /* NOTE: we do this now rather than just when we installed to allow proper token retransmissions */
                        Last_token->aru = 0;  /* wipe out EVS aru on last token due to transitioning from EVS -> OP */
                        Alarmp(SPLOG_INFO, PROTOCOL, "Prot_handle_token: leader just installed; setting Last_token->aru = 0!\n");
                }
        } 
        else 
        {
                if ( Get_arq( Token->type ) == Get_arq( Last_token->type ) )
                {
                        if ( Get_retrans( Token->type ) > Get_retrans( Last_token->type ) )
                        {
                                /* asked to send token again (almost lost) */
                                val = Get_retrans( Token->type );
                                Last_token->type = Set_retrans(Last_token->type,val);
                                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: not leader, asked to retrans %d %d\n",
                                        Get_arq( Token->type ), val );
                                Prot_token_hurry();
                        } 
                        else 
                        {
                                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: not leader, swallow same token %d %d\n",
                                        Get_arq( Token->type ), Get_retrans( Token->type ) );
                        }

                        goto END;
                } 
                else if ( Get_arq( Token->type ) != ( ( Get_arq( Last_token->type ) + 1 ) % 0x10 ) ) 
                {
                        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: not leader, swallowing very outdated token: ARQ(%d) RETRANS(%d) vs. Last ARQ(%d)\n",
                                Get_arq( Token->type ), Get_retrans( Token->type ), Get_arq( Last_token->type ) );

                        goto END;
                } 
                else 
                {
                        if ( Memb_Just_Installed ) 
                        {
                                Memb_Just_Installed  = FALSE;
                                /* NOTE: we do this now rather than just when we installed to allow proper token retransmissions */
                                Last_token->aru = 0;  /* wipe out EVS aru on last token due to transitioning from EVS -> OP */
                                Alarmp(SPLOG_INFO, PROTOCOL, "Prot_handle_token: not-leader just installed; setting Last_token->aru = 0!\n");
                        }

                        if ( Get_retrans( Token->type ) > 0 ) 
                        {
                                GlobalStatus.token_hurry++;
                        }
                }
        }

        if ( Highest_seq < Token->seq ) 
        {
                Highest_seq = Token->seq;
        }
                
        /* I don't want to process my next token until I have processed
         *  all bcast packets sent in the previous round, so I should give 
         *  bcast channels a higher priority until then */
        Received_token_rounds++;
        if ( Token_has_priority )
        {
                bcast_channels = Net_bcast_channel();
                token_channels = Net_token_channel();
                Net_num_channels( &num_bcast, &num_token );
                for ( i = 0; i < num_bcast; i++ ) {
                        ret = E_detach_fd_priority( *bcast_channels, READ_FD, MEDIUM_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_token: bcast_channel being detached was not found\n");
                        }
                        E_attach_fd( *bcast_channels, READ_FD, Prot_handle_bcast, 0, NULL, HIGH_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_token: bcast_channel could not be attached\n");
                        }
                        bcast_channels++;
                }
                for ( i = 0; i < num_token; i++ ) {
                        ret = E_detach_fd_priority( *token_channels, READ_FD, HIGH_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_token: token_channel being detached was not found\n");
                        }
                        E_attach_fd( *token_channels, READ_FD, Prot_handle_token, 0, NULL, MEDIUM_PRIORITY );
                        if ( ret < 0 ) {
                                Alarm( EXIT, "Prot_handle_token: token_channel could not be attached\n");
                        }
                        token_channels++;
                }
                Token_has_priority = FALSE;
        }
                
        /* Handle retransmissions */
        num_retrans = Answer_retrans( &new_ptr, &rtr_proc_id, &rtr_seg_index );
        GlobalStatus.retrans += num_retrans;

        new_rtr = New_token.elements[1].buf;

        /* Handle new packets */
        flow_control = (int) Token->flow_control;
        num_allowed = FC_allowed( flow_control, num_retrans );
        num_sent    = Send_new_packets( num_allowed );
        GlobalStatus.packet_sent += num_sent;

        /* Flow control calculations */
        Token->flow_control = Token->flow_control
                - Last_num_retrans - Last_num_sent 
                + num_retrans      + num_sent;
        Last_num_retrans = num_retrans;
        Last_num_sent    = num_sent;

        /* Prepare my retransmission requests */

        for( i = My_aru+1; i <= Highest_seq; i++ )
        {
                if ( ! Packets[i & PACKET_MASK].exist ) break;
                My_aru++;
        }
        GlobalStatus.my_aru = My_aru;

        /* Determine sequence up through which retransmissions should be
         * requested: If Accelerated_ring is true, packets with sequence
         * numbers between the sequence on the last token this process sent
         * and the sequence on the token it just received may still be in
         * transit or not yet sent, so they should not be requested at this point*/
        if ( FC_accelerated_ring() && Memb_state() != EVS ){
                max_rtr_seq = Last_token->seq;
        }else{
                max_rtr_seq = Highest_seq;
        }

        if ( My_aru < max_rtr_seq )
        {
                /* Compute how many of my retransmission requests are possible to fit */
                retrans_allowed = ( sizeof( token_body ) - new_ptr - sizeof( ring_rtr ) ) / sizeof( int32 );
                if ( retrans_allowed > 1 )
                {
                        ring_rtr_ptr = (ring_rtr *)&new_rtr[new_ptr];
                        ring_rtr_ptr->memb_id   = Memb_id();
                        ring_rtr_ptr->proc_id   = rtr_proc_id;
                        ring_rtr_ptr->seg_index = rtr_seg_index;
                        ring_rtr_ptr->num_seq   = 0;
                        new_ptr += sizeof(ring_rtr);
                        for( i=My_aru+1; i <= max_rtr_seq && retrans_allowed > 0; i++ )
                        {
                                if ( ! Packets[i & PACKET_MASK].exist ) 
                                {
                                        memcpy( &new_rtr[new_ptr], &i, sizeof(int32) ); 
                                        new_ptr += sizeof(int32);
                                        ring_rtr_ptr->num_seq++;
                                        --retrans_allowed;
                                }
                        }
                }
        }

        if ( Memb_state() == EVS )
        {
                if ( My_aru == Highest_seq )
                {
                        My_aru = Last_seq;
                        Memb_commit();
                }
        }

        Token->rtr_len = new_ptr;
        New_token.elements[1].len = new_ptr;

        /* Calculating Token->aru and Token->aru_last_id */

        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: calculating Token->aru: Token->aru = %d, My_aru = %d, Token->aru_last_id = 0x%08X, My.id = 0x%08X, Token->seq = %d, Memb_state() = %d\n",
                Token->aru, My_aru, Token->aru_last_id, My.id, Token->seq, Memb_state() );

        if ( Token->aru > My_aru         ||                           /* this daemon is missing packets: lower aru to My_aru */
             Token->aru_last_id == My.id ||                           /* this daemon last updated aru: try raising to My_aru */
             ( Token->aru == Token->seq && Memb_state() != EVS ) ) {  /* everyone has everything so far: try raising to My_aru; NOTE: Token->seq is meaningless in EVS */

                Token->aru         = My_aru;
                Token->aru_last_id = My.id;
                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: setting Token->aru = %d, Token->aru_last_id = 0x%08X\n", Token->aru, Token->aru_last_id );
        }
        
        Token->proc_id = My.id;
        if ( Memb_state() != EVS ) Token->seq = Highest_seq;

        if ( Conf_leader( Memb_active_ptr() ) == My.id )
        {
                val = Get_arq( Token->type );
                val = (val + 1)% 0x10;
                Token->type = Set_arq(     Token->type, val );
                Token->type = Set_retrans( Token->type, 0   );
        }

        /* Send token */
        if ( ! ( Conf_leader( Memb_active_ptr() ) == My.id &&
                To_hold_token() ) )
        {
                /* sending token */
                Net_send_token( &New_token );
/* ### Bug fix for SGIs */
#ifdef  ARCH_SGI_IRIX
                Net_send_token( &New_token );
#endif  /* ARCH_SGI_IRIX */

                if ( Get_retrans( Token->type ) > 1 )
                {
                        /* problems */ 
                        Net_send_token( &New_token );
                        Net_send_token( &New_token );
                }
        }

        Token_rounds++;

        if ( Conf_leader( Memb_active_ptr() ) == My.id ) 
                E_queue( Prot_token_hurry_event, 0, NULL, Hurry_timeout );

        E_queue( Memb_token_loss_event, 0, NULL, Token_timeout );

        /* Send any packets remaining in queue */
        Prot_flush_bcast(&Send_pack_queue);

        /* calculating Aru */
        if ( Token->aru > Last_token->aru ) {

                if ( Last_token->aru >= Aru ) {
                        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: updating Aru from Last_token: Aru %d -> %d; (Token->aru = %d)\n", Aru, Last_token->aru, Token->aru );

                } else {
                        Alarmp( SPLOG_FATAL, PROTOCOL, "Prot_handle_token: illegally lowering Aru from Last_token: Aru %d -> %d; (Token->aru = %d)\n", Aru, Last_token->aru, Token->aru );
                }

                Aru = Last_token->aru;

        } else {

                if ( Token->aru >= Aru ) {
                        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: updating Aru from Token: Aru %d -> %d; (Last_token->aru = %d)\n", Aru, Token->aru, Last_token->aru);

                } else {
                        Alarmp( SPLOG_FATAL, PROTOCOL, "Prot_handle_token: illegally lowering Aru from Token: Aru %d -> %d; (Last_token->aru = %d)\n", Aru, Token->aru, Last_token->aru);
                }

                Aru = Token->aru;
        }

        if ( Highest_seq == Aru ) Token_counter++;
        else Token_counter = 0;

        dispose( Last_token );
        Last_token = Token;
        New_token.elements[0].buf = (char *) new(TOKEN_HEAD_OBJ);
        New_token.elements[1].len = sizeof(token_body);
        Token = (token_header *)New_token.elements[0].buf;

        /* Deliver & discard packets */

        Discard_packets();
        Deliver_agreed_packets();
        Deliver_reliable_packets( Highest_seq-num_sent+1, num_sent );

        if ( Memb_state() == EVS && Token_rounds > MAX_EVS_ROUNDS ) 
        {
                Alarmp( SPLOG_ERROR, PRINT, "Prot_handle_token: BUG WORKAROUND: Too many rounds in EVS state; swallowing token; state:\n" );
                Alarmp( SPLOG_ERROR, PRINT, "\tAru:              %d\n",   Aru );
                Alarmp( SPLOG_ERROR, PRINT, "\tMy_aru:           %d\n",   My_aru );
                Alarmp( SPLOG_ERROR, PRINT, "\tHighest_seq:      %d\n",   Highest_seq );
                Alarmp( SPLOG_ERROR, PRINT, "\tHighest_fifo_seq: %d\n",   Highest_fifo_seq );
                Alarmp( SPLOG_ERROR, PRINT, "\tLast_discarded:   %d\n",   Last_discarded );
                Alarmp( SPLOG_ERROR, PRINT, "\tLast_delivered:   %d\n",   Last_delivered );
                Alarmp( SPLOG_ERROR, PRINT, "\tLast_seq:         %d\n",   Last_seq );
                Alarmp( SPLOG_ERROR, PRINT, "\tToken_rounds:     %d\n",   Token_rounds );
                Alarmp( SPLOG_ERROR, PRINT, "Last Token:\n" );
                Alarmp( SPLOG_ERROR, PRINT, "\ttype:             0x%x\n", Last_token->type );
                Alarmp( SPLOG_ERROR, PRINT, "\ttransmiter_id:    %d\n",   Last_token->transmiter_id );
                Alarmp( SPLOG_ERROR, PRINT, "\tseq:              %d\n",   Last_token->seq );
                Alarmp( SPLOG_ERROR, PRINT, "\tproc_id:          %d\n",   Last_token->proc_id );
                Alarmp( SPLOG_ERROR, PRINT, "\taru:              %d\n",   Last_token->aru );
                Alarmp( SPLOG_ERROR, PRINT, "\taru_last_id:      %d\n",   Last_token->aru_last_id );
                Alarmp( SPLOG_ERROR, PRINT, "\tflow_control:     %d\n",   Last_token->flow_control );
                Alarmp( SPLOG_ERROR, PRINT, "\trtr_len:          %d\n",   Last_token->rtr_len );
                Alarmp( SPLOG_ERROR, PRINT, "\tconf_hash:        %d\n",   Last_token->conf_hash );

                Memb_token_loss();
        }

        GlobalStatus.highest_seq = Highest_seq;
        GlobalStatus.aru = Aru;
        GlobalStatus.token_rounds = Token_rounds;

END:
        Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: LEAVING!\n" );
}

/* Provide boolean result of whether the membership system needs to initiate a configuration reload
 * because it was delayed by an ongoing membership change
 */
bool    Prot_need_conf_reload( void )
{
        return( Prot_Need_Conf_Reload );
}

void    Prot_clear_need_conf_reload( void )
{
        Prot_Need_Conf_Reload = FALSE;
}

/* If we are in OP state for the daemon membership, initiate conf reload to new configuration file and membership,
 * If we are not, then a membership change is ongoing and we need to let that complete before starting a 
 * new one to load in the new configuration.
 */
static  void    Prot_handle_conf_reload(sys_scatter *scat)
{
        if ( Memb_state() == OP ) {
                Prot_initiate_conf_reload(0, NULL);
        } else {
                Prot_Need_Conf_Reload = TRUE;
        }
}

/* Basic algorithm:
 * 1) have configuration code load new conf file and check for modifications to conf.
 * 2) If only add/sub of daemons, then initiate membership change with token_loss and return;
 * 3) else, then set Conf_reload_state, create singleton partition, and schedule token_loss.
 * 4) When membership completes in Discard_packets() cleanup partition and probe for new members.
 */
void    Prot_initiate_conf_reload( int code, void *data )
{
        bool    need_memb_partition;
        int16   singleton_partition[MAX_PROCS_RING];
        int     i;

        if (Memb_state() != OP ) {
                /* This is a race condition, that the Prot_initiate_conf_reload was scheduled when OP state, 
                 * but another membership occured before it was executed.
                 * The membership system will requeue this function when it reaches OP state again.
                 */
                return;
        }
        /* Disable queueing of this function when OP state reached in membership */
        Prot_clear_need_conf_reload();

        need_memb_partition = Conf_reload_initiate();

        /* Signal all subsystems to update Conf and My strucures */
        Net_signal_conf_reload();
        Memb_signal_conf_reload();
        /* Note: the Sess_signal also calls G_signal_conf_reload() */
        Sess_signal_conf_reload();
        /* Signal flow control to reload window parameters */
        FC_signal_conf_reload();

        /* update protocol variables with new conf */
        My = Conf_my();
        My_index = Conf_proc_by_id( My.id, &My );

        if (need_memb_partition) {
                /* make partition */
                for ( i = 0 ; i < Conf_num_procs( Conf_ref() ); i++ ) 
                {
                        singleton_partition[i] = i;
                }
                Net_set_partition(singleton_partition);

                Conf_reload_singleton_state_begin();
        }
        E_queue( Memb_token_loss_event, 0, NULL, Zero_timeout );
}


void    Prot_new_message( down_link *down_ptr, int not_used_in_spread3_p )
{
        int32   leader_id;

        if ( Down_queue_ptr->num_mess > 0 )
        {
                down_ptr->next = NULL;
                Down_queue_ptr->last->next = down_ptr;
                Down_queue_ptr->last = down_ptr;
        }else if ( Down_queue_ptr->num_mess == 0 ){
                Down_queue_ptr->first = down_ptr;
                Down_queue_ptr->last  = down_ptr;
        }else{
                Alarm( EXIT,"fast_spread_new_message: Down_queue_ptr->num_mess is %d\n",
                       Down_queue_ptr->num_mess );
        }
        Down_queue_ptr->num_mess++;
        if ( Down_queue_ptr->num_mess >= WATER_MARK ) 
                Sess_block_users_level();

        if ( Down_queue_ptr->num_mess == 1  && Is_token_hold() )
        {
                leader_id = Conf_leader( Memb_active_ptr() );
                if ( leader_id == My.id )
                {
                        Handle_hurry( Hurry_head );
                }else{
                        Net_ucast( leader_id, &Hurry_pack );
                }
        }
}

static  int     Answer_retrans( int *ret_new_ptr, 
                                int32 *proc_id, int16 *seg_index )
{
        int             num_retrans;
        sys_scatter     *send_pack_ptr;
        char            *rtr;
        int             old_ptr,new_ptr;
        ring_rtr        *ring_rtr_ptr;
        int             pack_entry;
        int             bytes_to_copy;
        packet_header   *pack_ptr;
        int             i, ret;
        int32           *req_seq;

        num_retrans     = 0;
        new_ptr         = 0;
        *proc_id     = My.id;
        *seg_index   = My.seg_index;
        if ( Token->rtr_len > 0 )
        {
                rtr = New_token.elements[1].buf;
                old_ptr = 0;
                while( old_ptr < Token->rtr_len )
                {
                        ring_rtr_ptr = (ring_rtr *)&rtr[old_ptr];
                        if ( Memb_is_equal(ring_rtr_ptr->memb_id,Memb_id() ) )
                        {
                                /* retransmit requests from my ring */
                                old_ptr += sizeof(ring_rtr);
                                for( i=0; i < ring_rtr_ptr->num_seq; i++ )
                                {
                                        req_seq = (int32 *)&rtr[old_ptr];
                                        old_ptr += sizeof(int32);
                                        pack_entry = *req_seq & PACKET_MASK;
                                        if ( *req_seq < Aru ) 
                                                Alarm( EXIT, "Answer_retrans: retrans of %d requested while Aru is %d\n", *req_seq, Aru );

                                        if ( Packets[pack_entry].exist )
                                        {
                                                send_pack_ptr = new(SYS_SCATTER);
                                                send_pack_ptr->num_elements = 2;
                                                send_pack_ptr->elements[0].len = sizeof(packet_header);
                                                pack_ptr = Packets[pack_entry].head;
                                                send_pack_ptr->elements[0].buf = (char *)Packets[pack_entry].head;
                                                send_pack_ptr->elements[1].buf = (char *)Packets[pack_entry].body;
                                                send_pack_ptr->elements[1].len = pack_ptr->data_len; 

                                                if ( ring_rtr_ptr->proc_id != -1 )
                                                {
                                                        ret = Net_ucast ( ring_rtr_ptr->proc_id, send_pack_ptr );
                                                        dispose(send_pack_ptr);
                                                        GlobalStatus.u_retrans++;

                                                        Alarmp( SPLOG_INFO, PROTOCOL, "Answer_retrans: retransmit %d to proc 0x%08X\n", *req_seq, ring_rtr_ptr->proc_id );

                                                }else if ( ring_rtr_ptr->seg_index != -1 ) {
                                                        ret = Net_scast ( ring_rtr_ptr->seg_index, send_pack_ptr );
                                                        dispose(send_pack_ptr);
                                                        GlobalStatus.s_retrans++;

                                                        Alarmp( SPLOG_INFO, PROTOCOL, "Answer_retrans: retransmit %d to seg 0x%08X\n", *req_seq, ring_rtr_ptr->seg_index );

                                                }else{
                                                        /* NOTE: we bcast retransmissions immediately rather than queue them to give such retranmissions a better chance of 
                                                           being received before the token at the next member, so that they don't re-request retransmissions we are sending now */
                                                        ret = Net_bcast( send_pack_ptr );
                                                        dispose( send_pack_ptr );
                                                        GlobalStatus.b_retrans++;

                                                        Alarmp( SPLOG_INFO, PROTOCOL, "Answer_retrans: retransmit %d to all\n", *req_seq);
                                                }
                                                num_retrans++;
                                        }else{
                                                *proc_id = -1;
                                                if ( ring_rtr_ptr->seg_index != My.seg_index )
                                                        *seg_index = -1;
                                        }
                                }
                        }else{
                                /* copy requests of other rings */
                                bytes_to_copy = sizeof(ring_rtr) + 
                                        ring_rtr_ptr->num_seq * sizeof(int32);

                                if ( new_ptr != old_ptr )
                                        memmove( &rtr[new_ptr], &rtr[old_ptr], bytes_to_copy);

                                old_ptr += bytes_to_copy;
                                new_ptr += bytes_to_copy;

                                Alarmp( SPLOG_INFO, PROTOCOL, "Prot_handle_token: Coping foreign rtr\n");
                        }
                }
        }
        *ret_new_ptr = new_ptr;
        return (num_retrans);
}

static  int     Send_new_packets( int num_allowed )
{
        packet_header   *pack_ptr;
        scatter         *scat_ptr;
        sys_scatter     *send_pack_ptr;
        fragment_header *frag_ptr;
        char            *body_ptr;
        int             pack_entry;
        int             num_sent;
        int             padding_bytes;
        int             available_bytes;
        int             ret;

        num_sent = 0;
        while( num_sent < num_allowed )
        {
                /* check if down queue is empty */
                if ( Down_queue_ptr->num_mess == 0 ) break;

                /* initialize packet_header */
                pack_ptr =  new(PACK_HEAD_OBJ);

                scat_ptr = Down_queue_ptr->first->mess;

                pack_ptr->type = Down_queue_ptr->first->type;
                pack_ptr->proc_id = My.id;
                pack_ptr->memb_id = Memb_id();
                pack_ptr->seq = Highest_seq+1;
                Highest_seq++;
                /*pack_ptr->fifo_seq = Highest_fifo_seq+1; ### Commented out because fifo_seq was replaced with token_round*/
                Highest_fifo_seq++;
                pack_ptr->data_len = scat_ptr->elements[
                        Down_queue_ptr->cur_element].len;
                pack_ptr->first_frag_header.fragment_len = scat_ptr->elements[
                        Down_queue_ptr->cur_element].len;

                send_pack_ptr = new(SYS_SCATTER); /* send_pack_ptr is freed when the packet is sent (after Net_bcast is called)  */
                send_pack_ptr->num_elements = 2;
                send_pack_ptr->elements[0].len = sizeof(packet_header);
                send_pack_ptr->elements[1].buf = scat_ptr->elements[
                        Down_queue_ptr->cur_element].buf;
                body_ptr = send_pack_ptr->elements[1].buf;

                /* Set frag_ptr to point to the fragment header in the packet header */
                frag_ptr = &(pack_ptr->first_frag_header); 

                /* Loop for filling the packet:
                 *
                 * Advance the down queue and set the fragment index for the fragment
                 * just added to the packet.
                 *
                 * Break if another fragment cannot be added to this packet.
                 *    This happens in 3 cases:
                 *    1. The down queue is empty.
                 *    2. The next fragment in the down queue is too big to fit in the packet.
                 *    3. The next message does not have a type compatible with the current
                 *       packet type.
                 *
                 * If the next fragment can be added (i.e. you didn't break for one of the 3
                 * reasons above), set the frag_ptr to point after the end of the last fragment
                 * in the packet body and copy the fragment from the down queue into the packet
                 * body.
                 */
                for(;;)
                {
                        /* Advance the down queue and set the fragment index for the fragment
                         * just added to the packet. */
                        Down_queue_ptr->cur_element++;
                        if ( Down_queue_ptr->cur_element < (int) scat_ptr->num_elements )
                        {
                                /* not last packet in message */
                                frag_ptr->fragment_index = Down_queue_ptr->cur_element;
                        }else if ( Down_queue_ptr->cur_element == scat_ptr->num_elements ){
                                down_link       *tmp_down;

                                /* last packet in message */
                                frag_ptr->fragment_index = -(int16) scat_ptr->num_elements;

                                tmp_down = Down_queue_ptr->first;
                                Down_queue_ptr->first = Down_queue_ptr->first->next;
                                Down_queue_ptr->cur_element = 0;
                                Down_queue_ptr->num_mess--;
                                dispose( tmp_down->mess );
                                dispose( tmp_down );
                                if ( Down_queue_ptr->num_mess < WATER_MARK ) 
                                        Sess_unblock_users_level();
                        }else{
                                Alarm( EXIT, 
                                       "Send_new_packets: error in packet index: %d %d\n",
                                       Down_queue_ptr->cur_element,scat_ptr->num_elements );
                        }
                        /* Break if another fragment cannot be added to this packet.
                         *    This happens in 3 cases:
                         *    1. The down queue is empty.
                         *    2. The next fragment in the down queue is too big to fit in the packet.
                         *    3. The next message does not have a type compatible with the current
                         *       packet type.
                         */
                        if ( Down_queue_ptr->num_mess == 0 ) break;

                        padding_bytes = 0; 
                        switch (pack_ptr->data_len % 4)
                        {
                        case 1:
                                padding_bytes++;
                        case 2:
                                padding_bytes++;
                        case 3:
                                padding_bytes++;
                        case 0:
                                /* already aligned */
                                break;
                        }
                        available_bytes = sizeof(packet_body) - pack_ptr->data_len - padding_bytes - sizeof(fragment_header);
                        scat_ptr = Down_queue_ptr->first->mess;
                        /* 
                         * The comparison doesn't work without the (int) cast because len is size_t and available_bytes is int
                         * It is probable that size_t is defined as unsigned. Note that available_bytes can be negative.
                         * Without the casting of size_t to int, available_bytes will be automatically casted to size_t
                         * making it a very large number and causing a bug. Therefore, we have to cast the left side
                         * of the equation to int
                         */ 
                        if ( ( (int) scat_ptr->elements[Down_queue_ptr->cur_element].len ) > available_bytes ) break;

                        if ( pack_ptr->type != Down_queue_ptr->first->type )
                        {
                                if ( !(Is_fifo(pack_ptr->type) || Is_agreed(pack_ptr->type)) ||
                                    !(Is_fifo(Down_queue_ptr->first->type) || Is_agreed(Down_queue_ptr->first->type)) )
                                        break;
                        }
                        /* 
                         * If the next fragment can be added (i.e. we didn't break for one of the 3
                         * reasons above), set the frag_ptr to point after the end of the last fragment
                         * in the packet body and copy the fragment from the down queue into the packet
                         * body.
                         */
                        frag_ptr = (fragment_header *) &body_ptr[ pack_ptr->data_len + padding_bytes ];
                        frag_ptr->fragment_len = scat_ptr->elements[Down_queue_ptr->cur_element].len;
                        pack_ptr->data_len += padding_bytes + sizeof(fragment_header);
                        memcpy(&body_ptr[pack_ptr->data_len], scat_ptr->elements[Down_queue_ptr->cur_element].buf, frag_ptr->fragment_len);
                        pack_ptr->data_len += frag_ptr->fragment_len;
                        /* 
                         * Dispose buf for copied fragment. Note that this is only possible because
                         * copied fragments are guaranteed to be whole messages (so we can't have
                         * a scenario in which it is part of a partially sent message). The first fragment
                         * will be disposed when the packet is discarded/the message is delivered 
                         * */
                        dispose( (packet_body *) (scat_ptr->elements[Down_queue_ptr->cur_element].buf) );
                }

                send_pack_ptr->elements[0].buf = (char *) pack_ptr;
                send_pack_ptr->elements[1].len = pack_ptr->data_len;

                ret = Prot_queue_bcast( send_pack_ptr, &Send_pack_queue );
                num_sent++;
 
                pack_entry = pack_ptr->seq & PACKET_MASK;
                if ( Packets[pack_entry].exist ) 
                        Alarm( EXIT, 
                               "Send_new_packets: created packet %d already exist %d\n",
                               pack_ptr->seq, Packets[pack_entry].exist );

                /* insert new created packet */
                Packets[pack_entry].head       = pack_ptr;
                Packets[pack_entry].body       = (packet_body *) body_ptr;
                Packets[pack_entry].exist      = 1;
                Packets[pack_entry].proc_index = My_index;
                Alarm( PROTOCOL, 
                       "Send_new_packets: packet %d sent and inserted \n",
                       pack_ptr->seq );
        }
        return ( num_sent );
}
 
static  int     Prot_queue_bcast( sys_scatter *send_pack_ptr, packet_queue *pack_queue )
{
        sys_scatter *scat_ptr;
        queue_link *link_ptr;
        packet_header *pack_ptr; /* Added to set token_rounds before sending */
        int num_pack_to_send;
        int ret;
        int i; 

        /* To queue or send a new packet:
         * 1. Determine what you need to send from queue.
         *    If Accelerated_ring is not used, send everything. (unless the Accelerated_ring
         *    option is changed dynamically, there should not be anything in the queue)
         *    Otherwise, send the difference between the number of packets in the queue
         *      and the max packets in the queue, plus one to account for the new packet.
         *      (unless max_packets is changed dynamically, this should be 1, if the queue
         *      is full, or 0, if the queue is not full)
         * 2. Send the number of packets that you calculated.
         * 3. If Accelerated_ring is not used, or the max packets in the queue is 0, send
         *    the new packet.
         *    Otherwise, add the new packet to the queue.
         */
        if ( !FC_accelerated_ring() || Memb_state() == EVS)
        {
                num_pack_to_send = pack_queue->num_packets;
        }else{
                num_pack_to_send = pack_queue->num_packets - FC_accelerated_window() + 1;
                if ( num_pack_to_send < 0 ) num_pack_to_send = 0;
                /* The case below can happen if accelerated window changes dynamically */
                if ( num_pack_to_send > pack_queue->num_packets ) num_pack_to_send = pack_queue->num_packets;
        }

        for( i = 0; i < num_pack_to_send; i++ )
        {
                scat_ptr = pack_queue->first->packet;
                link_ptr = pack_queue->first;
                pack_queue->first = pack_queue->first->next;
                pack_queue->num_packets--;
                dispose(link_ptr);
                pack_ptr = (packet_header *) scat_ptr->elements[0].buf;
                pack_ptr->token_round = Token_rounds;
                Net_bcast(scat_ptr);
                dispose(scat_ptr);
        }
        ret = num_pack_to_send;

        if ( !FC_accelerated_ring() || FC_accelerated_window() == 0 || Memb_state() == EVS )
        {
                pack_ptr = (packet_header *) send_pack_ptr->elements[0].buf;
                pack_ptr->token_round = Token_rounds;
                Net_bcast(send_pack_ptr);
                dispose(send_pack_ptr);
                ret++;
        }else{
                link_ptr = new(QUEUE_LINK);
                link_ptr->packet = send_pack_ptr;
                link_ptr->next = NULL;

                if ( pack_queue->num_packets == 0 )
                {
                        pack_queue->first = link_ptr;
                }else{
                        pack_queue->last->next = link_ptr;
                }
                pack_queue->last = link_ptr;
                pack_queue->num_packets++;
        }

        return ( ret );
}

static  int     Prot_flush_bcast( packet_queue *pack_queue )
{
        sys_scatter *scat_ptr;
        queue_link *link_ptr;
        packet_header *pack_ptr;
        int ret;

        ret = pack_queue->num_packets;
        while( pack_queue->num_packets > 0 )
        {
                scat_ptr = pack_queue->first->packet;
                link_ptr = pack_queue->first;
                pack_queue->first = pack_queue->first->next;
                pack_queue->num_packets--;
                dispose(link_ptr);
                pack_ptr = (packet_header *) scat_ptr->elements[0].buf;
                pack_ptr->token_round = Token_rounds;
                Net_bcast(scat_ptr);
                dispose(scat_ptr);
        }
        return( ret );
}
 
static  void    Deliver_packet( int pack_entry, int to_copy )
{       
        int             proc_index;
        up_queue        *up_ptr;
        packet_header   *pack_ptr;
        message_link    *mess_link;
        fragment_header *frag_ptr;
        char            *pack_body_ptr;
        message_queue   mess_queue;
        int             processed_bytes;
        int             padding_bytes;
        int             index;

        pack_ptr = Packets[pack_entry].head;

        /* 
         * For multi-fragment packets, the following observations can be made:
         *     1. The first fragment is either the last fragment in a multi-packet message
         *        or a whole message.
         *     2. Each fragment after the first must be a whole message.
         *
         * For any fragment beyond the first, we create a message (with a new packet body)
         * and put it in a queue. We then process the first fragment, including the
         * decision whether to copy it or not. Finally, we deliver to session all queued
         * messages coming from the additional fragments.
         */

        mess_queue.num_messages = 0;
        mess_queue.first = NULL;
        mess_queue.last = NULL;
        pack_body_ptr = (char *) Packets[pack_entry].body;
        frag_ptr = &(pack_ptr->first_frag_header);
        processed_bytes = frag_ptr->fragment_len;
        while( processed_bytes < pack_ptr->data_len )
        {
                padding_bytes = 0;
                switch (processed_bytes % 4)
                {
                case 1:
                        padding_bytes++;
                case 2:
                        padding_bytes++;
                case 3:
                        padding_bytes++;
                case 0:
                        /* already aligned */
                        break;
                }
                processed_bytes += padding_bytes;
                frag_ptr = (fragment_header *) &pack_body_ptr[processed_bytes];
                processed_bytes += sizeof(fragment_header);
                if ( processed_bytes + frag_ptr->fragment_len > pack_ptr->data_len )
                {
                        Alarm( PRINT, "Deliver_packet: invalid packet with seq %d from %d, fragments exceed data_len %d %d %d\n", 
                               pack_ptr->seq, pack_ptr->transmiter_id, processed_bytes, frag_ptr->fragment_len, pack_ptr->data_len );
                        break;
                }

                /* Creating new message */
                mess_link = new(MESSAGE_LINK);
                mess_link->next = NULL;
                mess_link->mess = new(SCATTER);
                mess_link->mess->num_elements = 1;
                mess_link->mess->elements[0].len = frag_ptr->fragment_len;
                mess_link->mess->elements[0].buf = new(PACKET_BODY);
                memcpy(mess_link->mess->elements[0].buf, &pack_body_ptr[processed_bytes], frag_ptr->fragment_len);
                processed_bytes += frag_ptr->fragment_len;
                if ( mess_queue.num_messages == 0 ) {
                        mess_queue.first = mess_link;
                        mess_queue.last = mess_link;
                } else {
                        mess_queue.last->next = mess_link;
                        mess_queue.last = mess_link;
                }
                mess_queue.num_messages++;
        }

        if ( Is_reliable( pack_ptr->type ) &&
            pack_ptr->first_frag_header.fragment_index == -1 ) 
        {
                /*
                 * for reliable single-packets message : deliver regardless
                 * of what is already in the queue.
                 * This also applies to reliable multi-fragment packets where all
                 * fragments have fragment index -1.
                 */
                proc_index = MAX_PROCS_RING;
        }else{
                proc_index = Packets[pack_entry].proc_index;
        }

        up_ptr = &Up_queue[proc_index];

        if ( up_ptr->exist == 0 )
        { 
                /* no message for proc - need to create one */
                up_ptr->mess = new( SCATTER );
                up_ptr->mess->num_elements = 0;
                up_ptr->exist = 1;
        }

        /* validity check */
        index = pack_ptr->first_frag_header.fragment_index;
        if ( index < 0 ) index = -index;
        if ( up_ptr->mess->num_elements+1 != index )
        {
                Alarm( EXIT, "Deliver_packet: sequence error: sec is %d, should be %d\n",
                       pack_ptr->first_frag_header.fragment_index,
                       up_ptr->mess->num_elements+1 );
        }

        /* chain this packet */
        up_ptr->mess->num_elements++;
        up_ptr->mess->elements[index-1].len = Packets[pack_entry].head->first_frag_header.fragment_len;
        up_ptr->mess->elements[index-1].buf = (char *)Packets[pack_entry].body;
        if ( to_copy )
        {
                /* 
                 * copy the packet.
                 *
                 * Note, that the original packet space was delivered
                 * and a new space was left in Packets. This is done to
                 * guarantee that in case of a configuration change, the
                 * original packet space in down_queue will be the same as
                 * the one in the up_queue for messages that were partially sent.
                 */
                Packets[pack_entry].body = new(PACKET_BODY);
                memcpy( Packets[pack_entry].body, up_ptr->mess->elements[index-1].buf, 
                        Packets[pack_entry].head->data_len );
        }
        Packets[pack_entry].exist = 2;

        GlobalStatus.packet_delivered++;

        if ( pack_ptr->first_frag_header.fragment_index < 0 )
        {
                /* end of message */
                /* Push up big_scatter. i.e. up_ptr->mess */
                mess_link = new(MESSAGE_LINK);
                mess_link->mess = up_ptr->mess;
                up_ptr->exist = 0;
                Sess_deliver_message( mess_link );
        }

        /* Delivering all messages coming from non-first fragments */
        while( mess_queue.num_messages > 0 )
        {
                mess_link = mess_queue.first;
                mess_queue.first = mess_queue.first->next;
                mess_queue.num_messages--;
                Sess_deliver_message(mess_link);
        }
}

static void Deliver_all_reliable_packets_event( int dmy, void *dmy_ptr )
{
        Deliver_reliable_packets( 1, Highest_seq );
}

static  void    Deliver_reliable_packets( int32 start_seq, int num_packets )
{
        int             pack_entry;
        int             end_seq;
        int             i;

        if ( Memb_state() == EVS ) return;
        if ( Prot_delivery_threshold > RELIABLE_TYPE ) return;

        end_seq = start_seq+num_packets-1;
        if ( start_seq <= Last_delivered ) start_seq = Last_delivered + 1;

        for( i = start_seq; i <= end_seq  ; i++ )
        {
                pack_entry = i & PACKET_MASK;

                if ( Packets[pack_entry].exist == 1 )
                {
                        if ( Is_reliable( Packets[pack_entry].head->type ) &&
                            Packets[pack_entry].head->first_frag_header.fragment_index == -1 )
                        {
                                Alarm( PROTOCOL, "Deliver_reliable_packets: delivering packet %d\n", i );
                                Deliver_packet( pack_entry, 1 );
                        }
                }
        }
}

static void Deliver_agreed_packets_event( int dmy, void *dmy_ptr )
{
        Deliver_agreed_packets();
}

static  void    Deliver_agreed_packets()
{
        /* deliver all non-safe packets that are ordered and not delivered */

        int             pack_entry;
        int             i;

        if ( My_aru <= Last_delivered ) return;
        if ( Memb_state() == EVS ) return;
        if ( Prot_delivery_threshold > AGREED_TYPE ) return;

        for( i = Last_delivered+1; i <= My_aru; i++ )
        {
                pack_entry = i & PACKET_MASK;

                if ( Packets[pack_entry].exist == 1 ) 
                {
                        if ( !Is_safe( Packets[pack_entry].head->type ) )
                        {
                                Alarm( PROTOCOL, "Deliver_agreed_packets: delivering packet %d\n", i );
                                Deliver_packet( pack_entry, 1 );
                                Last_delivered++;
                        }else return;

                }else if ( Packets[pack_entry].exist == 2 ){

                        /* This is possible only for delivery prior to agreed */

                        if ( Is_regular( Packets[pack_entry].head->type ) >= AGREED_TYPE )
                                Alarmp( SPLOG_FATAL, PROTOCOL, "Deliver_agreed_packets: packet %d already delivered but had wrong type 0x%08X\n", 
                                        i, Packets[pack_entry].head->type );
                        
                        Alarmp( SPLOG_INFO, PROTOCOL, "Deliver_agreed_packets: packet %d was already delivered\n", i );
                        Last_delivered++;

                }else Alarm( EXIT, "Deliver_agreed_packets: Error, packet %d; exist is %d\n", i, Packets[pack_entry].exist );
        }
}

void    Discard_packets()
{
        int             pack_entry;
        packet_body     *body_ptr;
        up_queue        *up_ptr;
        int             proc_index;
        int             i;

        if ( Aru <= Last_discarded ) return;
        if ( Memb_state() == EVS )
        {
                int             found_hole;
                membership_id   reg_memb_id;

                if ( Aru != Last_seq ) return;

                /* Deliver packets that must be delivered before the transitional signal.
                 * Those up to the Aru for my old ring were delivered in Read_form2().
                 * So, it remains to deliver all packets up to the first hole or the first
                 * SAFE message. */
                Alarmp( SPLOG_INFO, PROTOCOL,
                        "Discard_packets: delivering messages after old ring Aru before transitional\n" );

                for( i = Last_discarded+1; i <= Highest_seq; i++ )
                {
                        pack_entry = i & PACKET_MASK;
                        if ( ! Packets[pack_entry].exist )
                                Alarmp( SPLOG_FATAL, PROTOCOL, "Discard_packets: (EVS before transitional) packet %d not exist\n", i);
                        if ( Packets[pack_entry].exist == 3 )
                        {
                                Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Found first Hole in %d\n", i);
                                break;
                        }
                        if ( Is_safe( Packets[pack_entry].head->type ) ) {
                                Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Found first SAFE message in %d", i);
                                break;
                        }
                        /* should deliver packet or dispose the body if it was delivered already */
                        if ( Packets[pack_entry].exist == 1 ){
                                Deliver_packet( pack_entry, 0 );
                        } else {
                                dispose( Packets[pack_entry].body );
                        }
                        /* dispose packet header in any case */
                        dispose( Packets[pack_entry].head );
                        Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: delivering %d in EVS\n",i);
                        Packets[pack_entry].exist = 0;
                        Last_discarded = i;
                }

                /* calculate and deliver transitional membership */
                Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Delivering transitional membership\n" );
                Memb_transitional();
                Sess_deliver_trans_memb( Trans_membership, Memb_trans_id() );

                /* deliver all remaining packets for EVS */
                found_hole = 0;
                for( i = Last_discarded+1; i <= Highest_seq; i++ )
                {
                        pack_entry = i & PACKET_MASK;
                        if ( ! Packets[pack_entry].exist )
                                Alarm( EXIT, "Discard_packets: (EVS after transitional) packet %d not exist\n", i);
                        if ( Packets[pack_entry].exist == 3 )
                        {
                                /* 
                                 * There is a hole! 
                                 * from here, we need to check if the proc_id of the packet
                                 * is in commited membership. 
                                 */
                                found_hole = 1;
                                Alarm( PROTOCOL, "Discard_packets: Found a Hole in %d \n",i);

                        }else if ( !found_hole || Conf_id_in_conf( &Commit_membership, Packets[pack_entry].head->proc_id ) != -1 ){
                                /* should deliver packet or dispose the body if it was delivered already */
                                if ( Packets[pack_entry].exist == 1 ){
                                        Deliver_packet( pack_entry, 0 );
                                }else{
                                        dispose( Packets[pack_entry].body );
                                }
                                /* dispose packet header in any case */
                                dispose( Packets[pack_entry].head );
                                Alarm( PROTOCOL, "Discard_packets: delivering %d in EVS\n",i);
                        }else{
                                /* should not deliver packet */
                                dispose( Packets[pack_entry].head );
                                dispose(   Packets[pack_entry].body );
                                Alarm( PROTOCOL, "Discard_packets: Due to hole, not delivering %d \n",i);
                        }
                        Packets[pack_entry].exist = 0;
                }

                for ( i = 0; i < MAX_PACKETS_IN_STRUCT; ++i )
                        if ( Packets[i].exist )
                                Alarmp( SPLOG_FATAL, PROTOCOL, "Discard_packets: Just delivered all packets, but some (%d) still exist?!!!\n", i );

                /* check up_queue and down_queue */
                if ( Down_queue_ptr->num_mess > 0 )
                {
                        Down_queue_ptr->cur_element = 0;
                }

                for( proc_index=0; proc_index < MAX_PROCS_RING; proc_index++ )
                {
                        if ( Up_queue[proc_index].exist )
                        {
                                if ( proc_index != My_index )
                                {
                                        /* 
                                         * dispose only packets that are not mine 
                                         * my packets will stay in Down_queue if the message is not
                                         * ready to be delivered (because not fully sent yet) 
                                         * so we need not to dispose them! 
                                         */
                                        up_ptr = &Up_queue[proc_index];
                                        for( i=0; i < (int) up_ptr->mess->num_elements; i++ )
                                        {
                                                body_ptr = (packet_body *)up_ptr->mess->elements[i].buf;
                                                dispose( body_ptr );
                                        }
                                }
                                dispose( Up_queue[proc_index].mess );
                                Up_queue[proc_index].exist = 0;
                        }
                }

                /* calculate and deliver regular membership */
                Memb_regular();

                Log_membership();
                reg_memb_id = Memb_id();
                Sess_deliver_reg_memb( Reg_membership, reg_memb_id );

                /* If in change conf mode; then if singleton (which should be true) and GOP state then:
                 * Remove partition
                 * Initiate Memb_lookup() to find other daemons 
                 */
                if ( Conf_in_reload_singleton_state() ) {
                        /* GOP state equals value 1, but is private declaration in groups.c */
                        if ( (GlobalStatus.gstate != 1 ) || ( Conf_num_procs( &Reg_membership ) != 1 ) ) {
                                Alarmp( SPLOG_FATAL, PROTOCOL, "Discard_packets: Failed to reload configuration - gstate: %d and num_procs in membership: %d\n", 
                                        GlobalStatus.gstate, Conf_num_procs( &Reg_membership) );
                        }
                        Net_clear_partition();
                        E_queue( Memb_lookup_new_members_event, 0, NULL, Zero_timeout);
                        Conf_reload_singleton_state_end();
                }

                /* set variables for next membership */
                if ( Conf_debug_initial_sequence() ) {  /* TODO: get rid of this debug stuff */
                        Highest_seq      = INITIAL_SEQUENCE_NEAR_WRAP;
                        Highest_fifo_seq     = INITIAL_SEQUENCE_NEAR_WRAP;
                        My_aru           = INITIAL_SEQUENCE_NEAR_WRAP;
                        Aru                      = INITIAL_SEQUENCE_NEAR_WRAP;
                        Last_discarded   = INITIAL_SEQUENCE_NEAR_WRAP;
                        Last_delivered   = INITIAL_SEQUENCE_NEAR_WRAP;
                } else {
                        Highest_seq      = 0;
                        Highest_fifo_seq     = 0;
                        My_aru           = 0;
                        Aru                      = 0;
                        Last_discarded   = 0;
                        Last_delivered   = 0;
                }

                Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Updated Aru to %d for next membership\n", Aru );

                GlobalStatus.my_aru      = My_aru;
                Token_counter   = 0;

        }else{

                for( i = Last_discarded+1; i <= Aru; i++ )
                {
                        pack_entry = i & PACKET_MASK;
                        if ( ! Packets[pack_entry].exist )
                                Alarm( EXIT, "Discard_packets: (NOT EVS) packet %d not exist\n",i);

                        /* should deliver packet or dispose the body if it was delivered already */
                        if ( Packets[pack_entry].exist == 1 ) Deliver_packet( pack_entry, 0 );
                        else dispose( Packets[pack_entry].body );
                        /* dispose packet header in any case */
                        dispose( Packets[pack_entry].head );
                        Packets[pack_entry].exist = 0;
                }
                Alarm( PROTOCOL, "Discard_packets: packets %d-%d were discarded\n",
                       Last_discarded+1, Aru );

                Last_discarded = Aru;
                if ( Last_delivered < Last_discarded ) Last_delivered = Last_discarded;
        }
}

/* The Token is in "hold" state at a leader or non-leader daemon 
 * if the following conditions hold:
 *
 * 1) A normal token is circling:
 *      "Memb_state() == OP or GATHER with Alive token"
 *      These are the only states when a token that allows message sends is circling
 * 2) All messages are stable at all daemons (i.e. every daemon knows that all messages are stable):
 *      "ARU == Highest_Seq"
 *      Otherwise we need to cycle the token so all daemons will learn about the messages and the ARU of
 *      other daemons so they can laearn about stability of all messages.
 * 3) Only one copy of a token with a particular ARQ value has been sent around the ring:
 *      "Get_retrans(Last_token->type) <= 1"
 *      This is true when the token is sent on in the Prot_handle_token() function after being recreated or
 *      if it is sent the first time Prot_token_hurry() is called because of a timeout after Prot_handle_token has created a new token.
 * 4) The token are circulated at least once without doing any work (no new messages or updates to counters)
 *    AND no daemon has requested the token in order to send new messages (sent a Hurry request)
 *      "Token_counter > 1"
 *      This measures the lack of interest by any daemon in introducing new work and is only used in Spread
 *      to detect this lack of interest - i.e. it has no other role. 
 */
static  int     Is_token_hold()
{
        if ( ( Memb_state() == OP || 
              ( Memb_state() == GATHER && Memb_token_alive() ) )&&
            Get_retrans(Last_token->type) <= 1                  &&
            Aru == Highest_seq && Token_counter > 1 ) return ( 1 );
        else return( 0 );
}

static  int     To_hold_token()
{
        if ( ( Memb_state() == OP || 
              ( Memb_state() == GATHER && Memb_token_alive() ) )&&
            Get_retrans(Last_token->type) <= 1                  &&
            Aru == Highest_seq && Token_counter > 1 ) return ( 1 );
        else return( 0 );
}

static  void    Handle_hurry( packet_header *pack_ptr )
{
        if ( Conf_leader( Memb_active_ptr() ) == My.id &&
            Is_token_hold() ) 
        {
                if ( Conf_id_in_conf( Memb_active_ptr(), pack_ptr->proc_id ) != -1 )
                {
                        Alarm( PROTOCOL, "Handle_hurry: sending token now\n");
                        /* Reset token_counter so token protocol knows someone wants to send 
                         * and token should not go into hold state until everyone gets a chance
                         * to send.
                         */
                        Token_counter = 0;
                        Prot_token_hurry();
                }
        }
}

void    Prot_token_hurry(void)
{
        /* asked to send token again (almost lost) */
                        
        sys_scatter     retrans_token;
        int32           val;

        retrans_token.num_elements = 1;
        retrans_token.elements[0].len = sizeof(token_header);
        retrans_token.elements[0].buf = (char *)Last_token;
        Last_token->rtr_len=0;
        if ( Conf_leader( Memb_active_ptr() ) == My.id ) 
        {
                val = Get_retrans(Last_token->type);
                val++;
                Last_token->type = Set_retrans( Last_token->type, val );
                E_queue( Prot_token_hurry_event, 0, NULL, Hurry_timeout );
                GlobalStatus.token_hurry++;
        }
        /* sending token */
        Net_send_token( &retrans_token );
        if ( Wide_network && 
            Conf_seg_last(Memb_active_ptr(), My.seg_index) == My.id )
        {
                /* sending again to another segment */
                Net_send_token( &retrans_token );
        }
        if ( Get_retrans( Last_token->type ) > 1 )
        {
                /* problems */ 
                Net_send_token( &retrans_token );
                Net_send_token( &retrans_token );
        }

        Alarm( PROTOCOL, "Prot_token_hurry: retransmiting token %d %d\n",
               Get_arq(Last_token->type), Get_retrans(Last_token->type) );
}

void    Prot_token_hurry_event(int dmy, void *dmy_ptr)
{
        Prot_token_hurry();
}

void    Prot_set_prev_proc(configuration *memb)
{
        Prev_proc_id = Conf_previous(memb);
        Alarm( PROTOCOL, "Prev_proc_id: %d, My.id: %d\n", Prev_proc_id, My.id );
}

void    Flip_token_body( char *buf, token_header *token_ptr )
{
/*
 * This routine can not be called twice for the same buffer because
 * of ring_rtr_ptr->num_seq.
 */
        ring_rtr        *ring_rtr_ptr;
        int32           *req_seq;
        char            *rtr;
        int             ptr;
        int             i;

        if ( token_ptr->rtr_len <= 0 ) return;

        rtr = buf;
        ptr = 0;
        while( ptr < token_ptr->rtr_len )
        {
                ring_rtr_ptr = (ring_rtr *)&rtr[ptr];

                ring_rtr_ptr->memb_id.proc_id = Flip_int32( ring_rtr_ptr->memb_id.proc_id );
                ring_rtr_ptr->memb_id.time    = Flip_int32( ring_rtr_ptr->memb_id.time );
                ring_rtr_ptr->proc_id     = Flip_int32( ring_rtr_ptr->proc_id );
                ring_rtr_ptr->seg_index   = Flip_int16( ring_rtr_ptr->seg_index );
                ring_rtr_ptr->num_seq     = Flip_int16( ring_rtr_ptr->num_seq );

                ptr += sizeof(ring_rtr);
                for( i=0; i < ring_rtr_ptr->num_seq; i++ )
                {
                        req_seq = (int32 *)&rtr[ptr];
                        *req_seq = Flip_int32( *req_seq );
                        ptr += sizeof(int32);
                }       
        }
}

void Flip_frag( fragment_header *frag_ptr )
{
        frag_ptr->fragment_index = Flip_int16( frag_ptr->fragment_index );
        frag_ptr->fragment_len   = Flip_int16( frag_ptr->fragment_len );
}

int Prot_get_delivery_threshold( void )
{
        return Prot_delivery_threshold;
}

/* NOTE: This fcn only affects regular delivery functions like Deliver_reliable_packets, Deliver_agreed_packets, etc.
         This fcn does not affect direct, low-level calls to Deliver_packet, which will always deliver a packet.
         Notably, it does not affect Discard_packets (which is essentially Deliver_safe_packets) because that
         is a low level fcn deeply tied into membership, etc., and it only calls Deliver_packet.
*/

void Prot_set_delivery_threshold( int new_thresh )
{
        int old_thresh = Prot_delivery_threshold;

        switch ( new_thresh )
        {
        case UNRELIABLE_TYPE:
        case RELIABLE_TYPE:
        case FIFO_TYPE:
        /*case CAUSAL_TYPE:*/
        case AGREED_TYPE:
        case SAFE_TYPE:
        case BLOCK_REGULAR_DELIVERY:
                break;

        default:
                Alarmp( SPLOG_FATAL, PROTOCOL, "Prot_set_delivery_threshold: illegal service 0x%08X\n", new_thresh );
                break;
        }

        Prot_delivery_threshold = new_thresh;

        if ( new_thresh < old_thresh )
        {
                /* NOTE: we de/queue callbacks here rather than calling directly to avoid "recursive" calls that lead to improper operation */

                switch ( new_thresh )
                {
                case UNRELIABLE_TYPE:
                case RELIABLE_TYPE:       E_queue( Deliver_all_reliable_packets_event, 0, NULL, Zero_timeout );
                case FIFO_TYPE:
                /*case CAUSAL_TYPE:*/
                case AGREED_TYPE:         E_queue( Deliver_agreed_packets_event, 0, NULL, Zero_timeout );
                case SAFE_TYPE:       
                case BLOCK_REGULAR_DELIVERY:
                        break;
                }
        }
        else if ( new_thresh > old_thresh )
        {
                switch ( new_thresh )
                {
                case BLOCK_REGULAR_DELIVERY:
                case SAFE_TYPE:           E_dequeue( Deliver_agreed_packets_event, 0, NULL );
                case AGREED_TYPE:
                /*case CAUSAL_TYPE:*/
                case FIFO_TYPE:           E_dequeue( Deliver_all_reliable_packets_event, 0, NULL );
                case RELIABLE_TYPE:
                case UNRELIABLE_TYPE:
                        break;
                }
        }
}
