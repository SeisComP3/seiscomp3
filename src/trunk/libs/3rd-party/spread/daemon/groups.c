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

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "spread_params.h"
#include "net_types.h"
#include "protocol.h"
#include "sess_types.h"
#include "sess_body.h"
#include "groups.h"
#include "spu_objects.h"
#include "spu_memory.h"
#include "spu_events.h"
#include "status.h"
#include "spu_alarm.h"
#include "membership.h"
#if     ( SPREAD_PROTOCOL > 3 )
#include "queues.h"
#endif
#include "message.h"

#ifndef NULL
#define		NULL 	0
#endif	/* NULL */

/* An unknown membership id (on a daemon_members struct), defined as having a proc_id of -1,
 * means that the daemon is "partitioned."  "Established" is the complement.
 * Since -1 == 255.255.255.255 is the universal broadcast address, this can never
 * conflict with a *real* proc_id. */
#define         Is_unknown_memb_id( midp ) ( (midp)->proc_id == -1 )

#define         Is_established_daemon( dp ) ( !Is_unknown_memb_id( &((dp)->memb_id) ) )
#define         Is_partitioned_daemon( dp ) (  Is_unknown_memb_id( &((dp)->memb_id) ) )

/* Flag values - working with a pointer to char, set it 0x01 or test if its 0x01 */
#define         Set_first_message( cp ) ( (*cp) = 0x01  )
#define         Set_later_message( cp ) ( (*cp) = 0x00  )
#define         Is_first_message( cp )  ( (*cp) == 0x01 )
#define         Is_later_message( cp )  ( (*cp) == 0x00 )

/* IP should be a 32-bit integer, and
 * STR should be a character array of size at least 16. */
#define IP_to_STR( IP, STR ) snprintf( STR, 16, "%d.%d.%d.%d",                 \
                                       IP1(IP), IP2(IP), IP3(IP), IP4(IP) )

/* The representative of a synced set is the first member.
 * The set is sorted by daemon order in conf. */
#define Is_synced_set_leader( proc_id ) ( (proc_id) == MySyncedSet.proc_ids[0] )

typedef struct dummy_groups_buf_link {
  char                           buf[GROUPS_BUF_SIZE];
  int                            bytes;
  struct dummy_groups_buf_link  *next;
} groups_buf_link;

typedef struct dummy_synced_set {
        int32u size;
        int32  proc_ids[MAX_PROCS_RING];
} synced_set;

/* This message link struct enables the messages from a given synced
 * set to be kept together. */
typedef struct dummy_groups_message_link {
        int32                              rep_proc_id;
        int                                complete;
        message_link                      *first;
        struct dummy_groups_message_link  *next;
} groups_message_link;

char *printgroup(void *vgrp) {
  group *grp = (group *)vgrp;
  return grp->name;
}

static  configuration   Cn_active;
static  configuration   Cn_interim;
static	int		Gstate;
static	configuration	Trans_memb;
static	membership_id	Trans_memb_id;
static	configuration	Reg_memb;
static	membership_id	Reg_memb_id;
static	char		Mess_buf[MAX_MESSAGE_BODY_LEN];

static  groups_buf_link        *Groups_bufs;
static  int                     Groups_bufs_fresh;
static  int                     Num_mess_gathered;
static  int                     Num_daemons_gathered;
static  groups_message_link     Gathered; /* Groups messages */

static  int             Groups_control_down_queue;

static  stdskl          GroupsList;   /* (group*) -> nil */
/* TODO: might want to add a "secondary" index of daemon IDs -> groups for potentially faster performance */
/* TODO: might want to add a "secondary" index of member IDs -> groups for potentially faster performance */
static  synced_set      MySyncedSet;
static  membership_id   unknown_memb_id = { -1, -1 }; /* See explanation above. */

/* Unused function
 *static	int		G_id_is_equal( group_id g1, group_id g2 );
 */
static  void            G_print_group_id( int priority, group_id g, char *func_name );
static	group		*G_get_group( char *group_name );
static  daemon_members  *G_get_daemon( group *grp, int32u proc_id );
static	member		*G_get_member( daemon_members *dmn, char *private_group_name );

static  void            G_send_lightweight_memb( group *grp, int32 caused, char *private_group_name );
static  void            G_send_self_leave( group *grp, int ses );
static  void            G_send_heavyweight_memb( group *grp );
static  void            G_send_heavyweight_join( group *grp, member *joiner, mailbox new_mbox );
static  void            G_send_trans_memb( group *grp );
static  void            G_compute_group_mask( group *grp, char *func_name );

static	int		G_build_memb_buf( group *grp, message_obj *msg,
					  char buf[], int32 caused );
static	int		G_build_memb_vs_buf( group *grp, message_obj *msg,
					     char buf[], int32 caused, member *joiner );
static	message_link	*G_build_trans_mess( group *grp );

static  void            G_build_groups_msg_hdr( message_obj *msg, int groups_bytes );
static	int		G_build_groups_buf( char buf[], stdit *git, stdit *dit, int first_time );
static  void            G_build_new_groups_bufs(void);
static  int             G_send_groups_messages(void);
static	void		G_stamp_groups_bufs(void);
static  void            G_discard_groups_bufs(void);
static	int		G_mess_to_groups( message_link *mess_link, synced_set *sset );
static	void		G_compute_and_notify(void);
static	void		G_print(void);
static  void            G_update_daemon_memb_ids( group *grp );

static  void            G_add_to_synced_set( synced_set *s );
static  void            G_update_synced_set( synced_set *s, configuration *m );
static  bool            G_update_synced_set_status( synced_set *s, configuration *m );
static  void            G_print_synced_set( int priority, synced_set *s, char *func_name );

static  void            G_eliminate_partitioned_daemons( group *grp );
static  bool            G_eliminate_partitioned_daemons_status( group *grp );
static  bool            G_check_if_changed_by_cascade( group *grp );
static  void            G_remove_daemon( group *grp, daemon_members *dmn );
static  void            G_remove_group( group *grp );
static  void            G_remove_mailbox( group *grp, mailbox m );

static  int             G_compare_nameptr(const void *, const void *);
static  int             G_compare_proc_ids_by_conf( const void *, const void * );
static  int             G_compare_proc_ids_by_conf_internal( configuration *config, const void *a, const void *b);
static  int             G_compare_daemon_vs_set( const void *, const void * );

static  int             G_compare_memb_ids( const membership_id *mid1,
                                            const membership_id *mid2 );

static  void            G_shift_to_GOP( void );

static int G_compare_nameptr(const void *a, const void *b)
{
  return strncmp(*(const char**) a, *(const char**) b, MAX_GROUP_NAME);
}

static int G_compare_proc_ids_by_conf(const void *a, const void *b)
{
    return G_compare_proc_ids_by_conf_internal( &Cn_active, a, b);
}

static int G_compare_proc_ids_by_conf_interim(const void *a, const void *b)
{
    return G_compare_proc_ids_by_conf_internal( &Cn_interim, a, b);
}

/* Compare two procs based on the specified configuration structure
 * The ordering is defined by the order the procs occur in the configuration. 
 * If a proc is no longer in the active configuration, it is ordered after all active procs
 * based on it's IP address.
 */
static int G_compare_proc_ids_by_conf_internal( configuration *config, const void *a, const void *b)
{
  proc dummy_proc;
  const int32u aip = **(const int32u**)a;
  const int32u bip = **(const int32u**)b;

  int  ia = Conf_proc_by_id_in_conf( config, aip, &dummy_proc );
  int  ib = Conf_proc_by_id_in_conf( config, bip, &dummy_proc );

  /* common case */
  if (ia > -1 && ib > -1) {
      /* both are active */
      return ia - ib;
  }
  if (ia < 0 && ib < 0 ) {
      /* both not active so compare IP addresses */
      if (aip == bip)
          return 0;
      else 
          if ( aip < bip )
              return -1;
          else  
              return 1;

  } else  
      if (ia < 0)
          return 1;
      else
          if (ib < 0)
              return -1;
          else
              /* dup rule for all are active to keep compiler happy */
              return ia - ib;
}

static int G_compare_daemon_vs_set(const void *a, const void *b)
{
        const daemon_members *da  = *(const daemon_members**) a;
	const daemon_members *db  = *(const daemon_members**) b;
        int                   cmp = G_compare_memb_ids(&da->memb_id, &db->memb_id);

	if ( cmp == 0 ) {
	        const int32 * da_proc_id_ptr = &da->proc_id;
	        const int32 * db_proc_id_ptr = &db->proc_id;

		cmp = G_compare_proc_ids_by_conf( &da_proc_id_ptr, &db_proc_id_ptr );
	}

	return cmp;
}

/* Compares two memb_ids.  Arbitrary, but deterministic.  A memb_id with a proc_id
 * of -1 compares after ANY other memb_id, by definition. */
static  int  G_compare_memb_ids( const membership_id *mid1, const membership_id *mid2 )
{
        int unknown_ids = 0;
        if( Is_unknown_memb_id( mid1 ) )
                unknown_ids += 1;
        if( Is_unknown_memb_id( mid2 ) )
                unknown_ids += 2;
        switch( unknown_ids ) {
        case 0: break;
        case 1: return  1;
        case 2: return -1;
        case 3: return  0;
        }
        if( mid1->proc_id != mid2->proc_id )
                return mid1->proc_id - mid2->proc_id;
        if( mid1->time != mid2->time )
                return mid1->time - mid2->time;
        return 0;
}

static  void G_shift_to_GOP( void )
{
	Gstate = GOP;
	GlobalStatus.gstate = Gstate;
}

void	G_init()
{
        int     ret;

	Alarmp( SPLOG_INFO, GROUPS, "G_init: \n" );

	Num_groups = 0;
	GlobalStatus.num_groups = Num_groups;
	
        MySyncedSet.size        = 1;
        MySyncedSet.proc_ids[0] = My.id;
        G_print_synced_set( SPLOG_INFO, &MySyncedSet, "G_init" );

        Cn_active.allprocs = Mem_alloc( MAX_PROCS_RING * sizeof( proc ) );
        if (Cn_active.allprocs == NULL) {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failed to allocate memory for Cn_active procs array\n");
        }

	ret = stdskl_construct(&GroupsList, sizeof(group*), 0, G_compare_nameptr);
	if (ret != 0) {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize GroupsList\n");
	}
        ret = Mem_init_object(GROUP, "group", sizeof(group), 1000, 0);
        if (ret < 0)
        {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize GROUP memory objects\n");
        }
        ret = Mem_init_object(DAEMON_MEMBERS, "daemon_members", sizeof(daemon_members), 2000, 0);
        if (ret < 0)
        {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize DAEMON_MEMBERS memory objects\n");
        }
        ret = Mem_init_object(MEMBER, "member", sizeof(member), 10000, 0);
        if (ret < 0)
        {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize MEMBER memory objects\n");
        }
        ret = Mem_init_object(GROUPS_BUF_LINK, "groups_buf_link", sizeof(groups_buf_link), 1, 1);
        if( ret < 0 )
        {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize GROUPS_BUF_LINK memory objects\n");
        }
        ret = Mem_init_object(GROUPS_MESSAGE_LINK, "groups_message_link", sizeof(groups_message_link), 1, 1);
        if( ret < 0 )
        {
                Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize GROUPS_MESSAGE_LINK memory objects\n");
        }

#if     ( SPREAD_PROTOCOL == 3 )
        Groups_control_down_queue = 0;
#else
        Groups_control_down_queue = init_queuesess(Groups_down_qs);
#endif

        Groups_bufs          = NULL;
        Groups_bufs_fresh    = 0;
        Num_mess_gathered    = 0;
        Num_daemons_gathered = 0;
        Gathered.complete    = 0;
	Gathered.next        = NULL;

        Conf_config_copy( Conf_ref(), &Cn_active );

        G_shift_to_GOP();
}


/* 
 * Called from Prot_initiate_conf_reload after configuration file is reloaded (potentially with changes to spread configuration)
 * Needs to update any static-scope variables that depend on current configuration
 *
  Algorithm to clean up existing DaemonList skiplists that are stored for every group. 
  Each skiplist needs to be reformed with a different comparison function using the new Config structure.
  Once they are all reformed, we need to move the temporary daemon lists back into the main GroupsList structure entries.

  Steps:
  1) 
    make new grp skiplist to store temporary copies of all of the daemon lists.
    loop over all grps:
      copy damon list from current skiplist to new skiplist with comparison function (G_compare_proc_ids_by_conf_interim) that uses the new config structure. This maintains the daemon_members in each entry, but inserts them into a new skiplist in a new order based on the new configuration. 
      store new skiplist in the new grp list so we can store all of them with new structures before destroying them all and switching the active Conf
    end
  2)
    loop over all grps:
      destruct the current daemonlist for all groups
    end
  3)
    Set Config variable to point to new config (old config no longer available)
  4) 
    loop over all grps:
      create new daemon skiplsit for each group with normal comparison function (will effectively use new config now)
      get begin and end iterators for temp daemon skiplist with stdstk_begin() and stdskl_end()
      call stdstk_insert_seq() to insert contents of temp skiplist into new permantent skiplist
      destrcut temp skiplist
    end
    destruct temp grp skiplist.

 At the end of this function (which runs atomically with regards to any other groups functions):
  - all of the skiplists with daemons in them will be valid with the new Conf structure and can find the 'old' daemons. 
  - There are no changes to the handling of the groups membership code that runs when the ring is reformed as it can correctly remove the daemons when it thinks it should beauase they are correctly indexed and sorted in the skiplist. 

 */

