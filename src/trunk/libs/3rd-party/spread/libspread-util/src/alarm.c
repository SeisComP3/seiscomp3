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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* undef redefined variables under windows */
#ifdef ARCH_PC_WIN95
#  undef EINTR
#  undef EAGAIN
#  undef EWOULDBLOCK
#  undef EINPROGRESS
#  undef EALREADY
#  ifndef va_copy
#    define va_copy(d,s) (d) = (s)
#  endif
#endif
#include <errno.h>

#ifdef HAVE_GOOD_VARGS
#  include <stdarg.h>
#endif

#include "spu_alarm.h"
#include "spu_events.h"

#define MAX_ALARM_MESSAGE_BUF 256

static const char Alarm_Warning_High_Res[]  = "*** WARNING *** Alarmp: High precision timestamp output failed!\n";
static const char Alarm_Warning_Timestamp[] = "*** WARNING *** Alarmp: Timestamp didn't fit in default size buffer!\n";
static const char Alarm_Warning_Alloc[]     = "*** WARNING *** Alarmp: message longer than default; dynamically allocated alarm string!\n";
static const char Alarm_Warning_Truncate[]  = "*** WARNING *** Alarmp: message longer than default and dynamic alloc failed -- alarm message truncated!\n";
static const char Alarm_Warning_Realtime[]  = "*** WARNING *** Alarmp: real time print handler failed!\n";

#ifdef USE_THREADED_ALARM
static void Threaded_Alarm_Append(const char *str, size_t str_len);
static void Threaded_Alarm_Exit(void);
#endif

static int32    Alarm_type_mask = PRINT | EXIT  ;
static int16    Alarm_cur_priority = SPLOG_DEBUG ;

static alarm_realtime_handler *Alarm_realtime_print_handler = NULL;

static char    *Alarm_timestamp_format = NULL;
static int       Alarm_timestamp_high_res = 0;

static const char *DEFAULT_TIMESTAMP_FORMAT = "%m/%d/%y %H:%M:%S"; 
/* alternate longer default timestamp "[%a %d %b %Y %H:%M:%S]"; */

static int      AlarmInteractiveProgram = FALSE;

/* priority_level_active returns true if the priority level is active (i.e. should be printed)
 * given the currently allowed priority levels.
 */
static bool priority_level_active( int16 priority )
{
    return( Alarm_cur_priority <= ( priority & SPLOG_PRIORITY_FIELDS ) );
}

/* is_priority_set() checks whether the specific priority_to_check value is set
 * in the 'priority' variable passed in.
 * If value is set, then it returns true, otherwise it returns false. 
 */
static bool is_priority_set( int16 priority, int16 priority_to_check )
{
    return( (priority & SPLOG_PRIORITY_FIELDS) == priority_to_check );
}

/* is_priority_flag_active() checks whether the specific priority_flag is set
 * in the 'priority' variable passed in.
 * If flag is set, then it returns true, otherwise it returns false. 
 */
static bool is_priority_flag_active( int16 priority, int16 priority_flag )
{
    return( (priority & SPLOG_PRIORITY_FLAGS) & priority_flag );
}

static void Output_Msg(const char *msg, int msg_len)
{
#ifndef USE_THREADED_ALARM
    fwrite(msg, sizeof(char), msg_len, stdout);
#else
    Threaded_Alarm_Append(msg, msg_len);
#endif
}

#ifdef HAVE_GOOD_VARGS

