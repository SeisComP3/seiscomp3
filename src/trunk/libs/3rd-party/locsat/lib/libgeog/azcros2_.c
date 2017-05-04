
/*
 * NAME
 *	azcros2_ -- Find the lat./lon. crossing point from two great circles.

 * FILE
 *	azcros2_.c

 * SYNOPSIS
 *	Given 2 reference points with great-circle azimuths, compute their
 *	intersection.

 * DESCRIPTION
 *	Function.  Given the locations of 2 reference points and 2 azimuths 
 *	of great circles passing through those points, this routine computes 
 *	the location of the crossing of 2 great circles and the distances 
 *	from two reference points.  Assuming a spherical Earth model, 
 *	compute a second lat/lon position on the globe given the first 
 *	position and an azimuth. 

 *	---- On entry ----
 *	alat1:	Latitudinal position of point 1 (deg.)
 *	alon1:	Longitudinal position of point 1 (deg.)
 *	alat2:	Latitudinal position of point 2 (deg.)
 *	alon2:	Longitudinal position of point 2 (deg.)
 *	aza:	Azimuth from north of first  great circle path (deg.)
 *	azb:	Azimuth from north of second great circle path (deg.)

 *	---- On return ----
 *	dista:	Distance from  first reference point to crossing point (deg.)
 *	dista:	Distance from second reference point to crossing point (deg.)
 *	alat:	Latitudinal crossing point (deg.)
 *	alon:	Longitudinal crossing point (deg.)
 *	ierr:	= 0, All OK
 *		= 1, Lines do not cross within a reasonable distance

 *	---- Functions called ----
 *	Local
 *		distaz2:	Determine the distance between between two
 *				lat./lon. pairs
 *		latlon2:	Compute a second lat./lon. from first
 *				distance and azimuth

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
 *	Walter Nagy, December 1990.
 */


#ifdef SCCSID
static  char	SccsId[] = "@(#)azcros2_.c	41.1	12/28/90";
#endif

#include <math.h>

#define RAD_TO_DEG 57.2957795
#define DEG_TO_RAD 1.0/RAD_TO_DEG
#define SIGN(a1, a2) ((a2) >= 0 ? -(a1) : (a1))

void distaz2_(double *alat1, double *alon1, double *alat2, double *alon2, double *delta, double *azi, double *baz);
void latlon2_(double *alat1, double *alon1, double *delta, double *azi, double *alat2, double *alon2);

void azcros2_ (alat1, alon1, aza, alat2, alon2, azb, dista, distb, alat, alon, ierr)

int	*ierr;
double	*alat1, *alat2, *alon1, *alon2, *aza, *azb;
double	*alat, *alon, *dista, *distb;

{
	double	alatin, alonin, az, azi, baz, c1, c2, c3, c4, c5; 
        double  delta, dist, e, f, fa, fb, g, h, ra, rb, rc;
	double	atan(), cos(), fabs(), sin(), tan();

	/* Find azimuth, back azimuth and radial distance between stations */

	distaz2_(alat1, alon1, alat2, alon2, &delta, &azi, &baz);

	/* Find angle measured from line between two stations to aza and azb */

	fa = *aza - azi;
	fb = *azb - baz;
	ra = fa;
	rb = fb;

	if (fabs(ra) > 180.0)
		ra = SIGN((360.0-fabs(ra)), ra);
	if (fabs(rb) > 180.0)
		rb = SIGN((360.0-fabs(rb)), rb);

	/* If the signs of ra and rb are the same, the great circles along
	 * those azimuths will not cross within a "reasonable" distance.
	 */

	if (SIGN(1.0, ra) == SIGN(1.0, rb))
	{
		*ierr = 1;
		return;
	}

	ra = fabs(ra);
	rb = fabs(rb);

	/* If the sum of ra and rb is > 180., there will be no crossing
	 * within a reasonable distance
	 */

	if ((ra + rb) > 180.0)
	{
		*ierr = 1;
		return;
	}

	ra = ra * DEG_TO_RAD;
	rb = rb * DEG_TO_RAD;
	rc = delta * DEG_TO_RAD;

	c1 = tan(0.5*rc);
	c2 = 0.5 * (ra - rb);
	c3 = 0.5 * (ra + rb);

	/* Equations for solving for the distances */

	f = c1 * sin(c2);
	g = sin(c3);
	h = c1 * cos(c2);
	e = cos(c3);

	c4 = atan(f/g);
	c5 = atan(h/e);

	/* Compute distances (lengths of the triangle) */

	*distb = (c4 + c5) * RAD_TO_DEG;
	*dista = (c5 - c4) * RAD_TO_DEG;

	if ((*dista < 0.0) || (*distb < 0.0))
	{
		*ierr = 1;
		return;
	}

	if (*dista < *distb)
	{
		dist   = *dista;
		az     = *aza;
		alatin = *alat1;
		alonin = *alon1;
	}
	else
	{
		dist   = *distb;
		az     = *azb;
		alatin = *alat2;
		alonin = *alon2;
	}
	latlon2_ (&alatin, &alonin, &dist, &az, alat, alon);
	*ierr = 0;
}

