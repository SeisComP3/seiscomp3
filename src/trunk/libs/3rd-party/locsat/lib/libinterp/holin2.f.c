#include "utils.h"


int brack_(int *, float *, float *, int *);
int holint_(int *, float *, float *, float *, float *, float *, float *, int *, int *);
int quaint_(int *n, float *, float *, float *, float *, float *, int *);


/* NAME */
/* 	holin2 -- Monotone, quadratic interpolation routine. */
/* FILE */
/* 	holin2.f */
/* SYNOPSIS */
/* 	Perfrom a monotone, quadratic interpolation on a function which */
/* 	may have holes. */
/* DESCRIPTION */
/* 	Subroutine.  Monotone, quadratic interpolation of function f(x,y) */
/* 	which might have holes (bad values).  Bad function samples are */
/* 	given the value, fbad (see below). */
/* 	---- Indexing ---- */
/* 	i = 1, m;	j = 1, n; */
/* 	---- On entry ---- */
/* 	m:	Number of x() samples */
/* 	n:	Number of y() samples */
/* 	x(i):	Sample values of x() */
/* 	y(j):	Sample values of y() */
/* 	f(i,j):	Value of function at (x(i), y(j)) */
/* 	ldf:	Leading dimension of array f() */
/* 	fbad:	Function value denoting bad sample */
/* 	x0:	Value of x() for interpolation */
/* 	y0:	Value of y() for interpolation */
/* 	---- On return ---- */
/* 	f0:	Interpolated value of function at (x0,y0) */
/* 	fx0:	Interpolated value of x-derivative of function at (x0,y0) */
/* 	fy0:	Interpolated value of y-derivative of function at (x0,y0) */
/* 	fxy0:	Interpolated value of x-y-derivative of function at (x0,y0) */
/* 	iext:	Error flag;  0, No error;  -1, x0 < x(1);  1, x0 > x(m) */
/* 	jext:	Error flag;  0, No error;  -1, y0 < Y(1);  1, y0 > y(n) */
/* 	ibad:	Flag indicating whether interopolation point is in a hole; */
/* 		0, No;	1, Yes */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		- Calls brack, holint and quaint directly */
/* 		- Calls brack, fixhol and hermit indirectly */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */
/* 	- If ibad = 1, f0 is set to fbad and the derivatrives to zero. */
/* 	- If x0 or y0 is out of range (iext != 0 or jext != 0) then, */
/* 	  f0, fx0, fy0 and fxy0 are defined through linear function */
/* 	  extrapolation. */
/* SEE ALSO */

/* AUTHOR */

int holin2_(int *m, int *n,
            float *x, float *y, float *f,
            int *ldf,
            float *fbad, float *x0, float *y0, float *f0,
            float *fx0, float *fy0, float *fxy0,
            int *iext, int *jext, int *ibad)
{
	/* System generated locals */
	int f_dim1, f_offset, i__1, i__2;

	/* Local variables */
	int imin, jmin, imax, jmax, muse, nuse, j;

	int ileft, jleft, js;
	float f0s[4], fx0s[4];

	/*     K.S. 1-Dec-97, changed 'undefined' to 'none' */
	/*     ---- On entry ---- */
	/*     ---- On return ---- */
	/*     ---- Internal variables ---- */
	/*     Bracket x0 -- Find 4 relevant x() samples, or as many as needed */
	/* Parameter adjustments */
	--x;
	--y;
	f_dim1 = *ldf;
	f_offset = 1 + f_dim1 * 1;
	f -= f_offset;

	/* Function Body */
	brack_(m, &x[1], x0, &ileft);
	/* Computing MAX */
	i__1 = 1, i__2 = ileft - 1;
	imin = max(i__1,i__2);
	/* Computing MIN */
	i__1 = *m, i__2 = ileft + 2;
	imax = min(i__1,i__2);
	muse = imax - imin + 1;
	/*     Do the same for y() */
	brack_(n, &y[1], y0, &jleft);
	/* Computing MAX */
	i__1 = 1, i__2 = jleft - 1;
	jmin = max(i__1,i__2);
	/* Computing MIN */
	i__1 = *n, i__2 = jleft + 2;
	jmax = min(i__1,i__2);
	nuse = jmax - jmin + 1;
	/*     Now interpolate to (x0, y(j)), j = jmin, jmax) */
	i__1 = jmax;
	for (j = jmin; j <= i__1; ++j) {
		js = j - jmin + 1;
		holint_(&muse, &x[imin], &f[imin + j * f_dim1], fbad, x0, &f0s[js - 1], &fx0s[js - 1], iext, ibad);
	}

	/*     Now interpolate to (x0,y0) */
	holint_(&nuse, &y[jmin], f0s, fbad, y0, f0, fy0, jext, ibad);

	if (*ibad > 0) {
		*fx0 = (float)0.;
		*fxy0 = (float)0.;
	}
	else {
		quaint_(&nuse, &y[jmin], fx0s, y0, fx0, fxy0, jext);
	}

	/*     Find minimum interpolated x-derivative */
	return 0;
}
