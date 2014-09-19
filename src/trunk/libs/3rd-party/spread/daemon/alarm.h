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


#ifndef INC_ALARM
#define INC_ALARM

#include <stdio.h>
#include "arch.h"

#define		DEBUG		0x00000001
#define 	EXIT  		0x00000002
#define		PRINT		0x00000004
/* new type to replace general prints */
#define         SYSTEM          0x00000004

#define		DATA_LINK	0x00000010
#define		NETWORK		0x00000020
#define		PROTOCOL	0x00000040
#define		SESSION		0x00000080
#define		CONF		0x00000100
#define		MEMB		0x00000200
#define		FLOW_CONTROL	0x00000400
#define		STATUS		0x00000800
#define		EVENTS		0x00001000
#define		GROUPS		0x00002000

#define         HOP             0x00004000
#define         OBJ_HANDLER     0x00008000
#define         MEMORY          0x00010000
#define         ROUTE           0x00020000
#define         QOS             0x00040000
#define         RING            0x00080000
#define         TCP_HOP         0x00100000

#define         SKIPLIST        0x00200000
#define         ACM             0x00400000

#define         SECURITY        0x00800000

#define		ALL		0xffffffff
#define		NONE		0x00000000

/* Priority levels */   
#define         SPLOG_DEBUG     1       /* Program information that is only useful for debugging. 
                                           Will normally be turned off in operation. */
#define         SPLOG_INFO      2       /* Program reports information that may be useful for 
                                           performance tuning, analysis, or operational checks. */
#define         SPLOG_WARNING   3       /* Program encountered a situation that is not erroneous, 
                                           but is uncommon and may indicate an error. */
#define         SPLOG_ERROR     4       /* Program encountered an error that can be recovered from. */
#define         SPLOG_CRITICAL  5       /* Program will not exit, but has only temporarily recovered 
                                           and without help may soon fail. */
#define         SPLOG_FATAL     6       /* Program will exit() or abort(). */

#define         SPLOG_PRINT     7       /* Program should always print this information */
#define         SPLOG_PRINT_NODATE     8       /* Program should always print this information, but the datestamp should be omitted. */

#ifdef  HAVE_GOOD_VARGS
void Alarmp( int16 priority, int32 type, char *message, ...);
void Alarm( int32 type, char *message, ...);

#else
void Alarm();
#endif

void Alarm_set_output(char *filename);

void Alarm_enable_timestamp(char *format);
void Alarm_disable_timestamp(void);

void Alarm_set_types(int32 mask);
void Alarm_clear_types(int32 mask);
int32 Alarm_get_types(void);

void Alarm_set_priority(int16 priority);
int16 Alarm_get_priority(void);

void Alarm_set_interactive(void);
int  Alarm_get_interactive(void);

#define IP1( address )  ( ( 0xFF000000 & (address) ) >> 24 )
#define IP2( address )  ( ( 0x00FF0000 & (address) ) >> 16 )
#define IP3( address )  ( ( 0x0000FF00 & (address) ) >> 8 )
#define IP4( address )  ( ( 0x000000FF & (address) ) )

#endif	/* INC_ALARM */
