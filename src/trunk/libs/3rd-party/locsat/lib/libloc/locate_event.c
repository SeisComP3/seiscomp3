
/*
 * Copyright 1991 Science Applications International Corporation.
 *

 * NAME
 *	locate_event -- Make a single event location

 * FILE
 *	locate_event.c

 * SYNOPSIS
 *	This routine serves as the interface for calculating a single event 
 *	location from the applications: ARS, ESAL, LocSAT, and Locator.

 * DESCRIPTION
 *	Determine a single event location [locate_event()] and read the
 *	travel-time [setup_tttables()] and source-specific station correction
 *	[ssstab()] tables, as necessary.  Also read from the site (station)
 *	[setup_sites()] file on the first call through.   The calculation of
 *	travel-times [ttime_calc()] and slownesses [slow_calc()] along with
 *	their respective derivatives are done here as well.

 *	---- Internal Functions ----

 *	----------------------------------------------------------------
 *	setup_sites:
 *		Reads the site (station) file.

 *	int
 *	setup_sites (new_net, new_sites, new_num_sta)
 *		char	*new_net;
 *		Site	*new_sites;
 *		int	new_num_sta;
 *	----------------------------------------------------------------
 *	setup_tttables:
 *		Reads and initializes the travel-time tables for the locator.
 *		These tables are read from disk each time a change is 
 *		detected in the list of tables being maintained and the 
 *		list of tables input through the argument list.

 *	int
 *	setup_tttables (new_dir, new_phase_types, new_num_phase_types)
 *		char	*new_dir;
 *		char	**new_phase_types;
 *		int	new_num_phase_types;
 *	----------------------------------------------------------------
 *	locate_event:
 *		The main interface to the location procedure (algorithm).

 *	int
 *	locate_event (sites, num_sites, arrival, assoc, origin, origerr,
 *		      locator_params, locator_errors, num_obs)
 *		Site		*sites;
 *		int		num_sites;
 *		Arrival		*arrival;
 *		Assoc		*assoc;
 *		Origin		*origin;
 *		Origerr		*origerr;
 *		Locator_params  *locator_params;
 *		Locator_errors  *locator_errors;
 *		int		num_obs;
 *	----------------------------------------------------------------
 *	ttime_calc_:
 *		Calculate travel-times and their derivatives.

 *	void
 *	ttime_calc_ (phase_id, atx, azi, delta, radius, zfoc, dcalx, iterr)
 *		int	*phase_id;
 *		double	*atx;
 *		float	*azi, *delta, *radius, *zfoc;
 *		int	*iterr;
 *		float	*dcalx;
 *	----------------------------------------------------------------
 *	slow_calc_:
 *		Calculate slownesses and their derivatives.

 *	void
 *	slow_calc_ (phase_id, atx, azi, delta, radius, zfoc, dcalx, iserr)
 *		int	*phase_id;
 *		double	*atx;
 *		float	*azi, *delta, *radius, *zfoc;
 *		int	*iserr;
 *		float	*dcalx;
 *	----------------------------------------------------------------
 *	lstcmp:
 *		Test to see if two lists are identical.

 *	static int
 *	lstcmp (list1, n1, list2, n2)
 *		char ** list1;
 *		int     n1;
 *		char ** list2;
 *		int     n2;
 *	----------------------------------------------------------------
 *	find_ttime_:
 *		Make a crude travel-time determination from a given distance.

 *	void
 *	find_ttime_ (phase_id, delta, tcalc, iterr)
 *		int	*phase_id;
 *		double	*delta;
 *		int	*iterr;
 *		double	*tcalc;
 *	----------------------------------------------------------------
 *	compute_ttime:
 *		Compute a theoretical travel-time.

 *	double
 *	compute_ttime (distance, depth, phase)
 *		double	distance, depth;
 *		char	*phase;
 *	----------------------------------------------------------------

 * DIAGNOSTICS
 *	See golocate_error_table[] character string for global-type errors
 *	and locator_errors structure for local errors (i.e., errors found
 *	in individual datum's).

 * FILES
 *	Indirect access to travel-time tables through function, rdtttab().
 *	All other access is through local and database structures.

 * NOTES
 *	All travel-time access is currently handled internally in this file.

 * SEE ALSO
 *	user_locate() function calls this routine in ARS.
 *	mainloc() main function calls this routine in LocSAT.
 *	gL_locate() function calls this routine in Locator.

 * AUTHORS
 *	?? ??? 19?? (JG)
 *		Created.
 *	02 Feb 1991 (WCN)
 *		Changed parameters and variables to accomodate new locator
 *		interface.
 *	13 Feb 1991 (CKS)
 *		Added code to locate_event() and setup _sites() to accomodate 
 *		above.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "aesir.h"
#include "db_site.h"
#include "db_arrival.h"
#include "db_assoc.h"
#include "db_origin.h"
#include "db_origerr.h"
#include "loc_params.h"
/* #include "sysdefs.h" */
#include "css/trim.h"

