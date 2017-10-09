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


#include <assert.h>
#include "arch.h"
#include "spread_params.h"
#include "network.h"
#include "net_types.h"
#include "spu_data_link.h"
#include "spu_events.h"
#include "status.h"
#include "spu_alarm.h"
#include "configuration.h"

/* for Memb_print_form_token() */
#include "membership.h"

static	channel		Bcast_channel[MAX_INTERFACES_PROC];
static  channel		Token_channel[MAX_INTERFACES_PROC];
static	channel		Send_channel;

static  int             Num_bcast_channels;
static  int             Num_token_channels;

static	int		Bcast_needed;
static	int32		Bcast_address;
static	int16		Bcast_port;

static	int		Num_send_needed;
static  int32		Send_address[MAX_SEGMENTS];
static	int16		Send_ports[MAX_SEGMENTS];

/* address for token sending - which is always needed */
static	int32		Token_address;
static	int16		Token_port;

static	configuration	Net_membership;
static	int		Segment_leader;

static 	configuration	*Cn;
static	proc		My;

static	int16		Partition[MAX_PROCS_RING];
static	sp_time		Partition_timeout 	= { 60, 0};
static	int		Partition_my_index;

static	void		Clear_partition_cb(int dummy, void *dummy_p);
static	int		In_my_component( int32	proc_id );
static	void		Flip_pack( packet_header *pack_ptr );
static	void		Flip_token( token_header *token_ptr );


void	Net_init()
{
	proc		dummy_proc;
        int32u          interface_addr;
        int             i;
        bool            bcast_bound = FALSE;

	Cn = Conf_ref();
	My = Conf_my();

	Partition_my_index = Conf_proc_by_id( My.id, &dummy_proc );
	Net_clear_partition();

	if( Cn->segments[My.seg_index].num_procs > 1 )
	{
		/* I am not allone in segment */
		Bcast_needed  = 1;
		Bcast_address = Cn->segments[My.seg_index].bcast_address;
		Bcast_port    = My.port;

		Alarm( NETWORK, "Net_init: Bcast needed to address (%d, %d)\n",
			Bcast_address, Bcast_port );
	}else{
		Bcast_needed  = 0;
		Bcast_address = 0;
		Alarm( NETWORK, "Net_init: Bcast is not needed\n" );
	}

        /* To receive broadcast (and possibly multicast) packets on a socket
         * bound to a specific interface, we also have to bind to the broadcast
         * address on the interface as well as the unicast interface. That is 
         * what the double bind of the bcast_address does.
         * 
         * The above statement is not true for Windows -- there binding to 
         * the broadcast address is forbidden. So we do not compile in the
         * bind call on windows.
         */
        for ( i=0; i < My.num_if; i++)
        {
                if (Is_IfType_Daemon( My.ifc[i].type ) || Is_IfType_Any( My.ifc[i].type ) )
                {
                        if (Is_IfType_Any( My.ifc[i].type ) )
                                interface_addr = 0;
                        else {
                                interface_addr = My.ifc[i].ip;
                                if (Bcast_needed && !bcast_bound) {
#ifndef ARCH_PC_WIN95
                                    Bcast_channel[Num_bcast_channels++] = DL_init_channel( RECV_CHANNEL | NO_LOOP, My.port, Bcast_address, Bcast_address );
#endif
                                    bcast_bound = TRUE;
                                }
                        }
                        Bcast_channel[Num_bcast_channels++] = DL_init_channel( RECV_CHANNEL | DL_BIND_ALL | NO_LOOP, My.port, Bcast_address, interface_addr );
                        Token_channel[Num_token_channels++] = DL_init_channel( RECV_CHANNEL, My.port+1, 0, interface_addr );
                }
        }

	Send_channel  = DL_init_channel( SEND_CHANNEL, My.port+2, 0, My.id );

	Num_send_needed = 0;
}
/* Called from above when configuration file is reloaded (potentially with changes to spread configuration
 * Needs to update any static-scope variables that depend on current configuration
 */
void    Net_signal_conf_reload(void)
{
	proc		dummy_proc;

        Partition_my_index = Conf_proc_by_id( My.id, &dummy_proc );

        Cn = Conf_ref();
        My = Conf_my();
}

