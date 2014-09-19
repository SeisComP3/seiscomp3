#ifndef	lint
static	char	SccsId[] = "@(#)mkdate_.c	44.1	9/23/91";
#endif

#include	"pfile.h"

untime_(time,date)
	double *time;	/*time to convert from */
	int *date;
{
	intime_t intime;	/* declare structure for results */

	makeintt(*time, &intime);

	*date = (intime.tyr)*1000 + intime.tdoy;
}