/* #include "free_debug.h" */

#include <stdlib.h>

typedef int32_t ftnlen;

#if WIN32
void bzero(char *s, int n) {
	memset(s, 0, n);
}
#endif

#ifdef SCCSID
static char	SccsId[] = "@(#)locate_event.c	44.3	10/31/91 Copyright 1990 Science Applications International Corporation.";
#endif

/* Error Table Values */
#define GLerror1        1
#define GLerror2        2
#define GLerror3        3
#define GLerror4        4
#define GLerror5        5
#define GLerror6        6
#define GLerror7        7
#define GLerror8        8
#define GLerror9        9
#define GLerror10       10
#define GLerror11       11
#define TTerror1	12
#define TTerror2        13
#define TTerror3        14
#define TTerror4	15
#define SSerror1        16

#define	NULL_TIME	-9999999999.999
#define	STRIKE_NULL	-1.0
#define	STRIKE_MIN	0.0
#define	STRIKE_MAX	360.0

#define	MAXTBD		301
#define	MAXTBZ		50
#define	MAX(x,y)	(((x) > (y)) ? (x) : (y))
#define	MIN(x,y)	(((x) < (y)) ? (x) : (y))

#define	VALID_TIME(x)	((x) > -9999999999.000)
#define	VALID_SEAZ(x)	(((x) >= 0.0) && ((x) <= STRIKE_MAX))
#define	VALID_SLOW(x)	((x) >= 0.0)
#define	DEG_TO_RAD	0.017453293

#define	ERROR	1
#define	NOERROR	0

static char	*dir;
static char	*phase_type;
static char	**phase_type_ptr;
static int	num_phase_types;
static int	maxtbd = MAXTBD;
static int	maxtbz = MAXTBZ;
static float	*tbd;
static float	*tbz;
static float	*tbtt;
static int	first_table_read = TRUE;
static int	*ntbd;
static int	*ntbz;
static int	len_dir;
static int len_n_p_t = 9;

static char  *net;
static char  *sta_id;
static int    num_sta;
static char  *cortyp;
static int    sta_cor_level;
static int    len_sta_id;
static float *sta_lat;
static float *sta_lon;
static float *sta_elev;
static float *sta_cor;
static int    first_site_list = TRUE;

int brack_(int *n, float *x, float *x0, int *ileft);
int holint2_(int *phase_id,
             int *do_extrapolate,
             int *nd, /* Number of distance samples */
             int *nz, /* Number of depth samples */
             float *x, float *y, float *func, int *ldf,
             float *fbad, float *x0, float *y0, float *f0,
             float *fx0, float *fy0, float *fxy0,
             int *idist, int *idepth, int *ihole);
void rdtttab(char *froot, /* Size [ca. 1024] */
             char **phase_type_ptr, /* Size [nwav][8] */
             int nwav,    /* Array lengths */
             int maxtbd,  /* Array lengths */
             int maxtbz,  /* Array lengths */
             int *ntbd,   /* Array lengths */
             int *ntbz,   /* Array lengths */
             float *tbd,  /* Size [nwav][maxtbd] */
             float *tbz,  /* Size [nwav][maxtbz] */
             float *tbtt, /* Size [nwav][maxtbz][maxtbd] */
             int *ierr,   /* Error flag */
             int verbose);/* Verbose mode */
int rdcortab_(char *froot, char *cortyp,
              int *ntype, char *staid, char *wavid,
              int *nsta, int *nwav, int *ierr,
              ftnlen froot_len,
              ftnlen cortyp_len,
              ftnlen staid_len,
              ftnlen wavid_len);
int locsat0_(char *dstaid, char *dwavid, char *dtype, char *atype,
             float *dobs, float *dsd,
             int *idarid, int *ndata,
             char *staid, float *stalat, float *stalon, float *stelev, float *stacor,
             int *nsta, char *wavid, int *nwav, int *maxtbd, int *maxtbz,
             int *ntbd, int *ntbz, float *tbd, float *tbz, float *tbtt,
             float *alat0, float *alon0, float *zfoc0, float *sig0,
             int *ndf0, float *pconf, float *azwt, float *damp,
             int *maxit, char *prtflg, char *fxdflg, char *outfile,
             int *luout, float *alat, float *alon, float *zfoc, float *torg,
             float *sighat, float *snssd, int *ndf,
             float *epmaj, float *epmin, float *epstr, float *zfint,
             float *toint, float *sxx, float *syy, float *szz, float *stt, float *sxy,
             float *sxz, float *syz, float *stx, float *sty, float *stz,
             float *stadel, float *staazi, float *stabaz, float *epimp,
             float *zfimp, float *resid,
             int *ipsta, int *iderr, int *niter, int *ierr,
             ftnlen dstaid_len, ftnlen dwavid_len, ftnlen dtype_len,
             ftnlen atype_len, ftnlen staid_len, ftnlen wavid_len,
             ftnlen prtflg_len, ftnlen fxdflg_len, ftnlen outfile_len);

