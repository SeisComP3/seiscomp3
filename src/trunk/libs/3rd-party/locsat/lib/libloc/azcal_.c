
/*
 * NAME
 *	azcal_ -- Compute azimuthal partial derivatives.

 * FILE
 *	azcal_.c

 * SYNOPSIS
 *	Compute azimuthal partial derivatives for a fixed hypocenter.

 * DESCRIPTION
 *	Function.  Given a distance and azimuth from a fixed event 
 *	hypocenter to a station, azcal_() computes azimuthal partial 
 *	derivatives as determined from pre-processed f-k analysis. 

 *	---- On entry ----
 *	radius:	Radius of Earth (km)
 *	delta:	Distance from the event to the station (deg)
 *	azi:	Forward-azimuth from event to station (deg) 
 *	baz:	Back-azimuth from the event to the station (deg) 
 *		(azimuth measured from the station to the event)

 *	---- On return ----
 *	dcalx:	Azimuth data computed from alat and alon (deg)
 *	atx[4]:	Partial derivatives (deg/km)
 
 * DIAGNOSTICS
 *	Currently assumes a constant radius Earth (i.e., it ignores
 *	ellipicity of the Earth).

 * NOTES
 *	Future plans include dealing with ellipicity and higher-order 
 *	derivatives.

 * SEE ALSO
 *	Subroutines ttcal0, and slocal0 are parallel routines for computing
 *	travel-time and slowness partial derivatives, respectively.

 * AUTHOR
 *	Walter Nagy, January 1991.
 */

#ifdef SCCSID
static	char	SccsId[] = "@(#)azcal_.c	44.1	9/20/91";
#endif

#include <math.h>

#define	DEG_TO_RAD	0.017453293

void azcal_ (radius, delta, azi, baz, dcalx, atx)
 
float	*azi, *baz, *delta, *radius;
float	*dcalx;
double	atx[];

{
	double	azir, rt;

	azir = *azi*DEG_TO_RAD;
	rt   = sin(*delta*DEG_TO_RAD)*(*radius)*DEG_TO_RAD;
	if (rt == 0.0) rt = 0.0001;

	/* Calculated azimuth in degrees */
	*dcalx = *baz;

	/* Partial derivatives w.r.t. origin time (time; sec) = 1.0 */
	atx[0] = 0.0;

	/* Partial derivatives w.r.t. East (deg/km) */
	atx[1] = -cos(azir)/rt;

	/* Partial derivatives w.r.t. North (deg/km) */
	atx[2] = sin(azir)/rt;

	/* Partial derivatives w.r.t. Up (depth; km) = 0.0 */
	atx[3] = 0.0;
}

