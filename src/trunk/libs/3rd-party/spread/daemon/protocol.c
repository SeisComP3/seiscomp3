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
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */


#define	ext_prot_body

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
#include "sess_types.h" /* for message_header */

/* Prot variables */
static	proc		My;
static	int		My_index;

static	int32		Set_aru;
static	int		Token_counter;

/* Used ONLY in Prot_handle_bcast, inited in Prot_init */
static	sys_scatter	New_pack;

/* Used ONLY in Prot_handle_token and grurot, inited in Prot_init */
static	sys_scatter	New_token;
static	token_header	*Token;
static	sys_scatter	Send_pack;

static	packet_header	*Hurry_head;
static	sys_scatter	Hurry_pack;
static  sp_time         Zero_timeout    = {  0, 0};

/* Used to indicate a need to reload configuration at end of current membership */
static  bool            Prot_Need_Conf_Reload  = FALSE;

/* ### Pack: 1 line */
static	packet_info	Buffered_packets[ARCH_SCATTER_SIZE];

static	down_queue	Protocol_down_queue[2]; /* only used in spread3 */

static	void	Prot_handle_bcast();
static	void	Prot_handle_token();
static	int	Answer_retrans( int *ret_new_ptr, int32 *proc_id, int16 *seg_index );
static	int	Send_new_packets( int num_allowed );
static	int	Is_token_hold();
static	int	To_hold_token();
static	void	Handle_hurry( packet_header *pack_ptr );
static	void	Deliver_packet( int pack_entry, int to_copy );
static	void	Flip_token_body( char *buf, token_header *token_ptr );
static	void	Deliver_reliable_packets( int32 start_seq, int num_packets );
static	void	Deliver_agreed_packets();

static  void    Prot_handle_conf_reload(sys_scatter *scat);

void	Prot_init(void)
{
	int	i, num_bcast, num_token;
        channel *bcast_channels;
        channel *token_channels;

	Mem_init_object( PACK_HEAD_OBJ, "pack_head", sizeof( packet_header ), MAX_PACKETS_IN_STRUCT, 0 );
	Mem_init_object( PACKET_BODY, "packet_body", sizeof( packet_body ), MAX_PACKETS_IN_STRUCT, 0 );
	Mem_init_object( TOKEN_HEAD_OBJ, "token_head", sizeof( token_header ), 10, 0 );
	Mem_init_object( TOKEN_BODY_OBJ, "token_body", sizeof( token_body ), 10, 0 );
	Mem_init_object( SCATTER, "scatter", sizeof( scatter ), 200+MAX_PROCS_RING, 0 );

	My = Conf_my();
	My_index = Conf_proc_by_id( My.id, &My );
	GlobalStatus.my_id = My.id;
	GlobalStatus.packet_delivered = 0;

	for( i=0; i < MAX_PROCS_RING+1; i++ )
		Up_queue[i].exist = 0;

	for( i=0; i < MAX_PACKETS_IN_STRUCT; i++ )
		Packets[i].exist = 0;

        if ( Conf_debug_initial_sequence() ) {
            Highest_seq 	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Highest_fifo_seq     = INITIAL_SEQUENCE_NEAR_WRAP;
            My_aru	    	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Aru		         = INITIAL_SEQUENCE_NEAR_WRAP;
            Set_aru		 = INITIAL_SEQUENCE_NEAR_WRAP -1;
            Last_discarded	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Last_delivered	 = INITIAL_SEQUENCE_NEAR_WRAP;
        } else {
            Highest_seq 	 = 0;
            Highest_fifo_seq     = 0;
            My_aru	    	 = 0;
            Aru		         = 0;
            Set_aru		 = -1;
            Last_discarded	 = 0;
            Last_delivered	 = 0;
        }

	New_pack.num_elements = 2;
	New_pack.elements[0].len = sizeof(packet_header);
	New_pack.elements[0].buf = (char *) new(PACK_HEAD_OBJ);
	New_pack.elements[1].len = sizeof(packet_body);
	New_pack.elements[1].buf = (char *) new(PACKET_BODY);

	New_token.num_elements	= 2;
	New_token.elements[0].len = sizeof(token_header);
	New_token.elements[0].buf = (char *) new(TOKEN_HEAD_OBJ);
	New_token.elements[1].len = sizeof(token_body);
	New_token.elements[1].buf = (char *) new(TOKEN_BODY_OBJ);

	Send_pack.num_elements = 2;
	Send_pack.elements[0].len = sizeof(packet_header);

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

	FC_init( );
	Memb_init();

	Net_set_membership( Reg_membership );

}

void    Prot_init_down_queues(void)
{
        Protocol_down_queue[NORMAL_DOWNQUEUE].num_mess	= 0 ;
	Protocol_down_queue[NORMAL_DOWNQUEUE].cur_element = 0;
	Protocol_down_queue[GROUPS_DOWNQUEUE].num_mess	= 0 ;
	Protocol_down_queue[GROUPS_DOWNQUEUE].cur_element = 0;
}

