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
 *  Copyright (C) 1993-2006 Spread Concepts LLC <info@spreadconcepts.com>
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

/* undef redefined variables under windows */
#ifdef ARCH_PC_WIN95
#undef EINTR
#undef EAGAIN
#undef EWOULDBLOCK
#undef EINPROGRESS
#endif
#include <errno.h>

#ifndef	ARCH_PC_WIN95

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#else 	/* ARCH_PC_WIN95 */

#include <winsock.h>
#include <sys/timeb.h>

#endif	/* ARCH_PC_WIN95 */

#include <string.h>
#include "sp_events.h"
#include "objects.h"    /* For memory */
#include "memory.h"     /* for memory */
#include "alarm.h"

typedef	struct dummy_t_event {
	sp_time		t;
	void		(* func)( int code, void *data );
        int             code;
        void            *data;
	struct dummy_t_event	*next;
} time_event;

typedef struct dummy_fd_event {
	int		fd;
	int		fd_type;
	void		(* func)( mailbox mbox, int code, void *data );
	int		code;
        void            *data;
        int             active; /* true if active, false if inactive */
} fd_event;

typedef struct dummy_fd_queue {
	int		num_fds;
        int             num_active_fds;
	fd_event	events[MAX_FD_EVENTS];
} fd_queue;

static	time_event	*Time_queue;
static	sp_time		Now;

static	fd_queue	Fd_queue[NUM_PRIORITY];
static	fd_set		Fd_mask[NUM_FDTYPES];
static	int		Active_priority;
static	int		Exit_events;

int 	E_init(void)
{
	int	i,ret;
	
	Time_queue = NULL;

        ret = Mem_init_object(TIME_EVENT, sizeof(time_event), 100,0);
        if (ret < 0)
        {
                Alarm(EXIT, "E_Init: Failure to Initialize TIME_EVENT memory objects\n");
        }

	for ( i=0; i < NUM_PRIORITY; i++ )
        {
		Fd_queue[i].num_fds = 0;
                Fd_queue[i].num_active_fds = 0;
        }
	for ( i=0; i < NUM_FDTYPES; i++ )
        {
		FD_ZERO( &Fd_mask[i] );
        }
	Active_priority = LOW_PRIORITY;

	E_get_time();

	Alarm( EVENTS, "E_init: went ok\n");

	return( 0 );
}

sp_time	E_get_time(void)
{
#ifndef	ARCH_PC_WIN95
        struct timeval  read_time;

#if HAVE_STRUCT_TIMEZONE
        struct timezone dummy_tz;
#else
	sp_time		dummy_tz;
#endif
	int		ret;

	ret = gettimeofday( &read_time, &dummy_tz );
	if ( ret < 0 ) Alarm( EXIT, "E_get_time: gettimeofday problems.\n" );
        Now.sec = read_time.tv_sec;
        Now.usec = read_time.tv_usec;

#else	/* ARCH_PC_WIN95 */

	struct _timeb timebuffer;

	_ftime( &timebuffer );

	Now.sec = timebuffer.time;
	Now.usec= timebuffer.millitm;
	Now.usec= Now.usec * 1000;

#endif	/* ARCH_PC_WIN95 */
#if 0
	Alarm( EVENTS, "E_get_time: time is (%d, %d)\n", Now.sec, Now.usec);
#endif
	return ( Now );
}

sp_time	E_sub_time( sp_time t, sp_time delta_t )
{
	sp_time	res;

	res.sec  = t.sec  - delta_t.sec;
	res.usec = t.usec - delta_t.usec;
	if ( res.usec < 0 )
	{
		res.usec = res.usec + 1000000;
		res.sec--;
	} 
	if ( res.sec < 0 ) Alarm( EVENTS, "E_sub_time: negative time result.\n");
	return ( res );
}

sp_time	E_add_time( sp_time t, sp_time delta_t )
{
	sp_time	res;

	res.sec  = t.sec  + delta_t.sec;
	res.usec = t.usec + delta_t.usec;
	if ( res.usec > 1000000 )
	{
		res.usec = res.usec - 1000000;
		res.sec++;
	}
	return ( res );
}