static void Internal_Alarmp(int16 priority, int32 mask, char *message, va_list ap)
{
    /* log event if in mask and of higher priority, or if FATAL event, always log */

    if (((Alarm_type_mask & mask) != 0 && priority_level_active(priority)) || is_priority_set(priority, SPLOG_FATAL)) {

        char      buf[MAX_ALARM_MESSAGE_BUF];
        char     *alloc_buf = NULL;
        char     *tot_ptr   = buf;
        int       tot_len   = 0;
        char     *ts_ptr    = buf;
        int       ts_len    = 0;
        char     *msg_ptr   = buf;
        int       msg_len   = 0;
        int       tmp;
        time_t    time_now;
        struct tm tm_now;
        va_list   ap_copy;
	sp_time   t;

        if (Alarm_timestamp_format != NULL && !is_priority_flag_active(priority, SPLOG_NODATE)) {

	    t        = E_get_time();
	    time_now = t.sec;
	    
#ifdef HAVE_LOCALTIME_R
            localtime_r(&tm_now, &time_now);
#else
            tm_now = *localtime(&time_now);  /* NOTE: not thread safe; but worst case is we get an incorrect timestamp */
#endif

            ts_len = (int) strftime(ts_ptr, MAX_ALARM_MESSAGE_BUF - 1, Alarm_timestamp_format, &tm_now);  /* reserve 1 char for space appended below */

	    /* ensure ts_len in half open range [0, MAX_ALARM_MESSAGE_BUF - 1) */

	    if (ts_len >= MAX_ALARM_MESSAGE_BUF - 1) {
	        ts_len = MAX_ALARM_MESSAGE_BUF - 2;
	    }

	    if (ts_len < 0) {
	        ts_len = 0;
	    }

	    ts_ptr[ts_len] = '\0';

            if (ts_len != 0) {  /* strftime output successfully fit into ts_ptr; optionally append sub-second precision */

	        if ( Alarm_timestamp_high_res ) {

		    tmp = snprintf(ts_ptr + ts_len, MAX_ALARM_MESSAGE_BUF - 1 - ts_len, ".%06lu", t.usec);  /* always nul terminates */

		    if (tmp != 7) {
		        Output_Msg(Alarm_Warning_High_Res, sizeof(Alarm_Warning_High_Res) - 1);
			tmp = 0;
		    }

		    if ((ts_len += tmp) >= MAX_ALARM_MESSAGE_BUF - 1) {
		        Output_Msg(Alarm_Warning_Timestamp, sizeof(Alarm_Warning_Timestamp) - 1);
		        ts_len = MAX_ALARM_MESSAGE_BUF - 2;  /* adjust ts_len to reflect snprintf truncation */
		    }
		}

                ts_ptr[ts_len]   = ' ';        /* append space for single alarm string */
		ts_ptr[++ts_len] = '\0';

                msg_ptr = ts_ptr + ts_len;
		tot_len = ts_len;

	    } else {
	        Output_Msg(Alarm_Warning_Timestamp, sizeof(Alarm_Warning_Timestamp) - 1);
	    }
        }

        va_copy(ap_copy, ap);                               /* make a copy of param list in case of vsnprintf truncation */
        msg_len  = vsnprintf(msg_ptr, MAX_ALARM_MESSAGE_BUF - tot_len, message, ap);
        tot_len += msg_len;

        if (tot_len >= MAX_ALARM_MESSAGE_BUF) {             /* alarm string doesn't fit in buf; dynamically allocate big enough buffer */

            if ((alloc_buf = (char*) malloc(sizeof(Alarm_Warning_Alloc) - 1 + tot_len + 1)) != NULL) {

                tot_ptr  = alloc_buf;
                tot_len  = sizeof(Alarm_Warning_Alloc) - 1;
		memcpy(tot_ptr, Alarm_Warning_Alloc, tot_len);                            /* prepend dynamic allocation warning string */

                memcpy(tot_ptr + tot_len, ts_ptr, ts_len);                                /* copy timestamp after warning */
                ts_ptr   = tot_ptr + tot_len;
		tot_len += ts_len;

                msg_ptr  = ts_ptr + ts_len;
                tmp      = vsnprintf(msg_ptr, msg_len + 1, message, ap_copy);             /* write msg after timestamp */
                assert(tmp == msg_len);
                msg_len  = tmp;
		tot_len += msg_len;

            } else {                                                                      /* dynamic alloc failed; overwrite warning + truncate msg in buf */

	        memcpy(tot_ptr + MAX_ALARM_MESSAGE_BUF - sizeof(Alarm_Warning_Truncate), Alarm_Warning_Truncate, sizeof(Alarm_Warning_Truncate));
                msg_len          = MAX_ALARM_MESSAGE_BUF - ts_len - 1;
		tot_len          = MAX_ALARM_MESSAGE_BUF - 1;
		tot_ptr[tot_len] = '\0';
            }
        }

        va_end(ap_copy);                                    /* clean up ap copy */

	Output_Msg(tot_ptr, tot_len);

        if (ts_len != 0) {
            ts_ptr[--ts_len] = '\0';                        /* overwrite appended space between timestamp and msg with nul terminator */
        }

        if (Alarm_realtime_print_handler != NULL && 
            (is_priority_flag_active(priority, SPLOG_REALTIME) || is_priority_set(priority, SPLOG_FATAL))) {

            tmp = Alarm_realtime_print_handler(priority, mask, ts_ptr, ts_len, msg_ptr, msg_len);

            if (tmp) {
	        Output_Msg(Alarm_Warning_Realtime, sizeof(Alarm_Warning_Realtime) - 1);
            }
        }

        if (alloc_buf != NULL) {                            /* clean up any dynamically allocated buffer */
            free(alloc_buf);
        }
    }

    if ((EXIT & mask) != 0 || is_priority_set(priority, SPLOG_FATAL)) {

#ifndef USE_THREADED_ALARM
        fprintf(stdout, "Exit caused by Alarm(EXIT)\n");
#ifndef ARCH_PC_WIN95
        abort();
#endif
        exit(0);
#else
        Threaded_Alarm_Exit();
#endif

    }
}

