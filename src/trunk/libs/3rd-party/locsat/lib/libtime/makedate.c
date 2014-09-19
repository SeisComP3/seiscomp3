#ifndef	lint
static	char	SccsId[] = "@(#)makedate.c	44.1	9/23/91";
#endif

#include	"pfile.h"

makedate(time, date)
double time;
long *date;
{
	intime_t timevec;

	makeintt(time, &timevec);
	*date = timevec.tyr*1000 + timevec.tdoy;
}