int	E_compare_time( sp_time t1, sp_time t2 )
{
	if	( t1.sec  > t2.sec  ) return (  1 );
	else if ( t1.sec  < t2.sec  ) return ( -1 );
	else if ( t1.usec > t2.usec ) return (  1 );
	else if ( t1.usec < t2.usec ) return ( -1 );
	else			      return (  0 );
}

int 	E_queue( void (* func)( int code, void *data ), int code, void *data,
		 sp_time delta_time )
{
	time_event *t_pre;
	time_event *t_post;
	time_event *t_e;
	int	   inserted;
	int	   deleted;
	int	   compare;

	t_e       = new( TIME_EVENT );

	t_e->t    = E_add_time( E_get_time(), delta_time );
	t_e->func = func;
        t_e->code = code;
        t_e->data = data;
	deleted   = 0;
	inserted  = 0;

	if( Time_queue != NULL )
	{
		if( Time_queue->func == t_e->func && 
                    Time_queue->data == t_e->data &&
                    Time_queue->code == t_e->code )
		{
			t_pre = Time_queue;
			Time_queue = Time_queue->next;
			dispose( t_pre );
			deleted = 1;
			Alarm( EVENTS, "E_queue: dequeued a (first) simillar event\n" );
		}
	}
	if( Time_queue == NULL )
	{
		t_e->next  = NULL;
		Time_queue = t_e;
		Alarm( EVENTS, "E_queue: (only) event queued func 0x%x code %d data 0x%x in future (%u:%u)\n",t_e->func,t_e->code, t_e->data, delta_time.sec, delta_time.usec );
		return( 0 );
	}else{
		compare = E_compare_time ( t_e->t, Time_queue->t );
		if( compare < 0 )
		{
			t_e->next   = Time_queue;
			Time_queue  = t_e;
			inserted    = 1;
			Alarm( EVENTS, "E_queue: (first) event queued func 0x%x code %d data 0x%x in future (%u:%u)\n",t_e->func,t_e->code, t_e->data, delta_time.sec,delta_time.usec );
		}
	}
	t_pre    = Time_queue ; 
	t_post   = Time_queue->next;
	while ( t_post != NULL && ( !inserted || !deleted ) )
	{
		if( t_post->func == t_e->func && 
                    t_post->data == t_e->data &&
                    t_post->code == t_e->code )
		{
			t_pre->next = t_post->next;
			dispose( t_post );
			t_post = t_pre->next;
			deleted = 1;
			Alarm( EVENTS, "E_queue: dequeued a simillar event\n" );
			continue;
		}

		if ( !inserted )
		{
			compare = E_compare_time ( t_e->t, t_post->t );
			if( compare < 0 )
			{
				t_pre->next = t_e;
				t_e->next   = t_post;
				inserted    = 1;
				Alarm( EVENTS, "E_queue: event queued for func 0x%x code %d data 0x%x in future (%u:%u)\n",t_e->func,t_e->code, t_e->data, delta_time.sec, delta_time.usec );
			}
		}

		t_pre  = t_post;
		t_post = t_post->next;
	}

	if( !inserted )
	{
		t_pre->next = t_e;
		t_e->next   = NULL;
		Alarm( EVENTS, "E_queue: (last) event queued func 0x%x code %d data 0x%x in future (%u:%u)\n",t_e->func,t_e->code, t_e->data, delta_time.sec,delta_time.usec );
	}

	return( 0 );
}

int 	E_dequeue( void (* func)( int code, void *data ), int code,
		   void *data )
{
	time_event *t_pre;
	time_event *t_ptr;

	if( Time_queue == NULL )
	{
		Alarm( EVENTS, "E_dequeue: no such event\n" );
		return( -1 );
	}

	if( Time_queue->func == func && 
            Time_queue->data == data &&
            Time_queue->code == code )
	{
		t_ptr = Time_queue;
		Time_queue = Time_queue->next;
		dispose( t_ptr );
		Alarm( EVENTS, "E_dequeue: first event dequeued func 0x%x code %d data 0x%x\n",func,code, data);
		return( 0 );
	}

	t_pre = Time_queue;
	while ( t_pre->next != NULL )
	{
		t_ptr = t_pre->next;
		if( t_ptr->func == func && 
                    t_ptr->data == data &&
                    t_ptr->code == code )   
		{
			t_pre->next = t_ptr->next;
			dispose( t_ptr );
			Alarm( EVENTS, "E_dequeue: event dequeued func 0x%x code %d data 0x%x\n",func,code, data);
			return( 0 );
		}
		t_pre = t_ptr;
	}

	Alarm( EVENTS, "E_dequeue: no such event\n" );
	return( -1 );
}

