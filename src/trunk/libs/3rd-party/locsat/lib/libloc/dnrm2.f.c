/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdnrm2_1_ {
    char sccsid[80];
};

#define sccsdnrm2_1 (*(struct sccsdnrm2_1_ *) &sccsdnrm2_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdnrm2_ = { {'@', '(', '#', ')', 'd', 'n', 'r', 'm', '2', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	dnrm2 -- Compute the Euclidean norm of a vector. */
/* FILE */
/* 	dnrm2.f */
/* SYNOPSIS */
/* 	LINPACK routine to calculate the L2 (Euclidean) norm of a vector. */
/* DESCRIPTION */
/* 	Function.  Given a vector dx() of length n, compute the Euclidean */
/* 	norm and return as a scalar in dnrm2. */
/* 	---- On entry ---- */
/* 	n:	Length of vector;  If n <= 0, return n = 0; else n = 1 */
/* 	incx:	x-storage increment counter (= 1, if entire loop accessed) */
/* 	dx():	Vector of interest */
/* 	---- On return ---- */
/* 	dnrm2:	Scalar Eucildean norm of dx() */
/* DIAGNOSTICS */
/* 	Bases norm solution on precision of machine constants. */
/* NOTES */
/* 	Uses unrolled loops for increments equal to 1. */
/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	C.L. Lawson, Jan. 1978. */
doublereal dnrm2_(n, dx, incx)
integer *n;
doublereal *dx;
integer *incx;
{
    /* Initialized data */

    static doublereal zero = 0.;
    static doublereal one = 1.;
    static doublereal cutlo = 8.232e-11;
    static doublereal cuthi = 1.304e19;

    /* Format strings */
    static char fmt_1020[] = "";
    static char fmt_1030[] = "";
    static char fmt_1060[] = "";
    static char fmt_1070[] = "";

    /* System generated locals */
    integer i__1, i__2;
    doublereal ret_val, d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static doublereal xmax;
    static integer next, i__, j, nn;
    static doublereal hitest, sum;

    /* Assigned format variables */
    static char *next_fmt;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     real*8 function dnrm2 */
/*     ---- Internal variables ---- */
/* 	Four phase method     Using two built-in constants that are */
/* 	hopefully applicable to all machines. */
/* 	  cutlo = Maximum of  dsqrt(u/eps)	over all known machines */
/* 	  cuthi = Minimum of  dsqrt(v)  	over all known machines */
/* 	where, */
/* 	  eps = Smallest no. such that eps + 1. .gt. 1. */
/* 	  u   = Smallest positive no.	(underflow limit) */
/* 	  v   = Largest  no.    	(overflow  limit) */
/* 	Brief outline of algorithm: */
/* 	Phase 1	Scans zero components */
/* 	Move to phase 2 when a component is nonzero and .le. cutlo */
/* 	Move to phase 3 when a component is .gt. cutlo */
/* 	Move to phase 4 when a component is .ge. cuthi/m */
/* 	  where, m = n for x() real and m = 2*n for complex */
/* 	Values for cutlo and cuthi: */
/* 	From the environmental parameters listed in the IMSL converter */
/* 	document the limiting values are as follows: */
/* 	cutlo, S.P.   u/eps = 2**(-102) for  HONEYWELL.  Close seconds */
/* 	are: */
/* 		UNIVAC and DEC at 2**(-103) */
/* 		thus cutlo = 2**(-51) = 4.44089e-16 */
/* 	cuthi, S.P.   v = 2**127 for UNIVAC, HONEYWELL, and DEC */
/* 		thus cuthi = 2**(63.5) = 1.30438e19 */
/* 	cutlo, D.P.   u/eps = 2**(-67) for HONEYWELL and DEC */
/* 		thus cutlo = 2**(-33.5) = 8.23181d-11 */
/* 	cuthi, D.P.   same as S.P.  cuthi = 1.30438d19 */
/* 	data cutlo, cuthi / 8.232d-11,  1.304d19 / */
/* 	data cutlo, cuthi / 4.441e-16,  1.304e19 / */
/*     For SUN system computers */
    /* Parameter adjustments */
    --dx;

    /* Function Body */
    if (*n > 0) {
	goto L1000;
    }
    ret_val = zero;
    goto L1130;
L1000:
    next = 0;
    next_fmt = fmt_1020;
    sum = zero;
    nn = *n * *incx;
/*     Begin main loop */
    i__ = 1;
L1010:
    switch ((int)next) {
	case 0: goto L1020;
	case 1: goto L1030;
	case 2: goto L1060;
	case 3: goto L1070;
    }
L1020:
    if ((d__1 = dx[i__], abs(d__1)) > cutlo) {
	goto L1100;
    }
    next = 1;
    next_fmt = fmt_1030;
    xmax = zero;
/*     Phase 1.  sum is zero */
L1030:
    if (dx[i__] == zero) {
	goto L1120;
    }
    if ((d__1 = dx[i__], abs(d__1)) > cutlo) {
	goto L1100;
    }
/*     Prepare for phase 2 */
    next = 2;
    next_fmt = fmt_1060;
    goto L1050;
/*     Prepare for phase 4 */
L1040:
    i__ = j;
    next = 3;
    next_fmt = fmt_1070;
    sum = sum / dx[i__] / dx[i__];
L1050:
    xmax = (d__1 = dx[i__], abs(d__1));
    goto L1080;
/*     Phase 2.  Sum is small */
/*        Scale to avoid destructive underflow */
L1060:
    if ((d__1 = dx[i__], abs(d__1)) > cutlo) {
	goto L1090;
    }
/*     Common code for phases 2 and 4 */
/*     In phase 4, sum is large, so scale to avoid overflow */
L1070:
    if ((d__1 = dx[i__], abs(d__1)) <= xmax) {
	goto L1080;
    }
/* Computing 2nd power */
    d__1 = xmax / dx[i__];
    sum = one + sum * (d__1 * d__1);
    xmax = (d__1 = dx[i__], abs(d__1));
    goto L1120;
L1080:
/* Computing 2nd power */
    d__1 = dx[i__] / xmax;
    sum += d__1 * d__1;
    goto L1120;
/*     Prepare for phase 3 */
L1090:
    sum = sum * xmax * xmax;
/*     For real or D.P. set hitest = cuthi/n */
/*     For complex      set hitest = cuthi/(2*n) */
L1100:
    hitest = cuthi / (real) (*n);
/*     Phase 3:  sum is mid-range, no scaling */
    i__1 = nn;
    i__2 = *incx;
    for (j = i__; i__2 < 0 ? j >= i__1 : j <= i__1; j += i__2) {
	if ((d__1 = dx[j], abs(d__1)) >= hitest) {
	    goto L1040;
	}
/* L1110: */
/* Computing 2nd power */
	d__1 = dx[j];
	sum += d__1 * d__1;
    }
    ret_val = sqrt(sum);
    goto L1130;
L1120:
    i__ += *incx;
    if (i__ <= nn) {
	goto L1010;
    }
/*     Compute square root and adjust for scaling */
    ret_val = xmax * sqrt(sum);
L1130:
    return ret_val;
} /* dnrm2_ */

