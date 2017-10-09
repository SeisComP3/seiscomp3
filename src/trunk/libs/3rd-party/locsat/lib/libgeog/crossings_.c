
/*
 * NAME
 *	crossings_ -- Determine crossing points of two small circles.

 * FILE
 *	crossings_.c

 * SYNOPSIS
 *	Calculate the latitude and longitude crossing points of two
 *	geographical small circles. 

 * DESCRIPTION
 *	Function.  Compute the latitude and longitude crossing points
 *	from two geographical small circles using non-Naperian 
 *	trigonometric relations.

 *	---- On entry ----
 *	olat1:	Latitudinal center of smaller circle (deg)
 *	olon1:	Longitudinal center of smaller circle (deg)
 *	olat2:	Latitudinal center of larger  circle (deg)
 *	olon2:	Longitudinal center of larger  circle (deg)
 *	rsmall:	Radius of smaller circle (deg)
 *	rlarge:	Radius of larger circle (deg)

 *	---- On return ----
 *	xlat1:	First latitudinal  crossing of 2 small circles (deg)
 *	xlon1:	First longitudinal crossing of 2 small circles (deg)
 *	xlat2:	Second latitudinal  crossing of 2 small circles (deg)
 *	xlon2:	Second longitudinal crossing of 2 small circles (deg)
 *	icerr:	= 0, OK
 *		= 1, No crossing points exist

 *	---- Functions called ----
 *	Local
 *		distaz2:	Determine the distance between between two
 *				lat./lon. pairs
 *		latlon2:	Compute a second lat./lon. from first 
 *				distance and azimuth

 * DIAGNOSTICS
 *	If no valid crossing points exist, set error flag, icerr.

 * NOTES
 *

 * SEE ALSO
 *

 * AUTHOR
 *	Walter Nagy, August 1990.
 */


#include <math.h>

#define DEG_TO_RAD	0.017453293
#define	SIGN(a1, a2)	((a2) >= 0 ? -(a1) : (a1))

void distaz2_(double *alat1, double *alon1, double *alat2, double *alon2, double *delta, double *azi, double *baz);
void latlon2_(double *alat1, double *alon1, double *delta, double *azi, double *alat2, double *alon2);

void
crossings_(double *olat1, double *olon1, double *olat2, double *olon2, double *rsmall, double *rlarge,
           double *xlat1, double *xlon1, double *xlat2, double *xlon2, int *icerr) {
	double alpha, arg, azi, baz, s, stadist, tmp;

	*icerr = 0;

	/* If small circle greater than larger circle, then correct */

	if (*rsmall > *rlarge)
	{
		tmp     = *rlarge;
		*rlarge = *rsmall;
		*rsmall = tmp;
		tmp     = *olat1;
		*olat1  = *olat2;
		*olat2  = tmp;
		tmp     = *olon1;
		*olon1  = *olon2;
		*olon2  = tmp;
	}

	distaz2_(olat1, olon1, olat2, olon2, &stadist, &azi, &baz);

	/* Test for the case of no intersection, if so, ignore */

	if (fabs(*rsmall-stadist) > *rlarge || *rsmall + stadist < *rlarge)
	{
		*icerr = 1;
		return;
	}

	/*
	 * To determine the angle between the two stations and their 
	 * intersections use non-Naperian trigonometry.  
	 *   The defining equation is:
	 *	alpha =	2 * atan (sqrt(( sin(s-stadist)*sin(s-rsmall) /
	 *		sin(s)*sin(s-rlarge) )) )
	 *	where,	rsmall	= Radius of 1st (smaller) circle
	 *		rlarge	= Radius of 2nd (largest) circle
	 *	and	s	= (rsmall + rlarge + stadist) / 2
	 */

	s   = (*rsmall + *rlarge + stadist) / 2.0;
	s   = s * DEG_TO_RAD;
	arg = ( sin(s - DEG_TO_RAD*stadist)*sin(s - DEG_TO_RAD*(*rlarge)) ) /
	      ( sin(s) * sin(s - DEG_TO_RAD*(*rsmall)) );
	if (arg < 0.0)
	{
		*icerr = 1;
		return;
	}
	alpha = 2.0 * atan( sqrt(arg) );
	alpha = alpha/DEG_TO_RAD;

	/*
	 * Now find the two intersection points from the center of the
	 * larger of the two small circles
	 */

	azi = baz + alpha;
	if (fabs(azi) > 180.0)
		azi = SIGN((360.0-fabs(azi)), azi);
	latlon2_(olat2, olon2, rlarge, &azi, xlat1, xlon1);

	azi = azi - 2.0*alpha;
	if (fabs(azi) > 180.0)
		azi = SIGN((360.0-fabs(azi)), azi);
	latlon2_ (olat2, olon2, rlarge, &azi, xlat2, xlon2);
}
