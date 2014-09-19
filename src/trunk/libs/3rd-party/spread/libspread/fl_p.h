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

#ifndef flush_p_h_2000_04_24_14_16_31_jschultz_at_cnds_jhu_edu
#define flush_p_h_2000_04_24_14_16_31_jschultz_at_cnds_jhu_edu

/* In this library I assume that the stdutil library I am linking with
   was compiled with the -D STD_USE_EXCEPTIONS flag. This allows me to
   assume that all reasonable calls to stdutil fcns won't fail, and if
   they do (due to a system problem, such as malloc failing) they abort
   with an appropriate error message, line number, etc.

   JLS 2006 - That's bad form for a library John. Oh well ... ;)
*/

#include <fl.h>
#include <scatp.h>
#include <stdutil/stddefines.h>
#include <stdutil/stderror.h>
#include <stdutil/stddll.h>
#include <stdutil/stdhash.h>

#define FL_MAJOR_VERSION 2
#define FL_MINOR_VERSION 0
#define FL_PATCH_VERSION 0

#ifdef _REENTRANT
#  include <stdutil/stdthread.h>
#  define FL_MUTEX                      stdmutex
#  define FL_COND                       stdcond
#  define FL_MUTEX_construct(mut, type) stdmutex_construct((mut), (type))
#  define FL_MUTEX_destruct(mut)        stdmutex_destruct(mut)
#  define FL_MUTEX_grab(mut)            stdmutex_grab(mut)
#  define FL_MUTEX_drop(mut)            stdmutex_drop(mut)
#  define FL_COND_construct(cond)       stdcond_construct(cond)
#  define FL_COND_destruct(cond)        stdcond_destruct(cond)
#  define FL_COND_wait(cond, mut)       stdcond_wait((cond), (mut))
#  define FL_COND_wake_all(cond)        stdcond_wake_all(cond)
#else
#  define FL_MUTEX                      int
#  define FL_COND                       int
#  define FL_MUTEX_construct(mut, type)
#  define FL_MUTEX_destruct(mut)
#  define FL_MUTEX_grab(mut)
#  define FL_MUTEX_drop(mut)
#  define FL_COND_construct(cond)
#  define FL_COND_destruct(cond)
#  define FL_COND_wait(cond, mut)
#  define FL_COND_wake_all(cond)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* The type fl_view_state specifies the possible states a connection
   can be in, in reference to a particular group's membership algorithm.

   STEADY    - normal operation, member of the group
   AUTHORIZE - a SP lvl memb event has occured, user needs to flush this group
   AGREE     - user has flushed this group, collecting floks for next fl view
   VERIFY    - new flush view installed, protect against non-causal msgs
*/
typedef enum { STEADY, AUTHORIZE, AGREE, VERIFY } fl_view_state;

/* The type member_join_state specifies the possible states a connection 
   can be in, in reference to being a member of a particular group. Only
   JOINED connections are allowed to leave and send to a group.

   JOINING - this connection is in the process of joining this group
   JOINED  - this connection has been installed in a fl view of this group 
   LEAVING - this connection is leaving this group 
*/
typedef enum { JOINING, JOINED, LEAVING } member_join_state;

/* A gc_buff_mess contains all of the information necessary to replicate a message. */
/* This is the struct I use when I have to buffer messages. */
typedef struct {
  mailbox mbox;
  service serv_type;
  char    sender[MAX_GROUP_NAME];
  int     num_groups;
  char    (*groups)[MAX_GROUP_NAME];
  int16   mess_type;
  int     endian_mismatch;
  int     mess_len;
  char    *mess;
  int     total_size;         /* used for reporting number of bytes buffered: set by deliver() */
} gc_buff_mess;

/* A gc_recv_mess contains references to user's parameters from a call to receive.        */
/* I use this struct to pass data around in the internal state machine code.              */
/* The struct member "delivered" indicates whether or not the user's parameters have been */
/* filled with legal return values and is currently returnable to the user or not.        */
typedef struct {
  mailbox  mbox;
  service  orig_serv_type;                  /* user's original service type request to receive */
  service  *serv_type;
  char     *sender;
  int      max_groups;
  int      *num_groups;
  char     (*groups)[MAX_GROUP_NAME];
  int16    *mess_type;
  int      *endian_mismatch;
  scatter  *scat_mess;
  int      ret;                                              /* return value from receive call */

  int      delivered;                  /* does this recv_mess already contain a delivered msg? */
  int      vulnerable;                                              /* is this msg vulnerable? */
  group_id dst_gid;                                /* the view id this message is addressed to */

  int      num_new_grps;                    /* potentially alloc'ed buffers for DROP_RECV msgs */
  char     (*new_grps)[MAX_GROUP_NAME];
  scatter  new_msg;
} gc_recv_mess;

/* This struct contains information that specifies a group membership view. */
typedef struct {
  group_id gid;
  service  memb_type;
  int      in_trans_memb;               /* has a transitional msg been delivered in this view? */
  int      orig_num_membs;
  char     (*membs_names)[MAX_GROUP_NAME];
  stdhash  orig_membs, curr_membs;                      /* <char*, nil>, ptrs into membs_names */
  int16    my_index;                                /* index of this connection in membs_names */
} view;