void    G_signal_conf_reload(void)
{
    int                 ret;
    group		*grp, *tmp_grp;
    daemon_members      *dmn;
    stdit               git, dit;
    stdskl              tmp_GroupsList;

    ret = stdskl_construct(&tmp_GroupsList, sizeof(group*), 0, G_compare_nameptr);
    if (ret != 0) {
        Alarmp( SPLOG_FATAL, GROUPS, "G_init: Failure to Initialize GroupsList\n");
    }

    /* 
     0) Get copy of new configuration to use while we still have old config also.
        A shallow copy is fine as:
         - we want the current Conf(), 
         - do not need our own copy of allprocs,
         - and only need it valid for this function call.
    */

    Cn_interim = Conf();

    /* 
     1) 
      loop over all grps:
        copy damon list from current skiplist to new skiplist with new config as comparison fucntion 
        store new skiplist in the new grp list.
      end
    */
    for (stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ) 
    {
        grp = *(group**) stdskl_it_key(&git);
        stdskl_it_next(&git);  /* NOTE: need to do advancement before potential erasure below */
        
        tmp_grp = new( GROUP );
        memset( tmp_grp->name, 0, MAX_GROUP_NAME );
        strcpy( tmp_grp->name, grp->name );

        if (stdskl_construct(&tmp_grp->DaemonsList, sizeof(daemon_members*), 0, G_compare_proc_ids_by_conf_interim) != 0) {
            Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
        }
        tmp_grp->changed     = FALSE;
        tmp_grp->num_members = 0;
        tmp_grp->grp_id = grp->grp_id;

        if (stdskl_put(&tmp_GroupsList, NULL, &tmp_grp, NULL, STDFALSE) != 0) {
            Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
        }

        for (stdskl_begin(&grp->DaemonsList, &dit); !stdskl_is_end(&grp->DaemonsList, &dit); ) 
        {
            dmn = *(daemon_members**) stdskl_it_key(&dit);
            stdskl_it_next(&dit);  /* NOTE: need to do advancement before potential erasure below */
            
            /* insert this dmn into the new skiplist */
            if (stdskl_put(&tmp_grp->DaemonsList, NULL, &dmn, NULL, STDFALSE) != 0) {
                Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
            }
        }

    }
    /*
      2) 
      loop over all grps:
        destruct the current daemonlist
      end
    */
    for (stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ) 
    {
        grp = *(group**) stdskl_it_key(&git);
        stdskl_it_next(&git);  /* NOTE: need to do advancement before potential erasure below */
        
        stdskl_destruct( &grp->DaemonsList);
    }

    /* 3) 
       Set Cn_active variable to point to new config (old config no longer available) 
       This has to be a deep copy because it must preserve all of the procs array when the configuration is next reloaded 
       and the configuration.c version of Conf changes. 
     */
    Conf_config_copy( Conf_ref(), &Cn_active );

    /* 4)
       loop over all grps:
         create new daemon skiplsit for each group with normal comparison function (will effectively use new config now)
         get begin and end iterators for temp daemon skiplist with stdstk_begin() and stdskl_end()
         call stdstk_insert_seq() to insert contents of temp skiplist into new permantent skiplist
         destruct temp skiplist
         free temp grp structure
       end
       destruct temp grp skiplist.
     */

    for (stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ) 
    {
        stdit bskl, eskl, it;
        
        grp = *(group**) stdskl_it_key(&git);
        stdskl_it_next(&git);  /* NOTE: need to do advancement before potential erasure below */

        if (stdskl_construct(&grp->DaemonsList, sizeof(daemon_members*), 0, G_compare_proc_ids_by_conf) != 0) {
            Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
        }

        /* find tmp_grp corresponding to grp */
        stdskl_find(&tmp_GroupsList, &it, &grp);
        if (stdskl_is_end(&tmp_GroupsList, &it)) {
            Alarmp( SPLOG_FATAL, GROUPS, "G_signal_conf_reload: failed to find group (%s) in tmp_GroupsList\n", grp->name);
        }
        tmp_grp = *(group**) stdskl_it_key(&it);

        stdskl_begin(&tmp_grp->DaemonsList, &bskl);
        stdskl_end(&tmp_grp->DaemonsList, &eskl);
        stdskl_begin(&grp->DaemonsList, &dit);
        /* Insert entire tmp_grp->DaemonsList (from begin to last) into grp->DaemonsList */
        stdskl_insert_seq(&grp->DaemonsList, &dit, &bskl, &eskl, STDTRUE);

        /* destroy interim DaemonList since we are done with it */
	stdskl_erase(&tmp_GroupsList, &it);
        stdskl_destruct( &tmp_grp->DaemonsList);
	dispose(tmp_grp);
    }

    if ( ! stdskl_empty( &tmp_GroupsList) ) {
        Alarmp( SPLOG_FATAL, GROUPS, "G_signal_conf_reload: About to destroy temporary GroupsList but it isn't empty.\n");
    }
    stdskl_destruct( &tmp_GroupsList );

}


void	G_handle_reg_memb( configuration reg_memb, membership_id reg_memb_id )
{
	group		    *grp;
        groups_message_link *grp_mlink;
        message_link        *mess_link;
        synced_set           sset;
        int                  ret;
        char                 ip_string[16];
        bool                 group_changed, synced_set_changed;
	stdit                it;

        IP_to_STR( reg_memb_id.proc_id, ip_string );
	Alarmp( SPLOG_INFO, GROUPS, "G_handle_reg_memb:  with (%s, %d) id\n", 
               ip_string, reg_memb_id.time );
        
	switch( Gstate )
	{
	    case GOP:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_reg_memb in GOP\n");

		break;

	    case GTRANS:
		/* 
		 * Save reg_memb and reg_memb_id
		 * if previous Trans_memb is equal to reg_memb then:
                 * (Purely subtractive membership change!!)
		 * 	for every changed group
		 *		eliminate partitioned daemons
		 * 		set Grp_id to (reg_memb_id, 1)
		 *		notify local members of regular membership
		 *	Shift to GOP
                 * else
                 *	for every changed group
                 *		eliminate partitioned members
                 *	Replace protocol queue, raise event thershold
                 *      If I'm representative of synced set
                 *	        Build groups messages -- 
                 *                contains only members local to daemons in SyncedSet
                 *	        Send groups messages
                 *	Shift to GGATHER
                 */
		Alarmp( SPLOG_INFO, GROUPS, "G_handle_reg_memb in GTRANS\n");

		Reg_memb    = reg_memb;
		Reg_memb_id = reg_memb_id;

		if( Conf_num_procs( &Trans_memb ) == Conf_num_procs( &Reg_memb ) )
		{
		        for (stdskl_begin(&GroupsList, &it); !stdskl_is_end(&GroupsList, &it); ) 
		        {
				grp = *(group**) stdskl_it_key(&it);
				stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

				if( grp->changed )
				{
				        /* The group has changed */
				        /* eliminating partitioned daemons */
				        G_eliminate_partitioned_daemons( grp );
					if( grp->num_members == 0 )
					{
					        /* discard this empty group */
					        G_remove_group( grp );
					}else{
					        Alarmp( SPLOG_INFO, GROUPS,
							"G_handle_reg_memb: no state transfer needed for group %s.\n",
							grp->name );
						Alarmp( SPLOG_DEBUG, GROUPS,
							"G_handle_reg_memb: changing group_id for group %s\n", grp->name );
						G_print_group_id( SPLOG_DEBUG, grp->grp_id, "G_handle_reg_memb" );
						grp->grp_id.memb_id = Reg_memb_id;
						grp->grp_id.index   = 1;
						grp->changed        = FALSE;
						G_print_group_id( SPLOG_DEBUG, grp->grp_id, "G_handle_reg_memb" );
						G_update_daemon_memb_ids( grp );
						if( !stdarr_empty(&grp->mboxes) )
						{
						        G_send_heavyweight_memb( grp );
						}
					}
				}
			}

                        G_shift_to_GOP();

		} else {
                        /*
                         * else
                         *	for every changed group
                         *		eliminate partitioned members
                         *	Replace protocol queue, raise event thershold
                         *      If I'm representative of synced set
                         *	        Build groups messages -- 
                         *                contains only members local to daemons in SyncedSet
                         *	        Send groups messages
                         *	Shift to GGATHER
                         */

		        
		        for (stdskl_begin(&GroupsList, &it); !stdskl_is_end(&GroupsList, &it); ) 
		        {
				grp = *(group**) stdskl_it_key(&it);
				stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

                                if( grp->changed )
                                {
                                        /* The group has changed */
                                        /* eliminating partitioned members */
                                        G_eliminate_partitioned_daemons( grp );
                                        if( grp->num_members == 0 )
                                        {
                                                /* discard this empty group */
                                                G_remove_group( grp );
                                        }
                                }
                        }
                        /* raise events threshold */
                        Session_threshold = MEDIUM_PRIORITY;
                        Sess_set_active_threshold();

                        /* Replace down queue */
                        Prot_set_down_queue( GROUPS_DOWNQUEUE );

                        /* If I'm the head of my synced set, I send one or more GROUPS messages.
                         * No daemon's data about members for a given group is ever split across
                         * multiple messages.  As an optimization, only the last message is sent
                         * AGREED, and all previous messages are sent RELIABLE.  G_handle_groups depends
                         * on this to determine when it has all the messages it is waiting for. */
                        if( Is_synced_set_leader(My.id) ) {
                                G_build_new_groups_bufs();
                                Groups_bufs_fresh = 1;
                                ret = G_send_groups_messages();
                                Alarmp( SPLOG_INFO, GROUPS, 
                                        "G_handle_reg_memb: %d GROUPS messages sent in GTRANS\n", ret );
                        }

                        Gstate              = GGATHER;
                        GlobalStatus.gstate = Gstate;
                }
                break;

            case GGATHER:
                Alarmp( SPLOG_FATAL, GROUPS, "G_handle_reg_memb in GGATHER\n");
            
                break;

            case GGT:
                /*
                 * Save reg_memb and reg_memb_id
                 * If I received all of my synced set's group messages
                 *   For all synced sets for which I have all the Groups messages
                 *      Extract group information from Groups messages
                 *      Add the synced set to mine
                 * Clear all retained Groups messages
                 * Check the groups state against the last delivered Trans_memb
                 * Check my synced set against Trans_memb
                 * If I'm representative of synced set
                 *    If Groups bufs are still good (i.e. we didn't change anything)
                 *       Stamp Groups messages with current membership id
                 *    Else
                 *       Remove all Groups bufs
                 *       Build groups messages -- 
                 *          contains only members local to daemons in SyncedSet
                 *    Send groups messages
                 * Shift to GGATHER
                 */
                Alarmp( SPLOG_INFO, GROUPS, "G_handle_reg_memb in GGT\n");

                Reg_memb    = reg_memb;
                Reg_memb_id = reg_memb_id;

                /* If our messages have all arrived, then we can bring into our
                 * synced set anyone who's messages have all arrived.
                 * Regardless, we need to dispose of the groups messages. */
                for( grp_mlink = Gathered.next; grp_mlink != NULL; )
                {
                    for( mess_link = grp_mlink->first; mess_link != NULL; )
                    {
                        if( Gathered.complete && grp_mlink->complete )
                        {
                            ret = G_mess_to_groups( mess_link, &sset );
                            if( ret < 0 )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_handle_reg_memb:"
                                        " G_mess_to_groups errored %d\n", ret );
                            Groups_bufs_fresh = 0;
                        }
                        grp_mlink->first = mess_link->next;
                        Sess_dispose_message( mess_link );
                        mess_link = grp_mlink->first;
                    }
                    if( Gathered.complete && grp_mlink->complete )
                        G_add_to_synced_set( &sset );
                    Gathered.next = grp_mlink->next;
                    dispose( grp_mlink );
                    grp_mlink = Gathered.next;
                }

                /* We put off really handling the transitional configuration until now
                 * so as to not deliver potentially inconsistent groups messages
                 * if we completed the old state exchange.  Now, prepare for the next one.
                 */
		for (stdskl_begin(&GroupsList, &it); !stdskl_is_end(&GroupsList, &it); ) 
		{
		        grp = *(group**) stdskl_it_key(&it);
			stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

			group_changed = G_eliminate_partitioned_daemons_status( grp );
			if( group_changed )
			{
			        Groups_bufs_fresh = 0;
				if( grp->num_members == 0 )
				{
				        /* discard this empty group */
				        G_remove_group( grp );
				} else {
				        grp->changed = TRUE;
				}
			}
                }
                synced_set_changed = G_update_synced_set_status( &MySyncedSet, &Trans_memb );
                G_print_synced_set( SPLOG_INFO, &MySyncedSet, "G_handle_reg_memb" );
                /* Since one of the groups bufs holds the synced set... */
                if( synced_set_changed )
                    Groups_bufs_fresh = 0;

                Gathered.complete    = 0;
                Num_mess_gathered    = 0;
                Num_daemons_gathered = 0;

                if( Is_synced_set_leader(My.id) ) {
                    /*  Stamp own Groups message in buffer with current membership id */
                    if( Groups_bufs_fresh ) {
                        G_stamp_groups_bufs();
                    } else {
                        G_discard_groups_bufs();
                        G_build_new_groups_bufs();
                        Groups_bufs_fresh = 1;
                    }
                    ret = G_send_groups_messages();
                    Alarmp( SPLOG_INFO, GROUPS, "G_handle_reg_memb: %d GROUPS messages"
                            " sent in GGT\n", ret );
                }

                Gstate = GGATHER;
                GlobalStatus.gstate = Gstate;

                break;
        }
}

