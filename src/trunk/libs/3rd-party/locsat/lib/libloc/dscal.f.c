/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdscal_1_ {
    char sccsid[80];
};

#define sccsdscal_1 (*(struct sccsdscal_1_ *) &sccsdscal_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdscal_ = { {'@', '(', '#', ')', 'd', 's', 'c', 'a', 'l', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	dscal -- Scale a vector by a constant. */
/* FILE */
/* 	dscal.f */
/* SYNOPSIS */
/* 	LINPACK scalar constant times vector routine. */
/* DESCRIPTION */
/* 	Subroutine.  Multiply a scalar constant, da, times a vector, dx() */
/* 	of length n. */
/* 	---- On entry ---- */
/* 	n:	Length of vector */
/* 	incx:	x-increment loop counter (= 1, if entire loop accessed) */
/* 	da  :	Scalar constant ultimately multiplied to dx() */
/* 	dx():	Vector to which scalar is multiplied */
/* 	---- On return ---- */
/* 	dx():	New scaled vector */
/* DIAGNOSTICS */

/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
/* Subroutine */ int dscal_(n, da, dx, incx)
int *n;
doublereal *da, *dx;
int *incx;
{
    /* System generated locals */
    int i__1, i__2;

    /* Local variables */
    static int i__, m, nincx, mp1;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On entry and return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --dx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1) {
	goto L1010;
    }
/*     Code for increment not equal to 1 */
    nincx = *n * *incx;
    i__1 = nincx;
    i__2 = *incx;
    for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
/* L1000: */
	dx[i__] = *da * dx[i__];
    }
    return 0;
/*     Code for increment equal to 1 */
L1010:
    m = *n % 5;
    if (m == 0) {
	goto L1030;
    }
    i__2 = m;
    for (i__ = 1; i__ <= i__2; ++i__) {
/* L1020: */
	dx[i__] = *da * dx[i__];
    }
    if (*n < 5) {
	return 0;
    }
L1030:
    mp1 = m + 1;
    i__2 = *n;
    for (i__ = mp1; i__ <= i__2; i__ += 5) {
	dx[i__] = *da * dx[i__];
	dx[i__ + 1] = *da * dx[i__ + 1];
	dx[i__ + 2] = *da * dx[i__ + 2];
	dx[i__ + 3] = *da * dx[i__ + 3];
	dx[i__ + 4] = *da * dx[i__ + 4];
/* L1040: */
    }
    return 0;
} /* dscal_ */

