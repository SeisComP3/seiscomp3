/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/*
 * FILENAME
 *	db_arrival.h
 *
 * DESCRIPTION
 *	C structure declarations for the arrival relation.
 *
 * SCCSId:	@(#)db_arrival.h	43.1 	9/9/91 Copyright 1990 Science Applications International Corporation
 *
 */

#ifndef DB_ARRIVAL_H
#define DB_ARRIVAL_H

typedef struct arrival 
{
	char   sta[16];
	double time;
	long   arid;
	char   iphase[9];
	char   stype[2];
	float  deltim;
	float  azimuth;
	float  delaz;
	float  slow;
	float  delslo;
} Arrival;


#define Na_Arrival_Init \
{ \
	"-",	/*	sta 	*/ \
	-9999999999.999,	/*	time 	*/ \
	-1,	/*	arid 	*/ \
	"-",	/*	iphase 	*/ \
	"-",	/*	stype 	*/ \
	-1.0,	/*	deltim 	*/ \
	-1.0,	/*	azimuth 	*/ \
	-1.0,	/*	delaz 	*/ \
	-1.0,	/*	slow 	*/ \
	-1.0	/*	delslo 	*/ \
}


#endif /* DB_ARRIVAL_H */
