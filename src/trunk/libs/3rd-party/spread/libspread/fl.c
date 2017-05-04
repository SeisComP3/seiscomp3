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

#include <stdlib.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <fl_p.h>
#include <stdutil/stdutil.h>

#ifdef _REENTRANT
static FL_MUTEX glob_conns_lock;
#endif

/* glob_conns is a look up table that maps mailboxes to fl_conn*s */
static stdhash  glob_conns = STDHASH_STATIC_CONSTRUCT(sizeof(mailbox), sizeof(fl_conn*), NULL, NULL, 0);

/* only does indention properly for single threaded programs right now */
int std_stkfprintf(FILE *stream, int entering, const char *fmt, ...) 
{
#define MAX_TAB_IN 4096
#define INDENT 2
  static int tab_in = 0;
  static char tab[MAX_TAB_IN] = { 0 };
  va_list ap;
  int ret;

  if (entering < 0) {
    if ((tab_in -= INDENT) < 0)
      stderr_output(STDERR_ABORT, 0,"popped off of top of empty trace print stack!\n");
    memset(tab + tab_in, 0, INDENT);
    fprintf(stream, "%sST Leave: ", tab);
  } else if (entering > 0) {
    fprintf(stream, "%sST Enter: ", tab);
    if (tab_in + INDENT >= MAX_TAB_IN)
      stderr_output(STDERR_ABORT, 0,"execution stack depth exceded MAX_TAB_IN: %d\n", MAX_TAB_IN);
    memset(tab + tab_in, ' ', INDENT);
    tab_in += INDENT;
  } else
    fprintf(stream, "%s", tab);

  va_start(ap, fmt);
  ret = vfprintf(stream, fmt, ap);
  va_end(ap);

  return ret;
}

/********************************* public flush layer interface ********************************/

int FL_lib_init(void)
{
  static stdbool first_time = STDTRUE;  /* *TRY* to mitigate bad call race conditions to FL_lib_init */
  int            ret        = ILLEGAL_STATE;

  if (first_time) {
    first_time = STDFALSE;
    FL_MUTEX_construct(&glob_conns_lock, STDMUTEX_FAST);
    ret = 0;
  }

  return ret;
}

void FL_version(int *major, int *minor, int *patch) { 
  *major = FL_MAJOR_VERSION;
  *minor = FL_MINOR_VERSION;
  *patch = FL_PATCH_VERSION;
}

/* Establish a new fl connection. If SP_connect succceeds, create a new fl_conn instance. */
int FL_connect(const char *daemon, const char *user, int priority, 
			  mailbox *mbox, char private[MAX_GROUP_NAME]) {
  int ret;
  fl_conn *conn;        

  DEBUG(std_stkfprintf(stderr, 1, "FL_connect: daemon '%s', user '%s', priority %d\n",
		       daemon, user, priority));

  if (FL_SP_version() < (float) 3.12) {             /* flush depends on the DROP_RECV semantics */
    DEBUG(std_stkfprintf(stderr, 0, "REJECT_VERSION: SP too old v%f < v3.12\n", FL_SP_version()));
    ret = REJECT_VERSION;

  } else if ((ret = SP_connect(daemon, user, priority, 1, mbox, private)) == ACCEPT_SESSION) {
    DEBUG(std_stkfprintf(stderr, 0, "mbox %d, private '%s'\n", *mbox, private));

    if ((conn = (fl_conn*) calloc(1, sizeof(fl_conn))) == 0)
      stderr_output(STDERR_ABORT, 0,"(%s, %d): calloc(1, %u)\n", __FILE__, __LINE__, sizeof(fl_conn));
    
    FL_MUTEX_construct(&conn->reserve_lock, STDMUTEX_FAST);
    conn->reservations  = 0;
    conn->disconnecting = 0;
    FL_COND_construct(&conn->destroy_cond);
    
    FL_MUTEX_construct(&conn->recv_lock, STDMUTEX_FAST);
    FL_MUTEX_construct(&conn->conn_lock, STDMUTEX_FAST);
    
    conn->mbox       = *mbox;
    conn->priority   = priority;
    conn->group_memb = 1;
    strncpy(conn->daemon, daemon, MAX_GROUP_NAME);
    strncpy(conn->user, user, MAX_GROUP_NAME);
    strncpy(conn->private, private, MAX_GROUP_NAME);
    
    stdhash_construct(&conn->groups, sizeof(char*), sizeof(fl_group*), 
		      group_name_ptr_cmp, group_name_ptr_hashcode, 0);
    stddll_construct(&conn->mess_queue, sizeof(gc_buff_mess*));             /* <gc_buff_mess*> */
    conn->bytes_queued = 0;
    
    FL_MUTEX_grab(&glob_conns_lock);                                         /* LOCK CONNS TAB */
    stdhash_insert(&glob_conns, 0, mbox, &conn);                   /* add mbox -> conn mapping */
    FL_MUTEX_drop(&glob_conns_lock);                                     /* UNLOCK CONNS TAB */
  }

  DEBUG(std_stkfprintf(stderr, -1, "FL_connect: ret %d\n", ret));

  return ret;
}

/* Destroy a mbox. This fcn recursively reclaims all resources
   associated with a connection named mbox, in a synchronized, proper
   fashion and returns whatever SP_disconnect does. If mbox doesn't
   represent a valid fl connection, it just returns ILLEGAL_SESSION.  
*/
int FL_disconnect(mailbox mbox) {
  stdit hit;
  stdit lit;
  fl_conn *conn;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_disconnect: mbox %d\n", mbox));
  FL_MUTEX_grab(&glob_conns_lock);                                           /* LOCK CONNS TAB */
  if (!stdhash_is_end(&glob_conns, stdhash_find(&glob_conns, &hit, &mbox))) {              /* mbox found */
    conn = *(fl_conn**) stdhash_it_val(&hit);
    stdhash_erase(&glob_conns, &hit);                                 /* remove mbox -> fl_conn* mapping */
    FL_MUTEX_drop(&glob_conns_lock);                                     /* UNLOCK CONNS TAB */
    /* now threads cannot come behind us and make_reservations() on this fl_conn */
    
    FL_MUTEX_grab(&conn->reserve_lock);                                   /* LOCK RESERVE_LOCK */
    conn->disconnecting = 1;                                 /* set disconnection indicator */
    ret = SP_disconnect(mbox);                                /* unblock any blocked receivers */
    
    if (conn->reservations != 0) {                          /* WAIT if connection still in use */
      DEBUG(std_stkfprintf(stderr, 0, "Waiting for mbox(%d, %p) to no longer be in use: "
			   "%d reservations!\n", mbox, conn, conn->reservations));
      FL_COND_wait(&conn->destroy_cond, &conn->reserve_lock);
    }
    assert(conn->reservations == 0);                         /* should be no more reservations */

    /* now, no other threads are using the connection -> safe to reclaim */
    DEBUG(std_stkfprintf(stderr, 0, "Mbox(%d, %p) no longer in use: reclaiming!\n", mbox, conn));
    FL_MUTEX_drop(&conn->reserve_lock);                               /* UNLOCK RESERVE_LOCK */
    
    FL_MUTEX_destruct(&conn->reserve_lock);  /* recursively destroy fl_conn and sub-structures */
    FL_COND_destruct(&conn->destroy_cond);
    FL_MUTEX_destruct(&conn->recv_lock);
    FL_MUTEX_destruct(&conn->conn_lock);
    
    for (stdhash_begin(&conn->groups, &hit); !stdhash_is_end(&conn->groups, &hit); stdhash_it_next(&hit))
      free_fl_group(*(fl_group**) stdhash_it_val(&hit));
    stdhash_destruct(&conn->groups);
    
    for (stddll_begin(&conn->mess_queue, &lit); !stddll_is_end(&conn->mess_queue, &lit); stddll_it_next(&lit))
      free_gc_buff_mess(*(gc_buff_mess**) stddll_it_val(&lit));  
    stddll_destruct(&conn->mess_queue);
    
    free(conn);
  } else {
    FL_MUTEX_drop(&glob_conns_lock);                                     /* UNLOCK CONNS TAB */
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION; 
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_disconnect: ret %d\n", ret));
  return ret;                                             /* return SP_disconnect return value */
}

int FL_join(mailbox mbox, const char *grp) {
  fl_conn  *conn;
  fl_group *group;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_join: mbox %d, group '%s'\n", mbox, grp));
  if ((conn = lock_conn(mbox)) != 0) {
    /* ACHTUNG! You may not join a group when you are already involved with that group */
    if ((group = get_group(conn, grp)) == 0) {
      ret = SP_join(mbox, grp);
      unlock_conn(conn);
      if (ret != 0) {
	if (ret == CONNECTION_CLOSED || ret == ILLEGAL_SESSION)
	  FL_disconnect(mbox);
	else if (ret != ILLEGAL_GROUP)
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: group %s: SP_join: unexpected error %d\n",
		       __FILE__, __LINE__, mbox, grp, ret);
      }
    } else {
      ret = ILLEGAL_GROUP;
      unlock_conn(conn);
      DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_GROUP('%s', %p) in mstate %d\n", grp, 
			   group, group == 0 ? -1 : group->mstate));
    }
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_join: ret %d\n", ret));
  return ret;
}

int FL_leave(mailbox mbox, const char *grp) {
  fl_conn  *conn;
  fl_group *group;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_leave: mbox %d, group '%s'\n", mbox, grp));
  if ((conn = lock_conn(mbox)) != 0) {
    /* ACHTUNG! If you are a joined member of a fl group only then you can leave it only once! */
    if ((group = get_group(conn, grp)) != 0 && group->mstate == JOINED) {
      group->mstate = LEAVING;
      ret = SP_leave(mbox, grp);
      unlock_conn(conn);
      if (ret != 0) {
	if (ret == CONNECTION_CLOSED || ret == ILLEGAL_SESSION)
	  FL_disconnect(mbox);
	else if (ret == ILLEGAL_GROUP)
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: group %s: SP_leave: ILLEGAL_GROUP\n", 
		       __FILE__, __LINE__, mbox, grp);
	else
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: group %s: SP_leave: unexpected error %d\n", 
		       __FILE__, __LINE__, mbox, grp, ret);
      }
    } else {
      ret = ILLEGAL_GROUP;
      unlock_conn(conn);
      DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_GROUP('%s', %p) in mstate %d\n", grp, 
			   group, group == 0 ? -1 : group->mstate));
    }
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_leave: ret %d\n", ret));
  return ret;
}