void Alarmp( int16 priority, int32 mask, char *message, ... )
{
        va_list ap;

        va_start(ap, message);
        Internal_Alarmp(priority, mask, message, ap);
        va_end(ap);
}

/* For backwards compatibility while moving all Alarm calls over, this provides
 * the old interface and logs them as WARNING events.
 */

void Alarm( int32 mask, char *message, ... )
{
        va_list ap;

        va_start(ap, message);
        Internal_Alarmp(SPLOG_WARNING, mask, message, ap);
        va_end(ap);
}

#else

void Alarm( int32 mask, char *message, 
            void *ptr1, void *ptr2, void *ptr3, void *ptr4, 
            void *ptr5, void *ptr6, void *ptr7, void *ptr8,
            void *ptr9, void *ptr10, void*ptr11, void *ptr12,
            void *ptr13, void *ptr14, void *ptr15, void *ptr16,
            void *ptr17, void *ptr18, void *ptr19, void *ptr20)
{
        if ( ( Alarm_type_mask & mask ) && priority_level_active(SPLOG_WARNING) )
        {
            if ( Alarm_timestamp_format != NULL )
            {
		char timestamp[42];
		struct tm *tm_now;
		time_t time_now;
		size_t length;
		size_t length2;
		sp_time t = E_get_time();

		time_now = t.sec;
		tm_now   = localtime(&time_now);
		length   = strftime(timestamp, sizeof(timestamp) - 1, Alarm_timestamp_format, tm_now);  /* -1 reserves a char for appended ' ' below */

		if (length != 0) {

			if ( Alarm_timestamp_high_res && length <= sizeof(timestamp) - 1 ) {

				if ((length2 = snprintf(timestamp + length, sizeof(timestamp) - 1 - length, ".%06lu", t.usec)) < sizeof(timestamp) - 1 - length) {
					length += length2;
				}
				/* else attempted print will be overwritten immediately below by appending ' ' */
			}

			timestamp[length]   = ' ';
			timestamp[++length] = '\0';
			fwrite(timestamp, sizeof(char), length, stdout);
			timestamp[--length] = '\0';
		}
            }
            printf(message, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7, ptr8, ptr9, ptr10, ptr11, ptr12, ptr13, ptr14, ptr15, ptr16, ptr17, ptr18, ptr19, ptr20 );

        }
        if ( EXIT & mask )
        {
            printf("Exit caused by Alarm(EXIT)\n");
#ifndef ARCH_PC_WIN95
            abort();
#endif
            exit( 0 );
        }
}

