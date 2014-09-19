
/*
 * NAME
 *	distaz2_ -- Find distance and azimuth between two points on a sphere.

 * FILE
 *	distaz2_.c

 * SYNOPSIS
 *	Find the angular distance, azimuth and backazimuth between two 
 *	lat/lon points assuming a spherical Earth.

 * DESCRIPTION
 *	Assuming a spherical Earth model, compute the distance between
 *	the two positions (usually the event and the station) assuming
 *	an Earth of absolute sphericity. 

 *	---- On entry ----
 *	alat1:	Latitudinal position of point 1 (deg)
 *	alon1:	Longitudinal position of point 1 (deg)
 *	alat2:	Latitudinal position of point 2 (deg)
 *	alon2:	Longitudinal position of point 2 (deg)

 *	---- On return ----
 *	delta:	Distance between points 1 and 2 (deg)
 *	azi:	Azimuth from north of point 2 w.r.t. point 1 (clockwise)
 *	baz:	Back-azimuth from north (clockwise)

 * DIAGNOSTICS
 *

 * NOTES
 *	All arguments are in degrees.  Latitude, longitude and delta are 
 *	geocentric.  Latitude is zero at equator and positive North.
 *	Longitude is positive toward the East azi is measured clockwise 
 *	from local North.

 * SEE ALSO
 *

 * AUTHOR
 *	Walter Nagy, November 1990.
 */


#ifdef SCCSID
static  char	SccsId[] = "@(#)distaz2_.c	40.1	11/15/90";
#endif

#include <math.h>

#define RAD_TO_DEG	57.2957795
#define DEG_TO_RAD	1.0/RAD_TO_DEG

void distaz2_ (alat1, alon1, alat2, alon2, delta, azi, baz)

double	*alat1, *alon1, *alat2, *alon2;
double	*azi, *baz, *delta;

{
	double	clat1, clat2, cdlon, cdel;
	double	rlat1, rlat2, rdlon, slat1, slat2, sdlon;
	double	xazi, xbaz, yazi, ybaz;
	double	acos(), atan2(), cos(), sin();

        /*  changed for ellipticity of earth
	*   changed use of *alat1 and *alat2
        */

	double  esq, alat3, alat4, atan(), tan();
        esq=(1.0-1.0/298.25)*(1.0-1.0/298.25);
        alat3=atan(tan(*alat1*DEG_TO_RAD)*esq)*RAD_TO_DEG;
        alat4=atan(tan(*alat2*DEG_TO_RAD)*esq)*RAD_TO_DEG;


	if ((*alat1 == *alat2) && (*alon1 == *alon2))
	{
		*delta = 0.0;
		*azi = 0.0;
		*baz = 180.0;
		return;
	}

	rlat1 = DEG_TO_RAD * (alat3);
	rlat2 = DEG_TO_RAD * (alat4);
	rdlon = DEG_TO_RAD * (*alon2 - *alon1);

	clat1 = cos(rlat1);
	clat2 = cos(rlat2);
	slat1 = sin(rlat1);
	slat2 = sin(rlat2);
	cdlon = cos(rdlon);
	sdlon = sin(rdlon);

	cdel = slat1*slat2 + clat1*clat2*cdlon;
	cdel = (cdel <  1.0) ? cdel :  1.0;
	cdel = (cdel > -1.0) ? cdel : -1.0;
	yazi = sdlon * clat2;
	xazi = clat1*slat2 - slat1*clat2*cdlon;
	ybaz = -sdlon * clat1;
	xbaz = clat2*slat1 - slat2*clat1*cdlon;

	*delta = RAD_TO_DEG * acos(cdel);
	*azi   = RAD_TO_DEG * atan2(yazi, xazi);
	*baz   = RAD_TO_DEG * atan2(ybaz, xbaz);

	if (*azi < 0.0)
		*azi += 360.0;
	if (*baz < 0.0)
		*baz += 360.0;
}

