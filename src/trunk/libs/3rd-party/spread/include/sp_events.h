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



#ifndef	INC_SP_EVENTS
#define	INC_SP_EVENTS

/*
  User level code should NOT include this file directly. 
  Include sp.h instead.
  Spread daemon level code should include this file directly and NOT include sp.h
*/ 


/* Raise this number AND RECOMPILE events.c to handle more active FD's. 
 * This number limits the number of connections that 
 * can be handled.
 */
#define		MAX_FD_EVENTS		 2000

#define		NUM_PRIORITY	3

#define		LOW_PRIORITY	0
#define		MEDIUM_PRIORITY	1
#define		HIGH_PRIORITY	2

#define		NUM_FDTYPES	3

#define		READ_FD		0
#define		WRITE_FD	1
#define		EXCEPT_FD	2


typedef struct dummy_time {
	long	sec;
	long	usec;
} sp_time;

#ifndef NULL
#define NULL    (void *)0
#endif

/* Event routines */

int 	E_init(void);
sp_time	E_get_time(void);
sp_time	E_sub_time( sp_time t, sp_time delta_t );
sp_time	E_add_time( sp_time t, sp_time delta_t );
/* if t1 > t2 then returns 1;
   if t1 < t2 then returns -1;
   if t1 == t2 then returns 0; */
int	E_compare_time( sp_time t1, sp_time t2 );
int 	E_queue( void (* func)( int code, void *data ), int code, void *data,
		 sp_time delta_time );
/* Note: This does not dispose/free the data pointed at by the void
   *data pointer */
int 	E_dequeue( void (* func)( int code, void *data ), int code,
		   void *data );
void	E_delay( sp_time t );

int	E_attach_fd( int fd, int fd_type,
		     void (* func)( int fd, int code, void *data), int code,
		     void *data, int priority );
int 	E_detach_fd( int fd, int fd_type );
int 	E_set_active_threshold( int priority );
int     E_activate_fd( int fd, int fd_type );
int     E_deactivate_fd( int fd, int fd_type );
int	E_num_active( int priority );

void 	E_handle_events(void);
void 	E_exit_events(void);

#endif	/* INC_SP_EVENTS */
