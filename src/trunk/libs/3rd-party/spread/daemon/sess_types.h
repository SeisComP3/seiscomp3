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


#ifndef	INC_SESS_TYPES
#define	INC_SESS_TYPES

#include "spread_params.h"      /* For MAX_GROUP_NAME */
#include "spu_scatter.h"            /* For MAX_SCATTER_ELEMENTS */
#include "spu_data_link.h"          /* For MAX_PACKET_SIZE */

#define         MAX_MESSAGE_BODY_LEN    (MAX_SCATTER_ELEMENTS * (MAX_PACKET_SIZE - 32)) /* 32 is sizeof(packet_header) */

/*      Dont forget that 0x80000080 is kept for endians */

#define		service			int

#define		UNRELIABLE_MESS		0x00000001
#define		RELIABLE_MESS		0x00000002
#define		FIFO_MESS		0x00000004
#define		CAUSAL_MESS		0x00000008
#define		AGREED_MESS		0x00000010
#define		SAFE_MESS		0x00000020
#define		REGULAR_MESS		0x0000003f

#define         NOT_REGULAR_MESS        0x00ffff00

#define		SELF_DISCARD		0x00000040 /* allow the sender to hint
						      whether it wants the message
						      back or not */
#define         DROP_RECV               0x01000000 /* if set on recv, then drop data
                                                      that doesn't fit in buffers. 
                                                      Else, return error from recv and
                                                      wait for big enough buffer */

#define         REG_MEMB_MESS           0x00001000 /* between spread and the user */
#define         TRANSITION_MESS         0x00002000 
#define		CAUSED_BY_JOIN		0x00000100 /* added to reg_memb_mess */
#define		CAUSED_BY_LEAVE		0x00000200 /* added to reg_memb_mess or when this process left group*/
#define		CAUSED_BY_DISCONNECT	0x00000400 /* added to reg_memb_mess */
#define		CAUSED_BY_NETWORK	0x00000800 /* added to reg_memb_mess */
#define         MEMBERSHIP_MESS         0x00003f00

#define         RETRANSMIT_TYPE         0x00004000 /* set on msg_frag_obj which are 
                                                    * retransmits--not seen at sess layer
                                                    */
#define         LATENCY_MESS            0x00008000 /* Used for latency debugging messages */

#define		JOIN_MESS		0x00010000
#define		LEAVE_MESS		0x00020000
#define		KILL_MESS		0x00040000
#define		GROUPS_MESS		0x00080000

#define         DUMMY_MESS              0x00100000 /* EVS: Message which we will never get; placeholder */
#define         STATETRANS_MESS         0x00200000 /* EVS: StateTrans message, deliver only locally to Memb.c */
#define         REJECT_MESS             0x00400000 /* Message was rejected by auth module. */

#define		Is_unreliable_mess( type )		( type &  UNRELIABLE_MESS      )
#define		Is_reliable_mess( type )		( type &  RELIABLE_MESS        )
#define		Is_fifo_mess( type )			( type &  FIFO_MESS            )
#define		Is_causal_mess( type )			( type &  CAUSAL_MESS          )
#define		Is_agreed_mess( type )			( type &  AGREED_MESS          )
#define		Is_safe_mess( type )			( type &  SAFE_MESS            )
#define		Is_regular_mess( type )			( type &  REGULAR_MESS         )

#define         Is_only_regular_mess(type)      (((type) & REGULAR_MESS)&&(!((type) & NOT_REGULAR_MESS)))

#define		Is_self_discard( type )			( type &  SELF_DISCARD         )
#define		Is_drop_recv( type )			( type &  DROP_RECV            )

#define         Is_reg_memb_mess( type )		( type &  REG_MEMB_MESS        )
#define         Is_transition_mess( type )     		( type &  TRANSITION_MESS      )
#define         Is_caused_join_mess( type )     	( type &  CAUSED_BY_JOIN       )
#define         Is_caused_leave_mess( type )    	( type &  CAUSED_BY_LEAVE      )
#define         Is_caused_disconnect_mess( type )	( type &  CAUSED_BY_DISCONNECT )
#define         Is_caused_network_mess( type )		( type &  CAUSED_BY_NETWORK    )
#define         Is_membership_mess( type )      	( type &  MEMBERSHIP_MESS      )

#define		Is_join_mess( type )			( type &  JOIN_MESS            )
#define		Is_leave_mess( type )			( type &  LEAVE_MESS           )
#define		Is_kill_mess( type )			( type &  KILL_MESS            )
#define		Is_groups_mess( type )			( type &  GROUPS_MESS          )

#define		Is_Retransmit( type )	        ( (type) &  RETRANSMIT_TYPE     )
#define		Set_Retransmit( type )	        ( (type) |  RETRANSMIT_TYPE     )
#define		Clear_Retransmit( type )	( (type) & ~RETRANSMIT_TYPE     )

#define         Is_latency_mess( type )                 ( (type) & LATENCY_MESS        )
#define         Is_dummy_mess( type )                   ( (type) & DUMMY_MESS          )
#define         Is_statetrans_mess( type )              ( (type) & STATETRANS_MESS     )
#define         Is_reject_mess( type )                  ( (type) & REJECT_MESS         )

#define		ACCEPT_SESSION		 1
#define		ILLEGAL_SPREAD		-1
#define		COULD_NOT_CONNECT	-2
#define		REJECT_QUOTA		-3
#define		REJECT_NO_NAME		-4
#define		REJECT_ILLEGAL_NAME	-5
#define		REJECT_NOT_UNIQUE	-6
#define		REJECT_VERSION		-7
#define		CONNECTION_CLOSED	-8
#define         REJECT_AUTH             -9

#define		ILLEGAL_SESSION		-11
#define		ILLEGAL_SERVICE		-12
#define		ILLEGAL_MESSAGE		-13
#define		ILLEGAL_GROUP		-14
#define		BUFFER_TOO_SHORT	-15
#define         GROUPS_TOO_SHORT        -16
#define         MESSAGE_TOO_LONG        -17
#define         NET_ERROR_ON_SESSION    -18
#define         SP_BUG                  -19

typedef	struct	dummy_message_header {
	int32u	type;
	char	private_group_name[MAX_GROUP_NAME];
	int32	num_groups;
	int32	hint;
	int32	data_len;
} message_header;

#endif	/* INC_SESS_TYPES */
