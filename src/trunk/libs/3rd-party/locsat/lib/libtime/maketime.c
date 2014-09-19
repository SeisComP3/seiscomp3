#ifndef	lint
static	char	SccsId[] = "@(#)maketime.c	44.1	9/23/91";
#endif

#include	"pfile.h"

/* This is a subroutine to convert a date and time from an
   intermediate structure to a double precision "time in seconds from 1 Jan
   1970, 00:00:00.00."
   If any of the integer structure values are out of range,
   the result will be returned as 0.
*/

int ndays2[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

double maketime(intime)
	intime_t intime;
{
	double tdays;
	int inc,doy,nlpyrs,dyears;
	int n4,n100,n400;
	static int ndays[13] = {0,0,31,59,90,120,151,181,212,243,273,304,334};

	/* check values are in legal ranges */

	if (intime.tdoy == 0) {
		if ((intime.tmon < 1) || (intime.tmon > 12)) return(0.);
		if (LEAP(intime.tyr) && intime.tmon == 2) inc=1;
		else inc=0;
		if ((intime.tdom < 1) || (intime.tdom > ndays2[intime.tmon]+inc)) return(0.);
	}
	else if ((intime.tdoy < 1) || (intime.tdoy > 365+LEAP(intime.tyr))) return(0.);
	if ((intime.thr < 0) || (intime.thr > 23)) return(0.);
	if ((intime.tmin < 0) || (intime.tmin > 59)) return(0.);
	if ((intime.tsec < 0) || (intime.tsec > 59)) return(0.);
	if (intime.tfract>= 1.) return(0.);


	/* if given month and day, convert to day of year */

	doy = intime.tdoy;
	if (doy == 0) {
		if (LEAP(intime.tyr) && (intime.tmon > 2)) inc = 1;
		else inc = 0;
		doy = ndays[intime.tmon] + intime.tdom + inc;
	}

	/* convert year to number of days since Jan 1, 1970 */

	dyears = intime.tyr - 1970;
	if (dyears<0) {
		n4 = (2-dyears)/4;
		n100 = (30-dyears)/100;
		n400 = (30-dyears)/400;
	}
	else{
		n4 = (dyears+1)/4;
		n100 = (dyears+69)/100;
		n400 = (dyears+369)/400;
	}
	nlpyrs = n4 - n100 + n400;
	if (dyears < 0) nlpyrs = -nlpyrs;
	tdays = (dyears * (double)365) + nlpyrs;

	/* return time in units of seconds */

	return((((tdays+doy-1)*(double)24 + intime.thr) * (double)60 + intime.tmin) * (double)60 +
	  intime.tsec + intime.tfract);
}
