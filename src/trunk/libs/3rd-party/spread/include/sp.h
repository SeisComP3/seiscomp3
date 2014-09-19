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



#ifndef INC_SP
#define INC_SP

/* for size_t */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define 	int16	short
#define		int32	int

#define		Flip_int16( type )	( ( (type >> 8) & 0x00ff) | ( (type << 8) & 0xff00) )

#define		Flip_int32( type )	( ( (type >>24) & 0x000000ff) | ( (type >> 8) & 0x0000ff00) | ( (type << 8) & 0x00ff0000) | ( (type <<24) & 0xff000000) )

/* Events priorities */

#define		LOW_PRIORITY	0
#define		MEDIUM_PRIORITY	1
#define		HIGH_PRIORITY	2

/* Interface */

#define		DEFAULT_SPREAD_PORT	4803

#define         SPREAD_VERSION          ( (4 << 24) | ( 3 << 16) | 0 )

#define		MAX_GROUP_NAME		32
#define         MAX_PRIVATE_NAME        10 /* largest possible size of private_name field of SP_connect() */
#define         MAX_PROC_NAME           20 /* largest possible size of process name of daemon */

#define         UNRELIABLE_MESS         0x00000001
#define         RELIABLE_MESS           0x00000002
#define         FIFO_MESS               0x00000004
#define         CAUSAL_MESS             0x00000008
#define         AGREED_MESS             0x00000010
#define         SAFE_MESS               0x00000020
#define         REGULAR_MESS            0x0000003f

#define		SELF_DISCARD		0x00000040
#define         DROP_RECV               0x01000000

#define         REG_MEMB_MESS           0x00001000
#define         TRANSITION_MESS         0x00002000
#define		CAUSED_BY_JOIN		0x00000100
#define		CAUSED_BY_LEAVE		0x00000200
#define		CAUSED_BY_DISCONNECT	0x00000400
#define		CAUSED_BY_NETWORK	0x00000800
#define         MEMBERSHIP_MESS         0x00003f00

#define         ENDIAN_RESERVED         0x80000080
#define         RESERVED                0x003fc000
#define         REJECT_MESS             0x00400000

#define         Is_unreliable_mess( type )     		( type &  UNRELIABLE_MESS      )
#define         Is_reliable_mess( type )       		( type &  RELIABLE_MESS        )
#define         Is_fifo_mess( type )         		( type &  FIFO_MESS            )
#define         Is_causal_mess( type )          	( type &  CAUSAL_MESS          )
#define         Is_agreed_mess( type )          	( type &  AGREED_MESS          )
#define         Is_safe_mess( type )            	( type &  SAFE_MESS            )
#define         Is_regular_mess( type )         	( (type &  REGULAR_MESS) && !(type & REJECT_MESS)  )

#define         Is_self_discard( type )         	( type &  SELF_DISCARD         )

#define         Is_reg_memb_mess( type )        	( type &  REG_MEMB_MESS        )
#define         Is_transition_mess( type )      	( type &  TRANSITION_MESS      )
#define         Is_caused_join_mess( type )     	( type &  CAUSED_BY_JOIN       )
#define         Is_caused_leave_mess( type )    	( type &  CAUSED_BY_LEAVE      )
#define         Is_caused_disconnect_mess( type )	( type &  CAUSED_BY_DISCONNECT )
#define         Is_caused_network_mess( type )		( type &  CAUSED_BY_NETWORK )
#define         Is_membership_mess( type )      	( (type &  MEMBERSHIP_MESS) && !(type & REJECT_MESS) )

#define         Is_reject_mess( type )          	( type &  REJECT_MESS          )

#define         Is_self_leave( type )   (( (type) & CAUSED_BY_LEAVE) && !( (type) & (REG_MEMB_MESS | TRANSITION_MESS)))

#define         ACCEPT_SESSION           1
#define		ILLEGAL_SPREAD		-1
#define		COULD_NOT_CONNECT	-2
#define         REJECT_QUOTA            -3
#define         REJECT_NO_NAME          -4
#define         REJECT_ILLEGAL_NAME     -5
#define         REJECT_NOT_UNIQUE       -6
#define         REJECT_VERSION		-7
#define         CONNECTION_CLOSED	-8
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

#define		MAX_CLIENT_SCATTER_ELEMENTS	100

typedef 	int             mailbox;
typedef		int             service;	

typedef	struct	dummy_scat_element{
	char	*buf;
	size_t	len;
} scat_element;

typedef	struct	dummy_scatter{
	size_t		num_elements;
	scat_element	elements[MAX_CLIENT_SCATTER_ELEMENTS];
} scatter;

typedef struct	dummy_group_id {
	int32		id[3];
} group_id;

typedef struct dummy_vs_set_info {
        unsigned int num_members;
        unsigned int members_offset;  /* offset from beginning of msg body*/
} vs_set_info;

typedef struct dummy_membership_info {
        group_id     gid;
        char         changed_member[MAX_GROUP_NAME]; 
        unsigned int num_vs_sets;
        vs_set_info  my_vs_set;
} membership_info;

#include "sp_events.h"
#include "sp_func.h"
#ifdef __cplusplus
}
#endif

#endif /* INC_SP */

