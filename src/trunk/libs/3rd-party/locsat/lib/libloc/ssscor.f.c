/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsssscor_1_ {
    char sccsid[80];
};

#define sccsssscor_1 (*(struct sccsssscor_1_ *) &sccsssscor_)

static struct {
    real splat[18900]	/* was [3][15][2][210] */, splon[18900]	/* was [3][15]
	    [2][210] */, xlat1[1350]	/* was [3][15][2][15] */, xlat2[1350]	
	    /* was [3][15][2][15] */, xlon1[1350]	/* was [3][15][2][15] 
	    */, xlon2[1350]	/* was [3][15][2][15] */;
    shortint stacor[303750]	/* was [3][15][2][3375] */, numsrcs[2250]	
	    /* was [3][15][2][25] */, cumulsrcs[2250]	/* was [3][15][2][25] 
	    */, nodecumul[1350]	/* was [3][15][2][15] */, nlats[1350]	/* 
	    was [3][15][2][15] */, nlons[1350]	/* was [3][15][2][15] */, 
	    indexstacor[900]	/* was [3][150][2] */;
} corrs_;

#define corrs_1 corrs_

/* Initialized data */

struct {
    char e_1[80];
    } sccsssscor_ = { {'@', '(', '#', ')', 's', 's', 's', 'c', 'o', 'r', '.', 
	    'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9',
	     '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* NAME */
/* 	ssscor -- Apply source-specific station correction adjustments. */
/* FILE */
/* 	ssscor.f */
/* SYNOPSIS */
/* 	Correct for source-specific station effects in the observed travel */
/* 	times for a given, phase-type/correction-type/station/region group. */
/* DESCRIPTION */
/* 	Subroutine.  Apply source-specific station corrections here to */
/* 	partially compensate for lateral velocity heterogeneities. */
/* 	Distinct corrections exist for every applicable phase-type, */
/* 	correction-type (travel time, azimuth, slowness), station and */
/* 	region (regional or local). */
/* 	---- On entry ---- */
/* 	elat:	Event latitude (decmimal degrees) */
/* 	elon:	Event longitude (decmimal degrees) */
/* 	itype:	Type code for n'th observation */
/* 		  = 1, Arrival time datum */
/* 		  = 2, Azimuth datum */
/* 		  = 3, Slowness datum */
/* 	ista:	Station index for n'th detection */
/* 	iwav:	Wave index for n'th detection */
/* 	---- On return ---- */
/* 	correct:	Source-specific station correction (sec) */
/* DIAGNOSTICS */
/* 	Will complain if incompatabilities are encountered. */
/* FILES */
/* 	None. */
/* NOTES */
/* 	Many arrays are passed in common block, corrs, via file, 'rdcor.h'. */
/* SEE ALSO */
/* 	Source-specific station correction read routine, rdcortab(). */
/* AUTHOR */
/* 	Walter Nagy, October 1990. */
/* Subroutine */ int ssscor_(elat, elon, correct, itype, ista, iwav)
real *elat, *elon, *correct;
integer *itype, *ista, *iwav;
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer ibeg, iend;
    static real dlat;
    static integer ilat;
    static real dlon;
    static integer icnt, nlat, ilon, isrc, ncnt, nlon, isrc2, i__, j, iarea;
    extern /* Subroutine */ int splie2_(), splin2_();
    static real ya[225]	/* was [15][15] */;
    static integer is;
    static real x1a[15], x2a[15], y2a[225]	/* was [15][15] */;
    static integer is2;
    static real xln1, xln2;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     Parameter declarations */
/*     Station correction input file unit number */
/*     Maximum permissable different station correction types */
/*     Maximum number of total stations permitted */
/*     Maximum number of stations WITH corrections permitted */
/*     Maximum permissable number of nodes in lat/lon directions */
/*     Maximum average number of tables per station/correction-type pair */
/*     Maximum permissable number of phases */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Check if this station/correction-type exists */
    is = corrs_1.indexstacor[(797400 + (0 + (*itype + (*ista + 150) * 3 - 454 
	    << 1)) - 797400) / 2];
    if (is == 0) {
	return 0;
    }
/*     Check if this station/correction-type/phase-type exists */
    iend = corrs_1.cumulsrcs[*itype + (is + ((*iwav << 1) + 1) * 15) * 3 - 
	    139];
    if (iend == 0) {
	return 0;
    }
    ibeg = iend - corrs_1.numsrcs[*itype + (is + ((*iwav << 1) + 1) * 15) * 3 
	    - 139] + 1;
/*     Now determine whether event is located within one of the regional */
/*     source regions -- If not, return w/o a correction */
    i__1 = iend;
    for (isrc = ibeg; isrc <= i__1; ++isrc) {
	xln1 = corrs_1.xlon1[*itype + (is + ((isrc << 1) + 1) * 15) * 3 - 139]
		;
	xln2 = corrs_1.xlon2[*itype + (is + ((isrc << 1) + 1) * 15) * 3 - 139]
		;
	dlon = *elon;
	if (*elat < corrs_1.xlat1[*itype + (is + ((isrc << 1) + 1) * 15) * 3 
		- 139] && *elat > corrs_1.xlat2[*itype + (is + ((isrc << 1) + 
		1) * 15) * 3 - 139]) {
	    if (xln1 < (float)0.) {
		xln1 += (float)360.;
	    }
	    if (xln2 < (float)0.) {
		xln2 += (float)360.;
	    }
	    if (*elon < (float)0.) {
		dlon = *elon + (float)360.;
	    }
	    if (dlon > xln1 && dlon < xln2) {
		goto L1010;
	    }
	}
/* L1000: */
    }
    return 0;
/*     Regional source region found, now look for a local source region */
L1010:
    iarea = 1;
    is2 = corrs_1.indexstacor[*itype + (*ista + 300) * 3 - 454];
    if (is2 == 0) {
	goto L1040;
    }
    iend = corrs_1.cumulsrcs[*itype + (is2 + ((*iwav << 1) + 2) * 15) * 3 - 
	    139];
    if (iend == 0) {
	goto L1040;
    }
    ibeg = iend - corrs_1.numsrcs[*itype + (is2 + ((*iwav << 1) + 2) * 15) * 
	    3 - 139] + 1;
    i__1 = iend;
    for (isrc2 = ibeg; isrc2 <= i__1; ++isrc2) {
	xln1 = corrs_1.xlon1[*itype + (is2 + ((isrc2 << 1) + 2) * 15) * 3 - 
		139];
	xln2 = corrs_1.xlon2[*itype + (is2 + ((isrc2 << 1) + 2) * 15) * 3 - 
		139];
	if (*elat < corrs_1.xlat1[*itype + (is2 + ((isrc2 << 1) + 2) * 15) * 
		3 - 139] && *elat > corrs_1.xlat2[*itype + (is2 + ((isrc2 << 
		1) + 2) * 15) * 3 - 139]) {
	    if (xln1 < (float)0.) {
		xln1 += (float)360.;
	    }
	    if (xln2 < (float)0.) {
		xln2 += (float)360.;
	    }
	    if (*elon < (float)0.) {
		dlon = *elon + (float)360.;
	    }
	    if (dlon > xln1 && dlon < xln2) {
		is = is2;
		isrc = isrc2;
		iarea = 2;
		goto L1040;
	    }
	}
/* L1020: */
    }
/*     Now determine a regional/local source-specific station correction */
L1040:
    icnt = isrc - 1;
    dlat = corrs_1.xlat1[*itype + (is + (iarea + (isrc << 1)) * 15) * 3 - 139]
	     - *elat;
    dlon = *elon - corrs_1.xlon1[*itype + (is + (iarea + (isrc << 1)) * 15) * 
	    3 - 139];
    if (dlon < (float)0.) {
	dlon += (float)360.;
    }
    nlat = corrs_1.nlats[*itype + (is + (iarea + (isrc << 1)) * 15) * 3 - 139]
	    ;
    nlon = corrs_1.nlons[*itype + (is + (iarea + (isrc << 1)) * 15) * 3 - 139]
	    ;
    ncnt = corrs_1.nodecumul[*itype + (is + (iarea + (isrc << 1)) * 15) * 3 - 
	    139];
    ilat = icnt * (nlat - 1);
    ilon = icnt * (nlon - 1);
    x1a[0] = (float)0.;
    x2a[0] = (float)0.;
    i__1 = nlat;
    for (i__ = 2; i__ <= i__1; ++i__) {
/* L1050: */
	x1a[i__ - 1] = x1a[i__ - 2] + corrs_1.splat[*itype + (is + (iarea + (
		ilat + i__ - 1 << 1)) * 15) * 3 - 139];
    }
    i__1 = nlon;
    for (i__ = 2; i__ <= i__1; ++i__) {
/* L1060: */
	x2a[i__ - 1] = x2a[i__ - 2] + corrs_1.splon[*itype + (is + (iarea + (
		ilon + i__ - 1 << 1)) * 15) * 3 - 139];
    }
    i__1 = nlat;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = nlon;
	for (j = 1; j <= i__2; ++j) {
/* L1070: */
	    ya[i__ + j * 15 - 16] = (real) corrs_1.stacor[*itype + (is + (
		    iarea + (ncnt + j << 1)) * 15) * 3 - 139] / (float)100.;
	}
	ncnt += nlon;
/* L1080: */
    }
/*     Calculate 2nd derivatives */
    splie2_(x1a, x2a, ya, &nlat, &nlon, y2a);
/*     Determine bi-cubic interpolated value at point (dlat, dlon) */
    splin2_(x1a, x2a, ya, y2a, &nlat, &nlon, &dlat, &dlon, correct);
    return 0;
} /* ssscor_ */