int FL_flush(mailbox mbox, const char *grp) {
  fl_conn *conn;
  fl_group *group;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_flush: mbox %d, group '%s'\n", mbox, grp));
  if ((conn = lock_conn(mbox)) != 0) {
    if ((group = get_group(conn, grp)) != 0)
      ret = FL_int_flush(conn, group);
    else {
      DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_GROUP('%s', 0)\n", grp));
      ret = ILLEGAL_GROUP; 
    } 
    unlock_conn(conn); 
    if (ret == CONNECTION_CLOSED || ret == ILLEGAL_SESSION)
      FL_disconnect(mbox);
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_flush: ret %d\n"));
  return ret;
}

int FL_scat_subgroupcast(mailbox mbox, service serv_type, const char *grp,
				int num_recvrs, char recvrs[][MAX_GROUP_NAME],
				int16 mess_type, const scatter *scat) {
  return FL_int_scat_multicast(mbox, serv_type | SUBGROUP_CAST, grp, 
			       num_recvrs, recvrs, mess_type, scat);
}

int FL_subgroupcast(mailbox mbox, service serv_type, const char *grp,
			   int num_recvrs, char recvrs[][MAX_GROUP_NAME],
			   int16 mess_type, int mess_len, const char *mess) {
  scatter msg;

  msg.num_elements    = 1;
  msg.elements[0].len = mess_len;
  msg.elements[0].buf = (char*) mess;

  return FL_scat_subgroupcast(mbox, serv_type, grp, num_recvrs, recvrs, mess_type, &msg);
}

int FL_scat_unicast(mailbox mbox, service serv_type, const char *grp, const char *recvr, 
			   int16 mess_type, const scatter *scat) {
  return FL_scat_subgroupcast(mbox, serv_type, grp, 1, (char(*)[MAX_GROUP_NAME]) recvr, 
			      mess_type, scat);
}

int FL_unicast(mailbox mbox, service serv_type, const char *grp, const char *recvr, 
		      int16 mess_type, int mess_len, const char *mess) {
  scatter msg;

  msg.num_elements    = 1;
  msg.elements[0].len = mess_len;
  msg.elements[0].buf = (char*) mess;

  return FL_scat_unicast(mbox, serv_type, grp, recvr, mess_type, &msg);
}

int FL_scat_multicast(mailbox mbox, service serv_type, const char *grp,
			     int16 mess_type, const scatter *scat) {
  return FL_int_scat_multicast(mbox, serv_type & ~SUBGROUP_CAST, grp, 0, 0, mess_type, scat);
}

int FL_multicast(mailbox mbox, service serv_type, const char *grp,
			int16 mess_type, int mess_len, const char *mess) {
  scatter msg;

  msg.num_elements    = 1;
  msg.elements[0].len = mess_len;
  msg.elements[0].buf = (char*) mess;

  return FL_scat_multicast(mbox, serv_type, grp, mess_type, &msg);
}

int FL_scat_receive(mailbox mbox, service *serv_type, char *sender, int max_groups, 
			   int *num_groups, char groups[][MAX_GROUP_NAME], int16 *mess_type,
			   int *endian_mismatch, scatter *scat_mess, int *more_messes) {
  int blocking = (*serv_type & DONT_BLOCK) == 0;              /* user not using DONT_BLOCK */ 
  gc_recv_mess msg;                                    /* used to contain user's parameters */
  fl_conn *conn;

  DEBUG(std_stkfprintf(stderr, 1, "FL_scat_receive: mbox %d, service 0x%X, max_groups %d\n", 
		       mbox, *serv_type, max_groups));

  if (max_groups < 0) {
    DEBUG(std_stkfprintf(stderr, 0, "Illegal max_groups!\n"));
    msg.ret = ILLEGAL_PARAM;
  } else {
    /* I use msg rather than the individual parameters to pass user's variables around */
    msg.mbox            = mbox;
    msg.orig_serv_type  = *serv_type;
    msg.serv_type       = serv_type;
    msg.sender          = sender;
    msg.max_groups      = max_groups;
    msg.num_groups      = num_groups;
    msg.groups          = groups;
    msg.mess_type       = mess_type;
    msg.endian_mismatch = endian_mismatch;
    msg.scat_mess       = scat_mess;
    msg.delivered       = 0;       /* does msg contain values to be returned to the user? */

    if ((conn = make_reservation(mbox)) != 0) {      /* get conn and make sure it stays valid */
      /* NEED TO ENFORCE THAT THERE IS ONLY ONE READING THREAD AT A TIME PER CONNECTION,      */
      /* BECAUSE A THREAD COULD BECOME BLOCKED IN SP_recv AND THEN MSGS ARE GENERATED BY      */
      /* ANOTHER THREAD AND PUT INTO THE QUEUE TO BE RETURNED -> THE BLOCKED THREAD           */
      /* WOULDN'T REALIZE IT! SERIALIZATION ON RECV_LOCK ACCOMPLISHES THIS!!!!                */
      if (acquire_recv_lock(conn)) {
	if (acquire_conn_lock(conn)) {     /* get conn lock for so I can read connection info */
	  int have_conn_lock = 1;                             /* do I have the conn lock? */
	  
	FL_scat_re_receive:
	  msg.vulnerable           = 0;           /* is the contained message vulnerable? */
	  msg.num_new_grps         = 0;              /* ensure new_grps and new_msg are empty */
	  msg.new_msg.num_elements = 0;       /* used for DROP_RECV msgs (see FL_int_receive) */
	  
	  if (stddll_empty(&conn->mess_queue)) {                           /* empty msg queue */
	    if (blocking || SP_poll(mbox) != 0) {       /* blocking allowed, or msg available */
	      have_conn_lock = recv_and_handle(conn, &msg);     /* call SP_recv and enter FSA */
	      
	      if (msg.num_new_grps != 0)                     /* reclaim any allocated buffers */
		free(msg.new_grps);
	      
	      if (msg.new_msg.num_elements != 0)
		free(msg.new_msg.elements[0].buf);
	      
	      if (!msg.delivered) {               /* if msg doesn't contain returnable values */
		DEBUG(std_stkfprintf(stderr, 0, "No return msg, going to top of recv loop!\n"));
		goto FL_scat_re_receive;                   /* need to try and get a msg again */
	      } else
		DEBUG(std_stkfprintf(stderr, 0, "Msg contains values for user!\n"));
	    } else { 
	      DEBUG(std_stkfprintf(stderr, 0, "Call to receive would have blocked!\n"));
	      msg.ret = WOULD_BLOCK;                                    /* would have blocked */
	    }
	  } else {                                                       /* message in queue! */
	    gc_buff_mess *mess;                                              /* get buff mess */
	    stdit lit;

	    DEBUG(std_stkfprintf(stderr, 0, "Msg in buffer, reading in to user's parameters\n"));
	    mess = *(gc_buff_mess**) stddll_it_val(stddll_begin(&conn->mess_queue, &lit));  
	    if (buffm_to_userm(&msg, mess)) {     /* msg copy to user's parameters successful */
	      stddll_erase(&conn->mess_queue, &lit);                     /* pop mess off of the top of the queue */
	      conn->bytes_queued -= mess->total_size;                  /* update bytes queued */
	      free_gc_buff_mess(mess);                            /* destroy the gc_buff_mess */
	    } else
	      DEBUG(std_stkfprintf(stderr, 0, "Msg incompatible with user's parameters!\n"));
	  }
	  if (have_conn_lock) {                             /* if I didn't lose the conn lock */
	    if (msg.ret >= 0) 
	      *more_messes = (int) stddll_size(&conn->mess_queue);      /* how many msgs left */
	    release_conn_lock(conn);                               /* let go of the conn lock */
	  }
	} else { 
	  DEBUG(std_stkfprintf(stderr, 0, "Couldn't acquire conn lock -> disconnecting!\n"));
	  msg.ret = ILLEGAL_SESSION;
	}
	release_recv_lock(conn);
      } else {
	DEBUG(std_stkfprintf(stderr, 0, "Couldn't acquire recv lock -> disconnecting!\n"));
	msg.ret = ILLEGAL_SESSION;
      }
      cancel_reservation(conn);
    } else {
      DEBUG(std_stkfprintf(stderr, 0, "Couldn't get a reservation\n"));
      msg.ret = ILLEGAL_SESSION;
    }
    DEBUG(                                        /* only execute if statment when DEBUGGING */
    if (msg.ret >= 0) {
      int i;
      
      std_stkfprintf(stderr, 0, "Groups(%d):\n", *num_groups);
      for (i = 0; i < *num_groups; ++i)
	std_stkfprintf(stderr, 0, "\t'%s'\n", groups[i]);
    }); 
    if (msg.ret == CONNECTION_CLOSED || msg.ret == ILLEGAL_SESSION)
      FL_disconnect(mbox);
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_scat_receive: ret %d, mbox %d, service 0x%X, sender '%s'"
		       ", num_groups %d, mess_type %d, endian %d, more_messes %d\n", 
		       msg.ret, mbox, *serv_type, msg.ret >= 0 ? sender : "", *num_groups, 
		       *mess_type, *endian_mismatch, *more_messes));
  return msg.ret;
}

int FL_receive(mailbox mbox, service *serv_type, char *sender, int max_groups, 
		      int *num_groups, char groups[][MAX_GROUP_NAME], int16 *mess_type, 
		      int *endian_mismatch, int max_mess_len, char *mess, int *more_messes) {
  scatter msg;

  msg.num_elements    = 1;
  msg.elements[0].len = max_mess_len;
  msg.elements[0].buf = (char*) mess;

  return FL_scat_receive(mbox, serv_type, sender, max_groups, num_groups, groups, 
			 mess_type, endian_mismatch, &msg, more_messes);
}

