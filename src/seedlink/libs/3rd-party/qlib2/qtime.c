/************************************************************************/
/*  Time routines for Quanterra data processing.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2004 The Regents of the University of California.
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

#ifndef lint
static char sccsid[] = "$Id: qtime.c 1810 2009-05-14 12:45:21Z andres $ ";
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>
#ifdef	sun
#include <tzfile.h>
#endif
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>

#include "qdefines.h"
#include "qutils.h"
#include "timedef.h"
#include "data_hdr.h"
#include "qtime.h"

/*  Leapsecond file definition.  May be overridden at runtime by the	*/
/*  LEAPSECONDS environment variable.					*/
/*  Use this definition if you supply your own leapsecond file.		*/
#ifndef	    LEAPSECONDS
#define	    LEAPSECONDS	    "/usr/local/lib/leapseconds"
#endif

/*  Use this definition if your system comes with a leapsecond file.	*/
#ifndef	    LEAPSECONDS
#define	    LEAPSECONDS	    "/usr/share/lib/zoneinfo/leapseconds"
#endif

/*  Define max number of leaps we can handle if system does not define.	*/
#ifndef	    TZ_MAX_LEAPS
#define	    TZ_MAX_LEAPS    100
#endif

#define	LDOY(y,m)	(DOY[m] + (IS_LEAP(y) && m >= 2))
#define	SYNTAX_ERROR	{ ++error; break; }
#define LEAPLINELEN	255


/************************************************************************/
/*  True epoch time is defined as the number of true seconds since	*/
/*  January 1, 1970.							*/
/*  True epoch time includes leap seconds.				*/
/************************************************************************/

/************************************************************************/
/*	Information on dates (for non-leap years)			*/
/*		Names of months, days per months, and day of year.	*/

