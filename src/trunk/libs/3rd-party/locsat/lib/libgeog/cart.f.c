#include <math.h>


/* Table of constant values */

static double c_b2 = .99664710813076274;
static double c_b3 = 2.;

int cart_(float *alat, float *alon, float *z, float *radius, float *x) {
	/* Builtin functions */
	double pow_dd();

	/* Local variables */
	static float alat2, r13, r123, esq;


	/* Convert a geographical location to geocentric cartesian coordinates, */

	/*  changed by johannes schweitzer for geographical coordinates */
	/*     mar 17, 1992 */
	/*    (willmore (1979): manual of seismological practice) */

	/* ########### ( assuming a spherical earth.) */

	/* The cartesian axis are such that */
	/*     - Axis 1 intersects equator at 90 deg longitude (east) */
	/*     - Axis 2 intersects north pole */
	/*     - Axis 3 intersects equator at  0 deg longitude */

	/* Input */

	/*   ALAT  =  latitude (degrees) */
	/*   ALON  =  longitude (degrees) */
	/*   Z     =  depth */
	/*   RADIUS  =  radius of the earth */

	/* Output */

	/*   X(1:3)  =  vector of geocentric cartesian coordinates */
	/*              axis 1 intersects equator at  0 deg longitude */
	/*              axis 2 intersects equator at 90 deg longitude */
	/*              axis 3 intersects north pole */

	/*     transform geographical coodinates in geocentric (spherical) */
	/*     coordinates */

	/* Parameter adjustments */
	--x;

	/* Function Body */
	esq = pow_dd(&c_b2, &c_b3);
	alat2 = atan(tan(*alat * (float).017453292523928399) * esq);
	r123 = *radius - *z;
	r13 = r123 * cos(alat2);
	x[1] = r13 * sin(*alon * (float).017453292523928399);
	x[2] = r123 * sin(alat2);
	x[3] = r13 * cos(*alon * (float).017453292523928399);

	return 0;
}
