/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"
#define ftnlen int

/* Common Block Declarations */

struct sccssplie2_1_ {
    char sccsid[80];
};

#define sccssplie2_1 (*(struct sccssplie2_1_ *) &sccssplie2_)

struct sccssplin2_1_ {
    char sccsid[80];
};

#define sccssplin2_1 (*(struct sccssplin2_1_ *) &sccssplin2_)

struct sccsspline_1_ {
    char sccsid[80];
};

#define sccsspline_1 (*(struct sccsspline_1_ *) &sccsspline_)

struct sccssplint_1_ {
    char sccsid[80];
};

#define sccssplint_1 (*(struct sccssplint_1_ *) &sccssplint_)

/* Initialized data */

struct {
    char e_1[80];
    } sccssplie2_ = { {'@', '(', '#', ')', 'b', 'i', 'c', 'u', 'b', 'e', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };

struct {
    char e_1[80];
    } sccssplin2_ = { {'@', '(', '#', ')', 'b', 'i', 'c', 'u', 'b', 'e', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };

struct {
    char e_1[80];
    } sccsspline_ = { {'@', '(', '#', ')', 'b', 'i', 'c', 'u', 'b', 'e', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };

struct {
    char e_1[80];
    } sccssplint_ = { {'@', '(', '#', ')', 'b', 'i', 'c', 'u', 'b', 'e', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static float c_b4 = (float)1e30;
static int c__9 = 9;
static int c__1 = 1;

/* NAME */
/* 	splie2 -- Construct natural splines and second derivatives. */
/* FILE */
/* 	splie2.f */
/* SYNOPSIS */
/* 	Perform 1-D natural cubic splines on rows and return the second */
/* 	derivatives. */
/* DESCRIPTION */
/* 	Subroutine.  Given an m by n tabulated function ya(1..m,1..n), and */
/* 	tabulated independent variables x1a(1..m) and x2a(1..n), this */
/* 	routine constructs one-dimensional natural cubic splines of the */
/* 	rows of ya and returns the second derivatives in the array */
/* 	y2a(1..m,1..n). */
/* 	---- Subroutines called ---- */
/* 		spline:	Return 2nd derivatives of an interpolating function */
/* DIAGNOSTICS */
/* 	Values returned larger than 1.0e30 signal a natual spline. */
/* FILES */

/* NOTES */

/* SEE ALSO */
/* 	Press, W.H. et al., 1988, "Numerical Recipes", 94-110. */
/* AUTHOR */

/* Subroutine */ int splie2_(x1a, x2a, ya, m, n, y2a)
float *x1a, *x2a, *ya;
int *m, *n;
float *y2a;
{
    /* System generated locals */
    int i__1, i__2;

    /* Local variables */
    static float ytmp[15];
    static int j, k;
    static float y2tmp[15];
    extern /* Subroutine */ int spline_();

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --x1a;
    y2a -= 16;
    ya -= 16;
    --x2a;

    /* Function Body */
    i__1 = *m;
    for (j = 1; j <= i__1; ++j) {
	i__2 = *n;
	for (k = 1; k <= i__2; ++k) {
	    ytmp[k - 1] = ya[j + k * 15];
/* L1000: */
	}
	spline_(&x2a[1], ytmp, n, &c_b4, &c_b4, y2tmp);
	i__2 = *n;
	for (k = 1; k <= i__2; ++k) {
	    y2a[j + k * 15] = y2tmp[k - 1];
/* L1010: */
	}
/* L1020: */
    }
    return 0;
} /* splie2_ */

/* NAME */
/* 	splin2 -- Return values of a bi-cubic interpolated function. */
/* FILE */
/* 	splin2.f */
/* SYNOPSIS */
/* 	Return an interpolated function value by bi-cubic interpolation. */
/* DESCRIPTION */
/* 	Subroutine.  Given x1a, x2a, ya, m, n as described in subr. splie2 */
/* 	and y2a as produced by that routine; and given a desired */
/* 	interpolating point x1, x2; this routine returns an interpolated */
/* 	function value y by bi-cubic spline interpolation. */
/* 	---- Subroutines called ---- */
/* 		spline: Return 2nd derivatives of an interpolating function */
/* 		splint: Return a cubic spline interpolated value */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */
/* 	The above is accomplished by constructing row, then column splines, */
/* 	one-dimension at a time. */
/* SEE ALSO */
/* 	Press, W.H. et al., 1988, "Numerical Recipes", 94-110. */
/* AUTHOR */

/* Subroutine */ int splin2_(x1a, x2a, ya, y2a, m, n, x1, x2, y)
float *x1a, *x2a, *ya, *y2a;
int *m, *n;
float *x1, *x2, *y;
{
    /* System generated locals */
    int i__1, i__2;

    /* Local variables */
    static float ytmp[15];
    static int j, k;
    static float y2tmp[15], yytmp[15];
    extern /* Subroutine */ int spline_(), splint_();

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --x1a;
    y2a -= 16;
    ya -= 16;
    --x2a;

    /* Function Body */
    i__1 = *m;
    for (j = 1; j <= i__1; ++j) {
	i__2 = *n;
	for (k = 1; k <= i__2; ++k) {
	    ytmp[k - 1] = ya[j + k * 15];
	    y2tmp[k - 1] = y2a[j + k * 15];
/* L1000: */
	}
	splint_(&x2a[1], ytmp, y2tmp, n, x2, &yytmp[j - 1]);
/* L1010: */
    }
    spline_(&x1a[1], yytmp, m, &c_b4, &c_b4, y2tmp);
    splint_(&x1a[1], yytmp, y2tmp, m, x1, y);
    return 0;
} /* splin2_ */

/* NAME */
/* 	spline -- Return the second derivatives of an interpolating function. */
/* FILE */
/* 	spline.f */
/* SYNOPSIS */
/* 	Construct an array of second derivatives based on an interpolating */
/* 	function at given tabulated points. */
/* DESCRIPTION */
/* 	Subroutine.  Given arrays x(1..n) and y(1..n) containing a tabulated */
/* 	function, i.e., y(i) = f(x(i)), with x(1) < x(2) < x(n), and given */
/* 	values yp1 and ypn for the first derivative of the interpolating */
/* 	function at points 1 and n, respectively, this routine returns an */
/* 	array y2(1..n) that contains the second derivatives of the */
/* 	interpolating function at the tabulated points x(i). */
/* DIAGNOSTICS */
/* 	If yp1 and/or ypn are equal to 1.0e30 or larger, the routine is */
/* 	signalled to set the corresponding boundary condition for a natural */
/* 	spline, with zero second derivative on that boundary. */
/* FILES */

/* NOTES */
/* 	Note that this routine only need be called once to process the */
/* 	entire tabulated function in x and y arrays. */
/* SEE ALSO */
/* 	Press, W.H. et al., 1988, "Numerical Recipes", 94-110. */
/* AUTHOR */

/* Subroutine */ int spline_(x, y, n, yp1, ypn, y2)
float *x, *y;
int *n;
float *yp1, *ypn, *y2;
{
    /* System generated locals */
    int i__1;

    /* Local variables */
    static int i__, k;
    static float p, u[15], qn, un, sig;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --y2;
    --y;
    --x;

    /* Function Body */
    if (*yp1 > (float)9.9e29) {
	y2[1] = (float)0.;
	u[0] = (float)0.;
    } else {
	y2[1] = (float)-.5;
	u[0] = (float)3. / (x[2] - x[1]) * ((y[2] - y[1]) / (x[2] - x[1]) - *
		yp1);
    }
/*     Decomposition loop of a tridiagonal algorithm */
    i__1 = *n - 1;
    for (i__ = 2; i__ <= i__1; ++i__) {
	sig = (x[i__] - x[i__ - 1]) / (x[i__ + 1] - x[i__ - 1]);
	p = sig * y2[i__ - 1] + (float)2.;
	y2[i__] = (sig - (float)1.) / p;
	u[i__ - 1] = (((y[i__ + 1] - y[i__]) / (x[i__ + 1] - x[i__]) - (y[i__]
		 - y[i__ - 1]) / (x[i__] - x[i__ - 1])) * (float)6. / (x[i__ 
		+ 1] - x[i__ - 1]) - sig * u[i__ - 2]) / p;
/* L1000: */
    }
    if (*ypn > (float)9.9e29) {
	qn = (float)0.;
	un = (float)0.;
    } else {
	qn = (float).5;
	un = (float)3. / (x[*n] - x[*n - 1]) * (*ypn - (y[*n] - y[*n - 1]) / (
		x[*n] - x[*n - 1]));
    }
    y2[*n] = (un - qn * u[*n - 2]) / (qn * y2[*n - 1] + (float)1.);
/*     Back substituition loop of tridiagonal algorithm */
    for (k = *n - 1; k >= 1; --k) {
	y2[k] = y2[k] * y2[k + 1] + u[k - 1];
/* L1010: */
    }
    return 0;
} /* spline_ */

/* NAME */
/* 	splint -- Return a cubic spline interpolated value. */
/* FILE */
/* 	splint.f */
/* SYNOPSIS */
/* 	This routine return a cubic spline interpolated value from an array. */
/* DESCRIPTION */
/* 	Subroutine.  Given the arrays xa(1..n) and ya(1..n) which tabulate */
/* 	a function (with the xa(i)'s in order), and given the array */
/* 	y2a(1..n), which is the output from subroutine spline(), and given */
/* 	a value of x, this routine returns a cubic-spline interpolated */
/* 	value of y. */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */
/* 	The correct position in the table is obtained by means of bi-section. */
/* SEE ALSO */
/* 	Press, W.H. et al., 1988, "Numerical Recipes", 94-110. */
/* AUTHOR */

/* Subroutine */ int splint_(xa, ya, y2a, n, x, y)
float *xa, *ya, *y2a;
int *n;
float *x, *y;
{
    /* Builtin functions */
    int s_wsle(), do_lio(), e_wsle();

    /* Local variables */
    static float a, b, h__;
    static int k, khi, klo;

    /* Fortran I/O blocks */
    static cilist io___21 = { 0, 6, 0, 0, 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --y2a;
    --ya;
    --xa;

    /* Function Body */
    klo = 1;
    khi = *n;
L1000:
    if (khi - klo > 1) {
	k = (khi + klo) / 2;
	if (xa[k] > *x) {
	    khi = k;
	} else {
	    klo = k;
	}
	goto L1000;
    }
/*     klo and khi now bracket the input value of x */
    h__ = xa[khi] - xa[klo];
    if (h__ == (float)0.) {
	s_wsle(&io___21);
	do_lio(&c__9, &c__1, " Splint: bad input error! ", (ftnlen)26);
	e_wsle();
	return 0;
    }
    a = (xa[khi] - *x) / h__;
    b = (*x - xa[klo]) / h__;
    *y = a * ya[klo] + b * ya[khi] + (a * (a * a - (float)1.) * y2a[klo] + b *
	     (b * b - (float)1.) * y2a[khi]) * h__ * h__ / (float)6.;
    return 0;
} /* splint_ */

