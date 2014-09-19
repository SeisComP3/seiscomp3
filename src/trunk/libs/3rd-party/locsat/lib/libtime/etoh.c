#include <string.h>
#include <stdlib.h>

#ifndef	lint
static	char	SccsId[] = "@(#)etoh.c	44.1	9/23/91";
#endif

#include "csstime.h"

static int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};
static char *month_name[] =
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define mod(a,b)	a - ((int)(a/b)) * b
void month_day(dt)
register struct date_time *dt;
{
	int i,dim,leap;

	leap = isleap(dt->year);
	dt->day = dt->doy;
	for( i = 0 ; i < 12 ; i ++ ){
		dim = days_in_month[i];
		if( leap && i == 1 ) dim++;
		if( dt->day <= dim ) break;
		dt->day -= dim;
	}
	dt->month = i + 1;
	strcpy(dt->mname,month_name[i]);
}

void etoh(dt)
register struct date_time *dt;
{
	int diy;
	double secleft;

	dt->doy = dt->epoch / 86400.;
	secleft = mod(dt->epoch,86400.0);
	dt->hour = dt->minute = dt->second = 0;

	if(secleft) {			/* compute hours minutes seconds */
		if(secleft < 0) {	/* before 1970 */
			dt->doy--;		/* subtract a day */
			secleft += 86400;	/* add a day */
		}
		dt->hour = secleft/3600;
		secleft = mod(secleft,3600.0);
		dt->minute = secleft/60;
		dt->second = mod(secleft,60.0);
	}

	if(dt->doy >= 0){
		for( dt->year = 1970 ; ; dt->year++ ){
			diy = isleap(dt->year) ? 366:365;
			if( dt->doy < diy ) break;
			dt->doy -= diy;
		}
	}
	else{
		for( dt->year = 1969 ; ; dt->year-- ){
			diy = isleap(dt->year) ? 366:365;
			dt->doy += diy;
			if( dt->doy >= 0 ) break;
		}
	}
	dt->doy++;
	dt->date = dt->year * 1000 + dt->doy;
	month_day(dt);
	return;
}
