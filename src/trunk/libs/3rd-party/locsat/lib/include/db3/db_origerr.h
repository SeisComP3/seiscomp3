/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/*
 * FILENAME
 *	db_origerr.h
 *
 * DESCRIPTION
 *	C structure declarations for the origerr relation.
 *
 * SCCSId:	@(#)db_origerr.h	43.1 	9/9/91 Copyright 1990 Science Applications International Corporation
 *
 */

#ifndef DB_ORIGERR_H
#define DB_ORIGERR_H

typedef struct origerr {
	float sxx;
	float syy;
	float szz;
	float stt;
	float sxy;
	float sxz;
	float syz;
	float stx;
	float sty;
	float stz;
	float sdobs;
	float smajax;
	float sminax;
	float strike;
	float sdepth;
	float stime;
	float conf;
} Origerr;

#endif /* DB_ORIGERR_H */
