
/*
 * NAME
 *	ellips_ -- Compute hypocentral event error ellipsoid.

 * FILE
 *	ellips_.c

 * SYNOPSIS
 *	Determine event error ellipsoid and normalized confidence regions.

 * DESCRIPTION
 *	Function.  Given the covariance matrix of the hypocentral estimate,
 *	calculate the error ellipsoid and confidence regions from the 
 *	appropriate marginal variances with F-distribution factors ignored.
 *	These are normalized in the sense that they are scaled to a 
 *	particular confidence probability, thereby making the marginal 
 *	variances justified.

 *	The following parameters are described:
 *	   1.	The three-dimensional confidence ellipsoid of the hypocenter,
 *		as determined from the 3x3 variance matrix of the hypocenter
 *		(marginal w.r.t. origin time)
 *	   2.	The two-dimensional confidence ellipse of the epicenter, as
 *		determined from the 2x2 variance matrix of the epicenter
 *		(marginal w.r.t. origin time and depth)
 *	   3.	The one-dimensional confidence interval of focal depth, as
 *		determined from the scalar variance of the focal depth
 *		(marginal w.r.t. origin time and epicenter)
 
 *	Indexing ----
 *	j = 1, np;	k = 1, np;

 *	---- On entry ----
 *	np:	Number of hypocentral parameters; either 2 or 3
 *		The parameters in order are defined to be:
 *		   1.	Position along the local East direction (km),
 *		   2.	Position along the local North direction (km),
 *		   3.	Position along the local Up direction (km); 
 *		   	(i.e., minus depth).
 *	covar[j][k]:	Parameter variance matrix (covariance of j'th and 
 *			k'th parameters).  If np = 2, then covar(j,3) and 
 *			covar(3,k) are assumed to be zero.  Leading 
 *			dimension is 3.
 
 *	---- On return ----
 *	hymaj:	Length of major semi-axis of hypocenter confidence ellipsoid
 *	hymid:	Length of middle semi-axis of hypocenter confidence ellipsoid
 *	hymin:	Length of minor semi-axis of hypocenter confidence ellipsoid
 *	hystr:	Strike of major semi-axis of hypocenter confidence ellipsoid
 *		(degrees clockwise from north)
 *	hyplu:	Plunge of major semi-axis of hypocenter confidence ellipsoid
 *		(degrees downward from horizontal)
 *	hyrak:	Direction of middle semi-axis of hypocenter confidence ellipsoid
 *		(degrees clockwise from up, when looking down-plunge)
 *	epmaj:	Length of major semi-axis of epicenter confidence ellipse
 *	epmin:	Length of minor semi-axis of epicenter confidence ellipse
 *	epstr:	Strike of major semi-axis of epicenter confidence ellipse
 *		(degrees clockwise from north)
 *	zfint:	Length of focal depth confidence semi-interval
 *	stt  :	time-time parameter covariance element
 *	stx  :	time-lon parameter covariance element
 *	sty  :	time-lat parameter covariance element
 *	stz  :	time-depth parameter covariance element
 *	sxx  :	lon-lon parameter covariance element
 *	sxy  :	lon-lat parameter covariance element
 *	sxz  :	lon-depth parameter covariance element
 *	syy  :	lat-lat parameter covariance element
 *	syz  :	lat-depth parameter covariance element
 *	szz  :	depth-depth parameter covariance element
 
 *	If np = 2, then the variance of focal depth is assumed zero.
 *	This results in hymin = hyplu = hyrak = zfint = 0, hymaj = epmaj,
 *	hymid = epmin, and hystr = epstr.
 
 * DIAGNOSTICS
 *	Three-dimensional ellipsoid parameters not yet inplemented.  Output 
 *	variables hy[...] are currently returned as zero.

 * NOTES
 *	Eventually, a programmer should deal with the three-dimensional 
 *	ellipsoid calculations.

 * SEE ALSO
 *	Bratt and Bache (1988, BSSA, 78, 780-798; and 
 *	Jordan and Sverdrup (1981, BSSA, 71, 1105-1130).

 * AUTHOR
 *	Steve Bratt & Walter Nagy.
 */


#ifdef SCCSID
static	char	SccsId[] = "@(#)ellips_.c	44.1	9/20/91";
#endif

#include <math.h>

#define	RAD_TO_DEG	57.2957795
#define	PI		3.141592654
#define TWOPI		2.0*PI
#define	MAXPARM		4

void ellips_ (np, covar, hymaj, hymid, hymin, hystr, hyplu, hyrak,
	      epmaj, epmin, epstr, zfint, stt, stx, sty, sxx, sxy, syy,
	      stz, sxz, syz, szz)
 
int	*np;
double	covar[][MAXPARM];
float	*epstr, *stt, *stx, *sty, *stz, *sxx, *sxy, *sxz, *syy, *syz, *szz;
double	*epmaj, *epmin, *hymaj, *hymid, *hymin, *hyplu, *hyrak, *hystr, *zfint;

{
	double	a2, b2, c, cc, epstrr, s, ss, sxytcs, twosxy;

	/* Set variance elements */

	*stt = covar[0][0];
	*stx = covar[0][1];
	*sty = covar[0][2];
	*sxx = covar[1][1];
	*sxy = covar[1][2];
	*syy = covar[2][2];
	if (*np != MAXPARM)
	{
		*stz = 0.0;
		*sxz = 0.0;
		*syz = 0.0;
		*szz = 0.0;
	}
	else
	{
		*stz = covar[0][3];
		*sxz = covar[1][3];
		*syz = covar[2][3];
		*szz = covar[3][3];
	}
 
	/* Compute two-dimenstional ellipse parameters from marginal 
	   epicentral variance
	 */
 
	twosxy = 2.0*(*sxy);
	if (twosxy != 0.0)
		epstrr = 0.5*atan2(twosxy, *syy-*sxx);
	else
		epstrr = 0.0;
	c      = cos(epstrr);
	s      = sin(epstrr);
	cc     = c*c;
	ss     = s*s;
	sxytcs = twosxy*c*s;
	a2     = *sxx*ss + sxytcs + *syy*cc;
	b2     = *sxx*cc - sxytcs + *syy*ss;
	if (epstrr < 0.0)   epstrr = epstrr + TWOPI;
	if (epstrr > TWOPI) epstrr = epstrr - TWOPI;
	if (epstrr > PI)    epstrr = epstrr - PI;
	if (a2 < 0.0)
		*epmaj = -1.0;
	else
		*epmaj = sqrt(a2);
	if (b2 < 0.0)
		*epmin = -1.0;
	else
		*epmin = sqrt(b2);
	*epstr = RAD_TO_DEG*epstrr;
 
	/* Compute the one-dimensional depth confidence semi-interval
	   from marginal depth variance */
 
	if (*szz < 0.0)
		*zfint = -1.0;
	else
		*zfint = sqrt(*szz);
 
}

