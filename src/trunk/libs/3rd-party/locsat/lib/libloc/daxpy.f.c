/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdaxpy_1_ {
    char sccsid[80];
};

#define sccsdaxpy_1 (*(struct sccsdaxpy_1_ *) &sccsdaxpy_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdaxpy_ = { {'@', '(', '#', ')', 'd', 'a', 'x', 'p', 'y', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	daxpy -- Multiply constant to vector and add another vector. */
/* FILE */
/* 	daxpy.f */
/* SYNOPSIS */
/* 	LINPACK constant times a vector plus a vector routine. */
/* DESCRIPTION */
/* 	Subroutine.  Given a vector dx() of length n, multiply a constant, */
/* 	da, then add to vector dy().  Typically used in applying */
/* 	transformations, often in conjunction with subroutine ddot. */
/* 	---- On entry ---- */
/* 	n: Length of vector */
/* 	incx:	x-increment loop counter (= 1, if entire loop accessed) */
/* 	incy:	y-increment loop counter (= 1, if entire loop accessed) */
/* 	da:	Scalar constant ultimately multiplied to dx() */
/* 	dx():	Vector to which constant is multiplied */
/* 	dy():	Vector added to dx() */
/* 	---- On return ---- */
/* 	dy():	New vector to which original dy() is added */
/* DIAGNOSTICS */

/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
/* Subroutine */ int daxpy_(n, da, dx, incx, dy, incy)
int *n;
doublereal *da, *dx;
int *incx;
doublereal *dy;
int *incy;
{
    /* System generated locals */
    int i__1;

    /* Local variables */
    static int i__, m, ix, iy, mp1;

/* K.S. 1-Dec-97, change 'undefined' to 'none' */
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
    if (*da == 0.) {
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
	dy[iy] += *da * dx[ix];
	ix += *incx;
	iy += *incy;
/* L1000: */
    }
    return 0;
/*     Code for both increments equal to 1 */
/*     Clean-up loop */
L1010:
    m = *n % 4;
    if (m == 0) {
	goto L1030;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1020: */
	dy[i__] += *da * dx[i__];
    }
    if (*n < 4) {
	return 0;
    }
L1030:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 4) {
	dy[i__] += *da * dx[i__];
	dy[i__ + 1] += *da * dx[i__ + 1];
	dy[i__ + 2] += *da * dx[i__ + 2];
	dy[i__ + 3] += *da * dx[i__ + 3];
/* L1040: */
    }
    return 0;
} /* daxpy_ */

