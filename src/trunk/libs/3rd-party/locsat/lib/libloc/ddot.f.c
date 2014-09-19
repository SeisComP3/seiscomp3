/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsddot_1_ {
    char sccsid[80];
};

#define sccsddot_1 (*(struct sccsddot_1_ *) &sccsddot_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsddot_ = { {'@', '(', '#', ')', 'd', 'd', 'o', 't', '.', 'f', '\t', 
	    '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	ddot -- Compute the dot product of two vectors. */
/* FILE */
/* 	ddot.f */
/* SYNOPSIS */
/* 	LINPACK routine to form the dot product of two vectors. */
/* DESCRIPTION */
/* 	Function.  Given a two vectors dx() and dy() of length n, this */
/* 	routine forms their dot product and returns as a scalar in ddot. */
/* 	---- On entry ---- */
/* 	n:	Length of vector */
/* 	incx:	x-storage increment counter (= 1, if entire loop accessed) */
/* 	incy:	y-storage increment counter (= 1, if entire loop accessed) */
/* 	dx():	First vector */
/* 	dy():	Second vector */
/* 	---- On return ---- */
/* 	ddot:	Scalar dot product of dx() and dy() */
/* DIAGNOSTICS */

/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
doublereal ddot_(n, dx, incx, dy, incy)
integer *n;
doublereal *dx;
integer *incx;
doublereal *dy;
integer *incy;
{
    /* System generated locals */
    integer i__1;
    doublereal ret_val;

    /* Local variables */
    static integer i__, m;
    static doublereal dtemp;
    static integer ix, iy, mp1;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     real*8 function ddot */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --dy;
    --dx;

    /* Function Body */
    ret_val = 0.;
    dtemp = 0.;
    if (*n <= 0) {
	return ret_val;
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
	dtemp += dx[ix] * dy[iy];
	ix += *incx;
	iy += *incy;
/* L1000: */
    }
    ret_val = dtemp;
    return ret_val;
/*     Code for both increments equal to 1 */
L1010:
    m = *n % 5;
    if (m == 0) {
	goto L1030;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1020: */
	dtemp += dx[i__] * dy[i__];
    }
    if (*n < 5) {
	goto L1050;
    }
L1030:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 5) {
	dtemp = dtemp + dx[i__] * dy[i__] + dx[i__ + 1] * dy[i__ + 1] + dx[
		i__ + 2] * dy[i__ + 2] + dx[i__ + 3] * dy[i__ + 3] + dx[i__ + 
		4] * dy[i__ + 4];
/* L1040: */
    }
L1050:
    ret_val = dtemp;
    return ret_val;
} /* ddot_ */

