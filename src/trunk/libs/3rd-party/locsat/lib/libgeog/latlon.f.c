/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccslatlon_1_ {
    char sccsid[80];
};

#define sccslatlon_1 (*(struct sccslatlon_1_ *) &sccslatlon_)

/* Initialized data */

struct {
    char e_1[80];
    } sccslatlon_ = { {'@', '(', '#', ')', 'l', 'a', 't', 'l', 'o', 'n', '.', 
	    'f', '\t', '4', '3', '.', '1', '\t', '9', '/', '9', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static real c_b2 = (float)0.;
static real c_b3 = (float)1.;

/* Subroutine */ int latlon_(alat1, alon1, delta, azi, alat2, alon2)
real *alat1, *alon1, *delta, *azi, *alat2, *alon2;
{
    /* System generated locals */
    real r__1, r__2;

    /* Builtin functions */
    double r_sign();

    /* Local variables */
    extern /* Subroutine */ int geog_(), cart_();
    static real dlon, x[3], z__;
    extern /* Subroutine */ int rotate_();


/* Find a point on a sphere which is a given distance and azimuth */
/*  away from another point. */

/* Input */

/*   ALAT1,ALON1  =  latitude and longitude of point 1. */
/*   DELTA  =  angular distance between points 1 and 2. */
/*   AZI    =  azimuth from north of point 2 w.r.t. point 1. */

/* Output */

/*   ALAT2,ALON2  =  latitude and longitude of point 2. */

/* Subroutines called */

/*   cart */
/*   rotate */
/*   geog */

/* All arguments are in degrees. */
/* Latitude, longitude and DELTA are geocentric. */
/* Latitude is zero at equator and positive north. */
/* Longitude is positive toward the east. */
/* AZI is measured clockwise from local north. */


    r__1 = (float)90. - *delta;
    r__2 = (float)180. - *azi;
    cart_(&r__1, &r__2, &c_b2, &c_b3, x);
    r__1 = (float)90. - *alat1;
    rotate_(&r__1, &c_b2, x);
    geog_(x, &c_b3, alat2, &dlon, &z__);
    *alon2 = *alon1 + dlon;
    if (dabs(*alon2) > (float)180.) {
	r__1 = (float)360. - dabs(*alon2);
	*alon2 = -r_sign(&r__1, alon2);
    }

    return 0;
} /* latlon_ */

