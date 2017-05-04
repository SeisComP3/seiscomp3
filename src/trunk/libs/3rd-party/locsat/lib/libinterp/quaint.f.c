#include "utils.h"


/* NAME */
/* 	quaint -- Monotone, quadratic interpolation with linear derivatives. */
/* FILE */
/* 	quaint.f */
/* SYNOPSIS */
/* 	Constrain derivative to be linear during monotone, quadratic */
/* 	interpolation. */
/* DESCRIPTION */
/* 	Subroutine.  Perform monotone, quadratic interpolation of function */
/* 	f(x).  The interpolating function between two points is montone in */
/* 	value and linear in derivative. */
/* 	---- Indexing ---- */
/* 	i = 1, n; */
/* 	---- On entry ---- */
/* 	n:	Number of function samples */
/* 	x(i):	Sample values of independent variable; must be ordered: */
/* 		x(i) >= x(i-1) */
/* 	f(i):	Value of function at x(i) */
/* 	x0:	Value of independent variable for interpolation */
/* 	---- On return ---- */
/* 	f0:	Interpolated value of function at x0 */
/* 	fp0:	Interpolated value of derivative at x0 */
/* 	iext:	Flag indicating whether extrapolation has occurred; */
/* 		  =  0,	No extrapolation */
/* 		  = -1,	Yes, x0 < x(1) */
/* 		  = +1,	Yes, x0 > x(N) */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		- Calls brack and hermit */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */
/* 	- f(x) may be discontinuous.  A discontinuity is presumed to occur */
/* 	  when x(i) repeats for consecutive i. */
/* 	- If x0 is out of range (iext = -1 or +1), then f0 and fp0 are */
/* 	  defined through linear extrapolation of function. */
/* SEE ALSO */

/* AUTHOR */

/* Subroutine */ int quaint_(n, x, f, x0, f0, fp0, iext)
int *n;
float *x, *f, *x0, *f0, *fp0;
int *iext;
{
    /* System generated locals */
    int i__1, i__2;
    float r__1, r__2;

    /* Local variables */
    extern /* Subroutine */ int brack_();
    static int ileft;
    static float fpdev, f1, f2, f3;
    static int i1, i2, i3, i4;
    static float f4, x1, x2, x3, x4, fpdev2, fpdev3, h12, h23, h34, s12, s23,
	    s34;
    extern /* Subroutine */ int hermit_();
    static float fp2, fp3, fac;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Binary search for samples bounding x0 */
    /* Parameter adjustments */
    --f;
    --x;

    /* Function Body */
    brack_(n, &x[1], x0, &ileft);
/*     x0 < x(1) */
    if (ileft < 1) {
	if (x[2] > x[1]) {
	    *fp0 = (f[2] - f[1]) / (x[2] - x[1]);
	} else {
	    *fp0 = (float)0.;
	}
	*f0 = f[1] + *fp0 * (*x0 - x[1]);
	*iext = -1;
	return 0;
    }
/*     x0 > x(n) */
    if (ileft >= *n) {
	if (x[*n] > x[*n - 1]) {
	    *fp0 = (f[*n] - f[*n - 1]) / (x[*n] - x[*n - 1]);
	} else {
	    *fp0 = (float)0.;
	}
	*f0 = f[*n] + *fp0 * (*x0 - x[*n]);
	*iext = 1;
	return 0;
    }
/*     Normal case */
/*     Define points 1..4, such that x1 <= x2 <= x0 <= x3 <= x4 ---- */
/*     If necessary, make x1 = x2 or x3 = x4 */
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    i1 = max(i__1,i__2);
    i2 = ileft;
    i3 = ileft + 1;
/* Computing MIN */
    i__1 = *n, i__2 = ileft + 2;
    i4 = min(i__1,i__2);
    x1 = x[i1];
    x2 = x[i2];
    x3 = x[i3];
    x4 = x[i4];
    f1 = f[i1];
    f2 = f[i2];
    f3 = f[i3];
    f4 = f[i4];
/*     Find widths of three intervals */
/*     Note 'brack' guarantees x(ileft) < x(ileft+1), and thus h23 > 0 */
    h12 = x2 - x1;
    h23 = x3 - x2;
    h34 = x4 - x3;
/*     Set finite-difference derivative in center interval */
    s23 = (f3 - f2) / h23;
/*     Assign a function derivative to point 2; call it fp2.  The derivative */
/*     of the parabola fitting points 1, 2 and 3 c (evaluated at x2) is used, */
/*     howvever, if h12 is zero, s23 is used. */
    if (h12 > (float)0.) {
	s12 = (f2 - f1) / h12;
	fp2 = (s23 * h12 + s12 * h23) / (h12 + h23);
    } else {
	fp2 = s23;
    }
/*     Assign a function derivative to point 3; call it fp3.  The derivative */
/*     of the parabola fitting points 2, 3 and 4 (evaluated at x3) is used, */
/*     howvever, if h34 is zero, s23 is used. */
    if (h34 > (float)0.) {
	s34 = (f4 - f3) / h34;
	fp3 = (s23 * h34 + s34 * h23) / (h34 + h23);
    } else {
	fp3 = s23;
    }
/*     Adjust fp2 and fp3 such that they average to s23, but neither gets */
/*     farther from s23 */
    fpdev2 = s23 - fp2;
    fpdev3 = fp3 - s23;
    if (fpdev2 * fpdev3 <= (float)0.) {
	fpdev = (float)0.;
    } else if (fpdev2 < (float)0.) {
/* Computing MIN */
	r__1 = -fpdev2, r__2 = -fpdev3;
	fpdev = -dmin(r__1,r__2);
    } else {
	fpdev = dmin(fpdev2,fpdev3);
    }
/*     Adjust derivatives such that Hermite cubic interpolant is monotonic */
    if (s23 != (float)0.) {
	fac = (r__1 = fpdev / s23, dabs(r__1));
	if (fac > (float)1.) {
	    fpdev /= fac;
	}
    }
    fp2 = s23 - fpdev;
    fp3 = s23 + fpdev;
/*     Now do a straight Hermite cubic interpolation bewteen points 2 and 3 */
    hermit_(&x2, &x3, &f2, &f3, &fp2, &fp3, x0, f0, fp0);
    *iext = 0;
    return 0;
} /* quaint_ */

