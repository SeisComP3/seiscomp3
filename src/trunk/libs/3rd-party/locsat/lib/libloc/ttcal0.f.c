/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include <stdio.h>
#include "f2c.h"

/* Common Block Declarations */

struct sccsttcal0_1_ {
    char sccsid[80];
};

#define sccsttcal0_1 (*(struct sccsttcal0_1_ *) &sccsttcal0_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsttcal0_ = { {'@', '(', '#', ')', 't', 't', 'c', 'a', 'l', '0', '.', 
	    'f', '\t', '4', '4', '.', '2', '\t', '1', '0', '/', '3', '1', '/',
	     '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static float c_b2 = (float)-1.;

/* NAME */
/* 	ttcal0 -- Compute travel times and their partial derivatives. */
/* FILE */
/* 	ttcal0.f */
/* SYNOPSIS */
/* 	Compute travel times and their partial derivatives for a fixed */
/* 	hypocenter. */
/* DESCRIPTION */
/* 	Subroutine.  Travel times and partials are determined via inter/ */
/* 	extrapolation of pre-calculated travel-time curves.  A point in a */
/* 	hole is rejected. */
/* 	---- On entry ---- */
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
/* 	Subroutines azcal0, and slocal0 are parallel routines for computing */
/* 	azimuthal and slowness partial derivatives, respectively. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
/* Subroutine */ int ttcal0_(phase_id__, zfoc, radius, delta, azi, maxtbd, 
	maxtbz, ntbd, ntbz, tbd, tbz, tbtt, dcalx, atx, iterr)
int *phase_id__;
float *zfoc, *radius, *delta, *azi;
int *maxtbd, *maxtbz, *ntbd, *ntbz;
float *tbd, *tbz, *tbtt, *dcalx;
doublereal *atx;
int *iterr;
{
    /* System generated locals */
    int tbtt_dim1, tbtt_offset, i__1, i__2;

    /* Builtin functions */
    double sin(), cos();

    /* Local variables */
    static float dtdz;
    static doublereal azir;
    static int iext, jext;
    extern /* Subroutine */ int brack_();
    static int ileft, do_extrap__;
    static float dtddel;
    static int jz, nz;
    static doublereal cosazi;
    static float dcross;
    static doublereal sinazi;
    extern /* Subroutine */ int holint2_();
    static doublereal pd12;
    static int ibad;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declaration ---- */
/*     Convert radians to degrees */
/*     Max. number of permissable parameters */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Permit extrapolation */
    /* Parameter adjustments */
    --tbd;
    tbtt_dim1 = *maxtbd;
    tbtt_offset = 1 + tbtt_dim1 * 1;
    tbtt -= tbtt_offset;
    --tbz;
    --atx;

    /* Function Body */
    do_extrap__ = 0;
/*     Find relevant range of table depths */
    brack_(ntbz, &tbz[1], zfoc, &ileft);
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    jz = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbz, i__2 = ileft + 2;
    nz = min(i__1,i__2) - jz + 1;
/*     Subroutine HOLIN2 performs bivariate interpolation */
    i__1 = *phase_id__ - 1;
    holint2_(&i__1, &do_extrap__, ntbd, &nz, &tbd[1], &tbz[jz], &tbtt[jz * 
	    tbtt_dim1 + 1], maxtbd, &c_b2, delta, zfoc, dcalx, &dtddel, &dtdz,
	     &dcross, &iext, &jext, &ibad);
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
/*     Interpolate point less than first distance point in curve and */
/*     less than first depth point in curve */
    } else if (iext < 0 && jext < 0) {
	*iterr = 16;
/*     Interpolate point greater than last distance point in curve and */
/*     less than first depth point in curve */
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
/*     cartesian coordinate system is such that, */
    if (ibad == 0) {
	azir = *azi * (float).017453293;
	sinazi = sin(azir);
	cosazi = cos(azir);
	pd12 = dtddel / ((*radius - *zfoc) * (float).017453293);
/*        Axis 1 */
	atx[1] = (float)1.;
/*        Axis 2 points east */
	atx[2] = -pd12 * sinazi;
/*        Axis 3 points north */
	atx[3] = -pd12 * cosazi;
/*        Axis 4 points up */
	atx[4] = -dtdz;
    }
    return 0;
} /* ttcal0_ */

