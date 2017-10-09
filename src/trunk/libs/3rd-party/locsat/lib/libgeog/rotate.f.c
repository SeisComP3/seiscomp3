#include <math.h>

int rotate_(float *alat, float *alon, float *x) {
	/* Local variables */
	float a, b, c__, alatr, alonr, coslat, sinlat, coslon, sinlon;


	/* Rotate a 3-vector represented in cartesian coordinates. */

	/* The cartesian coordinate system is most easily described */
	/*  in geographic terms.  The origin is at the earth's center. */
	/*  The axes are such that */
	/*     - Axis 1 intersects equator at 90 deg longitude (east) */
	/*     - Axis 2 intersects north pole */
	/*     - Axis 3 intersects equator at  0 deg longitude */

	/* On input, the vector to be rotated has components X(i),i=1,2,3 */
	/*  in this system.  This procedure rotates the vector */
	/*  in the following two steps: */
	/*    1. Rotation by ALON degrees westward, about the 2-axis. */
	/*    2. Rotation by ALAT degrees southward,about the 1-axis. */

	/* On output, X contains the coordinates of the rotated vector. */

	/* Another way to interpret X on output:  as the components */
	/*  of the original vector in a rotated coordinate system. */
	/*  Put yourself on the earth's surface at the point having */
	/*  latitude ALAT and longitude ALON.  Call this point P. */
	/*  This rotated system is also geocentric, in which */
	/*     - Axis 1 points east */
	/*     - Axis 2 points north */
	/*     - Axis 3 points up */
	/*  where east, north and up are the directions local to P. */


	/* Do it */

	/* Parameter adjustments */
	--x;

	/* Function Body */
	alatr = *alat / (float)57.2957795;
	alonr = *alon / (float)57.2957795;
	sinlat = sin(alatr);
	coslat = cos(alatr);
	sinlon = sin(alonr);
	coslon = cos(alonr);

	a = x[1] * coslon - x[3] * sinlon;
	b = x[2];
	c__ = x[1] * sinlon + x[3] * coslon;
	x[1] = a;
	x[2] = b * coslat - c__ * sinlat;
	x[3] = b * sinlat + c__ * coslat;

	return 0;
}