/* This struct contains information on a spread level membership
   change for a particular group that has not yet been fully handled
   by the flush layer. This structure is used by the fl memb protocol
   while it attempts to install a view.  These structures are built in
   two different scenarios: (1) a FLUSH_OK or (2) a spread membership
   message is received for a previously unseen/unknown view id.

   There are 2 states that this structure can be in. These two states
   correspond to whether or not a spread membership message for the
   particular view id has been received yet or not. Which state the
   structure is in is indicated by the struct member memb_mess_recvd.

   The struct member memb_change_age indicates how many spread
   membership messages have been received in this group since this
   structure was built. If this value exceeds the value FLOK_AGE_LIMIT
   and !memb_mess_recvd, then I take this as an indication that the
   spread membership message that this structure is waiting for was
   already delivered in a view of which I wasn't a member. In that
   case, I will never receive that membership message and I can
   "safely" discard this struct, as that membership change was already
   handled by other members (this assumption is not 100% correct as
   FIFO messages can jump before an unbounded number of memberships;
   however, I feel that the probability of a FIFO message jumping
   before FLOK_AGE_LIMIT memberships is vanishingly small).

   If !memb_mess_recvd then the only fields that contain meaningful data
   are memb_change_age, flok_senders, and memb_info->gid.

   If memb_mess_recvd then delta_member contains either (1) a
   malloc'ed copy of the delta member (if the membership change was
   caused by join/leave/disconnect) or (2) null. All of the info in
   memb_info is appropriately filled in when in this state.

   The struct member flok_senders contains by value (ie - alloc'ed by
   flok_senders) members' names who have flok'ed to this view_id.  
*/
typedef struct {
  view     *memb_info;
  int      memb_mess_recvd;
  int      memb_change_age;
  char     *delta_member;
  stdhash  flok_senders;                                         /* <char[MAX_GROUP_NAME], nil> */
  group_id gid;             /* temporary: erase me */
} sp_memb_change;

/* This struct contains all of the group level context for a connection in 
   reference to a particular group. 

   group_name  - the name of the group with which this information is associated
   mstate      - current join state this member is in
   vstate      - current fl memb protocol state this member is in
   curr_change - points to current sp_memb_change fl is trying to resolve (head of memb_queue)
   sp_view     - my most recently installed spread membership view (initially just me)
   fl_view     - my most recently installed flush membership view (initially just me)
   flush_recvs - a count of how many flush_recvs have been recvd for fl_view
   mess_queue  - a queue of gc_buff_mess*s to be delivered later, in order
   memb_queue  - a queue of sp_memb_change*s to be handled in order
   pmemb_hash  - sp_memb_change*s that are being handled currently

   sp_view either points at fl_view or at the most recent pending SP
   memb event's view in memb_queue (see handle_recv_reg_memb_mess)

   fl_view steals curr_change's view when curr_change is installed (see update_fl_view)
*/
typedef struct {
  char group[MAX_GROUP_NAME];

  member_join_state mstate;
  fl_view_state     vstate;

  sp_memb_change *curr_change;

  view *sp_view;
  view *fl_view;
  int flush_recvs;

  stddll  mess_queue;                                                /* <gc_buff_mess*> */
  stddll  memb_queue;                                              /* <sp_memb_change*> */
  stdhash pmemb_hash;                                    /* <group_id, sp_memb_change*> */
} fl_group;

/* This struct contains all of the connection level context for a connection */
typedef struct {
  FL_MUTEX reserve_lock;  /* protects reservations, disconnecting; destroy_cond's mutex */
  size_t   reservations;                    /* count of reservations on this connection */
  int      disconnecting;                     /* is this connection being disconnected? */
  FL_COND  destroy_cond;   /* disconnector thread waits on this until reservations == 0 */

  FL_MUTEX recv_lock;             /* used to serialize calls to FL_recv on a connection */
  FL_MUTEX conn_lock;            /* lock for access to the connection's data, see below */

  /* acquiring conn_lock allows a thread to examine everything below here */
  mailbox mbox;           
  int     priority;
  int     group_memb;
  char    daemon[MAX_GROUP_NAME];
  char    user[MAX_GROUP_NAME];
  char    private[MAX_GROUP_NAME];

  /* acquiring conn_lock allows a thread to examine and modify everything below here */
  stdhash groups;              /* information on groups involved in: <char*, fl_group*> */
  stddll  mess_queue;           /* messages that the user can read out: <gc_buff_mess*> */
  int     bytes_queued;     /* number of bytes available in mess_queue, used by FL_poll */
} fl_conn;

/************************* Private Variables, Functions and Macros *****************************/

/* leave in or take out debugging printf's and checks? */
#ifdef NDEBUG
# define DEBUG(statement)
#else
# define DEBUG(statement) statement
#endif

/* age limit for a sp_memb_change with no sp memb mess */
#define FLOK_AGE_LIMIT 200   

