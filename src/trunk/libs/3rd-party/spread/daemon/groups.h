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


#ifndef	INC_GROUPS
#define	INC_GROUPS

#include "session.h"

#define		GOP	1
#define		GTRANS	2
#define		GGATHER	3
#define		GGT	4

#define GROUPS_BUF_SIZE 100000
#define GROUPS_BUF_PREAMBLE_SIZE    ( sizeof(membership_id) + sizeof(char) )
#define GROUPS_BUF_GROUP_INFO_SIZE  ( MAX_GROUP_NAME + sizeof(group_id) +     \
                                      sizeof(int16u) + sizeof(int16u) )
#define GROUPS_BUF_DAEMON_INFO_SIZE ( sizeof(int32) + sizeof(membership_id) + \
                                      sizeof(int16u) )
#define MAX_LOCAL_GROUP_MEMBERS (( GROUPS_BUF_SIZE - GROUPS_BUF_PREAMBLE_SIZE \
                                   - GROUPS_BUF_GROUP_INFO_SIZE               \
                                   - GROUPS_BUF_DAEMON_INFO_SIZE )            \
                                 / MAX_GROUP_NAME )

void	G_init(void);
void    G_signal_conf_reload(void);

void	G_handle_reg_memb( configuration reg_memb, membership_id reg_memb_id );
void	G_handle_trans_memb( configuration trans_memb,
                             membership_id trans_memb_id );
void	G_handle_join( char *private_group_name, char *group_name );
void	G_handle_leave( char *private_group_name, char *group_name );
void	G_handle_kill( char *private_group_name );
void	G_handle_groups( message_link *mess_link );

int	G_analize_groups( int num_groups, char target_groups[][MAX_GROUP_NAME], int target_sessions[] );
void    G_set_mask( int num_groups, char target_groups[][MAX_GROUP_NAME], int32u *grp_mask );

int	G_private_to_names( char *private_group_name, char *private_name, char *proc_name );
int     G_get_num_local( char *group_name );
 
#endif	/* INC_GROUPS */