void	Prot_set_down_queue( int queue_type )
{
        switch(queue_type) {
        case NORMAL_DOWNQUEUE:
                Down_queue_ptr = &Protocol_down_queue[NORMAL_DOWNQUEUE];
                break;
        case GROUPS_DOWNQUEUE:
                Down_queue_ptr = &Protocol_down_queue[GROUPS_DOWNQUEUE];
                break;
        default:
                Alarm(EXIT, "Prot_set_down_queue: Illegal queue_type (%d)\n", queue_type);
        }
}

void    Prot_Create_Local_Session(session *new_sess)
{
        /* Nothing to do for Spread3 */
        return;
}
void    Prot_Destroy_Local_Session(session *old_sess)
{
        /* Nothing to do for Spread3 */
        return;
}
void    Prot_kill_session(message_obj *msg)
{
        /* Nothing to do for Spread3 */
        return;
}
down_link       *Prot_Create_Down_Link(message_obj *msg, int type, int mbox, int cur_element)
{
        down_link       *down_ptr;
        message_header  *head_ptr;

	if ((down_ptr = new( DOWN_LINK )) == NULL )
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

static  void	Prot_handle_bcast(channel fd, int dummy, void *dummy_p)
{
	packet_header	*pack_ptr;
	int		pack_entry;
	proc		p;
	int		received_bytes;
        int             total_bytes_processed;
        int             num_buffered_packets;
	int		i;
	int32		j;
	/* int		r1,r2; */

	received_bytes = Net_recv( fd, &New_pack );
	/* My own packet or from another monitor component */
	if( received_bytes == 0 ) return;
	/* problem in receiving */
	if( received_bytes < 0 ) return;

	pack_ptr = (packet_header *)New_pack.elements[0].buf;

	/* ### Pack, this has to move down to network.c
	 * if( pack_ptr->data_len +sizeof(packet_header) != received_bytes )
	 * {
	 *	Alarm( PRINT, "Prot_handle_bcast: received %d, should be %d\n",
	 *	received_bytes, pack_ptr->data_len+sizeof(packet_header) );
	 * 	return; 
	 * }
	 */

	if( Is_status( pack_ptr->type ) )
	{
		Stat_handle_message( &New_pack );
		return;
	}

	if( Is_fc( pack_ptr->type ) )
	{
		FC_handle_message( &New_pack );
		return;
	}

        if( Is_conf_reload( pack_ptr->type ) )
        {
                Prot_handle_conf_reload( &New_pack );
                return;
        }
	/* delete random 
	r1 = ((-My.id)%17)+3;
	r2 = get_rand() % (r1+3 );
	if ( r2 == 0 ) return; */

	if( Is_membership( pack_ptr->type ) )
	{
		Memb_handle_message( &New_pack );
		return;
	}
	if( Is_hurry( pack_ptr->type ) )
	{
		Handle_hurry( pack_ptr );
		return;
	}
	if( !Is_regular( pack_ptr->type ) )
	{
		Alarm( PROTOCOL, "Prot_handle_bcast: Unknown packet type %d\n",
			pack_ptr->type );
		return;
	}
	if( ! Memb_is_equal( Memb_id(), pack_ptr->memb_id ) )
	{
		/* Foreign message */
		Memb_handle_message( &New_pack );
		return;
	}
        if (Memb_token_alive() ) {
                E_queue( Memb_token_loss, 0, NULL, Token_timeout );
                if( Conf_leader( Memb_active_ptr() ) == My.id ) 
                {
                        E_queue( Prot_token_hurry, 0, NULL, Hurry_timeout );
                }
        }

	/* ### Pack: next 70 lines (almost till the end of the routine) have changed */
        Buffered_packets[0].head = pack_ptr;
        Buffered_packets[0].body = (packet_body *)New_pack.elements[1].buf;
	received_bytes -= sizeof(packet_header);
        total_bytes_processed = pack_ptr->data_len;
        /* ignore any alignment padding */
        switch(total_bytes_processed % 4)
        {
        case 1:
                total_bytes_processed++;
        case 2:
                total_bytes_processed++;
        case 3:
                total_bytes_processed++;
        case 0:
                /* already aligned */
                break;
        }
        for( i = 1; received_bytes > total_bytes_processed; i++ )
        {                
        	/* copy into each of the elements after the first element*/
                Buffered_packets[i].head = (packet_header *)new(PACK_HEAD_OBJ);
                Buffered_packets[i].body = (packet_body *)new(PACKET_BODY);
                if (Buffered_packets[i].head == NULL) 
                        Alarm(EXIT, "Prot_handle_bcast: Memory allocation failed for PACK_HEAD_OBJ\n");
                if (Buffered_packets[i].body == NULL) 
                        Alarm(EXIT, "Prot_handle_bcast: Memory allocation failed for PACKET_BODY\n");
                        
                pack_ptr = (packet_header *)&New_pack.elements[1].buf[total_bytes_processed];
                memcpy( Buffered_packets[i].head, pack_ptr, sizeof( packet_header ) );
                total_bytes_processed += sizeof(packet_header);
                memcpy( Buffered_packets[i].body, &New_pack.elements[1].buf[total_bytes_processed], pack_ptr->data_len);
                total_bytes_processed += pack_ptr->data_len;
                /* ignore any alignment padding */
                switch(total_bytes_processed % 4)
                {
                case 1:
                        total_bytes_processed++;
                case 2:
                        total_bytes_processed++;
                case 3:
                        total_bytes_processed++;
                case 0:
                        /* already aligned */
                        break;
                }
        }
        num_buffered_packets = i;

        for( i = 0; i < num_buffered_packets; i++)
        {
                pack_ptr = Buffered_packets[i].head;
                 
                /* do we have this packet */
                if( pack_ptr->seq <= Aru )
                {
                        Alarm( PROTOCOL, "Prot_handle_bcast: delayed packet %d already delivered (Aru %d)\n",
                               pack_ptr->seq, Aru );
                        dispose(Buffered_packets[i].head);
                        dispose(Buffered_packets[i].body);
                        continue;
                }
                pack_entry = pack_ptr->seq & PACKET_MASK;
                if( Packets[pack_entry].exist ) 
                {
                        Alarm( PROTOCOL, "Prot_handle_bcast: packet %d already exist\n",
                               pack_ptr->seq );
                        dispose(Buffered_packets[i].head);
                        dispose(Buffered_packets[i].body);
                        continue;
                }

                Packets[pack_entry].proc_index = Conf_proc_by_id( pack_ptr->proc_id, &p );
                if( Packets[pack_entry].proc_index < 0 )
                {
                        Alarm( PROTOCOL, "Prot_handle_bcast: unknown proc %d\n", pack_ptr->proc_id );
                        dispose(Buffered_packets[i].head);
                        dispose(Buffered_packets[i].body);
                        continue;
                }
                /* insert new packet */
                Packets[pack_entry].head  = pack_ptr;
                Packets[pack_entry].body  = Buffered_packets[i].body;
                Packets[pack_entry].exist = 1;

                /* update variables */
                if( Highest_seq < pack_ptr->seq ) Highest_seq = pack_ptr->seq;
                if( pack_ptr->seq == My_aru+1 )
                {
                        for( j=pack_ptr->seq; j <= Highest_seq; j++ )
                        {
                                if( ! Packets[j & PACKET_MASK].exist ) break;
                                My_aru++;
                        }
                        Deliver_agreed_packets();
                }else Deliver_reliable_packets( pack_ptr->seq, 1 );

                Alarm( PROTOCOL, "Prot_handle_bcast: packet %d inserted\n",
                       pack_ptr->seq );
        }      /* END OF LOOP */

        GlobalStatus.packet_recv++;
	GlobalStatus.my_aru = My_aru;
	GlobalStatus.highest_seq = Highest_seq;

	/* prepare New_pack for next packet */
	New_pack.elements[0].buf = (char *) new(PACK_HEAD_OBJ);
	New_pack.elements[1].buf = (char *) new(PACKET_BODY);
}

void	Prot_handle_token(channel fd, int dummy, void *dummy_p)
{
	int		new_ptr;
	int		num_retrans, num_allowed, num_sent;
	int		flow_control;
	char		*new_rtr;
	ring_rtr	*ring_rtr_ptr;
	int32		rtr_proc_id;
	int16		rtr_seg_index;
	int32		val;
	int		retrans_allowed; /* how many of my retrans are allowed on token */
	int		i, ret;

	/*	int		r1,r2;*/

	ret = Net_recv_token( fd, &New_token );
	/* from another monitor component */
	if( ret == 0 ) return;

	/* delete random
	r1 = ((-My.id)%17)+3;
	r2 = get_rand() % (r1+3 );
	if ( r2 == 0 ) return; */

        Alarm( DEBUG, "Received Token\n");
	/* check if it is a regular token */
	if( Is_form( Token->type ) )
	{
                Alarm(PROTOCOL, "it is a Form Token.\n");
                Memb_handle_token( &New_token );
		return;
	}

	/* The Veto property for tokens - swallow this token */
	if( ! Memb_token_alive() ) {
                Alarm(PROTOCOL, "Prot_handle_token: Veto Property. Memb not alive.\n");
		return;
        }

	if( ret != sizeof(token_header) + Token->rtr_len )
	{
		Alarm( PRINT, 
		    "Prot_handle_token: recv token len is %d, should be %d\n",
		    ret,sizeof(token_header) + Token->rtr_len );
		return;
	}

	if( !Same_endian( Token->type ) ) 
		Flip_token_body( New_token.elements[1].buf, Token );

        /* Deal with wrapping seq values (2^32) by triggering a membership by dropping the token */
        if( (Memb_state() != EVS ) && (Token->seq > MAX_WRAP_SEQUENCE_VALUE ) )
        {
            Alarm( PRINT, "Prot_handle_token: Token Sequence number (%ld) approaching 2^31 so trigger membership to reset it.\n", Token->seq);
            /* return swallows token and will break ring and trigger membership */
            return;
        }

	if( Conf_leader( Memb_active_ptr() ) == My.id )
	{
		if( Get_arq(Token->type) != Get_arq(Last_token->type) )
		{
		    Alarm( PROTOCOL, 
			"Prot_handle_token: leader swallowing token %d %d %d\n", 
			 Get_arq(Token->type),Get_retrans(Token->type),Get_arq(Last_token->type) );
		    /* received double token - swallow it */
		    return; 
		}
	}else{
		if( Get_arq(Token->type) == Get_arq(Last_token->type) )
		{
		    if( Get_retrans(Token->type) > Get_retrans(Last_token->type) )
		    {
			val = Get_retrans(Token->type);
			Last_token->type = Set_retrans(Last_token->type,val);
			/* asked to send token again (almost lost) */
			Alarm( PROTOCOL,
			    "Prot_handle_token: not leader, asked to retrans %d %d\n",
				Get_arq(Token->type), val );
			Prot_token_hurry();
		    }else{
			Alarm( PROTOCOL,
			    "Prot_handle_token: not leader, swallow same token %d %d\n",
	   			Get_arq(Token->type), Get_retrans(Token->type) );
		    }
		    return;

		} else if ( Get_arq(Token->type) != ( ( Get_arq( Last_token->type ) + 1 ) % 0x10 ) ) {
		  Alarm( PROTOCOL, 
			 "Prot_handle_token: not leader, swallowing very outdated token: ARQ(%d) RETRANS(%d) vs. Last ARQ(%d)\n",
			 Get_arq(Token->type), Get_retrans(Token->type), Get_arq(Last_token->type) );
		  return;

		} else {
                        if ( Get_retrans(Token->type) > 0 ) {
                                GlobalStatus.token_hurry++;
                        }
                }
	}
	if( Highest_seq < Token->seq ) Highest_seq = Token->seq;
		
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
	Last_num_sent	 = num_sent;

	/* Prepare my retransmission requests */

	for( i = My_aru+1; i <= Highest_seq; i++ )
	{
		if( ! Packets[i & PACKET_MASK].exist ) break;
		My_aru++;
	}
	GlobalStatus.my_aru = My_aru;

	if( My_aru < Highest_seq )
	{
	    /* Compute how many of my retransmission requests are possible to fit */
	    retrans_allowed = ( sizeof( token_body ) - new_ptr - sizeof( ring_rtr ) ) / sizeof( int32 );
	    if( retrans_allowed > 1 )
	    {
	    	ring_rtr_ptr = (ring_rtr *)&new_rtr[new_ptr];
	    	ring_rtr_ptr->memb_id 	= Memb_id();
	    	ring_rtr_ptr->proc_id	= rtr_proc_id;
	    	ring_rtr_ptr->seg_index	= rtr_seg_index;
	    	ring_rtr_ptr->num_seq	= 0;
	    	new_ptr += sizeof(ring_rtr);
	    	for( i=My_aru+1; i <= Highest_seq && retrans_allowed > 0; i++ )
	    	{
			if( ! Packets[i & PACKET_MASK].exist ) 
			{
				memcpy( &new_rtr[new_ptr], &i, sizeof(int32) ); 
				new_ptr += sizeof(int32);
				ring_rtr_ptr->num_seq++;
				--retrans_allowed;
			}
	    	}
	    }
	}

	if( Memb_state() == EVS )
	{
		if( My_aru == Highest_seq )
		{
			My_aru = Last_seq;
			Memb_commit();
		}
	}

	Token->rtr_len = new_ptr;
	New_token.elements[1].len = new_ptr;

	/* Calculating Token->aru and Set_aru */
	if( ( Token->aru == Set_aru    ) ||
            ( Token->aru_last_id == My.id ) ||
	    ( Token->aru == Token->seq ) )
	{
		Token->aru = My_aru;
                Token->aru_last_id = My.id;
		if( My_aru < Highest_seq ) Set_aru = My_aru;
		else Set_aru = -1;
	}else if( Token->aru > My_aru ) {
		Token->aru = My_aru;
                Token->aru_last_id = My.id;
		Set_aru    = My_aru;
	}else{
		Set_aru    = -1;
	}
	
	Token->proc_id = My.id;
	if( Memb_state() != EVS ) Token->seq = Highest_seq;

	if( Conf_leader( Memb_active_ptr() ) == My.id )
	{
	    	val = Get_arq( Token->type );
		val = (val + 1)% 0x10;
		Token->type = Set_arq(     Token->type, val );
		Token->type = Set_retrans( Token->type, 0   );
	}

	/* Send token */
	if( ! ( Conf_leader( Memb_active_ptr() ) == My.id &&
		To_hold_token() ) )
	{
		/* sending token */
		Net_send_token( &New_token );
/* ### Bug fix for SGIs */
#ifdef	ARCH_SGI_IRIX
		Net_send_token( &New_token );
#endif  /* ARCH_SGI_IRIX */

		if( Get_retrans( Token->type ) > 1 )
		{
			/* problems */ 
			Net_send_token( &New_token );
			Net_send_token( &New_token );
		}
	}

	Token_rounds++;

	if( Conf_leader( Memb_active_ptr() ) == My.id ) 
		E_queue( Prot_token_hurry, 0, NULL, Hurry_timeout );

	E_queue( Memb_token_loss, 0, NULL, Token_timeout );


	/* calculating Aru */
	if( Token->aru > Last_token->aru )
		Aru = Last_token->aru;
	else
		Aru = Token->aru;
	if( Highest_seq == Aru ) Token_counter++;
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

	if( Memb_state() == EVS && Token_rounds > MAX_EVS_ROUNDS ) 
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
        E_queue( Memb_token_loss, 0, NULL, Zero_timeout );
}


void	Prot_new_message( down_link *down_ptr, int not_used_in_spread3_p )
{
	int32	leader_id;

	if( Down_queue_ptr->num_mess > 0 )
	{
		down_ptr->next = NULL;
		Down_queue_ptr->last->next = down_ptr;
		Down_queue_ptr->last = down_ptr;
	}else if( Down_queue_ptr->num_mess == 0 ){
		Down_queue_ptr->first = down_ptr;
		Down_queue_ptr->last  = down_ptr;
	}else{
		Alarm( EXIT,"fast_spread_new_message: Down_queue_ptr->num_mess is %d\n",
			Down_queue_ptr->num_mess );
	}
	Down_queue_ptr->num_mess++;
	if( Down_queue_ptr->num_mess >= WATER_MARK ) 
		Sess_block_users_level();

	if( Down_queue_ptr->num_mess == 1  && Is_token_hold() )
	{
		leader_id = Conf_leader( Memb_active_ptr() );
		if( leader_id == My.id )
		{
			Handle_hurry( Hurry_head );
		}else{
			Net_ucast( leader_id, &Hurry_pack );
		}
	}
}

/* ### Pack: this routine has changed */
static	int	Answer_retrans( int *ret_new_ptr, 
				int32 *proc_id, int16 *seg_index )
{
	int		num_retrans;
	char		*rtr;
	int		old_ptr,new_ptr;
	ring_rtr	*ring_rtr_ptr;
	int		pack_entry;
	int		bytes_to_copy;
	packet_header	*pack_ptr;
	int		i, ret;
	int32		*req_seq;

        num_retrans     = 0;
        new_ptr         = 0;
        *proc_id     = My.id;
        *seg_index   = My.seg_index;
        if( Token->rtr_len > 0 )
        {
            rtr = New_token.elements[1].buf;
            old_ptr = 0;
            while( old_ptr < Token->rtr_len )
            {
                ring_rtr_ptr = (ring_rtr *)&rtr[old_ptr];
                if( Memb_is_equal(ring_rtr_ptr->memb_id,Memb_id() ) )
                {
                    /* retransmit requests from my ring */
                    old_ptr += sizeof(ring_rtr);
                    for( i=0; i < ring_rtr_ptr->num_seq; i++ )
                    {
			req_seq = (int32 *)&rtr[old_ptr];
                        old_ptr += sizeof(int32);
                        pack_entry = *req_seq & PACKET_MASK;
                        if( *req_seq < Aru ) 
                                Alarm( EXIT, 
                "Answer_retrans: retrans of %d requested while Aru is %d\n",
                                *req_seq,Aru );

                        if( Packets[pack_entry].exist )
                        {
                                pack_ptr = Packets[pack_entry].head;
                                Send_pack.elements[0].buf = 
                                    (char *)Packets[pack_entry].head;
                                Send_pack.elements[1].buf = 
                                    (char *)Packets[pack_entry].body;
                                Send_pack.elements[1].len = 
                                    pack_ptr->data_len; 

                                if( ring_rtr_ptr->proc_id != -1 )
                                {
                                    ret = Net_ucast ( ring_rtr_ptr->proc_id, &Send_pack );
				    GlobalStatus.u_retrans++;

                                    Alarm( PROTOCOL, 
        "Answer_retrans: retransmit to proc %d\n", ring_rtr_ptr->proc_id );
                                }else if( ring_rtr_ptr->seg_index != -1 ) {
                                    ret = Net_scast ( ring_rtr_ptr->seg_index, &Send_pack );
				    GlobalStatus.s_retrans++;

                                    Alarm( PROTOCOL, 
        "Answer_retrans: retransmit to seg %d\n", ring_rtr_ptr->seg_index );
                                }else{
#if 1
                                    ret = Net_queue_bcast ( &Send_pack );
#else
                                    ret = Net_bcast ( &Send_pack );
#endif
				    if( ret > 0 ) GlobalStatus.b_retrans++;

                                    Alarm( PROTOCOL, 
        "Answer_retrans: retransmit to all\n");
                                }
				if( ret > 0 )
				{
                                	num_retrans++;
				}
                        }else{
                                *proc_id = -1;
                                if( ring_rtr_ptr->seg_index != My.seg_index )
                                        *seg_index = -1;
                        }
                    }
                }else{
                    /* copy requests of other rings */
                    bytes_to_copy = sizeof(ring_rtr) + 
                                ring_rtr_ptr->num_seq * sizeof(int32);

		    if( new_ptr != old_ptr )
                    	memmove( &rtr[new_ptr], &rtr[old_ptr], bytes_to_copy);

                    old_ptr += bytes_to_copy;
                    new_ptr += bytes_to_copy;

                    Alarm( PROTOCOL, "Prot_handle_token: Coping foreign rtr\n");
                }
            }
        }
	*ret_new_ptr = new_ptr;

	ret = Net_flush_bcast();
	if( ret > 0 )
	{
		GlobalStatus.b_retrans++;
		num_retrans++;
	}
	return (num_retrans);
}

/* ### Pack: this routine has changed */
static	int	Send_new_packets( int num_allowed )
{
	packet_header	*pack_ptr;
	scatter	        *scat_ptr;
	int		pack_entry;
	int		num_sent;
	int		ret;

	num_sent = 0;
        while( num_sent < num_allowed )
        {
		/* check if down queue is empty */
		if( Down_queue_ptr->num_mess == 0 ) break;

		/* initialize packet_header */
                pack_ptr =  new(PACK_HEAD_OBJ);

		scat_ptr = Down_queue_ptr->first->mess;

                pack_ptr->type = Down_queue_ptr->first->type;
                pack_ptr->proc_id = My.id;
                pack_ptr->memb_id = Memb_id();
                pack_ptr->seq = Highest_seq+1;
                Highest_seq++;
                pack_ptr->fifo_seq = Highest_fifo_seq+1;
		Highest_fifo_seq++;
                pack_ptr->data_len = scat_ptr->elements[
				Down_queue_ptr->cur_element].len;

                Send_pack.elements[1].buf = scat_ptr->elements[
					Down_queue_ptr->cur_element].buf;

		Down_queue_ptr->cur_element++;
		if( Down_queue_ptr->cur_element < scat_ptr->num_elements )
		{
			/* not last packet in message */
			pack_ptr->packet_index = Down_queue_ptr->cur_element;
		}else if( Down_queue_ptr->cur_element == scat_ptr->num_elements ){
			down_link	*tmp_down;

			/* last packet in message */
			pack_ptr->packet_index = -scat_ptr->num_elements;

			tmp_down = Down_queue_ptr->first;
			Down_queue_ptr->first = Down_queue_ptr->first->next;
			Down_queue_ptr->cur_element = 0;
			Down_queue_ptr->num_mess--;
			dispose( tmp_down->mess );
			dispose( tmp_down );
			if( Down_queue_ptr->num_mess < WATER_MARK ) 
				Sess_unblock_users_level();
		}else{
			Alarm( EXIT, 
                          "Send_new_packets: error in packet index: %d %d\n",
			  Down_queue_ptr->cur_element,scat_ptr->num_elements );
		}

                Send_pack.elements[0].buf = (char *) pack_ptr;
                Send_pack.elements[1].len = pack_ptr->data_len;

#if 1
                ret = Net_queue_bcast( &Send_pack );
#else
                ret = Net_bcast( &Send_pack );
#endif
		if( ret > 0 )
		{
			num_sent++;
		}

                pack_entry = pack_ptr->seq & PACKET_MASK;
                if( Packets[pack_entry].exist ) 
                    Alarm( EXIT, 
                        "Send_new_packets: created packet %d already exist %d\n",
                        pack_ptr->seq, Packets[pack_entry].exist );

                /* insert new created packet */
                Packets[pack_entry].head       = pack_ptr;
                Packets[pack_entry].body       = (packet_body *)Send_pack.elements[1].buf;
                Packets[pack_entry].exist      = 1;
                Packets[pack_entry].proc_index = My_index;
                Alarm( PROTOCOL, 
			"Send_new_packets: packet %d sent and inserted \n",
			 pack_ptr->seq );
        }
	ret = Net_flush_bcast();
	if( ret > 0 )
	{
		num_sent++;
	}
	return ( num_sent );
}
 
static	void	Deliver_packet( int pack_entry, int to_copy )
{	
	int		proc_index;
	up_queue	*up_ptr;
	packet_header	*pack_ptr;
	message_link	*mess_link;
	int		index;

	pack_ptr = Packets[pack_entry].head;

	if( Is_reliable( pack_ptr->type ) &&
	    pack_ptr->packet_index == -1 ) 
	{
		/*
		 * for reliable single-packets message : deliver regardless
		 * of what is already in the queue.
		 */
		proc_index = MAX_PROCS_RING;
	}else{
		proc_index = Packets[pack_entry].proc_index;
	}

	up_ptr = &Up_queue[proc_index];

	if( up_ptr->exist == 0 )
	{ 
		/* no message for proc - need to create one */
		up_ptr->mess = new( SCATTER );
		up_ptr->mess->num_elements = 0;
		up_ptr->exist = 1;
	}

	/* validity check */
	index = pack_ptr->packet_index;
	if( index < 0 ) index = -index;
	if( up_ptr->mess->num_elements+1 != index )
	{
		Alarm( EXIT, "Deliver_packet: sequence error: sec is %d, should be %d\n",
			pack_ptr->packet_index,
			up_ptr->mess->num_elements+1 );
	}

	/* chain this packet */
	up_ptr->mess->num_elements++;
	up_ptr->mess->elements[index-1].len = Packets[pack_entry].head->data_len;
	up_ptr->mess->elements[index-1].buf = (char *)Packets[pack_entry].body;
	if( to_copy)
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

	if( pack_ptr->packet_index < 0 )
	{
		/* end of message */
		/* Push up big_scatter. i.e. up_ptr->mess */
		mess_link = new(MESSAGE_LINK);
		mess_link->mess = up_ptr->mess;
		up_ptr->exist = 0;
		Sess_deliver_message( mess_link );
	}
}

static	void	Deliver_reliable_packets( int32 start_seq, int num_packets )
{
	int		pack_entry;
	int		end_seq;
	int		i;

	if( Memb_state() == EVS ) return;
	end_seq = start_seq+num_packets-1;
	if( start_seq <= Last_delivered ) start_seq = Last_delivered + 1;
	for( i = start_seq; i <= end_seq  ; i++ )
	{
		pack_entry = i & PACKET_MASK;

		if( Packets[pack_entry].exist == 1 )
		{
			if( Is_reliable( Packets[pack_entry].head->type ) &&
			    Packets[pack_entry].head->packet_index == -1 )
			{
				Deliver_packet( pack_entry, 1 );
				Alarm( PROTOCOL, "Deliver_reliable_packets: packet %d was delivered\n", i );
			}
		}
	}
}

static	void	Deliver_agreed_packets()
{
	/* deliver all non-safe packets that are ordered and not delivered */

	int		pack_entry;
	int		i;

	if( My_aru <= Last_delivered ) return;
	if( Memb_state() == EVS ) return;

	for( i = Last_delivered+1; i <= My_aru; i++ )
	{
		pack_entry = i & PACKET_MASK;

		if( Packets[pack_entry].exist == 1 ) 
		{
			if( !Is_safe( Packets[pack_entry].head->type ) )
			{
				Deliver_packet( pack_entry, 1 );
				Alarm( PROTOCOL, "Deliver_agreed_packets: packet %d was delivered\n", i );
				Last_delivered++;
			}else return;
		}else if( Packets[pack_entry].exist == 2 ){
			/* This is possible only for reliable delivery prior to agreed */
			Last_delivered++;
		}else Alarm( EXIT, "Deliver_agreed_packets: Error, exist is %d\n", Packets[pack_entry].exist );
	}

}

void	Discard_packets()
{
	int		pack_entry;
	packet_body	*body_ptr;
	up_queue	*up_ptr;
	int		proc_index;
	int		i;

    if( Aru <= Last_discarded ) return;
    if( Memb_state() == EVS )
    {
	int		found_hole;
	membership_id	reg_memb_id;

	if( Aru != Last_seq ) return;

        /* Deliver packets that must be delivered before the transitional signal.
         * Those up to the Aru for my old ring were delivered in Read_form2().
         * So, it remains to deliver all packets up to the first hole or the first
         * SAFE message. */
        Alarmp( SPLOG_INFO, PROTOCOL,
                "Discard_packets: delivering messages after old ring Aru before transitional\n" );

        for( i = Last_discarded+1; i <= Highest_seq; i++ )
        {
            pack_entry = i & PACKET_MASK;
	    if( ! Packets[pack_entry].exist )
		Alarmp( SPLOG_FATAL, PROTOCOL, "Discard_packets: (EVS before transitional) packet %d not exist\n", i);
	    if( Packets[pack_entry].exist == 3 )
	    {
		Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Found first Hole in %d\n", i);
                break;
	    }
            if( Is_safe( Packets[pack_entry].head->type ) ) {
                Alarmp( SPLOG_INFO, PROTOCOL, "Discard_packets: Found first SAFE message in %d", i);
                break;
            }
            /* should deliver packet or dispose the body if it was delivered already */
            if( Packets[pack_entry].exist == 1 ){
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
	    if( ! Packets[pack_entry].exist )
		Alarm( EXIT, "Discard_packets: (EVS after transitional) packet %d not exist\n", i);
	    if( Packets[pack_entry].exist == 3 )
	    {
		/* 
		 * There is a hole! 
		 * from here, we need to check if the proc_id of the packet
		 * is in commited membership. 
		 */
		found_hole = 1;
		Alarm( PROTOCOL, "Discard_packets: Found a Hole in %d \n",i);

	    }else if( (!found_hole) || 
   (Conf_id_in_conf( &Commit_membership, Packets[pack_entry].head->proc_id ) != -1) ){
		/* should deliver packet or dispose the body if it was delivered already */
		if( Packets[pack_entry].exist == 1 ){
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

	/* check up_queue and down_queue */
	if( Down_queue_ptr->num_mess > 0 )
	{
		Down_queue_ptr->cur_element = 0;
	}

	for( proc_index=0; proc_index < MAX_PROCS_RING; proc_index++ )
	{
		if( Up_queue[proc_index].exist )
		{
			if( proc_index != My_index )
			{
			    /* 
			     * dispose only packets that are not mine 
			     * my packets will stay in Down_queue if the message is not
			     * ready to be delivered (because not fully sent yet) 
			     * so we need not to dispose them! 
			     */
			    up_ptr = &Up_queue[proc_index];
			    for( i=0; i < up_ptr->mess->num_elements; i++ )
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
        if( Conf_in_reload_singleton_state() ) {
                /* GOP state equals value 1, but is private declaration in groups.c */
                if ( (GlobalStatus.gstate != 1 ) || ( Conf_num_procs( &Reg_membership ) != 1 ) ) {
                        Alarmp( SPLOG_FATAL, MEMB, "Discard_packets: Failed to reload configuration - gstate: %d and num_procs in membership: %d\n", GlobalStatus.gstate, Conf_num_procs( &Reg_membership) );
                }
                Net_clear_partition();
                E_queue( Memb_lookup_new_members, 0, NULL, Zero_timeout);
                Conf_reload_singleton_state_end();
        }

	/* set variables for next membership */
        if ( Conf_debug_initial_sequence() ) {
            Last_token->aru	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Highest_seq 	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Highest_fifo_seq     = INITIAL_SEQUENCE_NEAR_WRAP;
            My_aru	    	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Aru		         = INITIAL_SEQUENCE_NEAR_WRAP;
            Set_aru		 = INITIAL_SEQUENCE_NEAR_WRAP -1;
            Last_discarded	 = INITIAL_SEQUENCE_NEAR_WRAP;
            Last_delivered	 = INITIAL_SEQUENCE_NEAR_WRAP;
        } else {
            Last_token->aru	 = 0;
            Highest_seq 	 = 0;
            Highest_fifo_seq     = 0;
            My_aru	    	 = 0;
            Aru		         = 0;
            Set_aru		 = -1;
            Last_discarded	 = 0;
            Last_delivered	 = 0;
        }

	GlobalStatus.my_aru	 = My_aru;
	Token_counter 	= 0;

    }else{

	for( i = Last_discarded+1; i <= Aru; i++ )
	{
	    pack_entry = i & PACKET_MASK;
	    if( ! Packets[pack_entry].exist )
		Alarm( EXIT, "Discard_packets: (NOT EVS) packet %d not exist\n",i);

	    /* should deliver packet or dispose the body if it was delivered already */
	    if( Packets[pack_entry].exist == 1 ) Deliver_packet( pack_entry, 0 );
	    else dispose( Packets[pack_entry].body );
	    /* dispose packet header in any case */
	    dispose( Packets[pack_entry].head );
	    Packets[pack_entry].exist = 0;
	}
	Alarm( PROTOCOL, "Discard_packets: packets %d-%d were discarded\n",
			Last_discarded+1, Aru );

	Last_discarded = Aru;
	if( Last_delivered < Last_discarded ) Last_delivered = Last_discarded;
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
static	int	Is_token_hold()
{
	if( ( Memb_state() == OP || 
	      ( Memb_state() == GATHER && Memb_token_alive() ) )&&
	    Get_retrans(Last_token->type) <= 1			&&
	    Aru == Highest_seq && Token_counter > 1 ) return ( 1 );
	else return( 0 );
}

static	int	To_hold_token()
{
	if( ( Memb_state() == OP || 
	      ( Memb_state() == GATHER && Memb_token_alive() ) )&&
	    Get_retrans(Last_token->type) <= 1			&&
	    Aru == Highest_seq && Token_counter > 1 ) return ( 1 );
	else return( 0 );
}

static	void	Handle_hurry( packet_header *pack_ptr )
{
	if( Conf_leader( Memb_active_ptr() ) == My.id &&
	    Is_token_hold() ) 
	{
	    if( Conf_id_in_conf( Memb_active_ptr(), pack_ptr->proc_id ) != -1 )
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

void	Prot_token_hurry()
{
	/* asked to send token again (almost lost) */
			
	sys_scatter	retrans_token;
	int32		val;

	retrans_token.num_elements = 1;
	retrans_token.elements[0].len = sizeof(token_header);
	retrans_token.elements[0].buf = (char *)Last_token;
	Last_token->rtr_len=0;
	if( Conf_leader( Memb_active_ptr() ) == My.id ) 
	{
		val = Get_retrans(Last_token->type);
		val++;
		Last_token->type = Set_retrans( Last_token->type, val );
		E_queue( Prot_token_hurry, 0, NULL, Hurry_timeout );
		GlobalStatus.token_hurry++;
	}
	/* sending token */
	Net_send_token( &retrans_token );
	if( Wide_network && 
	    Conf_seg_last(Memb_active_ptr(), My.seg_index) == My.id )
	{
		/* sending again to another segment */
		Net_send_token( &retrans_token );
	}
	if( Get_retrans( Last_token->type ) > 1 )
	{
		/* problems */ 
		Net_send_token( &retrans_token );
		Net_send_token( &retrans_token );
	}

	Alarm( PROTOCOL, "Prot_token_hurry: retransmiting token %d %d\n",
		Get_arq(Last_token->type), Get_retrans(Last_token->type) );
}

void	Flip_token_body( char *buf, token_header *token_ptr )
{
/*
 * This routine can not be called twice for the same buffer because
 * of ring_rtr_ptr->num_seq.
 */
	ring_rtr	*ring_rtr_ptr;
	int32		*req_seq;
	char		*rtr;
	int		ptr;
	int		i;

	if( token_ptr->rtr_len <= 0 ) return;

	rtr = buf;
	ptr = 0;
	while( ptr < token_ptr->rtr_len )
	{
	    ring_rtr_ptr = (ring_rtr *)&rtr[ptr];

	    ring_rtr_ptr->memb_id.proc_id = Flip_int32( ring_rtr_ptr->memb_id.proc_id );
	    ring_rtr_ptr->memb_id.time    = Flip_int32( ring_rtr_ptr->memb_id.time );
	    ring_rtr_ptr->proc_id	  = Flip_int32( ring_rtr_ptr->proc_id );
	    ring_rtr_ptr->seg_index	  = Flip_int16( ring_rtr_ptr->seg_index );
	    ring_rtr_ptr->num_seq	  = Flip_int16( ring_rtr_ptr->num_seq );

	    ptr += sizeof(ring_rtr);
	    for( i=0; i < ring_rtr_ptr->num_seq; i++ )
	    {
		req_seq = (int32 *)&rtr[ptr];
		*req_seq = Flip_int32( *req_seq );
		ptr += sizeof(int32);
	    }	
	}
}
