/************************************************************************/
/*  Time routines for Quanterra data processing.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismographic Station						*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996 The Regents of the University of California.
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

/*	@(#)qtime.h	1.3 08/02/99 18:37:08	*/

#ifndef	__qtime_h
#define	__qtime_h

#include "timedef.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern void dy_to_mdy 
   (int		doy,		/* day of year (input).			*/
    int		year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*mday);		/* day of month (returned).		*/

extern int mdy_to_doy
   (int		month,		/* month of year (input).		*/
    int		day,		/* day of month (input).		*/
    int		year);		/* year (input).			*/

extern EXT_TIME normalize_ext 
   (EXT_TIME	et);		/* EXT_TIME to normalize.		*/

extern INT_TIME normalize_time
   (INT_TIME	it);		/* INT_TIME to normalize.   		*/

extern EXT_TIME int_to_ext
   (INT_TIME	it);		/* INT_TIME to convert to EXT_TIME.	*/

extern INT_TIME ext_to_int
   (EXT_TIME	et);		/* EXT_TIME to convert to INT_TIME.	*/

double int_to_tepoch
   (INT_TIME	it);		/* INT_TIME to convert to True epoch.	*/

INT_TIME tepoch_to_int
   (double	tepoch);	/* True epoch to convert to INT_TIME.	*/

extern int sec_per_year
   (int		year);		/* year (input).			*/

extern int missing_time
   (INT_TIME	time);		/* INT_TIME structure.			*/

extern INT_TIME add_time
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    int		seconds,	/* number of seconds to add.		*/
    int		usecs);		/* number of usecs to add.		*/

extern INT_TIME add_dtime
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    double	dusec);		/* number of usecs to add.		*/

extern double sps_rate 
   (int rate,			/* sample rate in qlib convention.	*/
    int rate_mult);		/* sampe_rate_mult in qlib convention.	*/

extern void time_interval2
   (int		n,		/* number of samples.			*/
    int		rate,		/* input rate in qlib convention.	*/
    int		rate_mult,	/* sample rate_mult in qlib convention.	*/
    int		*seconds,	/* number of seconds in time interval.	*/
    int		*usecs);	/* number of usecs in time interval.	*/

extern double dsamples_in_time2
   (int		rate,		/* sample rate in qlib convention.	*/
    int		rate_mult,	/* sample rate_mult in qlib convention.	*/
    double	dusecs);	/* number of usecs in time interval.	*/

extern double tdiff
   (INT_TIME	it1,		/* INT_TIME t1.				*/
    INT_TIME	it2);		/* INT_TIME t2.				*/

extern char *time_to_str 
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern char *utime_to_str 
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern char *interval_to_str
   (EXT_TIME	et,		/* Interval to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern INT_TIME *parse_date
   (char	*str);		/* string containing date to parse.	*/

extern EXT_TIME *parse_interval
   (char	*str);		/* string containing interval to parse.	*/

extern int valid_span
   (char	*span);		/* string containing timespan.		*/

extern INT_TIME end_of_span 
   (INT_TIME	it,		/* INT_TIME with initial time.		*/
    char    	*span);		/* string containing timespan.		*/

extern INT_TIME add_interval
   (INT_TIME	it,		/* INT_TIME containing initial time.	*/
    EXT_TIME	interval);	/* EXT_TIME containing time interval.	*/

extern INT_TIME int_time_from_time_tm
   (struct tm	*tm);		/* ptr to time to convert.		*/

extern time_t unix_time_from_ext_time 
   (EXT_TIME	et);		/* EXT_TIME to convert to unix timestamp*/

extern time_t unix_time_from_int_time
   (INT_TIME	it);		/* INT_TIME to convert to Unix timestamp*/

extern EXT_TIME det_time_to_int_time
   (long	evtsec,		/* quanterra seconds for detection time.*/
    int		msec);		/* quanterra msecs for detection time.	*/

extern INT_TIME int_time_from_timeval
   (struct timeval *tv);	/* ptr to struct timeval with input time*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to qtime routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern void f_add_time_ 
#else
extern void f_add_time
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    int		*seconds,	/* Number of seconds to add.		*/
    int		*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot);		/* Resultant time.			*/

#ifdef	fortran_suffix
void f_add_dtime_ 
#else
void f_add_dtime
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    double	*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot);		/* Resultant time.			*/

#ifdef	fortran_suffix
extern void f_time_interval2_ 
#else
extern void f_time_interval2
#endif
   (int		*n,		/* number of samples.			*/
    int		*rate,		/* sample rate.				*/
    int		*rate_mult,	/* sample rate_mult in qlib convention.	*/
    int		*seconds,	/* result interval for n samples (sec)	*/
    int		*usecs);	/* result interval for n samples (usec)	*/

#ifdef	fortran_suffix
extern double f_dsamples_in_time_ 
#else
extern double f_dsamples_in_time
#endif
   (int		*rate,		/* sample rate.				*/
    double	*dusecs);	/* number of usecs.			*/

#ifdef	fortran_suffix
extern double f_tdiff_ 
#else
extern double f_tdiff
#endif
   (INT_TIME	*it1,		/* time1.				*/
    INT_TIME	*it2);		/* time2.  Return (time1-time2)		*/

#ifdef	fortran_suffix
void f_dy_to_mdy_
#else
void f_dy_to_mdy
#endif
   (int		*doy,		/* day of year (input).			*/
    int		*year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*mday);		/* day of month (returned).		*/

#ifdef	fortran_suffix
int f_mdy_to_doy_
#else
int f_mdy_to_doy
#endif
   (int		*month,		/* month of year (input).		*/
    int		*day,		/* day of month (input).		*/
    int		*year);		/* year (input).			*/

#ifdef	fortran_suffix
extern void f_int_to_ext_ 
#else
extern void f_int_to_ext
#endif
   (INT_TIME	*it,		/* input INT_TIME to be convert.	*/
    EXT_TIME	*et);		/* returned equivalent EXT_TIME.	*/

#ifdef	fortran_suffix
extern void f_ext_to_int_ 
#else
extern void f_ext_to_int
#endif
   (EXT_TIME	*et,		/* input EXT_TIME to be converted.	*/
    INT_TIME	*it);		/* returned equivalent INT_TIME.	*/

#ifdef	fortran_suffix
extern void f_time_to_str_ 
#else
extern void f_time_to_str
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    int		*fmt,		/* format number for string.		*/
    char	*str,		/* output characters string.		*/
    int		slen);		/* (fortran supplied) length of string.	*/

#ifdef	fortran_suffix
extern int f_parse_date_ 
#else
extern int f_parse_date
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    char	*str,		/* output characters string.		*/
    int		slen);		/* (fortran supplied) length of string.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