void	E_delay( sp_time t )
{
	struct timeval 	tmp_t;

	tmp_t.tv_sec = t.sec;
	tmp_t.tv_usec = t.usec;

#ifndef ARCH_PC_WIN95
        if (select(0, NULL, NULL, NULL, &tmp_t ) < 0)
        {
                Alarm( EVENTS, "E_delay: select delay returned error: %s\n", strerror(errno));
        }
#else  /* ARCH_PC_WIN95 */
        SleepEx( tmp_t.tv_sec*1000+tmp_t.tv_usec/1000, 0 );
#endif /* ARCH_PC_WIN95 */   

}
	
int	E_attach_fd( int fd, int fd_type,
		     void (* func)( mailbox mbox, int code, void *data ),
		     int code, void *data, int priority )
{
	int	num_fds;
	int	j;

	if( priority < 0 || priority > NUM_PRIORITY )
	{
		Alarm( PRINT, "E_attach_fd: invalid priority %d for fd %d with fd_type %d\n", priority, fd, fd_type );
		return( -1 );
	}
	if( fd_type < 0 || fd_type > NUM_FDTYPES )
	{
		Alarm( PRINT, "E_attach_fd: invalid fd_type %d for fd %d with priority %d\n", fd_type, fd, priority );
		return( -1 );
	}
#ifndef	ARCH_PC_WIN95
	/* Windows bug: Reports FD_SETSIZE of 64 but select works on all
	 * fd's even ones with numbers greater then 64.
	 */
        if( fd < 0 || fd > FD_SETSIZE )
        {
                Alarm( PRINT, "E_attach_fd: invalid fd %d (max %d) with fd_type %d with priority %d\n", fd, FD_SETSIZE, fd_type, priority );
                return( -1 );
        }
#endif
	for( j=0; j < Fd_queue[priority].num_fds; j++ )
	{
		if( ( Fd_queue[priority].events[j].fd == fd ) && ( Fd_queue[priority].events[j].fd_type == fd_type ) )
		{
			Fd_queue[priority].events[j].func = func;
			Fd_queue[priority].events[j].code = code;
                        Fd_queue[priority].events[j].data = data;
                        if ( !(Fd_queue[priority].events[j].active) )
                                Fd_queue[priority].num_active_fds++;
                        Fd_queue[priority].events[j].active = TRUE;
			Alarm( PRINT, 
				"E_attach_fd: fd %d with type %d exists & replaced & activated\n", fd, fd_type );
			return( 1 );
		}
	}
	num_fds = Fd_queue[priority].num_fds;

        if ( num_fds == MAX_FD_EVENTS ) {
                Alarm( PRINT, "E_attach_fd: Reached Maximum number of events. Recompile with larger MAX_FD_EVENTS\n");
                return( -1 );
        }
	Fd_queue[priority].events[num_fds].fd	   = fd;
	Fd_queue[priority].events[num_fds].fd_type = fd_type;
	Fd_queue[priority].events[num_fds].func	   = func;
	Fd_queue[priority].events[num_fds].code    = code;
        Fd_queue[priority].events[num_fds].data    = data;
        Fd_queue[priority].events[num_fds].active  = TRUE;
	Fd_queue[priority].num_fds++;
        Fd_queue[priority].num_active_fds++;
	if( Active_priority <= priority ) FD_SET( fd, &Fd_mask[fd_type] );

	Alarm( EVENTS, "E_attach_fd: fd %d, fd_type %d, code %d, data 0x%x, priority %d Active_priority %d\n",
		fd, fd_type, code, data, priority, Active_priority );

	return( 0 );
}

