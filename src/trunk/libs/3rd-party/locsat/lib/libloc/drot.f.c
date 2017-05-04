/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdrot_1_ {
    char sccsid[80];
};

#define sccsdrot_1 (*(struct sccsdrot_1_ *) &sccsdrot_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdrot_ = { {'@', '(', '#', ')', 'd', 'r', 'o', 't', '.', 'f', '\t', 
	    '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	drot -- Apply a plane rotation. */
/* FILE */
/* 	drot.f */
/* SYNOPSIS */
/* 	LINPACK routine which applies a simple plane rotation. */
/* DESCRIPTION */
/* 	Subroutine.  Given two vectors dx() and dy() of length n, simply */
/* 	apply a plane rotation and return in the same two vectors. */
/* 	---- On entry ---- */
/* 	n:	Length of vector */
/* 	incx:	x-increment loop counter (= 1, if entire loop accessed) */
/* 	incy:	y-increment loop counter (= 1, if entire loop accessed) */
/* 	dx():	Original unrotated vector */
/* 	dy():	Original unrotated vector */
/* 	---- On return ---- */
/* 	dx():	New vector dx() in rotated system */
/* 	dy():	New vector dy() in rotated system */
/* DIAGNOSTICS */

/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
/* Subroutine */ int drot_(n, dx, incx, dy, incy, c__, s)
int *n;
doublereal *dx;
int *incx;
doublereal *dy;
int *incy;
doublereal *c__, *s;
{
    /* System generated locals */
    int i__1;

    /* Local variables */
    static int i__;
    static doublereal dtemp;
    static int ix, iy;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On entry and return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --dy;
    --dx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L1010;
    }
/*     Code for unequal increments or equal increments not equal to 1 */
    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	dtemp = *c__ * dx[ix] + *s * dy[iy];
	dy[iy] = *c__ * dy[iy] - *s * dx[ix];
	dx[ix] = dtemp;
	ix += *incx;
	iy += *incy;
/* L1000: */
    }
    return 0;
/*     Code for both increments equal to 1 */
L1010:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	dtemp = *c__ * dx[i__] + *s * dy[i__];
	dy[i__] = *c__ * dy[i__] - *s * dx[i__];
	dx[i__] = dtemp;
/* L1020: */
    }
    return 0;
} /* drot_ */

