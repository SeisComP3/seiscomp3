/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsslocal0_1_ {
    char sccsid[80];
};

#define sccsslocal0_1 (*(struct sccsslocal0_1_ *) &sccsslocal0_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsslocal0_ = { {'@', '(', '#', ')', 's', 'l', 'o', 'c', 'a', 'l', '0',
	     '.', 'f', '\t', '4', '4', '.', '2', '\t', '1', '0', '/', '3', 
	    '1', '/', '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' '} };


/* Table of constant values */

static real c_b6 = (float)-1.;
static integer c__4 = 4;

/* NAME */
/* 	slocal0 -- Compute horizontal slownesses and partial derivatives. */
/* FILE */
/* 	slocal0.f */
/* SYNOPSIS */
/* 	Compute horizontal slownesses and their partial derivatives for */
/* 	a fixed hypocenter. */
/* DESCRIPTION */
/* 	Subroutine.  Slownesses and their partials are determined via */
/* 	inter/extrapolation of pre-calculated curves.  A point in a hole */
/* 	is rejected. */
/*       ---- On entry ---- */
/* 	radius:		Radius of Earth (km) */
/* 	delta:		Distance from the event to the station (deg) */
/* 	azi:		Forward-azimuth from event to station (deg) */
/* 	zfoc:		Event focal depth (km below sea level) */
/* 	maxtbd:		Maximum dimension of i'th position in tbd(), tbtt() */
/* 	maxtbz:		Maximum dimension of j'th position in tbz(), tbtt() */
/* 	ntbd:		Number of distance samples in tables */
/* 	ntbz:		Number of depth samples in tables */
/* 	tbd(i):		Distance samples for tables (deg) */
/* 	tbz(j):		Depth samples in tables (km) */
/* 	tbtt(i,j):	Travel-time tables (sec) */
/* 	---- On return ---- */
/* 	dcalx:	Calculated slownesses (sec/deg) */
/* 	atx(4):	Partial derivatives (sec/km/deg) */
/* 	iterr:	Error code for n'th observation */
/* 		=  0, No problem, normal interpolation */
/* 		= 11, Distance-depth point (x0,z0) in hole of T-T curve */
/* 		= 12, x0 < x(1) */
/* 		= 13, x0 > x(max) */
/* 		= 14, z0 < z(1) */
/* 		= 15, z0 > z(max) */
/* 		= 16, x0 < x(1) and z0 < z(1) */
/* 		= 17, x0 > x(max) and z0 < z(1) */
/* 		= 18, x0 < x(1) and z0 > z(max) */
/* 		= 19, x0 > x(max) and z0 > z(max) */
/* 	[NOTE:	If any of these codes are negative (e.g., iderr = -17), */
/* 		then, the datum was used to compute the event location] */
/* 	---- Subroutines called ---- */
/* 	From libinterp */
/* 		brack:		Bracket travel-time data via bisection */
/* 		holint2:	Quadratic interpolation function */
/* DIAGNOSTICS */
/* 	Currently assumes a constant radius Earth (i.e., it ignores */
/* 	ellipicity of the Earth. */
/* NOTES */
/* 	Future plans include dealing with ellipicity and higher-order */
/* 	derivatives. */
/* SEE ALSO */
/* 	Subroutines azcal0, and ttcal0 are parallel routines for computing */
/* 	azimuthal and travl-time partial derivatives, respectively. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
/* Subroutine */ int slocal0_(phase_id__, zfoc, radius, delta, azi, maxtbd, 
	maxtbz, ntbd, ntbz, tbd, tbz, tbtt, dcalx, atx, iterr)
