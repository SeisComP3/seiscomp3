
/*
 * NAME
 *	denuis_ -- Denuisance matrix and data.

 * FILE
 *	denuis_.c

 * SYNOPSIS
 *	Remove variance-weighted mean from travel-time data and system
 *	matrix.

 * DESCRIPTION
 *	Function.  Denuisance data and system matrix with respect to a 
 *	baseline parameter.  This boils downs to removing a variance-weighted 
 *	mean from the travel-time data and from each column of the travel-time 
 *	system (derivative; sensitivity) matrix.

 *	---- Indexing ----
 *	i = 1, nd;	j = 1, np;

 *	On entry ----
 *	nd:		Number of data
 *	np:		Number of parameters
 *	idtyp[i]:	Data type code for i'th observation
 *			= 0, Data type unknown
 *			= 1, Arrival time datum
 *			= 2, Azimuth datum
 *			= 3, Slowness datum
 *	dsd[i]:		Data standard deviations
 *	resid[i]:	Data vector
 *	at[j][i]:	Transpose of system matrix (derivative of i'th 
 *			datum w.r.t. the j'th parameter).

 *	---- On return ----
 *	resid[i]:	Data, only arrival times are denuisanced
 *	at[j][i]:	Transpose of system matrix (only travel-time 
 *			partials are denuisanced)
 *	dmean:		Weighted mean of travel-time data (pre-denuisanced)
 *	inerr:		  = 0, OK
 *			  = 1, No valid arrival-time data, asum = 0

 * DIAGNOSTICS
 *	If no valid arrival-time information is found, set error flag.

 * NOTES
 *

 * SEE ALSO
 *

 * AUTHORS
 *	Steve Bratt & Walter Nagy.
 */


#ifdef SCCSID
static	char	SccsId[] = "@(#)denuis_.c	44.1	9/20/91";
#endif

#define	MAXPARM	4

void denuis_ (idtyp, nd, np, resid, dsd, at, dmean, inerr)
 
int	*nd, *np, idtyp[];
double	dsd[];
int	*inerr;
double	at[][MAXPARM], *dmean, resid[];

{
	int	m, n;
	double	amean[MAXPARM], asum, dacc;

	*inerr = 0;
	*dmean = 0.0;
	asum   = 0.0;
	for (m = 0; m < *np; m++)
		amean[m] = 0.0;

	for (n = 0; n < *nd; n++)
	{					/* Compute weighted sums */
		dacc = 1.0/(dsd[n]*dsd[n]);
		if (idtyp[n] == 1)
		{
			for (m = 1; m < *np; m++)
				amean[m] = amean[m] + dacc*at[n][m];
			asum   = asum  + dacc;
			*dmean = *dmean + dacc*resid[n];
		}
	}
 
	/*  Convert amean and dmean to weighted means if valid arrival-time
	 *  data is available.  Also de-mean data and rows of at[].  
	 *  amean[m] is the weighted mean of m'th column of travel-time 
	 *  system matrix (before denuisancing).
	 */
 
	if (asum > 0.0)
	{
		for (m = 1; m < *np; m++)
			amean[m] = amean[m]/asum;
		*dmean = *dmean/asum;

		for (n = 0; n < *nd; n++)
			if (idtyp[n] == 1)
			{
				for (m = 1; m < *np; m++)
					at[n][m] = at[n][m] - amean[m];
				resid[n] = resid[n] - *dmean;
			}
	}
	else
		*inerr = 1;
}

