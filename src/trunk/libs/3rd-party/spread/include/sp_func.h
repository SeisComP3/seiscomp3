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



/*
  Don't include this file, include sp.h
*/ 

/* Interface routines */
int	SP_version( int *major_version, int *minor_version, int *patch_version);

int     SP_set_auth_method( const char *auth_name, int (*auth_function) (int, void *), void * auth_data);

int     SP_set_auth_methods( int num_methods, const char *auth_name[], int (*auth_function[]) (int, void *), void * auth_data[]);

int	SP_connect( const char *spread_name, const char *private_name,
		    int priority, int group_membership, mailbox *mbox,
		    char *private_group );

int	SP_connect_timeout( const char *spread_name, const char *private_name,
		    int priority, int group_membership, mailbox *mbox,
		    char *private_group, sp_time time_out );

int	SP_disconnect( mailbox mbox );

void    SP_kill( mailbox mbox );

int	SP_join( mailbox mbox, const char *group );

int	SP_leave( mailbox mbox, const char *group );

int	SP_multicast( mailbox mbox, service service_type, 
		      const char *group,
		      int16 mess_type, int mess_len, const char *mess );

int	SP_scat_multicast( mailbox mbox, service service_type,
                           const char *group,
			   int16 mess_type, const scatter *scat_mess );

int	SP_multigroup_multicast( mailbox mbox, service service_type,
				 int num_groups,
				 const char groups[][MAX_GROUP_NAME],
				 int16 mess_type, int mess_len,
				 const char *mess );

int	SP_multigroup_scat_multicast( mailbox mbox, service service_type,
				      int num_groups,
				      const char groups[][MAX_GROUP_NAME],
				      int16 mess_type,
				      const scatter *scat_mess );

int	SP_receive( mailbox mbox, service *service_type,
		    char sender[MAX_GROUP_NAME], int max_groups,
		    int *num_groups, char groups[][MAX_GROUP_NAME],
		    int16 *mess_type, int *endian_mismatch,
		    int max_mess_len, char *mess );

int	SP_scat_receive( mailbox mbox, service *service_type,
			 char sender[MAX_GROUP_NAME], int max_groups,
			 int *num_groups, char groups[][MAX_GROUP_NAME],
			 int16 *mess_type, int *endian_mismatch,
			 scatter *scat_mess );

/* get membership info from a message */
int     SP_get_memb_info( const char *memb_mess, 
                          const service service_type,
                          membership_info *memb_info);
int     SP_get_vs_sets_info( const char *memb_mess,
                             vs_set_info *vs_sets, 
                             int num_vs_sets,
                             unsigned int *my_vs_set_index);
int     SP_get_vs_set_members( const char *memb_mess, 
                               const vs_set_info *vs_set, 
                               char member_names[][MAX_GROUP_NAME],
                               int member_names_count);

int     SP_scat_get_memb_info( const scatter *memb_mess_scat, 
                               const service service_type,
                               membership_info *memb_info);
int     SP_scat_get_vs_sets_info( const scatter *memb_mess_scat, 
                                  vs_set_info *vs_sets, 
                                  int num_vs_sets,
                                  unsigned int *my_vs_set_index);
int     SP_scat_get_vs_set_members( const scatter *memb_mess_scat, 
                                    const vs_set_info *vs_set, 
                                    char member_names[][MAX_GROUP_NAME],
                                    int member_names_count);

int	SP_poll( mailbox mbox );

int	SP_equal_group_ids( group_id g1, group_id g2 );

void	SP_error( int error );
