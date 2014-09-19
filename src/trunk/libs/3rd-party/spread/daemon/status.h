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


#ifndef INC_STATUS
#define INC_STATUS

#include "arch.h"
#include "scatter.h"

typedef	struct	dummy_status{
	int32	sec;
	int32	state;
	int32	gstate;
	int32	packet_sent;
	int32	packet_recv;
	int32	packet_delivered;
	int32	retrans;
	int32	u_retrans;
	int32	s_retrans;
	int32	b_retrans;
	int32   aru;
	int32   my_aru;
	int32   highest_seq;
	int32	token_hurry;
	int32	token_rounds;
	int32	my_id;
	int32	leader_id;
	int32	message_delivered;
	int16	membership_changes;
	int16	num_procs;
	int16	num_segments;
	int16	window;
	int16	personal_window;
	int16	num_sessions;
	int16	num_groups;
	int16	major_version;
	int16	minor_version;
	int16	patch_version;
} status;

#undef  ext
#ifndef	status_ext
#define	ext	extern
#else
#define ext
#endif

ext 	status	GlobalStatus; 

void	Stat_init();
void	Stat_handle_message( sys_scatter *scat );

#endif	/* INC_STATUS */ 
