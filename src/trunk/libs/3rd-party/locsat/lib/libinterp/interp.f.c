/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsinterp_1_ {
    char sccsid[80];
};

#define sccsinterp_1 (*(struct sccsinterp_1_ *) &sccsinterp_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsinterp_ = { {'@', '(', '#', ')', 'i', 'n', 't', 'e', 'r', 'p', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	interp -- Hermite cubic interpolation. */
/* FILE */
/* 	interp.f */
/* SYNOPSIS */
/* 	Hermite cubic interpolation routine for function y(x). */
/* DESCRIPTION */
/* 	Subroutine.  Hermitian cubic interpolation routine to act on a */
/* 	function, y(x). */
/* 	---- Indexing ---- */
/* 	i = 1, n; */
/* 	---- On entry ---- */
/* 	n:	Number of function samples */
/* 	x(i):	Sample values of independent variable */
/* 	y(i):	Value of function at x(i); y(x(i)) */
/* 	yp(i):	Value of derivative at x(i); y'(x(i)) */
/* 	x0:	Value of independent variable for interpolation */
/* 	---- On return ---- */
/* 	y0:	Interpolated value of function at x0 */
/* 	yp0:	Interpolated value of derivative at x0 */
/* 	ierr:	Error flag; */
/* 		  = 0,	No error */
/* 		  = 1,	x0 out of range. yp0 and y0 are then defined */
/* 			through linear extrapolation of function */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		- Calls hermit */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */

/* Subroutine */ int interp_(n, x, y, yp, x0, y0, yp0, ierr)
integer *n;
real *x, *y, *yp, *x0, *y0, *yp0;
integer *ierr;
{
    extern /* Subroutine */ int brack_();
    static integer ileft, i1, i2;
    extern /* Subroutine */ int hermit_();

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Binary search for samples bounding x0 */
    /* Parameter adjustments */
    --yp;
    --y;
    --x;

    /* Function Body */
    brack_(n, &x[1], x0, &ileft);
/*     x0 < x(1) */
    if (ileft < 1) {
	*ierr = 1;
	*yp0 = yp[1];
	*y0 = y[1] + *yp0 * (*x0 - x[1]);
	return 0;
    }
/*     x0 > x(n) */
    if (ileft >= *n) {
	*ierr = 1;
	*yp0 = yp[*n];
	*y0 = y[*n] + *yp0 * (*x0 - x[*n]);
	return 0;
    }
/*     Normal case */
    i1 = ileft;
    i2 = ileft + 1;
    hermit_(&x[i1], &x[i2], &y[i1], &y[i2], &yp[i1], &yp[i2], x0, y0, yp0);
    *ierr = 0;
    return 0;
} /* interp_ */

