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


#ifndef INC_CONFIGURATION
#define INC_CONFIGURATION

#include "arch.h"
#include "spread_params.h"

/* Part of sequence number debugging */
#define INITIAL_SEQUENCE_NEAR_WRAP      ( MAX_WRAP_SEQUENCE_VALUE - 5 )

/* For what spread services should listen on what interfaces */

#define IFTYPE_MONITOR  0x1
#define IFTYPE_CLIENT   0x2
#define IFTYPE_DAEMON   0x4
#define IFTYPE_ANY      0x8
#define IFTYPE_ALL      0x7

#define Is_IfType_Client( type )        ( type & IFTYPE_CLIENT )
#define Is_IfType_Monitor( type )        ( type & IFTYPE_MONITOR )
#define Is_IfType_Daemon( type )        ( type & IFTYPE_DAEMON )
#define Is_IfType_Any( type )        ( type & IFTYPE_ANY )

struct spread_if_info {
        int32u  ip;
        int16   port;
        int16   type;
};
typedef struct dummy_proc{
	char	name[MAX_PROC_NAME]; 
	int16	port;
	int16	seg_index;
	int16	index_in_seg;
	int32u	id;
        int     num_if;
        struct spread_if_info ifc[MAX_INTERFACES_PROC];
} proc;

typedef struct dummy_segment{
	int32 	bcast_address;
	int16	port;
	int	num_procs;
	proc    *procs[MAX_PROCS_SEGMENT];
} segment;

typedef struct dummy_configuration{
        int32u  hash_code;
	int	num_segments;
        int     num_total_procs;
        proc    *allprocs;
	segment	segments[MAX_SEGMENTS];
} configuration;

typedef enum dummy_port_reuse {
    port_reuse_auto,
    port_reuse_on,
    port_reuse_off
} port_reuse;

void		Conf_init( char *file_name, char *my_name );
void	        Conf_load_conf_file( char *file_name, char *my_name );
void            Conf_config_copy( configuration *src_conf, configuration *dst_conf);
configuration	Conf(void);
configuration   *Conf_ref(void);
proc		Conf_my(void);
int		Conf_proc_by_id( int32u id, proc *p );
int		Conf_proc_by_name( char *name, proc *p );
int		Conf_proc_by_id_in_conf( configuration *config, int32u id, proc *p );
int		Conf_proc_by_name_in_conf( configuration *config, char *name, proc *p );
int		Conf_id_in_seg( segment *seg, int32u id );	
int		Conf_id_in_conf( configuration *config, int32u id );	
int		Conf_num_procs( configuration *config );
int             Conf_num_segments( configuration *config );
int32u		Conf_leader( configuration *config );
int32u		Conf_last( configuration *config );
int32u		Conf_seg_leader( configuration *config, int16 seg_index );
int32u		Conf_seg_last( configuration *config, int16 seg_index );
int             Conf_append_id_to_seg( segment *seg, int32u id);
int	        Conf_num_procs_in_seg( configuration *config, int16 seg_index );
void		Conf_id_to_str( int32u id, char *str );
char 		Conf_print(configuration *config);
char	        Conf_print_procs(configuration *config);

bool            Conf_in_reload_state(void);
void            Conf_reload_state_begin(void);
void            Conf_reload_state_end(void);
bool            Conf_in_reload_singleton_state(void);
void            Conf_reload_singleton_state_begin(void);
void            Conf_reload_singleton_state_end(void);
bool            Conf_reload_initiate(void);

void            Conf_set_debug_initial_sequence(void);
bool            Conf_debug_initial_sequence(void);

bool            Conf_get_dangerous_monitor_state(void);
void            Conf_set_dangerous_monitor_state(bool new_state);
port_reuse      Conf_get_port_reuse_type(void);
void            Conf_set_port_reuse_type(port_reuse state);
char            *Conf_get_runtime_dir(void);
void            Conf_set_runtime_dir(char *dir);
char            *Conf_get_user(void);
void            Conf_set_user(char *dir);
char            *Conf_get_group(void);
void            Conf_set_group(char *dir);
int             Conf_get_link_protocol(void);
void            Conf_set_link_protocol(int protocol);
void            Conf_set_max_session_messages(int max_messages);
int             Conf_get_max_session_messages(void);
void		Conf_set_window(int window);
int		Conf_get_window(void);
void		Conf_set_personal_window(int pwindow);
int		Conf_get_personal_window(void);

int             Conf_memb_timeouts_set(void);
int             Conf_all_memb_timeouts_set(void);
void		Conf_set_token_timeout(int timeout);
int		Conf_get_token_timeout(void);
void		Conf_set_hurry_timeout(int timeout);
int		Conf_get_hurry_timeout(void);
void		Conf_set_alive_timeout(int timeout);
int		Conf_get_alive_timeout(void);
void		Conf_set_join_timeout(int timeout);
int		Conf_get_join_timeout(void);
void		Conf_set_rep_timeout(int timeout);
int		Conf_get_rep_timeout(void);
void		Conf_set_seg_timeout(int timeout);
int		Conf_get_seg_timeout(void);
void		Conf_set_gather_timeout(int timeout);
int		Conf_get_gather_timeout(void);
void		Conf_set_form_timeout(int timeout);
int		Conf_get_form_timeout(void);
void		Conf_set_lookup_timeout(int timeout);
int		Conf_get_lookup_timeout(void);

#endif /* INC_CONFIGURATION */
