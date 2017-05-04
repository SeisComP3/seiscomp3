
/*
 * NAME
 *	latlon2_ -- Given a lat/lon position and azi, find second lat/lon point.

 * FILE
 *	latlon2_.c

 * SYNOPSIS
 *	Find a point on a sphere which is a given distance and azimuth
 *	away from another point.

 * DESCRIPTION
 *	Assuming a spherical Earth model, compute a second lat/lon position
 *	on the globe given the first position and an azimuth. 

 *	---- On entry ----
 *	alat1:	Latitudinal position of point 1 (deg)
 *	alon1:	Longitudinal position of point 1 (deg)
 *	delta:	Distance between points 1 and 2 (deg)
 *	azi:	Azimuth from north of point 2 w.r.t. point 1 (clockwise)

 *	---- On return ----
 *	alat2:	Latitudinal position of point 2 (deg)
 *	alon2:	Longitudinal position of point 2 (deg)

 * DIAGNOSTICS
 *

 * NOTES
 *	All arguments are in degrees.  Latitude, longitude and delta are 
 *	geocentric.  Latitude is zero at equator and positive North.
 *	Longitude is positive toward the East azi is measured clockwise 
 *	from local North

 * SEE ALSO
 *

 * AUTHOR
 *	Walter Nagy, November 1990
 */


#include <math.h>

#define RAD_TO_DEG	57.2957795
#define DEG_TO_RAD	1.0/RAD_TO_DEG
#define	SIGN(a1, a2)	((a2) >= 0 ? -(a1) : (a1))

void latlon2_(double *alat1, double *alon1, double *delta, double *azi, double *alat2, double *alon2) {
	double alat, alatr, alon, b, c, coslat, dlon;
	double r13, sinlat, x1, x2, x3;

	/* changed for ellipticity of earth
	 * changed use of *alat1 and *alat2
	 */

	double esq, alat3;
	esq=(1.0-1.0/298.25)*(1.0-1.0/298.25);
	alat3=atan(tan(*alat1*DEG_TO_RAD)*esq)*RAD_TO_DEG;

	/* Convert a geographical location to geocentric cartesian 
	 * coordinates, assuming a spherical earth
	 */

	alat = 90.0 - *delta;
	alon = 180.0 - *azi;
	r13  = cos(DEG_TO_RAD*alat);

	/* x1:	Axis 1 intersects equator at  0 deg longitude  
	 * x2:	Axis 2 intersects equator at 90 deg longitude  
	 * x3:	Axis 3 intersects north pole
	 */

	x1 = r13*sin(DEG_TO_RAD*alon);
	x2 = sin(DEG_TO_RAD*alat);
	x3 = r13*cos(DEG_TO_RAD*alon);

	/* Rotate in cartesian coordinates.  The cartesian coordinate system 
	 * is most easily described in geographic terms.  The origin is at 
	 * the Earth's center.  Rotation by alat1 degrees southward, about 
	 * the 1-axis.
	 */

	alatr  = (90.0-alat3)/RAD_TO_DEG;
	sinlat = sin(alatr);
	coslat = cos(alatr);
	b      = x2;
	c      = x3;
	x2     = b*coslat - c*sinlat;
	x3     = b*sinlat + c*coslat;

	/* Convert geocentric cartesian coordinates to a geographical 
	 * location, assuming a spherical earth
	 */

	r13    = sqrt(x3*x3 + x1*x1);
	dlon   = RAD_TO_DEG*atan2(x1, x3);

	/*  changed for ellipticity of earth
	 *   *alat2 = RAD_TO_DEG*atan2(x2, r13);
	 */

	alat3= atan2(x2, r13);
	*alat2=RAD_TO_DEG * atan(tan(alat3)/esq);

	*alon2 = *alon1 + dlon;
	if (fabs(*alon2) > 180.0)
		*alon2 = SIGN((360.0-fabs(*alon2)), *alon2);
}