static const char *default_phases[] = {
	"LQ", "LR", "Lg", "P", "PKP", "PP",
	"PKPab", "PKPbc", "PKPdf", "SKPdf",
	"PcP", "Pg", "Pn", "Rg", "S", "SKS",
	"SS", "ScS", "Sn", "Sg", "pP", "sP",
	"Pb", "Sb", "Is", "It"
};


/*       Returns TRUE   if two lists are identical;
 *               FALSE  if two lists are not the same;
 *       The first list can be blank padded.
 */
static int lstcmp(const char **list1, int n1, const char **list2, int n2) {
	int found = FALSE, i, j;

	if ( n1 != n2 )
		return FALSE;

	for ( i = 0; i < n1; ++i ) {
		found = FALSE;
		for ( j = 0; j < n2; ++j ) {
			if ( !strncmp (list1[i], list2[j], strlen(list2[j])) ) {
				found = TRUE;
				break;
			}
		}
		if ( !found )
			break;
	}

	return found;
}


int setup_sites(char *new_net, Site *new_sites, int new_num_sta) {
	int  i;
	char *dummy_ptr;

	if ( !first_site_list ) {
		if ((new_net != (char *)NULL) && (net != (char *)NULL))
			if (STREQ(new_net, net))
				return (NOERROR);

		UFREE(net);
		UFREE(sta_id);
		UFREE(sta_lat);
		UFREE(sta_lon);
		UFREE(sta_cor);
		UFREE(sta_elev);
	}

	if (new_net != (char *)NULL)
		net = STRALLOC(new_net);

	if ( new_num_sta == 0 || ! new_sites ) {
		fprintf (stderr,"Error setup_sites: Null station list");
		return (SSerror1);
	}

	/*
	 *   Get the size of the station designator and allocate memory
	 *   for the station list
	 */

	first_site_list = FALSE;
	num_sta = new_num_sta;
	len_sta_id = sizeof(new_sites->sta);
	sta_id   = (char *)malloc((unsigned)num_sta * len_sta_id*sizeof(char));
	sta_lat  = UALLOC(float, num_sta);
	sta_lon  = UALLOC(float, num_sta);
	sta_cor  = UALLOC(float, num_sta);
	sta_elev = UALLOC(float, num_sta);

	for (i = 0; i < num_sta; i++)
	{
		dummy_ptr = sta_id + i*len_sta_id;
		strcpy (dummy_ptr, new_sites[i].sta);
		FPAD(dummy_ptr, len_sta_id);
		sta_lat[i]  = new_sites[i].lat;
		sta_lon[i]  = new_sites[i].lon;
		sta_elev[i] = new_sites[i].elev;
		sta_cor[i]  = 0.0;			/* FIX !!!!!!*/
	}
	return (NOERROR);
}