void	Net_set_membership( configuration memb )
{
	int	i;
	int	my_index_in_seg;
	int	my_next_index;
	segment my_seg;

	Net_membership = memb;
	my_seg = Net_membership.segments[My.seg_index];
	my_index_in_seg = Conf_id_in_seg( &my_seg, My.id );
	if( my_index_in_seg < 0 ) {
		Conf_print( &Net_membership );
		Alarm( EXIT, "Net_set_membership: I am not in membership\n" );
	}
	else if( my_index_in_seg == 0) {
		Segment_leader = 1;
		Alarm( NETWORK,"Net_set_membership: I am a Segment leader\n");
	} else  Segment_leader = 0;

	Num_send_needed = 0;
	my_next_index = -1;
	for( i=0; i < Conf_num_segments( Cn ); i++ )
	{
	    if( i == My.seg_index )
	    {
		/* 
		 * This is my segment.
		 * There is no need to send to it
		 * but I need my_next_index to calculate
		 * the token send address.
		 */
		my_next_index = Num_send_needed;

	    } else if( Net_membership.segments[i].num_procs > 0 ) {

		Send_address[Num_send_needed] = Net_membership.segments[i].procs[0]->id;
		Send_ports  [Num_send_needed] = Net_membership.segments[i].port;

		Num_send_needed++;
	    }
	}
	assert(my_next_index != -1);
	for( i=0; i < Num_send_needed; i++ )
		Alarm( NETWORK, 
			"Net_set_membership: Send_addr[%d] is (%u.%u.%u.%u:%d)\n",
                       i, IP1(Send_address[i]), IP2(Send_address[i]), IP3(Send_address[i]), IP4(Send_address[i]), Send_ports[i] );

	/* Calculate where to send the token */
	Token_address = 0;
	if( my_index_in_seg < my_seg.num_procs-1 )
	{
		Token_address = my_seg.procs[my_index_in_seg+1]->id;
		Token_port    = my_seg.port+1;
	}else{
		/* I am last in my segment */
		if( Num_send_needed == 0 )
		{
			/* 
			 * My segment is the only segment
			 * sending token to the first in my segment 
			 */
			Token_address = my_seg.procs[0]->id;
			Token_port    = my_seg.port+1;
		} else if( Num_send_needed == my_next_index ) {
			/* 
			 * My segment is the last segment
			 * sending token to the first in first valid segment
			 */	
			Token_address = Send_address[0];
			Token_port    = Send_ports[0]+1;
		} else {
			/*
			 * There is a valid segment after mine
			 * sending token to the first in next valid segment
			 */
			Token_address = Send_address[my_next_index];
			Token_port    = Send_ports[my_next_index]+1;
		}

	}
	Alarm( NETWORK, "Net_set_membership: Token_address : (%u.%u.%u.%u:%d)\n",
               IP1(Token_address), IP2(Token_address), IP3(Token_address), IP4(Token_address), Token_port );
}

int	Net_bcast( sys_scatter *scat )
{
	packet_header	*pack_ptr;
	int 		i;
	int		ret;

	ret = 0;
	/* routing on channels if needed according to membership */
	pack_ptr = (packet_header *)scat->elements[0].buf;
	pack_ptr->type  = Set_routed( pack_ptr->type );
	pack_ptr->type  = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
	for ( i=0; i< Num_send_needed; i++ )
	{
	    ret = DL_send( Send_channel, Send_address[i], Send_ports[i], scat );
	}
	pack_ptr->type = Clear_routed( pack_ptr->type );

	/* broadcasting if needed according to configuration */
	if( Bcast_needed )
	{	
	    ret = DL_send( Send_channel, Bcast_address, Bcast_port, scat );
	}

        if( !Bcast_needed && (Num_send_needed == 0) )
            ret = 1; /* No actual send is needed, but 'packet' can be considered 'sent' */

	return( ret );
}