integer *phase_id__;
real *zfoc, *radius, *delta, *azi;
integer *maxtbd, *maxtbz, *ntbd, *ntbz;
real *tbd, *tbz, *tbtt, *dcalx;
doublereal *atx;
integer *iterr;
{
    /* System generated locals */
    integer tbtt_dim1, tbtt_offset, i__1, i__2, i__3, i__4;

    /* Builtin functions */
    double sin(), cos();

    /* Local variables */
    static integer imin, jmin, imax, jmax;
    static real tbds[4], dtdz;
    static doublereal azir;
    static integer iext, jext;
    static real tbzs[4], slow;
    static integer i__, j;
    extern /* Subroutine */ int brack_();
    static integer ihole, ileft, jleft, idist;
    static real dsldz, ttime;
    static integer itotd;
    static real tbsls[16]	/* was [4][4] */;
    static integer do_extrap__, itotz, jz, nz, idepth;
    static doublereal cosazi;
    static real dcross;
    static doublereal sinazi;
    extern /* Subroutine */ int holint2_();
    static real dslddel;
    static integer ibad;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     Convert radians to degrees */
/*     Max. number of permissable parameters */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    --tbd;
    tbtt_dim1 = *maxtbd;
    tbtt_offset = 1 + tbtt_dim1 * 1;
    tbtt -= tbtt_offset;
    --tbz;
    --atx;

    /* Function Body */
    ihole = 0;
    idist = 0;
    idepth = 0;
/*     Permit extrapolation */
    do_extrap__ = 0;
/*     Form arrays holding distances and depths around point of interest */
    brack_(ntbd, &tbd[1], delta, &ileft);
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    imin = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbd, i__2 = ileft + 2;
    imax = min(i__1,i__2);
    itotd = 0;
    i__1 = imax;
    for (i__ = imin; i__ <= i__1; ++i__) {
	++itotd;
	tbds[itotd - 1] = tbd[i__];
/* L1000: */
    }
    brack_(ntbz, &tbz[1], zfoc, &jleft);
/* Computing MAX */
    i__1 = 1, i__2 = jleft - 1;
    jmin = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbz, i__2 = jleft + 2;
    jmax = min(i__1,i__2);
    itotz = 0;
    i__1 = jmax;
    for (j = jmin; j <= i__1; ++j) {
	++itotz;
	tbzs[itotz - 1] = tbz[j];
/* L1010: */
    }
/*     Compute travel time and horizontal slownesses for each point in */
/*     arrays around the point of interest */
/*     Find relevant range of table depths */
    i__1 = itotd;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = itotz;
	for (j = 1; j <= i__2; ++j) {
	    brack_(ntbz, &tbz[1], zfoc, &ileft);
/* Computing MAX */
	    i__3 = 1, i__4 = ileft - 1;
	    jz = max(i__3,i__4);
/* Computing MIN */
	    i__3 = *ntbz, i__4 = ileft + 2;
	    nz = min(i__3,i__4) - jz + 1;
/*           Return travel time and create a mini-table of partials */
	    i__3 = *phase_id__ - 1;
	    holint2_(&i__3, &do_extrap__, ntbd, &nz, &tbd[1], &tbz[jz], &tbtt[
		    jz * tbtt_dim1 + 1], maxtbd, &c_b6, &tbds[i__ - 1], &tbzs[
		    j - 1], &ttime, &slow, &dtdz, &dcross, &iext, &jext, &
		    ibad);
	    if (ibad != 0) {
		ihole = ibad;
	    }
	    if (iext != 0) {
		idist = iext;
	    }
	    if (jext != 0) {
		idepth = jext;
	    }
/*           if (ibad.ne.0) then */
/*              tbsls(i,j) = -1.0 */
/*           else */
	    tbsls[i__ + (j << 2) - 5] = slow;
/*           end if */
/* L1020: */
	}
/* L1030: */
    }
/*     Compute slowness and partials at point of interest from mini-table */
    i__1 = *phase_id__ - 1;
    holint2_(&i__1, &do_extrap__, &itotd, &itotz, tbds, tbzs, tbsls, &c__4, &
	    c_b6, delta, zfoc, &slow, &dslddel, &dsldz, &dcross, &iext, &jext,
	     &ibad);
    if (ihole != 0) {
	ibad = ihole;
    }
    if (idist != 0) {
	iext = idist;
    }
    if (idepth != 0) {
	jext = idepth;
    }
/*     Interpolate point in hole of curve -- Value no good */
    if (ibad != 0) {
	*iterr = 11;
/*     Interpolate point less than first distance point in curve */
    } else if (iext < 0 && jext == 0) {
	*iterr = 12;
/*     Interpolate point greater than last distance point in curve */
    } else if (iext > 0 && jext == 0) {
	*iterr = 13;
/*     Interpolate point less than first depth point in curve */
    } else if (iext == 0 && jext < 0) {
	*iterr = 14;
/*     Interpolate point greater than last depth point in curve */
    } else if (iext == 0 && jext > 0) {
	*iterr = 15;
/*     Interpolate point less than first distance point in curve and less */
/*     than first depth point in curve */
    } else if (iext < 0 && jext < 0) {
	*iterr = 16;
/*     Interpolate point greater than last distance point in curve and less */
/*     than first depth point in curve */
    } else if (iext > 0 && jext < 0) {
	*iterr = 17;
/*     Interpolate point less than first distance point in curve and */
/*     greater than first depth point in curve */
    } else if (iext < 0 && jext > 0) {
	*iterr = 18;
/*     Interpolate point greater than last distance point in curve and */
/*     greater than first depth point in curve */
    } else if (iext > 0 && jext > 0) {
	*iterr = 19;
/*     Reset error code to 0 if valid table interpolation */
    } else {
	*iterr = 0;
    }