int FL_more_msgs(mailbox mbox) {
  fl_conn *conn;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_more_msgs: mbox %d\n", mbox));
  if ((conn = lock_conn(mbox)) != 0) {
    ret = (int) stddll_size(&conn->mess_queue);
    unlock_conn(conn);
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_more_msgs: ret %d\n", ret));
  return ret;
}

int FL_poll(mailbox mbox) {
  fl_conn *conn;
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_poll: mbox %d\n", mbox));
  if ((conn = lock_conn(mbox)) != 0) {
    ret = conn->bytes_queued + SP_poll(mbox);
    unlock_conn(conn);
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_poll: ret %d\n", ret));
  return ret;
}

void FL_error(int err) { 
  switch (err) {
  case ILLEGAL_PARAM:
    printf("FL_error: (%d) Illegal parameter (eg a negative size) passed to a function.\n", err);
    break;
  case WOULD_BLOCK:
    printf("FL_error: (%d) Function call would have blocked.\n", err);
    break;
  case ILLEGAL_MESSAGE_TYPE:
    printf("FL_error: (%d) Illegal message type (int16) used, "
	   "value < FL_MIN_LEGAL_MESS_TYPE(%d).\n", err, FL_MIN_LEGAL_MESS_TYPE);
    break;
  case ILLEGAL_STATE:
    printf("FL_error: (%d) Function call peformed in a prohibited state.\n", err);
    break;
  case ILLEGAL_RECEIVERS:
    printf("FL_error: (%d) Illegal receivers specified.\n", err);
    break;
  default:
    SP_error(err);
    break;
  }
}

int FL_get_gid_offset_memb_mess(void)
{
  return 0;
}

int FL_get_num_vs_offset_memb_mess(void)
{
  return sizeof(group_id);
}

int FL_get_vs_set_offset_memb_mess(void)
{
  return sizeof(group_id) + sizeof(int32);
}

/************************************ private interface ****************************************/

/* structure routines */
/* When I initially create a group structure I set it up to be ready to insert in to a groups  */
/* hash of a connection. I set mstate to JOINING indicating this group hasn't installed this   */
/* connection as a member yet. I set vstate to AGREE, which will trigger auto-flush oks from   */
/* this joining connection on SP memb events, until it finally gets installed. I create a      */
/* fl_view with only the connection in fl_view->membs. I do this for three reasons: (1)        */
/* sp_view and fl_view are always pointing at valid views now (no special checks for null),    */
/* (2) in case this connection is joined through a NETWORK event, it will report the proper    */
/* singleton vs set, and (3) this also gives me correct behavior on delivery of user's data    */
/* msgs (is sender in my current fl view?).                                                    */
static fl_group *create_fl_group(const char *conn_name, const char *group_name) {
  fl_group *group;
  group_id gid = { { 0 } };

  DEBUG(std_stkfprintf(stderr, 1, "create_fl_group: conn_name '%s', group '%s'\n", 
		       conn_name, group_name));

  if ((group = (fl_group*) calloc(1, sizeof(fl_group))) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): calloc(1, %u)\n", __FILE__, __LINE__, sizeof(fl_group));

  strncpy(group->group, group_name, MAX_GROUP_NAME);
  group->mstate = JOINING;                                          /* not a JOINED member yet */
  group->vstate = AGREE;        /* when I handle_next_memb_change first time -> sends FLUSH_OK */
  group->curr_change = 0;
  group->sp_view = group->fl_view = create_view(gid);            /* put an empty view to start */
  fill_view(group->fl_view, 0, 1, (char(*)[MAX_GROUP_NAME]) conn_name, 0);      /* insert self */
  group->fl_view->in_trans_memb = 1;         /* don't deliver any trans sigs until a member */
  group->flush_recvs = 0;
  stddll_construct(&group->mess_queue, sizeof(gc_buff_mess*));
  stddll_construct(&group->memb_queue, sizeof(sp_memb_change*));
  stdhash_construct(&group->pmemb_hash, sizeof(group_id), sizeof(sp_memb_change*),
		    group_id_cmp, group_id_hashcode, 0); /* use default key fcns for group_id */
  DEBUG(std_stkfprintf(stderr, -1, "create_fl_group: group('%s', %p)\n", group->group, group));
  return group;
}

static void free_fl_group(fl_group *group) {
  stdit lit;
  stdit hit;

  DEBUG(std_stkfprintf(stderr, 1, "free_fl_group: group('%s', %p)\n", group->group, group));
  /* sp view is either equal to fl_view or points in to one of the sp_memb_change's */
  free_view(group->fl_view);

  for (stddll_begin(&group->mess_queue, &lit); !stddll_is_end(&group->mess_queue, &lit); stddll_it_next(&lit))
    free_gc_buff_mess(*(gc_buff_mess**) stddll_it_val(&lit));
  stddll_destruct(&group->mess_queue);

  /* curr_change and all sp_memb_changes contained in memb_queue are also in pmemb */
  stddll_destruct(&group->memb_queue);

  for (stdhash_begin(&group->pmemb_hash, &hit); !stdhash_is_end(&group->pmemb_hash, &hit); stdhash_it_next(&hit))
    free_sp_memb_change(*(sp_memb_change**) stdhash_it_val(&hit));
  stdhash_destruct(&group->pmemb_hash);

  free(group);
  DEBUG(std_stkfprintf(stderr, -1, "free_fl_group\n"));
}
  
static void free_gc_buff_mess(gc_buff_mess *msg) {
  DEBUG(std_stkfprintf(stderr, 1, "free_gc_buff_mess: m %p, serv 0x%X\n", msg, msg->serv_type));
  if (msg->num_groups)
    free(msg->groups);
  
  if (msg->mess_len)
    free(msg->mess);
  
  free(msg);
  DEBUG(std_stkfprintf(stderr, -1, "free_gc_buff_mess\n"));
}

static sp_memb_change *create_sp_memb_change(group_id gid) {
  sp_memb_change *spc;

  DEBUG(std_stkfprintf(stderr, 1, "create_sp_memb_change: gid %d %d %d\n", 
		       gid.id[0], gid.id[1], gid.id[2]));

  if ((spc = (sp_memb_change*) calloc(1, sizeof(sp_memb_change))) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): calloc(1, %u)\n", __FILE__, __LINE__, sizeof(sp_memb_change));

  spc->memb_info       = create_view(gid);
  spc->memb_mess_recvd = 0;
  spc->memb_change_age = 0;
  spc->delta_member    = 0;
  stdhash_construct(&spc->flok_senders, MAX_GROUP_NAME, 0, 
		    group_name_cmp, group_name_hashcode, 0);
  spc->gid             = gid;

  DEBUG(std_stkfprintf(stderr, -1, "create_sp_memb_change: ret %p, gid %d %d %d\n", spc,
		       spc->memb_info->gid.id[0], spc->memb_info->gid.id[1], 
		       spc->memb_info->gid.id[2])); 
  return spc;
}

static void free_sp_memb_change(sp_memb_change *spc) {
  DEBUG(std_stkfprintf(stderr, 1, "free_sp_memb_change: spc %p\n", spc));

  if (spc->memb_info != 0)
    free_view(spc->memb_info);

  if (spc->delta_member != 0)
    free(spc->delta_member);

  stdhash_destruct(&spc->flok_senders);
  free(spc);
  DEBUG(std_stkfprintf(stderr, -1, "free_sp_memb_change\n"));
}

static view *create_view(group_id gid) {
  view *v;

  DEBUG(std_stkfprintf(stderr, 1, "create_view: gid %d %d %d\n", 
		       gid.id[0], gid.id[1], gid.id[2]));

  if ((v = (view*) calloc(1, sizeof(view))) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): calloc(1, %u)\n", sizeof(view));

  v->gid            = gid;
  v->memb_type      = 0;
  v->in_trans_memb  = 0;
  v->orig_num_membs = 0;
  v->membs_names    = 0;
  stdhash_construct(&v->orig_membs, sizeof(char*), 0, 
		    group_name_ptr_cmp, group_name_ptr_hashcode, 0);
  stdhash_construct(&v->curr_membs, sizeof(char*), 0, 
		    group_name_ptr_cmp, group_name_ptr_hashcode, 0);
  v->my_index       = -1;

  DEBUG(std_stkfprintf(stderr, -1, "create_view: ret %p\n", v));
  return v;
}

static void free_view(view *v) {
  DEBUG(std_stkfprintf(stderr, 1, "free_view: view %p, gid %d %d %d\n", v, v->gid.id[0],
		       v->gid.id[1], v->gid.id[2]));
  if (v->orig_num_membs != 0)
    free(v->membs_names);
  stdhash_destruct(&v->orig_membs);
  stdhash_destruct(&v->curr_membs);
  free(v);
  DEBUG(std_stkfprintf(stderr, -1, "free_view\n"));
}

static void fill_view(view *v, service memb_type, int num_membs, 
		      char (*membs)[MAX_GROUP_NAME], int16 index) {
  size_t byte_size = num_membs * MAX_GROUP_NAME;

  DEBUG(std_stkfprintf(stderr, 1, "fill_view: view %p, serv 0x%X, num_membs %d, index %d\n",
		       v, memb_type, num_membs, index));

  assert(num_membs > 0 && index >= 0 && index < num_membs && v->orig_num_membs == 0 &&
	 stdhash_empty(&v->orig_membs) && stdhash_empty(&v->orig_membs));

  v->memb_type = memb_type;
  v->orig_num_membs = num_membs;

  if ((v->membs_names = (char(*)[MAX_GROUP_NAME]) malloc(byte_size)) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc (%d)\n", __FILE__, __LINE__, byte_size);
  memcpy(v->membs_names, membs, byte_size);

  for (membs = v->membs_names; num_membs--; ++membs)        /* I set membs = v->membs_names!!! */
    stdhash_insert(&v->orig_membs, 0, &membs, 0), stdhash_insert(&v->curr_membs, 0, &membs, 0);

  v->my_index = index;
  DEBUG(std_stkfprintf(stderr, -1, "fill_view: view %p, serv 0x%X, num_membs %lu, index %d\n",
		       v, v->memb_type, stdhash_size(&v->orig_membs), v->my_index));
}

/* Looks up a fl_conn "named" mbox, gets an outstanding reservation on
   it and acquires its conn_lock. Returns the valid locked fl_conn* on
   success or 0 on failure -> ILLEGAL_SESSION.
*/
static fl_conn *lock_conn(mailbox mbox) {
  fl_conn *conn;

  if ((conn = make_reservation(mbox)) == 0)
    return 0;

  if (!acquire_conn_lock(conn)) /* error -> disconnecting */
    return cancel_reservation(conn), (fl_conn*) 0;

  return conn;
}

/* Unlock a connection successfully locked with lock_conn. */
static void unlock_conn(fl_conn *conn) {
  release_conn_lock(conn);
  cancel_reservation(conn);
}

/* Get a pointer to a fl_conn "named" mbox and "acquire an outstanding
   reservation" by increasing a reservation counter. A successful
   return *ONLY* entitles the caller to make subsequent calls to
   acquire/release_conn_lock and acquire/release_recv_lock on the
   returned fl_conn. Each successful call to make_reservation MUST be
   matched by a call to cancel_reservation on the returned
   fl_conn. This outstanding reservation only ensures that the
   returned fl_conn is valid between a successful call to
   make_reservation and the subsequent call to cancel_reservation on
   the returned fl_conn. If mbox is not a valid fl connection this fcn
   fails and returns 0. NOTE: it is imperative that this fcn acquires
   the reserve lock before releasing the glob_conns_lock.  
*/
static fl_conn *make_reservation(mailbox mbox) { 
  stdit hit;
  fl_conn *conn;

  FL_MUTEX_grab(&glob_conns_lock);                                 /* LOCK CONNS TAB */

  if (stdhash_is_end(&glob_conns, stdhash_find(&glob_conns, &hit, &mbox))) {    /* no such mbox */
    FL_MUTEX_drop(&glob_conns_lock);
    return (fl_conn*) 0;                                                        /* UNLOCK CONNS TAB */
  }

  conn = *(fl_conn**) stdhash_it_val(&hit);
  FL_MUTEX_grab(&conn->reserve_lock);                           /* LOCK RESERVE_LOCK */
  FL_MUTEX_drop(&glob_conns_lock);                             /* UNLOCK CONNS TAB */
  if (!conn->disconnecting)                                /* check if disconnecting */
    ++conn->reservations;                               /* increment reservation cnt */
  FL_MUTEX_drop(&conn->reserve_lock);                       /* UNLOCK RESERVE_LOCK */

  return !conn->disconnecting ? conn : 0;
}

/* Relinquish a reservation this thread owns on a connection. */
static void cancel_reservation(fl_conn *conn) {
  FL_MUTEX_grab(&conn->reserve_lock);                           /* LOCK RESERVE_LOCK */
  assert(conn->reservations != 0);
  if (--conn->reservations == 0 && conn->disconnecting) { /* decrement reservation cnt */
    FL_COND_wake_all(&conn->destroy_cond);               /* signal disconnector thread */
  }
  FL_MUTEX_drop(&conn->reserve_lock);                       /* UNLOCK RESERVE_LOCK */
}

/* Acquire a connection's conn lock. The caller must have an
   outstanding reservation on conn. Once this lock is acquired, a
   thread can freely examine and/or modify the data in conn as
   indicated in <fl_p.h>. Returns 0 on failure -> ILLEGAL_SESSION.
   Each successful call made by a thread on a particular conn must be
   matched with a call to release_conn_lock to allow that conn to be
   used by other threads. A thread may not make a second call to
   acquire_conn_lock after a successful return on a particular conn
   without an intervening call to release_conn_lock on that conn to
   prevent self-deadlocks.  
*/
static int acquire_conn_lock(fl_conn *conn) {
  FL_MUTEX_grab(&conn->conn_lock);                                 /* LOCK CONN_LOCK */
  if (conn->disconnecting) {                                /* conn is disconnecting */
    FL_MUTEX_drop(&conn->conn_lock);
    return 0;
  }

  return 1;
}

/* Release a connection's conn lock. The caller must have an
   oustanding reservation on release. The calling thread must also
   have made a successful call to acquire_conn_lock on release without
   any intervening calls to release_conn_lock on release.  
*/
static void release_conn_lock(fl_conn *release) {
  FL_MUTEX_drop(&release->conn_lock);                            /* UNLOCK CONN_LOCK */
}

/* Acquire a connection's recv lock. The caller must have an
   outstanding reservation on conn. Once this lock is acquired, a
   thread can proceed into the body of FL_scat_recv. Returns 0
   on failure -> ILLEGAL_SESSION. Each successful call made by a
   thread on a particular conn must be matched with a call to
   release_recv_lock to allow that conn to be used by other threads. A
   thread may not make a second call to acquire_recv_lock after a
   successful return on a particular conn without an intervening call
   to release_recv_lock on that conn to prevent self-deadlocks.  NOTE
   THAT THIS LOCK DOES NOT GIVE A THREAD ANY RIGHTS TO EXAMINE OR
   MODIFY _ANY_ OF THE CONNECTION'S DATA!! IT ONLY ALLOWS A THREAD TO
   ENTER THE BODY OF FL_scat_recv!!  
*/
static int acquire_recv_lock(fl_conn *conn) {
  FL_MUTEX_grab(&conn->recv_lock);           /* LOCK RECV_LOCK */
  if (conn->disconnecting) {                 /* conn is disconnecting */
    FL_MUTEX_drop(&conn->recv_lock);
    return 0;
  }

  return 1;
}

/* Release a connection's recv lock. The caller must have an
   oustanding reservation on release. The calling thread must have
   made a successful call to acquire_recv_lock on release without any
   intervening calls to release_recv_lock on release.  
*/
static void release_recv_lock(fl_conn *release) {
  FL_MUTEX_drop(&release->recv_lock);           /* UNLOCK RECV_LOCK */
}

static fl_group *add_group(fl_conn *conn, const char *group_name) {
  fl_group *group;
  char *group_name_ptr;

  DEBUG(std_stkfprintf(stderr, 1, "add_group: mbox(%d, %p), group '%s'\n", 
		       conn->mbox, conn, group_name));
  group = create_fl_group(conn->private, group_name);
  group_name_ptr = group->group;
  stdhash_insert(&conn->groups, 0, &group_name_ptr, &group);
  DEBUG(std_stkfprintf(stderr, -1, "add_group: return group('%s', %p)\n", group->group, group));
  return group;
}

static void remove_group(fl_conn *conn, fl_group *group) {
  stdit hit;
  char *group_name_ptr;

  DEBUG(std_stkfprintf(stderr, 1, "remove_group: mbox(%d, %p), group ('%s', %p)\n",
		       conn->mbox, conn, group->group, group));
  group_name_ptr = group->group;
  stdhash_find(&conn->groups, &hit, &group_name_ptr);
  assert(!stdhash_is_end(&conn->groups, &hit) && group == *(fl_group**) stdhash_it_val(&hit));
  stdhash_erase(&conn->groups, &hit);
  free_fl_group(group);
  DEBUG(std_stkfprintf(stderr, -1, "remove_group\n"));
}

static fl_group *get_group(fl_conn *conn, const char *grp) {
  stdit hit;

  return (!stdhash_is_end(&conn->groups, stdhash_find(&conn->groups, &hit, &grp)) ? 
	  *(fl_group**) stdhash_it_val(&hit) : 0);
}

/* Pass null for bm if the data to be delivered is already in um.                         */
/* Otherwise fill out a gc_buff_mess with the appropriate data and indicate whether       */
/* that buff mess is dynamically allocated or not. If it is, then if the message needs to */
/* be buffered, then bm itself is buffered. If the dynamically allocated message doesn't  */
/* need to be buffered, then the msg is reclaimed (freed) before returning to the caller. */
/* Returns a pointer to the gc_buff_mess that was buffered, null if none was buffered.    */
/* um can be null if you force buffering somehow.                                         */
static gc_buff_mess *deliver(fl_conn *conn, gc_recv_mess *um, gc_buff_mess *bm,
					int bm_alloced) {
  gc_buff_mess *ret;

  DEBUG(std_stkfprintf(stderr, 1, "deliver: mbox(%d, %p), um %p, bm %p, bm_alloced %d\n",
		       conn->mbox, conn, um, bm, (int) bm_alloced));
  if (!um->delivered && stddll_empty(&conn->mess_queue) && (bm == 0 || buffm_to_userm(um, bm))) {
    DEBUG(std_stkfprintf(stderr, 0, "able to deliver to user's parameters!\n"));
    um->delivered = 1;
    ret = 0;
    if (bm != 0 && bm_alloced) {
      DEBUG(std_stkfprintf(stderr, 0, "bm was alloc'ed -> freeing %p\n", bm));
      free_gc_buff_mess(bm);
    }
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "UNABLE to deliver to user's parameters: BUFFERING!\n"));
    if (bm == 0 || !bm_alloced) {                    /* if I need to alloc a gc_buff_mess */
      if ((ret = (gc_buff_mess*) malloc(sizeof(gc_buff_mess))) == 0)
	stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", __FILE__, __LINE__, sizeof(gc_buff_mess));

      if (bm == 0) {                          /* if data was contained in user parameters */
	DEBUG(std_stkfprintf(stderr, 0, "data was in user's parameters copying to buff mess\n"));
	userm_to_buffm(ret, um);
      } else {                                           /* else data was contained in bm */
	DEBUG(std_stkfprintf(stderr, 0, "data was in bm, copying in to buff mess\n"));
	*ret = *bm;
      }
    } else {                             /* buff mess was already alloc'ed and filled out */
      DEBUG(std_stkfprintf(stderr, 0, "data in bm, but alloc'ed: stealing bm for buffer\n"));
      ret = bm;
    }
    /* calculate total size of buffered message */
    ret->total_size = sizeof(gc_buff_mess) + ret->num_groups * MAX_GROUP_NAME + ret->mess_len;
    stddll_push_back(&conn->mess_queue, &ret);
    conn->bytes_queued += ret->total_size;
  }
  DEBUG(std_stkfprintf(stderr, -1, "deliver: ret buff mess %p\n", ret));
  return ret;
}

static gc_buff_mess *deliver_trans_sig(fl_conn *conn, gc_recv_mess *um, char *group) {
  gc_buff_mess trans_sig = { 0 }, *ret; /* sets everything to zero */

  DEBUG(std_stkfprintf(stderr, 1, "deliver_trans_sig: mbox(%d, %p), group '%s'\n",
		       conn->mbox, conn, group));
  trans_sig.mbox = conn->mbox;
  trans_sig.serv_type = TRANSITION_MESS;
  strncpy(trans_sig.sender, group, MAX_GROUP_NAME);
  ret = deliver(conn, um, &trans_sig, 0);

  DEBUG(std_stkfprintf(stderr, -1, "deliver_trans_sig: ret buff mess %p\n", ret));
  return ret;
}

static gc_buff_mess *deliver_flush_req(fl_conn *conn, gc_recv_mess *um, char *group) {
  gc_buff_mess flush_req = { 0 }, *ret; /* sets everything to zero */

  DEBUG(std_stkfprintf(stderr, 1, "deliver_flush_req: mbox(%d, %p), group '%s'\n",
		       conn->mbox, conn, group));
  flush_req.mbox = conn->mbox;
  flush_req.serv_type = FLUSH_REQ_MESS;
  strncpy(flush_req.sender, group, MAX_GROUP_NAME);
  ret = deliver(conn, um, &flush_req, 0);

  DEBUG(std_stkfprintf(stderr, -1, "deliver_flush_req: ret buff mess %p\n", ret));
  return ret;
}

/* copy info from buffered msg to a user's parameters and make it presentable to the user  */
/* return 1 on successful copy to user, 0 on failure (incompatible user parameters) */
static int buffm_to_userm(gc_recv_mess *um, const gc_buff_mess *bm) {
  int ret = 1;
  scatp pos;
  long err;

  DEBUG(std_stkfprintf(stderr, 1, "buffm_to_userm: um %p, bm %p, serv 0x%X, sender '%s'\n", 
		       um, bm, bm->serv_type, bm->sender));

  if (scatp_begin(&pos, um->scat_mess) == 0 &&
      (err = scatp_cpy2(&pos, bm->mess, bm->mess_len)) >= 0) {      /* user's scatter is legal */
    um->mbox       = bm->mbox;
    *um->serv_type = bm->serv_type;
    *um->mess_type = bm->mess_type;
    
    if (err == bm->mess_len && bm->num_groups <= um->max_groups) {        /* no buffer problem */
      *um->num_groups = bm->num_groups;
      strncpy(um->sender, bm->sender, MAX_GROUP_NAME);
      memcpy(um->groups, bm->groups, 
	     (*um->num_groups >= 0 ? *um->num_groups : um->max_groups) * MAX_GROUP_NAME);
      *um->endian_mismatch = bm->endian_mismatch;
      um->ret = bm->mess_len;
    } else {                                                                 /* buffer problem */
      DEBUG(std_stkfprintf(stderr, 0, "User buffer problem!\n"));
      if ((um->orig_serv_type & DROP_RECV) == 0) {            /* user didn't request DROP_RECV */
	DEBUG(std_stkfprintf(stderr, 0, "User didn't request DROP_RECV\n"));
	if (err != bm->mess_len) {
	  um->ret = BUFFER_TOO_SHORT;
	  *um->endian_mismatch = -bm->mess_len;
	} else
	  *um->endian_mismatch = 0;
	
	if (bm->num_groups > um->max_groups) {
	  um->ret = GROUPS_TOO_SHORT;
	  *um->num_groups = -bm->num_groups;
	} else
	  *um->num_groups = 0;

	ret = 0;                      /* error: buffer problem and DROP_RECV not requested */
      } else {                                                     /* user requested DROP_RECV */
	DEBUG(std_stkfprintf(stderr, 0, "User DID request DROP_RECV!\n"));
	if (err != bm->mess_len)
	  um->ret = BUFFER_TOO_SHORT;
	
	if (bm->num_groups > um->max_groups) {
	  um->ret = GROUPS_TOO_SHORT;
	  *um->num_groups = -bm->num_groups;
	} else
	  *um->num_groups = bm->num_groups;

	strncpy(um->sender, bm->sender, MAX_GROUP_NAME);
	memcpy(um->groups, bm->groups, 
	       (*um->num_groups >= 0 ? *um->num_groups : um->max_groups) * MAX_GROUP_NAME);
	*um->endian_mismatch = bm->endian_mismatch;
      }
    }
  } else {                                                /* error: user's scat msg is illegal */
    um->ret = ILLEGAL_MESSAGE;
    ret = 0;
  }
  DEBUG(std_stkfprintf(stderr, -1, "buffm_to_userm: um %p, bm %p, serv 0x%X, sender '%s'\n", 
		       um, bm, bm->serv_type, bm->sender));
  return ret;
}

/* copy info from a user's parameters to a buffered message */
static void userm_to_buffm(gc_buff_mess *bm, const gc_recv_mess *um) {
  bm->mbox = um->mbox;
  bm->serv_type = *um->serv_type;
  strncpy(bm->sender, um->sender, MAX_GROUP_NAME);
  bm->mess_type = *um->mess_type;
  bm->endian_mismatch = *um->endian_mismatch;

  DEBUG(std_stkfprintf(stderr, 1, "userm_to_buffm: ret %d, vuln %d, delivered %d, serv 0x%X, "
		       "sender '%s'\n", um->ret, (int) um->vulnerable, (int) um->delivered, 
		       *um->serv_type, um->sender));
  /* if um isn't an error without a msg body and groups */
  if (um->ret >= 0 || um->ret == GROUPS_TOO_SHORT || um->ret == BUFFER_TOO_SHORT) { 
    scatter *scat;
    size_t groups_size;
    scatp um_pos;
    char (*groups)[MAX_GROUP_NAME];
    long err;

    /* get groups info out of the msg: either in um->groups or um->new_grps */
    get_groups_info(um, &bm->num_groups, &groups);

    if ((groups_size = bm->num_groups * MAX_GROUP_NAME) != 0) {
      if ((bm->groups = (char(*)[MAX_GROUP_NAME]) malloc(groups_size)) == 0)
	stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", __FILE__, __LINE__, groups_size);    
      memcpy(bm->groups, groups, groups_size);
    } else
      bm->groups = 0;

    /* get scat info out of the msg: either in um->scat_mess or um->new_msg */
    get_scat_info(um, &bm->mess_len, &scat);
    if (bm->mess_len != 0) {
      if ((bm->mess = (char*) malloc(bm->mess_len)) == 0)
	stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%d)\n", __FILE__, __LINE__, bm->mess_len);

      err = scatp_begin(&um_pos, scat);
      assert(err == 0);
      err = scatp_cpy1(bm->mess, &um_pos, bm->mess_len);
      assert(err == bm->mess_len);    
    } else
      bm->mess = 0;
  } else {
    stderr_output(STDERR_ABORT, 0,"not sure about this path: if ever triggered think about it\n");
    DEBUG(std_stkfprintf(stderr, 0, "copying an error message to a buffm: ret %d\n", um->ret));
    bm->num_groups = 0;
    bm->groups     = 0;
    bm->mess_len   = um->ret;
    bm->mess       = 0;
  }
  DEBUG(std_stkfprintf(stderr, -1, "userm_to_buffm\n"));
}