char	    *MON[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
/*		         Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec */
int	    DPM[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
int	    DOY [] = { 0, 31, 59, 90,120,151,181,212,243,273,304,334,365 };

/************************************************************************/
/*  Leapseconds.							*/
/*	In order to handle leapseconds, we have to keep a table of when	*/
/*	they ocurr.  This table needs to be indexed by external time	*/
/*	(eg year, day, hour, minute, second) as well as true second	*/
/*  	offset within the year.  When converting between external time	*/
/*	and internal time, use this table to determine if we need to	*/
/*	add &/or remove leap seconds.					*/
/************************************************************************/

/************************************************************************/
/*	Leap second time structures.					*/
/************************************************************************/
typedef struct lsinfo {
    EXT_TIME	exttime;	/*  External def. of leap time.		*/
    INT_TIME	inttime;	/*  Internal def. of leap time,		*/
				/*  incl. prior leaps this year.	*/
    int		leap_value;	/*  Leap increment in seconds.		*/
} LSINFO;

struct lstable {
    int		initialized;	/*  Leap second table inited?		*/
    int		nleapseconds;	/*  Total leap second entries.		*/
    LSINFO	lsinfo[TZ_MAX_LEAPS];
				/*  Info for each leapsecond.		*/
} lstable;

/************************************************************************/
/*  init_leap_second_table:						*/
/*	Initialize leap second table from external file.		*/
/*  Return:
/************************************************************************/
int init_leap_second_table ()
{
    FILE    *lf;
    char    line[LEAPLINELEN+1], keywd[10], s_month[10], corr[10], type[10];
    int	    i, l, ls;
    int	    lnum = 0;
    char    leap_file[MAXPATHLEN];
    char    *ep;
    LSINFO  *p;
    EXT_TIME et;
    static int status = 0;

    if (lstable.initialized) return (status);
    lstable.initialized = 1;

#ifdef	NO_LEAPSECONDS
    /* If people want to use qlib2 without leapsecond inclusion,	*/
    /* they can define NO_LEAPSECONDS in the Makefile.			*/
    if (1) return (status);
#endif

    /*	If the environment variable LEAPSECONDS exists, it should	*/
    /*	override the default LEAPSECONDS file.				*/
    if ((ep=getenv("LEAPSECONDS"))!=NULL)
	strcpy(leap_file,ep);
    else strcpy(leap_file, LEAPSECONDS);
    if ((lf=fopen(leap_file, "r"))==NULL) {
	fprintf (stderr, "Warning: unable to open leap second file: %s\n",leap_file);
	return (status);
    }

    while (fgets(line,LEAPLINELEN,lf)!=NULL) {
	++lnum;
	line[LEAPLINELEN]='\0';
	trim(line);
	l = strlen(line);
	if (l>0 && line[l-1]=='\n') line[--l] = '\0';
	if (l<=0 || line[0]=='#') continue;
	if ((ls=lstable.nleapseconds) >= TZ_MAX_LEAPS) {
	    fprintf (stderr, "Error: too many leapsecond entries - line %d\n", lnum);
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    status = QLIB2_TIME_ERROR;
	    continue;
	}
 	p = &lstable.lsinfo[ls];
	if ((l=sscanf(line,"%s %d %s %d %d:%d:%d %s %s", keywd, &p->exttime.year,
		      s_month, &p->exttime.day, &p->exttime.hour, &p->exttime.minute,
		      &p->exttime.second, corr, type))!= 9) {
	    fprintf (stderr, "Error: invalid leapsecond line - line %d\n", lnum);
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    status = QLIB2_TIME_ERROR;
	    continue;
	}
	i = 0;
	while (MON[i] != 0) {
	    if (strcasecmp(MON[i],s_month)==0) {
		p->exttime.month = i+1;
		p->exttime.doy = DOY[i] + p->exttime.day +
		    ( IS_LEAP(p->exttime.year) && (p->exttime.month > 2) );
		break;
	    }
	    ++i;
	}
	p->exttime.usec = 0;
	switch (corr[0]) {
	    case '+':	p->leap_value = 1; break;
	    case '-':	p->leap_value = -1; break;
	    default:
		p->leap_value = 0;
		fprintf(stderr, "Error: invalid leapsecond correction - line %d\n", lnum);
		fflush (stderr);
		if (QLIB2_CLASSIC) exit(1);
		status = QLIB2_TIME_ERROR;
		break;
	}
	if (p->leap_value == 0) continue;
	/* Ensure that the INT_TIME field does not wrap into the next	*/
	/* year for leapseconds occurring at the end of the year.	*/
	et = p->exttime;
	et.second -= p->leap_value;
	p->inttime = ext_to_int (et);
	p->inttime.second += p->leap_value;
	++lstable.nleapseconds;
    }
    fclose(lf);
    return (status);
}

/************************************************************************/
/*  is_leap_second:							*/
/*	Return lsinfo structure if this second is in leap second table.	*/
/************************************************************************/
static LSINFO *is_leap_second
   (INT_TIME it)		/* INT_TIME structure			*/
{
    int		i;
    /*	Search leap second table.   */
    if (!lstable.initialized) {
	int status = init_leap_second_table();
    }
    for (i=0; i<lstable.nleapseconds; i++) {
	if (it.year == lstable.lsinfo[i].inttime.year &&
	    it.second == lstable.lsinfo[i].inttime.second)
	    return (&lstable.lsinfo[i]);
    }
    return(NULL);
}

/************************************************************************/
/*  sec_per_min:							*/
/*	Return the number of seconds in this minute.			*/
/************************************************************************/
static int sec_per_min
   (EXT_TIME	et)		/* EXT_TIME structure to convert.	*/
{
    /*	Search leap second table.   */
    int i;
    if (!lstable.initialized) {
	int status = init_leap_second_table();
    }
    for (i=0; i<lstable.nleapseconds; i++) {
	if (et.year < lstable.lsinfo[i].exttime.year) break;
	if (et.year == lstable.lsinfo[i].exttime.year &&
	    et.month == lstable.lsinfo[i].exttime.month &&
	    et.day == lstable.lsinfo[i].exttime.day &&
	    et.hour == lstable.lsinfo[i].exttime.hour &&
	    et.minute == lstable.lsinfo[i].exttime.minute)
	    return (60+lstable.lsinfo[i].leap_value);
    }
    return(60);
}

/************************************************************************/
/*  prior_leaps_in_ext_time:						*/
/*	Return the number of leap seconds that must be added to this	*/
/*	EXT_TIME in order to compute the accurate number of seconds	*/
/*	within the year.						*/
/************************************************************************/
static int prior_leaps_in_ext_time
   (EXT_TIME	et)		/* EXT_TIME time structure.		*/
{
    LSINFO	*p;
    int		i;
    int		result = 0;
    if (!lstable.initialized) {
	int status = init_leap_second_table();
    }
    for (i=0; i<lstable.nleapseconds; i++) {
	p = &lstable.lsinfo[i];
	if (et.year == p->exttime.year &&
	    (et.doy > p->exttime.doy ||
	     (et.doy == p->exttime.doy &&
	      (et.hour > p->exttime.hour ||
	       (et.hour == p->exttime.hour &&
		(et.minute > p->exttime.minute ||
		 (et.minute == p->exttime.minute &&
		  (et.second > p->exttime.second))))))))
	    result += p->leap_value;
    }
    return(result);
}

/************************************************************************/
/*  prior_leaps_in_int_time:						*/
/*	Return the accumulated number of leap seconds in this year	*/
/*	prior to time.							*/
/************************************************************************/
static int prior_leaps_in_int_time
   (INT_TIME	it)		/* INT_TIME structure.			*/
{
    /*	Return the number of leap seconds that occurred prior to this 	*/
    /*	time within this year.						*/
    LSINFO	*p;
    int		i;
    int		result = 0;
    if (!lstable.initialized) {
	int status = init_leap_second_table();
    }
    for (i=0; i<lstable.nleapseconds; i++) {
	p = &lstable.lsinfo[i];
	if (it.year == p->inttime.year &&
	    (it.second > p->inttime.second))
	    result += p->leap_value;
    }
    return(result);
}

/************************************************************************/
/*  dy_to_mdy:								*/
/*	Return month and day from day,year info.  Handle leap years.	*/
/************************************************************************/
void dy_to_mdy
   (int		doy,		/* day of year (input).			*/
    int		year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*mday)		/* day of month (returned).		*/
{
    *month=1;
    *mday = doy;
    while (doy > LDOY(year,*month)) ++*month;
    *mday = doy - LDOY(year,*month-1);
}

/************************************************************************/
/*  mdy_to_doy:								*/
/*	Return day_of_year from month, day, year info.			*/
/*	Don't forget about leap year.					*/
/************************************************************************/
int mdy_to_doy
   (int		month,		/* month of year (input).		*/
    int		day,		/* day of month (input).		*/
    int		year)		/* year (input).			*/
{
    return(LDOY(year,month-1) + day);
}

/************************************************************************/
/*  normalize_ext:							*/
/*	Normalize time in an EXT_TIME structure.			*/
/*  return:								*/
/*	Normalized EXT_TIME structure.					*/
/************************************************************************/
EXT_TIME normalize_ext
   (EXT_TIME	et)		/* EXT_TIME to normalize.		*/
{
    int seconds_per_minute;
    /*  Normalize external time from the minute up.			*/
    while (et.minute >= 60) { et.minute -= 60; ++(et.hour); }
    while (et.minute <   0) { et.minute += 60; --(et.hour); }
    while (et.hour >= 24) { et.hour -= 24; ++(et.doy); }
    while (et.hour <   0) { et.hour += 24; --(et.doy); }
    while (et.doy > DAYS_PER_YEAR(et.year)) {
	et.doy -= DAYS_PER_YEAR(et.year);
	++(et.year);
    }
    while (et.doy <= 0) {
	--(et.year);
	et.doy += DAYS_PER_YEAR(et.year);
    }
    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
    /* Now worry about seconds, which may span a leap day.		*/
    /* For efficiency:							*/
    /* 1.  Convert time (ignoring second and usec) to int_time.		*/
    /* 2. Add second and usec field, the reconvert to ext_time.		*/
    if (et.second || et.usec) {
	EXT_TIME et2;
	INT_TIME it2;
	et2 = et;
	et2.second = et2.usec = 0;
	it2 = ext_to_int(et2);
	it2 = add_time(it2, et.second, et.usec);
	et = int_to_ext(it2);
    }
    return (et);
}

/************************************************************************/
/*  normalize_time:							*/
/*  	Normalize an INT_TIME time structure.				*/
/*  return:								*/
/*	Normalized INT_TIME structure.					*/
/************************************************************************/
INT_TIME normalize_time
   (INT_TIME	it)		/* INT_TIME to normalize.   		*/
{
    int		s_p_y;

    while (it.usec < 0) {
	--(it.second);
	it.usec += USECS_PER_SEC;
    }
    while (it.usec >= USECS_PER_SEC) {
	++(it.second);
	it.usec -= USECS_PER_SEC;
    }
    while (it.second < 0) {
	--(it.year);
	it.second += sec_per_year(it.year);
    }
    while (it.second >= (s_p_y = sec_per_year(it.year))) {
	it.second -= s_p_y;
	++(it.year);
    }
    return(it);
}

/************************************************************************/
/*  int_to_ext:								*/
/*	Convert internal time to external time, accounting for		*/
/*	leap seconds.							*/
/*  return:								*/
/*	EXT_TIME structure converted from input INT_TIME.		*/
/************************************************************************/
EXT_TIME int_to_ext
   (INT_TIME	it)		/* INT_TIME to convert to EXT_TIME.	*/
{
    EXT_TIME et;
    int		leaps;
    LSINFO	*lp;

    /*	Add or remove leap seconds that occur before this time within	*/
    /*	the year so that we can convert it to a string using code	*/
    /*	that is independent of leapseconds.  The only trick is that	*/
    /*	if the time is an exact leapsecond, we have to know it, since	*/
    /*	second 60 would normally be considered second 0 of the next	*/
    /*	minute.								*/
    /*  If the time is a "negative leap second", we just add 1 second	*/
    /*  to accomodate the skip.	 Since the time should be initially	*/
    /*	normalized, we can never represent a negative leapsecond at	*/
    /*	the end of the year, so we don't have to worry about		*/
    /*	re-normalizing and possibly crossing year boundaries.		*/

    et.year = it.year;
    et.second = it.second;
    leaps = prior_leaps_in_int_time (it);

    et.second = et.second - leaps;
    if ((lp = is_leap_second (it)) && (lp->leap_value < 0))
	/*  For a missing second, adjust accordingly.			*/
	et.second = et.second - lp->leap_value;

    if (lp && lp->leap_value > 0) {
	/*  This corresponds to an entry for a positive leap_second.	*/
	/*  If it is an added second, use the info in the returned	*/
	/*  leap_second structure for computing the external date.	*/
	et.doy = lp->exttime.doy;
	et.month = lp->exttime.month;
	et.day = lp->exttime.day;
	et.hour = lp->exttime.hour;
	et.minute = lp->exttime.minute;
	et.second = lp->exttime.second;
    }
    else {
	et.doy = (et.second / SEC_PER_DAY) + 1;
	et.second = et.second % SEC_PER_DAY;
	et.hour = et.second / SEC_PER_HOUR;
	et.second = et.second % SEC_PER_HOUR;
	et.minute = et.second / SEC_PER_MINUTE;
	et.second = et.second % SEC_PER_MINUTE;
    }
    et.usec = it.usec;
    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
    return (et);
}

/************************************************************************/
/*  ext_to_int:								*/
/*	Convert external time to internal time, accounting for		*/
/*	leap seconds.							*/
/*  return:								*/
/*	INT_TIME structure converted from input EXT_TIME.		*/
/************************************************************************/
INT_TIME ext_to_int
   (EXT_TIME	et)		/* EXT_TIME to convert to INT_TIME.	*/
{
    INT_TIME	it;
    int		leaps;
    int i;
    i = 0;
    et = normalize_ext(et);
    leaps = prior_leaps_in_ext_time (et);
    it.year = et.year;
    it.second = (et.doy-1) * (int)SEC_PER_DAY +
		et.hour * (int)SEC_PER_HOUR +
		et.minute * (int)SEC_PER_MINUTE +
		et.second + leaps;
    it.usec = et.usec;
    return(normalize_time(it));
}

/************************************************************************/
/*  int_to_tepoch:							*/
/*	Convert internal time to true epoch time, accounting for	*/
/*	leap seconds.							*/
/*  return:								*/
/*	True epoch time converted from INT_TIME structure.		*/
/************************************************************************/
double int_to_tepoch
   (INT_TIME	it)		/* INT_TIME to convert to True epoch.	*/
{
    double	tepoch = 0.0;
    int		year = 1970;
    while (it.year < year) {
	tepoch -= sec_per_year(--year);
    }
    while (it.year > year) {
	tepoch += sec_per_year(year++);
    }
    tepoch += it.second;
    tepoch += ((double)it.usec / (double)USECS_PER_SEC);
    return(tepoch);
}

/************************************************************************/
/*  tepoch_to_int:							*/
/*	Convert true epoch time to internal time, accounting for	*/
/*	leap seconds.							*/
/*  return:								*/
/*	INT_TIME structure converted from input true epoch time.	*/
/************************************************************************/
INT_TIME tepoch_to_int
   (double	tepoch)		/* True epoch to convert to INT_TIME.	*/
{
    INT_TIME	it;
    int		s_p_y;
    it.year = 1970;
    it.second = it.usec = 0;
    while (tepoch < 0) {
	--(it.year);
	tepoch += sec_per_year(it.year);
    }
    while (tepoch >= (s_p_y = sec_per_year(it.year))) {
	tepoch -= s_p_y;
	++(it.year);
    }
    it.second = (int)tepoch;
    tepoch -= it.second;
    it.usec = roundoff(tepoch*USECS_PER_SEC) % USECS_PER_SEC;
    return(normalize_time(it));
}

/************************************************************************/
/*  int_to_nepoch:							*/
/*	Convert internal time to nominal epoch time, ignoring		*/
/*	leap seconds.							*/
/*  return:								*/
/*	Nominal epoch time converted from INT_TIME structure.		*/
/************************************************************************/
double int_to_nepoch
   (INT_TIME	it)		/* INT_TIME to convert to True epoch.	*/
{
    double	nepoch = 0.0;
    int		year = 1970;
    while (it.year < year) {
	nepoch -= nsec_per_year(--year);
    }
    while (it.year > year) {
	nepoch += nsec_per_year(year++);
    }
    nepoch += it.second;
    nepoch += ((double)it.usec / (double)USECS_PER_SEC);
    return(nepoch);
}

/************************************************************************/
/*  nepoch_to_int:							*/
/*	Convert nominal epoch time to internal time, ignoring		*/
/*	leap seconds.							*/
/*  return:								*/
/*	INT_TIME structure converted from input nominal epoch time.	*/
/************************************************************************/
INT_TIME nepoch_to_int
   (double	nepoch)		/* True epoch to convert to INT_TIME.	*/
{
    INT_TIME	it;
    int		s_p_y;
    it.year = 1970;
    it.second = it.usec = 0;
    while (nepoch < 0) {
	--(it.year);
	nepoch += nsec_per_year(it.year);
    }
    while (nepoch >= (s_p_y = nsec_per_year(it.year))) {
	nepoch -= s_p_y;
	++(it.year);
    }
    it.second = (int)nepoch;
    nepoch -= it.second;
    it.usec = roundoff(nepoch*USECS_PER_SEC) % USECS_PER_SEC;
    return(normalize_time(it));
}

/************************************************************************/
/*  sec_per_year:							*/
/*	Return number of seconds in the year, accounting for 		*/
/*	leap seconds.							*/
/************************************************************************/
int sec_per_year
   (int		year)		/* year (input).			*/
{
    int		i;
    int		result = ( SEC_PER_DAY * (365 + IS_LEAP(year)) );
    /*	Search leap second table.   */
    if (!lstable.initialized) {
	int status = init_leap_second_table();
    }
    for (i=0; i<lstable.nleapseconds; i++) {
	if (year == lstable.lsinfo[i].inttime.year)
	    result += lstable.lsinfo[i].leap_value;
    }
    return(result);
}

/************************************************************************/
/*  nsec_per_year:							*/
/*	Return nominal number of seconds in the year, ignoring		*/
/*	leap seconds.							*/
/************************************************************************/
int nsec_per_year
   (int		year)		/* year (input).			*/
{
    int		i;
    int		result = ( SEC_PER_DAY * (365 + IS_LEAP(year)) );
    return(result);
}

/************************************************************************/
/*  missing_time:							*/
/*  	Check for an empty INT_TIME structure.				*/
/*  return:								*/
/*	1 if INT_TIME is empty (all zeros), 0 otherwise.		*/
/************************************************************************/
int missing_time
   (INT_TIME	time)		/* INT_TIME structure.			*/
{
    return (time.year == 0 && time.second == 0 && time.usec == 0);
}

/************************************************************************/
/*  add_time:								*/
/*	Add an increment to an INT_TIME.  Return INT_TIME structure.	*/
/************************************************************************/
INT_TIME add_time
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    int		seconds,	/* number of seconds to add.		*/
    int		usecs)		/* number of usecs to add.		*/
{
    it.usec += usecs;
    it.second += seconds;
    return(normalize_time(it));
}

/************************************************************************/
/*  add_dtime:								*/
/*	Add an increment to an INT_TIME.  Return INT_TIME structure.	*/
/************************************************************************/
INT_TIME add_dtime
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    double	dusec)		/* number of usecs to add.		*/
{
    double dtmp;
    int high, low;
    int sign;
    int split_val = 100 * USECS_PER_SEC;
				/* value used to split dusec		*/
				/* high and low int without overflow.	*/
    /* Prevent usec field from overflow by breaking the value an	*/
    /* appropriate number of seconds and usecs.				*/

    sign = (dusec >= 0.) ? 1 : -1;
    dtmp = fabs (dusec);
    high = dtmp / split_val;
    low = dtmp - ((double)high * (double)split_val);

    it.usec += sign * low;
    it.second += sign * high * 100;
    return(normalize_time(it));
}