int 	E_detach_fd( int fd, int fd_type )
{
	int	i,j;
	int	found;

	if( fd_type < 0 || fd_type > NUM_FDTYPES )
	{
		Alarm( PRINT, "E_detach_fd: invalid fd_type %d for fd %d\n", fd_type, fd );
		return( -1 );
	}

	found = 0;
	for( i=0; i < NUM_PRIORITY; i++ )
	    for( j=0; j < Fd_queue[i].num_fds; j++ )
	    {
		if( ( Fd_queue[i].events[j].fd == fd ) && ( Fd_queue[i].events[j].fd_type == fd_type ) )
		{
                        if (Fd_queue[i].events[j].active)
                                Fd_queue[i].num_active_fds--;
			Fd_queue[i].num_fds--;
			Fd_queue[i].events[j] = Fd_queue[i].events[Fd_queue[i].num_fds];

			FD_CLR( fd, &Fd_mask[fd_type] );
			found = 1;

			break; /* from the j for only */
		}
	    }

	if( ! found ) return( -1 );

	return( 0 );
}

int     E_deactivate_fd( int fd, int fd_type )
{
	int	i,j;
	int	found;

	if( fd_type < 0 || fd_type > NUM_FDTYPES )
	{
		Alarm( PRINT, "E_deactivate_fd: invalid fd_type %d for fd %d\n", fd_type, fd );
		return( -1 );
	}

	found = 0;

	for( i=0; i < NUM_PRIORITY; i++ )
	    for( j=0; j < Fd_queue[i].num_fds; j++ )
	    {
		if( ( Fd_queue[i].events[j].fd == fd ) && ( Fd_queue[i].events[j].fd_type == fd_type ) )
		{
                        if (Fd_queue[i].events[j].active)
                                Fd_queue[i].num_active_fds--;
                        Fd_queue[i].events[j].active = FALSE;
			FD_CLR( fd, &Fd_mask[fd_type] );
			found = 1;

			break; /* from the j for only */
		}
	    }

	if( ! found ) return( -1 );
	return( 0 );
}

int     E_activate_fd( int fd, int fd_type )
{
	int	i,j;
	int	found;

	if( fd_type < 0 || fd_type > NUM_FDTYPES )
	{
		Alarm( PRINT, "E_activate_fd: invalid fd_type %d for fd %d\n", fd_type, fd );
		return( -1 );
	}

	found = 0;

	for( i=0; i < NUM_PRIORITY; i++ )
	    for( j=0; j < Fd_queue[i].num_fds; j++ )
	    {
		if( ( Fd_queue[i].events[j].fd == fd ) && ( Fd_queue[i].events[j].fd_type == fd_type ) )
		{
                        if ( !(Fd_queue[i].events[j].active) )
                                Fd_queue[i].num_active_fds++;
                        Fd_queue[i].events[j].active = TRUE;
			if( i >= Active_priority ) FD_SET( fd, &Fd_mask[ fd_type ] );
			found = 1;

			break; /* from the j for only */
		}
	    }

	if( ! found ) return( -1 );
	return( 0 );
}

int 	E_set_active_threshold( int priority )
{
	int	fd_type;
	int	i,j;

	if( priority < 0 || priority > NUM_PRIORITY )
	{
		Alarm( PRINT, "E_set_active_threshold: invalid priority %d\n", priority );
		return( -1 );
	}

	if( priority == Active_priority ) return( priority );

	Active_priority = priority;
	for ( i=0; i < NUM_FDTYPES; i++ )
        {
		FD_ZERO( &Fd_mask[i] );
        }

	for( i = priority; i < NUM_PRIORITY; i++ )
	    for( j=0; j < Fd_queue[i].num_fds; j++ )
	    {
		fd_type = Fd_queue[i].events[j].fd_type;
                if (Fd_queue[i].events[j].active)
                	FD_SET( Fd_queue[i].events[j].fd, &Fd_mask[fd_type] );
	    }

	Alarm( EVENTS, "E_set_active_threshold: changed to %d\n",Active_priority);

	return( priority );
}