static int FL_int_flush(fl_conn *conn, fl_group *group) {
  int ret;

  DEBUG(std_stkfprintf(stderr, 1, "FL_int_flush: mbox(%d, %p), group ('%s', %p)\n",
		       conn->mbox, conn, group->group, group));
  /* ACHTUNG! You must be a non-leaving, AUTHORIZE member of a fl group to flush it */
  if (group->mstate != LEAVING) {
    if (group->vstate == AUTHORIZE) {
      DEBUG(std_stkfprintf(stderr, 0, "Going to state AGREE: Actually sending "
			   "the FLUSH_OK_MESS!\n"));
      group->vstate = AGREE;
      ret = SP_multicast(conn->mbox, FIFO_MESS, group->group, FLUSH_OK_MESS, 
			 sizeof(group_id), (char*) &group->curr_change->memb_info->gid);

      if (ret != sizeof(group_id)) {
	if (ret != CONNECTION_CLOSED && ret != ILLEGAL_SESSION)
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: group %s: SP_multicast: unexpected error(%d)\n", 
		       __FILE__, __LINE__, conn->mbox, group->group, ret);
      } else 
	ret = 0;                                                     /* set to zero -> success */
    } else {
      DEBUG(std_stkfprintf(stderr, 0, "Group not in AUTHORIZE state!\n"));
      ret = ILLEGAL_STATE;
    }
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "Group is in LEAVING state!\n"));
    ret = ILLEGAL_GROUP;
  }
  DEBUG(std_stkfprintf(stderr, -1, "FL_int_flush: ret %d\n", ret));
  return ret;
}

static int FL_int_scat_multicast(mailbox mbox, service serv_type, const char *grp,
					int num_recvrs, char recvrs[][MAX_GROUP_NAME],
					int16 mess_type, const scatter *user_scat) {
  fl_conn *conn;
  fl_group *group = 0;
  int grp_not_priv;                                           /* is grp not a private group ? */
  int ret, i;

  int fix_scat = 0;                                           /* did I modify the user's scat? */
  scatter *scat = (scatter*) user_scat;                           /* get rid of const warnings */
  scat_element senders_elem = { 0 };           /* for copy of user's element I might overwrite */
  char copy_buf[sizeof(group_id) + sizeof(int16) + MAX_GROUP_NAME];           /* append buffer */
  char *curr_ptr = copy_buf, *key;                             /* current pos in append buffer */

  DEBUG( /* debug print out user's parameters */
	std_stkfprintf(stderr, 1, "FL_int_scat_multicast: mbox %d, serv 0x%X, grp '%s', "
		       "mess_type %d, mess_len %ld\n", mbox, serv_type, grp, 
		       mess_type, scat_capacity(scat));
	std_stkfprintf(stderr, 0, "Recvrs(%d):\n", num_recvrs);
	for (i = 0; i < num_recvrs; ++i) {
	  std_stkfprintf(stderr, 0, "\t'%s'\n", recvrs[i]);
	});

  if ((conn = lock_conn(mbox)) != 0) {                                      /* LOCK CONNECTION */
    if (num_recvrs < 0) {                                                       /* num_recvrs? */
      DEBUG(std_stkfprintf(stderr, 0, "Illegal num_recvrs %d!\n", num_recvrs));
      ret = ILLEGAL_PARAM;
    } else if (IS_ILLEGAL_SEND_STYPE(serv_type)) {          /* used reserved bits in serv_type */
      DEBUG(std_stkfprintf(stderr, 0, "Illegal use of reserved service bits\n"));
      ret = ILLEGAL_SERVICE;
    } else if (IS_ILLEGAL_SEND_MTYPE(mess_type)) {                /* used a reserved mess type */
      DEBUG(std_stkfprintf(stderr, 0, "Illegal use of reserved message type!\n"));
      ret = ILLEGAL_MESSAGE_TYPE;
    } else if (scat->num_elements > FL_MAX_SCATTER_ELEMENTS) {
      DEBUG(std_stkfprintf(stderr, 0, "Illegal scatter num_elements %d\n", scat->num_elements));
      ret = ILLEGAL_MESSAGE;
    } else if ((grp_not_priv = !is_private_group(grp)) &&   /* I allow sends to private groups */
	       ((group = get_group(conn, grp)) == 0 || group->mstate != JOINED)) {
      DEBUG(std_stkfprintf(stderr, 0, "Illegal send to group of which not a member!\n"));
      ret = ILLEGAL_GROUP;                                       /* not a member of that group */
    } else if (grp_not_priv && group->vstate == AGREE) {
      DEBUG(std_stkfprintf(stderr, 0, "Illegal send: state AGREE: group '%s'!\n", group->group));
      ret = ILLEGAL_STATE;
    } else {          /* attempt to send msg: still need to check on recvrs for subgroup_casts */
      if (grp_not_priv && is_vulnerable_mess(group, serv_type)) { /* mess type, current fl vid */
	DEBUG(std_stkfprintf(stderr, 0, "Sending a vulnerable message!\n"));
	fix_scat = 1;
	memcpy(curr_ptr, &mess_type, sizeof(int16));
	curr_ptr += sizeof(int16);
	memcpy(curr_ptr, &group->fl_view->gid, sizeof(group_id));
	curr_ptr += sizeof(group_id);
	mess_type = VULNERABLE_MESS;
      }
      if (!Is_subgroup_mess(serv_type)) {                                  /* normal multicast */
	if (fix_scat) {                   /* actually change the scatter to reflect appendings */
	  senders_elem = scat->elements[scat->num_elements];            /* save user's element */
	  scat->elements[scat->num_elements].buf = copy_buf;
	  scat->elements[scat->num_elements].len = (int) (curr_ptr - copy_buf);
	  ++scat->num_elements;
	}
	ret = SP_scat_multicast(mbox, serv_type, grp, mess_type, scat);
      } else {                                                           /* subgroup multicast */
	DEBUG(std_stkfprintf(stderr, 0, "Sending a subgroupcast!\n"));
	if (grp_not_priv) {             /* check all recvrs were members of my current fl view */
	  stdhash *orig_membs = &group->fl_view->orig_membs;
	  stdit find;
      
	  for (i = 0; i < num_recvrs; ++i) { 
	    key = (char*) (recvrs + i);
	    if (stdhash_is_end(orig_membs, stdhash_find(orig_membs, &find, &key))) {  /* a recvr wasn't */
	      DEBUG(std_stkfprintf(stderr, 0, "Illegal recvr '%s' not in ref '%s'\n", key, grp));
	      break;
	    }
	  }
	} else {                                        /* allow subgroupcast to private group */
	  for (i = 0; i < num_recvrs; ++i) { 	  /* require ALL "receivers" to be same as grp */
	    key = (char*) (recvrs + i);
	    if (strncmp(grp, key, MAX_GROUP_NAME) != 0) {
	      DEBUG(std_stkfprintf(stderr, 0, "Illegal recvr '%s' not in ref '%s'\n", key, grp));
	      break;
	    }
	  }
	}
	fix_scat = 1;                    /* subgroup msgs have reference group appended to msg */
	strncpy(curr_ptr, grp, MAX_GROUP_NAME);
	curr_ptr += MAX_GROUP_NAME;

	/* actually change the scatter to reflect appendings */
	senders_elem = scat->elements[scat->num_elements];              /* save user's element */
	scat->elements[scat->num_elements].buf = copy_buf;
	scat->elements[scat->num_elements].len = (int) (curr_ptr - copy_buf);
	++scat->num_elements;

	if (i == num_recvrs)                                    /* all recvrs were legal above */
	  ret = SP_multigroup_scat_multicast(mbox, serv_type, num_recvrs, 
					     (const char(*)[MAX_GROUP_NAME]) recvrs, 
					     mess_type, scat);
	else
	  ret = ILLEGAL_RECEIVERS;
      }
      if (fix_scat) {                       /* I modified user's scatter: I need to restore it */
	--scat->num_elements;
	if (ret >= 0) {          /* successful return: need to adjust err to ignore appendings */
	  if (ret >= (int) scat->elements[scat->num_elements].len)
	    ret -= scat->elements[scat->num_elements].len;
	  else
	    stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: serv 0x%X: group '%s': SP_multicast returned %d\n",
			 __FILE__, __LINE__, mbox, serv_type, grp, ret);
	}
	scat->elements[scat->num_elements] = senders_elem;            /* restore modified elem */
      }
    }
    unlock_conn(conn);                                                    /* UNLOCK CONNECTION */
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "FL ILLEGAL_SESSION(%d)\n", mbox));
    ret = ILLEGAL_SESSION; 
  }
  if (ret == CONNECTION_CLOSED || ret == ILLEGAL_SESSION)
    FL_disconnect(mbox);

  DEBUG(std_stkfprintf(stderr, -1, "FL_int_scat_multicast: ret %d, mbox %d\n", ret, mbox));
  return ret;
}