int	Net_scast( int16 seg_index, sys_scatter *scat )
{
	packet_header	*pack_ptr;
	int		ret;
        bool            send_not_needed_p = FALSE;

	ret = 0;
	pack_ptr = (packet_header *)scat->elements[0].buf;
	pack_ptr->type = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
	if( seg_index == My.seg_index )
	{
	    if( Bcast_needed )
	    {
	    	ret = DL_send( Send_channel, Bcast_address, Bcast_port, scat );
	    } else 
                send_not_needed_p = TRUE;
	}else{
	    if( Net_membership.segments[seg_index].num_procs > 0 )
	    {
		pack_ptr->type = Set_routed( pack_ptr->type );
	    	ret = DL_send( Send_channel, 
			Net_membership.segments[seg_index].procs[0]->id,
			Net_membership.segments[seg_index].port,
			scat );
		pack_ptr->type = Clear_routed( pack_ptr->type );
	    } else
                send_not_needed_p = TRUE;
	}

        if (send_not_needed_p)
            ret = 1; /* notify that packet can be considered sent, even though no network send actually needed */

	return( ret );
}

int	Net_ucast( int32 proc_id, sys_scatter *scat )
{
	packet_header	*pack_ptr;
	proc		p;
	int		ret;

	pack_ptr = (packet_header *)scat->elements[0].buf;
	pack_ptr->type = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
	ret = Conf_proc_by_id( proc_id, &p );
	if( ret < 0 )
	{
		Alarm( PRINT, "Net_ucast: non existing proc_id %d\n",proc_id );
		return( ret );
	}
	ret = DL_send( Send_channel, proc_id, p.port, scat );
	return( ret );
}