void	G_handle_trans_memb( configuration trans_memb, membership_id trans_memb_id )
{
	group		    *grp;
        daemon_members      *dmn;
	bool    	     group_changed;
        char                 ip_string[16];
	stdit                git, dit;

        IP_to_STR( trans_memb_id.proc_id, ip_string );
	Alarmp( SPLOG_INFO, GROUPS, "G_handle_trans_memb:  with (%s, %d) id\n", 
               ip_string, trans_memb_id.time );

	switch( Gstate )
	{
	    case GOP:
		/* 
		 * Save transitional membership
		 * For every group that has daemons that are not in the trans_memb do:
		 *	mark group daemons that are not in trans_memb as partitioned.
		 * 	notify local members with an empty transitional group mess.
		 * 	mark group as changed
                 *      update group ID and daemon membership IDs with Trans_memb_id
		 * Shift to GTRANS
		 */
		Alarmp( SPLOG_INFO, GROUPS, "G_handle_trans_memb in GOP\n");

		Trans_memb    = trans_memb;
                Trans_memb_id = trans_memb_id;

		for (stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ) 
		{
		        grp = *(group**) stdskl_it_key(&git);
			stdskl_it_next(&git);  /* NOTE: need to do advancement before potential erasure below */

                        group_changed = FALSE;

		        for (stdskl_begin(&grp->DaemonsList, &dit); !stdskl_is_end(&grp->DaemonsList, &dit); ) 
		        {
				dmn = *(daemon_members**) stdskl_it_key(&dit);
				stdskl_it_next(&dit);  /* NOTE: need to do advancement before potential erasure below */

                                if( Conf_id_in_conf( &Trans_memb, dmn->proc_id ) == -1 )
                                {
                                        /* mark this daemon as partitioned - proc no longer in membership */
					dmn->memb_id  = unknown_memb_id;
					group_changed = TRUE;
                                }
                        }
                        if( group_changed )
                        {
                                if( !stdarr_empty(&grp->mboxes) )
                                {
                                        G_send_trans_memb( grp );
				}
                                Alarmp( SPLOG_DEBUG, GROUPS, "G_handle_trans_memb: changed group %s in GOP,"
                                       " change group id \n", grp->name );
                                G_print_group_id( SPLOG_DEBUG, grp->grp_id, "G_handle_trans_memb" );
                                grp->grp_id.memb_id = Trans_memb_id;
                                grp->grp_id.index   = 1;
                                grp->changed        = TRUE;
                                G_print_group_id( SPLOG_DEBUG, grp->grp_id, "G_handle_trans_memb" );
                                /* Here, we mark everyone who we are synced with as having the new memb_id
                                 * Specifically, that is, everyone who isn't partitioned from us now in the GroupsList */
                                G_update_daemon_memb_ids( grp );
			}
		}

		Gstate = GTRANS;
		GlobalStatus.gstate = Gstate;

                G_update_synced_set( &MySyncedSet, &Trans_memb );
                G_print_synced_set( SPLOG_INFO, &MySyncedSet, "G_handle_trans_memb" );

		break;

	    case GTRANS:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_trans_memb in GTRANS\n");

		break;

	    case GGATHER:
		/* 
		 * Save transitional membership
		 * For every group that has members that are not in the
		 * trans_memb do:
		 *   If group has daemons not in the transitional conf
                 *      mark it as changed (it might be already changed, but its ok)
                 *
                 * In order to correctly handle the case in which we complete the state
                 * exchange from the *last* reg memb, don't throw out information here
                 * but rather when we get the next reg memb.
                 *
		 * Shift to GGT
		 *
		 *	Note: there is no need to notify local members with a transitional group mess
		 *	      because no message will come between the trans group memb and the next reg group memb.
		 *	Note: this cascading deletes of members that are not in transitional membership actually
		 *	      opens the door for implementation of the ERSADS97 algorithm.
		 */
		Alarmp( SPLOG_INFO, GROUPS, "G_handle_trans_memb in GGATHER\n");

		Trans_memb    = trans_memb;
                Trans_memb_id = trans_memb_id; /* Need these either in G_handle_groups, or in
                                                * G_handle_reg_memb, depending on whether we
                                                * we complete state transfer for the current Reg_memb */
  
                /* We don't actually *need* to mark these groups as changed in the loop below.
                 * It will cause some groups that were not changed by the last membership
                 * to receive notifications anyway if we do complete the state exchange in GGT
                 * Pros: This avoids a small loss of information about who received which messages.
                 * Cons: This isn't strictly required by EVS.
                 */

		for (stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ) 
		{
		        grp = *(group**) stdskl_it_key(&git);
			stdskl_it_next(&git);  /* NOTE: need to do advancement before potential erasure below */

			group_changed = G_check_if_changed_by_cascade( grp );
			if( group_changed ) {
				grp->changed = TRUE;
			}
		}

		Gstate = GGT;
		GlobalStatus.gstate = Gstate;

		break;

	    case GGT:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_trans_memb in GGT\n");

		break;
	}
}

void	G_handle_join( char *private_group_name, char *group_name )
{
	group		*grp, *new_grp;
        daemon_members  *dmn, *new_dmn;
	member		*mbr, *new_mbr;
	char		proc_name[MAX_PROC_NAME];
	char		private_name[MAX_PRIVATE_NAME+1];
	int		new_p_ind;
	proc		new_p;
	int		ses;
	mailbox		new_mbox;

	Alarmp( SPLOG_INFO, GROUPS, "G_handle_join: %s joins group %s\n", private_group_name, group_name );

	switch( Gstate )
	{
        case GOP:
        case GTRANS:

		if (Gstate == GOP) Alarmp( SPLOG_INFO, GROUPS, "G_handle_join in GOP\n");
		if (Gstate == GTRANS) Alarmp( SPLOG_INFO, GROUPS, "G_handle_join in GTRANS\n");

		/*
                 * Find the group being joined
                 *    If group doesn't exist, then add it [unchanged, with gid index 0]
                 *       If in GOP, grp_id.memb_id is Reg_memb_id
                 *       If in GTRANS, grp_id.memb_id is Trans_memb_id
                 * Find the daemon
                 *    If daemon [for this group] doesn't exist, then add it
                 *       Record partitioned status when adding, based on Gstate and Trans_memb
                 * Find or create the member
                 *    If already in group then ignore (return)
                 * Initialize member object
                 * Increment group size
                 * Increment group ID
                 * Add mbox for local joiner
                 * Mark group as changed if daemon is partitioned
                 *
                 * If the group is changed
                 *    Notify all local members of a regular membership caused by network
                 *       Note: this regular membership lists the joiner in a separate vs_set from
                 *             its daemon's other members
                 *    Notify all local members of a transitional membership
                 * Else (if the group isn't changed)
                 *    Notify all local members of a regular membership caused by join
                 * Update the group mask (not needed for Spread 3)
		 */
		G_private_to_names( private_group_name, private_name, proc_name );

		new_p_ind = Conf_proc_by_name( proc_name, &new_p );
		if( new_p_ind < 0 )
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_join: illegal proc_name %s in private_group %s \n",
				proc_name, private_group_name );
			return;
		}
		grp = G_get_group( group_name );
		if( grp == NULL )
		{
			new_grp = new( GROUP );
			memset( new_grp->name, 0, MAX_GROUP_NAME );
			strcpy( new_grp->name, group_name );

			if (stdskl_construct(&new_grp->DaemonsList, sizeof(daemon_members*), 0, G_compare_proc_ids_by_conf) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

			if (stdarr_construct(&new_grp->mboxes, sizeof(mailbox), 0) != 0) {  
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

                        /* NOTE: Older versions of groups do mark a new group as changed if it's
                         * created in GTRANS.  This is only needed if the joiner is partitioned
                         * from us [handled below]. */
                        new_grp->changed = FALSE;
                        if( Gstate == GOP) {
                                new_grp->grp_id.memb_id = Reg_memb_id;
                                
                        } else { /* Gtrans */
                                new_grp->grp_id.memb_id = Trans_memb_id;
                        }
                        new_grp->grp_id.index = 0; /* This will be incremented to 1, below. */
                        Alarmp( SPLOG_DEBUG, GROUPS, "G_handle_join: New group added with group id:\n" );
                        G_print_group_id( SPLOG_DEBUG, new_grp->grp_id, "G_handle_join" );

			new_grp->num_members = 0;

			if (stdskl_put(&GroupsList, NULL, &new_grp, NULL, STDFALSE) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}
			
			Num_groups++;
			GlobalStatus.num_groups = Num_groups;
			grp = new_grp;
                }

                dmn = G_get_daemon( grp, new_p.id );
                if( dmn == NULL ) {
                        new_dmn = new( DAEMON_MEMBERS );
                        new_dmn->proc_id = new_p.id;

			if (stdskl_construct(&new_dmn->MembersList, sizeof(member*), 0, G_compare_nameptr) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

                        /* Are we partitioned from this daemon? */
                        if( Gstate == GOP || ( Conf_id_in_conf( &Trans_memb, new_p.id ) != -1 ) ) {
                                new_dmn->memb_id = grp->grp_id.memb_id;
                        } else {
                                new_dmn->memb_id = unknown_memb_id;
                        }

			if (stdskl_put(&grp->DaemonsList, NULL, &new_dmn, NULL, STDFALSE) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

                        dmn = new_dmn;                        
                }

		mbr = G_get_member( dmn, private_group_name );
		if( mbr != NULL )
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_join: %s is already in group %s\n",
				private_group_name, group_name );
			return;
		}
		/* Add a new member */
		new_mbr = new( MEMBER );
		memset( new_mbr->name, 0, MAX_GROUP_NAME );
		strcpy( new_mbr->name, private_group_name );

		if (stdskl_put(&dmn->MembersList, NULL, &new_mbr, NULL, STDFALSE) != 0) {
		  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
		}

		grp->num_members++;
                grp->grp_id.index++;

		/* if member is local then add mbox */
		if( dmn->proc_id == My.id )
		{
			ses = Sess_get_session( private_name );
			if( ses < 0 ) Alarmp( SPLOG_FATAL, GROUPS, "G_handle_join: local session does not exist\n" );

			new_mbox = Sessions[ ses ].mbox;

			/* TODO: do we need to ensure the insert is
			   unique or is that externally implied? Seems
			   to be implied by above checks. */

			if (stdarr_push_back(&grp->mboxes, &new_mbox) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}
		} else
                        new_mbox = -1;

                if( Is_partitioned_daemon( dmn ) && !grp->changed )
                        grp->changed = TRUE;

                if( !stdarr_empty(&grp->mboxes) ) {
                        if( grp->changed )
                        {
                                G_send_heavyweight_join( grp, new_mbr, new_mbox );
                                G_send_trans_memb( grp );
                        } else {
                                G_send_lightweight_memb( grp, CAUSED_BY_JOIN, new_mbr->name );
                        }
                }

		/* Compute the mask */
                G_compute_group_mask( grp, "G_handle_join" );

		break;

        case GGATHER:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_join in GGATHER\n");

		break;

        case GGT:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_join in GGT\n");

		break;
	}
}

void	G_handle_leave( char *private_group_name, char *group_name )
{

	char		proc_name[MAX_PROC_NAME];
	char		private_name[MAX_PRIVATE_NAME+1];
	char		departing_private_group_name[MAX_GROUP_NAME];
	int		p_ind;
	proc		p;
        int             ses;
	group		*grp;
        daemon_members  *dmn;
	member		*mbr;
	stdit           it;

	Alarmp( SPLOG_INFO, GROUPS, "G_handle_leave: %s leaves group %s\n", private_group_name, group_name );

	switch( Gstate )
	{
	    case GOP:
	    case GTRANS:

		if (Gstate == GOP) Alarmp( SPLOG_INFO, GROUPS, "G_handle_leave in GOP\n");
		if (Gstate == GTRANS) Alarmp( SPLOG_INFO, GROUPS, "G_handle_leave in GTRANS\n");

		/*
		 * If not already in group then ignore
		 * If this member is local, notify it [Self Leave] and extract its mbox
		 * Extract this member from group (and discard empty daemons/groups)
		 * Increment Grp_id
                 * If the group is changed, then:
                 *      Notify all local members of a regular membership caused by network
                 *              with all ESTABLISHED members in the vs_set
                 *      Notify all local members of a transitional membership
		 * If the group is unchanged (in GOP all groups are unchanged) then: 
		 *      Notify all local members of a regular membership caused by leave
                 * Update the group mask
		 */
		G_private_to_names( private_group_name, private_name, proc_name );
		p_ind = Conf_proc_by_name( proc_name, &p );
		if( p_ind < 0 )
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_leave: illegal proc_name %s in private_group %s \n",
				proc_name, private_group_name );
			return;
		}
		grp = G_get_group( group_name );
		if( grp == NULL ) 
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_leave: group %s does not exist\n",
				group_name );
			return;
		}
                dmn = G_get_daemon( grp, p.id );
                if( dmn == NULL )
                {
                        Alarmp( SPLOG_ERROR, GROUPS, "G_handle_leave: daemon %s doesn't exist in group %s\n",
                                proc_name, group_name );
                        return;
                }
		mbr = G_get_member( dmn, private_group_name );
		if( mbr == NULL )
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_leave: member %s does not exist in daemon/group %s/%s\n",
				private_group_name, proc_name, group_name );
			return;
		}

		if( p.id == My.id )
		{
		        /* notify this local member and extract its mbox from group */
		        ses = Sess_get_session( private_name );

			if (ses < 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "G_handle_leave: member '%s' has no local session in group '%s'\n", private_name, grp->name );
			}
			
			G_send_self_leave( grp, ses );
			G_remove_mailbox( grp, Sessions[ ses ].mbox );
		}

		/* extract this member from group */
		memcpy( departing_private_group_name, mbr->name, MAX_GROUP_NAME );

		if (stdskl_is_end(&dmn->MembersList, stdskl_find(&dmn->MembersList, &it, &mbr))) {
		  Alarmp( SPLOG_FATAL, GROUPS, "G_handle_leave: couldn't extract member(%s) from MembersList!\n", mbr->name );
		}

		stdskl_erase(&dmn->MembersList, &it);

		dispose(mbr);
		grp->num_members--;
                if( stdskl_empty(&dmn->MembersList) )
                {
                        G_remove_daemon( grp, dmn );
                }
		if( grp->num_members == 0 )
		{
			/* discard this empty group */
                        G_remove_group( grp );
                        return;
		}
		
		/* Increment group id */
		grp->grp_id.index++;

                /* Note: Groups become changed because they include partitioned daemons.
                 *       We never need to mark a group as changed here, or in G_handle_kill.
                 */

		if( !stdarr_empty(&grp->mboxes) )
		{
                        if( grp->changed )
                        {
                                G_send_heavyweight_memb( grp );
                                G_send_trans_memb( grp );
                        } else {
                                G_send_lightweight_memb( grp, CAUSED_BY_LEAVE, departing_private_group_name );
                        }
		}

		/* Compute the mask */
                G_compute_group_mask( grp, "G_handle_leave" );

		break;

	    case GGATHER:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_leave in GGATHER\n");

		break;

	    case GGT:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_leave in GGT\n");

		break;
	}
}