/************************************************************************/
/*  sps_rate:								*/
/*	Compute the samples_per_second given the sample_rate and the	*/
/*	sample_rate_mult.						*/
/************************************************************************/
double sps_rate
   (int rate,			/* sample rate in qlib convention.	*/
    int rate_mult)		/* sampe_rate_mult in qlib convention.	*/
{
    if (rate != 0 && rate_mult == 0) rate_mult = 1; /* backwards compat.*/
    if (rate == 0 || rate_mult == 0) return (0.);
    if (rate > 0 && rate_mult > 0) return ((double)rate * rate_mult);
    if (rate > 0 && rate_mult < 0) return ((double)-1*rate / rate_mult);
    if (rate < 0 && rate_mult > 0) return ((double)-1*rate_mult / rate);
    if (rate < 0 && rate_mult < 0) return ((double)rate_mult / rate);
    return (0);
}

/************************************************************************/
/*  time_interval2:							*/
/*  	Compute the time interval for n points at a given sample rate,	*/
/*	in seconds and usecs.						*/
/************************************************************************/
void time_interval2
   (int		n,		/* number of samples.			*/
    int		rate,		/* input rate in qlib convention.	*/
    int		rate_mult,	/* input rate_mult in qlib convention.	*/
    int		*seconds,	/* number of seconds in time interval.	*/
    int		*usecs)		/* number of usecs in time interval.	*/
{
    double dtime, dseconds, dusecs;
    double spsrate;

    spsrate = sps_rate(rate, rate_mult);
    if (spsrate == 0.) {
	*seconds = 0;
	*usecs = 0;
    }
    else {
	dtime = n/spsrate;
	dusecs = modf (dtime, &dseconds);
	*seconds = (int)dseconds;
	/* Correct for roundoff error.  */
	dusecs *= USECS_PER_SEC;
	*usecs = roundoff(dusecs);
    }
    return;
}

