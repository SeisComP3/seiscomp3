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



#ifndef fl_h_2000_03_20_14_36_26_jschultz_at_cnds_jhu_edu
#define fl_h_2000_03_20_14_36_26_jschultz_at_cnds_jhu_edu

#include <sp.h>

/* FL service types */
#define DONT_BLOCK     0x10000000  
#define FLUSH_REQ_MESS 0x20000000
#define SUBGROUP_CAST  0x40000000

/* FL service query macros */
#define Is_flush_req_mess(serv) (((serv) & FLUSH_REQ_MESS) != 0)
#define Is_subgroup_mess(serv)  (((serv) & SUBGROUP_CAST) != 0)

/* FL error codes */
#define ILLEGAL_PARAM        -24
#define WOULD_BLOCK          -25
#define ILLEGAL_MESSAGE_TYPE -26
#define ILLEGAL_STATE        -27
#define ILLEGAL_RECEIVERS    -28

/* maximum # of usable scatter elements for FL msgs */
#define FL_MAX_SCATTER_ELEMENTS (MAX_CLIENT_SCATTER_ELEMENTS - 1)

/* minimum message type a user is allowed to use - less than this is illegal */
#define FL_MIN_LEGAL_MESS_TYPE ((int16) -32765)

#ifdef __cplusplus
extern "C" {
#endif

int   FL_lib_init(void);

void  FL_version(int *major_ver, int *minor_ver, int *patch_ver);

int   FL_connect(const char *daemon_name, const char *user_name, int priority, 
		 mailbox *mbox, char *private_name);

int   FL_disconnect(mailbox mbox);

int   FL_join(mailbox mbox, const char *group_name);

int   FL_leave(mailbox mbox, const char *group_name);

int   FL_flush(mailbox mbox, const char *group_name);

int   FL_unicast(mailbox mbox, service serv_type, const char *group_name, 
		 const char *recvr_name, int16 mess_type, int mess_len, const char *mess);

int   FL_scat_unicast(mailbox mbox, service serv_type, const char *group_name,
		      const char *recvr_name, int16 mess_type, const scatter *scat);

int   FL_subgroupcast(mailbox mbox, service serv_type, const char *group_name,
		      int num_recvrs, char recvr_names[][MAX_GROUP_NAME],
		      int16 mess_type, int mess_len, const char *mess);

int   FL_scat_subgroupcast(mailbox mbox, service serv_type, const char *group_name,
			   int num_recvrs, char recvr_names[][MAX_GROUP_NAME],
			   int16 mess_type, const scatter *scat);

int   FL_multicast(mailbox mbox, service serv_type, const char *group_name,
		   int16 mess_type, int mess_len, const char *mess);

int   FL_scat_multicast(mailbox mbox, service serv_type, const char *group_name,
			int16 mess_type, const scatter *scat_mess);

int   FL_receive(mailbox mbox, service *serv_type, char *sender_name,
		 int max_groups, int *num_groups, char group_names[][MAX_GROUP_NAME],
		 int16 *mess_type, int *endian_mismatch, int max_mess_len,
		 char *mess, int *more_msgs);
  
int   FL_scat_receive(mailbox mbox, service *serv_type, char *sender_name,
		      int max_groups, int *num_groups, char group_names[][MAX_GROUP_NAME],
		      int16 *mess_type, int *endian_mismatch, scatter *scat_mess, 
		      int *more_msgs);
  
int   FL_more_msgs(mailbox mbox);

int   FL_poll(mailbox mbox);

void  FL_error(int error_code);

/* returns offset in memb. message of gid (group id), num_vs and vs_set */

int   FL_get_gid_offset_memb_mess(void);
int   FL_get_num_vs_offset_memb_mess(void);
int   FL_get_vs_set_offset_memb_mess(void);


#ifdef __cplusplus
}
#endif

#endif