void	G_handle_kill( char *private_group_name )
{
	char		proc_name[MAX_PROC_NAME];
	char		private_name[MAX_PRIVATE_NAME+1];
	char		departing_private_group_name[MAX_GROUP_NAME];
	int		p_ind;
	proc		p;
	group		*grp;
        daemon_members  *dmn;
	member		*mbr;
	int		ses = -1;       /* Fool compiler */
	stdit           it, tit;

	Alarmp( SPLOG_INFO, GROUPS, "G_handle_kill: %s is killed\n", private_group_name );

	switch( Gstate )
	{
	    case GOP:
	    case GTRANS:

		if (Gstate == GOP) Alarmp( SPLOG_INFO, GROUPS, "G_handle_kill in GOP\n");
		if (Gstate == GTRANS) Alarmp( SPLOG_INFO, GROUPS, "G_handle_kill in GTRANS\n");

		/*
		 * For every group this guy is a member of
		 *    Extract this member from group (and discard empty daemons/groups)
		 *    Increment Grp_id
                 *    If the group is changed, then:
                 *       Notify all local members of a regular membership caused by network
                 *              with all ESTABLISHED members in the vs_set
                 *       Notify all local members of a transitional membership
		 *    If the group is unchanged (in GOP all groups are unchanged) then: 
		 *       Notify all local members of a regular membership caused by disconnect
                 *    Update the group mask
		 */
		G_private_to_names( private_group_name, private_name, proc_name );
		p_ind = Conf_proc_by_name( proc_name, &p );
		if( p_ind < 0 )
		{
			Alarmp( SPLOG_ERROR, GROUPS, "G_handle_kill: illegal proc_name %s in private_group %s \n",
				proc_name, private_group_name );
			return;
		}

		if( p.id == My.id ) ses = Sess_get_session( private_name );  /* FIXME: check for negative answer and error? */
	       
		for (stdskl_begin(&GroupsList, &it); !stdskl_is_end(&GroupsList, &it); ) 
		{
		        grp = *(group**) stdskl_it_key(&it);
			stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

                        dmn = G_get_daemon( grp, p.id );
                        if( dmn == NULL ) continue; /* member's daemon not in group */
			mbr = G_get_member( dmn, private_group_name );
			if( mbr == NULL ) continue; /* no such member in that group */

			/* Extract this member from group */
			if( p.id == My.id )
			{
			  G_remove_mailbox( grp, Sessions[ ses ].mbox );
			}
                        memcpy( departing_private_group_name, mbr->name, MAX_GROUP_NAME );

			if (stdskl_is_end(&dmn->MembersList, stdskl_find(&dmn->MembersList, &tit, &mbr))) {
			    Alarmp( SPLOG_FATAL, GROUPS, "G_handle_kill: unable to extract member(%s) from MembersList!\n", private_group_name );
			}			
			
			stdskl_erase(&dmn->MembersList, &tit);

			dispose(mbr);
                        grp->num_members--;
                        if( stdskl_empty(&dmn->MembersList) )
                        {
                                G_remove_daemon( grp, dmn );
                        }
                        if( grp->num_members == 0 )
                        {
                                /* discard this empty group */
                                G_remove_group( grp );
                                continue;
                        }

			/* Increment group id */
			grp->grp_id.index++; 

                        if( !stdarr_empty(&grp->mboxes) )
                        {
                                if( grp->changed )
                                {
                                        G_send_heavyweight_memb( grp );
                                        G_send_trans_memb( grp );
                                } else {
                                        G_send_lightweight_memb( grp, CAUSED_BY_DISCONNECT,
                                                                 departing_private_group_name );
                                }
                        }

                        /* Compute the mask */
                        G_compute_group_mask( grp, "G_handle_kill" );
		}
		break;

	    case GGATHER:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_kill in GGATHER\n");

		break;

	    case GGT:
		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_kill in GGT\n");

		break;
	}
}

static  void  G_send_lightweight_memb( group *grp, int32 caused, char *private_group_name )
{
	int		needed;
	int		num_bytes;
	int		ses;
	message_link	*mess_link;
	message_header	*head_ptr;
        message_obj     *msg;
	int32u          temp;
	char		*num_vs_sets_ptr;   /* number of vs_sets */
        char            *vs_set_offset_ptr; /* Byte offset into the vs_set region of my vs_set */
        char            *num_vs_ptr;        /* num members in virtual-synchrony/failure-atomicity set */
	char		*vs_ptr;            /* the virtual synchrony set */
	mailbox         *mbox_ptr;
	mailbox         *endbox_ptr;

        msg = Message_new_message();
        num_bytes = G_build_memb_buf( grp, msg, Mess_buf, caused );

        num_vs_sets_ptr = &Mess_buf[num_bytes];
        num_bytes      += sizeof( int32u );
        temp            = 1;
        memcpy( num_vs_sets_ptr, &temp, sizeof(int32u) );   /* 1 vs_set */

        vs_set_offset_ptr = &Mess_buf[num_bytes];
        num_bytes        += sizeof( int32u );
        temp = 0;
        memcpy( vs_set_offset_ptr, &temp, sizeof(int32u) ); /* offset is zero, always */

        num_vs_ptr = &Mess_buf[ num_bytes ];
        num_bytes += sizeof( int32u );
        temp       = 1;
        memcpy( num_vs_ptr, &temp, sizeof( int32u ) );      /* with 1 member */

        vs_ptr = (char *)&Mess_buf[ num_bytes ];            /* vs_set has joiner/leaver/disconnecter */
        memcpy( vs_ptr, private_group_name, MAX_GROUP_NAME );
        num_bytes += MAX_GROUP_NAME;

        head_ptr = Message_get_message_header(msg);
        head_ptr->data_len += ( 3*sizeof(int32) + MAX_GROUP_NAME );

        mess_link = new( MESSAGE_LINK );
        Message_Buffer_to_Message_Fragments( msg, Mess_buf, num_bytes );
        mess_link->mess = msg;
        Obj_Inc_Refcount(mess_link->mess);
					
        needed = 0;

	mbox_ptr   = (mailbox*) grp->mboxes.begin;
	endbox_ptr = mbox_ptr + grp->mboxes.size;

	for (; mbox_ptr != endbox_ptr; ++mbox_ptr) 
	{
	        ses = Sess_get_session_index ( *mbox_ptr );

                if( Is_memb_session( Sessions[ ses ].status ) )
                        Sess_write( ses, mess_link, &needed );
        }
        if ( !needed ) Sess_dispose_message( mess_link );
        Message_Dec_Refcount(msg);
}

static  void  G_send_self_leave( group *grp, int ses )
{
	message_link	*mess_link;
	message_header	*head_ptr;
        message_obj     *msg;
	int		needed;

        msg = Message_new_message();
        head_ptr = Message_get_message_header(msg);
        head_ptr->type = CAUSED_BY_LEAVE;
        head_ptr->type = Set_endian( head_ptr->type );
        head_ptr->hint = Set_endian( 0 );
        memcpy( head_ptr->private_group_name, grp->name, MAX_GROUP_NAME );
        head_ptr->num_groups = 0;
        head_ptr->data_len = 0;

        /* create the mess_link */
        mess_link = new( MESSAGE_LINK );
        /* NOTE: Mess_buf contents are NOT used here. We only examine "0" bytes of it
         * We just need a valid pointer here to prevent faults */
        Message_Buffer_to_Message_Fragments( msg, Mess_buf, 0);
        mess_link->mess = msg;
        Obj_Inc_Refcount(mess_link->mess);
        /* notify member */
        needed = 0;
        if( Is_memb_session( Sessions[ ses ].status ) )
                Sess_write( ses, mess_link, &needed );
        if( !needed ) Sess_dispose_message( mess_link );
        Message_Dec_Refcount(msg);
}

static  void  G_send_heavyweight_memb( group *grp )
{
        G_send_heavyweight_join( grp, NULL, -1 );
}

/* If there is a new member, then joiner will be non-null.
 * new_mbox is -1 unless there is a new member, and it's local. */
static  void  G_send_heavyweight_join( group *grp, member *joiner, mailbox new_mbox )
{
	int		num_bytes;
	message_link	*mess_link, *joiner_mess_link;
	message_header	*head_ptr;
        message_obj     *msg, *joiner_msg;
	int32u          temp;
        char            *local_vs_set_offset_ptr;
        int             needed, joiner_needed;
        int             ses;
	mailbox         *mbox_ptr;
	mailbox         *endbox_ptr;

        msg = Message_new_message();
        num_bytes = G_build_memb_vs_buf( grp, msg, Mess_buf, CAUSED_BY_NETWORK, joiner );

        /* create the mess_link */
        mess_link = new( MESSAGE_LINK );
        Message_Buffer_to_Message_Fragments( msg, Mess_buf, num_bytes );
        mess_link->mess = msg;
        Obj_Inc_Refcount(mess_link->mess);

        /* notify local members */
        needed = 0;

	mbox_ptr   = (mailbox*) grp->mboxes.begin;
	endbox_ptr = mbox_ptr + grp->mboxes.size;

	for (; mbox_ptr != endbox_ptr; ++mbox_ptr) 
	{
                /* if new member is local we do not notify it here. */
                if( joiner != NULL && new_mbox == *mbox_ptr) continue;

                ses = Sess_get_session_index ( *mbox_ptr );
                if( Is_memb_session( Sessions[ ses ].status ) )
                        Sess_write( ses, mess_link, &needed );
        }

        /* notify new member if local */
        if( new_mbox != -1 )
        {
                /* Use (mostly) the same message as was sent to the other local members. */
                joiner_msg = Message_new_message();
                head_ptr   = Message_get_message_header(joiner_msg);
                memcpy( head_ptr, Message_get_message_header(msg), sizeof(message_header) );

                /* Change the local vs_set offset to be the right one for the joiner. */
                local_vs_set_offset_ptr = &Mess_buf[
                        head_ptr->num_groups * MAX_GROUP_NAME + sizeof(group_id) + sizeof(int32u)];
                /* Offset starts from the first vs_set's size, and goes to the size of the last. */
                temp = head_ptr->data_len 
                        - (sizeof(int32u) + MAX_GROUP_NAME)
                        - (sizeof(group_id) + 2*sizeof(int32u));
                memcpy( local_vs_set_offset_ptr, &temp, sizeof(int32u) );

                joiner_mess_link = new( MESSAGE_LINK );
                Message_Buffer_to_Message_Fragments( joiner_msg, Mess_buf, num_bytes );
                joiner_mess_link->mess = joiner_msg;
                Obj_Inc_Refcount(joiner_mess_link->mess);

                joiner_needed = 0;
                ses = Sess_get_session_index ( new_mbox );
                if( Is_memb_session( Sessions[ ses ].status ) )
                        Sess_write( ses, joiner_mess_link, &joiner_needed );
                if ( !joiner_needed ) Sess_dispose_message( joiner_mess_link );
                Message_Dec_Refcount(joiner_msg);
        }
        if( !needed ) Sess_dispose_message( mess_link );
        Message_Dec_Refcount(msg);
}

static  void  G_send_trans_memb( group *grp )
{
        message_link *mess_link;
        int           needed;
        int           ses;
	mailbox      *mbox_ptr;
	mailbox      *endbox_ptr;

        /* send members transitional membership */
        mess_link = G_build_trans_mess( grp );
        needed = 0;

	mbox_ptr   = (mailbox*) grp->mboxes.begin;
	endbox_ptr = mbox_ptr + grp->mboxes.size;

	for (; mbox_ptr != endbox_ptr; ++mbox_ptr) 
	{
                ses = Sess_get_session_index ( *mbox_ptr );
                if( Is_memb_session( Sessions[ ses ].status ) )
                        Sess_write( ses, mess_link, &needed );
        }
        if( !needed ) Sess_dispose_message( mess_link );
}

