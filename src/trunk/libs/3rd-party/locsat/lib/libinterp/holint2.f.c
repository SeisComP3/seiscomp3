/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsholint2_1_ {
    char sccsid[80];
};

#define sccsholint2_1 (*(struct sccsholint2_1_ *) &sccsholint2_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsholint2_ = { {'@', '(', '#', ')', 'h', 'o', 'l', 'i', 'n', 't', '2',
	     '.', 'f', '\t', '4', '4', '.', '2', '\t', '1', '0', '/', '3', 
	    '1', '/', '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' '} };


/* NAME */
/* 	holint2 -- Hybrid monotone, quadratic inter/extrapolation routine. */
/* FILE */
/* 	holint2.f */
/* SYNOPSIS */
/* 	Perfrom a monotone, quadratic interpolation on a function which */
/* 	may have holes. */
/* DESCRIPTION */
/* 	Subroutine.  Monotone, quadratic interpolation of function f(x,y) */
/* 	which might have holes (bad values).  Bad function samples are */
/* 	linearly extrapolated as necessary (see below). */
/* 	---- Indexing ---- */
/* 	i = 1, m;	j = 1, n; */
/* 	---- On entry ---- */
/* 	phase_id:	Phase type ID */
/* 	do_extrap:	Do we want to extrapolate data (0 = TRUE; 1 = FALSE) */
/* 	m:		Number of x() samples */
/* 	n:		Number of y() samples */
/* 	x(i):		Sample values of x() */
/* 	y(j):		Sample values of y() */
/* 	f(i,j):		Value of function at (x(i), y(j)) */
/* 	ldf:		Leading dimension of array f() */
/* 	fbad:		Function value denoting bad sample */
/* 	x0:		Value of x() for interpolation */
/* 	y0:		Value of y() for interpolation */
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
/* 	Walter Nagy, September 1991. */
/* Subroutine */ int holint2_(phase_id__, do_extrap__, m, n, x, y, f, ldf, 
	fbad, x0, y0, f0, fx0, fy0, fxy0, idist, idepth, ihole)