/*     Compute partial derivatives if point is not in a hole */
    *dcalx = slow;
    dslddel /= (*radius - *zfoc) * (float).017453293;
    if (ibad == 0) {
	azir = *azi * (float).017453293;
	sinazi = sin(azir);
	cosazi = cos(azir);
/*        Axis 1 */
	atx[1] = (float)0.;
/*        Axis 2 points east */
	atx[2] = -dslddel * sinazi;
/*        Axis 3 points north */
	atx[3] = -dslddel * cosazi;
/*        Axis 4 points up */
	atx[4] = -dsldz;
    }
    return 0;
} /* slocal0_ */

/* NAME */
/* 	slocal1 -- Compute horizontal slownesses and partial derivatives. */
/* FILE */
/* 	slocal1.f */
/* SYNOPSIS */
/* 	Compute horizontal slownesses and their partial derivatives for */
/* 	a fixed hypocenter. */
/* DESCRIPTION */
/* 	Subroutine.  Slownesses and their partials are determined via */
/* 	inter/extrapolation of pre-calculated curves.  A point in a hole */
/* 	is rejected. */
/*       ---- On entry ---- */
/* 	radius:		Radius of Earth (km) */
/* 	delta:		Distance from the event to the station (deg) */
/* 	azi:		Forward-azimuth from event to station (deg) */
/* 	zfoc:		Event focal depth (km below sea level) */
/* 	maxtbd:		Maximum dimension of i'th position in tbd(), tbtt() */
/* 	maxtbz:		Maximum dimension of j'th position in tbz(), tbtt() */
/* 	ntbd:		Number of distance samples in tables */
/* 	ntbz:		Number of depth samples in tables */
/* 	tbd(i):		Distance samples for tables (deg) */
/* 	tbz(j):		Depth samples in tables (km) */
/* 	tbtt(i,j):	Travel-time tables (sec) */
/* 	---- On return ---- */
/* 	dcalx:	Calculated slownesses (sec/deg) */
/* 	atx(4):	Partial derivatives (sec/km/deg) */
/* 	iterr:	Error code for n'th observation */
/* 		=  0, No problem, normal interpolation */
/* 		= 11, Distance-depth point (x0,z0) in hole of T-T curve */
/* 		= 12, x0 < x(1) */
/* 		= 13, x0 > x(max) */
/* 		= 14, z0 < z(1) */
/* 		= 15, z0 > z(max) */
/* 		= 16, x0 < x(1) and z0 < z(1) */
/* 		= 17, x0 > x(max) and z0 < z(1) */
/* 		= 18, x0 < x(1) and z0 > z(max) */
/* 		= 19, x0 > x(max) and z0 > z(max) */
/* 	[NOTE:	If any of these codes are negative (e.g., iderr = -17), */
/* 		then, the datum was used to compute the event location] */
/* 	---- Subroutines called ---- */
/* 	From libinterp */
/* 		brack:	Bracket travel-time data via bisection */
/* 		holin2:	Quadratic interpolation function */
/* DIAGNOSTICS */
/* 	Currently assumes a constant radius Earth (i.e., it ignores */
/* 	ellipicity of the Earth. */
/* NOTES */
/* 	Future plans include dealing with ellipicity and higher-order */
/* 	derivatives. */
/* SEE ALSO */
/* 	Subroutines azcal0, and ttcal0 are parallel routines for computing */
/* 	azimuthal and travl-time partial derivatives, respectively. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
/* Subroutine */ int slocal1_(zfoc, radius, delta, azi, maxtbd, maxtbz, ntbd, 
	ntbz, tbd, tbz, tbtt, dcalx, atx, iterr)