/* try to read in a msg from the SP layer into m and make it presentable to the user 
   returns non-zero if user's parameters shouldn't be processed, but, instead returned 
*/
static int FL_int_receive(gc_recv_mess *m) {
  int alloced_groups = 0;                             /* boolean - did I alloc groups buffers? */
  int alloced_buffer = 0;                           /* boolean - did I alloc a message buffer? */
  int max_groups;                                        /* max_groups to be passed to receive */
  int orig_max_groups = m->max_groups;                                    /* user's max_groups */
  char (*groups)[MAX_GROUP_NAME] = m->groups;                /* groups to be passed to receive */
  scatter *scat = m->scat_mess;                             /* scatter to be passed to receive */
  int success = 0;                                                /* boolean - did it succeed? */

  /* here I reserve space in groups for the destination group if it is a SUBGROUP_CAST msg */
  if (m->max_groups != 0)
    max_groups = m->max_groups - 1;
  else
    max_groups = 0;

  /* don't use DROP_RECV semantics: flush needs to see entire data of msg: then we can drop */
  *m->serv_type = (m->orig_serv_type & ~DROP_RECV);         

  DEBUG(std_stkfprintf(stderr, 1, "FL_int_receive: mbox %d, serv 0x%X, max groups %d, "
		       "grps %p, scat %p, scat_cap %ld\n", m->mbox, *m->serv_type, max_groups, 
		       groups, scat, scat_capacity(scat)));

  /* assert that there aren't any alloc'ed receive buffers, the user's 
     max_groups was legal and m doesn't already have a msg */
  assert(m->num_new_grps == 0 && m->new_msg.num_elements == 0 && 
	 orig_max_groups >= 0 && !m->delivered);

  /* perform the first receive */
  m->ret = SP_scat_receive(m->mbox, m->serv_type, m->sender, max_groups, m->num_groups,
			   groups, m->mess_type, m->endian_mismatch, scat);

  /* check for buffer errors: should be if buffers too short, I didn't use DROP_RECV */
  if (m->ret == GROUPS_TOO_SHORT || m->ret == BUFFER_TOO_SHORT) {
    DEBUG(std_stkfprintf(stderr, 0, "Buffer error after initial recv: ret %d, num_groups %d, "
			 "endian %d\n", m->ret, *m->num_groups, *m->endian_mismatch));

    /* if the groups buffer was too short and the message was a SUBGROUP_CAST then the groups
       buffer didn't have room for the destination group too: so decrement num_groups to reflect */
    if (*m->num_groups < 0 && Is_subgroup_mess(*m->serv_type)) {
      DEBUG(std_stkfprintf(stderr, 0, "GROUPS error, subgroup_mess: decrementing num_groups\n"));
      --*m->num_groups;
    }

    /* if the user requested DROP_RECV, or if the user's groups was exactly big enough (because 
       I shrunk max_groups above) and there was no msg buffer error (reported by endian_mismatch) 
       or I can't tell if there was a buffer error because 3.12 didn't report that properly then
       allocate the necessary buffers and re-receive - else return buffer error to user
    */
    if ((m->orig_serv_type & DROP_RECV) != 0 || 
	(-*m->num_groups <= orig_max_groups && 
	 (*m->endian_mismatch == 0 || FL_SP_version() == (float) 3.12))) {
      DEBUG(std_stkfprintf(stderr, 0, "Either DROP_RECV or might be able to re-receive!\n"));

      /* user's groups array actually was too small to contain the groups, so allocate some */
      if (-*m->num_groups > orig_max_groups) {         /* implies *m->num_groups is negative */
	size_t byte_size;

	max_groups = -*m->num_groups;                     /* set max_groups to be big enough */
	byte_size = max_groups * MAX_GROUP_NAME;          /* calculate necessary memory size */

	DEBUG(std_stkfprintf(stderr, 0, "User's GROUPS buff was actually TOO SMALL %d < %d\n", 
			     orig_max_groups, -*m->num_groups));

	alloced_groups  = 1;                         /* record that I alloc'ed group buffers */
	m->num_new_grps = max_groups;               /* record size of alloc'ed group buffers */

	/* actually allocate a new groups array to be used in re-receive */
	if ((groups = m->new_grps = (char(*)[MAX_GROUP_NAME]) malloc(byte_size)) == 0)
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", __FILE__, __LINE__, byte_size);
      } else {
	DEBUG(std_stkfprintf(stderr, 0, "User's GROUPS buff was big enough %d >= %d\n",
			     orig_max_groups, -*m->num_groups));
	max_groups = orig_max_groups;        /* groups will fit into user's original buffers */
      }
      
      /* now check and see if the user's msg buffer was too small: necessary size reported in
         endian_mismatch in versions later than 3.12, 3.12 didn't do it but did do DROP_RECV */
      if (*m->endian_mismatch < 0 || FL_SP_version() == (float) 3.12) {
	DEBUG(std_stkfprintf(stderr, 0, "Endian mismatch reports msg buffer is too small\n"));
	alloced_buffer = 1;                          /* record that I alloc'ed a mess buffer */
	scat = &m->new_msg;                         /* point scat at my alloc'ed mess buffer */
	scat->num_elements = 1;

	/* figure out how big the mess buffer needs to be */
	if (FL_SP_version() != (float) 3.12)
	  scat->elements[0].len = -*m->endian_mismatch;       /* capacity of msg to be recvd */
	else
	  scat->elements[0].len = 102400;      /* endian_mismatch is buggy, use max msg size */

	/* actually allocate the new mess buffer */
	if ((scat->elements[0].buf = (char*) malloc(scat->elements[0].len)) == 0)
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%d)\n", __FILE__, __LINE__, scat->elements[0].len);
      } else
	DEBUG(std_stkfprintf(stderr, 0, "No msg buffer error reported\n"));

      /* try to receive again (shouldn't fail now) and don't use DROP_RECV again */
      *m->serv_type = (m->orig_serv_type & ~DROP_RECV);
      DEBUG(std_stkfprintf(stderr, 0, "Re-receive: mbox %d, serv 0x%X, max groups %d, grps %p, "
			   "scat %p, scat_cap %ld\n", m->mbox, *m->serv_type, max_groups,
			   groups, scat, scat_capacity(scat)));

      /* perform the re-receive */
      m->ret = SP_scat_receive(m->mbox, m->serv_type, m->sender, max_groups, m->num_groups,
			       groups, m->mess_type, m->endian_mismatch, scat);

      /* if we had a buffer problem then code above or SP is buggy -> abort */
      if (m->ret == GROUPS_TOO_SHORT || m->ret == BUFFER_TOO_SHORT)
	stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: buggy SP_recv DROP_RECV ret %d: "
		     "max_groups %d: num_groups %d: scat_cap %ld: endian_mismatch %d\n", 
		     __FILE__, __LINE__, m->mbox, m->ret, max_groups, *m->num_groups, 
		     scat_capacity(scat), *m->endian_mismatch);
    }
  }
  /* receive or re-receive succeeded */
  if (m->ret >= 0) {
    int not_end = 1;                            /* boolean - did I not seek to end of msg yet? */
    scatp pos, user_pos;
    long err;

    /* successful recv: debug print out what I got! */
    DEBUG(std_stkfprintf(stderr, 0, "Successful recv: ret %d, serv 0x%X, sender '%s', "
			 "num_groups %d, mess_type %d, endian %d, Groups:\n", m->ret, 
			 *m->serv_type, m->sender, *m->num_groups, 
			 *m->mess_type, *m->endian_mismatch);
	  for (err = 0; err < *m->num_groups; ++err) {
	    std_stkfprintf(stderr, 0, "\t`%s'\n", groups[err]);
	  });

    /* subgroup and flush messages have appended data that needs to be extracted and removed */
    if (Is_regular_mess(*m->serv_type)) {
      if (Is_subgroup_mess(*m->serv_type)) {  /* subgroup msgs have destination group appended */
	char *ref_grp;

	assert(m->ret >= MAX_GROUP_NAME);                                /* ensure fully recvd */
	m->ret -= MAX_GROUP_NAME;                           /* correct to ignore the appendage */
	
	err = scatp_set(&pos, scat, m->ret, SEEK_SET);                /* seek to copy position */
	assert(err == 0);
	not_end = 0;                                       /* pos was seeked to new end of msg */
	
	/* copy out reference group: put after last used element in groups: should fit */
	ref_grp = (char*) (groups + *m->num_groups);
	err = scatp_cpy1(ref_grp, &pos, MAX_GROUP_NAME);
	assert(err == MAX_GROUP_NAME);
	++*m->num_groups;                 /* increment num_groups to include destination group */
	DEBUG(std_stkfprintf(stderr, 0, "Subgroupcast mess: Ref group is '%s'!\n", ref_grp));
      }

      /* all reserved FLUSH msg types have a group_id attached on end of msg */
      if (IS_ILLEGAL_SEND_MTYPE(*m->mess_type)) {
	DEBUG(std_stkfprintf(stderr, 0, "Recvd an internal flush mess of type %s\n", 
			     *m->mess_type == FLUSH_OK_MESS ? "FLUSH_OK_MESS" : 
			     (*m->mess_type == FLUSH_RECV_MESS ? "FLUSH_RECV_MESS" : 
			      (*m->mess_type == VULNERABLE_MESS ? "VULNERABLE_MESS" : 
			       "UNKNOWN TYPE!"))));
	assert(m->ret >= sizeof(group_id));                              /* ensure fully recvd */
	m->ret -= sizeof(group_id);                         /* correct to ignore the appendage */
	
	if (not_end) {
	  err = scatp_set(&pos, scat, m->ret, SEEK_SET);              /* seek to copy position */
	  assert(err == 0);
	  not_end = 0;                                                        /* seeked to end */
	} else {
	  err = scatp_jbackward(&pos, sizeof(group_id));                 /* move back from end */
	  assert(err == sizeof(group_id));
	}
	err = scatp_cpy1((char*) &m->dst_gid, &pos, sizeof(group_id));         /* copy out vid */
	assert(err == sizeof(group_id));

	/* endian correct the vid if necessary */
	if (*m->endian_mismatch != 0) {
	  stdflip32(m->dst_gid.id);
	  stdflip32(m->dst_gid.id + 1);
	  stdflip32(m->dst_gid.id + 2);
	}
	DEBUG(std_stkfprintf(stderr, 0, "Dst gid is %d %d %d!\n", m->dst_gid.id[0], 
			     m->dst_gid.id[1], m->dst_gid.id[2]));

	/* VULNERABLE msgs also have the original user's msg type appended */
	if (*m->mess_type == VULNERABLE_MESS) { 
	  assert(m->ret >= sizeof(int16));                               /* ensure fully recvd */
	  m->ret -= sizeof(int16);                          /* correct to ignore the appendage */
	  
	  m->vulnerable = 1;                                           /* mark m as vulnerable */

	  /* here should already be seeked to end */
	  err = scatp_jbackward(&pos, sizeof(int16));              /* move back from end again */
	  assert(err == sizeof(int16));	  
	  err = scatp_cpy1((char*) m->mess_type, &pos, sizeof(int16));    /* cpy out mess type */
	  assert(err == sizeof(int16));
	  
	  /* endian correct the message type if necessary */
	  if (*m->endian_mismatch != 0)
	    stdflip16(m->mess_type);

	  DEBUG(std_stkfprintf(stderr, 0, "VULNERABLE: Orig msg type is %d!\n", *m->mess_type));
	}
      }
    } else if (Is_reg_memb_mess(*m->serv_type)) {
      membership_info m_info;
      /* if its a regular membership message read the group id out of the msg */
      assert(m->ret >= sizeof(group_id) + sizeof(int) + MAX_GROUP_NAME);    /* ensure received */
      err = SP_scat_get_memb_info( scat, *m->serv_type, &m_info);

      memcpy(&m->dst_gid, &m_info.gid, sizeof(group_id) );
      assert(err == sizeof(group_id));                             /* already endian corrected */
      DEBUG(std_stkfprintf(stderr, 0, "Recvd a SP reg memb mess: New SP gid is %d %d %d\n", 
			   m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2]));
    }

    /* if I alloc'ed a msg buffer -> user probably requested DROP_RECV, need to copy from 
       alloc'ed buffers into user's buffers and set m->ret to an appropriate value */
    if (alloced_buffer) {
      m->new_msg.elements[0].len = m->ret;                          /* record size of user msg */
      err = scatp_begin(&user_pos, m->scat_mess);
      assert(err == 0);
      err = scatp_begin(&pos, &m->new_msg);
      assert(err == 0);

      /* perform copy from alloc'ed scat to user's scat */
      err = scatp_cpy0(&user_pos, &pos, m->ret); 
      assert(err >= 0 && err <= m->ret);                 /* shouldn't detect illegal scat here */

      /* possible that removing appended data made the user's msg buffer big enough */
      /* this also catches if I didn't really need to allocate a msg buffer (SP 3.12 bug) */
      if (err != m->ret) {                              /* didn't all fit into user's msg buff */
	DEBUG(std_stkfprintf(stderr, 0, "User msg buffer still too short!\n"));

	/* 3.12: if !DROP_RECV, then report negative msg size correctly in endian_mismatch */
	if ((m->orig_serv_type & DROP_RECV) == 0 && FL_SP_version() == (float) 3.12) {
	  DEBUG(std_stkfprintf(stderr, 0, "Alloced msg buff: SP 3.12 DROP_RECV bug: error!\n"));
	  success = -1;    /* non-zero success indicates an error that should be returned now! */
	  *m->endian_mismatch = -m->ret;
	}
	m->ret = BUFFER_TOO_SHORT;
      } else /* their buffer was big enough! */
	DEBUG(std_stkfprintf(stderr, 0, "Unnecessary msg buff alloc! No BUFFER_TOO_SHORT!\n"));
    }

    /* if I alloc'ed groups buffers then user's buffers were too small and requested DROP_RECV */
    if (alloced_groups) {
      DEBUG(std_stkfprintf(stderr, 0, "Alloced groups buff, copying over: GROUPS_TOO_SHORT\n"));
      memcpy(m->groups, m->new_grps, m->max_groups * MAX_GROUP_NAME);

      /* if it is a subgroup msg and there is space put the destination group at end of groups */
      if (Is_subgroup_mess(*m->serv_type) && m->max_groups > 0)
	memcpy(m->groups[m->max_groups - 1], m->new_grps[*m->num_groups - 1], MAX_GROUP_NAME);

      *m->num_groups = -*m->num_groups;                                    /* report too short */
      m->ret = GROUPS_TOO_SHORT;
    }
  } else {
    DEBUG(std_stkfprintf(stderr, 0, "Error from recv or re-recv that needs to be returned\n"));
    success = -1;     /* non-zero success indicates an error that should be given to user now! */
  }
  DEBUG(std_stkfprintf(stderr, -1, "SP_int_receive: success %d, ret %d, mbox %d, serv 0x%X, "
		       "mess_type %d\n", success, m->ret, m->mbox, *m->serv_type, 
		       *m->mess_type));
  return success;
}