integer *phase_id__, *do_extrap__, *m, *n;
real *x, *y, *f;
integer *ldf;
real *fbad, *x0, *y0, *f0, *fx0, *fy0, *fxy0;
integer *idist, *idepth, *ihole;
{
    /* System generated locals */
    integer f_dim1, f_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer ichk;
    static real hold;
    static integer imin, jmin, imax, jmax, iext, jext, muse, nuse;
    static real dist_min__, dist_max__;
    static integer i__, j, k;
    extern /* Subroutine */ int brack_();
    static integer ileft, jleft, js;
    static real tt_min__, tt_max__;
    extern /* Subroutine */ int holint_(), quaint_();
    static real f0s[4];
    static integer extrap_in_hole__;
    static real vel;
    static integer min_idx__, max_idx__;
    static real subgrid[724]	/* was [181][4] */, fx0s[4];
    static integer num_samples__, extrap_distance__, ibad;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --x;
    --y;
    f_dim1 = *ldf;
    f_offset = 1 + f_dim1 * 1;
    f -= f_offset;

    /* Function Body */
    iext = 0;
    jext = 0;
    ibad = 0;
    *ihole = 0;
    *idist = 0;
    *idepth = 0;
    num_samples__ = *ldf;
/*     Should we extrapolate ? */
    if ((*x0 > x[*m] || *x0 < x[1]) && *do_extrap__ != 1) {
	*f0 = (float)-1.;
	return 0;
    }
/*     Bracket x0 -- Find 4 relevant x() samples, or as many as needed */
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
/*     Fill in subgrid with valid times where available and fill-in empty */
/*     parts of the desired curve with linearly extrapolated values.  For */
/*     travel-time tables x(i) contains the distance elements, while y(j) */
/*     holds the depth samples. */
    i__1 = jmax;
    for (j = jmin; j <= i__1; ++j) {
	i__2 = imax;
	for (i__ = imin; i__ <= i__2; ++i__) {
	    if (f[i__ + j * f_dim1] == (float)-1.) {
		if (*do_extrap__ != 1) {
		    *f0 = (float)-1.;
		    return 0;
		}
		extrap_in_hole__ = *ihole;
		extrap_distance__ = *idist;
		i__3 = num_samples__;
		for (min_idx__ = 1; min_idx__ <= i__3; ++min_idx__) {
/* L1000: */
		    if (f[min_idx__ + j * f_dim1] != (float)-1.) {
			goto L1010;
		    }
		}
L1010:
		dist_min__ = x[min_idx__];
		ichk = 0;
		i__3 = num_samples__;
		for (max_idx__ = min_idx__; max_idx__ <= i__3; ++max_idx__) {
		    if (f[max_idx__ + j * f_dim1] == (float)-1.) {
			if (ichk == 0) {
			    ichk = max_idx__ - 1;
			}
		    } else if (max_idx__ == num_samples__) {
			ichk = max_idx__;
		    } else {
			ichk = 0;
		    }
/* L1020: */
		}
		max_idx__ = ichk;
		dist_max__ = x[max_idx__];
/*              Off the high end ? */
		if (x[i__] > dist_max__) {
		    for (k = max_idx__; k >= 1; --k) {
/* L1030: */
			if (dist_max__ - x[k] >= (float)5.) {
			    goto L1040;
			}
		    }
L1040:
		    tt_max__ = f[max_idx__ + j * f_dim1];
		    vel = (dist_max__ - x[k]) / (tt_max__ - f[k + j * f_dim1])
			    ;
		    if (dist_max__ <= (float)110. && x[i__] > (float)110.) {
			hold = ((float)110. - dist_max__) / vel + tt_max__ + (
				float)238.;
			vel *= (float)2.4;
			subgrid[i__ + j * 181 - 182] = (x[i__] - (float)110.) 
				/ vel + hold;
		    } else {
			subgrid[i__ + j * 181 - 182] = (x[i__] - dist_max__) /
				 vel + tt_max__;
		    }
		    extrap_distance__ = 1;
/*              Off the low end ? */
		} else if (x[i__] < dist_min__) {
		    i__3 = num_samples__;
		    for (k = min_idx__; k <= i__3; ++k) {
/* L1050: */
			if (x[k] - dist_min__ >= (float)5.) {
			    goto L1060;
			}
		    }
L1060:
		    tt_min__ = f[min_idx__ + j * f_dim1];
		    vel = (x[k] - dist_min__) / (f[k + j * f_dim1] - tt_min__)
			    ;
		    subgrid[i__ + j * 181 - 182] = tt_min__ - (dist_min__ - x[
			    i__]) / vel;
		    extrap_distance__ = -1;
/*              In a hole ? */
		} else {
		    for (k = max_idx__; k >= 1; --k) {
			if (x[k] < x[i__]) {
			    if (f[k + j * f_dim1] != (float)-1.) {
				dist_max__ = x[k];
				max_idx__ = k;
				goto L1080;
			    }
			}
/* L1070: */
		    }
L1080:
		    for (k = max_idx__; k >= 1; --k) {
/* L1090: */
			if (dist_max__ - x[k] >= (float)5.) {
			    goto L1100;
			}
		    }
L1100:
		    tt_max__ = f[max_idx__ + j * f_dim1];
		    vel = (dist_max__ - x[k]) / (tt_max__ - f[k + j * f_dim1])
			    ;
		    subgrid[i__ + j * 181 - 182] = (x[i__] - dist_max__) / 
			    vel + tt_max__;
		    extrap_in_hole__ = 1;
		}
		*ihole = extrap_in_hole__;
		*idist = extrap_distance__;
	    } else {
		subgrid[i__ + j * 181 - 182] = f[i__ + j * f_dim1];
	    }
/* L1110: */
	}
/* L1120: */
    }
    if (*y0 > y[jmax]) {
	*idepth = 1;
    }
/*     Now interpolate to (x0, y(j)), j = jmin, jmax) */
    i__1 = jmax;
    for (j = jmin; j <= i__1; ++j) {
	js = j - jmin + 1;
	holint_(&muse, &x[imin], &subgrid[imin + j * 181 - 182], fbad, x0, &
		f0s[js - 1], &fx0s[js - 1], &iext, &ibad);
/* L1130: */
    }
/*     Now interpolate to (x0,y0) */
    holint_(&nuse, &y[jmin], f0s, fbad, y0, f0, fy0, &jext, &ibad);
/*     if (ibad.gt.0) then */
/*        fx0  = 0.0 */
/*        fxy0 = 0.0 */
/*     else */
    quaint_(&nuse, &y[jmin], fx0s, y0, fx0, fxy0, &jext);
/*     end if */
    iext = *idist;
    jext = *idepth;
    ibad = *ihole;
    return 0;
} /* holint2_ */

