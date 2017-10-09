#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef	lint
static	char	SccsId[] = "@(#)htoe.c	44.1	9/23/91";
#endif

#include "csstime.h"
static int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};

int htoe(register struct date_time *dt) {
	double dtoepoch();
	dt->epoch = 
	dtoepoch(dt->date) + 
	dt->hour * 3600. + 
	dt->minute * 60. +
	dt->second;
	return(0);
}

int timeprint(register struct date_time *dt) {
	printf("%15.3lf %8d %s %2d,%4d %2d:%02d:%02.3f\n",
	dt->epoch,
	dt->date,
	dt->mname,
	dt->day,
	dt->year,
	dt->hour,
	dt->minute,
	dt->second);
	return(0);
}

int zh_today(register struct date_time *dt) {
	double dtoepoch();
	long todaysdate();
	dt->epoch = dtoepoch( todaysdate());
	etoh(dt);
	return(0);
}

int mdtodate(register struct date_time *dt) {
	int i,dim;
	dt->doy = 0;
	for( i = 0 ; i < dt->month - 1 ; i++ ){
		dim = days_in_month[i];
		if( i == 1 && isleap(dt->year) ) dim++;
		dt->doy += dim;
	}
	dt->doy += dt->day;
	dt->date = 1000 * dt->year + dt->doy;
	return(0);
}
