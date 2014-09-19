/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/*
 * FILENAME
 *	db_origin.h
 *
 * DESCRIPTION
 *	C structure declarations for the origin relation.
 *
 * SCCSId:	@(#)db_origin.h	43.1 	9/9/91 Copyright 1990 Science Applications International Corporation
 *
 */

#ifndef DB_ORIGIN_H
#define DB_ORIGIN_H

typedef struct origin {
	float	lat;
	float	lon;
	float	depth;
	double	time;
	long	orid;
	long	evid;
	long	jdate;
	long	nass;
	long	ndef;
	long	ndp;
	long	grn;
	long	srn;
	char	etype[8];
	float	depdp;
	char	dtype[2];
	float	mb;
	long	mbid;
	float	ms;
	long	msid;
	float	ml;
	long	mlid;
	char	algorithm[16];
	char	auth[16];
	long	commid;
	char	lddate[18];
} Origin;

#endif /* DB_ORIGIN_H */
