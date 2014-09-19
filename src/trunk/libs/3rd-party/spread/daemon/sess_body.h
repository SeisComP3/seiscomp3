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


#ifndef	INC_SESS_BODY
#define	INC_SESS_BODY

#ifndef ARCH_PC_WIN95

#include <sys/time.h>
#include <sys/types.h>

#else	/* ARCH_PC_WIN95 */

#include <winsock.h>

#endif	/* ARCH_PC_WIN95 */

#include "arch.h"
#include "protocol.h"
#include "session.h"

#include <stdutil/stdskl.h>
#include <stdutil/stdarr.h>

#define		MEMB_SESSION		0x00000001
#define		OP_SESSION		0x00000010
#define         PRE_AUTH_SESSION        0x00000100

#define		Is_memb_session( status )	( status &  MEMB_SESSION )
#define		Set_memb_session( status )	( status |  MEMB_SESSION )
#define		Clear_memb_session( status )	( status & ~MEMB_SESSION )

#define		Is_op_session( status )		( status &  OP_SESSION )
#define		Set_op_session( status )	( status |  OP_SESSION )
#define		Clear_op_session( status )	( status & ~OP_SESSION ) 

#define		Is_preauth_session( status )	( status &  PRE_AUTH_SESSION )
#define		Set_preauth_session( status )	( status |  PRE_AUTH_SESSION )
#define		Clear_preauth_session( status )	( status & ~PRE_AUTH_SESSION ) 

/* All the information we need to maintain per group member is its private
 * group name. */
typedef struct dummy_member {
        char  name[MAX_GROUP_NAME];     /* NOTE: groups.c depends on 'name' being first member (MembersList) */
} member;

typedef struct  dummy_daemon_members {
        int32           proc_id;        /* NOTE: groups.c depends on 'proc_id' being the first member (DaemonsList) */
	membership_id   memb_id;        /* used for vs_set sorting in G_build_memb_vs_buf; unknown_memb_id means partitioned. */
        stdskl          MembersList;    /* (member*) -> nil */
} daemon_members;

typedef	struct	dummy_group {
        char            name[MAX_GROUP_NAME];  /* NOTE: groups.c depends on 'name' being the first member (GroupsList) */
	group_id        grp_id;
        bool            changed;
        int             num_members;    /* sums over all daemons in DaemonsList */
        stdskl          DaemonsList;    /* (daemon_members*) -> nil */
        stdarr          mboxes;         /* (mailbox): local clients unordered */
        route_mask      grp_mask;
} group;

#undef	ext
#ifndef ext_sess_body
#define ext extern
#else
#define ext
#endif

ext	proc		My;

ext	int		Num_groups;
ext	group		Groups;

ext	int		Num_sessions;
ext	session		Sessions[MAX_SESSIONS+1]; /* +1 for rejecting the next one */

ext	int		Session_threshold;

ext	char		Temp_buf[100000];
/*
 * ext	char		Temp_MessageGroupName_buf[MAX_GROUPS_PER_MESSAGE*MAX_GROUP_NAME];
 */
void	Sess_set_active_threshold(void);
void	Sess_write( int ses, message_link *mess_link, int *needed );
void	Sess_dispose_message( message_link *mess_link );
int	Sess_get_session( char *name );
int	Sess_get_session_index (int mbox);

#endif	/* INC_SESS_BODY */
