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

#endif /* DB_ARRIVAL_H */
