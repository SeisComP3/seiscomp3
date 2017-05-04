#include "utils.h"


/* Table of constant values */

static float c_b2 = (float)0.;
static float c_b3 = (float)1.;


int cart_(float *alat, float *alon, float *z, float *radius, float *x);
int geog_(float *x, float *radius, float *alat, float *alon, float *z);
int rotate_(float *alat, float *alon, float *x);
double r_sign(float *a, float *b);


int latlon_(float *alat1, float *alon1, float *delta, float *azi, float *alat2, float *alon2) {
	/* System generated locals */
	float r__1, r__2;

	/* Local variables */
	float dlon, x[3], z__;


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

	if ( dabs(*alon2) > (float)180. ) {
		r__1 = (float)360. - dabs(*alon2);
		*alon2 = -r_sign(&r__1, alon2);
	}

	return 0;
}