void	G_handle_groups( message_link *mess_link )
{
	char		    *memb_id_ptr;
	membership_id	     temp_memb_id;
        message_obj         *msg;
	message_header	    *head_ptr;
        proc                 p;
        int32u               num_daemons_represented;
        int                  needed;
        groups_message_link *grp_mlink = NULL;

	Alarmp( SPLOG_INFO, GROUPS, "G_handle_groups: \n" );

	switch( Gstate )
	{
	    case GOP:

		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_groups in GOP\n");

		break;

	    case GTRANS:

		Alarmp( SPLOG_FATAL, GROUPS, "G_handle_groups in GTRANS\n");

		break;

	    case GGATHER:
	    case GGT:

		if (Gstate == GGATHER) Alarmp( SPLOG_INFO, GROUPS, "G_handle_groups in GGATHER\n");
		if (Gstate == GGT) Alarmp( SPLOG_INFO, GROUPS, "G_handle_groups in GGT\n");

		msg = mess_link->mess;
                Obj_Inc_Refcount(msg);
		head_ptr = Message_get_message_header(msg);
		memb_id_ptr = Message_get_first_data_ptr(msg);
		memcpy( &temp_memb_id, memb_id_ptr, sizeof( membership_id ) );
		if( !Same_endian( head_ptr->type ) )
		{
			/* Flip membership id */
			temp_memb_id.proc_id = Flip_int32( temp_memb_id.proc_id );
			temp_memb_id.time    = Flip_int32( temp_memb_id.time    );
		}
		if( ! Memb_is_equal( temp_memb_id, Reg_memb_id ) )
		{
                        Alarmp( SPLOG_INFO, GROUPS, 
                               "G_handle_groups: GROUPS message received from bad memb id proc %d, time %d, daemon %s.\n",
                               temp_memb_id.proc_id, temp_memb_id.time, head_ptr->private_group_name );
			Sess_dispose_message( mess_link );
                        Message_Dec_Refcount(msg);
			return;
		}
                if (0 > Conf_proc_by_name( head_ptr->private_group_name , &p ) )
                {
                        Alarmp( SPLOG_ERROR, GROUPS, "G_handle_groups: Groups message from someone (%s) not in conf\n",
                                head_ptr->private_group_name);
			Sess_dispose_message( mess_link );
                        Message_Dec_Refcount(msg);
                        return;
                }

                /* This is a message from my rep -- don't process it. */
                if( Is_synced_set_leader(p.id) )
                {
                        grp_mlink = &Gathered;
                        needed    = 0;
                /* else, find the appropriate rep's message list. */
                } else {
                        needed = 1;
                        for( grp_mlink = Gathered.next; grp_mlink != NULL; grp_mlink = grp_mlink->next )
                                if( p.id == grp_mlink->rep_proc_id )
                                        break;
                        if( grp_mlink == NULL )
                        {
                                grp_mlink = new( GROUPS_MESSAGE_LINK );
                                grp_mlink->rep_proc_id = p.id;
                                grp_mlink->complete    = 0;
                                mess_link->next        = NULL;
                                grp_mlink->first       = mess_link;
                                grp_mlink->next        = Gathered.next;
                                Gathered.next          = grp_mlink;
                        } else {
                                mess_link->next        = grp_mlink->first;
                                grp_mlink->first       = mess_link;
                        }
                }
                
                Num_mess_gathered++;
                /* The last Groups message a daemon sends is AGREED. */
                if( Is_agreed_mess( head_ptr->type ) )
                {
                        /* The last 4 bytes of the private group name field are overridden to hold
                         * the size of the synced set of this daemon.  This way, G_handle_groups
                         * doesn't have to find the groups message containing the synced set. */
                        memcpy( &num_daemons_represented,
                                &(head_ptr->private_group_name[MAX_GROUP_NAME-sizeof(int32u)]),
                                sizeof(int32u) );
                        if( !Same_endian( head_ptr->type ) )
                                num_daemons_represented = Flip_int32( num_daemons_represented );
                        Num_daemons_gathered += num_daemons_represented;
                        grp_mlink->complete   = 1;
                }

                Alarmp( SPLOG_INFO, GROUPS, "G_handle_groups: GROUPS message received from %s - msgs %d, daemons %d\n", 
                       head_ptr->private_group_name, Num_mess_gathered, Num_daemons_gathered );

                /* At this point, we no longer need to work with the message object in this function. */
                if( !needed )
                        Sess_dispose_message( mess_link );
                Message_Dec_Refcount(msg);


		if( Num_daemons_gathered != Conf_num_procs( &Reg_memb ) )
                {
                        return;
                }
                Alarmp( SPLOG_INFO, GROUPS, "G_handle_groups: Last GROUPS message received - msgs %d, daemons %d\n",
                       Num_mess_gathered, Num_daemons_gathered );
		/* Replace protocol queue */
		Prot_set_down_queue( NORMAL_DOWNQUEUE );

		/* lower events threshold */
		Session_threshold = LOW_PRIORITY;
		Sess_set_active_threshold();

		/* 
		 * Compute new groups membership and notify members of
		 * groups that have changed 
		 */
		G_compute_and_notify();

		if( Gstate == GGATHER )
		{
                        G_shift_to_GOP();
		}else{
                        G_shift_to_GOP();
                        /* We do want to deliver a transitional signal to any
                         * groups that are going to get a CAUSED_BY_NETWORK
                         * after our Reg_memb is delivered. */
			G_handle_trans_memb( Trans_memb, Trans_memb_id );
		}

		break;
	}
}

static	void		G_compute_and_notify()
{
	group		     *grp;
	int		      ret;
        groups_message_link  *grp_mlink;
        message_link         *mess_link;
        synced_set            sset;
	stdit                 it;
	
	Alarmp( SPLOG_INFO, GROUPS, "G_compute_and_notify:\n");
	/* Add contents of groups messages from other synced sets to my GroupsList,
         * from gathered messages.  Then discard messages */

	for( grp_mlink = Gathered.next; grp_mlink != NULL; )
	{
                for( mess_link = grp_mlink->first; mess_link != NULL; )
                {
                        ret = G_mess_to_groups( mess_link, &sset );
                        if( ret < 0 )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_compute_and_notify:"
                                        " G_mess_to_groups errored %d\n", ret );               
                        grp_mlink->first = mess_link->next;
                        Sess_dispose_message( mess_link );
                        mess_link = grp_mlink->first;
                }
                G_add_to_synced_set( &sset );
                Gathered.next = grp_mlink->next;
                dispose( grp_mlink );
                grp_mlink = Gathered.next;
	}
        G_print_synced_set( SPLOG_INFO, &MySyncedSet, "G_compute_and_notify" );

        /* At this point, our GroupsList is complete, as is our synced_set. */

	for (stdskl_begin(&GroupsList, &it); !stdskl_is_end(&GroupsList, &it); ) 
	{
	        grp = *(group**) stdskl_it_key(&it);
		stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

                /* 
                 * for every group:
                 *	If the group has changed (*)
                 *		Set new gid
                 *		notify all local members (who came with whom)
                 *
                 * Note: the group is changed if any of the following is true:
                 *       (1) It had partitioned members during the transitional period
                 *           [If it lost members to transitional, or gained some
                 *            because of a join from a partitioned daemon.]
                 *       (2) Daemons marked with different memb_ids were included
                 *           [We found new daemons with info for this group,
                 *            according to G_mess_to_groups.]
                 *       (3) If we got a cascading transitional that affects this group.
                 *           Note: I'm not sure that this behavior is necessary, but it's
                 *                 consistent with all versions of Spread that I know.
                 *                 See GGATHER case of G_handle_trans_memb for more info.
                 */
            
                if( !grp->changed )
                        continue;
                /* the group has changed */
                grp->grp_id.memb_id = Reg_memb_id;
                grp->grp_id.index   = 1;
                grp->changed        = FALSE;
                if( !stdarr_empty(&grp->mboxes) )
                        G_send_heavyweight_memb( grp );
                G_update_daemon_memb_ids( grp );
	}
        Gathered.complete    = 0;
        Num_mess_gathered    = 0;
        Num_daemons_gathered = 0;

        /* We're going back to GOP... destroy our groups messages. */
        G_discard_groups_bufs();
        Groups_bufs_fresh = 0;

	G_print();
}

/* Commented out -- not currently needed
 *static	int		G_id_is_equal( group_id g1, group_id g2 )
 *{
 *	if( g1.index == g2.index && Memb_is_equal( g1.memb_id, g2.memb_id ) )
 *		return( 1 );
 *	else	return( 0 );
 *}
 */

static	group		*G_get_group( char *group_name )
{
        stdit it;

	stdskl_find(&GroupsList, &it, &group_name);

	return (!stdskl_is_end(&GroupsList, &it) ? *(group**) stdskl_it_key(&it) : NULL);
}

static  daemon_members  *G_get_daemon( group *grp, int32u proc_id ) 
{
        stdit    it;
	int32u * proc_id_ptr = &proc_id;

	stdskl_find(&grp->DaemonsList, &it, &proc_id_ptr);

        return (!stdskl_is_end(&grp->DaemonsList, &it) ? *(daemon_members**) stdskl_it_key(&it) : NULL);
}

static	member		*G_get_member( daemon_members *dmn, char *private_group_name )
{
        stdit it;

	stdskl_find(&dmn->MembersList, &it, &private_group_name);

        return (!stdskl_is_end(&dmn->MembersList, &it) ? *(member**) stdskl_it_key(&it) : NULL);
}

static	message_link  *G_build_trans_mess( group *grp )
{
	/* 
	 * This routine builds a ready-to-be-sent transitional message signal 
	 * to the members of the process group grp 
	 */

        /* FIXME: the documentation says the gid field isn't there.  Should
         *        it be removed? */

	message_link	*mess_link;
	scatter	        *scat;
	message_header	*head_ptr;
	char		*gid_ptr;

	mess_link = new( MESSAGE_LINK );
	mess_link->mess = Message_create_message(TRANSITION_MESS, grp->name);

	scat = Message_get_data_scatter(mess_link->mess);
	scat->elements[0].len = Message_get_data_header_size() +
				sizeof( group_id );
	head_ptr = Message_get_message_header(mess_link->mess);
	gid_ptr = Message_get_first_data_ptr(mess_link->mess );

	head_ptr->data_len = sizeof( group_id );
	memcpy( gid_ptr, &grp->grp_id, sizeof(group_id) );

	return( mess_link );
}

/* The buffer built needs to be deterministic and ordered according first
 * to daemon order in conf, second by member name. */
static	int  G_build_memb_buf( group *grp, message_obj *msg, char buf[], int32 caused )
{
	int		     num_bytes;
	message_header	    *head_ptr;
	char		    *gid_ptr;
	member		    *mbr;
        daemon_members      *dmn;
	char		    *memb_ptr;
	stdit                it, mit;

	head_ptr = Message_get_message_header(msg);
	head_ptr->type = REG_MEMB_MESS;
	head_ptr->type = Set_endian( head_ptr->type );
	head_ptr->type |= caused;
	head_ptr->hint = Set_endian( 0 );
	memcpy( head_ptr->private_group_name, grp->name, MAX_GROUP_NAME );
	head_ptr->num_groups = grp->num_members;
	head_ptr->data_len = sizeof( group_id );

        num_bytes = 0;
	for (stdskl_begin(&grp->DaemonsList, &it); !stdskl_is_end(&grp->DaemonsList, &it); ) 
	{
	        dmn = *(daemon_members**) stdskl_it_key(&it);
		stdskl_it_next(&it);  /* NOTE: need to do advancement before potential erasure below */

		for (stdskl_begin(&dmn->MembersList, &mit); !stdskl_is_end(&dmn->MembersList, &mit); ) 
		{
		        mbr = *(member**) stdskl_it_key(&mit);
			stdskl_it_next(&mit);  /* NOTE: need to do advancement before potential erasure below */

                        memb_ptr = &buf[num_bytes];
                        num_bytes += MAX_GROUP_NAME;
                        memcpy( memb_ptr, mbr->name, MAX_GROUP_NAME );
                }
        }
	gid_ptr = &buf[num_bytes];
	num_bytes += sizeof( group_id );
	memcpy( gid_ptr, &grp->grp_id, sizeof(group_id) );

	return( num_bytes );
}


