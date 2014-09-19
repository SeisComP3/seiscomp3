/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsholint_1_ {
    char sccsid[80];
};

#define sccsholint_1 (*(struct sccsholint_1_ *) &sccsholint_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsholint_ = { {'@', '(', '#', ')', 'h', 'o', 'l', 'i', 'n', 't', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	holint -- Monotone, quadratic interpolation routine. */
/* FILE */
/* 	holint.f */
/* SYNOPSIS */
/* 	Perform a monotone, quadratic interpolation, even when holes in the */
/* 	data are present. */
/* DESCRIPTION */
/* 	Subroutine.  Monotone, quadratic interpolation of function f(x) */
/* 	which might have holes (bad values).   Bad function samples are */
/* 	given the value fbad. */
/* 	---- Indexing ---- */
/* 	i = 1, n; */
/* 	---- On entry ---- */
/* 	n:	Number of function samples */
/* 	x(i):	Sample values of independent variable; Must be ordered: */
/* 		x(i) >= x(i-1) */
/* 	f(i):	Value of function at x(i) */
/* 	fbad:	Function value denoting bad sample */
/* 	x0:	Value of independent variable for interpolation */
/* 	---- On return ---- */
/* 	f0:	Interpolated value of function at x0 */
/* 	fp0:	Interpolated value of derivative at x0 */
/* 	iext:	Flag indicating whether extrapolation has occurred; */
/* 		  =  0,	No extrapolation */
/* 		  = -1,	Yes, x0 < x(1) */
/* 		  = +1,	Yes, x0 > x(N) */
/* 	ibad:	Flag indicating whether interopolation point is in a hole; */
/* 		  =  0,	No */
/* 		  =  1,	Yes */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		- Calls brack, fixhol and quaint, directly */
/* 		- Calls brack and hermit, indirectly */

/* DIAGNOSTICS */

/* FILES */

/* NOTES */
/* 	- f(x) may be discontinuous.  A discontinuity is presumed to occur */
/* 	  when x(i) repeats for consecutive i. */
/* 	- If x0 is out of range (iext = -1 or +1), then f0 and fp0 are */
/* 	  defined through linear extrapolation of function. */
/* 	- If f(i) = fbad, then the function is assumed to be fbad in the */
/* 	  intervals on either side of x(i), and then jump discontinuously */
/* 	  to the next good sample value.  See subroutine fixhol for details */
/* 	  on how holes are defined. */
/* 	- If x0 is in a hole (ibad = 1), then f0 is returned fbad and fp0 */
/* 	  is zero. */
/* SEE ALSO */

/* AUTHOR */

/* Subroutine */ int holint_(n, x, f, fbad, x0, f0, fp0, iext, ibad)
integer *n;
real *x, *f, *fbad, *x0, *f0, *fp0;
integer *iext, *ibad;
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer imin, imax, nuse;
    extern /* Subroutine */ int brack_();
    static integer ileft;
    static real fh[6];
    static integer nh;
    static real xh[6];
    extern /* Subroutine */ int fixhol_(), quaint_();

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Find four relevant samples and then construct a version of this */
/*     piece of the function with holes fixed */
    /* Parameter adjustments */
    --f;
    --x;

    /* Function Body */
    brack_(n, &x[1], x0, &ileft);
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    imin = max(i__1,i__2);
/* Computing MIN */
    i__1 = *n, i__2 = ileft + 2;
    imax = min(i__1,i__2);
    nuse = imax - imin + 1;
    fixhol_(&nuse, &x[imin], &f[imin], fbad, &nh, xh, fh);
/*     Interpolate fixed function */
    if (nh <= 1) {
	*f0 = fh[0];
	*fp0 = (float)0.;
    } else {
	quaint_(&nh, xh, fh, x0, f0, fp0, iext);
    }
/*     Now check if interpolation point is in a hole */
    if (*f0 == *fbad && *fp0 == (float)0.) {
	*ibad = 1;
    } else {
	*ibad = 0;
    }
    return 0;
} /* holint_ */

