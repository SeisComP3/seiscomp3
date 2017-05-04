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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "fl.h"
#include "stats.h"

#define MY_MAX_NUM_GROUPS 1000
#define MY_MAX_MESS_SIZE 102400

enum { LOAD_MEMBER = 0, DELTA = 1, DIE_MESS = 2048 };

mailbox mbox;
service service_type;
char    sender[MAX_GROUP_NAME];
int     num_groups;
char    groups[MY_MAX_NUM_GROUPS][MAX_GROUP_NAME];
int16   mess_type;
int     endian_mismatch;
int     mess_len;
char    mess[MY_MAX_MESS_SIZE];
int     more_messes;

char    daemon_name[MAX_GROUP_NAME] = "4803";
char    user_name[MAX_GROUP_NAME]   = "1";
char    priv_name[MAX_GROUP_NAME];
char    group_name[MAX_GROUP_NAME]  = "test";

char *exe = 0;             /* executable's name */
int user_type = -1;        /* user type of application */
int num_joins_leaves = 0;  /* i: tmp var, num_joins_leaves: # of memberships to cause */
int about_to_die = 0;      /* boolean: am I dieing after next membership? */
int num_members;

int should_sleep = 1;      /* should there be sleeps between memberships? Ususally yes. */
int pretty_print = 1;      /* should the output be verbose and labeled for human consumption? */

/* the results of the scenarios timings */
stats_results sp_join_stats, sp_leave_stats, sp_cmbo_stats;
stats_results fl_join_stats, fl_leave_stats, fl_cmbo_stats;

int i, err, num_joins;

static int printUsage(FILE *outstream) {
  return fprintf(outstream, 
		 "Usage: %s\r\n"
		 "\t[-S <group size>]                  : # members in group (for stats; also login name)\r\n"
		 "\t[-s <address>]                     : spread daemon name - either port or port@machine\r\n"
		 "\t[-g <group name>]                  : group name to join\r\n"
		 "\t[-t <user type> <# joins/leaves>]  : type of user "
		 "(LOAD_MEMBER = %d, DELTA = %d), # of join/leave events\r\n"
		 "\t[-f]                               : don't sleep between memberships (default is to sleep)\r\n"
		 "\t[-r]                               : print raw stats w/ no pretty headings\r\n", 
		 exe, LOAD_MEMBER, DELTA);
}

static void usage(int argc, char **argv) {
  for (++argv, --argc; argc > 0; ++argv, --argc) {
    if (!strcmp(*argv, "-S") && --argc) {
      strncpy(user_name, *++argv, MAX_GROUP_NAME);

    } else if (!strcmp(*argv, "-s") && --argc) {
      strncpy(daemon_name, *++argv, MAX_GROUP_NAME);

    } else if (!strcmp(*argv, "-g") && --argc) {
      strncpy(group_name, *++argv, MAX_GROUP_NAME);

    } else if (!strcmp(*argv, "-t") && (argc -= 2) > 0) {
      user_type = atoi(*++argv);
      num_joins_leaves = atoi(*++argv); 

    } else if (!strcmp(*argv, "-f")) {
      should_sleep = 0;

    } else if (!strcmp(*argv, "-r")) {
      pretty_print = 0;

    } else {
      fprintf(stderr, "Unknown cmd line param: %s\r\n", *argv);
      exit(printUsage(stderr));
    }
  }
  num_members = atoi(user_name);  
}

