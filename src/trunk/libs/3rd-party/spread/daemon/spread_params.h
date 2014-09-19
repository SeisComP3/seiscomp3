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


#ifndef	INC_SPREAD_PARAMS
#define INC_SPREAD_PARAMS

#define		SP_MAJOR_VERSION	4
#define         SP_MINOR_VERSION        3
#define         SP_PATCH_VERSION        0
#define         SPREAD_PROTOCOL         3

#define         SPREAD_BUILD_DATE       "11/June/2013"

#define		DEFAULT_SPREAD_PORT	4803

#ifndef SP_RUNTIME_DIR
#define         SP_RUNTIME_DIR          "/var/run/spread"
#endif
#ifndef SP_GROUP
#define         SP_GROUP                "spread"
#endif
#ifndef SP_USER
#define         SP_USER                 "spread"
#endif
#ifndef SP_UNIX_SOCKET
#define         SP_UNIX_SOCKET          "/tmp"
#endif

#define		MAX_PROC_NAME		 20     /* including the null, so actually max 19, look for it if changed */

#define		MAX_PROCS_SEGMENT	128
#define		MAX_SEGMENTS		 20
#define		MAX_PROCS_RING		128
#define         MAX_INTERFACES_PROC      10

#define         MAX_REPS                 25
#define         MAX_FORM_REPS            20

#define		MAX_PACKETS_IN_STRUCT 	8192
#define		PACKET_MASK		0x00001fff

#define		MAX_SEQ_GAP		1600	/* used in flow control to limit difference between highest_seq and aru */

#define		MAX_EVS_ROUNDS		500 	/* used in EVS state to limit total # of rounds to complete EVS */

#define		WATER_MARK		500	/* used to limit incoming user messages */

#define		MAX_PRIVATE_NAME	 10     /* not including the null, look for it if changed */

#define		MAX_GROUP_NAME		 (1+MAX_PRIVATE_NAME+1+MAX_PROC_NAME)
					/* #private_name#proc_name  including the null */
#include        "spu_events.h"
#define		MAX_SESSIONS		( ( MAX_FD_EVENTS-5 ) / 2 ) /* reserves 2 for each connection */

#define		DEFAULT_MAX_SESSION_MESSAGES	1000
#define         MAX_GROUPS_PER_MESSAGE  100     /* Each multicast can't send to more groups then this */

#define         MAX_WRAP_SEQUENCE_VALUE (1<<30) /* Maximum value for token->seq before reseting to zero with membership */

#define		DEFAULT_WINDOW 		100
#define		DEFAULT_PERSONAL_WINDOW	 20

#endif /* INC_SPREAD_PARAMS */
