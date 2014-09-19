/************************************************************************/
/*	Header file for qlib time structures.				*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: timedef.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__timedef_h
#define	__timedef_h

#include <time.h>
#include <sys/time.h>

/*	Time structures.					*/

typedef struct _ext_time {
    int		year;		/* Year.			*/
    int		doy;		/* Day of year (1-366)		*/
    int		month;		/* Month (1-12)			*/
    int		day;		/* Day of month (1-31)		*/
    int		hour;		/* Hour (0-23)			*/
    int		minute;		/* Minute (0-59)		*/
    int		second;		/* Second (0-60 (leap))		*/
    int		usec;		/* Microseconds (0-999999)	*/
} EXT_TIME;

typedef struct	_int_time {
    int		year;		/* Year.			*/
    int		second;		/* Seconds in year (0-...)	*/
    int		usec;		/* Microseconds (0-999999)	*/
} INT_TIME;

#endif