int main(int argc, char **argv) {
  double t;

  exe = *argv;
  usage(argc, argv);

  FL_lib_init();

  if ((err = FL_connect(daemon_name, user_name, 0, &mbox, priv_name)) != ACCEPT_SESSION) {
    fprintf(stderr, "FL_connect failure: ");
    FL_error(err);
    exit(1);
  }  
  if (user_type == DELTA) {
    double *fl_join_times  = (double*) malloc(sizeof(double) * num_joins_leaves);
    double *fl_leave_times = (double*) malloc(sizeof(double) * num_joins_leaves);
    double *fl_cmbo_times  = (double*) malloc(sizeof(double) * num_joins_leaves);

    if (!fl_join_times || !fl_leave_times || !fl_cmbo_times) {
      exit(fprintf(stderr, "Couldn't mallocate tracking arrays!\r\n"));
    }

    for (i = 0; i < num_joins_leaves; ++i) {
      t = get_time_timeofday();

      if ((err = FL_join(mbox, group_name)) < 0) {
	fprintf(stderr, "FL_join failure: ");
	FL_error(err);
	exit(1);
      }	
      do { 
	if ((mess_len = FL_receive(mbox, &service_type, sender, MY_MAX_NUM_GROUPS, 
				   &num_groups, groups, &mess_type, &endian_mismatch, 
				   MY_MAX_MESS_SIZE, mess, &more_messes)) < 0) {
	  fprintf(stderr, "FL_receive failure: ");
	  FL_error(mess_len);
	  exit(1);
	}
      } while (!Is_reg_memb_mess(service_type)); 

      fl_join_times[i] = get_time_timeofday() - t;

      if (should_sleep) {
	/* sleep for 4 times how long the join membership took */
	/* allows membership to stabilize */
	usleep((unsigned long) (4 * fl_join_times[i] * 1000));
      }
      
      /* send a kill message if we are done */
      if (i == num_joins_leaves - 1) {
	if ((mess_len = FL_multicast(mbox, SAFE_MESS, group_name, DIE_MESS, 0, 0)) < 0) {
	  fprintf(stderr, "FL_multicast failure: ");
	  FL_error(err);
	  exit(1);
	}
      }      

      t = get_time_timeofday();
      
      if ((err = FL_leave(mbox, group_name)) < 0) {
	fprintf(stderr, "FL_leave failure: ");
	FL_error(err);
	exit(1); 
      }	
      do {
	if ((mess_len = FL_receive(mbox, &service_type, sender, MY_MAX_NUM_GROUPS, 
				   &num_groups, groups, &mess_type, &endian_mismatch, 
				   MY_MAX_MESS_SIZE, mess, &more_messes)) < 0) {
	  fprintf(stderr, "FL_receive failure: ");
	  FL_error(mess_len);
	  exit(1);
	}
      } while (!Is_self_leave(service_type));

      fl_leave_times[i] = get_time_timeofday() - t;
      fl_cmbo_times[i]  = fl_join_times[i] + fl_leave_times[i];

      if (should_sleep) {
	/* sleep for 4 times how long the join membership took */
	/* allows membership to stabilize */
	usleep((unsigned long) (4 * fl_join_times[i] * 1000));
      }
    }
    /* compute timing statistics */
    comp_stats(&fl_join_stats, fl_join_times, num_joins_leaves);
    comp_stats(&fl_leave_stats, fl_leave_times, num_joins_leaves);
    comp_stats(&fl_cmbo_stats, fl_cmbo_times, num_joins_leaves);

    /* output timing statistics */
    if (pretty_print) { 
      printf("Flush Membership Timings (includes Spread): Group Size: %d, # Joins/Leaves: %d\r\n\r\n", 
	     num_members, num_joins_leaves);

      printf("\tFlush Join: ind. total (90%%): %.6fms, ind. total: %.6fms\r\n", 
	     sp_join_stats.total90, sp_join_stats.total);

      printf("\tFlush Leave: ind. total (90%%): %.6fms, ind. total: %.6fms\r\n", 
	     sp_leave_stats.total90, sp_leave_stats.total);

      printf("\tFlush Join/Leave: ind. total (90%%): %.6fms, ind. total: %.6fms\r\n", 
	     sp_cmbo_stats.total90, sp_cmbo_stats.total);

      printf("\r\n\t\tGroup Size\t# Trials\tQ1\tMEDIAN\tQ3"
	     "\tMIN (5%%)\tMEAN90\tSTDDEV90\tMAX (95%%)"
	     "\tMIN\tMEAN\tSTDDEV\tMAX\r\n");
    }
    printf("\tFL_Join:\t%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\r\n",
	   num_members, num_joins_leaves, fl_join_stats.quart1, fl_join_stats.median,
	   fl_join_stats.quart3, fl_join_stats.min5, fl_join_stats.mean90, 
	   fl_join_stats.stddev90, fl_join_stats.max95, fl_join_stats.min, 
	   fl_join_stats.mean, fl_join_stats.stddev, fl_join_stats.max);

    printf("\tFL_Leave:\t%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\r\n",
	   num_members, num_joins_leaves, fl_leave_stats.quart1, fl_leave_stats.median,
	   fl_leave_stats.quart3, fl_leave_stats.min5, fl_leave_stats.mean90, 
	   fl_leave_stats.stddev90, fl_leave_stats.max95, fl_leave_stats.min, 
	   fl_leave_stats.mean, fl_leave_stats.stddev, fl_leave_stats.max);

    printf("\tFL_Join + FL_Leave\t%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\r\n",
	   num_members, num_joins_leaves, fl_cmbo_stats.quart1, fl_cmbo_stats.median,
	   fl_cmbo_stats.quart3, fl_cmbo_stats.min5, fl_cmbo_stats.mean90, 
	   fl_cmbo_stats.stddev90, fl_cmbo_stats.max95, fl_cmbo_stats.min, 
	   fl_cmbo_stats.mean, fl_cmbo_stats.stddev, fl_cmbo_stats.max);

  } else if (user_type == LOAD_MEMBER) { 
    if ((err = FL_join(mbox, group_name)) < 0) {
      fprintf(stderr, "FL_join failure: ");
      FL_error(err);
      exit(1);
    }      
    while (1) {
      if ((mess_len = FL_receive(mbox, &service_type, sender,
				 MY_MAX_NUM_GROUPS, &num_groups, groups,
				 &mess_type, &endian_mismatch,
				 MY_MAX_MESS_SIZE, mess, &more_messes)) < 0) { 
	fprintf(stderr, "FL_receive failure: ");
	FL_error(mess_len);
	exit(1);
      }
      if (Is_flush_req_mess(service_type)) {
	if ((err = FL_flush(mbox, group_name)) < 0) {
	  fprintf(stderr, "FL_flush failure: ");
	  FL_error(err);
	  exit(1);
	}
      } else if (Is_reg_memb_mess(service_type)) {
	if (Is_caused_leave_mess(service_type)) {
	  if (about_to_die) {
	    break;
	  }
	} else if (!Is_caused_join_mess(service_type)) {
	  exit(fprintf(stderr, "Unexpected membership type: %d\n", service_type));
	}
      } else if (Is_safe_mess(service_type) && mess_type == DIE_MESS) {
	about_to_die = 1;
      }
    }
    printf("Success!\r\n");

  } else { 
    fprintf(stderr, "Unknown user type: %d\n", user_type); 
    exit(printUsage(stderr));
  }
  
  if ((err = FL_disconnect(mbox)) < 0) {
    fprintf(stderr, "FL_disconnect failure: ");
    FL_error(err);
    exit(1);
  }
  return 0;
}
