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


#ifndef	INC_NET_TYPES
#define	INC_NET_TYPES

#include "arch.h"       /* For int32, etc */
#include "spu_data_link.h"  /* For MAX_PACKET_SIZE */

/*	Dont forget that 0x80000080 is kept for endians */

#define		UNRELIABLE_TYPE		0x00000001
#define		RELIABLE_TYPE		0x00000002
#define		FIFO_TYPE		0x00000004
#define		AGREED_TYPE		0x00000008
#define		SAFE_TYPE		0x00000010
#define		REGULAR_TYPE		0x0000001f

#define		ROUTED_TYPE		0x00000020

#define		HURRY_TYPE		0x00000040

#define		ALIVE_TYPE		0x00000100
#define		JOIN_TYPE		0x00000200
#define		REFER_TYPE		0x00000400
#define		MEMBERSHIP_TYPE		0x00000700

#define		FORM1_TYPE		0x00001000
#define		FORM2_TYPE		0x00002000
#define		FORM_TYPE		0x00003000

#define		ARQ_TYPE		0x000f0000
#define	        RETRANS_TYPE		0x00f00000

#define		STATUS_TYPE		0x01000000
#define		PARTITION_TYPE		0x02000000
#define		FC_TYPE			0x04000000
#define		RELOAD_TYPE		0x08000000
#define		CONTROL_TYPE		0x0f000000


#define		Is_unreliable( type )	( type &  UNRELIABLE_TYPE )
#define		Is_reliable( type )	( type &  RELIABLE_TYPE   )
#define		Is_fifo( type )		( type &  FIFO_TYPE       )
#define		Is_agreed( type )	( type &  AGREED_TYPE     )
#define		Is_safe( type )		( type &  SAFE_TYPE       )
#define		Is_regular( type )	( type &  REGULAR_TYPE    )

#define		Is_routed( type )	( type &  ROUTED_TYPE     )
#define		Set_routed( type )	( type |  ROUTED_TYPE     )
#define		Clear_routed( type )	( type & ~ROUTED_TYPE     )

#define		Is_hurry( type )	( type &  HURRY_TYPE      )

#define		Is_alive( type )	( type &  ALIVE_TYPE      )
#define		Is_join( type )		( type &  JOIN_TYPE       )
#define		Is_refer( type )	( type &  REFER_TYPE      )
#define		Is_membership( type )	( type &  MEMBERSHIP_TYPE )

#define		Is_form( type )		( type &  FORM_TYPE	  )
#define		Is_form1( type )	( type &  FORM1_TYPE	  )
#define		Is_form2( type )	( type &  FORM2_TYPE	  )

#define		Get_arq( type )		( (type &  ARQ_TYPE) >> 16)
#define		Set_arq( type, val )	( (type & ~ARQ_TYPE) | ((val << 16)&ARQ_TYPE) )
#define		Get_retrans( type )	( (type &  RETRANS_TYPE) >> 20)
#define		Set_retrans( type, val) ( (type & ~RETRANS_TYPE) | ((val << 20)&RETRANS_TYPE) )

#define		Is_status( type )	( type &  STATUS_TYPE     )
#define		Is_partition( type )	( type &  PARTITION_TYPE  )
#define		Is_fc( type )		( type &  FC_TYPE         )
#define		Is_conf_reload( type )	( type &  RELOAD_TYPE     )
#define		Is_control( type )	( type &  CONTROL_TYPE    )


#define MONITOR_HASH    1100    /* Conf_hash code for packets from spmonitor program */

typedef	struct	dummy_packet_header {
	int32		type;
	int32		transmiter_id;
	int32		proc_id;
	membership_id	memb_id;
	int32		seq;
	int32		fifo_seq;
	int16		packet_index;
	int16		data_len;
        int32           conf_hash;
} packet_header;

typedef	char       packet_body[MAX_PACKET_SIZE-sizeof(packet_header)];

typedef	struct	dummy_token_header {
	int32		type;
	int32		transmiter_id;
	int32		seq;
	int32		proc_id;
	int32		aru;
        int32           aru_last_id;
	int16		flow_control;
	int16		rtr_len;
        int32           conf_hash;
} token_header;

typedef	char       token_body[MAX_PACKET_SIZE-sizeof(token_header)];

typedef	struct	dummy_ring_rtr {
	membership_id	memb_id;
	int32		proc_id;
	int16		seg_index;
	int16		num_seq;
} ring_rtr;

#endif	/* INC_NET_TYPES */