#endif /* HAVE_GOOD_VARGS */

void Alarm_set_interactive(void) 
{
        AlarmInteractiveProgram = TRUE;
}

int  Alarm_get_interactive(void)
{
        return(AlarmInteractiveProgram);
}

void Alarm_set_output(char *filename) 
{
        FILE *newfile;
        newfile = freopen(filename, "a", stdout);
        if ( NULL == newfile ) {
                printf("failed to open file (%s) for stdout. Error: %d\n", filename, errno);
        }
        newfile = freopen(filename, "a", stderr);
        if ( NULL == newfile ) {
                printf("failed to open file (%s) for stderr. Error: %d\n", filename, errno);
        }
        setvbuf(stderr, (char *)0, _IONBF, 0);
        setvbuf(stdout, (char *)0, _IONBF, 0);
}

static void Alarm_enable_timestamp_intrnl(const char *format, int high_res)
{
        static char _local_timestamp[40];
        size_t len;

	if (format) {
		strncpy(_local_timestamp, format, sizeof(_local_timestamp) - 1);

	} else {
		strncpy(_local_timestamp, DEFAULT_TIMESTAMP_FORMAT, sizeof(_local_timestamp) - 1);
	}

	_local_timestamp[sizeof(_local_timestamp) - 1] = 0;  /* ensure termination */

	Alarm_timestamp_high_res = high_res;

	if (high_res != 0) {
		/* check to see if format string ends in %s or %S; if it doesn't then append " %s" to it */

		len = strlen(_local_timestamp);

		if (len < 2 || (strncmp(_local_timestamp + len - 2, "%s", 3) != 0 && strncmp(_local_timestamp + len - 2, "%S", 3) != 0)) {

			if (sizeof(_local_timestamp) - (len + 1) >= 3) {
				strncpy(_local_timestamp + len, " %s", 4);
				len += 3;

			} else {
				Alarm_timestamp_high_res = 0;  /* append wouldn't fit */
			}
		}
	}

        Alarm_timestamp_format = _local_timestamp;
}  

void Alarm_enable_timestamp(const char *format)
{
	Alarm_enable_timestamp_intrnl(format, 0);
}

void Alarm_enable_timestamp_high_res(const char *format)
{
	Alarm_enable_timestamp_intrnl(format, 1);
}

void Alarm_disable_timestamp(void)
{
        Alarm_timestamp_format   = NULL;
	Alarm_timestamp_high_res = 0;
}

void Alarm_set_types(int32 mask)
{
        Alarm_type_mask = Alarm_type_mask | mask;
}

void Alarm_clear_types(int32 mask)
{
        Alarm_type_mask = Alarm_type_mask & ~mask;
}

int32 Alarm_get_types(void)
{
        return(Alarm_type_mask);
}

void Alarm_set_priority(int16 priority)
{
        Alarm_cur_priority = ( priority & SPLOG_PRIORITY_FIELDS );
}

int16 Alarm_get_priority(void)
{
        return(Alarm_cur_priority);
}

void Alarm_set_realtime_print_handler( alarm_realtime_handler *output_message_function )
{
        Alarm_realtime_print_handler = output_message_function;
}

#ifdef USE_THREADED_ALARM

/************************************************************************************************
 * A conditionally compiled threaded implemenation of Alarm that
 * writes to an in-memory buffer rather than a FILE.  That output is
 * eventually written by a dedicated thread to stdout.
 ***********************************************************************************************/

#include <stdarg.h>
#include <assert.h>

#include <pthread.h>

/************************************************************************************************
 ***********************************************************************************************/

#define BUFFER_VALID() (Buffer != NULL && \
                        Buffer_Lock_Offset >= 0 && Buffer_Lock_Size >= 0 && \
                        Buffer_Written_Offset >= 0 && Buffer_Written_Size >= 0 && Buffer_Wrapped_Size >= 0 && \
                        Buffer_Lock_Offset + Buffer_Lock_Size <= Buffer_Alloced && \
                        Buffer_Written_Offset == Buffer_Lock_Offset + Buffer_Lock_Size && \
                        Buffer_Written_Offset + Buffer_Written_Size <= Buffer_Alloced && \
                        Buffer_Wrapped_Size <= Buffer_Written_Offset)    

