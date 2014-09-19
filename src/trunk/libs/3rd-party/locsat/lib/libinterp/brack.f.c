/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsbrack_1_ {
    char sccsid[80];
};

#define sccsbrack_1 (*(struct sccsbrack_1_ *) &sccsbrack_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsbrack_ = { {'@', '(', '#', ')', 'b', 'r', 'a', 'c', 'k', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	brack -- Bracket an array of interpolative values. */
/* FILE */
/* 	brack.f */
/* SYNOPSIS */
/* 	Using bi-section, brack an array of interpolative values by */
/* 	performing a binary search. */
/* DESCRIPTION */
/* 	Subroutine.  Perform a binary search to find those elements of */
/* 	array x() that bracket x0.  Given the array x(i), i = 1,.,N, in */
/* 	non-decreasing order, and given the number x0, this routine finds */
/* 	ileft from 0..n, such that (pretend x(0) = -infinity, */
/* 	x(n+1) = +infinity): */
/* 		x(ileft) <= x0 <= x(ileft+1) */
/* 		x(ileft) < x(ileft+1) */
/* 	Note that x() may contain duplicate values, but ileft will still */
/* 	point to a non-zero interval. */
/* 	---- On entry ---- */
/* 	n:	Dimension of input vector (array), x() */
/* 	x(n):	One-dimensional input array of values to be bracketed */
/* 	x0:	Value being compared against */
/* 	---- On return ---- */
/* 	ileft:	Left bracketed indice */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */

/* Subroutine */ int brack_(n, x, x0, ileft)
integer *n;
real *x, *x0;
integer *ileft;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer imid, i__, iright;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Initialize */
    /* Parameter adjustments */
    --x;

    /* Function Body */
    *ileft = 0;
    iright = *n + 1;
L1000:
    imid = (*ileft + iright) / 2;
    if (imid == *ileft) {
	return 0;
    } else if (*x0 < x[imid]) {
	iright = imid;
	goto L1000;
    } else if (*x0 > x[imid]) {
	*ileft = imid;
	goto L1000;
    }
/*     Special case: The point x(imid) found to equal x0.  Find bracket */
/*     [x(ileft),x(ileft+1)], such that x(ileft+1) > x(ileft). */
    i__1 = *n;
    for (i__ = imid + 1; i__ <= i__1; ++i__) {
	if (x[i__] > *x0) {
	    *ileft = i__ - 1;
	    return 0;
	}
/* L1010: */
    }
    for (i__ = imid - 1; i__ >= 1; --i__) {
	if (x[i__] < *x0) {
	    *ileft = i__;
	    return 0;
	}
/* L1020: */
    }
    *ileft = 0;
    return 0;
} /* brack_ */