static int
setup_tttables(const char *new_dir, const char **new_phase_types,
               int new_num_phase_types, int verbose) {
	int i, ierr, num_type;
	char *dummy_ptr;
	int malloc_err = 0;

	/* First determine whether it is necessary to read in new tables */

	if (new_num_phase_types == 0 || ! new_phase_types)
	{
		fprintf (stderr,"Error setup_tttables: Null phase_type list");
		return (TTerror1);
	}

	if ( !first_table_read )  {
		if ( STREQ(new_dir, dir) && lstcmp((const char**)phase_type_ptr,
		     num_phase_types, new_phase_types, new_num_phase_types) )
			return (NOERROR);

		/* Free up previous space, and then, read new tables */

		UFREE(phase_type);
		UFREE(phase_type_ptr);
		UFREE(dir);
		UFREE(tbd);
		UFREE(tbz);
		UFREE(tbtt);
		UFREE(ntbd);
		UFREE(ntbz);
	}

	first_table_read = FALSE;
	dir = STRALLOC(new_dir);	/* Grab the dir to keep */
	len_dir = strlen(dir);
	num_phase_types = new_num_phase_types;	/* How many wave types */

	/* Grab all of the wave id's */

	phase_type = (char *)malloc((unsigned)(num_phase_types * len_n_p_t) * (sizeof(char)));
	phase_type_ptr = (char **)malloc ((unsigned) num_phase_types * sizeof(char *));
	bzero((char *)phase_type, (num_phase_types*len_n_p_t) * (sizeof(char)));
	bzero((char *)phase_type_ptr, num_phase_types * sizeof(char *));
	
	for (i = 0; i < num_phase_types; i++)
	{
		dummy_ptr = phase_type + i*len_n_p_t;
		strcpy (dummy_ptr, new_phase_types[i]);
		phase_type_ptr[i] = dummy_ptr;
	}

	malloc_err = 0;
	ntbd = ntbz = (int *)NULL;
	tbd = tbz = tbtt = (float *)NULL;

	if (!(ntbd = UALLOC(int, num_phase_types)))
		malloc_err++;

	else if (! (ntbz = UALLOC(int, num_phase_types)))
		malloc_err++;

	else if (! (tbd = UALLOC(float, maxtbd*num_phase_types)))
		malloc_err++;

	else if (! (tbz = UALLOC(float, maxtbz*num_phase_types)))
		malloc_err++;

	else if (! (tbtt = UALLOC(float, maxtbd*maxtbz*num_phase_types)))
		malloc_err++;

	if (malloc_err)
	{
		printf ("Insufficient memory for travel-time tables (file locate_event.c)\n");
		UFREE(ntbd);
		UFREE(ntbz);
		UFREE(tbd);
		UFREE(tbz);
		UFREE(tbtt);
		return (ERROR);
	}
	
	/* Read the travel-time tables */
	
	rdtttab(dir, phase_type_ptr, num_phase_types, maxtbd, maxtbz,
	        ntbd, ntbz, tbd, tbz, tbtt, &ierr, verbose);

	if (ierr == 0)
	{
		if (sta_cor_level > 0)
		{
			num_type = 1;
			cortyp = (char *) malloc((unsigned) num_type * 9 
					  * sizeof(char));
			dummy_ptr = cortyp + (num_type-1)*9;
			strcpy (dummy_ptr, "TT");
			FPAD(dummy_ptr, 9);
			rdcortab_ (dir, cortyp, &num_type, sta_id, phase_type, 
				   &num_sta, &num_phase_types, &ierr, len_dir,
				   2, len_sta_id, len_n_p_t);
			if (ierr > 1)
				fprintf (stdout, "Problems with sta. corr. tables\n");
		}
		return (NOERROR);
	}

	else
	{
		UFREE(phase_type);
		UFREE(phase_type_ptr);
		UFREE(dir);
		UFREE(tbd);
		UFREE(tbz);
		UFREE(tbtt);
		UFREE(ntbd);
		UFREE(ntbz);
		first_table_read = TRUE;

		if (ierr == 1)
		{
			fprintf (stderr, "setup_tttables: Error opening travel-time tables");
			return (TTerror2);
		}
		else if (ierr == 2)
		{
			fprintf (stderr, "setup_tttables: Error reading travel-time tables: Unexpected E-O-F");
			return (TTerror3);
		}
		else
		{
			fprintf (stderr, "setup_tttables: Unknown error reading travel-time tables");
			return (TTerror4);
		}
	}
}


int setup_tttables_dir(const char *new_dir, int verbose) {
	return setup_tttables(new_dir, default_phases,
	                      (sizeof default_phases) / sizeof(const char*), verbose);
}