/************************************************************************************************
 ***********************************************************************************************/

static pthread_mutex_t Output_Mutex          = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  Output_Cond           = PTHREAD_COND_INITIALIZER;
static int             Output_Thread_Running = 0;         /* is Threaded_Alarm_Thread running yet? */

static long            Buffer_Alloced        = 1048576L;  /* size of circular Buffer to allocate */
static char           *Buffer                = NULL;      /* circular Buffer for spooling Alarm output */

static long            Buffer_Lock_Offset    = 0;         /* offset from which thread is writing */
static long            Buffer_Lock_Size      = 0;         /* size of thread's current write */

static long            Buffer_Written_Offset = 0;         /* end of current Lock block */
static long            Buffer_Written_Size   = 0;         /* amount written past current Lock block */
static long            Buffer_Wrapped_Size   = 0;         /* amount written that has wrapped around */

/************************************************************************************************
 * Threaded_Alarm_Thread: Entry fcn for single, global thread to
 * handle pushing Alarm output to a file.
 ***********************************************************************************************/

static void *Threaded_Alarm_Thread(void *dmy)
{
  long   lock_offset;
  size_t lock_size;
  size_t tmp_size;
  int    tmp;

  while (1) {

    if ((tmp = pthread_mutex_lock(&Output_Mutex)) != 0) {
      assert(0);
    }
    {
      /* free up Lock block we just wrote for more writing */

      assert(BUFFER_VALID());

      Buffer_Lock_Offset += Buffer_Lock_Size;
      Buffer_Lock_Size    = 0;

      assert(BUFFER_VALID());

      /* wait for input */

      while (Buffer_Written_Size == 0 && Buffer_Wrapped_Size == 0) {

        if ((tmp = pthread_cond_wait(&Output_Cond, &Output_Mutex)) != 0) {
          assert(0);
        }

        assert(BUFFER_VALID());
      }

      /* lock portion of Buffer to write + update variables */

      if (Buffer_Written_Size != 0) {  /* output waiting to be written at end of Buffer (and possibly at beginning too but ignored for now) */

        Buffer_Lock_Offset     = Buffer_Written_Offset;
        Buffer_Lock_Size       = Buffer_Written_Size;

        Buffer_Written_Offset += Buffer_Written_Size;
        Buffer_Written_Size    = 0;

        assert(BUFFER_VALID());

      } else {                         /* output waiting to be written only at beginning of Buffer */

        Buffer_Lock_Offset     = 0;
        Buffer_Lock_Size       = Buffer_Wrapped_Size;

        Buffer_Written_Offset  = Buffer_Wrapped_Size;
        Buffer_Written_Size    = 0;
        Buffer_Wrapped_Size    = 0;

        assert(BUFFER_VALID());
      }

      lock_offset = Buffer_Lock_Offset;
      lock_size   = (size_t) Buffer_Lock_Size;  /* NOTE: we use local vars in case we ever use race detectors */

    }
    if ((tmp = pthread_mutex_unlock(&Output_Mutex)) != 0) {
      assert(0);
    }

    if ((tmp_size = fwrite(Buffer + lock_offset, sizeof(char), lock_size, stdout)) < lock_size) {
      /*assert(0);*/
    }
  }

  assert(0);

  return NULL;
}

/************************************************************************************************
 * Threaded_Alarm_Append: Core fcn for trying to output an Alarm.
 * Writes to an in-memory buffer that will later be pushed to disk by
 * Alarm_Thread.
 ***********************************************************************************************/

