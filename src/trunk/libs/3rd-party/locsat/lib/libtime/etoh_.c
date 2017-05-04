#include <string.h>
#include <stdlib.h>

/* FORTRAN callable routine to convert epochal time to human time */

/* INPUT
**	epoch -- epochal time to be converted
**
** OUTPUT
**	date,year,month,mname,day,doy (day of year),hour,minute,second
**	corresponding to the input epoch
*/

#include "csstime.h"

void etoh_(double *epoch, long *date,
           int *year, int *month, char mname[4],
           int *day, int *doy, int *hour, int *minute, float *second) {
	struct date_time dp;

	dp.epoch = *epoch;

	etoh(&dp);

	*date = dp.date;
	*year = dp.year;
	*month = dp.month;
	strcpy(mname,dp.mname);
	*day = dp.day;
	*doy = dp.doy;
	*hour = dp.hour;
	*minute = dp.minute;
	*second = dp.second;
}