static	int  G_build_memb_vs_buf( group *grp, message_obj *msg, char buf[], int32 caused, member *joiner )
{
/* 
 * This routine builds the memb buffer message, including a virtual synchrony
 * (failure atomicity) part with a set of vs_sets (ordered deterministically by the
 * daemon membership IDs).  Each vs_set specifies a set of members (ordered deterministically
 * by daemon order in conf, then by private group name) with the property that the members
 * listed are either virtually syncrhonous with each other, or crashed.
 *
 * Partitioned daemons get singleton sets, as do new joiners in the case of
 * a join delivered during a transitional period for a changed group.  That is, we provide
 * all the information we have, which is that the members at a given daemon are together.
 * 
 * Note that in (non-GTRANS/changed) join, leave, and disconnect we provide the member
 * that joined, left, or got disconnected in the vs_set. Therefore, caused will always be
 * CAUSED_BY_NETWORK.
 * The joiner should be NULL, except in the case of a join during GTRANS for a group
 * that has some partitioned daemons.
 */

/* The buffer constructed should have two regions, as exposed to the user:
 * groups array (ordered by daemon order in conf, then by member name)
 * data
 *
 * The data portion should look like the following:
 *   group id (group_id)
 *   number of vs sets (int32u)
 *   offset to the vs set for the member this is sent to (This is a byte offset into
 *     the vs_set region.  Could do just vs_set number, but that would be slower in the
 *     [assumed to be] common case that people just want their set.) (int32u)
 *   vs sets (ordered by group id, with partitioned daemons singleton, and joiner last)
 *
 *   Each vs set looks like:
 *     number of members (int32u)
 *     members (ordered by daemon order in conf, then by member name) (array of group names)
 */

	int		     num_bytes;
	message_header	    *head_ptr;
        char                *vs_set_region_ptr;       /* int32u */
        char                *num_vs_sets_ptr;         /* int32u */
        int32u               num_vs_sets;
        char                *local_vs_set_offset_ptr; /* int32u */
        int32u               local_vs_set_offset;
        
        char                *curr_vs_set_size_ptr;    /* int32u */
        int32u               curr_vs_set_size;
        membership_id        curr_vs_set_memb_id;

        daemon_members      *dmn;
        member              *mbr;
	char		    *membs_ptr;
        stdskl               temp;
        int                  needed;
        int                  found_joiner = 0;
	stdit                it, mit;

	num_bytes           = G_build_memb_buf( grp, msg, buf, caused );
        head_ptr            = Message_get_message_header(msg);

	num_vs_sets_ptr     = &buf[num_bytes];
	num_bytes          += sizeof( int32u );
        head_ptr->data_len += sizeof( int32u );
	num_vs_sets         = 0;

        local_vs_set_offset_ptr = &buf[num_bytes];
        num_bytes              += sizeof( int32u );
        head_ptr->data_len     += sizeof( int32u );
        /* This function is only called if I have local members.  So, if the offset
         * isn't found, then the joiner is my member. */
        local_vs_set_offset     = 0;

        /* Points to the front of the vs_sets */
        vs_set_region_ptr       = &buf[num_bytes];

        /* use a skiplist to sort all of the group's daemons by memb_id (primary) and then by proc_id (secondary) */

	if (stdskl_construct(&temp, sizeof(daemon_members*), 0, G_compare_daemon_vs_set) != 0) {
	  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
	}

	if (stdskl_put_seq_n(&temp, NULL, stdskl_begin(&grp->DaemonsList, &it), stdskl_size(&grp->DaemonsList), STDFALSE) != 0) {
	  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
	}

        curr_vs_set_memb_id  = unknown_memb_id;
        curr_vs_set_size_ptr = NULL;

	for (stdskl_begin(&temp, &it); !stdskl_is_end(&temp, &it); stdskl_it_next(&it))
	{
	        dmn = *(daemon_members**) stdskl_it_key(&it);
                needed = 0;
                if( Is_unknown_memb_id(&curr_vs_set_memb_id) ||
                    !Memb_is_equal( curr_vs_set_memb_id, dmn->memb_id ) )
                        needed = 1;
                if( needed ) {
                        num_vs_sets++;
                        curr_vs_set_memb_id   = dmn->memb_id;
                        curr_vs_set_size_ptr  = &buf[num_bytes];
                        num_bytes            += sizeof(int32u);
                        head_ptr->data_len   += sizeof(int32u);
                        curr_vs_set_size      = 0;
                }
                if( dmn->proc_id == My.id ) {
                        if( local_vs_set_offset != 0 )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_build_memb_vs_buf: Found my vs set twice for group %s\n",
                                        grp->name );
                        local_vs_set_offset = curr_vs_set_size_ptr - vs_set_region_ptr;
                        memcpy( local_vs_set_offset_ptr, &local_vs_set_offset, sizeof(int32u) );
                }

		for (stdskl_begin(&dmn->MembersList, &mit); !stdskl_is_end(&dmn->MembersList, &mit); stdskl_it_next(&mit)) 
		{
		        mbr = *(member**) stdskl_it_key(&mit);

                        /* Handle changed-group join during transitional.  The joiner does not
                         * get to be listed with everyone else from his daemon, but rather at
                         * the end, in a self vs set. */
                        if( joiner != NULL && !found_joiner ) {
                                if( strcmp( joiner->name, mbr->name ) == 0 )
                                {
                                        found_joiner = 1;
                                        continue;
                                }
			}
			membs_ptr           = &buf[num_bytes];
			num_bytes          += MAX_GROUP_NAME;
			head_ptr->data_len += MAX_GROUP_NAME;
			memcpy( membs_ptr, mbr->name, MAX_GROUP_NAME );
			curr_vs_set_size++;
		}
                /* Every time we finish one daemon, update the size of the current vs_set */
                memcpy( curr_vs_set_size_ptr, &curr_vs_set_size, sizeof(int32u) );
	}
        if( joiner != NULL ) {
                if( !found_joiner )
                        Alarmp( SPLOG_FATAL, GROUPS, "G_build_memb_vs_buf: Expected to find joining member %s.\n",
                               joiner->name );
                num_vs_sets++;
                curr_vs_set_size_ptr  = &buf[num_bytes];
                num_bytes            += sizeof(int32u);
                head_ptr->data_len   += sizeof(int32u);
                curr_vs_set_size      = 1;
                memcpy( curr_vs_set_size_ptr, &curr_vs_set_size, sizeof(int32u) );
                membs_ptr             = &buf[num_bytes];
                memcpy( membs_ptr, joiner->name, MAX_GROUP_NAME );
                num_bytes            += MAX_GROUP_NAME;
                head_ptr->data_len   += MAX_GROUP_NAME;
        }
        /* Make sure we don't leak memory before the stack gets freed and takes
         * the skiplist with it.  We don't actually want to free the daemons. */
	stdskl_destruct(&temp);
        memcpy( num_vs_sets_ptr, &num_vs_sets, sizeof(int32u) );

	return( num_bytes );
}

static  void  G_build_groups_msg_hdr( message_obj *msg, int groups_bytes )
{
	message_header	*head_ptr;

	head_ptr = Message_get_message_header(msg);
	head_ptr->type = GROUPS_MESS;
	head_ptr->type = Set_endian( head_ptr->type );
	head_ptr->hint = Set_endian( 0 );
	memset(head_ptr->private_group_name, 0, MAX_GROUP_NAME);
        /* Note: this copy uses at most 20 bytes (including terminating NULL),
         *       because proc.name is limited to MAX_PROC_NAME. */
	strcpy( head_ptr->private_group_name, My.name );
        /* The last 4 bytes of the private group name field are overridden to hold
         * the size of the synced set of this daemon.  This way, G_handle_groups
         * doesn't have to mess around with the data region of the groups messages. */
        memcpy( &head_ptr->private_group_name[MAX_GROUP_NAME-sizeof(int32u)],
                &(MySyncedSet.size), sizeof(int32u) );
	head_ptr->num_groups = 0;
	head_ptr->data_len = groups_bytes;
}

/* This function guarantees that each daemon's data about a given group appears in only one buffer in
 * a sequence, and that the sorted order is preserved from the GroupsList. */
static	int  G_build_groups_buf( char buf[], stdit *git, stdit *dit, int first_time)
{
        int   		          num_bytes;

        char		            *memb_id_ptr;
        char                *flag_ptr;             /* char, only need 1 bit, really */
        char                *synced_set_size_ptr;  /* int32u */
        char                *synced_set_procs_ptr; /* int32 */

        group		            *grp;
        daemon_members      *dmn;
        char                *proc_id_ptr;          /* int32 */
        char		            *dmn_memb_id_ptr;      /* membership_id */
        member		          *mbr;
        char                *num_dmns_ptr;         /* int16u */
        int16u               num_dmns;
        char    	          *num_memb_ptr;         /* int16u */
        int16u               num_memb;
        char		            *memb_ptr;

        int32u               size_needed;
        int                  couldnt_fit_daemon;
	stdit                mit;
        int16u               send_group_changed;

        /* A GROUPS message looks like this:
         * (Representative's name is in header, so we can get his proc id)
         *   This is necessary, because we store received GROUPS messages by
         *   synced set, to more easily recognize which to keep.
         * Membership id
         * flag (1 if first message from set, 0 else)  (char)
         * if flag is 1
         *   size of synced set   (int32u)
         *   proc ids of synced set represented by this daemon (int32*size)
         * For each group:
         *   group name (repeated for each message it appears in) (MAX_GROUP_NAME)
         *   group_id at the representative
         *      [Either there is only one ID for the group (i.e. the group is not
         *       changed in any respect by this membership) or this ID can be
         *       discarded.  This is here so that daemons that don't know about
         *       the group at all can get the correct ID in the unchanged case.]
         *   changed flag (int16u --boolean so always 0 or 1)
         *   number of daemons for this group (in this message) (int16u)
         *   For each daemon:
         *     daemon proc id  (int32)
         *     memb id at daemon (membership_id)
         *     number of local members at daemon (int16u)
         *     For each local member at daemon
         *        member's private group name (MAX_GROUP_NAME)
         */

        num_bytes   = 0;

        memb_id_ptr = &buf[num_bytes];
        num_bytes  += sizeof( membership_id );
        memcpy( memb_id_ptr, &Reg_memb_id, sizeof( membership_id ) );

        flag_ptr   = &buf[num_bytes];
        num_bytes += sizeof(char);

	if (!first_time) {
                Set_later_message(flag_ptr);

	} else {
	        Set_first_message(flag_ptr);
                synced_set_size_ptr   = &buf[num_bytes];
                num_bytes            += sizeof(int32u);
                memcpy( synced_set_size_ptr, &(MySyncedSet.size), sizeof(int32u) );
                synced_set_procs_ptr  = &buf[num_bytes];
                num_bytes            += MySyncedSet.size * sizeof(int32);
                memcpy( synced_set_procs_ptr, &MySyncedSet.proc_ids, MySyncedSet.size*sizeof(int32) );
		
		stdskl_begin(&GroupsList, git);
	}

        /* Resume where we left off in the GroupsList */
        couldnt_fit_daemon = 0;
        while (!stdskl_is_end(&GroupsList, git))
        {
	        grp = *(group**) stdskl_it_key(git);

		if (first_time) {  /* initialize dit on first call to this fcn */
		  stdskl_begin(&grp->DaemonsList, dit);
		}

                /* To have information about this group, we need to be able to fit
                 * its name, ID, and the number of daemons it has in this message. */
                size_needed = GROUPS_BUF_GROUP_INFO_SIZE + Message_get_data_header_size();
                if( size_needed > GROUPS_BUF_SIZE - num_bytes ) break;

                memcpy( &buf[num_bytes], grp->name, MAX_GROUP_NAME );
                num_bytes += MAX_GROUP_NAME;

                memcpy( &buf[num_bytes], &grp->grp_id, sizeof(group_id) );
                num_bytes += sizeof(group_id);

                send_group_changed = grp->changed;
                memcpy( &buf[num_bytes], &send_group_changed, sizeof(int16u) );
                num_bytes += sizeof(int16u);

                num_dmns_ptr  = &buf[num_bytes];
                num_bytes    += sizeof(int16u);
                num_dmns      = 0;

		for (; !stdskl_is_end(&grp->DaemonsList, dit); stdskl_it_next(dit))
		{
		        dmn = *(daemon_members**) stdskl_it_key(dit);
                        /* To store this daemon's information about the current group,
                         * we need to be able to store its proc_id, memb_id, number of
                         * local members, and the private group names of its local members. */
                        size_needed = GROUPS_BUF_DAEMON_INFO_SIZE +
                                (stdskl_size(&dmn->MembersList) * MAX_GROUP_NAME) + Message_get_data_header_size();
                        /* This requires that the number of local group members be limited. */
                        if( size_needed > GROUPS_BUF_SIZE - num_bytes )
                        {
                                couldnt_fit_daemon = 1;
                                break;
                        }
                        proc_id_ptr = &buf[num_bytes];
                        num_bytes  += sizeof(int32);
                        memcpy( proc_id_ptr, &dmn->proc_id, sizeof(int32) );

                        dmn_memb_id_ptr = &buf[num_bytes];
                        num_bytes      += sizeof(membership_id);
                        memcpy( dmn_memb_id_ptr, &grp->grp_id.memb_id, sizeof(membership_id) );

                        num_memb_ptr  = &buf[num_bytes];
                        num_bytes    += sizeof(int16u);
                        num_memb      = 0;

			for (stdskl_begin(&dmn->MembersList, &mit); !stdskl_is_end(&dmn->MembersList, &mit); stdskl_it_next(&mit)) 
			{
			        mbr = *(member**) stdskl_it_key(&mit);
                                /* Add to the buffer all group members from this daemon. */
                                memb_ptr   = &buf[num_bytes];
                                num_bytes += MAX_GROUP_NAME;
                                memcpy( memb_ptr, mbr->name, MAX_GROUP_NAME );
                                num_memb++;
                        }
                        memcpy( num_memb_ptr, &num_memb, sizeof(int16u) );

                        if( num_memb != stdskl_size(&dmn->MembersList) )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_build_groups_buf: group %s has %d %d members\n",
                                       grp->name, num_memb, stdskl_size(&dmn->MembersList) );
                        num_dmns++;
                }
                memcpy( num_dmns_ptr, &num_dmns, sizeof(int16u) );
                if( couldnt_fit_daemon )
                        break;

		stdskl_it_next(git);                     /* advance group iterator */

		if (!stdskl_is_end(&GroupsList, git)) {  /* if loop not done, then init dit iterator for the advanced git */
		  grp = *(group**) stdskl_it_key(git);
		  stdskl_begin(&grp->DaemonsList, dit);
		}
        }
        return( num_bytes );
}

static  void  G_build_new_groups_bufs()
{
	stdit                git, dit;
	int                  first_time = 1;
        groups_buf_link     *grps_buf_link;

        do {
                grps_buf_link        = new( GROUPS_BUF_LINK );
                grps_buf_link->next  = Groups_bufs;
                Groups_bufs          = grps_buf_link;
                grps_buf_link->bytes = G_build_groups_buf(grps_buf_link->buf, &git, &dit, first_time);
		first_time           = 0;

        } while (!stdskl_is_end(&GroupsList, &git));
}

/* This function used to be called G_refresh_groups_msg. */
static	void  G_stamp_groups_bufs()
{
        groups_buf_link *curr;
	char	        *memb_id_ptr;
        for( curr = Groups_bufs; curr; curr = curr->next )
        {
                memb_id_ptr = curr->buf;
                memcpy( memb_id_ptr, &Reg_memb_id, sizeof( membership_id ) );
        }
}

static  void  G_discard_groups_bufs()
{
        groups_buf_link *next;

        for( ; Groups_bufs;  Groups_bufs = next )
        {
                next = Groups_bufs->next;
                dispose( Groups_bufs );
        }
        return;
}

