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


#ifndef INC_PROTOCOL
#define INC_PROTOCOL

#include "arch.h"
#include "scatter.h"
#include "session.h"

typedef struct  dummy_down_link {
	int32			type;
	scatter 		*mess; 
	struct	dummy_down_link *next;
} down_link;

typedef struct  dummy_down_queue {
	int		num_mess;
	int		cur_element;
	down_link	*first;
	down_link	*last;
} down_queue;

#define NORMAL_DOWNQUEUE        0
#define GROUPS_DOWNQUEUE        1

void	Prot_init(void);
void	Prot_set_down_queue( int queue_type );
void	Prot_new_message( down_link *down_ptr, int not_used_in_spread3_p );
void    Prot_init_down_queues(void);
void    Prot_Create_Local_Session(session *new_sess);
void    Prot_Destroy_Local_Session(session *old_sess);
down_link       *Prot_Create_Down_Link(message_obj *msg, int type, int mbox, int cur_element);
void    Prot_kill_session(message_obj *msg);
void	Prot_set_prev_proc(configuration *memb);

/* thresholds defined in net_types.h: UNRELIABLE_TYPE, AGREED_TYPE, BLOCK_REGULAR_DELIVERY, etc. */

int     Prot_get_delivery_threshold( void );
void    Prot_set_delivery_threshold( int thresh );

#endif	/* INC_PROTOCOL */ 
