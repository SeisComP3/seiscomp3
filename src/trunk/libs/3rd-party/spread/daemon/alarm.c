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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* undef redefined variables under windows */
#ifdef ARCH_PC_WIN95
#undef EINTR
#undef EAGAIN
#undef EWOULDBLOCK
#undef EINPROGRESS
#endif
#include <errno.h>

#ifdef HAVE_GOOD_VARGS
#include <stdarg.h>
#endif

#include "alarm.h"

static int32	Alarm_type_mask = PRINT | EXIT  ;
static  int16   Alarm_cur_priority = SPLOG_DEBUG ;

static char     *Alarm_timestamp_format = NULL;

static const char *DEFAULT_TIMESTAMP_FORMAT="[%a %d %b %Y %H:%M:%S]";

static int      AlarmInteractiveProgram = FALSE;

#ifdef HAVE_GOOD_VARGS

/* Probably should work on all platforms, but just in case, I leave it to the
   developers...
*/

void Alarmp( int16 priority, int32 mask, char *message, ...)
{
    /* log event if in mask and of higher priority, or if FATAL event, always log */
    if ( (( Alarm_type_mask & mask ) && (Alarm_cur_priority <=  priority)) 
         || (priority == SPLOG_FATAL) )
        {
	    va_list ap;

	    if (  Alarm_timestamp_format && (priority != SPLOG_PRINT_NODATE) )
            {
	        char timestamp[42];
		struct tm *tm_now;
		time_t time_now;
		size_t length;
		
		time_now = time(NULL);
		tm_now = localtime(&time_now);
		length = strftime(timestamp, 40,
				  Alarm_timestamp_format, tm_now);
		timestamp[length] = ' ';
		fwrite(timestamp, length+1, sizeof(char), stdout);
            }

	    va_start(ap,message);
	    vprintf(message, ap);
	    va_end(ap);
        }

        if ( ( EXIT & mask ) || (priority == SPLOG_FATAL) )
	{
	    printf("Exit caused by Alarm(EXIT)\n");
            exit( 0 );
	}
}

/* For backwards compatibility while moving all Alarm calls over, this provides
 * the old interface and logs them as WARNING events.
 */
void Alarm( int32 mask, char *message, ...)
{
        if ( ( Alarm_type_mask & mask ) && (Alarm_cur_priority <=  SPLOG_WARNING) )
        {
	    va_list ap;

	    if ( Alarm_timestamp_format )
            {
	        char timestamp[42];
		struct tm *tm_now;
		time_t time_now;
		size_t length;
		
		time_now = time(NULL);
		tm_now = localtime(&time_now);
		length = strftime(timestamp, 40,
				  Alarm_timestamp_format, tm_now);
		timestamp[length] = ' ';
		fwrite(timestamp, length+1, sizeof(char), stdout);
            }

	    va_start(ap,message);
	    vprintf(message, ap);
	    va_end(ap);
        }

	if ( EXIT & mask )
	{
	    printf("Exit caused by Alarm(EXIT)\n");
            exit( 0 );
	}
}
#else

void Alarm( int16 priority, int32 mask, char *message, 
                        void *ptr1, void *ptr2, void *ptr3, void *ptr4, 
                        void *ptr5, void *ptr6, void *ptr7, void *ptr8,
                        void *ptr9, void *ptr10, void*ptr11, void *ptr12,
                        void *ptr13, void *ptr14, void *ptr15, void *ptr16,
                        void *ptr17, void *ptr18, void *ptr19, void *ptr20)
{
        if ( ( Alarm_type_mask & mask ) && (Alarm_cur_priority <  priority) )
        {
            if ( Alarm_timestamp_format )
            {
		char timestamp[42];
		struct tm *tm_now;
		time_t time_now;
		size_t length;
		
		time_now = time(NULL);
		tm_now = localtime(&time_now);
		length = strftime(timestamp, 40,
				  Alarm_timestamp_format, tm_now);
		timestamp[length] = ' ';
		fwrite(timestamp, length+1, sizeof(char), stdout);
            }
	    printf(message, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7, ptr8, ptr9, ptr10, ptr11, ptr12, ptr13, ptr14, ptr15, ptr16, ptr17, ptr18, ptr19, ptr20 );

        }
	if ( EXIT & mask )
	{
	    printf("Exit caused by Alarm(EXIT)\n");
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

void Alarm_set_output(char *filename) {
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

void Alarm_enable_timestamp(char *format)
{
        static char _local_timestamp[40];
	if(format)
	  strncpy(_local_timestamp, format, 40);
	else
	  strncpy(_local_timestamp, DEFAULT_TIMESTAMP_FORMAT, 40);
        Alarm_timestamp_format = _local_timestamp;
}

void Alarm_disable_timestamp(void)
{
        Alarm_timestamp_format = NULL;
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
        Alarm_cur_priority = priority;
}

int16 Alarm_get_priority(void)
{
        return(Alarm_cur_priority);
}