static  int  G_send_groups_messages()
{
        groups_buf_link *grps_buf_link;
	down_link	*down_ptr;
        message_obj     *msg;
        message_header  *head_ptr;
        int              i = 0;

        for( grps_buf_link = Groups_bufs; grps_buf_link != NULL; grps_buf_link = grps_buf_link->next ) {
                msg      = Message_new_message();
                G_build_groups_msg_hdr( msg, grps_buf_link->bytes );
                head_ptr = Message_get_message_header(msg);
                if( grps_buf_link->next )
                        head_ptr->type |= RELIABLE_MESS;
                else
                        head_ptr->type |= AGREED_MESS;
                Message_Buffer_to_Message_Fragments( msg, grps_buf_link->buf, grps_buf_link->bytes );

                down_ptr       = Prot_Create_Down_Link(msg, Message_get_packet_type(head_ptr->type), 0, 0);
                down_ptr->mess = msg; 
                Obj_Inc_Refcount(down_ptr->mess);
                /* Use control queue--not normal session queues */
                Prot_new_message( down_ptr, Groups_control_down_queue );
                Message_Dec_Refcount(msg);
                ++i;
        }
        return i;
}

/* This function fills the synced set from the synced set portion of a
 * groups message if there is one, and adds all the group membership
 * information to the GroupsList. */
