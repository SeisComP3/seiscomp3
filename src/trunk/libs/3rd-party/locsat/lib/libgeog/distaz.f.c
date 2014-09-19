/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdistaz_1_ {
    char sccsid[80];
};

#define sccsdistaz_1 (*(struct sccsdistaz_1_ *) &sccsdistaz_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdistaz_ = { {'@', '(', '#', ')', 'd', 'i', 's', 't', 'a', 'z', '.', 
	    'f', '\t', '4', '3', '.', '1', '\t', '9', '/', '9', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Subroutine */ int distaz_(alat1, alon1, alat2, alon2, delta, azi, baz)
real *alat1, *alon1, *alat2, *alon2, *delta, *azi, *baz;
{
    /* Builtin functions */
    double cos(), sin(), acos(), atan2();

    /* Local variables */
    static doublereal cdel, xbaz, ybaz, xazi, yazi, clat1, clat2, rlat1, 
	    rlat2, slat1, slat2, cdlon, rdlon, sdlon;


/* Calculate angular distance, azimuth and backazimuth between two */
/*  points on a sphere. */

/* Input */

/*   ALAT1,ALON1  =  latitude and longitude of point 1. */
/*   ALAT2,ALON2  =  latitude and longitude of point 2. */

/* Output */

/*   DELTA  =  angular distance between points 1 and 2. */
/*   AZI    =  azimuth from north of point 2 w.r.t. point 1. */
/*   BAZ    =  azimuth from north of point 1 w.r.t. point 2. */

/* All arguments are in degrees. */
/* Latitude, longitude and DELTA are geocentric. */
/* Latitude is zero at equator and positive north. */
/* Longitude is positive toward the east. */
/* AZI and BAZ are positive and measured clockwise from local north. */

/* K.S. 1-Dec-97, moved 1 line */

/* If we are given the same point twice, don't do the heavy work */
/* (et) 1/24/89 */

    if (*alat1 != *alat2 || *alon1 != *alon2) {
	goto L10;
    }
    *delta = (float)0.;
    *azi = (float)0.;
    *baz = (float)180.;
    return 0;
/* Continue with original code */

L10:
    rlat1 = *alat1 * .017453292519943295;
    rlat2 = *alat2 * .017453292519943295;
    rdlon = (*alon2 - *alon1) * .017453292519943295;

    clat1 = cos(rlat1);
    clat2 = cos(rlat2);
    slat1 = sin(rlat1);
    slat2 = sin(rlat2);
    cdlon = cos(rdlon);
    sdlon = sin(rdlon);

    cdel = slat1 * slat2 + clat1 * clat2 * cdlon;
    cdel = min(cdel,1.);
    cdel = max(cdel,-1.);
    yazi = sdlon * clat2;
    xazi = clat1 * slat2 - slat1 * clat2 * cdlon;
    ybaz = -sdlon * clat1;
    xbaz = clat2 * slat1 - slat2 * clat1 * cdlon;

    *delta = acos(cdel) * 57.295779513082323;
    *azi = atan2(yazi, xazi) * 57.295779513082323;
    *baz = atan2(ybaz, xbaz) * 57.295779513082323;
    if (*azi < (float)0.) {
	*azi += (float)360.;
    }
    if (*baz < (float)0.) {
	*baz += (float)360.;
    }

    return 0;
} /* distaz_ */

