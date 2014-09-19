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



#ifndef ACM_H
#define ACM_H

#include "arch.h"
#include "spread_params.h"      /* For MAX_GROUP_NAME */

#define MAX_AUTH_METHODS 3
#define MAX_AUTH_NAME 30
#define MAX_AUTH_LIST_LEN (MAX_AUTH_METHODS * MAX_AUTH_NAME)
/* NOTE: (MAX_AUTH_NAME * MAX_AUTH_METHODS) must be < 255
 * This is because we send the length of the auth methods list as a unsigned char
 * If we want to increase the number of methods we will have to switch to sending a 
 * short int. This requires a change in the client-server protocol.
 */
typedef enum ACM_ReturnVal {
     ACM_ACCESS_DENIED,
     ACM_ACCESS_ALLOWED,
} ACM_ReturnVal;

struct session_auth_info {
        mailbox mbox;
        void *module_data;
        int num_required_auths;
        int completed_required_auths;
        int required_auth_methods[MAX_AUTH_METHODS];
        int required_auth_results[MAX_AUTH_METHODS];
};

struct auth_ops {
       void (*auth_client_connection) (struct session_auth_info *sess_auth_p);
       /* not currently used -- placeholder for future stuff */
       void (*auth_monitor_connection) (mailbox mbox, int32 ip_addr);
       void (*deliver_authinfo) (int info_len, void *authinfo);
};

struct acp_ops {
       ACM_ReturnVal (*open_connection) (char *user);
       ACM_ReturnVal (*open_monitor) (char *user); /* not user currently */
       ACM_ReturnVal (*join_group) (char *user, char *group, void *acm_token);
       ACM_ReturnVal (*leave_group) (char *user, char *group, void *acm_token);
       ACM_ReturnVal (*p2p_send) (char *user, int num_dests, char dests[][MAX_GROUP_NAME], int service_type, int16 mess_type);
       ACM_ReturnVal (*mcast_send) (char *user, int num_groups, char groups[][MAX_GROUP_NAME], int service_type, int16 mess_type);
};

/* Function declarations */
void Acm_init(void);

/* Auth Functions */
bool Acm_auth_query_allowed(char *auth_name);
char *Acm_auth_get_allowed_list(void);
bool Acm_auth_add_method(char *name, struct auth_ops *ops);
bool Acm_auth_set_enabled(char *auth_name);
bool Acm_auth_set_disabled(char *auth_name);
bool Acm_auth_set_required(char *auth_name);
void * Acm_auth_get_auth_client_connection(int authid);
void * Acm_auth_get_auth_client_connection_byname(char *auth_name);
struct session_auth_info *Acm_auth_create_sess_info_forIP(mailbox mbox);
struct session_auth_info *Acm_auth_create_sess_info(mailbox mbox, char *auth_name);

/* Access Control Policy Functions */
void Acm_acp_fill_ops(struct acp_ops *acp_ops_h);
bool Acm_acp_set_policy(char *policy_name);
bool Acm_acp_query_allowed(char *acp_name);
bool Acm_acp_add_method(char *name, struct acp_ops *ops);

#endif /* ACM_H */