static void Threaded_Alarm_Append(const char *str, size_t str_len)
{
  int       try_wrap   = 0;
  int       fail_print = 0;
  char     *write_here;
  long      space_left;
  pthread_t thr;
  int       tmp;

  if ((tmp = pthread_mutex_lock(&Output_Mutex)) != 0) {
    assert(0);
  }
  {
    /* ensure that Alarm_Thread is running + memory is alloc'ed */

    if (!Output_Thread_Running) {

      assert(Buffer_Alloced > 0);

      if ((Buffer = (char*) malloc(Buffer_Alloced)) == NULL) {
        assert(0);
      }

      if ((tmp = pthread_create(&thr, NULL, Threaded_Alarm_Thread, NULL)) != 0) {
        assert(0);
      }

      if ((tmp = pthread_detach(thr)) != 0) {
        assert(0);
      }

      Output_Thread_Running = 1;
    }

    /* try to append to end of Buffer */

    assert(BUFFER_VALID());

    if (Buffer_Wrapped_Size == 0) {  /* output waiting to be written hasn't yet run off end of Buffer and wrapped around to beginning */

      write_here = Buffer + Buffer_Written_Offset + Buffer_Written_Size;
      space_left = Buffer_Alloced - (Buffer_Written_Offset + Buffer_Written_Size);

      assert(space_left >= 0 && write_here >= Buffer && write_here + space_left <= Buffer + Buffer_Alloced);

      if (str_len <= space_left) {
        memcpy(write_here, str, str_len);
        Buffer_Written_Size += (long) str_len;
        assert(BUFFER_VALID());

      } else {
        try_wrap = 1;
      }

    } else {
      try_wrap = 1;
    }

    /* try writing to wrap area if we didn't successfully append above */

    if (try_wrap) {

      write_here = Buffer + Buffer_Wrapped_Size;
      space_left = Buffer_Lock_Offset - Buffer_Wrapped_Size;
      
      assert(space_left >= 0 && write_here >= Buffer && write_here + space_left <= Buffer + Buffer_Lock_Offset);

      if (str_len <= space_left) {
        memcpy(write_here, str, str_len);
        Buffer_Wrapped_Size += (long) str_len;
        assert(BUFFER_VALID());

      } else {
        fail_print = 1;
      }
    }

    /* signal Alarm_Thread of output to be written */

    if ((tmp = pthread_cond_signal(&Output_Cond)) != 0) {
      assert(0);
    }
  }
  if ((tmp = pthread_mutex_unlock(&Output_Mutex)) != 0) {
    assert(0);
  }

  if (fail_print) {
    fprintf(stdout, "*** WARNING ***: Threaded_Alarm_Append: Not enough room in in-memory buffer to append Alarm msg!  Writing directly to stdout!\n%s\n", str);
  }
}

/************************************************************************************************
 * Threaded_Alarm_Exit: Pushes out any buffered output before exiting.
 ***********************************************************************************************/

static void Threaded_Alarm_Exit(void)
{
  size_t tmp_size;
  int    tmp;

  if ((tmp = pthread_mutex_lock(&Output_Mutex)) != 0) {
    assert(0);
  }
  {
    /* flush buffer out */

    assert(BUFFER_VALID());

    if (Buffer_Lock_Size != 0) {  /* NOTE: might be duplicated by Threaded_Alarm_Thread -- better to duplicate than to miss though */

      if ((tmp_size = fwrite(Buffer + Buffer_Lock_Offset, sizeof(char), Buffer_Lock_Size, stdout)) != Buffer_Lock_Size) {
        assert(0);
      }
    }

    if (Buffer_Written_Size != 0) {

      if ((tmp_size = fwrite(Buffer + Buffer_Written_Offset, sizeof(char), Buffer_Written_Size, stdout)) != Buffer_Written_Size) {
        assert(0);
      }
    }

    if (Buffer_Wrapped_Size != 0) {

      if ((tmp_size = fwrite(Buffer, sizeof(char), Buffer_Wrapped_Size, stdout)) != Buffer_Wrapped_Size) {
        /*assert(0);*/
      }
    }

    fprintf(stdout, "Exit caused by Alarm(EXIT)\n");
#ifndef ARCH_PC_WIN95
    abort();
#endif
    exit(0);
  }
}

#endif  /* #ifdef USE_THREADED_ALARM */