#define IS_ILLEGAL_SEND_STYPE(serv_type) \
(((serv_type) & (FLUSH_REQ_MESS | MEMBERSHIP_MESS | ENDIAN_RESERVED | RESERVED)) != 0)

/* mess types reserved by flush layer indicating special flush messages */
/* MAKE SURE THAT FL_MIN_LEGAL_MESS_TYPE (in fl.h) IS ONE MORE THAN THE HIGHEST RESERVED MESS TYPE!!! */
#define FLUSH_OK_MESS                    ((int16) -32768)
#define FLUSH_RECV_MESS                  ((int16) -32767)
#define VULNERABLE_MESS                  ((int16) -32766)
#define IS_ILLEGAL_SEND_MTYPE(mess_type) ((mess_type) < FL_MIN_LEGAL_MESS_TYPE)

/* structure routines */
static fl_group *create_fl_group(const char *conn_name, const char *group_name);
static void free_fl_group(fl_group *group);

static void free_gc_buff_mess(gc_buff_mess *msg);

static sp_memb_change *create_sp_memb_change(group_id gid);
static void free_sp_memb_change(sp_memb_change *change);

static view *create_view(group_id gid);
static void free_view(view *v);
static void fill_view(view *v, service memb_type, int num_membs, 
			     char (*membs)[MAX_GROUP_NAME], int16 index);

/* interface for reserving/locking connections */
static fl_conn *lock_conn(mailbox mbox);             /* reserve and lock connection */
static void    unlock_conn(fl_conn *conn);
static fl_conn *make_reservation(mailbox mbox);   /* declare interest in connection */
static void    cancel_reservation(fl_conn *conn);
static int     acquire_conn_lock(fl_conn *conn);       /* get rights to read/modify */
static void    release_conn_lock(fl_conn *release);
static int     acquire_recv_lock(fl_conn *conn);     /* get rights to enter FL_recv */
static void    release_recv_lock(fl_conn *release);

/* interface for groups for a connection */
static fl_group *add_group(fl_conn *conn, const char *group);
static void      remove_group(fl_conn *conn, fl_group *group);
static fl_group *get_group(fl_conn *conn, const char *grp);

/* functions for trying to deliver/return msgs to the user */
static gc_buff_mess *deliver(fl_conn *c, gc_recv_mess *um, gc_buff_mess *bm, int al);
static gc_buff_mess *deliver_trans_sig(fl_conn *conn, gc_recv_mess *um, char *group);
static gc_buff_mess *deliver_flush_req(fl_conn *conn, gc_recv_mess *um, char *group);

/* converting back and forth between user's variables and buffered messages (dst, src) */
static int buffm_to_userm(gc_recv_mess *um, const gc_buff_mess *bm);
static void userm_to_buffm(gc_buff_mess *bm, const gc_recv_mess *um);

/* complex "low level" internal fcns that deal with SP layer */
static int FL_int_flush(fl_conn *conn, fl_group *group);
static int FL_int_scat_multicast(mailbox m, service s, const char *g,
					int nr, char r[][MAX_GROUP_NAME],
					int16 mess_type, const scatter *scat);
static int FL_int_receive(gc_recv_mess *m);

/* state machine fcns: handler functions */
static int  recv_and_handle(fl_conn *c, gc_recv_mess *m);
static void handle_recv_flush_ok(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_recv_flush_recv(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_recv_reg_mess(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_recv_reg_memb_mess(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_next_memb_change(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_recv_trans_memb_mess(fl_conn *c, fl_group *g, gc_recv_mess *m);
static void handle_recv_self_leave_memb_mess(fl_conn *c, fl_group *g, gc_recv_mess *m);

/* helper functions for handler functions */ 
static void  install_new_view(fl_conn *c, const fl_group *g, gc_recv_mess *m);
static void  update_fl_view(fl_group *group);
static char *determine_leavers(stdhash *leavers, view *last_sp_view, gc_recv_mess *m);
static void  age_and_invalidate_pmembs(fl_group *group);
static sp_memb_change *collapse_memberships(fl_group *group, stdhash *leavers, 
						   sp_memb_change *c);
/* miscellaneous functions */
/* where is full data for a msg (ie - groups or new_grps?) due to DROP_RECV considerations */
static void get_groups_info(const gc_recv_mess *m, int *num_groups, 
				   char (**groups)[MAX_GROUP_NAME]);
static void get_scat_info(const gc_recv_mess *m, int *mess_len, scatter **scat_mess);

static int is_private_group(const char *group);
static int is_vulnerable_mess(const fl_group *group, service serv_type);

#ifndef NDEBUG
static const char *state_str(fl_view_state state);
#endif

static float FL_SP_version(void);

/* group name hash fcns: used by stdhashs where the keys are group_names */
static int      group_name_cmp(const void *str1, const void *str2);
static stdhcode group_name_hashcode(const void *str);
static int      group_name_ptr_cmp(const void *str_ptr1, const void *str_ptr2);
static stdhcode group_name_ptr_hashcode(const void *str_ptr);
static int      group_id_cmp(const void *gid1, const void *gid2);
static stdhcode group_id_hashcode(const void *gid);

#ifdef __cplusplus
}
#endif

#endif
