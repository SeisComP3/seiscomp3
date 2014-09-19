/*
 * NAME
 *	predsat -- Predict Slowness, Azimuth and Travel-time bounds.

 * FILE    
 *	predsat.c
 
 * SYNOPSIS
 *	Computes maximun and minimum bounds on the travel-time slowness, 
 *	and azimuth for a set of data (station-phase-datatype) based on 
 *	an event location, confidence ellipse and travel-time curves.
 
 * DESCRIPTION
 *	Function.  Information on travel-time tables, stations, event 
 *	and confidence parameters, the desired stations and phases are 
 *	passed to and from PredSAT via the argument list.  The phase and 
 *	station names given for each datum (dstaid,dwavid) must match in 
 *	the phase and station lists (staid,phase_type).  If all that you desire 
 *	are the predictions from a point, just input 0.0 for all of the 
 *	confidence bounds parameters.  

 *	---- Indexing ----
 *	i = 0, num_sta-1;	j = 0, num_phases-1;	n = 0, ndata-1;

 *	---- On entry ----
 *	num_sta:	Number of stations
 *	num_phases:	Number of phases in list

 *	out_file:	Output file name
 *	data_sta_id:	Name of station for which prediction to be made  
 *	data_phase_type:Name of phase for which prediction to be made
 *	phase_type:	List of accepted phases for arrival times

 *	slow_flag:	Compute slowness bounds (y or n)
 *	print_flag:	Verbose printout (y or n)

 *	---- On return ----
 *	dist_min:	Minimum distance from the ellipse to the station (deg)
 *	dist_max:	Maximum distance from the ellipse to the station (deg)
 *	dist_center:	Distance from the center of ellipse to the station (deg)
 *	tt_min:  	Minimum travel-time for data_phase_type from the
 *			ellipse to the station (sec)
 *	tt_max:  	Maximum travel-time for data_phase_type from the
 *			ellipse to the station (sec)
 *	tt_center: 	Travel-time for data_phase_type from the center of the 
 *			ellipse to the station (sec)
 *	az_min:  	Minimum azimuth from event to the to the station (deg)
 *	az_max:  	Maximum azimuth from data_sta_id ellipse (deg)
 *	az_center: 	Maximum azimuth from data_sta_id to the center of the 
 *			ellipse (deg)
 *	slow_min: 	Minimum slowness for data_phase_type from the 
 *			ellipse to the station (sec/deg)
 *	slow_max: 	Maximum slowness for data_phase_type from the ellipse 
 *			to the station (sec/deg)
 *	slow_center:	Slowness for data_phase_type from the center of the
 *			ellipse to the station (sec/deg)
 *	tt_err_min, tt_err_max, tt_err_center: Error codes for travel-times 
 *	slow_err_min, slow_err_max, slow_err_center: Error codes for slownesses
 *			  =  0,	No problem, normal prediction
 *			  =  1,	No station information for datum
 *			  =  2,	No travel-time tables for datum
 *			  = 11,	Interpolation point in hole of travel-time curve
 *			  = 12,	Interpolation point less than first distance
 *				point in curve
 *			  = 13,	Interpolation point greater than last 
 *				distance point in curve
 *			  = 14,	Interpolation point less than first depth 
 *				point in curve
 *			  = 15,	Interpolation point greater than last depth 
 *				point in curve
 *			  = 16,	Interpolation point less than first distance 
 *				point in curve and less than first depth 
 *				point in curve
 *			  = 17,	Interpolation point greater than last 
 *				distance point in curve and less than first 
 *				depth point in curve
 *			  = 18,	Interpolation point less than first distance 
 *				point in curve and greater than first depth 
 *				point in curve
 *			  = 19,	Interpolation point greater than last 
 *				distance point in curve and greater than 
 *				first depth point in curve

 *	---- Functions called ----
 *	Local
 *		ttime_calc_: 	Compute travel-times and their partials
 *		slow_calc_:	Compute horizontal slownesses and partials

 *	From libgeog
 *		distaz2:	Determine the distance between between two
 *				lat./lon. pairs
 *		latlon2:	Compute a second lat./lon. from first
 *				distance and an azimuth
 
 * DIAGNOSTICS
 *	Complains when input data are bad ...
 
 * FILES
 *	None.
 
 * NOTES
 *	None.
 
 * SEE ALSO
 *	Bratt and Bache (1988) Locating events with a sparse network of
 *      regional arrays, BSSA, 78, 780-798.
 
 * AUTHORS
 *	Steve Bratt, December 1988.
 *	Walter Nagy, May 1991.
 */


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "aesir.h"
#include "db_origerr.h"
#include "db_origin.h"
#include "db_site.h"

#ifdef SCCSID
static char	SccsId[] = "@(#)predsat.c	44.1	9/20/91	Copyright 1990 Science Applications International Corporation.";
#endif

