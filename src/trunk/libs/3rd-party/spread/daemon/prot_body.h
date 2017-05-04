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


#ifndef	INC_PROT_BODY
#define	INC_PROT_BODY

#include "configuration.h"
#include "spread_params.h"
#include "net_types.h"
#include "spu_events.h" /* for sp_time */
#include "protocol.h"


typedef	struct	dummy_packet_info {
	packet_header	*head;
	packet_body	*body;
	int		exist;
	int		proc_index;
} packet_info;

typedef	struct	dummy_up_queue {
	int		exist;
	scatter 	*mess;
} up_queue;

#undef	ext
#ifndef	ext_prot_body
#define	ext	extern
#else
#define ext
#endif

ext	down_queue	*Down_queue_ptr;
ext	up_queue	Up_queue[MAX_PROCS_RING+1];

ext	packet_info	Packets[MAX_PACKETS_IN_STRUCT];

ext	int32		Aru;
ext	int32		My_aru;
ext	int32		Highest_seq;
ext	int32		Highest_fifo_seq;
ext	int32		Last_discarded;
ext	int32		Last_delivered;
ext	int32		Last_seq;
ext	int32		Token_rounds;
ext     int32           Received_token_rounds; /* ### Added to determine when to switch priorities*/
ext	token_header	*Last_token;

ext	int		Transitional;
ext	configuration	Trans_membership;
ext	configuration	Commit_membership;
ext	configuration	Reg_membership;

ext	int		Last_num_retrans;
ext	int		Last_num_sent;

ext	sp_time		Token_timeout;
ext	sp_time		Hurry_timeout;

ext	sp_time		Alive_timeout;
ext	sp_time		Join_timeout;
ext	sp_time		Rep_timeout;
ext	sp_time		Seg_timeout;
ext	sp_time		Gather_timeout;
ext	sp_time		Form_timeout;
ext	sp_time		Lookup_timeout;
ext	int		Wide_network;

void	Prot_token_hurry(void);
void	Prot_token_hurry_event(int dmy, void *dmy_ptr);
void	Discard_packets();
void    Prot_initiate_conf_reload( int code, void *data );
bool    Prot_need_conf_reload( void );
void    Prot_clear_need_conf_reload( void );

#endif	/* INC_PROT_BODY */
