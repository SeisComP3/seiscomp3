#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef	lint
static	char	SccsId[] = "@(#)tmutils.c	44.1	9/23/91";
#endif

/* collection of time conversion utility subroutines */
#include <ctype.h>
#include <time.h>
	/* convert julian date to epoch time */
double
dtoepoch(date)
long date;
{
	long i,year,day,days=0;
	year = date / 1000;
	day  = date % 1000;

	if( year > 1970 ){
		for( i=1970 ; i < year ; i++ ){
			days += 365;
			if( isleap(i) ) days++;
		}
	}
	if( year < 1970 ) {
		for( i=year ; i < 1970 ; i++ ){
			days -= 365;
			if( isleap(i) ) days--;
		}
	}
	days += day - 1;
	return( (double)days * 86400. );
}
/* return true if leap year else false */
isleap(year)
int year;
{
	return(year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
}
	/* time conversion stolen from original archive */
double
timecon(timstr)
char *timstr;
{
	double tnum,atof();
	char con[20];
	long len,i,j;

	strcpy(con,timstr);
	len = strlen(con);

	for( i=0 ; isdigit(con[i]) && i < len ; i++ );
	con[i] = '\0';		/* cut it off before first non-digit */
	tnum = atoi(con) * 3600.;
	if( i >= len ) return( tnum );

	for( j = ++i ; isdigit(con[j]) && j < len ; j++ );
	con[j] = '\0';
	tnum += atoi(&con[i]) * 60.;
	if( i >= len ) return( tnum );

	for( i= ++j ; (isdigit(con[i]) || con[i] == '.') && i < len ; i++ );
	con[i] = '\0';
	tnum += atof(&con[j]);

	return(tnum);
}
/* return todays date in a long (epoch = Jan 1,1970) */
todaysdate()
{
	long now;
	struct tm *tsp,*gmtime();

	now = time(0);		/* get epoch time (in GMT) */
	tsp = gmtime(&now);	/* disect it */
	now = 1000 * ( tsp->tm_year ) + ( tsp->tm_yday ) + 1;
	return( now + 1900000 );
}
