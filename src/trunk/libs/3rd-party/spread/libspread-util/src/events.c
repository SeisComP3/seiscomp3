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


/* Must come before any system headers as otherwise it is ignored */
#define _GNU_SOURCE

#include "arch.h"

/* undef redefined variables under windows */
#ifdef ARCH_PC_WIN95
#undef EINTR
#undef EAGAIN
#undef EWOULDBLOCK
#undef EINPROGRESS
#undef EALREADY
#endif

#include <errno.h>

#ifndef	ARCH_PC_WIN95

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#else 	/* ARCH_PC_WIN95 */

#include <winsock.h>
#include <sys/timeb.h>

#endif	/* ARCH_PC_WIN95 */

#include <string.h>
#include "spu_events.h"
#include "spu_objects.h"    /* For memory */
#include "spu_memory.h"     /* for memory */
#include "spu_alarm.h"

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

static sp_time E_get_time_monotonic(void);

static	time_event	*Time_queue;
static	sp_time		Now;

static	fd_queue	Fd_queue[NUM_PRIORITY];
static	fd_set		Fd_mask[NUM_FDTYPES];
static	int		Active_priority;
static	int		Exit_events;

enum ev_type {
    NULL_EVENT_t = 0,
    TIME_EVENT_t,
    FD_EVENT_t,
};

#define EVENT_RECORD_NAMELEN    128

struct event_record {
    sp_time     dur;
    enum ev_type type;
    char        funcname[EVENT_RECORD_NAMELEN];
    fd_event    fev;
    time_event  tev;
};

int     Slow_events_max = 5;
int     Slow_events_active = 0;
static  struct event_record    Slow_events[5];

int 	E_init(void)
{
	int	i,ret;
	
	Time_queue = NULL;

        ret = Mem_init_object(TIME_EVENT, "time_event", sizeof(time_event), 100,0);
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

	E_get_time_monotonic();

	Alarm( EVENTS, "E_init: went ok\n");

	return( 0 );
}

sp_time	E_get_time(void)
{
        sp_time t;

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
        t.sec  = read_time.tv_sec;
        t.usec = read_time.tv_usec;

#else	/* ARCH_PC_WIN95 */

	struct _timeb timebuffer;

	_ftime( &timebuffer );

	t.sec   = timebuffer.time;
	t.usec  = timebuffer.millitm;
	t.usec *= 1000;	

#endif	/* ARCH_PC_WIN95 */
#if 0
	Alarm( EVENTS, "E_get_time: time is (%d, %d)\n", t.sec, t.usec);
#endif
	return ( t );
}

static sp_time E_get_time_monotonic(void)
#ifdef HAVE_CLOCK_GETTIME_CLOCK_MONOTONIC
{
  struct timespec t;

  if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
    Alarm( EXIT, "E_get_time_monotonic: clock_gettime problems: %d '%s'\n", errno, strerror(errno) );
  }

  Now.sec  = t.tv_sec;
  Now.usec = (t.tv_nsec + 500) / 1000;

  return Now;
}
#else
{
  Now = E_get_time();

  return Now;
}
#endif

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

	t_e->t    = E_add_time( E_get_time_monotonic(), delta_time );
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

