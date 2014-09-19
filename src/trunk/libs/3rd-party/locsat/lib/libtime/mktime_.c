#ifndef	lint
static	char	SccsId[] = "@(#)mktime_.c	44.1	9/23/91";
#endif

#include	"pfile.h"

/* calling sequence:

	integer yr,doy,mon,dom,hr,min,sec
	real fract
	real*8 time
	real*8 mktime 

	time = mktime(yr,doy,mon,dom,hr,min,sec,fract)
*/

double mktime_(yr,doy,mon,dom,hr,min,sec,fract)
	int *yr;	/* year */
	int *doy;	/* day of year */
	int *mon;	/* month */
	int *dom;	/* day of month */
	int *hr;	/* hour */
	int *min;	/* minutes */
	int *sec; 	/* seconds */
	float *fract;	/* fraction of second */
{
	intime_t intime;	/* declare structure for results */

	intime.tyr = *yr;	/* transfer arguments into the structure */
	intime.tdoy = *doy;	/*		the C routine wants. 	 */
	intime.tmon = *mon;
	intime.tdom = *dom;
	intime.thr = *hr;
	intime.tmin = *min;
	intime.tsec = *sec;
	intime.tfract = *fract;

	return (maketime (intime));
}
