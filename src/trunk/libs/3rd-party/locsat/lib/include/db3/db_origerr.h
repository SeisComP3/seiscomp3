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


#define Na_Origerr_Init \
{ \
	-1.0,   /*      sxx     */ \
	-1.0,   /*      syy     */ \
	-1,     /*      szz     */ \
	-1.0,   /*      stt     */ \
	-1.0,   /*      sxy     */ \
	-1.0,   /*      sxz     */ \
	-1.0,   /*      syz     */ \
	-1.0,   /*      stx     */ \
	-1.0,   /*      sty     */ \
	-1.0,   /*      stz     */ \
	-1.0,   /*      sdobs   */ \
	-1.0,   /*      smajax  */ \
	-1.0,   /*      sminax  */ \
	-1.0,   /*      strike  */ \
	-1.0,   /*      sdepth  */ \
	-1.0,   /*      stime   */ \
	 0.0    /*      conf    */ \
}


#endif /* DB_ORIGERR_H */