/* state machine code */
/* recv_and_handle: main director function: returns whether or not still have conn lock */
static int recv_and_handle(fl_conn *conn, gc_recv_mess *m) { 
  fl_group *group;
  int ret = 1;                                           /* do I still have the conn lock? */

  DEBUG(std_stkfprintf(stderr, 1, "recv_and_handle: mbox(%d, %p)\n", conn->mbox, conn));
  release_conn_lock(conn);    /* let go of conn lock so I don't block sends on this connection */
  
  if (FL_int_receive(m) != 0) {           /* if an immediate return recv error: return to user */
    DEBUG(std_stkfprintf(stderr, 0, "immediate return from FL_int_receive %d\n", m->ret));
    m->delivered = 1;
    ret = 0;
  } else if (!acquire_conn_lock(conn)) {        /* couldn't acquire conn lock -> disconnecting */
    DEBUG(std_stkfprintf(stderr, 0, "Couldn't reacquire conn lock -> disconnecting\n"));
    m->ret = ILLEGAL_SESSION;
    m->delivered = 1;
    ret = 0;
  } else if (Is_regular_mess(*m->serv_type)) {                           /* SP regular message */
    char (*dst_grp)[MAX_GROUP_NAME];                     /* group this message is intended for */
    int num_groups;

    /* for regular msgs dst group is the first group, for subgroup msgs it is the last group */
    get_groups_info(m, &num_groups, &dst_grp);
    if (Is_subgroup_mess(*m->serv_type))
      dst_grp += num_groups - 1;
    
    DEBUG(std_stkfprintf(stderr, 0, "A regular mess destined for group '%s'\n", dst_grp));
    if ((group = get_group(conn, (char*) dst_grp)) != 0) {     /* look up fl_group structure */
      if (*m->mess_type == FLUSH_OK_MESS)
	handle_recv_flush_ok(conn, group, m);
      else if (*m->mess_type == FLUSH_RECV_MESS)
	handle_recv_flush_recv(conn, group, m);
      else
	handle_recv_reg_mess(conn, group, m);
    } else if (strncmp((char*) dst_grp, conn->private, MAX_GROUP_NAME) == 0) {  /* my pgroup */
      DEBUG(std_stkfprintf(stderr, 0, "Mess is addressed to my private group! Deliver!\n"));
      deliver(conn, m, 0, 0);                                 /* deliver msg immediately */
    } else
      stderr_output(STDERR_RETURN, 0,"(%s, %d): WARNING: a regular msg for a group of which I'm not a member!\n"
		 "mbox %d: serv 0x%X: sender '%s': dst_grp '%s': mess_type %d\n", __FILE__, 
		 __LINE__, m->mbox, *m->serv_type, m->sender, dst_grp, *m->mess_type);
  } else if (Is_membership_mess(*m->serv_type)) {                   /* SP membership message */
    group = get_group(conn, m->sender);       /* look up fl_group structure: might not exist */

    DEBUG(std_stkfprintf(stderr, 0, "A memb mess destined for group ('%s', %p)\n", 
			 m->sender, group));
    if (Is_reg_memb_mess(*m->serv_type))
      handle_recv_reg_memb_mess(conn, group, m);
    else if (Is_transition_mess(*m->serv_type))
      handle_recv_trans_memb_mess(conn, group, m);
    else if (Is_caused_leave_mess(*m->serv_type))
      handle_recv_self_leave_memb_mess(conn, group, m);
    else
      stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: recv_and_handle: unexpected membership 0x%X: "
		   "group '%s'\n", __FILE__, __LINE__, conn->mbox, *m->serv_type, m->sender);
  } else
    stderr_output(STDERR_ABORT, 0,"(%s, %d): mbox %d: recv_and_handle: unexpected service 0x%X: "
		 "sender '%s'\n", __FILE__, __LINE__, conn->mbox, *m->serv_type, m->sender);

  DEBUG(std_stkfprintf(stderr, -1, "recv_and_handle: ret %d (have_conn_lock)\n", (int) ret));
  return ret;
}

