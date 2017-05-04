#include "utils.h"


int brack_(int *, float *, float *, int *);
int holint_(int *, float *, float *, float *, float *, float *, float *, int *, int *);
int quaint_(int *n, float *, float *, float *, float *, float *, int *);


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
/*  If ibad = 1, f0 is set to fbad and the derivatrives to zero. */
/*  If x0 or y0 is out of range (iext != 0 or jext != 0) then, */
/*  f0, fx0, fy0 and fxy0 are defined through linear function */
/*  extrapolation. */
/* SEE ALSO */

/* AUTHOR */
/* 	Walter Nagy, September 1991. */
int holint2_(int *phase_id,
             int *do_extrapolate,
             int *nd, /* Number of distance samples */
             int *nz, /* Number of depth samples */
             float *x, float *y, float *func, int *ldf,
             float *fbad, float *x0, float *y0, float *f0,
             float *fx0, float *fy0, float *fxy0,
             int *idist, int *idepth, int *ihole) {
	/* System generated locals */
	int f_dim1, f_offset, tmp1, tmp2;

	/* Local variables */
	int ichk;
	float hold;
	int imin, jmin, imax, jmax, iext, jext, muse, nuse;
	float dist_min, dist_max;
	int i, j, k;
	int ileft, jleft, js;
	float tt_min, tt_max;
	float f0s[4];
	int extrap_in_hole;
	float vel;
	int min_idx, max_idx;
	float subgrid[724] /* was [181][4] */, fx0s[4];
	int num_samples, extrap_distance, ibad;

	/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
	/*     ---- On entry ---- */
	/*     ---- On return ---- */
	/*     ---- Internal variables ---- */
	/* Parameter adjustments */
	--x;
	--y;

	f_dim1 = *ldf;
	f_offset = 1 + f_dim1 * 1;
	func -= f_offset;

	/* Function Body */
	iext = 0;
	jext = 0;
	ibad = 0;
	*ihole = 0;
	*idist = 0;
	*idepth = 0;
	num_samples = *ldf;

	/*     Should we extrapolate ? */
	if ( (*x0 > x[*nd] || *x0 < x[1]) && *do_extrapolate != 1 ) {
		*f0 = (float)-1.;
		return 0;
	}

	/* Bracket x0 -- Find 4 relevant x() samples, or as many as needed */
	brack_(nd, &x[1], x0, &ileft);

	/* Computing MAX */
	tmp1 = 1, tmp2 = ileft - 1;
	imin = max(tmp1,tmp2);

	/* Computing MIN */
	tmp1 = *nd, tmp2 = ileft + 2;
	imax = min(tmp1,tmp2);
	muse = imax - imin + 1;

	/*     Do the same for y() */
	brack_(nz, &y[1], y0, &jleft);

	/* Computing MAX */
	tmp1 = 1, tmp2 = jleft - 1;
	jmin = max(tmp1,tmp2);

	/* Computing MIN */
	tmp1 = *nz, tmp2 = jleft + 2;
	jmax = min(tmp1,tmp2);
	nuse = jmax - jmin + 1;

	/* Fill in subgrid with valid times where available and fill-in empty */
	/* parts of the desired curve with linearly extrapolated values.  For */
	/* travel-time tables x(i) contains the distance elements, while y(j) */
	/* holds the depth samples. */
	for ( j = jmin; j <= jmax; ++j ) {
		for ( i = imin; i <= imax; ++i ) {
			if ( func[i + j * f_dim1] == (float)-1. ) {
				if ( *do_extrapolate != 1 ) {
					*f0 = (float)-1.;
					return 0;
				}

				extrap_in_hole = *ihole;
				extrap_distance = *idist;

				for ( min_idx = 1; min_idx <= num_samples; ++min_idx ) {
					if ( func[min_idx + j * f_dim1] != (float)-1. )
						break;
				}

				dist_min = x[min_idx];
				ichk = 0;

				for ( max_idx = min_idx; max_idx <= num_samples; ++max_idx ) {
					/*
					if ( max_idx + j * f_dim1 >= *nd )
						printf("Out of range: %d >= %d\n", max_idx + j * f_dim1, *nd);
					*/

					if ( func[max_idx + j * f_dim1] == (float)-1. ) {
						if ( ichk == 0 ) {
							ichk = max_idx - 1;
						}
					}
					else if ( max_idx == num_samples ) {
						ichk = max_idx;
					}
					else {
						ichk = 0;
					}
				}

				max_idx = ichk;
				dist_max = x[max_idx];

				/* Off the high end ? */
				if ( x[i] > dist_max ) {
					for ( k = max_idx; k >= 1; --k ) {
						if ( dist_max - x[k] >= (float)5. )
							break;
					}

					tt_max = func[max_idx + j * f_dim1];
					vel = (dist_max - x[k]) / (tt_max - func[k + j * f_dim1]);

					if ( dist_max <= (float)110. && x[i] > (float)110. ) {
						hold = ((float)110. - dist_max) / vel + tt_max + (float)238.;
						vel *= (float)2.4;
						subgrid[i + j * 181 - 182] = (x[i] - (float)110.) / vel + hold;
					}
					else
						subgrid[i + j * 181 - 182] = (x[i] - dist_max) / vel + tt_max;

					extrap_distance = 1;
					/* Off the low end ? */
				}
				else if ( x[i] < dist_min ) {
					for (k = min_idx; k <= num_samples; ++k) {
						if (x[k] - dist_min >= (float)5.)
							break;
					}

					tt_min = func[min_idx + j * f_dim1];
					vel = (x[k] - dist_min) / (func[k + j * f_dim1] - tt_min);
					subgrid[i + j * 181 - 182] = tt_min - (dist_min - x[i]) / vel;
					extrap_distance = -1;
					/* In a hole ? */
				}
				else {
					for (k = max_idx; k >= 1; --k) {
						if (x[k] < x[i]) {
							if (func[k + j * f_dim1] != (float)-1.) {
								dist_max = x[k];
								max_idx = k;
								break;
							}
						}
					}

					for (k = max_idx; k >= 1; --k) {
						if (dist_max - x[k] >= (float)5.)
							break;
					}

					tt_max = func[max_idx + j * f_dim1];
					vel = (dist_max - x[k]) / (tt_max - func[k + j * f_dim1]);
					subgrid[i + j * 181 - 182] = (x[i] - dist_max) / vel + tt_max;
					extrap_in_hole = 1;
				}

				*ihole = extrap_in_hole;
				*idist = extrap_distance;
			}
			else
				subgrid[i + j * 181 - 182] = func[i + j * f_dim1];
		}
	}

	if ( *y0 > y[jmax] )
		*idepth = 1;

	/* Now interpolate to (x0, y(j)), j = jmin, jmax) */
	for ( j = jmin; j <= jmax; ++j ) {
		js = j - jmin + 1;
		holint_(&muse, &x[imin], &subgrid[imin + j * 181 - 182], fbad, x0, & f0s[js - 1], &fx0s[js - 1], &iext, &ibad);
	}

	/* Now interpolate to (x0,y0) */
	holint_(&nuse, &y[jmin], f0s, fbad, y0, f0, fy0, &jext, &ibad);

	/* if (ibad.gt.0) then */
	/*   fx0  = 0.0 */
	/*   fxy0 = 0.0 */
	/* else */
	quaint_(&nuse, &y[jmin], fx0s, y0, fx0, fxy0, &jext);

	/* end if */
	iext = *idist;
	jext = *idepth;
	ibad = *ihole;

	return 0;
}

