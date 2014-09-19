#ifndef	lint
static	char	SccsId[] = "@(#)doy.c	44.1	9/23/91";
#endif

/* DOY
 * 
 * This function will convert mon and day within the year and return it as
 * the value of the function.  If mon or day are illegal values, the value 
 * of the function will be 0 (0).
 */

int ndays[13] = {0,0,31,59,90,120,151,181,212,243,273,304,334};

doy(mon,day,year)
	int mon, day, year;
{
	int inc;
	if (mon < 1 || mon > 12) return(0);
	if (day < 1 || day > 31) return(0);
	if (lpyr(year) && mon > 2) inc = 1;
	else inc = 0;
	
	return(ndays[mon] + day + inc);
}

/******************************************************************************/

/* DOM
 *
 * This function converts a day within the year (dofy) to the month and
 * day of the month.  If dofy is an illegal value, mon and day will be 
 * returned as 0.
 */

int mdays[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

dom(dofy,mon,day,year)
	int dofy,*mon,*day,year;
{
	int iday;
	if (dofy < 1) {
		*mon = 0;
		*day = 0;
		return(0);
	}

	if (lpyr(year)) mdays[2] = 29;
	else mdays[2] = 28;

	for (*mon = 1; *mon <= 12; (*mon)++) {
		*day = dofy;
		if ((dofy -= mdays[*mon]) <= 0) return(1);
	}

	*mon = 0;
	*day = 0;
	return(0);
}

/******************************************************************************/

/* LPYR
 * 
 * This function returns 1 if year is a leap year.
 */

lpyr(year)
	int year;
{
	if (year%400 == 0) return(1);
	if (year%4 != 0) return(0);
	if (year%100 == 0) return(0);
	else return(1);
}
