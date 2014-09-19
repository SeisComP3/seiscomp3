/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdswap_1_ {
    char sccsid[80];
};

#define sccsdswap_1 (*(struct sccsdswap_1_ *) &sccsdswap_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdswap_ = { {'@', '(', '#', ')', 'd', 's', 'w', 'a', 'p', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	dswap -- Interchange two vectors. */
/* FILE */
/* 	dswap.f */
/* SYNOPSIS */
/* 	LINPACK routine to interchange (swap) two vectors. */
/* DESCRIPTION */
/* 	Subroutine.  Interchange (swap) vector dx() for dy() and */
/* 	vice-a-versa. */
/* 	---- On entry ---- */
/* 	n:	Length of vector */
/* 	incx:	x-increment loop counter (= 1, if entire loop accessed) */
/* 	incy:	y-increment loop counter (= 1, if entire loop accessed) */
/* 	dx():	First  vector */
/* 	dy():	Second vector */
/* 	---- On return ---- */
/* 	dx():	New swaped vector, old dy() or second vector */
/* 	dy():	New swaped vector, old dx() or first vector */
/* DIAGNOSTICS */

/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
/* Subroutine */ int dswap_(n, dx, incx, dy, incy)
integer *n;
doublereal *dx;
integer *incx;
doublereal *dy;
integer *incy;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, m;
    static doublereal dtemp;
    static integer ix, iy, mp1;

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
	dtemp = dx[ix];
	dx[ix] = dy[iy];
	dy[iy] = dtemp;
	ix += *incx;
	iy += *incy;
/* L1000: */
    }
    return 0;
/*     Code for both increments equal to 1 */
/*     Clean-up loop */
L1010:
    m = *n % 3;
    if (m == 0) {
	goto L1030;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	dtemp = dx[i__];
	dx[i__] = dy[i__];
	dy[i__] = dtemp;
/* L1020: */
    }
    if (*n < 3) {
	return 0;
    }
L1030:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 3) {
	dtemp = dx[i__];
	dx[i__] = dy[i__];
	dy[i__] = dtemp;
	dtemp = dx[i__ + 1];
	dx[i__ + 1] = dy[i__ + 1];
	dy[i__ + 1] = dtemp;
	dtemp = dx[i__ + 2];
	dx[i__ + 2] = dy[i__ + 2];
	dy[i__ + 2] = dtemp;
/* L1040: */
    }
    return 0;
} /* dswap_ */

