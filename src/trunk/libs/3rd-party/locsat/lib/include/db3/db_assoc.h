/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/*
 * FILENAME
 *	db_assoc.h
 *
 * DESCRIPTION
 *	C structure declarations for the assoc relation.
 *
 * SCCSId:	@(#)db_assoc.h	43.1 	9/9/91 Copyright 1990 Science Applications International Corporation
 *
 */

#ifndef DB_ASSOC_H
#define DB_ASSOC_H

typedef struct assoc {
	long  arid;
	char  sta[16];
	char  phase[9];
	float belief;
	float delta;
	float seaz;
	float esaz;
	float timeres;
	char  timedef[2];
	float azres;
	char  azdef[2];
	float slores;
	char  slodef[2];
	float emares;
	float wgt;
	char  vmodel[16];
} Assoc;


#define Na_Assoc_Init \
{ \
	-1,	/*	arid 	*/ \
	"-",	/*	sta 	*/ \
	"-",	/*	phase 	*/ \
	-1.0,	/*	belief 	*/ \
	-1.0,	/*	delta 	*/ \
	-999.0,	/*	seaz 	*/ \
	-999.0,	/*	esaz 	*/ \
	-999.0,	/*	timeres 	*/ \
	"-",	/*	timedef 	*/ \
	-999.0,	/*	azres 	*/ \
	"-",	/*	azdef 	*/ \
	-999.0,	/*	slores 	*/ \
	"-",	/*	slodef 	*/ \
	-999.0,	/*	emares 	*/ \
	-1.0,	/*	wgt 	*/ \
	"-" 	/*	vmodel 	*/ \
}


#endif /* DB_ASSOC_H */
