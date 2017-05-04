/* Author: T. McElfresh */

/* FORTRAN callable function to convert "human" to epochal time */

/* Input date hour minute second */
/*    or year month/mname day hour minute second */
/*    or year doy hour minute second */

/* Assigns epochal time & other unfilled fields */

struct date_time{
	double epoch;
	long date;
	int year;
	int month;
	char mname[4];
	int day;
	int doy;
	int hour;
	int minute;
	float second;
};

int
htoe_(epoch,date,year,month,mname,day,doy,hour,minute,second)

	double *epoch;
	long *date;
	int *year;
	int *month;
	char mname[4];
	int *day;
	int *doy;
	int *hour;
	int *minute;
	float *second;

   {
     struct date_time dp;

     dp.date = *date;
     dp.year = *year;
     dp.month = *month;
     dp.mname[0] = mname[0];
     dp.mname[1] = mname[1];
     dp.mname[2] = mname[2];
     dp.mname[3] = mname[3];
     dp.day = *day;
     dp.doy = *doy;
     dp.hour = *hour;
     dp.minute = *minute;
     dp.second = *second;

     /* htoe needs julian day, so calculate it if necesary */
     if(*date <= 0)
       {
       if(dp.year < 100)
	 dp.year = dp.year + 1900;
       if(*doy <= 0)
         mdtodate(&dp);
       dp.date = (dp.year*1000)+dp.doy;
       }

     htoe(&dp);

     *epoch = dp.epoch;
	*date = dp.date;
	*year = dp.year;
	*month = dp.month;
	mname[0] = dp.mname[0];
	mname[1] = dp.mname[1];
	mname[2] = dp.mname[2];
	mname[3] = dp.mname[3];
	*day = dp.day;
	*doy = dp.doy;
	*hour = dp.hour;
	*minute = dp.minute;
	*second = dp.second;
     return(1);
   }