static	int  G_mess_to_groups( message_link *mess_link, synced_set *sset )
{
	/* The function returns 0 for success or -1 if an error occured 
         *    Right now, there are no errors that can occur.  However,
         *    if we add stricter checks on the daemons, that may change. */

        message_obj     *msg;
        message_header  *head_ptr;
        scatter         *scat;
	int		num_bytes, total_bytes;

        char            *flag_ptr;             /* char, only need 1 bit, really */
        char            *synced_set_size_ptr;  /* int32u */
        char            *synced_set_procs_ptr; /* int32 */

	group		*grp;
        char            *group_name_ptr;
        int16u           num_dmns;
        daemon_members  *dmn;
        int16u           num_memb;
	member		*mbr;
        int              i,j;
        char             ip_string[16];
	stdit            it;
        int16u           sent_group_changed;

	total_bytes = 0;
	msg = mess_link->mess;
        scat = Message_get_data_scatter(msg);
	for( i=0; i < scat->num_elements ; i++ )
	{
		memcpy( &Temp_buf[total_bytes], scat->elements[i].buf, scat->elements[i].len );
		total_bytes += scat->elements[i].len;
	}

        num_bytes  = Message_get_data_header_size();
	head_ptr   = Message_get_message_header(msg);

        Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: message from rep %s\n", head_ptr->private_group_name );
	num_bytes += sizeof( membership_id );

        flag_ptr   = &Temp_buf[num_bytes];
        num_bytes += sizeof(char);
        if( Is_first_message(flag_ptr) )
        {
                /* Populate the synced_set from the first message */
                synced_set_size_ptr  = &Temp_buf[num_bytes];
                num_bytes           += sizeof(int32u);
                memcpy( &sset->size, synced_set_size_ptr, sizeof(int32u) );
                if( !Same_endian( head_ptr->type ) )
                        sset->size = Flip_int32( sset->size );
                synced_set_procs_ptr = &Temp_buf[num_bytes];
                num_bytes           += sset->size * sizeof(int32) ;
                memcpy( &sset->proc_ids, synced_set_procs_ptr, sset->size * sizeof(int32) );
                if( !Same_endian( head_ptr->type ) )
                        for( i = 0; i < sset->size; ++i )
                                sset->proc_ids[i] = Flip_int32( sset->proc_ids[i] );                
        }

        /* Read the groups data, and insert it into the GroupsList */
	for( ; num_bytes < total_bytes; )
	{
                group_name_ptr = &Temp_buf[num_bytes];
                num_bytes     += MAX_GROUP_NAME;

                Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: group %s\n", group_name_ptr );
                /* Create a group if necessary, and be careful to mark as changed if needed. */
                grp = G_get_group( group_name_ptr );
                if( grp == NULL )
                {
			grp = new( GROUP );
			memset( grp->name, 0, MAX_GROUP_NAME );
			strcpy( grp->name, group_name_ptr );

			if (stdskl_construct(&grp->DaemonsList, sizeof(daemon_members*), 0, G_compare_proc_ids_by_conf) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

			if (stdarr_construct(&grp->mboxes, sizeof(mailbox), 0) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

                        grp->changed     = FALSE;
			grp->num_members = 0;

                        /* Set a group id here, so that if the group isn't changed,
                         * everyone will have the right ID (because all must have same). */
                        memcpy( &grp->grp_id, &Temp_buf[num_bytes], sizeof(group_id) );
                        if( !Same_endian( head_ptr->type ) )
                        {
                                /* Flip group id */
                                grp->grp_id.memb_id.proc_id = Flip_int32( grp->grp_id.memb_id.proc_id );
                                grp->grp_id.memb_id.time    = Flip_int32( grp->grp_id.memb_id.time );
                                grp->grp_id.index    	    = Flip_int32( grp->grp_id.index );
                        }

			if (stdskl_put(&GroupsList, NULL, &grp, NULL, STDFALSE) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

			Num_groups++;
			GlobalStatus.num_groups = Num_groups;
                } 
                num_bytes += sizeof(group_id);
                /* Get the changed flag for sent group and set local group changed flag if sent group was marked changed */
                memcpy( &sent_group_changed, &Temp_buf[num_bytes], sizeof(int16u) );
                num_bytes += sizeof(int16u);
                
                if (sent_group_changed)
                    grp->changed = TRUE;

                memcpy( &num_dmns, &Temp_buf[num_bytes], sizeof(int16u) );
                num_bytes += sizeof(int16u);
                if( !Same_endian( head_ptr->type ) )
                        num_dmns = Flip_int16( num_dmns );
                Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: \twith %u daemons\n", num_dmns );

                /* For each daemon in the message for this group.
                 *    Create a daemon object, and add it to the DaemonsList.
                 *    Add all members to the MembersList. */
                for( i = 0; i < num_dmns; ++i )
                {
                        /* FIXME: If I was paranoid, I could always check here that the daemon
                         *        isn't already in my GroupsList, or that it is in my conf (from Reg_memb). */
                        dmn = new( DAEMON_MEMBERS );
                        memcpy( &dmn->proc_id, &Temp_buf[num_bytes], sizeof(int32) );
                        num_bytes += sizeof(int32);
                        memcpy( &dmn->memb_id, &Temp_buf[num_bytes], sizeof(membership_id) );
                        num_bytes += sizeof(membership_id);
                        memcpy( &num_memb, &Temp_buf[num_bytes], sizeof(int16u) );
                        num_bytes += sizeof(int16u);
                        if( !Same_endian( head_ptr->type ) )
                        {
                                dmn->proc_id         = Flip_int32( dmn->proc_id );
                                dmn->memb_id.proc_id = Flip_int32( dmn->memb_id.proc_id );
                                dmn->memb_id.time    = Flip_int32( dmn->memb_id.time );
                                num_memb             = Flip_int16( num_memb );
                        }
                        IP_to_STR( dmn->proc_id, ip_string );
                        Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: \tdaemon with proc_id %s\n", ip_string );
                        IP_to_STR( dmn->memb_id.proc_id, ip_string );
                        Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: \t\twith memb_id (%s, %d)\n",
                                ip_string, dmn->memb_id.time );
                        Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: \t\twith %u members:\n", num_memb );

			if (stdskl_construct(&dmn->MembersList, sizeof(member*), 0, G_compare_nameptr) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

			if (stdskl_put(&grp->DaemonsList, NULL, &dmn, NULL, STDFALSE) != 0) {
			  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
			}

                        if( !grp->changed &&
                            !Memb_is_equal( dmn->memb_id, grp->grp_id.memb_id ) )
                                grp->changed = TRUE;

                        /* creating members */
                        for( j = 0; j < num_memb; ++j )
                        {
                                mbr = new( MEMBER );
                                memcpy( mbr->name, &Temp_buf[num_bytes], MAX_GROUP_NAME );
                                Alarmp( SPLOG_DEBUG, GROUPS, "G_mess_to_groups: \t\t%s\n", mbr->name );
                                num_bytes += MAX_GROUP_NAME;

				/* this inserts into MembersList hinting that the insertion should be at the end of the list (faster if input sorted) */

				if (stdskl_put(&dmn->MembersList, stdskl_end(&dmn->MembersList, &it), &mbr, NULL, STDTRUE) != 0) {
				  Alarmp( SPLOG_FATAL, GROUPS, "%s: %d: memory allocation failed\n", __FILE__, __LINE__ );
				}
                        }
                        grp->num_members += num_memb;
                }
	}
	return( 0 );
}

/* TODO: G_analize_groups is O(n) when num_groups = 1 and O(n^2) when
   num_groups > 1 (multi_group_multicast) in the number of targeted
   mailboxes.  In the latter case, if n is large a O(n lg n) solution
   leveraging a skiplist would be preferable.
*/

int  G_analize_groups( int num_groups, char target_groups[][MAX_GROUP_NAME], int target_sessions[] )
{
static  mailbox mboxes[MAX_SESSIONS];
	int	num_mbox;
	mailbox *bigbox_ptr;
	mailbox *bigend_ptr;
	mailbox *litbox_ptr;
	mailbox *litend_ptr;
	group	*grp;
	char	proc_name[MAX_PROC_NAME];
	char	private_name[MAX_PRIVATE_NAME+1];
	int	ses;
	int	ret;
	int	i;
	int     *orig_target_sessions = target_sessions;

	/* collect the target local mailboxen */

	num_mbox   = 0;
	litbox_ptr = NULL;
	litend_ptr = NULL;

	for ( i=0; i < num_groups; ++i )
	{
		if( target_groups[i][0] != '#' )
		{
			/* regular group */
			grp = G_get_group( target_groups[i] );

			if( grp == NULL ) {
			  continue; 
			}

                        if( Gstate == GOP || Gstate == GTRANS ) {
			        litbox_ptr = (mailbox*) grp->mboxes.begin;  /* point litbox pointer at grp->mboxes */
				litend_ptr = litbox_ptr + grp->mboxes.size;

			} else {
                                Alarmp( SPLOG_FATAL, GROUPS, "G_analize_groups: Gstate is %d\n", Gstate );
                        }

		} else {
			/* private group */
			ret = G_private_to_names( target_groups[i], private_name, proc_name );

			/* Illegal group OR this private group is not local OR we have no such session */
			if( ret < 0 || strcmp( My.name, proc_name ) != 0 || (ses = Sess_get_session( private_name )) < 0) { 
			  continue; 
			}

			litbox_ptr = &Sessions[ ses ].mbox;  /* just point litbox pointer at the session's mbox */
			litend_ptr = litbox_ptr + 1;
		}

		/* NOTE: the following code assumes that grp->mboxes contains no duplicates */

		if (num_groups == 1) {  /* no need to do extra copy work if only one group -> just use litbox 'array' directly */
		  break;
		}

		if (num_mbox != 0) {  /* mboxes contains some entries already */
		        bigend_ptr = mboxes + num_mbox;

			for (; litbox_ptr != litend_ptr; ++litbox_ptr)
			{
			        mailbox m = *litbox_ptr;

				/* linear search over mboxes for 'm' */

				for (bigbox_ptr = mboxes; bigbox_ptr != bigend_ptr && *bigbox_ptr != m; ++bigbox_ptr);  

				if (bigbox_ptr == bigend_ptr) {
				        mboxes[ num_mbox++ ] = m;
				}
			}

		} else {
		  num_mbox = (int) (litend_ptr - litbox_ptr);  /* if num_mbox is (still) zero, then adopt litbox 'array' */
		  memcpy(mboxes, litbox_ptr, num_mbox * sizeof(mailbox));
		}
	}

	/* convert mailboxes to sessions */

	if (num_groups == 1) {  /* on non-multi group multicast just use litbox 'array' in place */
	  bigbox_ptr = litbox_ptr;
	  bigend_ptr = litend_ptr;

	} else {                /* otherwise use unique entries we've built in mboxes */
	  bigbox_ptr = mboxes;
	  bigend_ptr = mboxes + num_mbox;
	}
	
	for (; bigbox_ptr != bigend_ptr; ++bigbox_ptr, ++target_sessions)
	{
	  *target_sessions = Sess_get_session_index( *bigbox_ptr );
	}

	return (int) (target_sessions - orig_target_sessions);
}
        
static  void  G_compute_group_mask( group *grp, char *func_name )
{
#if (SPREAD_PROTOCOL == 4)
        int                     i;
        int                     temp;
        daemon_members         *dmn;
        proc                    p;
        stdit                   it;

        for(i=0; i<4; i++)
        {
                grp->grp_mask[i] = 0;
        }
	for (stdskl_begin(&grp->DaemonsList, &it); !stdskl_is_end(&grp->DaemonsList, &it); stdskl_it_next(&it)) 
	{
	        dmn = *(daemon_members**) stdskl_it_key(&it);
                Conf_proc_by_id( dmn->proc_id, &p );

		/* FIXME: TODO: isn't the following loop the same as: temp = (0x1 << (p.seg_index & 0x1F)); ??? */

                temp   = 1;
                for( i = 0; i < p.seg_index%32; i++ )
                {
			temp *= 2;
                }
                grp->grp_mask[p.seg_index/32] |= temp;
        }
        Alarmp( SPLOG_INFO, GROUPS, "%s: Mask for group %s set to %x %x %x %x\n", func_name, 
                grp->name, grp->grp_mask[3], grp->grp_mask[2], grp->grp_mask[1], grp->grp_mask[0]);
#endif
}

void  G_set_mask( int num_groups, char target_groups[][MAX_GROUP_NAME], int32u *grp_mask )
{
	group	*grp;
	char	proc_name[MAX_PROC_NAME];
	char	private_name[MAX_PRIVATE_NAME+1];
	int	ret;
	int	i, j;
	proc    p;
	int32u  temp;


	for(i=0; i<4; i++)
        {
	   grp_mask[i] = 0;
	}

	for( i=0; i < num_groups; i++ )
	{
		if( target_groups[i][0] == '#' )
		{
			/* private group */
			ret = G_private_to_names( target_groups[i], private_name, proc_name );

			/* Illegal group */
			if( ret < 0 ) continue;

		        Conf_proc_by_name( proc_name, &p ); 
		        temp = 1;
		        for(j=0; j<p.seg_index%32; j++)
		        {
			    temp *= 2;
		        }
		        grp_mask[p.seg_index/32] |= temp;

		}else{
			/* regular group */
			grp = G_get_group( target_groups[i] );
			if( grp == NULL )
			{
			    p = Conf_my();
		            temp = 1;
		            for(j=0; j<p.seg_index%32; j++)
		            {
			        temp *= 2;
		            }
		            grp_mask[p.seg_index/32] |= temp;
			}
			else if(( Gstate == GOP )||(Gstate == GTRANS))
			{
		            for(j=0; j<4; j++)
		            {
			        grp_mask[j] |= grp->grp_mask[j]; 
		            }
			    p = Conf_my();
		            temp = 1;
		            for(j=0; j<p.seg_index%32; j++)
		            {
			        temp *= 2;
		            }
		            grp_mask[p.seg_index/32] |= temp;

			}else Alarmp( SPLOG_FATAL, GROUPS, "G_set_mask: Gstate is %d\n", Gstate );
		}
	}
}

int  G_private_to_names( char *private_group_name, char *private_name, char *proc_name )
{
	char	name[MAX_GROUP_NAME];
	char	*pn, *prvn;
	unsigned int	priv_name_len, proc_name_len;
	int	i,legal_private_name;

        memcpy(name, private_group_name, MAX_GROUP_NAME );
        proc_name_len = 0; /* gcc not smart enough to detect that proc_name_len is always initialized when used */

        pn = strchr(&name[1], '#');
        if (pn != NULL)
        {
                pn[0] = '\0';
                proc_name_len = strlen( &(pn[1]));
        }
        priv_name_len = strlen( &(name[1]));
        if ( (pn == NULL) || (name[0] != '#' ) || 
             ( priv_name_len > MAX_PRIVATE_NAME) ||
             ( priv_name_len < 1 ) ||
             ( proc_name_len >= MAX_PROC_NAME ) ||
             ( proc_name_len < 1 ) )
        {
                Alarmp( SPLOG_ERROR, GROUPS, "G_private_to_names: Illegal private_group_name %s (priv, proc)\n",
                        private_group_name );
                return( ILLEGAL_GROUP );
        }
        /* start strings at actual beginning */
        pn++;
        pn[proc_name_len] = '\0';
        prvn = &name[1];
        legal_private_name = 1;
        for( i=0; i < priv_name_len; i++ )
                if( prvn[i] <= '#' ||
                    prvn[i] >  '~' ) 
                {
                        legal_private_name = 0;
                        prvn[i] = '.';
                }
        for( i=0; i < proc_name_len; i++ )
                if( pn[i] <= '#' ||
                    pn[i] >  '~' ) 
                {
                        legal_private_name = 0;
                        pn[i] = '.';
                }
        if( !legal_private_name )
        {
                Alarmp( SPLOG_ERROR, GROUPS, "G_private_to_names: Illegal private_group_name characters (%s) (%s)\n",
                       prvn, pn );
                return( ILLEGAL_GROUP );
        }
        /* copy name components including null termination */
        memcpy( private_name, prvn, priv_name_len + 1 );
        memcpy( proc_name, pn, proc_name_len + 1 );
	return( 1 );
}

static	void	G_print()
{
	group	            *grp;
        daemon_members      *dmn;
	member	            *mbr;
	int	             i, j, k;
	stdit                git, dit, mit;

	Alarmp( SPLOG_PRINT, GROUPS, "++++++++++++++++++++++\n" );
	Alarmp( SPLOG_PRINT, GROUPS, "Num of groups: %d\n", Num_groups );

	for (i = 0, stdskl_begin(&GroupsList, &git); !stdskl_is_end(&GroupsList, &git); ++i, stdskl_it_next(&git))
	{
	        grp = *(group**) stdskl_it_key(&git);
		Alarmp( SPLOG_PRINT, GROUPS, "[%d] group %s with %d members:\n", i+1, grp->name, grp->num_members );

		for (j = 0, stdskl_begin(&grp->DaemonsList, &dit); !stdskl_is_end(&grp->DaemonsList, &dit); ++j, stdskl_it_next(&dit)) 
		{
		        dmn = *(daemon_members**) stdskl_it_key(&dit);

			for (k = 0, stdskl_begin(&dmn->MembersList, &mit); !stdskl_is_end(&dmn->MembersList, &mit); ++k, stdskl_it_next(&mit)) 
			{
			        mbr = *(member**) stdskl_it_key(&mit);
                                Alarmp( SPLOG_PRINT, GROUPS, "\t[%d] %s\n", k+1, mbr->name );
                        }
                }
                Alarmp( SPLOG_PRINT, GROUPS, "----------------------\n" );
        }
}

int  G_get_num_local( char *group_name )
{
        group *grp = G_get_group( group_name );
        if( grp == NULL ) return 0;
        return stdarr_size(&grp->mboxes);
}

/* Add new members to my synced set. */
static  void  G_add_to_synced_set( synced_set *sset ) {
        synced_set temp;
        int32u     i = 0, j = 0;
        int        index_l = -1, index_r = -1;
        proc       dummy_proc;

        temp.size = 0;
        while( i < MySyncedSet.size || j < sset->size ) {
                if( i < MySyncedSet.size && index_l == -1 ) {
                        index_l = Conf_proc_by_id( MySyncedSet.proc_ids[i], &dummy_proc );
                        if( index_l == -1 )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_add_to_synced_set: proc_id %u not in conf\n",
                                        MySyncedSet.proc_ids[i] );
                }
                if( j < sset->size && index_r == -1 ) {
                        index_r = Conf_proc_by_id( sset->proc_ids[j], &dummy_proc );
                        if( index_r == -1 )
                                Alarmp( SPLOG_FATAL, GROUPS, "G_add_to_synced_set: proc_id %u not in conf\n",
                                        sset->proc_ids[j] );
                }
                if( ( index_l < index_r && index_l != -1 ) || index_r == -1 ) {
                        temp.proc_ids[temp.size++] = MySyncedSet.proc_ids[i++];
                        index_l = -1;
                } else if( ( index_r < index_l && index_r != -1 ) || index_l == -1 ) {
                        temp.proc_ids[temp.size++] = sset->proc_ids[j++];
                        index_r = -1;
                } else {
                        Alarmp( SPLOG_FATAL, GROUPS, "G_add_to_synced_set: intersection isn't empty --"
                                " equal procs %u and %u\n", MySyncedSet.proc_ids[i], sset->proc_ids[j] );
                }
        }
        memcpy( &MySyncedSet, &temp, sizeof(synced_set) );
}

/* Remove members who aren't in the membership. */
static  void  G_update_synced_set( synced_set *s, configuration *memb_p ) {
        bool ret;
        ret = G_update_synced_set_status( s, memb_p );
        return;
}

/* Remove members who aren't in the membership and
 * return true if group changed membership or false otherwise */
static  bool  G_update_synced_set_status( synced_set *s, configuration *memb_p ) 
{
        int    i, j = 0;
        bool changed = FALSE;
    
        for( i = 0; i < s->size; ++i )
                if( Conf_id_in_conf( memb_p, s->proc_ids[i] ) >= 0 )
                        s->proc_ids[j++] = s->proc_ids[i];
        /* If we lost members. */
        if( j != s->size ) {
                s->size = j;
                changed = TRUE;
        }
        return changed;
}

/* Print the synced set.  For debugging. */
static  void  G_print_synced_set( int priority, synced_set *s, char *func_name ) 
{
        int  i;
        proc p;
        Alarmp( priority, GROUPS, "%s: Synced Set (with %u members):\n", func_name, s->size );
        for( i = 0; i < s->size; ++i ) {
                Conf_proc_by_id( s->proc_ids[i], &p );
                Alarmp( priority, GROUPS, "%s: \t%s\n", func_name, p.name );
        }
}

/* Eliminate the partitioned daemons of a group. */
static  void G_eliminate_partitioned_daemons( group *grp ) 
{
    bool ret;
    ret = G_eliminate_partitioned_daemons_status( grp );
    return;
}

/* Eliminate the partitioned daemons of a group.  Return true if we changed the
 * group. */
static  bool  G_eliminate_partitioned_daemons_status( group *grp ) 
{
        daemon_members      *dmn;
        bool                 group_changed = FALSE;
        int                  needed;
	stdit                it;

	for (stdskl_begin(&grp->DaemonsList, &it); !stdskl_is_end(&grp->DaemonsList, &it); ) 
	{
	        dmn = *(daemon_members**) stdskl_it_key(&it);
	        stdskl_it_next(&it);  /* NOTE: advance here to protect against potential removal below */

                needed = 0;
                /* The first condition is sufficient, but we can optimize a bit this way. */
                if( Gstate == GGT ) /* Called in G_handle_reg_memb after we got a cascading transitional */
                {
                        if( Conf_id_in_conf( &Trans_memb, dmn->proc_id ) == -1 )
                        {
                                needed = 1;
                        } else {
                                needed = 0;
                        }
                } else {            /* Called because we got the non-cascading regular membership */
                        if( Is_partitioned_daemon( dmn ) )
                        {
                                needed = 1;
                        } else {
                                needed = 0;
                        }
                }
                if( needed )
                {
                        /* discard this daemon and its members - proc no longer in membership */
                        G_remove_daemon( grp, dmn );
                        group_changed = TRUE;
                }
        }
        return group_changed;
}

/* This function is only called when we handle a cascading transitional membership.
 * Gstate should be GGATHER, about to change to GGT */
static  bool  G_check_if_changed_by_cascade( group *grp ) 
{
        daemon_members      *dmn;
        bool                 group_changed = FALSE;
	stdit                it;

	for (stdskl_begin(&grp->DaemonsList, &it); !stdskl_is_end(&grp->DaemonsList, &it); stdskl_it_next(&it)) 
	{
	        dmn = *(daemon_members**) stdskl_it_key(&it);
                if( Conf_id_in_conf( &Trans_memb, dmn->proc_id ) == -1 )
                {
                        group_changed = TRUE;
                        break;
                }
        }
        return group_changed;
}

static  void  G_remove_daemon( group *grp, daemon_members *dmn )
{
        stdit   it;
	int32 * proc_id_ptr = &dmn->proc_id;
	
	if (stdskl_is_end(&grp->DaemonsList, stdskl_find(&grp->DaemonsList, &it, &proc_id_ptr))) {
	  Alarmp( SPLOG_FATAL, GROUPS, "G_remove_daemon: invalid daemon(%d.%d.%d.%d) removal from group(%s)\n", 
		  IP1(dmn->proc_id), IP2(dmn->proc_id), IP3(dmn->proc_id), IP4(dmn->proc_id), grp->name );
	}

	stdskl_erase(&grp->DaemonsList, &it);

	grp->num_members -= stdskl_size(&dmn->MembersList);

	for (stdskl_begin(&dmn->MembersList, &it); !stdskl_is_end(&dmn->MembersList, &it); stdskl_it_next(&it)) {
	  dispose(*(member**) stdskl_it_key(&it));  /* NOTE: this is only safe because we do destruct immediately after */
	}

	stdskl_destruct(&dmn->MembersList);
	dispose(dmn);
}

/* Remove a group that is known to be empty. */
static  void  G_remove_group( group *grp ) 
{
        stdit it;

        assert( stdskl_empty(&grp->DaemonsList) );
	assert( stdarr_empty(&grp->mboxes) );

 	if (stdskl_is_end(&GroupsList, stdskl_find(&GroupsList, &it, &grp))) {
	  Alarmp( SPLOG_FATAL, GROUPS, "G_remove_group: invalid group removal(%s)\n", grp->name );
	}

	stdskl_erase(&GroupsList, &it);

	stdskl_destruct(&grp->DaemonsList);
	stdarr_destruct(&grp->mboxes);
	dispose(grp);
	Num_groups--;
        GlobalStatus.num_groups = Num_groups;
}

/* Remove a local mailbox from grp->mboxes */
static void G_remove_mailbox( group *grp, mailbox m )
{
  mailbox *mbox_ptr   = (mailbox*) grp->mboxes.begin;                     /* address of first entry */
  mailbox *endbox_ptr = mbox_ptr + grp->mboxes.size;                      /* address of 1 past last entry */

  for (; mbox_ptr != endbox_ptr && *mbox_ptr != m; ++mbox_ptr);            /* linear search over grp->mboxes */

  if (mbox_ptr == endbox_ptr) {
    Alarmp( SPLOG_FATAL, GROUPS, "G_remove_mailbox: didn't find local mailbox %d in group '%s'\n", m, grp->name );
  }

  --endbox_ptr;                                                            /* point at last entry in grp->mboxes */
  *mbox_ptr = *endbox_ptr;                                                 /* overwrite 'm' entry w/ last entry */

  stdarr_pop_back(&grp->mboxes);                                           /* pop off redundant last entry */
}

/* All non-partitioned daemons get the membership ID component of the
 * groups current group ID. */
static  void  G_update_daemon_memb_ids( group *grp ) 
{
        stdit it;

	for (stdskl_begin(&grp->DaemonsList, &it); !stdskl_is_end(&grp->DaemonsList, &it); stdskl_it_next(&it))
	{
	        daemon_members *dmn = *(daemon_members**) stdskl_it_key(&it);

                if( Is_established_daemon( dmn ) ) {
                        dmn->memb_id = grp->grp_id.memb_id;
		}
	}
}

static  void  G_print_group_id( int priority, group_id g, char *func_name )
{
        char ip_string[16];
        IP_to_STR( g.memb_id.proc_id, ip_string );
        Alarmp( priority, GROUPS,
                "%s: Group_id {Proc ID: %s, Time: %d, Index: %d}\n", func_name,
                ip_string, g.memb_id.time, g.index );
}