real *zfoc, *radius, *delta, *azi;
integer *maxtbd, *maxtbz, *ntbd, *ntbz;
real *tbd, *tbz, *tbtt, *dcalx;
doublereal *atx;
integer *iterr;
{
    /* System generated locals */
    integer tbtt_dim1, tbtt_offset, i__1, i__2, i__3, i__4;

    /* Builtin functions */
    double sin(), cos();

    /* Local variables */
    static integer imin, jmin, imax, jmax;
    static real tbds[4], dtdz;
    static doublereal azir;
    static integer iext, jext;
    static real tbzs[4], slow;
    static integer i__, j;
    extern /* Subroutine */ int brack_();
    static integer ileft, jleft;
    static real dsldz, ttime;
    static integer itotd;
    static real tbsls[16]	/* was [4][4] */;
    static integer itotz;
    extern /* Subroutine */ int holin2_();
    static integer jz, nz;
    static doublereal cosazi;
    static real dcross;
    static doublereal sinazi;
    static real dslddel;
    static integer ibad;

/*     ---- Parameter declarations ---- */
/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     Convert radians to degrees */
/*     Max. number of permissable parameters */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Form arrays holding distances and depths around point of interest */
    /* Parameter adjustments */
    --tbd;
    tbtt_dim1 = *maxtbd;
    tbtt_offset = 1 + tbtt_dim1 * 1;
    tbtt -= tbtt_offset;
    --tbz;
    --atx;

    /* Function Body */
    brack_(ntbd, &tbd[1], delta, &ileft);
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    imin = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbd, i__2 = ileft + 2;
    imax = min(i__1,i__2);
    itotd = 0;
    i__1 = imax;
    for (i__ = imin; i__ <= i__1; ++i__) {
	++itotd;
	tbds[itotd - 1] = tbd[i__];
/* L1000: */
    }
    brack_(ntbz, &tbz[1], zfoc, &jleft);
/* Computing MAX */
    i__1 = 1, i__2 = jleft - 1;
    jmin = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbz, i__2 = jleft + 2;
    jmax = min(i__1,i__2);
    itotz = 0;
    i__1 = jmax;
    for (j = jmin; j <= i__1; ++j) {
	++itotz;
	tbzs[itotz - 1] = tbz[j];
/* L1010: */
    }
/*     Compute travel time and horizontal slownesses for each point in */
/*     arrays around the point of interest */
/*     Find relevant range of table depths */
    i__1 = itotd;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = itotz;
	for (j = 1; j <= i__2; ++j) {
	    brack_(ntbz, &tbz[1], zfoc, &ileft);
/* Computing MAX */
	    i__3 = 1, i__4 = ileft - 1;
	    jz = max(i__3,i__4);
/* Computing MIN */
	    i__3 = *ntbz, i__4 = ileft + 2;
	    nz = min(i__3,i__4) - jz + 1;
/*           Return travel time and create a mini-table of partials */
	    holin2_(ntbd, &nz, &tbd[1], &tbz[jz], &tbtt[jz * tbtt_dim1 + 1], 
		    maxtbd, &c_b6, &tbds[i__ - 1], &tbzs[j - 1], &ttime, &
		    slow, &dtdz, &dcross, &iext, &jext, &ibad);
	    if (ibad != 0) {
		tbsls[i__ + (j << 2) - 5] = (float)-1.;
	    } else {
		tbsls[i__ + (j << 2) - 5] = slow;
	    }
/* L1020: */
	}
/* L1030: */
    }
/*     Compute slowness and partials at point of interest from nimi-table */
    holin2_(&itotd, &itotz, tbds, tbzs, tbsls, &c__4, &c_b6, delta, zfoc, &
	    slow, &dslddel, &dsldz, &dcross, &iext, &jext, &ibad);
/*     Interpolate point in hole of curve -- Value no good */
    if (ibad != 0) {
	*iterr = 11;
/*     Interpolate point less than first distance point in curve */
    } else if (iext < 0 && jext == 0) {
	*iterr = 12;
/*     Interpolate point greater than last distance point in curve */
    } else if (iext > 0 && jext == 0) {
	*iterr = 13;
/*     Interpolate point less than first depth point in curve */
    } else if (iext == 0 && jext < 0) {
	*iterr = 14;
/*     Interpolate point greater than last depth point in curve */
    } else if (iext == 0 && jext > 0) {
	*iterr = 15;
/*     Interpolate point less than first distance point in curve and less */
/*     than first depth point in curve */
    } else if (iext < 0 && jext < 0) {
	*iterr = 16;
/*     Interpolate point greater than last distance point in curve and less */
/*     than first depth point in curve */
    } else if (iext > 0 && jext < 0) {
	*iterr = 17;
/*     Interpolate point less than first distance point in curve and */
/*     greater than first depth point in curve */
    } else if (iext < 0 && jext > 0) {
	*iterr = 18;
/*     Interpolate point greater than last distance point in curve and */
/*     greater than first depth point in curve */
    } else if (iext > 0 && jext > 0) {
	*iterr = 19;
/*     Reset error code to 0 if valid table interpolation */
    } else {
	*iterr = 0;
    }
/*     Compute partial derivatives if point is not in a hole.  The local */
/*     cartesian coordinate system is such that */
    if (ibad == 0) {
	azir = *azi * (float).017453293;
	sinazi = sin(azir);
	cosazi = cos(azir);
	*dcalx = slow;
	dslddel /= (*radius - *zfoc) * (float).017453293;
/*        Axis 1 */
	atx[1] = (float)0.;
/*        Axis 2 points east */
	atx[2] = -dslddel * sinazi;
/*        Axis 3 points north */
	atx[3] = -dslddel * cosazi;
/*        Axis 4 points up */
	atx[4] = -dsldz;
    }
    return 0;
} /* slocal1_ */

