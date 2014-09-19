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


#ifndef	INC_SESSION
#define	INC_SESSION

#include "arch.h"
#include "scatter.h"
#include "configuration.h"
#include "prot_objs.h"
#include "sess_types.h"
#include "acm.h"

typedef	struct	dummy_message_link {
	message_obj			*mess;
	struct	dummy_message_link	*next;
} message_link;

struct partial_message_info {
        int     in_mess_head;
        int     cur_element;
        int     cur_byte;
        int     total_bytes;
};

typedef	struct	dummy_session {
	char		name[MAX_PRIVATE_NAME+1]; /* +1 for the null */
        char            lib_version[3];
	int32		address;
	int		status; /* OP_SESSION or KILLED SESSION and with or without membership */
        int             priority;
	mailbox		mbox;
	int		type; /* inet or unix */ 
        struct acp_ops  acp_ops;
        int             down_queue;             /* Down queue to protocol */
        struct partial_message_info     read;   /* Read Msg from Client */
        message_obj     *read_mess;             /* Read Msg from Client */
	int		num_mess;               /* Write Queue to Client */
        struct partial_message_info     write;  /* Write Queue to Client */
	message_link	*first;                 /* Write Queue to Client */
	message_link	*last;                  /* Write Queue to Client */
	struct dummy_session *sort_prev;
	struct dummy_session *sort_next;
	struct dummy_session *hash_next;
} session;

void	Sess_init(void);
void    Sess_signal_conf_reload(void);
void	Sess_block_users_level(void);
void	Sess_unblock_users_level(void);
void    Sess_block_user(int xxx);
void    Sess_unblock_user(int xxx);
void	Sess_deliver_message( message_link *mess_link );
void	Sess_deliver_reg_memb( configuration reg_memb, membership_id reg_memb_id );
void	Sess_deliver_trans_memb( configuration trans_memb, membership_id trans_memb_id );
void    Flip_mess( message_header *head_ptr );
void    Sess_write_scat( int ses, message_link *mess_link, int *needed );
void    Sess_write(int ses, message_link *mess_link, int *needed );
void    Sess_session_authorized(int ses);
void    Sess_session_denied(int ses);
void    Sess_session_report_auth_result(struct session_auth_info *sess_auth_h, int authenticated_p );

#endif	/* INC_SESSION */