int locate_event(char *network, Site *sites, int num_sites, Arrival *arrival,
                 Assoc *assoc, Origin *origin, Origerr *origerr,
                 Locator_params *locator_params, Locator_errors *locator_errors,
                 int num_obs) {

	int   loc_err;
	char  *new_dir;

	char	*user;
	char	*data_sta_id, *data_phase_type, *data_type, *data_defining;
	int	len_d_s_i = sizeof(arrival->sta);
	int	len_d_p_t = sizeof(assoc->phase);
	int	len_d_d	  = sizeof(assoc->timedef);
	int	len_d_t  = 4;
	int	num_data;
	float	*obs_data, *data_std_err;
	int	*data_arrival_id_index;

	float	lat, lon, depth, torg;
	float	semi_major_axis, semi_minor_axis, strike;
	float	sxx, syy, szz, stt;
	float	sxy, sxz, syz;
	float	stx, sty, stz;
	float	sighat, snssd;
	int	ndf;
	float	depth_error, origin_time_error;
	float	*data_importances, *zfimp;
	float	*data_residual;
	float	*sta_delta, *sta_azimuth, *sta_back_azimuth;
	int	*sta_index, *data_err_code;
	int	niter;
	int	ierr;

	float	lat_init, lon_init, depth_init, fixing_depth;
	float	est_std_error;
	int	num_dof;
	float	conf_level;
	float	azimuth_wt;
	float	damp;
	int	max_iterations;
	int	luout;
	char	fix_depth, verbose;
	char	*outfile_name;

	int	*obs_data_index;
	int	i, j, k;
	int	max_data;
	char	*dummy_ptr;
	double	time_offset;
	int     error_found = FALSE;


	/* Read site (station) information */

	if ((loc_err = setup_sites (network, sites, num_sites)) != 0)
	{
		fprintf (stderr,"locate_event: Aborting location process");
		return (loc_err);
	}

	/* Check if travel-time tables have been read */

	/*
	if (first_table_read)
	{
	*/
		new_dir		    = locator_params->prefix;
		sta_cor_level	    = locator_params->cor_level;
		
		if ((loc_err = setup_tttables_dir (new_dir, locator_params->verbose == 'y')) != 0)
			return (loc_err);
	/*
	}
	*/

	/* Check input */

	if ((num_obs == 0) || (! arrival))
	{
		/* It is not an error to simply initialize */ 
		fprintf (stderr,"Warning locate_event: No observations to process");
		return (GLerror7);
	}
	if (! assoc)
	{
		fprintf (stderr, "Error locate_event: Bad assoc data");
		return (GLerror8);
	}
	else if (! origin)
	{
		fprintf (stderr, "Error locate_event: Bad origin pointer");
		return (GLerror9);
	}
	else if (! origerr)
	{
		fprintf (stderr, "Error locate_event: Bad origerr pointer");
		return (GLerror10);
	}

	/* Check alignment of arrival/assoc pointers */

	for (i = 0; i < num_obs; i++)
	{
		if (arrival[i].arid != assoc[i].arid)
		{
			fprintf (stderr, 
				 "Error locate_event: Mismatch between arrival/assoc.\n");
			return (GLerror11);
		}
	}
	
	
	lat_init	= locator_params->lat_init;
	lon_init	= locator_params->lon_init;
	depth_init	= locator_params->depth_init;
	est_std_error	= locator_params->est_std_error;
	num_dof		= locator_params->num_dof;
	conf_level	= locator_params->conf_level;
	damp		= locator_params->damp;
	max_iterations	= locator_params->max_iterations;
	fix_depth	= locator_params->fix_depth;
	verbose		= locator_params->verbose;
	outfile_name	= locator_params->outfile_name;
	fixing_depth    = locator_params->fixing_depth;

	/* 
	 * kludge to get output to print to a file, if the file exists
	 * the output will be appended to the end; this is handle in
	 * locsat0
	 */

	if (STREQ(locator_params->outfile_name, ""))
		luout = 6;
	else
		luout = 17;
	
	/*
	 * Use current lat/lon as initial guess.  Depth is set in
	 * get_locator_variables().
	 */

	lat_init = origin->lat;
	lon_init = origin->lon;

	/*
	 * If the use location flag is zero than set the lat and lon
	 * init values to something locsat0 knows to ignore
	 */

	if (!(locator_params->use_location))
	{
		lon_init = -999.0;
		lat_init = -999.0;
	}
	
	if (fix_depth == 'y')
		depth_init = fixing_depth;
	
	/* Allocate a bunch of space for data, */

	max_data	= 3*num_obs;
	data_phase_type	= (char *) malloc(len_d_p_t * max_data * sizeof(char));
	data_sta_id	= (char *) malloc(len_d_s_i * max_data * sizeof(char));
	data_type	= (char *) malloc(len_d_t * max_data * sizeof(char));
	data_defining	= (char *) malloc(len_d_d * max_data * sizeof(char));

	obs_data		= UALLOC(float, max_data);
	data_std_err		= UALLOC(float, max_data);
	data_arrival_id_index	= UALLOC(int, max_data);
	obs_data_index		= UALLOC(int, max_data);

	/* and some more for stuff returned for locsat0 */

	sta_delta		= UALLOC(float, num_sta);
	sta_azimuth		= UALLOC(float, num_sta);
	sta_back_azimuth	= UALLOC(float, num_sta);
	data_importances	= UALLOC(float, max_data);
	zfimp			= UALLOC(float, max_data);
	data_residual		= UALLOC(float, max_data);
	sta_index		= UALLOC(int, max_data);
	data_err_code		= UALLOC(int, max_data);
	
	for (i = 0; (! VALID_TIME(arrival[i].time)) && i < num_obs; i++);

	if (i < num_obs)				/* Find offset time */
		time_offset = arrival[i].time;
	else
		time_offset = 0.0;

	for (i = 0; i < num_obs; i++)
	{
		if (VALID_TIME(arrival[i].time))
			time_offset = MIN(time_offset,arrival[i].time);
	}

	/* Extract the observations from the arrival/assoc structures
         * Assume that these are stored in same order in both structures */
	
	for (i = 0, num_data = 0; i < num_obs; i++)
	{
		{
			data_arrival_id_index[num_data] = arrival[i].arid;

			dummy_ptr = data_sta_id + num_data*len_sta_id;
			strcpy (dummy_ptr, arrival[i].sta);
			FPAD(dummy_ptr, len_sta_id);
			
			dummy_ptr = data_type + num_data*len_d_t;
			strcpy (dummy_ptr, "t");
			FPAD(dummy_ptr, len_d_t);
			
			dummy_ptr = data_defining +  num_data*len_d_d;
			strcpy (dummy_ptr, assoc[i].timedef);
			FPAD(dummy_ptr, len_d_d);

			dummy_ptr = data_phase_type + num_data*len_d_p_t;
			strcpy (dummy_ptr, assoc[i].phase); 
			FPAD(dummy_ptr, len_d_p_t);

			obs_data[num_data] = 
				(float)(arrival[i].time - time_offset);
			data_std_err[num_data]  = arrival[i].deltim;
			obs_data_index[num_data] = i;
			num_data++;
		}
		if (VALID_SEAZ(arrival[i].azimuth))
		{
			data_arrival_id_index[num_data] = arrival[i].arid;

			dummy_ptr = data_sta_id + num_data*len_sta_id;
			strcpy (dummy_ptr, arrival[i].sta);
			FPAD(dummy_ptr, len_sta_id);
			
			dummy_ptr = data_type + num_data*len_d_t;
			strcpy (dummy_ptr, "a");
			FPAD(dummy_ptr, len_d_t);
			
			dummy_ptr = data_defining +  num_data*len_d_d;
			strcpy (dummy_ptr, assoc[i].azdef);
			FPAD(dummy_ptr, len_d_d);

			dummy_ptr = data_phase_type + num_data*len_d_p_t;
			strcpy (dummy_ptr, assoc[i].phase);
			FPAD(dummy_ptr, len_d_p_t);

			obs_data[num_data] = arrival[i].azimuth;
			data_std_err[num_data]  = arrival[i].delaz;
			obs_data_index[num_data] = i;
			num_data++;
		}
		if (VALID_SLOW(arrival[i].slow))
		{
			data_arrival_id_index[num_data] = arrival[i].arid;

			dummy_ptr = data_sta_id + num_data*len_sta_id;
			strcpy (dummy_ptr, arrival[i].sta);
			FPAD(dummy_ptr, len_sta_id);
			
			dummy_ptr = data_type + num_data*len_d_t;
			strcpy (dummy_ptr, "s");
			FPAD(dummy_ptr, len_d_t);
			
			dummy_ptr = data_defining +  num_data*len_d_d;
			strcpy (dummy_ptr, assoc[i].slodef);

			dummy_ptr = data_phase_type + num_data*len_d_p_t;
			strcpy (dummy_ptr, assoc[i].phase);
			FPAD(dummy_ptr, len_d_p_t);

			obs_data[num_data] = arrival[i].slow;
		        data_std_err[num_data]  = arrival[i].delslo;
			obs_data_index[num_data] = i;
			num_data++;
		}
	}		

	/* call locsat0 */

	locsat0_(data_sta_id, data_phase_type, data_type, data_defining, 
		 obs_data, data_std_err, data_arrival_id_index, &num_data,
		 sta_id, sta_lat, sta_lon, sta_elev, sta_cor, &num_sta, 
		 phase_type, &num_phase_types, &maxtbd, &maxtbz, ntbd, ntbz, 
		 tbd, tbz, tbtt, &lat_init, &lon_init, &depth_init, 
		 &est_std_error, &num_dof, &conf_level, &azimuth_wt, &damp, 
		 &max_iterations, &verbose, &fix_depth, outfile_name, &luout, 
		 &lat, &lon, &depth, &torg, &sighat, &snssd, &ndf, 
		 &semi_major_axis, &semi_minor_axis, &strike, &depth_error, 
		 &origin_time_error, &sxx, &syy, &szz, &stt, &sxy, &sxz, &syz,
		 &stx, &sty, &stz, sta_delta, sta_azimuth, sta_back_azimuth, 
		 data_importances, zfimp, data_residual, sta_index, 
		 data_err_code, &niter, &ierr, len_d_s_i, len_d_p_t, 
		 len_d_t, len_d_d, len_sta_id, len_n_p_t, 1, 1, 
		 strlen(outfile_name));
	
	/* Check the return codes from locsat */

// 	if (ierr == 6)
// 		fprintf (stderr,
// 		"locate_event: Error 6 from locsat0: SVD routine can't decompose matrix\n");
// 
// 	else if (ierr == 5)
// 		fprintf (stderr,
//                 "locate_event: Error 5 from locsat0: Insufficient data for a solution\n");
// 
// 	else if (ierr == 4)
// 		fprintf (stderr,
//                 "locate_event: Error 4 from locsat0: Too few data to constrain O.T.\n");
// 
// 	else if (ierr == 3)
// 		fprintf (stderr,
//                 "locate_event: Error 3 from locsat0: Too few usable data\n");
// 
// 	else if (ierr == 2)
// 		fprintf (stderr,
//                 "locate_event: Error 2 from locsat0: Solution did not converge\n");
// 
// 	else if (ierr == 1)
// 		fprintf (stderr,
//                 "locate_event: Error 1 from locsat0: Exceeded maximum iterations\n");
// 
// 	/* Fill back in the structures */
// 
// 	else
// 
	if (ierr == 0 || ierr == 4)
	{
		/* Fill in origin/origerr structure */
		origin->lat   = lat;
		origin->lon   = lon;
		origin->depth = depth;

		/* Stuff "Locator:<username>" into author field */

		strncpy (origin->auth, "Locator:", sizeof(origin->auth) - 1);
		user = getenv ("USER");

		if (user)
			strncat(origin->auth, user,
				sizeof(origin->auth) -
				strlen(origin->auth) - 1);

		/* Replace whitespace in author name with "_" */

		for (i = 0; i < strlen(origin->auth); i++)
			if (isspace(origin->auth[i]))
				origin->auth[i] = '_';
		
		/* Compute geographic and seismic region numbers. */

		//origin->grn	= nmreg(lat, lon);
		//origin->srn	= gtos(origin->grn);
		
		origerr->sdobs	= sighat;
		origerr->sxx	= sxx;
		origerr->syy	= syy;
		origerr->szz	= szz;
		origerr->sxy	= sxy;
		origerr->sxz	= sxz;
		origerr->syz	= syz;
		origerr->stt	= stt;
		origerr->stx	= stx;
		origerr->sty	= sty;
		origerr->stz	= stz;
		origerr->smajax = semi_major_axis;
		origerr->sminax = semi_minor_axis;
		origerr->stime	= origin_time_error;
		origerr->sdepth	= depth_error;

		if (strike < STRIKE_MIN && strike != STRIKE_NULL)
			while (strike < STRIKE_MIN)
				strike += 180;

		origerr->strike = strike;
		
		/* Special case for unconstained origin time */

		if (ierr == 4)
		{
			fprintf (stderr,
			"locate_event: Error 4 from locsta0: Unconstrained origin time");
			origin->time	= (double)NULL_TIME;
			origerr->stt	= stt;
			origerr->stx	= stx;
			origerr->sty	= sty;
			origerr->stz	= stz;
		}
		else if (ierr == 0)
			origin->time = (double)torg + time_offset;
		/*
		 * obs_data_index is an array of indexes to the assoc
		 * tuple with a maximum length of 3 (time,slow,az) *
		 * the number of assoc records.  data_err_code is an array,
		 * parallel with obs_data_index, returned by locsat0, 
		 * which contains error numbers corresponding to an
		 * error_table in the Locator GUI.  data_type is a string
		 * of maximum length 3 * number of assocs (with blank
		 * spaces between each letter) indicating the data
		 * type, t-time, a-az, and s-slow.  data_type and
		 * obs_data_index are matched pairs.  The locator_error
		 * structure is allocated and initialized to zero
		 * in the Locator GUI.  If there is an error code in
		 * data_err_code the appropriate type (time,slow,az) receives
		 * the error code and the arid is assigned to the
		 * arid field of locator_errors.  Otherwise the values
		 * remain 0.  Later the Locator GUI prints out errors
		 * corresponding to the error type for each data type.
		 */

		for (i = 0; i < num_data; i++)
		{
			j = obs_data_index[i];
			
			if (! strncmp (data_type + i*len_d_t, "t", 1))
			{
				if (ierr != 4)
				{
					assoc[j].timeres = data_residual[i];
					locator_errors[j].time = data_err_code[i];
					if (data_err_code[i] != 0)
					{
						error_found = TRUE;
						strcpy(assoc[j].timedef, "n");
					}
				}
				assoc[j].azres	= -999.0;
				assoc[j].slores	= -999.0;
			}
			
			else if (! strncmp (data_type + i*len_d_t, "a", 1))
			{
				assoc[j].azres = data_residual[i];
				locator_errors[j].az = data_err_code[i];
       				if (data_err_code[i] != 0)
       				{
       					error_found = TRUE;
					strcpy(assoc[j].azdef, "n");
       				}
			}

			else if (! strncmp (data_type + i*len_d_t, "s", 1))
			{
				assoc[j].slores = data_residual[i];
				locator_errors[j].slow = data_err_code[i];
       				if (data_err_code[i] != 0)
       				{
       					error_found = TRUE;
					strcpy(assoc[j].slodef, "n");
       				}
			}
			/* If there's been an error fill-in the arid */
			if (error_found)
			{
				locator_errors[j].arid = assoc[j].arid;
				error_found = FALSE;
			}
			
		}

		/* Try (!!) to get the del's, azi's, and baz's */

		for (i = 0; i < num_data; i++)
		{
			j		= obs_data_index[i];
			k		= sta_index[i] - 1;
			assoc[j].delta	= sta_delta[k];
			assoc[j].seaz	= sta_back_azimuth[k];
			assoc[j].esaz	= sta_azimuth[k];
		}

		/* Count number of defining phases */

		for (i = 0, j = 0; i < num_obs; i++)
		{
			if (! strncmp (assoc[i].timedef,"d", 1))
				j++;

		}
		origin->ndef = j;
		origin->nass = i;
	}



	UFREE(data_phase_type);
	UFREE(data_sta_id);
	UFREE(data_type);
	UFREE(data_defining);
	UFREE(obs_data);
	UFREE(data_std_err);
	UFREE(data_arrival_id_index);
	UFREE(obs_data_index);
	UFREE(sta_delta);
	UFREE(sta_azimuth);
	UFREE(sta_back_azimuth);
	UFREE(data_importances);
	UFREE(zfimp);
	UFREE(data_residual);
	UFREE(sta_index);
	UFREE(data_err_code);

	return (ierr);
}



