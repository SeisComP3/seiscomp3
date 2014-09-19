#ifndef	lint
static	char	SccsId[] = "@(#)stime_.c	44.1	9/23/91";
#endif

#include	"pfile.h"

/* calling sequence:

	real*8 time
	char*22 string
	char*22 stime

	string = stime(time)
*/

char *stime_(result,length,time)
char *result;
long length;
double *time;

{
	char *timestr();
	char *tmp;

	tmp = timestr(*time);
	while (*tmp != '\0')
		*result++ = *tmp++;
}
