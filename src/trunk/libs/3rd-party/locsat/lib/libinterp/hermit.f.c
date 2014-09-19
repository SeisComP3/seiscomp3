/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccshermit_1_ {
    char sccsid[80];
};

#define sccshermit_1 (*(struct sccshermit_1_ *) &sccshermit_)

/* Initialized data */

struct {
    char e_1[80];
    } sccshermit_ = { {'@', '(', '#', ')', 'h', 'e', 'r', 'm', 'i', 't', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	hermit -- Two-point Hermite cubic interpolation routine. */
/* FILE */
/* 	hermit.f */
/* SYNOPSIS */
/* 	A simple two-point Hermitian cubic interpolation routine. */
/* DESCRIPTION */
/* 	Subroutine.  Perform a Hermite cubic interpolation of function y(x) */
/* 	bewteen two sample points. */
/* 	---- On entry ---- */
/* 	x1, x2:		Sample values of independent variable */
/* 	y1, y2:		Values of function at x1 and x2, respectively */
/* 	yp1, yp2:	Values of derivative of function at x1 and x2 */
/* 	x0:		Value of independent variable for interpolation */
/* 	---- On return ---- */
/* 	y0:		Interpolated value of function at x0 */
/* 	yp0:		Interpolated value of derivative at x0 */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */

/* Subroutine */ int hermit_(x1, x2, y1, y2, yp1, yp2, x0, y0, yp0)
real *x1, *x2, *y1, *y2, *yp1, *yp2, *x0, *y0, *yp0;
{
    static real a, b, c__, d__, t, f1, f2, df, dx, fp1, fp2, sfp;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
    dx = *x2 - *x1;
    t = (*x0 - *x1) / dx;
    if (t <= (float).5) {
	f1 = *y1;
	f2 = *y2;
	fp1 = *yp1;
	fp2 = *yp2;
    } else {
	t = (float)1. - t;
	dx = -dx;
	f1 = *y2;
	f2 = *y1;
	fp1 = *yp2;
	fp2 = *yp1;
    }
    fp1 *= dx;
    fp2 *= dx;
    df = f2 - f1;
    sfp = fp1 + fp2;
    a = f1;
    b = fp1;
    c__ = df * (float)3. - sfp - fp1;
    d__ = df * (float)-2. + sfp;
    *y0 = ((d__ * t + c__) * t + b) * t + a;
    *yp0 = ((d__ * (float)3. * t + c__ * (float)2.) * t + b) / dx;
    return 0;
} /* hermit_ */