int	Net_recv ( channel fd, sys_scatter *scat )
{
static	scatter		save;
	packet_header	*pack_ptr;
	int		bytes_left;
	int		received_bytes;
	int		i;
        bool            ch_found;

	pack_ptr = (packet_header *)scat->elements[0].buf;

        ch_found = FALSE;
        for (i = 0 ; i < Num_bcast_channels; i++) {
            if ( fd == Bcast_channel[i]) {
                ch_found = TRUE; 
                break;
            }
        }
        if (ch_found == FALSE) {
            Alarm(EXIT, "Net_recv: Listening and received packet on un-used interface %d\n", fd);
        }

	received_bytes = DL_recv( fd, scat ); 

	if( received_bytes <= 0 ) return( received_bytes );

	if( received_bytes < sizeof( packet_header ) ) 
	{
		Alarm(PRINT, "Net_recv: ignoring packet of size %d, smaller than packet header size %d\n", 
			received_bytes, sizeof(packet_header) );
		return( -1 );
	}

	/* Flipping packet header to my form if needed */
	if( !Same_endian( pack_ptr->type ) ) Flip_pack( pack_ptr );

        /* First reject any message whose daemon has a different configuration */
        if ( (pack_ptr->conf_hash != Cn->hash_code) && (pack_ptr->conf_hash != MONITOR_HASH) ){
            Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: Received message (pkthdr_len = %u) from host %d.%d.%d.%d with different spread configuration file (hash %u != local hash %u)\n", 
                    scat->elements[0].len,
                    IP1(pack_ptr->proc_id),
                    IP2(pack_ptr->proc_id),
                    IP3(pack_ptr->proc_id),
                    IP4(pack_ptr->proc_id),
                    pack_ptr->conf_hash, 
                    Cn->hash_code);
            return( 0 );
        }

	if( Is_partition( pack_ptr->type ) )
	{
		/* Monitor : updating partition */
		int16	*cur_partition;

                if ( ! Conf_get_dangerous_monitor_state() ) {
                        Alarm( PRINT, "Net_recv: Request to set partition or kill daemons from (%d.%d.%d.%d) denied. Monitor in safe mode\n", IP1(pack_ptr->proc_id), IP2(pack_ptr->proc_id), IP3(pack_ptr->proc_id), IP4(pack_ptr->proc_id) );
                        return( 0 );
                }

		if( ! ( pack_ptr->memb_id.proc_id == 15051963 && Conf_id_in_conf( Cn, pack_ptr->proc_id ) != -1  ) ) return( 0 );

		cur_partition = (int16 *)scat->elements[1].buf;

		for( i=0; i < Conf_num_procs( Cn ); i++ )
		    if( ! Same_endian( pack_ptr->type ) ) 
                            cur_partition[i] = Flip_int16( cur_partition[i] );

                Net_set_partition(cur_partition);

		E_queue( Clear_partition_cb, 0, NULL, Partition_timeout );
		if( Partition[Partition_my_index] == -1 )
		    Alarm( EXIT, "Net_recv: Instructed to exit by monitor\n");
		Alarm( PRINT  , "Net_recv: Got monitor message, component %d\n", 
			Partition[Partition_my_index] );

		return( 0 );
	} 

	/* Monitor : drop packet from daemon in different monitor-caused partition */
	if( ! ( pack_ptr->memb_id.proc_id == 15051963 || In_my_component( pack_ptr->transmiter_id ) ) )
		return( 0 );

	/* no need to return my own packets */
	if( pack_ptr->transmiter_id == My.id )
		return( 0 );

        /* packet validity check */
	if( received_bytes != sizeof( packet_header ) + pack_ptr->data_len) {
                Alarm( PRINT, "Net_recv: Received invalid packet - received bytes (%d) != expected length (%d)\n", 
                       received_bytes, sizeof( packet_header ) + pack_ptr->data_len );
                return( -1 );
        }

	if( Bcast_needed && Is_routed( pack_ptr->type ) )
	{
		if( !Segment_leader ) Alarm( NETWORK, 
		"Net_recv: recv routed message from %d but not seg leader\n",
			pack_ptr->proc_id);

		/* saving scat lens for another DL_recv */
		save.num_elements = scat->num_elements;
		for( i=0; i < (int) save.num_elements; i++ )
			save.elements[i].len = scat->elements[i].len;

		/* computing true scat lens for sending */
		bytes_left = received_bytes;
		i = 0;
		while ( bytes_left > 0 )
		{
			if( bytes_left < (int) scat->elements[i].len )
				scat->elements[i].len = bytes_left;
			bytes_left -=  scat->elements[i].len;			
			i ++;
		}
		scat->num_elements = i;

		pack_ptr->type = Clear_routed ( pack_ptr->type );
		pack_ptr->transmiter_id = My.id;

		/* fliping to original form */
		if( !Same_endian( pack_ptr->type ) ) Flip_pack( pack_ptr );
		DL_send( Send_channel, Bcast_address, Bcast_port, scat );
		/* re-fliping to my form */
		if( !Same_endian( pack_ptr->type ) ) Flip_pack( pack_ptr );

		/* restoring scat lens for another DL_recv */
		scat->num_elements = save.num_elements;
		for( i=0; i < (int) save.num_elements; i++ )
			scat->elements[i].len = save.elements[i].len;

	}
	/*
	 * we clear routed anyway in order not to ask if Bcast_needed again.
	 * This way, if bcast is not needed we give it to the upper layer
	 * right away. It will always get to the upper layer with this
	 * bit cleared.
	 */
	pack_ptr->type = Clear_routed ( pack_ptr->type );

	return( received_bytes );
}

int	Net_send_token( sys_scatter *scat )
{
	token_header *token_ptr = (token_header *)scat->elements[0].buf;
        int           send_len  = 0;
	int	      ret;
        int           i;

        for ( i = 0; i < ( int ) scat->num_elements; ++i )
                send_len += scat->elements[i].len;

        if ( send_len != (int) sizeof( token_header ) + token_ptr->rtr_len )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_send_token: Wrong size token %d (send_len) != %d (expected)!\n",                        
                        send_len, (int) sizeof( token_header) + token_ptr->rtr_len );
        }
        else if ( send_len > (int) ( sizeof( token_header) + sizeof( token_body ) ) )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_send_token: Token too long (%d > %d bytes)!\n",
                        send_len, (int) ( sizeof( token_header) + sizeof( token_body ) ) );
        }
        else if ( send_len > MAX_PACKET_SIZE )
        {
                Alarmp( SPLOG_WARNING, PRINT, "Net_send_token: WARNING!!! Token is longer (%d bytes) than a single MTU (%d bytes)! "
                        "IP fragmentation will occur and greatly increase the chance the token is lost!\n",
                        send_len, MAX_PACKET_SIZE );
        }

	token_ptr->type          = Set_endian( token_ptr->type );
        token_ptr->conf_hash     = Cn->hash_code;
	token_ptr->transmiter_id = My.id;

	Alarmp( SPLOG_INFO, NETWORK, 
		"Net_send_token: type = 0x%08X; transmitter_id = 0x%08X; seq = %d; proc_id = 0x%08X; "
                "aru = %d; aru_last_id = 0x%08X; Token_address = 0x%08X; send_len = %d\n", 
		token_ptr->type, token_ptr->transmiter_id, token_ptr->seq, token_ptr->proc_id, 
                token_ptr->aru, token_ptr->aru_last_id, Token_address, send_len );

	ret = DL_send( Send_channel, Token_address, Token_port, scat );
	return ( ret );
}

