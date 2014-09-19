#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef	lint
static	char	SccsId[] = "@(#)makeintt.c	44.1	9/23/91";
#endif

#include	"pfile.h"

double di400y=146097.;	/* days in 400 years */
double di100y=36524.;	/* days in 100 years */
double di4y=1461.;	/* days in 4 years */

makeintt (time,intime)	/* convert double time to structure intime */
double time;
intime_t *intime;
{
	double tot,days,secs,hours,mins,n400yp,n100yp,n4yp,n1yp,i,secs70;
	double year,modf();
	int month,dim;

	secs70=31536000.*1492.+31622400.*477.;	/* number of seconds from year 1 to Jan 1., 1970 */
	time+=secs70;
	intime->tfract=modf(time,&tot);
	modf(tot/86400.,&days);
	secs=tot-days*86400.;
	modf(secs/3600.,&hours);
	secs-=hours*3600.;
	modf(secs/60.,&mins);
	secs-=mins*60.;
	intime->tsec=(int)secs;
	intime->tmin=(int)mins;
	intime->thr=(int)hours;
	modf(days/di400y,&n400yp);	/* find number of 400 year periods */
	days-=di400y*n400yp;	/* days < 400 years */
	if (days == 146096.) n100yp=3;	/* last year in a 400 year period */
	else modf(days/di100y,&n100yp);
	days-=di100y*n100yp;	/* days < 100 years */
	modf(days/di4y,&n4yp);	/* find number of 4 year periods */
	days-=di4y*n4yp;	/* days < 4 years */
	modf(days/365.,&n1yp);		/* find number of 1 year periods */
	if (n1yp == 4) n1yp=3;
	days-=365.*n1yp;	/* days < 1 year */
	year=400.*n400yp+100.*n100yp+4.*n4yp+n1yp+1.;	/* compute year */
	days+=1.;
	i=dom((int)days,&month,&dim,(int)year);	/* days = day < 1 year ; mon = month 1-12, returned */
/* dim = days in month 1-31, returned ; year = the year, passed only to test for leap year */
	if (i <= 0) {
		printf("ERROR makeintt - unable to convert time (%f)\n",time);
		printf("%d returned by dom\n",i);
		return(0);
	}
	intime->tyr=(int)year;		/* year */
	intime->tdoy=(int)days;	/* day of year */
	intime->tmon=month;	/* month */
	intime->tdom=dim;	/* day of month */
	return(1);
}
