#ifndef	lint
static	char	SccsId[] = "@(#)untime_.c	44.1	9/23/91";
#endif

#include	"pfile.h"

/* calling sequence:

	integer yr,doy,mon,dom,hr,min,sec
	real fract
	real*8 time

	call untime(time,yr,doy,mon,dom,hr,min,sec,fract)
*/

extime_(time,yr,doy,mon,dom,hr,min,sec,fract)
	double *time;	/*time to convert from */
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

	makeintt(*time, &intime);

	*yr = intime.tyr;	/* transfer results to calling program's */
	*doy = intime.tdoy;	/* 			variables.	 */
	*mon = intime.tmon;
	*dom = intime.tdom;
	*hr = intime.thr;
	*min = intime.tmin;
	*sec = intime.tsec;
	*fract = intime.tfract;
}
