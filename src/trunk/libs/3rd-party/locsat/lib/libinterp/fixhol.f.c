#include "utils.h"


/* NAME */
/* 	fixhol -- Deal with bad values (holes) in function. */
/* FILE */
/* 	fixhol.f */
/* SYNOPSIS */
/* 	When "holes" in a given function are found, look for "good" samples. */
/* DESCRIPTION */
/* 	Subroutine.  Fix up the sampled function f(x) to allow for "holes" */
/* 	with bad values.  A bad function value at a sample point x(i) is */
/* 	assumed to occur when the function sample f(i) has the value fbad. */
/* 	For interpolation purposes, the function is assumed then to be fbad */
/* 	in the intervals surrounding x(i) up to the next "good" samples, */
/* 	where a jump discontinuity is assumed to occur. */
/* 	Given the original function samples -- x(i), f(i), i = 1, m -- this */
/* 	routine creates a new set of samples -- xs(i), fs(j), j = 1, ms -- */
/* 	in which the intervals of bad values and discontinuities between */
/* 	good and bad values are explicitly sampled.  To create a */
/* 	discontinuity at a given point, the point is included as two */
/* 	samples with different function values. */
/* 	---- Example ---- */
/* 	x =  0.0  2.0   3.0  7.0  8.5  12.0  14.5  18.0  19.0  20.5 21.5  22.0 */
/* 	f =  2.3  1.1  fbad  7.6  4.5  fbad  fbad  12.1  fbad   6.2  4.3  fbad */
/* 	xs =  0.0  2.0   2.0  7.0  7.0   8.5   8.5  20.5 20.5  21.5  21.5 */
/* 	fs =  2.3  1.1  fbad fbad  7.6   4.5  fbad  fbad  6.2   4.3  fbad */
/* 	---- Indexing ---- */
/* 	i = 1, m;	j = 1, ms; */
/* 	---- On entry ---- */
/* 	m:	Number of x() samples */
/* 	x(i):	Sample values of x(); must be ordered */
/* 	f(i):	Value of function at x(i) */
/* 	fbad:	Function value signifying that f(x) is not well-defined (bad) */
/* 	---- On return ---- */
/* 	ms:	Number of new x() samples; May be as large as 1 + (4*m)/3 */
/* 	xs(j):	New sample values of x() */
/* 	fs(j):	New value of function at x(j) */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */

int fixhol_(int *m, float *x, float *f, float *fbad, int *ms, float *xs, float *fs) {
	/* System generated locals */
	int i__1, i__2, i__3;

	/* Local variables */
	int i__;

	/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
	/*     ---- On entry ---- */
	/*     ---- On return ---- */
	/*     ---- Internal variables ---- */
	/*     Trivial case */
	/* Parameter adjustments */
	--f;
	--x;
	--xs;
	--fs;

	/* Function Body */
	if (*m <= 0) {
		*ms = 0;
		return 0;
	}

	/*     Set up first point */
	*ms = 1;
	xs[1] = x[1];
	fs[1] = f[1];
	/*     Do the rest */
	i__1 = *m;

	for (i__ = 2; i__ <= i__1; ++i__) {
		if (f[i__] != *fbad) {
			if (fs[*ms] != *fbad) {
				if (x[i__] == xs[*ms]) {
					if (f[i__] == fs[*ms]) {
						goto L1010;
					}
				}
			}
			else {
				if (*ms > 1) {
					++(*ms);
				}

				xs[*ms] = x[i__];
				fs[*ms] = *fbad;
			}

			++(*ms);
			xs[*ms] = x[i__];
			fs[*ms] = f[i__];
		}
		else {
			if (fs[*ms] != *fbad) {
				if (*ms > 1) {
					if (fs[*ms - 1] == *fbad) {
						/* Computing MAX */
						i__2 = 1, i__3 = *ms - 2;
						*ms = max(i__2,i__3);
						goto L1010;
					}
				}

				++(*ms);
				xs[*ms] = xs[*ms - 1];
				fs[*ms] = *fbad;
			}
		}

L1000:
		if (*ms > 2) {
			if (xs[*ms] == xs[*ms - 2]) {
				fs[*ms - 1] = fs[*ms];
				--(*ms);
				goto L1000;
			}
		}

L1010:
		;
	}

	return 0;
}

