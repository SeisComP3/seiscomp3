/************************************************************************/
/*  Global definitions for qlib.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2002 The Regents of the University of California.
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

/*	$Id: qdefines.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__qdefines_h
#define	__qdefines_h

#include <stdio.h>
#include <sys/param.h>

extern char *qlib2_version;

#define	QUOTE(x)	#x
#define	STRING(x)	QUOTE(x)

#define	IS_LEAP(yr)	( yr%400==0 || (yr%4==0 && yr%100!=0) )
#define	SEC_PER_MINUTE	60
#define	SEC_PER_HOUR	3600
#define	SEC_PER_DAY	86400
#define SEC_PER_YEAR(yr) sec_per_year(yr)
#define	TICKS_PER_SEC	10000
#define	TICKS_PER_MSEC	(TICKS_PER_SEC/1000)
#define USECS_PER_SEC	1000000
#define	USECS_PER_MSEC	(USECS_PER_SEC/1000)
#define	USECS_PER_TICK	(USECS_PER_SEC/TICKS_PER_SEC)

#define	    DAYS_PER_YEAR(yr)	    \
			(365 + ((yr%4==0)?1:0) + \
			 ((yr%100==0)?-1:0) + \
			 ((yr%400==0)?1:0))
#define	BIT(a,i)	((a >> i) & 1)
#define	IHUGE		(65536*32767)
#define	DIHUGE		(140737488355328.)

#ifndef	MAX
#define MAX(a,b)	((a >= b) ? a : b)
#endif
#ifndef	MIN
#define MIN(a,b)	((a <= b) ? a : b)
#endif

#define	UNKNOWN_STREAM	"UNK"
#define	UNKNOWN_COMP	"U"

#define	DATA_HDR_IND_D	'D'
#define	DATA_HDR_IND_R	'R'
#define	DATA_HDR_IND_Q	'Q'
#define	VOL_HDR_IND	'V'
#define	DATA_HDR_IND	DATA_HDR_IND_D

#define	UNK_HDR_TYPE	0
#define	QDA_HDR_TYPE	1
#define	SDR_HDR_TYPE	2
#define SDR_VOL_HDR_TYPE 3
#define	DRM_HDR_TYPE	4

#define	JULIAN_FMT	0
#define	JULIAN_FMT_1	1
#define	MONTH_FMT	2
#define	MONTH_FMT_1	3
#define	JULIANC_FMT	4
#define	JULIANC_FMT_1	5
#define	MONTHS_FMT	6
#define	MONTHS_FMT_1	7

#define	TRUE		1
#define	FALSE		0

/* Library definitions that are not always in math.h	*/
#ifndef M_LN2
#define M_LN2	    0.69314718055994530942
#endif

#define	log2(x)	    ((double)(log(x)/M_LN2))
#define	exp2(x)	    pow(2.,x)

#define	QLIB2_CLASSIC	(qlib2_op_mode == 0)
#define	QLIB2_NOEXIT	(qlib2_op_mode == 1)

#define	MS_ERROR		-2
#define	QLIB2_MALLOC_ERROR	-3
#define	QLIB2_TIME_ERROR	-4

#endif