int	Net_recv_token( channel fd, sys_scatter *scat )
{
	token_header	*token_ptr;
	int		ret, i;
        bool            ch_found;

	token_ptr = (token_header *)scat->elements[0].buf;

        ch_found = FALSE;
        for (i = 0 ; i < Num_token_channels; i++) {
            if ( fd == Token_channel[i]) {
                ch_found = TRUE; 
                break;
            }
        }
        if (ch_found == FALSE) {
            Alarm(EXIT, "Net_recv: Listening and received packet on un-used interface %d\n", fd);
        }

	ret = DL_recv( fd, scat );

	if( ret <= 0 ) return( ret );

	/* Fliping token header to my form if needed */
	if( !Same_endian( token_ptr->type ) ) Flip_token( token_ptr );

        /* First reject any token whose daemon has a different configuration */
        if (token_ptr->conf_hash != Cn->hash_code) {
            Alarmp( SPLOG_WARNING, NETWORK, "Net_recv_token: Received token from host %d.%d.%d.%d with different spread configuration file (hash %u != local hash %u)\n", 
                    IP1(token_ptr->proc_id),
                    IP2(token_ptr->proc_id),
                    IP3(token_ptr->proc_id),
                    IP4(token_ptr->proc_id),
                    token_ptr->conf_hash, 
                    Cn->hash_code);
            return( 0 );
        }

	/* Monitor : drop token from daemon in different monitor-caused partition */
	if( !In_my_component( token_ptr->transmiter_id ) )
		return( 0 );

	return ( ret );
}

int	Net_ucast_token( int32 proc_id, sys_scatter *scat )
{
	token_header *token_ptr = ( token_header * ) scat->elements[0].buf;
        int           send_len  = 0;
	proc	      p;
	int	      ret;
        int           i;

        for ( i = 0; i < ( int ) scat->num_elements; ++i )
                send_len += scat->elements[i].len;

        if ( send_len != (int) sizeof( token_header ) + token_ptr->rtr_len )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_ucast_token: Wrong size token %d (send_len) != %d (expected)!\n",                        
                        send_len, (int) sizeof( token_header) + token_ptr->rtr_len );
        }
        else if ( send_len > (int) ( sizeof( token_header) + sizeof( token_body ) ) )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_ucast_token: Token too long (%d > %d bytes)!\n",
                        send_len, (int) ( sizeof( token_header) + sizeof( token_body ) ) );
        }
        else if ( send_len > MAX_PACKET_SIZE )
        {
                Alarmp( SPLOG_WARNING, PRINT, "Net_ucast_token: WARNING!!! Token is longer (%d bytes) than a single MTU (%d bytes)! "
                        "IP fragmentation will occur and greatly increase the chance the token is lost!\n",
                        send_len, MAX_PACKET_SIZE );
        }

	token_ptr->type = Set_endian( token_ptr->type );
        token_ptr->conf_hash = Cn->hash_code;
	token_ptr->transmiter_id = My.id;

	ret = Conf_proc_by_id( proc_id, &p );

	if( ret < 0 )
	{
		Alarm( PRINT, "Net_ucast_token: non existing proc_id %d\n",
			proc_id );
		return( ret );
	}

	Alarmp( SPLOG_INFO, NETWORK, 
		"Net_ucast_token(" IPF "): type = 0x%08X; transmitter_id = 0x%08X; seq = %d; proc_id = 0x%08X; "
                "aru = %d; aru_last_id = 0x%08X; Token_address = 0x%08X; send_len = %d\n", 
		IP( proc_id ), token_ptr->type, token_ptr->transmiter_id, token_ptr->seq, token_ptr->proc_id, 
                token_ptr->aru, token_ptr->aru_last_id, Token_address, send_len );

	ret = DL_send( Send_channel, proc_id, p.port+1, scat );
	return( ret );
}

