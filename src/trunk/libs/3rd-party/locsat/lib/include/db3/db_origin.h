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
	float  lat;
	float  lon;
	float  depth;
	double time;
	long   nass;
	long   ndef;
	long   ndp;
	char   auth[16];
} Origin;

#endif /* DB_ORIGIN_H */