/************************************************************************/
/*  dsamples_in_time2:							*/
/*	Compute the dp number of samples that ocurr within a specified 	*/
/*	time given a sample rate.					*/
/*  return:								*/
/*	number of samples (double precision).				*/
/************************************************************************/
double dsamples_in_time2
   (int		rate,		/* sample rate in qlib convention.	*/
    int		rate_mult,	/* sample rate_mult in qlib convention.	*/
    double	dusecs)		/* number of usecs in time interval.	*/
{
    double dsamples;

    dsamples = (sps_rate(rate,rate_mult)*dusecs/USECS_PER_SEC);
    return (dsamples);
}

/************************************************************************/
/*  tdiff:								*/
/*	Compare 2 times, and return the difference (t1-t2) in usecs.	*/
/*	If overflow would ocurr, return appropriate DIHUGE value.	*/
/*  return:								*/
/*	time difference in usecs (double precision).			*/
/************************************************************************/
double tdiff
   (INT_TIME	it1,		/* INT_TIME t1.				*/
    INT_TIME	it2)		/* INT_TIME t2.				*/
{
    INT_TIME	x1, x2;
    int i;
    int m = 1;
    double seconds, usecs;
    int	d[3];

    /* Ensure x1 >& x2	*/

    d[0] = it1.year - it2.year;
    d[1] = it1.second - it2.second;
    d[2] = it1.usec - it2.usec;

    for (i=0; i<3; i++) {
	if (d[i] > 0) break;
	if (d[i] < 0) {
	    m = -1;
	    break;
	}
    }
    if (m == 1) {
	x1 = it1; x2 = it2;
    }
    else {
	x1 = it2; x2 = it1;
	m = -1;
    }

    /* Check for gross differences that would generate over/underflows.	*/
/*::    if ( (x1.year - x2.year > 8) ) return (DIHUGE * m);*/

    /* Normalize to a common year.   */
    /* Accumulate seconds in double precision to avoid integer overflow.*/
    seconds = x1.second;
    while (x1.year > x2.year) {
	    --x1.year;
	    seconds += sec_per_year(x1.year);
    }

    /* Compute usec difference.	*/
    seconds = (seconds - x2.second);
/*::    if (second > (DIHUGE/USECS_PER_SEC)) return (DIHUGE * m);*/
    usecs = (x1.usec - x2.usec);
    usecs += seconds*USECS_PER_SEC;
    usecs *= m;

    return (usecs);
}

/************************************************************************/
/*  time_to_str:							*/
/*	Convert internal time to printable string.			*/
/************************************************************************/
char *time_to_str
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt)		/* format specifier.			*/
{
    static char str[80];	    /* contains printable time string.	*/
    int delim;
    EXT_TIME	et = int_to_ext (it);

    switch (fmt) {
	case MONTH_FMT:
	case MONTH_FMT_1:
	    delim = (fmt == MONTH_FMT) ? ' ' : ',';
	    sprintf (str, "%04d.%02d.%02d%c%02d:%02d:%02d.%04d",
		     et.year, et.month, et.day, delim, et.hour,
		     et.minute, et.second, et.usec/USECS_PER_TICK);
	    break;
	case MONTHS_FMT:
	case MONTHS_FMT_1:
	    delim = (fmt == MONTHS_FMT) ? ' ' : ',';
	    sprintf (str, "%04d/%02d/%02d%c%02d:%02d:%02d.%04d",
		     et.year, et.month, et.day, delim, et.hour,
		     et.minute, et.second, et.usec/USECS_PER_TICK);
	    break;
	case JULIANC_FMT:
	case JULIANC_FMT_1:
	    delim = (fmt == JULIANC_FMT) ? ' ' : ',';
	    sprintf (str, "%04d,%03d%c%02d:%02d:%02d.%04d",
		     et.year, et.doy, delim, et.hour,
		     et.minute, et.second, et.usec/USECS_PER_TICK);
	    break;
	case JULIAN_FMT:
	case JULIAN_FMT_1:
	default:
	    delim = (fmt == JULIAN_FMT) ? ' ' : ',';
	    sprintf (str, "%04d.%03d%c%02d:%02d:%02d.%04d",
		     et.year, et.doy, delim, et.hour,
		     et.minute, et.second, et.usec/USECS_PER_TICK);
	    break;
    }
    return (str);
}

/************************************************************************/
/*  utime_to_str:							*/
/*	Convert extended internal time to printable string.		*/
/************************************************************************/
char *utime_to_str
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt)		/* format specifier.			*/
{
    static char str[80];	/* contains printable time string.	*/
    int delim;
    EXT_TIME	et = int_to_ext (it);

    switch (fmt) {
	case MONTH_FMT:
	case MONTH_FMT_1:
	    delim = (fmt == MONTH_FMT) ? ' ' : ',';
	    sprintf (str, "%04d.%02d.%02d%c%02d:%02d:%02d.%06d",
		     et.year, et.month, et.day, delim, et.hour,
		     et.minute, et.second, et.usec);
	    break;
	case JULIAN_FMT:
	case JULIAN_FMT_1:
	default:
	    delim = (fmt == JULIAN_FMT) ? ' ' : ',';
	    sprintf (str, "%04d.%03d%c%02d:%02d:%02d.%06d",
		     et.year, et.doy, delim, et.hour,
		     et.minute, et.second, et.usec);
	    break;
    }
    return (str);
}

/************************************************************************/
/*  interval_to_str:							*/
/*	Convert interval store in EXT_TIME format to printable string.	*/
/************************************************************************/
char *interval_to_str
   (EXT_TIME	et,		/* Interval to convert to string.	*/
    int		fmt)		/* format specifier.			*/
{
    static char str[80];	    /* contains printable time string.	*/
    int		delim = ',';
    sprintf (str, "%d.%d%c%02d:%02d:%02d.%04d",
	     et.year,et.doy, delim, et.hour, et.minute, et.second,
	     et.usec/USECS_PER_TICK);
    return (str);
}