void     Net_num_channels(int *num_bcast, int *num_token)
{
    *num_bcast = Num_bcast_channels;
    *num_token = Num_token_channels;
}

channel *Net_bcast_channel()
{
	return( &(Bcast_channel[0]) );
}

channel *Net_token_channel()
{
	return( &(Token_channel[0]) );
}

void    Net_set_partition(int16 *new_partition)
{
        int     i;

        if ( Conf_in_reload_singleton_state() ) {
                Alarmp(SPLOG_DEBUG, NETWORK, "Net_set_partition: Can not change partition since daemon configuration change in progress\n");
                return;
        }

        for( i=0; i < Conf_num_procs( Cn ); i++ )
                Partition[i] = new_partition[i];
}

void    Net_clear_partition(void)
{
	int	i;

	for( i=0; i < Conf_num_procs( Cn ); i++ )
		Partition[i] = 0;
}

static	void	Clear_partition_cb(int dummy, void *dummy_p)
{
        Net_clear_partition();
}

static	int	In_my_component( int32	proc_id )
{
	int	proc_index;
	proc	dummy_proc;
	char	ip[16];

	proc_index = Conf_proc_by_id( proc_id, &dummy_proc );
	if( proc_index < 0 )
	{
		Conf_id_to_str( proc_id, ip );
		Alarm( PRINT, "In_my_component: unknown proc %s\n", ip );
		return( 0 );
	}

	return( Partition[Partition_my_index] == Partition[proc_index] );
}

/* The first fragment header is not flipped here, even though it is
   part of the packet header. It will be flipped with the other
   fragment headers in Prot_handle_bcast */
void	Flip_pack( packet_header *pack_ptr )
{
	pack_ptr->type		  = Flip_int32( pack_ptr->type );
	pack_ptr->transmiter_id	  = Flip_int32( pack_ptr->transmiter_id );
	pack_ptr->proc_id	  = Flip_int32( pack_ptr->proc_id );
	pack_ptr->memb_id.proc_id = Flip_int32( pack_ptr->memb_id.proc_id );
	pack_ptr->memb_id.time	  = Flip_int32( pack_ptr->memb_id.time );
	pack_ptr->seq		  = Flip_int32( pack_ptr->seq );
	pack_ptr->token_round	  = Flip_int32( pack_ptr->token_round ); /* fifo_seq changed to token_round */
	pack_ptr->conf_hash	  = Flip_int32( pack_ptr->conf_hash );
	pack_ptr->data_len	  = Flip_int16( pack_ptr->data_len );
}

void	Flip_token( token_header *token_ptr )
{
	token_ptr->type		   = Flip_int32( token_ptr->type );
	token_ptr->transmiter_id   = Flip_int32( token_ptr->transmiter_id );
	token_ptr->proc_id	   = Flip_int32( token_ptr->proc_id );
	token_ptr->memb_id.proc_id = Flip_int32( token_ptr->memb_id.proc_id );
	token_ptr->memb_id.time	   = Flip_int32( token_ptr->memb_id.time );
	token_ptr->seq		   = Flip_int32( token_ptr->seq );
	token_ptr->aru		   = Flip_int32( token_ptr->aru );
	token_ptr->aru_last_id	   = Flip_int32( token_ptr->aru_last_id );
	token_ptr->flow_control	   = Flip_int16( token_ptr->flow_control );
	token_ptr->rtr_len	   = Flip_int16( token_ptr->rtr_len );
        token_ptr->conf_hash       = Flip_int32( token_ptr->conf_hash );
}
