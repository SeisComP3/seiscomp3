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

int brack_(int *n, float *x, float *x0, int *ileft) {
	int i1;

	/* Local variables */
	int imid, i, iright;

	/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
	/*     ---- On entry ---- */
	/*     ---- On return ---- */
	/*     ---- Internal variables ---- */
	/*     Initialize */
	/* Parameter adjustments */
	--x;

	*ileft = 0;
	iright = *n + 1;

	while ( 1 ) {
		imid = (*ileft + iright) / 2;
		if (imid == *ileft) {
			return 0;
		}
		else if (*x0 < x[imid]) {
			iright = imid;
		}
		else if (*x0 > x[imid]) {
			*ileft = imid;
		}
		else
			break;
	}

	/*     Special case: The point x(imid) found to equal x0.  Find bracket */
	/*     [x(ileft),x(ileft+1)], such that x(ileft+1) > x(ileft). */
	i1 = *n;
	for (i = imid + 1; i <= i1; ++i) {
		if (x[i] > *x0) {
			*ileft = i - 1;
			return 0;
		}
	}

	for (i = imid - 1; i >= 1; --i) {
		if (x[i] < *x0) {
			*ileft = i;
			return 0;
		}
	}

	*ileft = 0;
	return 0;
}
