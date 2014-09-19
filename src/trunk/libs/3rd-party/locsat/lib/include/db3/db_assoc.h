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
	long	arid;
	long	orid;
	char	sta[16];
	char	phase[9];
	float	belief;
	float	delta;
	float	seaz;
	float	esaz;
	float	timeres;
	char	timedef[2];
	float	azres;
	char	azdef[2];
	float	slores;
	char	slodef[2];
	float	emares;
	float	wgt;
	char	vmodel[16];
	long	commid;
	char	lddate[18];
} Assoc;

#endif /* DB_ASSOC_H */