/* NAME */
/* 	ttcal1 -- Compute travel times and their partial derivatives. */
/* FILE */
/* 	ttcal1.f */
/* SYNOPSIS */
/* 	Compute travel times and their partial derivatives for a fixed */
/* 	hypocenter. */
/* DESCRIPTION */
/* 	Subroutine.  Travel times and partials are determined via inter/ */
/* 	extrapolation of pre-calculated travel-time curves.  A point in a */
/* 	hole is rejected. */
/* 	---- On entry ---- */
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
/* 	Subroutines azcal0, and slocal0 are parallel routines for computing */
/* 	azimuthal and slowness partial derivatives, respectively. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
/* Subroutine */ int ttcal1_(zfoc, radius, delta, azi, maxtbd, maxtbz, ntbd, 
	ntbz, tbd, tbz, tbtt, dcalx, atx, iterr)
float *zfoc, *radius, *delta, *azi;
int *maxtbd, *maxtbz, *ntbd, *ntbz;
float *tbd, *tbz, *tbtt, *dcalx;
doublereal *atx;
int *iterr;
{
    /* System generated locals */
    int tbtt_dim1, tbtt_offset, i__1, i__2;

    /* Builtin functions */
    double sin(), cos();

    /* Local variables */
    static float dtdz;
    static doublereal azir;
    static int iext, jext;
    extern /* Subroutine */ int brack_();
    static int ileft;
    extern /* Subroutine */ int holin2_();
    static float dtddel;
    static int jz, nz;
    static doublereal cosazi;
    static float dcross;
    static doublereal sinazi, pd12;
    static int ibad;

/*     ---- Parameter declaration ---- */
/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     Convert radians to degrees */
/*     Max. number of permissable parameters */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Find relevant range of table depths */
    /* Parameter adjustments */
    --tbd;
    tbtt_dim1 = *maxtbd;
    tbtt_offset = 1 + tbtt_dim1 * 1;
    tbtt -= tbtt_offset;
    --tbz;
    --atx;

    /* Function Body */
    brack_(ntbz, &tbz[1], zfoc, &ileft);
/* Computing MAX */
    i__1 = 1, i__2 = ileft - 1;
    jz = max(i__1,i__2);
/* Computing MIN */
    i__1 = *ntbz, i__2 = ileft + 2;
    nz = min(i__1,i__2) - jz + 1;
/*     Subroutine HOLIN2 performs bivariate interpolation */
    holin2_(ntbd, &nz, &tbd[1], &tbz[jz], &tbtt[jz * tbtt_dim1 + 1], maxtbd, &
	    c_b2, delta, zfoc, dcalx, &dtddel, &dtdz, &dcross, &iext, &jext, &
	    ibad);
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
/*     Interpolate point less than first distance point in curve and */
/*     less than first depth point in curve */
    } else if (iext < 0 && jext < 0) {
	*iterr = 16;
/*     Interpolate point greater than last distance point in curve and */
/*     less than first depth point in curve */
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
/*     cartesian coordinate system is such that, */
    if (ibad == 0) {
	azir = *azi * (float).017453293;
	sinazi = sin(azir);
	cosazi = cos(azir);
	pd12 = dtddel / ((*radius - *zfoc) * (float).017453293);
/*        Axis 1 */
	atx[1] = (float)1.;
/*        Axis 2 points east */
	atx[2] = -pd12 * sinazi;
/*        Axis 3 points north */
	atx[3] = -pd12 * cosazi;
/*        Axis 4 points up */
	atx[4] = -dtdz;
    }
    return 0;
} /* ttcal1_ */

