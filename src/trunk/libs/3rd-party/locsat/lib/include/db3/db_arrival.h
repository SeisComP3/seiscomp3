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
	char	sta[16];
	char	chan[9];
	double	time;
	long	arid;
	long	stassid;
	long	chanid;
	long	jdate;
	char	iphase[9];
	char	stype[2];
	float	deltim;
	float	azimuth;
	float	delaz;
	float	slow;
	float	delslo;
	float	ema;
	float	rect;
	float	amp;
	float	per;
	float	logat;
	char	clip[2];
	char	fm[3];
	float	snr;
	char	qual[2];
	char	auth[16];
	long	commid;
	char	lddate[18];
} Arrival;

#endif /* DB_ARRIVAL_H */
