#include <math.h>

/* Table of constant values */

static double c_b2 = .99664710813076274;
static double c_b3 = 2.;

double pow_dd(double *ap, double *bp);


int geog_(float *x, float *radius, float *alat, float *alon, float *z) {
	/* System generated locals */
	float r__1, r__2;

	/* Local variables */
	static float r13sq, alat2, r13, r123, esq;


	/* Convert geocentric cartesian coordinates to a geographical location, */
	/* ################assuming a spherical earth. */

	/*     changed by johannes schweitzer for transformation back into */
	/*     geographical coordinates on the real elliptic earth. */
	/*     mar 17, 1992 */


	/* The cartesian axis are such that */
	/*     - Axis 1 intersects equator at 90 deg longitude (east) */
	/*     - Axis 2 intersects north pole */
	/*     - Axis 3 intersects equator at  0 deg longitude */

	/* Input */

	/*   X(1:3)  =  vector of geocentric cartesian coordinates */
	/*   RADIUS  =  radius of the earth */

	/* Output */

	/*   ALAT  =  latitude (degrees) */
	/*   ALON  =  longitude (degrees) */
	/*   Z     =  depth */


	/* Parameter adjustments */
	--x;

	/* Computing 2nd power */
	r__1 = x[3];

	/* Computing 2nd power */
	r__2 = x[1];
	r13sq = r__1 * r__1 + r__2 * r__2;
	r13 = sqrt(r13sq);

	/* Computing 2nd power */
	r__1 = x[2];
	r123 = sqrt(r13sq + r__1 * r__1);
	*alon = atan2(x[1], x[3]) * (float)57.2957795;
	alat2 = atan2(x[2], r13);
	*z = *radius - r123;

	/*     ellipticity correction */
	esq = pow_dd(&c_b2, &c_b3);
	*alat = atan(tan(alat2) / esq) * (float)57.2957795;

	return 0;
}