int	E_num_active( int priority )
{
	if( priority < 0 || priority > NUM_PRIORITY )
	{
		Alarm( PRINT, "E_num_active: invalid priority %d\n", priority );
		return( -1 );
	}
	return( Fd_queue[priority].num_active_fds );
}

void 	E_handle_events(void)
{
static	int			Round_robin	= 0;
static	const sp_time		long_timeout 	= { 10000,    0};
static  const sp_time           zero_sec        = {     0,    0};
#ifdef  BADCLOCK
static	const sp_time		mili_sec 	= {     0, 1000};
	int			clock_sync;
#endif
	int			num_set;
	int			treated;
	int			fd;
	int			fd_type;
	int			i,j;
	sp_time			timeout;
        struct timeval          sel_timeout, wait_timeout;
	fd_set			current_mask[NUM_FDTYPES];
	time_event		*temp_ptr;
        int                     first=1;
#ifdef TESTTIME
        sp_time         	tmp_late,start,stop,req_time;       /* DEBUGGING */
#endif
#ifdef BADCLOCK
    clock_sync = 0;
#endif
    for( Exit_events = 0 ; !Exit_events ; )
    {
	Alarm( EVENTS, "E_handle_events: next event \n");

	/* Handle time events */
	timeout = long_timeout;
#ifdef TESTTIME
        start = E_get_time();
#endif
	while( Time_queue != NULL )
	{
#ifdef BADCLOCK
		if ( clock_sync >= 0 )
		{
		    E_get_time();
		    clock_sync = -20;
		}
#else
                E_get_time();
#endif
		if ( !first && E_compare_time( Now, Time_queue->t ) >= 0 )
		{
#ifdef TESTTIME
                        tmp_late = E_sub_time( Now, Time_queue->t );
#endif
			temp_ptr = Time_queue;
			Time_queue = Time_queue->next;
			Alarm( EVENTS, "E_handle_events: exec time event \n");
#ifdef TESTTIME 
                        Alarm( DEBUG, "Events: TimeEv is %d %d late\n",tmp_late.sec, tmp_late.usec); 
#endif
			temp_ptr->func( temp_ptr->code, temp_ptr->data );
			dispose( temp_ptr );
#ifdef BADCLOCK
			Now = E_add_time( Now, mili_sec );
			clock_sync++;
#else
                        E_get_time();
#endif
                        if (Exit_events) goto end_handler;
		}else{
			timeout = E_sub_time( Time_queue->t, Now );
			break;
		}
	}
        if (timeout.sec < 0 )
                timeout.sec = timeout.usec = 0; /* this can happen until first is unset */
#ifdef TESTTIME
        stop = E_get_time();
        tmp_late = E_sub_time(stop, start);
        Alarm(DEBUG, "Events: TimeEv's took %d %d to handle\n", tmp_late.sec, tmp_late.usec); 
#endif
	/* Handle fd events   */
	for( i=0; i < NUM_FDTYPES; i++ )
	{
		current_mask[i] = Fd_mask[i];
	}
	Alarm( EVENTS, "E_handle_events: poll select\n");
#ifdef TESTTIME
        req_time = zero_sec;
#endif
        wait_timeout.tv_sec = zero_sec.sec;
        wait_timeout.tv_usec = zero_sec.usec;
	num_set = select( FD_SETSIZE, &current_mask[READ_FD], &current_mask[WRITE_FD], &current_mask[EXCEPT_FD], 
			  &wait_timeout );
	if (num_set == 0 && !Exit_events)
	{
#ifdef BADCLOCK
		clock_sync = 0;
#endif
		for( i=0; i < NUM_FDTYPES; i++ )
		{
			current_mask[i] = Fd_mask[i];
		}
		Alarm( EVENTS, "E_handle_events: select with timeout (%d, %d)\n",
			timeout.sec,timeout.usec );
#ifdef TESTTIME
                req_time = E_add_time(req_time, timeout);
#endif
                sel_timeout.tv_sec = timeout.sec;
                sel_timeout.tv_usec = timeout.usec;
		num_set = select( FD_SETSIZE, &current_mask[READ_FD], &current_mask[WRITE_FD], 
				  &current_mask[EXCEPT_FD], &sel_timeout );
	}
#ifdef TESTTIME
        start = E_get_time();
        tmp_late = E_sub_time(start, stop);
        Alarm( DEBUG, "Events: Waiting for fd or timout took %d %d asked for %d %d\n", tmp_late.sec, tmp_late.usec, req_time.sec, req_time.usec);
#endif
	/* Handle all high and medium priority fd events */
	for( i=NUM_PRIORITY-1,treated=0; 
	     i > LOW_PRIORITY && num_set > 0 && !treated;
	     i-- )
	{
	    for( j=0; j < Fd_queue[i].num_fds && num_set > 0; j++ )
	    {
		fd      = Fd_queue[i].events[j].fd;
		fd_type = Fd_queue[i].events[j].fd_type;
		if( FD_ISSET( fd, &current_mask[fd_type] ) )
		{
		    Alarm( EVENTS, "E_handle_events: exec handler for fd %d, fd_type %d, priority %d\n", 
					fd, fd_type, i );
		    Fd_queue[i].events[j].func( 
				Fd_queue[i].events[j].fd,
				Fd_queue[i].events[j].code,
				Fd_queue[i].events[j].data );
		    treated = 1;
		    num_set--;
#ifdef BADCLOCK
		    Now = E_add_time( Now, mili_sec );
		    clock_sync++;
#else
                    E_get_time();
#endif
                    if (Exit_events) goto end_handler;
		}
	    }
	}
        /* Don't handle timed events until all non-low-priority fd events have been handled 
         * FIXME: This may or may not be right. If continual high priority events occur, then
         * timed events will starve, I'm not sure if that is better then what we have. We 
         * could also set first=0 no matter what after trying the high events once, then
         * they will get a shot first, but after that timed events will also be handled.
         */
        if (!treated)
                first = 0;

#ifdef TESTTIME
        stop = E_get_time();
        tmp_late = E_sub_time(stop, start);
        Alarm(DEBUG, "Events: High & Med took %d %d time to handle\n", tmp_late.sec, tmp_late.usec);
#endif
	/* Handle one low priority fd event. 
           However, verify that Active_priority still allows LOW_PRIORITY events. 
           Active_priority can change because of calls to E_set_threshold() during the current select loop.
        */
	for( i=0; i < Fd_queue[LOW_PRIORITY].num_fds 
                     && num_set > 0
                     && Active_priority == LOW_PRIORITY; 
             i++ )
	{
	    j = ( i + Round_robin ) % Fd_queue[LOW_PRIORITY].num_fds;
	    fd      = Fd_queue[LOW_PRIORITY].events[j].fd;
	    fd_type = Fd_queue[LOW_PRIORITY].events[j].fd_type;
	    if( FD_ISSET( fd, &current_mask[fd_type] ) )
	    {
		Round_robin = ( j + 1 ) % Fd_queue[LOW_PRIORITY].num_fds;

		Alarm( EVENTS , "E_handle_events: exec ext fd event \n");
	 	Fd_queue[LOW_PRIORITY].events[j].func( 
				Fd_queue[LOW_PRIORITY].events[j].fd,
				Fd_queue[LOW_PRIORITY].events[j].code,
				Fd_queue[LOW_PRIORITY].events[j].data );
		num_set--;
#ifdef BADCLOCK
		Now = E_add_time( Now, mili_sec );
		clock_sync++;
#else
                E_get_time();
#endif
                if (Exit_events) goto end_handler;
		break;
	    }
	}	
#ifdef TESTTIME
        start = E_get_time();
        tmp_late = E_sub_time(start, stop);
        Alarm(DEBUG, "Events: Low priority took %d %d to handle\n", tmp_late.sec, tmp_late.usec);
#endif
    }
 end_handler:
    /* Clean up data structures for exit OR restart of handler loop */
    /* Actually nothing needs to be cleaned up to allow E_handle_events()
     * to be called again. The events are still registered (or not registered)
     * and the only state for the actual events loop is Exit_events which is reset
     * in the for loop.
     */

    return;
}

void 	E_exit_events(void)
{
	Alarm( EVENTS, "E_exit_events:\n");
	Exit_events = 1;
}
