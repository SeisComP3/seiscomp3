#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef	lint
static	char	SccsId[] = "@(#)timestr.c	44.1	9/23/91";
#endif

#include	"pfile.h"
/* TIMESTR
 *
 * This is a subroutine to convert a time to a printable string 
 * of the form:
 *	yy/mm/dd hh:mm:ss.fff
 */

char *timestr(t)
double t;
{
	intime_t it;
	static char string[22];

	strcpy(string,"-------- ------------");

	/* convert to intermediate time */
	if (makeintt(t,&it)) 

	/* convert it to string */
	sprintf(string,"%02d/%02d/%02d %02d:%02d:%02d.%03d",it.tyr-1900,it.tmon,
	  it.tdom,it.thr,it.tmin,it.tsec,(int)(it.tfract*1000.+.5));
	return(string);
}