#define YMD_FMT		3
#define	YDOY_FMT	2
#define YM_FMT		2
#define	DATE_DELIMS	"/.,"
#define DATETIME_DELIMS	"/., \t"
/************************************************************************/
/*  parse_date:								*/
/*	Parse a date string and return ptr to INT_TIME structure.	*/
/*	Return NULL if error parsing the date string.			*/
/*  return:								*/
/*	Date stored in INT_TIME structure.				*/
/************************************************************************/
INT_TIME *parse_date
   (char	*str)		/* string containing date to parse.	*/
{
    /*
    Permissible input formats:
        yyyy/mm/dd/hh:mm:ss.ffff
        yyyy/mm/dd.hh:mm:ss.ffff
        yyyy/mm/dd,hh:mm:ss.ffff
        yyyy/mm/dd hh:mm:ss.ffff
        yyyy.ddd.hh:mm:ss.ffff
        yyyy.ddd,hh:mm:ss.ffff
        yyyy,ddd,hh:mm:ss.ffff
        yyyy,ddd hh:mm:ss.ffff
        numberT or numbert - true epoch time
	numberN or numbern - nominal epoch time
    where
	yy = year (yyyy)
	mm = month (1-12)
	dd = day (1-31)
	dddd = day-of-year (1-365)
	hh = hour (0-23)
	mm = minute (0-59)
	ss = second (0-59)
	ffff = fractional part of second
    The time is optional.  If not specified, it is 00:00:00.0000
    */

    char	*p, *q, *eos;
    char	*delim;
    EXT_TIME	et;
    int		trip, nd;
    int		error = 0;
    static INT_TIME	it;
    int		format;
    int		ndelim;
    int		l;
    char	*p1, *p2;
    double	epoch;

    et.year = et.doy = et.month = et.day = 0;
    et.hour = et.minute = et.second = et.usec = 0;

    /* First check for true or nominal epoch time.  */
    l = strlen(str);
    p = strpbrk (str, "TtNn");
    epoch = strtod (str, &q);
    if (l > 1 && p != NULL && p-str == l-1 && p == q) {
	/* We have an epoch time specification. */
	if (strpbrk(p, "Tt") != NULL) it = tepoch_to_int(epoch);
	else if (strpbrk(p, "Nn") != NULL) it = nepoch_to_int(epoch);
	else return ((INT_TIME *)NULL);
	return (&it);
    }

    /* Now check for normal date time specification in various format.	*/
    /* Scan for first ":", and then determine the number of		*/
    /* delimiters before to determine year.doy or year.mm.dd format.	*/
    eos = str + strlen(str);
    while (eos > str && *(eos-1) == ' ') --eos;
    q = strchr(str,':');
    if (q == NULL) q = eos;
    ndelim = 0;
    /* Allow blank(s) as delimiter between date and time. */
    /* Skip any leading blanks in the string. */
    for (p=str; p!=q && *p == ' '; p++) ;
    for (p=p; p!=q; p++) {
	if (strchr(DATETIME_DELIMS,*p)) ++ndelim;
	/* Skip consecutive blanks. */
	for (; p+1!=q && *p==' ' && *(p+1)==' '; p++) ;
    }
    if (*q == ':') switch (ndelim) {
	case 2:	format=YDOY_FMT; break;
	case 3: format=YMD_FMT; break;
	default: ++error;
    }
    else switch (ndelim) {
	case 1:	format=YDOY_FMT; break;
	case 2: format=YMD_FMT; break;
	default: ++error;
    }

    if (error) {
	return ((INT_TIME *)NULL);
    }

    p = str;
    for (trip=0; trip<1; trip++) {
	/* Parse date.	*/
	et.year = (int)strtol (p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
/*::	Y2K compliance.	*/
/*::	Do not allow century abbreviation.  */
/*::	if (et.year > 0 && et.year <= 99) et.year += 1900;  */
/*::	Flag 2 digit years as an error.	*/
	if (delim-p < 3) SYNTAX_ERROR;
/*::	End Y2K compliance. */
	if (strchr(DATE_DELIMS, *delim) == NULL) SYNTAX_ERROR
	if (format == YMD_FMT) {
	    /* Syntax should be yy/mm/dd    */
	    p = ++delim;
	    et.month = (int)strtol (p, &delim, 10);
	    if (delim == p) SYNTAX_ERROR
	    if (et.month < 1 || et.month > 12) SYNTAX_ERROR
	    if (strchr(DATE_DELIMS, *delim) == NULL) SYNTAX_ERROR
	    p = ++delim;
	    et.day = strtol (p, &delim, 10);
	    if (delim == p) SYNTAX_ERROR
	    if (et.day < 1 || et.day > DPM[et.month] + (IS_LEAP(et.year) && et.month == 2))
		SYNTAX_ERROR
	    et.doy = DOY[et.month-1] + et.day +
		    ( IS_LEAP(et.year) && (et.month > 2) );
	}
	else {
	    /* Syntax should be yy.ddd	    */
	    p = ++delim;
	    et.doy= strtol (p, &delim, 10);
	    if (delim == p) SYNTAX_ERROR
	    if (et.doy < 1 || et.doy > 365 + IS_LEAP(et.year)) SYNTAX_ERROR
	    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
	    if (*delim == 0 || delim == eos) break;
	}

	/* Parse time.	*/
	if (*delim == 0 || delim == eos) break;
	if (strchr(DATETIME_DELIMS, *delim) == NULL) SYNTAX_ERROR
	/* Skip consecutive blanks. */
	for (; delim+1!=q && *delim==' ' && *(delim+1)==' '; delim++) ;
	p = ++delim;
	et.hour = strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (et.hour < 0 || et.hour >= 24) SYNTAX_ERROR

	if (*delim == 0 || delim == eos) break;
	if (*delim != ':') SYNTAX_ERROR
	p = ++delim;
	et.minute = strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (et.minute < 0 || et.hour >= 60) SYNTAX_ERROR

	if (*delim == 0 || delim == eos) break;
	if (*delim != ':') SYNTAX_ERROR
	p = ++delim;
	et.second = strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	/*  Allow leap second.	*/
	if (et.second < 0 || et.hour > 60) SYNTAX_ERROR

	if (*delim == 0 || delim == eos) break;
	if (*delim != '.') SYNTAX_ERROR
	p = ++delim;
	et.usec = 0;
	et.usec = strtol(p, &delim, 10);
	if (delim == p && p != eos) SYNTAX_ERROR
	if (! (*delim == 0 || delim == eos)) SYNTAX_ERROR
	nd = delim-p;
	if (nd < 0 || nd > 6) SYNTAX_ERROR
	while (nd < 6) {
	    et.usec *= 10;
	    nd++;
	}
    }
    if (error) {
	return ((INT_TIME *)NULL);
    }

/*::
    printf ("year = %d, doy = %d, hour = %d, min = %d, sec = %d, usec = %d\n",
	    et.year, et.doy, et.hour, et.minute, et.second, et.usec);
::*/

    it = ext_to_int (et);
    return (&it);
}

/************************************************************************/
/*  parse_date_month:							*/
/*	Parse year & month string and return ptr to INT_TIME structure.	*/
/*	Return NULL if error parsing the date string.			*/
/*  return:								*/
/*	Date stored in INT_TIME structure.				*/
/************************************************************************/
INT_TIME *parse_date_month
   (char	*str)		/* string containing date to parse.	*/
{
    /*
    Permissible input formats:
        yyyy/mm
        yyyy.mm
        yyyy,mm
    where
	yy = year (yyyy)
	mm = month (1-12)
    */

    char	*p, *q, *eos;
    char	*delim;
    EXT_TIME	et;
    int		trip, nd;
    int		error = 0;
    static INT_TIME	it;
    int		format;
    int		ndelim;

    et.year = et.doy = et.month = et.day = 0;
    et.hour = et.minute = et.second = et.usec = 0;

    ndelim = 0;
    eos = str + strlen(str);
    while (eos > str && *(eos-1) == ' ') --eos;
    q = eos;
    /* Skip any leading blanks in the string. */
    for (p=str; p!=q && *p == ' '; p++) ;
    format=YM_FMT;

    for (trip=0; trip<1; trip++) {
	/* Parse date.	*/
	et.year = (int)strtol (p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
/*::	Y2K compliance.	*/
/*::	Do not allow century abbreviation.  */
/*::	if (et.year > 0 && et.year <= 99) et.year += 1900;  */
/*::	Flag 2 digit years as an error.	*/
	if (delim-p < 3) SYNTAX_ERROR;
/*::	End Y2K compliance. */
	if (strchr(DATE_DELIMS, *delim) == NULL) SYNTAX_ERROR
	p = ++delim;
	et.month= strtol (p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (et.month < 1 || et.month > 12) SYNTAX_ERROR
	et.day = 1;
	et.doy = mdy_to_doy(et.month, et.day, et.year);

	if (! (*delim == 0 || delim == eos)) SYNTAX_ERROR
    }
    if (error) {
	return ((INT_TIME *)NULL);
    }

/*::
    printf ("year = %d, month = %d\n",
	    et.year, et.month);
::*/

    it = ext_to_int (et);
    return (&it);
}

#define	SETSIGN(p,sign)	\
	switch (*p) { \
	  case '+': sign=+1; p++; break; \
	  case '-': sign=-1; p++; break; \
	  default:  break; \
	}
/************************************************************************/
/*  parse_interval:							*/
/*	Parse a time interval into an EXT_TIME struct.			*/
/*	Return NULL if error parsing the date string.			*/
/*	Note that we must use an EXT_TIME strucuture since we need to	*/
/*	preserve all units as they were specified.  Only once we add	*/
/*	the interval to	a base time can we convert to an INT_TIME,	*/
/*	due to the possible presence of leapseconds.			*/
/*  return:								*/
/*	Interval stored in EXT_TIME structure.				*/
/************************************************************************/
EXT_TIME *parse_interval
   (char	*str)		/* string containing interval to parse.	*/
{
    /*
    Permissible input formats:
	yy,ddd,hh:mm:ss.ffff
	yy,ddd hh:mm:ss.ffff
    where
        yy = year
	ddd = day-of-year (1-n)
	hh = hour (0-23)
	mm = minute (0-59)
	ss = second (0-59)
	ffff = fractional part of second
    The time is optional.  If not specified, it is 00:00:00.0000
    */

    static EXT_TIME	et;
    char	*p = str;
    char	*delim, *eos;
    int		trip, nd;
    int		error = 0;
    int		sign = 1;

    et.year = et.doy = et.month = et.day = 0;
    et.hour = et.minute = et.second = et.usec = 0;

    /* Skip trailing blanks. */
    eos = str + strlen(str);
    while (eos > str && *(eos-1) == ' ') --eos;

    for (trip=0; trip<1; trip++) {

	/* Skip leading blanks. */
	while (*p==' ') p++;

	/* Parse year */
	SETSIGN(p,sign)
	et.year = sign * strtol (p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (*delim == 0) SYNTAX_ERROR

	/* Parse date.	*/
	if (*delim != '.' && *delim != '/' && *delim != ',') SYNTAX_ERROR
	p = ++delim;
	SETSIGN(p,sign)
	et.doy = sign * strtol (p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (*delim == 0) SYNTAX_ERROR

	/* Parse time.	*/
	if (*delim != '.' && *delim != '/' && *delim != ',' && *delim != ' ') SYNTAX_ERROR
	/* Skip over multiple blanks */
	for (; *delim==' ' && *(delim+1)==' '; delim++) ;
	p = ++delim;
	SETSIGN(p,sign)
	et.hour = sign * strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR
	if (*delim == 0 || delim == eos) SYNTAX_ERROR

	if (*delim != ':') SYNTAX_ERROR
	p = ++delim;
	SETSIGN(p,sign)
	et.minute = sign * strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR

	if (*delim == 0 || delim == eos) break;
	if (*delim != ':') SYNTAX_ERROR
	p = ++delim;
	SETSIGN(p,sign)
	et.second = sign * strtol(p, &delim, 10);
	if (delim == p) SYNTAX_ERROR

	if (*delim == 0 || delim == eos) break;
	if (*delim != '.') SYNTAX_ERROR
	p = ++delim;
	SETSIGN(p,sign)
	et.usec = sign * strtol(p, &delim, 10);
	if (delim == p && p != eos) SYNTAX_ERROR
	if (! (*delim == 0 || delim == eos)) SYNTAX_ERROR
	nd = delim-p;
	if (nd < 0 || nd > 6) SYNTAX_ERROR
	while (nd < 6) {
	    et.usec *= 10;
	    nd++;
	}
    }
    if (error) {
	return ((EXT_TIME *)NULL);
    }

/*::
    printf ("year = %d, doy = %d, hour = %d, min = %d, sec = %d, usec = %d\n",
	    et.year, et.doy, et.hour, et.minute, et.second, et.usec);
::*/
    return (&et);
}

/************************************************************************/
/*  valid_span:								*/
/*	Ensure time span has valid syntax.				*/
/*  return:								*/
/*	1 if string is valid timespan, 0 otherwise.			*/
/************************************************************************/
int valid_span
   (char	*span)		/* string containing timespan.		*/
{
    char	*p;
    int		span_value = strtol(span,&p,10);
    return ((span_value == 0 || p == span || (int)strlen(p) > 1 ||
	     (strlen(p)==1 && strchr ("FSMHdmy",*p) == NULL)) ? 0 : 1);
}

/************************************************************************/
/*  end_of_span:							*/
/*	Compute the end time of a time span.				*/
/*  return:								*/
/*	INT_TIME containing the end of the time span.			*/
/*	Uninitialized time if invalid time span.			*/
/************************************************************************/
INT_TIME end_of_span
   (INT_TIME	it,		/* INT_TIME with initial time.		*/
    char	*span)		/* string containing timespan.		*/
{
    EXT_TIME	et;
    char	*p;
    int		l;
    int		span_value;
    int		second = 0;
    int		usec = 0;
    INT_TIME	invalid_time;

    /*	Compute end of span time based on initial time and specified	*/
    /*	span value.  Units for span values can be:			*/
    /*	    F -			Ticks.					*/
    /*	    U -			Usecs.					*/
    /*	    S (or nothing) -	Seconds.				*/
    /*	    M -			Minutes.				*/
    /*	    H -			Hours.					*/
    /*	    d -			Days.					*/
    /*	    m -			Month					*/
    /*	    y -			Year.					*/
    /*	Process the span by:						*/
    /*	    Convert beginning internal time to external time.		*/
    /*	    Add the span value to the corresponding external time field.*/
    /*		(for months, normalize and get doy).			*/
    /*	    Normalize external time.					*/
    /*	    Convert external time back to internal time.		*/

    et = int_to_ext (it);

    if (! valid_span(span)) {
	fprintf (stderr, "Error: invalid span value: %s\n", span);
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	memset (&invalid_time, 0, sizeof(invalid_time));
	return (invalid_time);
    }
    span_value = strtol(span,&p,10);

    switch (*p) {
      case  0:
      case 'S':	et.second += span_value; break;
      case 'F':	et.usec += span_value * USECS_PER_TICK ; break;
      case 'U':	et.usec += span_value ; break;
      case 'M':	et.minute += span_value; break;
      case 'H':	et.hour += span_value; break;
      case 'd':	et.doy += span_value; break;
      case 'm':	et.month += span_value;
	while (et.month > 12) { ++et.year; et.month -=12; }
	while (et.month <= 0) { --et.year; et.month +=12; }
	et.doy = mdy_to_doy (et.month, et.day, et.year);
	break;
      case 'y':	et.year += span_value; break;
      default:
	fprintf (stderr, "Error: invalid span value: %s\n", span);
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	memset (&invalid_time, 0, sizeof(invalid_time));
	return (invalid_time);
    }
    return (ext_to_int(normalize_ext(et)));
}

/************************************************************************/
/*  add_interval:							*/
/*	Add interval to specified internal time, and return result.	*/
/*  return:								*/
/*	INT_TIME containing input time + time interval.			*/
/*	Uninitialized time if invalid interval.				*/
/************************************************************************/
INT_TIME add_interval
   (INT_TIME	it,		/* INT_TIME containing initial time.	*/
    EXT_TIME	interval)	/* EXT_TIME containing time interval.	*/
{
    INT_TIME	it2;
    EXT_TIME	et;
    INT_TIME	invalid_time;

    if (interval.doy && (interval.month || interval.day)) {
	fprintf (stderr, "Error: Interval may not have month,day and doy\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	memset (&invalid_time, 0, sizeof(invalid_time));
	return (invalid_time);
    }

    /*	Add in the various parts of the interval in a specified order.	*/
    /*	Start with the highest unit first, and go down from there.	*/
    /*	Normalize after each unit is added.				*/
    /*	Note that seconds must be accrued in int_time format.		*/
    et = int_to_ext (it);
    if (interval.year)	{
	et.year += interval.year;
	et = normalize_ext(et);
    }
    if (interval.doy)	{
	et.doy += interval.doy;
	et = normalize_ext(et);
    }
    if (interval.month)	{
	et.month += interval.month;
	et = normalize_ext(et);
    }
    if (interval.day)	{
	et.day += interval.day;
	et = normalize_ext(et);
    }
    if (interval.hour)	{
	et.hour += interval.hour;
	et = normalize_ext(et);
    }
    if (interval.minute) {
	et.minute += interval.minute;
	et = normalize_ext(et);
    }
    it2 = ext_to_int (et);
    if (interval.second) { it2.second += interval.second; it2 = normalize_time(it2); }
    if (interval.usec)	{ it2.usec += interval.usec; it2 = normalize_time(it2); }
    return (it2);
}

/************************************************************************/
/*  int_time_from_time_tm:						*/
/*	Convert unix struct tm * to INT_TIME.				*/
/************************************************************************/
INT_TIME int_time_from_time_tm
   (struct tm	*tm)		/* ptr to time to convert.		*/
{
    EXT_TIME	et;
    et.year = tm->tm_year + 1900;
    et.doy = tm->tm_yday + 1;
    et.month = tm->tm_mon + 1;
    et.day = tm->tm_mday;
    et.hour =tm->tm_hour;
    et.minute = tm->tm_min;
    et.second = tm->tm_sec;
    et.usec = 0;
    return (ext_to_int(et));
}

/************************************************************************/
/*  unix_time_from_ext_time:						*/
/*	Convert EXT_TIME to unix timestamp.				*/
/*	Ticks will be ignored.						*/
/*  return:								*/
/*	Unix time_t timestamp.						*/
/************************************************************************/
time_t unix_time_from_ext_time
   (EXT_TIME	et)		/* EXT_TIME to convert to unix timestamp*/
{
    struct tm tm;
    time_t gtime;

    /* Map into units required by unix time routines.		*/
    /*	NOTE - Posix time does not deal with leapseconds.	*/
    /*	Therefore, if this is a leapsecond, set it to be the	*/
    /*	the previous second.					*/

    tm.tm_sec = (et.second < 60) ? et.second : 59;
    tm.tm_min = et.minute;
    tm.tm_hour = et.hour;
    tm.tm_mday = et.day;			/* 1-31		*/
    tm.tm_mon = et.month - 1;			/* 0-11		*/
    tm.tm_year = et.year - 1900;		/* year - 1900	*/
    tm.tm_wday = 0;
    tm.tm_yday = et.doy - 1;			/* 0-365	*/
    tm.tm_isdst =0;
#if defined(SUNOS) || defined(__APPLE__)
    tm.tm_zone = "GMT";
    tm.tm_gmtoff = 0;
    gtime = timegm (&tm);
#else
    tzset();
    gtime = mktime(&tm) - timezone;
#endif
    return (gtime);
}

/************************************************************************/
/*  unix_time_from_int_time:						*/
/*  	Convert INT_TIME to unix timestamp.				*/
/*	Ticks will be ignored.						*/
/*  return:								*/
/*	Unix time_tm timestamp.						*/
/************************************************************************/
time_t unix_time_from_int_time
   (INT_TIME	it)		/* INT_TIME to convert to Unix timestamp*/
{
    return (unix_time_from_ext_time(int_to_ext(it)));
}

/* Number of seconds in a year, ignoring leap seconds.			*/
#define	fixed_sec_per_year(yr)	\
    ( (365 + IS_LEAP(yr)) * SEC_PER_DAY )

/************************************************************************/
/*  det_time_to_ext:							*/
/*	Convert quanterra detection time to internal time, ignoring	*/
/*	leap seconds.							*/
/*  return:								*/
/*	EXT_TIME structure containing detection time.			*/
/************************************************************************/
EXT_TIME det_time_to_int_time
   (long	evtsec,		/* quanterra seconds for detection time.*/
    int		msec)		/* quanterra msecs for detection time.	*/
{
    EXT_TIME	et;
    int		n;

    /*  NOTE:  The quanterra detection time is represented as:		*/
    /*  a.	(long) #seconds since Jan 1 1984			*/
    /*  b.	(short) milliseconds.					*/
    /*  The #seconds was computed in the quanterra from an EXT_TIME	*/
    /*	with NO KNOWLEDGE OF LEAP SECONDS.  Therefore, we must convert	*/
    /*  if back without using leap second info.				*/
    /*	Note that we expect msec argument to be an int.			*/

    /*	Determine the proper year.					*/
    et.year = 1984;
    while (evtsec > (n = fixed_sec_per_year(et.year))) {
	evtsec -= n;
	++et.year;
    }

    /*	Determine the rest of the year information.			*/
    et.doy = (et.second / SEC_PER_DAY) + 1;
    et.second = et.second % SEC_PER_DAY;
    et.hour = et.second / SEC_PER_HOUR;
    et.second = et.second % SEC_PER_HOUR;
    et.minute = et.second / SEC_PER_MINUTE;
    et.second = et.second % SEC_PER_MINUTE;
    et.usec = msec * USECS_PER_MSEC;
    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
    return (et);
}

/************************************************************************/
/*  int_time_from_timeval:						*/
/*	Convert unix timeval structure to INT_TIME.			*/
/*  return:								*/
/*	INT_TIME of unix timeval structure.				*/
/************************************************************************/
INT_TIME int_time_from_timeval
   (struct timeval *tv)		/* ptr to struct timeval with input time*/
{
	INT_TIME it;
	it = int_time_from_time_tm(gmtime(&(tv->tv_sec)));
	it.usec = tv->tv_usec;
	return (it);
}

/************************************************************************/
/* Fortran interludes to qtime routines.				*/
/************************************************************************/

/* Add a number of seconds and usecs to INT_TIME, and return result.	*/
#ifdef	fortran_suffix
void f_add_time_
#else
void f_add_time
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    int		*seconds,	/* Number of seconds to add.		*/
    int		*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot)		/* Resultant time.			*/
{
    *ot = add_time(*it, *seconds, *usecs);
}

/* Add a number of usecs to INT_TIME, and return result.		*/
#ifdef	fortran_suffix
void f_add_dtime_
#else
void f_add_dtime
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    double	*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot)		/* Resultant time.			*/
{
    *ot = add_dtime(*it, *usecs);
}

/* Return the time spanned by N samples at RATE sample rate.		*/
/* Returned time is represented by seconds and usecs.			*/
#ifdef	fortran_suffix
void f_time_interval2_
#else
void f_time_interval2
#endif
   (int		*n,		/* number of samples.			*/
    int		*rate,		/* sample rate.				*/
    int		*rate_mult,	/* sample rate_mult in qlib convention.	*/
    int		*seconds,	/* result interval for n samples (sec)	*/
    int		*usecs)		/* result interval for n samples (usec)	*/
{
    time_interval2 (*n, *rate, *rate_mult, seconds, usecs);
}

/* Compute the number of samples that would span the specified time	*/
/* (in usec) at the specified sample rate.				*/
#ifdef	fortran_suffix
double f_dsamples_in_time2_
#else
double f_dsamples_in_time2
#endif
   (int		*rate,		/* sample rate.				*/
    int		*rate_mult,	/* sample rate_mult in qlib convention.	*/
    double	*dusecs)	/* number of usecs.			*/
{
    return (dsamples_in_time2(*rate, *rate_mult, *dusecs));
}

/* Compute the time difference in usecs of time1 - time2.		*/
#ifdef	fortran_suffix
double f_tdiff_
#else
double f_tdiff
#endif
   (INT_TIME	*it1,		/* time1.				*/
    INT_TIME	*it2)		/* time2.  Return (time1-time2)		*/
{
    return (tdiff(*it1,*it2));
}

/*  Return month and day from day,year info.  Handle leap years.	*/
#ifdef	fortran_suffix
void f_dy_to_mdy_
#else
void f_dy_to_mdy
#endif
   (int		*day,		/* day of year (input).			*/
    int		*year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*doy)		/* day of month (returned).		*/
{
    dy_to_mdy (*day, *year, month, doy);
}

/*  Return day_of_year from month, day, year info.			*/
#ifdef	fortran_suffix
int f_mdy_to_doy_
#else
int f_mdy_to_doy
#endif
   (int		*month,		/* month of year (input).		*/
    int		*day,		/* day of month (input).		*/
    int		*year)		/* year (input).			*/
{
    return (mdy_to_doy (*month, *day, *year));
}

/* Convert INT_TIME to EXT_TIME.					*/
#ifdef	fortran_suffix
void f_int_to_ext_
#else
void f_int_to_ext
#endif
   (INT_TIME	*it,		/* input INT_TIME to be convert.	*/
    EXT_TIME	*et)		/* returned equivalent EXT_TIME.	*/
{
    *et = int_to_ext(*it);
}

/* Convert EXT_TIME to INT_TIME.					*/
#ifdef	fortran_suffix
void f_ext_to_int_
#else
void f_ext_to_int
#endif
   (EXT_TIME	*et,		/* input EXT_TIME to be converted.	*/
    INT_TIME	*it)		/* returned equivalent INT_TIME.	*/
{
    *it = ext_to_int(*et);
}

/* Convert INT_TIME to tepoch.						*/
#ifdef	fortran_suffix
void f_int_to_tepoch_
#else
void f_int_to_tepoch
#endif
   (INT_TIME	*it,		/* input INT_TIME to be converted.	*/
    double	*tepoch)	/* returned true epoch time.		*/

{
    *tepoch = int_to_tepoch(*it);
}

/* Convert tepoch to INT_TIME.						*/
#ifdef	fortran_suffix
void f_tepoch_to_int_
#else
void f_tepoch_to_int
#endif
   (double	*tepoch,	/* true epoch to convert to INT_TIME.	*/
    INT_TIME	*it)		/* returned equivalent INT_TIME.	*/
{
    *it = tepoch_to_int(*tepoch);
}

/* Convert INT_TIME to nepoch.						*/
#ifdef	fortran_suffix
void f_int_to_nepoch_
#else
void f_int_to_nepoch
#endif
   (INT_TIME	*it,		/* input INT_TIME to be converted.	*/
    double	*nepoch)	/* returned true epoch time.		*/

{
    *nepoch = int_to_nepoch(*it);
}

/* Convert nepoch to INT_TIME.						*/
#ifdef	fortran_suffix
void f_nepoch_to_int_
#else
void f_nepoch_to_int
#endif
   (double	*nepoch,	/* true epoch to convert to INT_TIME.	*/
    INT_TIME	*it)		/* returned equivalent INT_TIME.	*/
{
    *it = nepoch_to_int(*nepoch);
}

/* Convert INT_TIME to ascii string, according to specified format.	*/
#ifdef	fortran_suffix
void f_time_to_str_
#else
void f_time_to_str
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    int		*fmt,		/* format number for string.		*/
    char	*str,		/* output characters string.		*/
    int		slen)		/* (fortran supplied) length of string.	*/
{
    char *tstr;
    int mlen, i;
    tstr = time_to_str(*it, *fmt);
    mlen = strlen(tstr);
    mlen = (slen < mlen) ? slen : mlen;
    strncpy (str, tstr, mlen);
    /* blank pad if necessary */
    for (i=mlen; i<slen; i++) str[i] = ' ';
}

/* Int function to parse a date/time string into an INT_TIME structure.	*/
/* Return 1 if successful, 0 if unsuccessful.				*/
#ifdef	fortran_suffix
int f_parse_date_
#else
int f_parse_date
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    char	*str,		/* output characters string.		*/
    int		slen)		/* (fortran supplied) length of string.	*/
{
    INT_TIME *pt;
    char tstr[40];
    int tlen = 40;
    int i;

    i = (slen < tlen) ? slen : tlen-1;
    strncpy (tstr, str, i);
    tstr[i] = 0;
    trim (tstr);
    pt = parse_date(tstr);
    if (pt != NULL) {
	*it = *pt;
	return (1);
    }
    else return (0);
}

/* Int function to parse a year.month string into an INT_TIME structure.*/
/* Return 1 if successful, 0 if unsuccessful.				*/
#ifdef	fortran_suffix
int f_parse_date_month_
#else
int f_parse_date_month
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    char	*str,		/* output characters string.		*/
    int		slen)		/* (fortran supplied) length of string.	*/
{
    INT_TIME *pt;
    char tstr[40];
    int tlen = 40;
    int i;

    i = (slen < tlen) ? slen : tlen-1;
    strncpy (tstr, str, i);
    tstr[i] = 0;
    trim (tstr);
    pt = parse_date_month(tstr);
    if (pt != NULL) {
	*it = *pt;
	return (1);
    }
    else return (0);
}