int 	E_in_queue( void (* func)( int code, void *data ), int code,
		   void *data )
{
	time_event *t_pre;
	time_event *t_ptr;

	if( Time_queue == NULL )
	{
	    Alarm( EVENTS, "E_in_queue: no such event\n" );
		return( 0 );
	}

	if( Time_queue->func == func && 
            Time_queue->data == data &&
            Time_queue->code == code )
	{
		Alarm( EVENTS, "E_in_queue: found event in queue func 0x%x code %d data 0x%x\n",func,code, data);
		return( 1 );
	}

	t_pre = Time_queue;
	while ( t_pre->next != NULL )
	{
		t_ptr = t_pre->next;
		if( t_ptr->func == func && 
                    t_ptr->data == data &&
                    t_ptr->code == code )   
		{
		    Alarm( EVENTS, "E_in_queue: found event in queue func 0x%x code %d data 0x%x\n",func,code, data);
			return(1);
		}
		t_pre = t_ptr;
	}

	Alarm( EVENTS, "E_in_queue: no such event\n" );
	return( 0 );
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


void    E_print_slow_event( struct event_record *ev )
{

    if (ev->type == NULL_EVENT_t)
        return;

    Alarmp( SPLOG_PRINT, SYSTEM, "Slow Event: %s \ttook %d.%06d sec:", &ev->funcname[0], ev->dur.sec, ev->dur.usec);
    if (ev->type == FD_EVENT_t) 
        Alarmp( SPLOG_PRINT | SPLOG_NODATE, SYSTEM, "fd event: fd (%d) type (%d) funcptr (0x%x) code (%d) data ptr (0x%x) active (%d)\n", ev->fev.fd, ev->fev.fd_type, ev->fev.func, ev->fev.code, ev->fev.data, ev->fev.active);
    if (ev->type == TIME_EVENT_t)
        Alarmp( SPLOG_PRINT | SPLOG_NODATE, SYSTEM, "time event: funcptr (0x%x) code (%d) data ptr (0x%x)\n", ev->tev.func, ev->tev.code, ev->tev.data);

    return;
}

void    E_print_slow_events(void)
{
    int i;

    for ( i = 0; i < Slow_events_active; i++ ) {
        E_print_slow_event( &Slow_events[i] );
    }

}

#ifdef DISABLE_FUNCTION_NAME_LOOKUP
void    E_lookup_function_name( void* fptr, char *fname, int fname_len )
{
    snprintf( fname, fname_len -1, "LOOKUP_FAIL_0x%p", fptr);
    /* NOTE: snprintf is safe if fname is too short, the string will be truncated and null terminated */
    return;
}
#else
void    E_lookup_function_name( void* fptr, char *fname, int fname_len )
{
    Dl_info dli;
    int ret, len;

    ret = dladdr(fptr, &dli);

    if (ret == 0) {
        /* Failed call */
        len = snprintf( fname, fname_len -1, "LOOKUP_FAIL_0x%p", fptr);
        /* NOTE: snprintf is safe if fname is too short, the string will be truncated and null terminated */
    } else {
        if (dli.dli_sname == NULL) {
            len = snprintf( fname, fname_len -1, "NO_NAME");
        } else {
            len = strlen(dli.dli_sname);
            strncpy( fname, dli.dli_sname, fname_len - 1);
            if (len >= fname_len) {
                /* function name too long to store, so truncate it */
                fname[fname_len -1] = '\0';
            }
        }
    }
    return;
}
#endif

void    E_time_events( sp_time start, sp_time stop, fd_event *fev, time_event *tev)
{
    sp_time ev_dur;
    int slot,i;


    if ( (fev != NULL && tev != NULL) || (fev == NULL && tev == NULL) ) {
        Alarm( EXIT, "E_time_events: Bug! called with invalid fev (0x%x)  and tev (0x%x) pointers. Exactly one must be non NULL\n", fev, tev);
    }

    ev_dur = E_sub_time( stop, start );
    if ( Slow_events_active != 0 && E_compare_time( ev_dur, Slow_events[Slow_events_active-1].dur) <= 0 ) {
        /* Fast event so skip */
        return;
    } else {
        if ( Slow_events_active == 0 ) {
            slot = 0;
        } else {
            /* this event is slower then at least one current slow_events so it gets added */
            slot = Slow_events_active -1;
            i = slot -1;
            while (i >= 0 && E_compare_time( ev_dur, Slow_events[i].dur) > 0) {
                slot=i;
                i--;
            }
        }
        /* slot is now the slot holding the new location of this slow event. */
        Alarmp( SPLOG_DEBUG, EVENTS, "DEBUG: Currently %d events stored -- Insert slow event (dur %d.%06d) into slot %d. Prev duration %d.%06d\n", Slow_events_active, ev_dur.sec, ev_dur.usec, slot, Slow_events[slot].dur.sec, Slow_events[slot].dur.usec);

        if (slot < Slow_events_max -1)
            memmove( &Slow_events[slot+1], &Slow_events[slot], (Slow_events_max - slot - 1) * sizeof(struct event_record));
        Slow_events[slot].dur = ev_dur;
        if (fev == NULL) {
            Slow_events[slot].type = TIME_EVENT_t;
            E_lookup_function_name( tev->func, &Slow_events[slot].funcname[0], EVENT_RECORD_NAMELEN);
            Slow_events[slot].tev = *tev;
        } else if (tev == NULL) {
            Slow_events[slot].type = FD_EVENT_t;
            E_lookup_function_name( fev->func, &Slow_events[slot].funcname[0], EVENT_RECORD_NAMELEN);
            Slow_events[slot].fev = *fev;
        }

     
        if (Slow_events_active < Slow_events_max)
            Slow_events_active++;

    }

    return;
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
			Alarm( EVENTS, 
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
        sp_time                 ev_start;
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
        start = E_get_time_monotonic();
#endif
	while( Time_queue != NULL )
	{
#ifdef BADCLOCK
		if ( clock_sync >= 0 )
		{
		    E_get_time_monotonic();
		    clock_sync = -20;
		}
#else
                E_get_time_monotonic();
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
                        ev_start = Now;
			temp_ptr->func( temp_ptr->code, temp_ptr->data );
			dispose( temp_ptr );
#ifdef BADCLOCK
			Now = E_add_time( Now, mili_sec );
			clock_sync++;
#else
                        E_get_time_monotonic();
#endif
                        E_time_events( ev_start, Now, NULL, temp_ptr );

                        if (Exit_events) goto end_handler;
		}else{
			timeout = E_sub_time( Time_queue->t, Now );
			break;
		}
	}
        if (timeout.sec < 0 )
                timeout.sec = timeout.usec = 0; /* this can happen until first is unset */
#ifdef TESTTIME
        stop = E_get_time_monotonic();
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
        start = E_get_time_monotonic();
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
#ifdef BADCLOCK
		    Now = E_add_time( Now, mili_sec );
		    clock_sync++;
#else
                    E_get_time_monotonic();
#endif
                    ev_start = Now;
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
                    E_get_time_monotonic();
#endif
                    E_time_events(ev_start, Now, &(Fd_queue[i].events[j]), NULL);

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
        stop = E_get_time_monotonic();
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
#ifdef BADCLOCK
                Now = E_add_time( Now, mili_sec );
                clock_sync++;
#else
                E_get_time_monotonic();
#endif
                ev_start = Now;
	 	Fd_queue[LOW_PRIORITY].events[j].func( 
				Fd_queue[LOW_PRIORITY].events[j].fd,
				Fd_queue[LOW_PRIORITY].events[j].code,
				Fd_queue[LOW_PRIORITY].events[j].data );
		num_set--;
#ifdef BADCLOCK
		Now = E_add_time( Now, mili_sec );
		clock_sync++;
#else
                E_get_time_monotonic();
#endif

                E_time_events(ev_start, Now, &(Fd_queue[LOW_PRIORITY].events[j]), NULL);

                if (Exit_events) goto end_handler;
		break;
	    }
	}	
#ifdef TESTTIME
        start = E_get_time_monotonic();
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
