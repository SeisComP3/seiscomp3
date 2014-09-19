#ifndef	lint
static	char	SccsId[] = "@(#)doy_.c	44.1	9/23/91";
#endif

idoy_(mon,day,year)
	int *mon, *day, *year;
{
	return(  doy(*mon,*day,*year) );
}

dom_(dofy,mon,day,year)
	int *dofy, *mon, *day, *year;
{
	int smon, sday;

	dom(*dofy,&smon,&sday,*year);
	*mon = smon;
	*day = sday;
}

lpyr_(year)
	int *year;
{
	return(  lpyr(*year) );
}
