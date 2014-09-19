
/*
 *	Copyright 1991 Science Applications International Corporation.

 * NAME		
 *	loc_params -- Event location specific structure.

 * FILE
 *	loc_params.h

 * SYNOPSIS
 *	Structure interface between the Locator GUI, LocSAT, ARS and ESAL 
 *	to the locate_event() function interface.

 * DESCRIPTION
 *	Above.

 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	None.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	31 Jan 1991	Cynde K. Smith	Created
 *	 7 Jun 1991	Walt Nagy	Augmented prologue.
 */

/*
static char SccsId[] = "@(#)loc_params.h	43.1	9/9/91 loc_params.h  Copyright 1991 Science Applications International Corporation";
*/


#ifndef LOC_PARAMS_H
#define LOC_PARAMS_H


/*
 * The Locator_params structure will be used to pass parameter values from
 * the Locator GUI, LocSAT, ARS and ESAL to locate_event().  locate_event() 
 * will then use these parameters for it's call to the locatation algorithm,
 * LocSAT0.
 */

typedef struct locator_params 
{
				/* DEFAULT - DESCRIPTION                     */
	int	num_dof;	/* 9999    - number of degrees of freedom    */
	float	est_std_error;	/* 1.0     - estimate of data std error      */
	float	conf_level;	/* 0.9     - confidence level    	     */
	float	damp;		/* -1.0    - damping (-1.0 means no damping) */
	int	max_iterations;	/* 20      - limit iterations to convergence */
	char	fix_depth;	/* true    - use fixed depth ?               */
	float	fixing_depth;	/* 0.0     - fixing depth value              */
	float	lat_init;	/* modifiable - initial latitude             */
	float	lon_init;	/* modifiable - initial longitude            */
	float	depth_init;	/* modifiable - initial depth                */
	int	use_location;	/* true    - use current origin data ?       */
	char	verbose;	/* true    - verbose output of data ?        */
	int	cor_level;	/* 0       - correction table level          */
	char *	outfile_name;	/* NULL    - name of file to print data      */
	char *	prefix;		/* NULL    - dir name & prefix of tt tables  */
} Locator_params;


/*
 * The Locator_errors structure will be used to pass data from locate_event()
 * to the Locator GUI, LocSAT, ARS and ESAL.  The applications will then use 
 * the values to report errors that may have occurred during the location 
 * calculation called by locate_event() and performed by location algoritm.
 */

typedef struct locator_errors
{
	int	arid;
	int	time;
	int	az;
	int	slow;
} Locator_errors;

	
#endif /* LOCATOR_PARAMS_H */