static void handle_recv_flush_ok(fl_conn *conn, fl_group *group, gc_recv_mess *m) {
  sp_memb_change *spc;
  stdit hit;
  stdit lit;

  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_flush_ok: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));

  /* see if I know about this flok's gid yet or not: if not create a sp_memb_change */
  if (!stdhash_is_end(&group->pmemb_hash, stdhash_find(&group->pmemb_hash, &hit, &m->dst_gid))) {
    spc = *(sp_memb_change**) stdhash_it_val(&hit);
    DEBUG(std_stkfprintf(stderr, 0, "Recvd a flok for a known gid %d %d %d: spc %p\n", 
			 m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2], spc));
    assert(SP_equal_group_ids(m->dst_gid, spc->memb_info->gid));
  } else {
    spc = create_sp_memb_change(m->dst_gid);
    DEBUG(std_stkfprintf(stderr, 0, "Recvd a flok for an UNKNWON gid %d %d %d: CREATED spc %p\n",
			 m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2], spc));
    stdhash_insert(&group->pmemb_hash, 0, &m->dst_gid, &spc);           /* put into pmemb_hash */
  }
  stdhash_insert(&spc->flok_senders, 0, m->sender, 0);              /* record flok from sender */
  DEBUG(std_stkfprintf(stderr, 0, "curr_change(%p) ?= spc(%p) now has %lu floks, needs %d\n", 
		       group->curr_change, spc, stdhash_size(&spc->flok_senders), 
		       spc->memb_mess_recvd ? spc->memb_info->orig_num_membs : -1));
  switch (group->vstate) {
  case AGREE:
    assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
    if (spc == group->curr_change &&
	stdhash_size(&spc->flok_senders) == spc->memb_info->orig_num_membs) {
      DEBUG(std_stkfprintf(stderr, 0, "Recvd last needed flok for curr change! INSTALL!\n"));
      /* Received all the floks necessary to install the next fl view! */
      install_new_view(conn, group, m);            /* deliver an appropriate fl membership msg */

      DEBUG(std_stkfprintf(stderr, 0, "Going to state VERIFY!\n"));
      group->vstate = VERIFY;                                            /* update group state */
      if (group->mstate == JOINING)                                  
	group->mstate = JOINED;                               /* has been installed in a group */

      update_fl_view(group);                     /* update the view information for this group */

      /* if no cascading membership: send a FLUSH_RECV, else deliver a FLUSH_REQ mess */
      if (stddll_empty(&group->memb_queue)) {
	int err;

	DEBUG(std_stkfprintf(stderr, 0, "No cascading memberships -> sending FLUSH_RECV\n"));
	err = SP_multicast(conn->mbox, FIFO_MESS, group->group, FLUSH_RECV_MESS,
			   sizeof(group_id), (char*) &group->fl_view->gid);

	if (err == CONNECTION_CLOSED || err == ILLEGAL_SESSION) {    /* immediate return error */
	  DEBUG(std_stkfprintf(stderr, 0, "SP_multicast failure %ld\n", err));
	  m->delivered = 1;                                   /* return to user immediately */
	  m->ret = (int) err;
	  break;
	} else if (err != sizeof(group_id))
	  stderr_output(STDERR_ABORT, 0,"(%s, %d): SP_multicast unexpected return %d\n", __FILE__, __LINE__, err);
      } else {                            /* buffer a FLUSH_REQ mess, update curr_change, etc. */
	DEBUG(std_stkfprintf(stderr, 0, "Cascading Memberships(%lu) to handle!\n", 
			     stddll_size(&group->memb_queue)));
	handle_next_memb_change(conn, group, m);
      }
      /* if new members have already died (after they sent their floks): deliver TRANS */
      if ((int) stdhash_size(&group->fl_view->curr_membs) < group->fl_view->orig_num_membs) {
	DEBUG(std_stkfprintf(stderr, 0, "New view members have already died! Deliver TRANS!\n"));
	assert(!group->fl_view->in_trans_memb);
	group->fl_view->in_trans_memb = 1;
	deliver_trans_sig(conn, m, group->group);                               /* will buffer */
      }
      /* deliver any VULNERABLE messages that were pre-delivered */
      stddll_begin(&group->mess_queue, &lit);
      for (; !stddll_is_end(&group->mess_queue, &lit); stddll_it_next(&lit)) {
	DEBUG(std_stkfprintf(stderr, 0, "Delivering a vulnerable SP pre-delivered msg!\n"));
	deliver(conn, m, *(gc_buff_mess**) stddll_it_val(&lit), 1);          /* will buffer */
      }
      stddll_clear(&group->mess_queue);                    /* empty postponed vulnerable queue */
    } else
      assert(!spc->memb_mess_recvd || 
	     (int) stdhash_size(&spc->flok_senders) < spc->memb_info->orig_num_membs);
    break;
  case AUTHORIZE: 
    assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
    assert(!spc->memb_mess_recvd || 
	   (int) stdhash_size(&spc->flok_senders) < spc->memb_info->orig_num_membs);
    break;
  case STEADY: case VERIFY: 
    assert(group->curr_change == 0);
    assert(!spc->memb_mess_recvd || 
	   (int) stdhash_size(&spc->flok_senders) < spc->memb_info->orig_num_membs);
    break;
  default: stderr_output(STDERR_ABORT, 0,"(%s, %d): impossible vstate %d\n", __FILE__, __LINE__, group->vstate);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_flush_ok: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void handle_recv_flush_recv(fl_conn *conn, fl_group *group, gc_recv_mess *m) {
  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_flush_recv: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
  switch (group->vstate) {
  case VERIFY: 
    assert(group->curr_change == 0);
    if (SP_equal_group_ids(m->dst_gid, group->fl_view->gid)) {
      DEBUG(std_stkfprintf(stderr, 0, "Got a FLUSH_RECV for curr FL vid need %d flush_recvs!\n", 
			   group->fl_view->orig_num_membs));
      if (++group->flush_recvs == group->fl_view->orig_num_membs) {
	DEBUG(std_stkfprintf(stderr, 0, "Got last FLUSH_RECV: going to state STEADY!\n"));
	group->vstate = STEADY;
      }
    } else
      DEBUG(std_stkfprintf(stderr, 0, "Ignoring FLUSH_RECV for vid %d %d %d while in VERIFY!\n",
			   m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2]));
    break;
  case AGREE:
    assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
    if (SP_equal_group_ids(m->dst_gid, group->curr_change->memb_info->gid)) {
      DEBUG(std_stkfprintf(stderr, 0, "Got a FLUSH_RECV for CURR CHANGE need %d flush_recvs!\n", 
			   group->fl_view->orig_num_membs));
      ++group->flush_recvs;
      assert(group->flush_recvs < group->curr_change->memb_info->orig_num_membs);
    } else 
      DEBUG(std_stkfprintf(stderr, 0, "Ignoring FLUSH_RECV for vid %d %d %d while in AGREE!\n",
			   m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2]));
    break;
  case STEADY: 
    assert(group->curr_change == 0);
    DEBUG(std_stkfprintf(stderr, 0, "Ignoring FLUSH_RECV for vid %d %d %d while in STEADY!\n",
			 m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2]));
    break;
  case AUTHORIZE: 
    assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
    DEBUG(std_stkfprintf(stderr, 0, "Ignoring FLUSH_RECV for vid %d %d %d while in AUTHORIZE!\n",
			 m->dst_gid.id[0], m->dst_gid.id[1], m->dst_gid.id[2]));
    break;
  default: stderr_output(STDERR_ABORT, 0,"(%s, %d): impossible vstate %d\n", __FILE__, __LINE__, group->vstate);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_flush_recv: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void handle_recv_reg_mess(fl_conn *conn, fl_group *group, gc_recv_mess *um) {
  stdit hit;
  gc_buff_mess *bm;

  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_reg_mess: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
  switch (group->vstate) {
  case AGREE:
    assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
    /* if vulnerable message meant for the next membership from a member of the next one */
    if (um->vulnerable) {
      DEBUG(std_stkfprintf(stderr, 0, "Recvd a vulnerable msg!\n"));
      if (SP_equal_group_ids(um->dst_gid, group->curr_change->memb_info->gid)) {
	DEBUG(std_stkfprintf(stderr, 0, "Addressed to my next FL vid!\n"));
	if (!stdhash_is_end(&group->curr_change->memb_info->curr_membs, stdhash_find(&group->curr_change->memb_info->curr_membs, 
					    &hit, &um->sender))) {
	  DEBUG(std_stkfprintf(stderr, 0, "Recvd a vuln msg that needs to be POSTPONED!\n"));
	  if ((bm = (gc_buff_mess*) malloc(sizeof(gc_buff_mess))) == 0)
	    stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", __FILE__, __LINE__, sizeof(gc_buff_mess));
	  
	  userm_to_buffm(bm, um);
	  stddll_push_back(&group->mess_queue, &bm);
	  break;
	} else
	  stderr_output(STDERR_ABORT, 0,"'%s' Not in my next FL curr membs!\n", um->sender);
      } else
	DEBUG(std_stkfprintf(stderr, 0, "Vuln msg not to next FL vid: should ignore below\n"));
    } /* ELSE FALL THROUGH TO NORMAL CASES, BELOW (other case statements)!!!! */
  case STEADY: case AUTHORIZE: case VERIFY:
    /* if the message is from a current flush member then deliver it */
    if (!stdhash_is_end(&group->fl_view->curr_membs, stdhash_find(&group->fl_view->curr_membs, &hit, &um->sender))) {
      DEBUG(std_stkfprintf(stderr, 0, "Deliver reg mess from group memb '%s'\n", um->sender));
      deliver(conn, um, 0, 0);
    } else
      DEBUG(std_stkfprintf(stderr, 0, "Ignore reg mess from non group memb '%s'\n", um->sender));
    break;
  default: stderr_output(STDERR_ABORT, 0,"(%s, %d): impossible vstate %d\n", __FILE__, __LINE__, group->vstate);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_reg_mess: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void 
handle_recv_reg_memb_mess(fl_conn *conn, fl_group *group, gc_recv_mess *m) {
  sp_memb_change *spc, *new_curr_change;
  stdhash leavers;               /* leavers<char*, nil>: members that left since last SP memb  */
  stdit hit, flit;
  stdit lit;
  char (*groups)[MAX_GROUP_NAME];
  int num_groups;
  int fl_memb_shrunk = 0;

  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_reg_memb_mess: mbox(%d, %p), group('%s', %p), %s"
		       ", gid %d %d %d\n", conn->mbox, conn, m->sender, group, 
		       group != 0 ? state_str(group->vstate) : "NO STATE", m->dst_gid.id[0],
		       m->dst_gid.id[1], m->dst_gid.id[2]));

  if (group == 0) {     /* no fl_group struct for this group yet -> create one and put in conn */
    group = add_group(conn, m->sender);
    DEBUG(std_stkfprintf(stderr, 0, "No group structure yet, CREATED group %p!\n", group));
  }
  /* if a sp_memb_change for this gid doesn't exist yet: create one and put in pmemb */
  if (stdhash_is_end(&group->pmemb_hash, stdhash_find(&group->pmemb_hash, &hit, &m->dst_gid))) {
    spc = create_sp_memb_change(m->dst_gid);
    DEBUG(std_stkfprintf(stderr, 0, "Unknown gid: CREATED spc %p\n", spc));
    stdhash_insert(&group->pmemb_hash, 0, &m->dst_gid, &spc);
  } else {
    spc = *(sp_memb_change**) stdhash_it_val(&hit);
    DEBUG(std_stkfprintf(stderr, 0, "KNOWN gid: got from pmemb_hash %p\n", spc));
  }
  get_groups_info(m, &num_groups, &groups);                    /* fill in rest of data for spc */
  assert(!spc->memb_mess_recvd);
  spc->memb_mess_recvd = 1;                             /* I got the memb mess for this gid */
  spc->delta_member = determine_leavers(&leavers, group->sp_view, m); /* who left from last SP */
  fill_view(spc->memb_info, *m->serv_type, num_groups, groups, *m->mess_type);     /* spc view */

  /* update the current fl_view: shrink the fl membership by removing any leavers */
  for (stdhash_begin(&leavers, &hit); !stdhash_is_end(&leavers, &hit); stdhash_it_next(&hit)) {
    if (!stdhash_is_end(&group->fl_view->curr_membs, stdhash_find(&group->fl_view->curr_membs, 
					&flit, stdhash_it_key(&hit)))) {
      DEBUG(std_stkfprintf(stderr, 0, "FL member '%s' left in memb event, remove from fl view\n",
			   *(char**) stdhash_it_key(&hit)));
      fl_memb_shrunk = 1;                                 /* remember the membership shrunk */
      stdhash_erase(&group->fl_view->curr_membs, &flit);                              /* remove from fl_view->curr_membs */
    }
  }
  /* remove any memberships that this SP memb change invalidated */
  age_and_invalidate_pmembs(group);            /* age unknown spc's and remove them if too old */
  new_curr_change = collapse_memberships(group, &leavers, spc);   /* call before pushing spc!! */
  stdhash_destruct(&leavers);       /* after collapse members, leavers values could be invalid */

  /* push spc on to end of memb queue and point sp view at spc's view */
  /* spc must be pushed on to end of memb_queue after collapse memberships due to leave prob   */
  stddll_push_back(&group->memb_queue, &spc); /* push this SP memb change on to the memb queue */
  group->sp_view = spc->memb_info;                    /* sp_view refers to most recent SP view */

  /* if I wasn't working on a SP memb change or I just invalidated it -> update curr_change */
  /* this will deliver a FLUSH_REQ message if I wasn't already working on a change */
  if (group->curr_change != new_curr_change) {
    DEBUG(std_stkfprintf(stderr, 0, "New SP memb change to work on now: %p -> %p!!\n", 
			 group->curr_change, new_curr_change));

    /* drop any pre-delivered messages as they must have come from partioned members */
    for (stddll_begin(&group->mess_queue, &lit); !stddll_is_end(&group->mess_queue, &lit); stddll_it_next(&lit)) {
      DEBUG(std_stkfprintf(stderr, 0, "Destroying pre-delivered msg from leaver member\n"));
      free_gc_buff_mess(*(gc_buff_mess**) stddll_it_val(&lit));
    }
    stddll_clear(&group->mess_queue);
    handle_next_memb_change(conn, group, m);                   /* possibly deliver a FLUSH_REQ */
  }
  /* if the group shrunk and I haven't delivered a trans yet -> deliver a trans signal */
  if (!group->fl_view->in_trans_memb && fl_memb_shrunk) {
    DEBUG(std_stkfprintf(stderr, 0, "FL memb shrunk, haven't delivered TRANS: deliver TRANS\n"));
    group->fl_view->in_trans_memb = 1;
    deliver_trans_sig(conn, m, group->group);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_reg_memb mess mbox(%d, %p), group('%s', %p), "
		       "%s\n", conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void handle_next_memb_change(fl_conn *conn, fl_group *group, gc_recv_mess *m) {
  stdit lit;
  int err;

  DEBUG(std_stkfprintf(stderr, 1, "handle_next_memb_change: mbox(%d, %p), group('%s', %p), "
		       "%s\n", conn->mbox, conn, group->group, group, state_str(group->vstate)));

  assert(!stddll_empty(&group->memb_queue));
  /* special case for JOINING: don't have a curr change yet! */
  assert((group->vstate == STEADY || group->vstate == VERIFY) ? 
	 group->curr_change == 0 : group->mstate == JOINING || group->curr_change != 0);

  group->curr_change = *(sp_memb_change**) stddll_it_val(stddll_begin(&group->memb_queue, &lit));
  assert(group->curr_change != 0 && group->curr_change->memb_mess_recvd);
  group->flush_recvs = 0;

  switch (group->vstate) {
  case STEADY: case VERIFY:
    DEBUG(std_stkfprintf(stderr, 0, "Going to state AUTHORIZE!\n"));
    group->vstate = AUTHORIZE;
    deliver_flush_req(conn, m, group->group);
    break;
  case AUTHORIZE:            /* already waiting for a response from user */
    break;
  case AGREE:
    DEBUG(std_stkfprintf(stderr, 0, "Going to state AUTHORIZE!\n"));
    group->vstate = AUTHORIZE;
    DEBUG(std_stkfprintf(stderr, 0, "AGREE: already auth'ed memb, send flok to new gid!\n"));
    if ((err = FL_int_flush(conn, group)) != 0) { /* unrecoverable error */
      DEBUG(std_stkfprintf(stderr, 0, "FL_int flush error: return to user\n"));
      m->ret = err;
      m->delivered = 1;
    }
    break;
  default: stderr_output(STDERR_ABORT, 0,"(%s, %d): impossible vstate %d\n", __FILE__, __LINE__, group->vstate);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_next_memb_change: mbox(%d, %p), group('%s', %p), "
		       "%s\n", conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void 
handle_recv_trans_memb_mess(fl_conn *conn, fl_group *group, gc_recv_mess *m) {    
  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_trans_memb_mess: mbox(%d, %p), group('%s', %p), "
		       "%s\n", conn->mbox, conn, group->group, group, state_str(group->vstate)));
  group->sp_view->in_trans_memb = 1;
  if (!group->fl_view->in_trans_memb) {
    DEBUG(std_stkfprintf(stderr, 0, "Got a TRANS for first time in FL view: deliver TRANS\n"));
    group->fl_view->in_trans_memb = 1;
    deliver_trans_sig(conn, m, group->group);
  }
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_trans_memb_mess: mbox(%d, %p), group('%s', %p), "
		       "%s\n", conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void 
handle_recv_self_leave_memb_mess(fl_conn *conn, fl_group *group, gc_recv_mess *m){
  DEBUG(std_stkfprintf(stderr, 1, "handle_recv_self_leave_memb_mess: mbox(%d, %p), "
		       "group('%s', %p), %s\n", conn->mbox, conn, group->group, group, 
		       state_str(group->vstate)));
  remove_group(conn, group);
  deliver(conn, m, 0, 0);
  DEBUG(std_stkfprintf(stderr, -1, "handle_recv_self_leave_memb_mess: mbox(%d, %p)\n",
		       conn->mbox, conn));
}

/* install the current view I was trying to finish */
static void install_new_view(fl_conn *conn, const fl_group *group, gc_recv_mess *m) {
  view *new_fl_view = group->curr_change->memb_info, *last_fl_view = group->fl_view;
  int not_network = !Is_caused_network_mess(new_fl_view->memb_type);
  char (*curr_vs)[MAX_GROUP_NAME];
  gc_buff_mess *memb_mess;
  size_t size, byte_size;

  DEBUG(std_stkfprintf(stderr, 1, "install_new_view: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));

  /* I create a gc_buff_mess and then deliver it to the user for to simplify msg construction */
  if ((memb_mess = (gc_buff_mess*) malloc(sizeof(gc_buff_mess))) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", sizeof(gc_buff_mess));
      
  memb_mess->mbox = conn->mbox;              /* set mbox, service type, group, and num_groups */
  memb_mess->serv_type = new_fl_view->memb_type;
  assert(Is_reg_memb_mess(memb_mess->serv_type));
  strncpy(memb_mess->sender, group->group, MAX_GROUP_NAME);
  memb_mess->num_groups = new_fl_view->orig_num_membs;
      
  byte_size = memb_mess->num_groups * MAX_GROUP_NAME;             /* fill in the groups array */
  if ((memb_mess->groups = (char(*)[MAX_GROUP_NAME]) malloc(byte_size)) == 0)
    stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", byte_size);
  memcpy(memb_mess->groups, new_fl_view->membs_names, byte_size);

  memb_mess->mess_type = new_fl_view->my_index;           /* set mess_type and endian_mismatch */
  memb_mess->endian_mismatch = 0;

  if (not_network) {                                       /* set the mess_len (return val) */
    size = 1;
    memb_mess->mess_len = sizeof(group_id) + sizeof(int) + MAX_GROUP_NAME;
  } else {
    size = stdhash_size(&last_fl_view->curr_membs);
    byte_size = size * MAX_GROUP_NAME;
    memb_mess->mess_len = sizeof(group_id) + sizeof(int) + byte_size;
  }
  if ((memb_mess->mess = (char*) malloc(memb_mess->mess_len)) == 0) /* allocate body of msg */
    stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%u)\n", byte_size);
      
  memcpy(memb_mess->mess, &new_fl_view->gid, sizeof(group_id));   /* fill in body: gid, num_vs */
  memcpy(memb_mess->mess + sizeof(group_id), &size, sizeof(int));
  curr_vs = (char(*)[MAX_GROUP_NAME]) (memb_mess->mess + sizeof(group_id) + sizeof(int));

  /* fill in body of msg: vs_membs */
  if (not_network)
    memcpy(curr_vs, group->curr_change->delta_member, MAX_GROUP_NAME);    
  else {
    char (*old_grps)[MAX_GROUP_NAME]     = last_fl_view->membs_names;
    char (*old_grps_end)[MAX_GROUP_NAME] = old_grps + last_fl_view->orig_num_membs;
    stdit hit;
    
    /* Step through the last fl_view's membs_names checking to see if each name is still */
    /* in the membs (vs) set. This ensures the vs set is deterministically ordered. */
    for (; old_grps != old_grps_end; ++old_grps)
      if (!stdhash_is_end(&last_fl_view->curr_membs, stdhash_find(&last_fl_view->curr_membs, &hit, &old_grps)))
	memcpy(curr_vs++, old_grps, MAX_GROUP_NAME);
  }
  deliver(conn, m, memb_mess, 1);
  DEBUG(std_stkfprintf(stderr, -1, "install_new_view: mbox(%d, %p), group('%s', %p), %s\n",
		       conn->mbox, conn, group->group, group, state_str(group->vstate)));
}

static void update_fl_view(fl_group *group) {
  stdit hit;
  stdit lit;

  DEBUG(std_stkfprintf(stderr, 1, "update_fl_view: group('%s', %p), %s, old gid %d %d %d\n",
		       group->group, group, state_str(group->vstate), group->fl_view->gid.id[0],
		       group->fl_view->gid.id[1], group->fl_view->gid.id[2]));
  free_view(group->fl_view);                                         /* delete old fl_view */
  group->fl_view = group->curr_change->memb_info;              /* steal curr_change's view */
  group->curr_change->memb_info = 0;                  /* don't let fl_view be free'd below */

  stddll_begin(&group->memb_queue, &lit);
  assert(!stddll_is_end(&group->memb_queue, &lit) && 
	 group->curr_change == *(sp_memb_change**) stddll_it_val(&lit));
  stddll_erase(&group->memb_queue, &lit);                        /* remove curr_change from head of memb_queue */
  assert(stddll_is_end(&group->memb_queue, &lit) || 
	 group->curr_change != *(sp_memb_change**) stddll_it_val(&lit));

  stdhash_find(&group->pmemb_hash, &hit, &group->fl_view->gid);
  assert(!stdhash_is_end(&group->pmemb_hash, &hit) && 
	 group->curr_change == *(sp_memb_change**) stdhash_it_val(&hit));
  stdhash_erase(&group->pmemb_hash, &hit);                            /* remove curr_change from pmemb_hash */
  assert(stdhash_is_end(&group->pmemb_hash, stdhash_find(&group->pmemb_hash, &hit, &group->fl_view->gid)));

  free_sp_memb_change(group->curr_change);                     /* reclaim non-stolen parts */
  group->curr_change = 0;
  DEBUG(std_stkfprintf(stderr, -1, "update_fl_view: group('%s', %p), %s, new gid %d %d %d\n",
		       group->group, group, state_str(group->vstate), group->fl_view->gid.id[0],
		       group->fl_view->gid.id[1], group->fl_view->gid.id[2]));
}

static char*
determine_leavers(stdhash *leavers, view *last_sp_view, gc_recv_mess *m) {
  char     *ret = 0;
  int      i;
  long     err;
  stduint32   num_membs;
  membership_info       m_info;
  char    (*members)[MAX_GROUP_NAME];

  DEBUG(std_stkfprintf(stderr, 1, "determine_leavers: old gid %d %d %d -> new gid %d %d %d\n", 
		       last_sp_view->gid.id[0], last_sp_view->gid.id[1], 
		       last_sp_view->gid.id[2], m->dst_gid.id[0], m->dst_gid.id[1],
		       m->dst_gid.id[2]));

  err = SP_scat_get_memb_info( m->scat_mess, *m->serv_type, &m_info);
  assert(err == 1);
  num_membs = m_info.my_vs_set.num_members;

  if (!Is_caused_network_mess(*m->serv_type)) {               /* join, leave, or disconnection */
    assert(num_membs == 1);

    DEBUG(std_stkfprintf(stderr, 0, "Not caused by network! "));
 
    stdhash_construct(leavers, sizeof(char*), 0, 
		      group_name_ptr_cmp, group_name_ptr_hashcode, 0);
    

    if ((ret = (char*) malloc(MAX_GROUP_NAME)) == 0)
      stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%d)\n", __FILE__, __LINE__, MAX_GROUP_NAME);
    memcpy( ret, m_info.changed_member, MAX_GROUP_NAME); /* read out joiner/leaver/disconnector */
    ret[MAX_GROUP_NAME - 1] = 0;                                  /* ensure null termination */

    DEBUG(std_stkfprintf(stderr, 0, "delta memb: '%s'\n", ret));
    if (!Is_caused_join_mess(*m->serv_type)) {                   /* insert leaver into leavers */
      DEBUG(std_stkfprintf(stderr, 0, "Wasn't a JOIN: adding delta to leavers\n"));
      stdhash_insert(leavers, 0, &ret, 0);
    }

  } else {
    DEBUG(std_stkfprintf(stderr, 0, "Caused by network! "));
    stdhash_copy_construct(leavers, &last_sp_view->orig_membs);     /* copy last SP membership */

    DEBUG(std_stkfprintf(stderr, 0, "Num_membs %u\n", num_membs));
    if ((members = (char (*)[MAX_GROUP_NAME])malloc(num_membs * sizeof( *members ) ) ) == 0)
      stderr_output(STDERR_ABORT, 0,"(%s, %d): malloc(%d)\n", __FILE__, __LINE__, num_membs * MAX_GROUP_NAME);
    err = SP_scat_get_vs_set_members( m->scat_mess, &m_info.my_vs_set, members, num_membs);
    assert(err == num_membs);
    for (i = 0; i < (int) num_membs; ++i) {        /* remove members that came with me: leaves who left */
      members[i][MAX_GROUP_NAME -1] = 0;
      err = stdhash_size(leavers);
      stdhash_erase_key(leavers, &(members[i][0]));
      assert(err == stdhash_size(leavers) + 1);
    }
  }
  DEBUG(std_stkfprintf(stderr, -1, "determine_leavers: delta('%s', %p), leavers size is %lu\n",
		       ret != 0 ? ret : "", ret, stdhash_size(leavers)));
  return ret;                                               /* strndup'ed delta member or null */
}

static void age_and_invalidate_pmembs(fl_group *group) {
  sp_memb_change *spc;
  stdit hit;

  DEBUG(std_stkfprintf(stderr, 1, "age_and_invalidate_pmembs: group('%s', %p), %s\n",
		       group->group, group, state_str(group->vstate)));

  stdhash_begin(&group->pmemb_hash, &hit); 
  for (; !stdhash_is_end(&group->pmemb_hash, &hit); stdhash_it_next(&hit)) {
    spc = *(sp_memb_change**) stdhash_it_val(&hit);
    if (!spc->memb_mess_recvd && ++spc->memb_change_age > FLOK_AGE_LIMIT) {
      DEBUG(std_stkfprintf(stderr, 0, "destroy too old spc %p, %d %d %d\n", spc,
			   spc->memb_info->gid.id[0], spc->memb_info->gid.id[1],
			   spc->memb_info->gid.id[2]));
      stdhash_erase(&group->pmemb_hash, &hit);
      free_sp_memb_change(spc);
      break;                             /* only one memb can become too old per SP memb event */
    }
  }
  DEBUG(std_stkfprintf(stderr, -1, "age_and_invalidate_pmembs: group('%s', %p), %s\n",
		       group->group, group, state_str(group->vstate)));
}

static sp_memb_change*
collapse_memberships(fl_group *group, stdhash *leavers, sp_memb_change *c) {
  stdit leavers_it, spc_it, flok_it, pmemb_it;
  int invalidating = 0;
  sp_memb_change *spc;
  stdit lit;
  char *leaver;

  DEBUG(std_stkfprintf(stderr, 1, "collapse_memberships: group('%s', %p), %s, num_leavers %lu\n",
		       group->group, group, state_str(group->vstate), stdhash_size(leavers)));
  if (!stddll_empty(&group->memb_queue) && !stdhash_empty(leavers)) {
    for (stddll_begin(&group->memb_queue, &lit); !stddll_is_end(&group->memb_queue, &lit); ) { /* loop SP membs */
      spc = *(sp_memb_change**) stddll_it_val(&lit);
      
      stdhash_begin(leavers, &leavers_it);                                /* loop over leavers */
      for (; !stdhash_is_end(leavers, &leavers_it); stdhash_it_next(&leavers_it)) {  
	leaver = *(char**) stdhash_it_key(&leavers_it);

	/* see if a leaver is a member of this SP memb change */
	if (!stdhash_is_end(&spc->memb_info->curr_membs, stdhash_find(&spc->memb_info->curr_membs, &spc_it, &leaver))) {
	  DEBUG(std_stkfprintf(stderr, 0, "leaver '%s' is in a pending membership!\n", leaver));
	  /* if leaver hasn't sent his flok yet, then invalidate this sp_memb_change: NO & !!! */
	  if (stdhash_is_end(&spc->flok_senders, stdhash_find(&spc->flok_senders, &flok_it, leaver))) {
	    DEBUG(std_stkfprintf(stderr, 0, "leaver didn't send flok: collapsing membship!\n"));
	    invalidating = 1;
	    stdhash_find(&group->pmemb_hash, &pmemb_it, &spc->memb_info->gid);
	    assert(!stdhash_is_end(&group->pmemb_hash, &pmemb_it));
	    stdhash_erase(&group->pmemb_hash, &pmemb_it);       /* removes spc from pmemb hash */
	    stddll_erase(&group->memb_queue, &lit);                /* removes spc from memb_queue and advances lit */
	    free_sp_memb_change(spc);
	    break;                             /* break out of inner loop: move on to next spc */
	  } else             /* remove leaver from &spc->memb_info->curr_membs because he left */
	    stdhash_erase(&spc->memb_info->curr_membs, &spc_it);
	}
      }
      if (stdhash_is_end(leavers, &leavers_it)) { /* didn't break out of loop due to an invalidation */
	if (invalidating) {                  /* did invalidate some spc's previous to this one */
	  DEBUG(std_stkfprintf(stderr, 0, "collapsing invalidates here: CAUSED_BY_NETWORK\n"));
	  invalidating = 0;
	  spc->memb_info->memb_type = REG_MEMB_MESS | CAUSED_BY_NETWORK; /* collapse them here */
	}
	stddll_it_next(&lit);                            /* advance lit to next sp_memb_change */
      }
    }
    if (invalidating) {        /* invalidated last few on the memb queue: collapse them into c */
      DEBUG(std_stkfprintf(stderr, 0, "collapsing invalidates into current spc\n"));
      c->memb_info->memb_type = REG_MEMB_MESS | CAUSED_BY_NETWORK;
    }
  }
  spc = (stddll_empty(&group->memb_queue) ? c : 
	 *(sp_memb_change**) stddll_it_val(stddll_begin(&group->memb_queue, &lit)));

  DEBUG(std_stkfprintf(stderr, -1, "collapse_memberships: curr change %p, group('%s', %p), %s\n",
		       spc, group->group, group, state_str(group->vstate)));
  return spc;
}

static void get_groups_info(const gc_recv_mess *m, int *num_groups,
				       char (**groups)[MAX_GROUP_NAME]) {
  if (m->num_new_grps == 0) {
    *num_groups = *m->num_groups;
    *groups = m->groups;
  } else {
    *num_groups = m->num_new_grps;
    *groups = m->new_grps;
  }
}

static void get_scat_info(const gc_recv_mess *m, int *mess_len,
				     scatter **scat_mess) {
  if (m->new_msg.num_elements == 0) {
    *mess_len = m->ret;
    *scat_mess = (scatter*) m->scat_mess;
  } else {
    *mess_len = m->new_msg.elements[0].len;
    *scat_mess = (scatter*) &m->new_msg;
  }
}

/* is a group name a private group name or not? */
static int is_private_group(const char group[MAX_GROUP_NAME]) {
  const char *end = group + MAX_GROUP_NAME;

  while (group != end && *group != 0 && *group != '#')
    ++group;
  
  return group != end && *group != 0;
}

/* check if a msg wrt a particular group is vulnerable or not */
static int is_vulnerable_mess(const fl_group *group, service serv_type) {
  /* need to include AUTHORIZE state because of coming here directly from the VERIFY state */
  return ((group->vstate == AUTHORIZE || group->vstate == VERIFY) &&
	  (serv_type & (UNRELIABLE_MESS | RELIABLE_MESS | FIFO_MESS)) != 0);
}

#ifndef NDEBUG
static const char *state_str(fl_view_state state) {
  switch (state) {
  case STEADY:    return "STEADY";
  case AUTHORIZE: return "AUTHORIZE";
  case AGREE:     return "AGREE";
  case VERIFY:    return "VERIFY";
  default:        return "UNKOWN STATE!!!\n";
  }
}
#endif

static float FL_SP_version(void) {
#ifdef SPREAD_VERSION
  int major, minor, patch, divBy;

  SP_version(&major, &minor, &patch);

  if (minor < 100)
    divBy = 100;
  else
    divBy = 1000;

  return major + (float) minor / divBy;
#else
  return SP_version();
#endif
}

/* hashing fcns for group names */
static int group_name_cmp(const void *str1, const void *str2) {
  return strncmp((const char*) str1, (const char*) str2, MAX_GROUP_NAME);
}

static stdhcode group_name_hashcode(const void *str) {
  return stdhcode_sfh(str, strlen((const char*) str));
}

static int group_name_ptr_cmp(const void *strptr1, const void *strptr2) {
  return strncmp(*(const char**) strptr1, *(const char**) strptr2, MAX_GROUP_NAME);
}

static stdhcode group_name_ptr_hashcode(const void *strptr) {
  return group_name_hashcode(*(const void**) strptr);
}

static int group_id_cmp(const void *gid1, const void *gid2) {
  return memcmp(gid1, gid2, sizeof(group_id));
}

static stdhcode group_id_hashcode(const void *gid) {
  return stdhcode_sfh(gid, sizeof(group_id));
}