#define	DEG_TO_KM	111.195
#define	DEG_TO_RAD	0.017453293
#define SIGN(b1, b2)	((b2) >= 0 ? -(b1) : (b1))

void
predsat (sites, num_sta, origin, origerr, data_sta_id, data_phase_type,
	 phase_type, len_phase_type, num_phases, num_data, slow_flag,
	 print_flag, out_file, dist_min, dist_max, dist_center, tt_min,
	 tt_max, tt_center, az_min, az_max, az_center, slow_min, slow_max,
	 slow_center, tt_err_min, tt_err_max, tt_err_center, slow_err_min,
	 slow_err_max, slow_err_center)

Site	*sites;
Origin	*origin;
Origerr	*origerr;

int	len_phase_type, num_data, num_sta, num_phases, print_flag, slow_flag;
char	*data_sta_id, *data_phase_type, *out_file, *phase_type;

int	*slow_err_center, *slow_err_max, *slow_err_min, *tt_err_center,
	*tt_err_max, *tt_err_min;
float	*az_center, *az_max, *az_min, *dist_center, *dist_max, *dist_min;
float	*slow_center, *slow_max, *slow_min, *tt_center, *tt_max, *tt_min;

{

	FILE	*ofp;
	int	i, iserr[3], ista, iterr[3], k, kwav, n, num_times;
	int	lprt[20];
	float	aztt[3], dcalx, dist[3], radius, z[3];
	double	angle, atx[3], az_diff, azim[3], azimuth, az_neg, az_pos;
	double	back_azimuth, distance, e, epj, epn, plat[4], plon[4];
	double	sl_calc[3], sta_lat, sta_lon, tt_calc[3], x, xp, y, yp;
	double	a1, a2, azim1, dist1, epr, epr_add;


	radius		= 6371.0;
	tt_calc[0]	= -999.0;
	tt_calc[1]	=  999.0;
	tt_calc[2]	= -999.0;
	sl_calc[0]	= -999.0;
	sl_calc[1]	=  999.0;
	sl_calc[2]	= -999.0;
	azim[1]		=  999.0;
	azim[2]		= -999.0;
	dist[1]		=  999.0;
	dist[2]		= -999.0;
	*tt_center	= -999.0;
	*slow_center	= -999.0;
	*dist_min	= -999.0;
	*tt_min		= -999.0;
	*az_min		= -999.0;
	*slow_min	= -999.0;
	*dist_max	= -999.0;
	*tt_max		= -999.0;
	*az_max		= -999.0;
	*slow_max	= -999.0;
	az_neg		=  999.0;
	az_pos		= -999.0;
	iterr[0]	= 0;
	iterr[1]	= 0;
	iterr[2]	= 0;
	iserr[0]	= 0;
	iserr[1]	= 0;
	iserr[2]	= 0;

	n  = 0;
	a1 = origin->lat;
	a2 = origin->lon;


	/* Open output file */
 
	ofp = fopen (out_file, "a");

	/*
	 * Check that datum's station and phase are valid. 
	 * If so, assign a pointer!
	 */

	for (i = 0; i < num_sta; i++)
	{
		if (! strcmp (data_sta_id, sites[i].sta))
			goto stafnd;
	}

	fprintf (ofp, "? PredSAT: Station %s unknown\n", data_sta_id);
	iterr[0] = 1;
	iterr[1] = 1;
	iterr[2] = 1;
	goto skip;
stafnd:
	ista = i;

	for (k = 0; k < num_phases; k++)
		if (! strcmp (data_phase_type, phase_type + k*len_phase_type))
			goto wavfnd;
	fprintf (ofp, "? PredSAT: Wave %s unknown\n", data_phase_type);
	iterr[0] = 2;
	iterr[1] = 2;
	iterr[2] = 2;
	goto skip;
wavfnd:
	kwav = k;
 
	/* Compute distance and azimuth between station and event */

	sta_lat = sites[ista].lat;
	sta_lon = sites[ista].lon;
	distaz2_ (&sta_lat, &sta_lon, &a1, &a2, &dist1, &azim1, &back_azimuth);
	*dist_center	= dist1;
	*az_center	= azim1;
	dist[0]		= *dist_center;
	aztt[0]		= *az_center;
	z[0]		= origin->depth;

	/*
	 * If no confidence ellipse,
	 * compute travel-time from station to point
	 */
 
	if (origerr->smajax <= 0.0 || origerr->sminax <= 0.0)
		num_times = 1;
	else
	{
		/*
		 * Find points on confidence ellipse that are the minimum and
		 * maximum distance from the station.  Assume that these points
		 * are at one of the tips of the axes of the confidence ellipse.
		 */

		num_times = 3;
		epj	= origerr->smajax/DEG_TO_KM;
		epn	= origerr->sminax/DEG_TO_KM;
		epr	= origerr->strike;
		latlon2_ (&a1, &a2, &epj, &epr, &plat[0], &plon[0]);
		epr_add = epr + 180.0;
		latlon2_ (&a1, &a2, &epj, &epr_add, &plat[1], &plon[1]);
		epr_add = epr + 90.0;
		latlon2_ (&a1, &a2, &epn, &epr_add, &plat[2], &plon[2]);
		epr_add = epr + 270.0;
		latlon2_ (&a1, &a2, &epn, &epr_add, &plat[3], &plon[3]);

		for (n = 0; n < 4; n++)
		{
			distaz2_ (&sta_lat, &sta_lon, &plat[n], &plon[n],
				  &distance, &azimuth, &back_azimuth);

			/* Find min/max distance with associated azimuth */

			if (distance < dist[1])
			{
				dist[1]	= distance;
				aztt[1]	= azimuth;
			}
			if (distance > dist[2])
			{
				dist[2]	= distance;
				aztt[2]	= azimuth;
			}

			/* Find minimum and maximum azimuth */

			az_diff = azimuth - *az_center;
			if (fabs(az_diff) > 180.0) 
				az_diff = SIGN(360.0-fabs(az_diff), az_diff);
			if (az_diff < az_neg)
			{
				azim[1]	= azimuth;
				az_neg	= az_diff;
			}
			else if (az_diff > az_pos)
			{
				azim[2]	= azimuth;
				az_pos	= az_diff;
			}
		}
 
		/*
		 * If station is within ellipse, set minimum distance to 0.0
		 * and azimuth bounds to 0.0 and 360.0 degrees
		 */

		if (*az_center >= 0.0 && *az_center <= 90.0)
			angle = 90.0 - *az_center;

		else if (*az_center >= 90.0 && *az_center <= 180.0)
			angle = 180.0 + *az_center;

		else if (*az_center >= 180.0 && *az_center <= 270.0)
			angle = *az_center;

		else
			angle = *az_center - 180.0;

		x = *dist_center*DEG_TO_KM*cos(angle*DEG_TO_RAD);
		y = *dist_center*DEG_TO_KM*sin(angle*DEG_TO_RAD);

		/*
		 * Rotate into ellipsoid coordinate system and check 
		 * if station is within ellipse
		 */

		angle	= 90.0 - origerr->strike;
		xp	= x*cos(angle) + y*sin(angle);
		yp	= -x*sin(angle) + y*cos(angle);
		a1	= xp/(origerr->smajax);
		a2	= yp/(origerr->sminax);
		e	= a1*a1 + a2*a2;
		if (e <= 1.0)
		{
			dist[1]	= 0.0;
			azim[1]	= 0.0;
			azim[2]	= 360.0;
		}

		/* 
		 * Minimum time path will be the deepest epicenter for all 
		 * but (a) depth phases, where the shallower depth will 
		 * produce earlier arrivals, and (b) phases from events 
		 * who's epicenter-station distance is less than its depth.
		 */

		if (! strncmp (data_phase_type, "p", 1) || 
		    ! strncmp (data_phase_type, "s", 1) || 
		    (dist[1]*DEG_TO_KM < origin->depth))
		{
			z[1] = origin->depth - origerr->sdepth;
			z[2] = origin->depth + origerr->sdepth;
		}
		else
		{
			z[1] = origin->depth + origerr->sdepth;
			z[2] = origin->depth - origerr->sdepth;
		}
      
		if (z[1] < 0.0)
			z[1] = 0.0;
		if (z[2] < 0.0)
			z[2] = 0.0;
	}

	for (n = 0; n < num_times; n++)
	{
		ttime_calc_ (&kwav, atx, &aztt[n], &dist[n], &radius, &z[n],
			     &dcalx, &iterr[n]);

		/*
		 * Use only sucessfully interpolated and extrapolated results
		 * - Don't use if in a hole (dcalx = -1.0)
		 */

		if (dcalx < 0.0)
			tt_calc[n] = -999.0;
		else
			tt_calc[n] = dcalx;

		if (slow_flag)
			slow_calc_ (&kwav, atx, &aztt[n], &dist[n], &radius, 
				    &z[n], &dcalx, &iserr[n]);

		/*
		 * Use only sucessfully interpolated and extrapolated results
		 * - Don't use if in hole (dcalx = -1.0)
		 */

		if (dcalx < 0.0)
			sl_calc[n] = -999.0;
		else
			sl_calc[n] = dcalx;
	}

	/*
	 * Load array data into arguments for return to calling routine
	 * Sometimes, the most distant point may not have the largest slowness 
	 */

skip:
	if (slow_flag)
	{
		*slow_center	 = sl_calc[0];
		*slow_err_center = iserr[0];
		if (sl_calc[2] < 0.0 || sl_calc[1] < sl_calc[2])
		{
			*slow_min	= sl_calc[1];
			*slow_max	= sl_calc[2];
			*slow_err_min	= iserr[1];
			*slow_err_max	= iserr[2];
		}
		else 
		{
			*slow_min	= sl_calc[2];
			*slow_max	= sl_calc[1];
			*slow_err_min	= iserr[2];
			*slow_err_max	= iserr[1];
		}
	}

	*tt_center	= tt_calc[0];
	*tt_min		= tt_calc[1];
	*tt_max		= tt_calc[2];
	*az_min		= azim[1];
	*az_max		= azim[2];
	*dist_min	= dist[1];
	*dist_max	= dist[2];
	*tt_err_center	= iterr[0];
	*tt_err_min	= iterr[1];
	*tt_err_max	= iterr[2];

	/* Print location results if desired */

	if (print_flag)
	{
		for (i = 0; i < 20; i++)
			lprt[i] = FALSE;

		fprintf (ofp, "== EVENT INFORMATION ============================================\n");
		fprintf (ofp, " Lat:%9.3f deg   Lon:%9.3f deg   Depth:%9.3f km\n", origin->lat, origin->lon, origin->depth);

		fprintf (ofp, "Semi-major axis:%9.1f km   Semi-minor axis:%9.1f km\n", origerr->smajax, origerr->sminax);
         	fprintf (ofp, "Major-axis strike:%7.1f deg clockwise from North\n", origerr->strike);
         	fprintf (ofp, "Depth error:%8.1f km   Orig. time error:%8.1f sec\n", origerr->sdepth, origerr->stime);

		fprintf (ofp, "== PREDICTED TIME/AZIMUTH/SLOWNESS BOUNDS =======================\n");

		fprintf (ofp, " Arrival ID = %6d   Station = %s   Phase = %s\n", num_data, data_sta_id, data_phase_type);

		fprintf (ofp, "Min Cntr Max Azim (deg)    : %12.2f%12.2f%12.2f\n", *az_min, *az_center, *az_max);
		fprintf (ofp, "Min Cntr Max Dist (deg)    : %12.2f%12.2f%12.2f\n", *dist_min, *dist_center, *dist_max);
		fprintf (ofp, "Min Cntr Max Time (sec)    : %12.2f%12.2f%12.2f\n", *tt_min, *tt_center, *tt_max);
		fprintf (ofp, "Min Cntr Max Slow (sec/deg): %12.2f%12.2f%12.2f\n", *slow_min, *slow_center, *slow_max);
		fprintf (ofp, "Time Errors       (sec)    : %12d%12d%12d\n", 
			 *tt_err_min, *tt_err_center, *tt_err_max);
		fprintf (ofp, "Slowness Errors   (sec/deg): %12d%12d%12d\n", 
			 *slow_err_min, *slow_err_center, *slow_err_max);

		if (*tt_err_min > 0)  lprt[*tt_err_min]  = TRUE;
		if (*tt_err_center > 0) lprt[*tt_err_center] = TRUE;
		if (*tt_err_max > 0)  lprt[*tt_err_max]  = TRUE;
		if (*slow_err_min > 0)  lprt[*slow_err_min]  = TRUE;
		if (*slow_err_center > 0) lprt[*slow_err_center] = TRUE;
		if (*slow_err_max > 0)  lprt[*slow_err_max]  = TRUE;

		fprintf (ofp, " =  0, No problem, normal interpolation\n");
		if (lprt[1])  fprintf (ofp, " =  1, No station information\n");
		if (lprt[2])  fprintf (ofp, " =  2, No travel-time tables\n");
		if (lprt[3])  fprintf (ofp, " =  3, Data type unknown\n");
		if (lprt[4])  fprintf (ofp, " =  4, S.D. <= 0.0\n");
		if (lprt[11]) fprintf (ofp, " = 11, Distance-depth point '(x0,z0) in hole of travel-time curve\n");
		if (lprt[12]) fprintf (ofp, " = 12, x0 < x(1)\n");
		if (lprt[13]) fprintf (ofp, " = 13, x0 > x(max)\n");
		if (lprt[14]) fprintf (ofp, " = 14, z0 < z(1)\n");
		if (lprt[15]) fprintf (ofp, " = 15, z0 > z(max)\n");
		if (lprt[16]) fprintf (ofp, " = 16, x0 < x(1) & z0 < z(1)\n");
		if (lprt[17]) fprintf (ofp, " = 17, x0 > x(max) & z0 < z(1)\n");
		if (lprt[18]) fprintf (ofp, " = 18, x0 < x(1) & z0 > z(max)\n");
		if (lprt[19]) fprintf (ofp, " = 19, x0 > x(max) & z0 > z(max)\n");
		fprintf (ofp, "\n");
	}

	fclose (ofp);
}