int num_phases() {
	return num_phase_types;
}


char **phase_types() {
	return phase_type_ptr;
}


int find_phase(const char *phase) {
	int i;

	if (!phase || !*phase)
		return ERR;

	if (len_n_p_t <= 0 || strlen(phase) >= len_n_p_t)
		return ERR;

	for (i = 0; i < num_phase_types; i++)
		if (phase_type_ptr[i] && 
		    !strncmp (phase, phase_type_ptr[i], len_n_p_t))
			break;

	if (i < num_phase_types)
		return i;
	else
		return ERR;
}


double compute_ttime(double distance, double depth, char *phase, int extrapolate, double *rdtdel, int *errorflag) {
	int ileft, jz, nz;
	float zfoc = depth;
	float bad_sample = -1.0;
	float tcal; /* Travel time to return */
	float delta, dtdel, dtdz, dcross;
	int iext, jext, ibad; /* Errors from holint2_ */
	int phase_id;

	phase_id = find_phase(phase);
	if (phase_id < 0)
		return -1.0;

	if (ntbz[phase_id] <= 0)
		return -1.0;

	delta = distance;

	brack_(&ntbz[phase_id], &tbz[phase_id * maxtbz], &zfoc, &ileft);

	jz = ((ileft - 1) > 1) ? (ileft - 1) : 1;
	nz = (((ileft + 2) < ntbz[phase_id]) ? (ileft + 2) : ntbz[phase_id]) - jz + 1;

	/*
	 * Indexing into two and three dimensional arrays:
	 *
	 * &tbz[phase_id][jz - 1] == &tbz[phase_id *maxtbz + jz - 1]
	 *
	 * &tbtt[phase_id][jz - 1][0] == 
	 * &tbtt[(phase_id * maxtbz * maxtbd) + ((jz -1) * maxtbd)+0]
	 *
	 */

	holint2_(&phase_id, &extrapolate, &ntbd[phase_id], &nz,
	         &tbd[phase_id * maxtbd], &tbz[phase_id * maxtbz + jz - 1],
	         &tbtt[(phase_id * maxtbz * maxtbd) + ((jz - 1) * maxtbd)],
	         &maxtbd, &bad_sample, &delta, &zfoc, &tcal, &dtdel, &dtdz,
	         &dcross, &iext, &jext, &ibad);

	*errorflag = 0;
	if (iext != 0 || jext != 0 || ibad != 0)
		*errorflag = 1;

	if (rdtdel)
		*rdtdel = dtdel;

	return (tcal);
}

