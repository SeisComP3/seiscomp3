#pragma ident "$Id: mstime.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
Program  : Any
Task     : Any program needing Time functions
File     : mstime.c
Purpose  : Y2K compliant general time functions
Host     : CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
Target   : Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
Author    : Robert Banfill (r.banfill@reftek.com)
Company  : Refraction Technology, Inc.
2626 Lombardy Lane, Suite 105
Dallas, Texas  75220  USA
(214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
Notes    :
$Revision: 165 $
$Logfile : R:/cpu68000/rt422/struct/version.h_v  $
Revised  :
15Sep04  ----  (RS) add ParseEHTime
21May98  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#ifdef WINNT
   #include <stdio.h>
   #include <stdlib.h>
   #include <conio.h>
#else
   #include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/timeb.h>

#include "platform.h"
#include "mstime.h"

/* Macros ------------------------------------------------------------- */
/* floor(x/y), where x, y>0 are integers, using integer arithmetic */
#define QFLOOR( x, y ) ((x) > 0 ? (x) / (y) : -(((y) - 1 - (x)) / (y)))

/* Local Prototypes --------------------------------------------------- */

#ifdef ANSI_C
   CHAR *MSTimeField(CHAR *ptr);
#else
   CHAR *MSTimeField();
#endif

/* Module Globals ----------------------------------------------------- */
static INT32 EndOfMonth[2][15] =
   {
      {
      0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 396, 424
      }
   ,
      {
      0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366, 397, 425
      }
   ,
};

static CHAR *Month[] =
   {
   "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   BOOL IsLeapYear(INT32 year)
#else
   BOOL IsLeapYear(year)INT32 year;

#endif
   {
   BOOL leap;

   if (year < 0)
      year++;

   leap = (year % 4 == 0);

   leap = leap && (year % 100 != 0 || year % 400 == 0);

   return (leap);
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   INT32 DayOfYear(INT32 year, INT32 month, INT32 day)
#else
   INT32 DayOfYear(year, month, day)INT32 year;
   INT32 month;
   INT32 day;

#endif
   {

   if (IsLeapYear(year))
      return (EndOfMonth[1][month - 1] + day);
   else
      return (EndOfMonth[0][month - 1] + day);
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   BOOL MonthDay(INT32 year, INT32 doy, INT32 *month, INT32 *day)
#else
   BOOL MonthDay(year, doy, month, day)INT32 year;
   INT32 doy;
   INT32 *month;
   INT32 *day;

#endif
   {
   INT32 i, leap;

   if (doy < 1 || doy > 366)
      return (FALSE);

   leap = (IsLeapYear(year) ? 1 : 0);

   i = 1;
   while (doy > EndOfMonth[leap][i])
      i++;

   *month = i;
   *day = doy - EndOfMonth[leap][i - 1];

   return (TRUE);
   ;
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   INT32 JulianDayNumber(INT32 year, INT32 month, INT32 day)
#else
   INT32 JulianDayNumber(year, month, day)INT32 year;
   INT32 month;
   INT32 day;

#endif
   {
   INT32 jdn;

   if (year < 0)
      year++;

   /* Move Jan. & Feb. to end of previous year */
   if (month <= 2)
      {
      --year;
      month += 12;
      }

   /* 1461 = (4*365+1) */
   jdn = QFLOOR(1461L *(year + 4712L), 4L) + EndOfMonth[0][month - 1] + day +  - QFLOOR(year, 100) + QFLOOR(year, 400) + 2;

   return (jdn);
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   BOOL Date(INT32 jdn, INT32 *year, INT32 *month, INT32 *day)
#else
   BOOL Date(jdn, year, month, day)INT32 jdn;
   INT32 *year;
   INT32 *month;
   INT32 *day;

#endif
   {
   INT32 y, d, t;

   /* Find position within cycles that are nd days INT32 */
   #define CYCLE( n, nd ) { t = QFLOOR( d - 1, nd ); y += t * n; d -= t * nd; }

   /* The same, with bound on cycle number */
   #define LCYCLE( n, nd, l ) { t = QFLOOR( d - 1, nd ); if( t > l ) t = l; y += t * n; d -=t * nd; }

   y =  - 4799;
   d = jdn + 31739; /* JD -31739 = 31 Dec 48 *01 B.C. */
   CYCLE(400, 146097) /* Four-century cycle */
   LCYCLE(100, 36524, 3) /* 100-year cycle */
   CYCLE(4, 1461) /* Four-year cycle */
   LCYCLE(1, 365, 3) /* Yearly cycle */
   if (y <= 0)
      --y;
   *year = y;

   return (MonthDay(y, d, month, day));
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   REAL64 SystemMSTime(VOID)
#else
   REAL64 SystemMSTime()
#endif
   {
   REAL64 systime;
   #ifdef WINNT
      struct _timeb tp;

      _ftime(&tp);

      if (tp.dstflag)
         systime = (REAL64)tp.time + ((REAL64)tp.millitm / 1000.0) - (((REAL64)tp.timezone - 60.0) *60.0);
      else
         systime = (REAL64)tp.time + ((REAL64)tp.millitm / 1000.0) - ((REAL64)tp.timezone *60.0);

   #else
      struct timeval tp;
      REAL64 secs, frac;

      while (gettimeofday(&tp, (void*)NULL) != 0)
         {
         perror("SystemMSTime: gettimeofday");
      }

      secs = (REAL64)tp.tv_sec;
      frac = (REAL64)tp.tv_usec / (REAL64)1000000;
      systime = secs + frac;

   #endif

   return (systime);
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   REAL64 EncodeMSTimeMD(INT32 year, INT32 month, INT32 day, INT32 hour, INT32 minute, REAL64 second)
#else
   REAL64 EncodeMSTimeMD(year, month, day, hour, minute, second)INT32 year;
   INT32 month;
   INT32 day;
   INT32 hour;
   INT32 minute;
   REAL64 second;

#endif
   {
   INT32 leap;

   if (year == 0 || month < 1 || month > 12)
      return ((REAL64)0.0);

   leap = (IsLeapYear(year) ? 1 : 0);

   if (day < 1 || day > EndOfMonth[leap][month] - EndOfMonth[leap][month - 1])
      return ((REAL64)0.0);

   return ((REAL64)(DAY *(JulianDayNumber(year, month, day) - BASEJDN) + hour * HOUR + minute * MINUTE + second * SECOND));
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   REAL64 EncodeMSTimeDOY(INT32 year, INT32 doy, INT32 hour, INT32 minute, REAL64 second)
#else
   REAL64 EncodeMSTimeDOY(year, doy, hour, minute, second)INT32 year;
   INT32 doy;
   INT32 hour;
   INT32 minute;
   REAL64 second;

#endif
   {
   INT32 month, day;

   if (MonthDay(year, doy, &month, &day))
      return (EncodeMSTimeMD(year, month, day, hour, minute, second));
   else
      return ((REAL64)0.0);
   }

/*--------------------------------------------------------------------- */

VOID DecodeMSTimeMD(REAL64 mstime, INT32 *year, INT32 *month, INT32 *day, INT32 *hour, INT32 *minute, REAL64 *second)
   {
   INT32 d;

   d = (INT32)QFLOOR((INT32)mstime, DAY);
   Date(d + BASEJDN, year, month, day);
   mstime -= (REAL64)(d *DAY);

   *hour = (INT32)mstime / HOUR;
   mstime -= (REAL64)((*hour) *HOUR);

   *minute = (INT32)mstime / MINUTE;

   *second = mstime - (((REAL64) *minute)*(REAL64)MINUTE);

   return ;
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   VOID DecodeMSTimeDOY(REAL64 mstime, INT32 *year, INT32 *doy, INT32 *hour, INT32 *minute, REAL64 *second)
#else
   VOID DecodeMSTimeDOY(mstime, year, month, doy, hour, second)REAL64 mstime;
   INT32 *year;
   INT32 *doy;
   INT32 *hour;
   INT32 *minute;
   REAL64 *second;

#endif
   {
   #ifdef WINNT
      INT32 month, day;

      DecodeMSTimeMD(mstime, year, &month, &day, hour, minute, second);

      *doy = DayOfYear(*year, month, day);

   #else

      UINT16 yr, da, hr, mn, sc, ms;

      util_tsplit((REAL64)mstime, &yr, &da, &hr, &mn, &sc, &ms);
      *year = yr;
      *doy = da;
      *hour = hr;
      *minute = mn;
      *second = (REAL64)sc + ((REAL64)ms / (REAL64)1000);
   #endif
   return ;
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   VOID DecomposeMSTime(REAL64 mstime, MSTIME_COMP *components)
#else
   VOID DecomposeMSTime(mstime, components)REAL64 mstime;
   MSTIME_COMP components;
#endif
   {
   DecodeMSTimeMD(mstime, &components->year, &components->month, &components->day, &components->hour, &components->minute, &components->second);

   components->doy = DayOfYear(components->year, components->month, components->day);

   return ;
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   CHAR *FormatMSTime(CHAR *string, REAL64 mstime, INT32 format)
#else
   CHAR *FormatMSTime(string, mstime, format)CHAR *string;
   REAL64 mstime;
   INT32 format;

#endif
   {
   INT32 year, doy, month, day, hour, minute;
   REAL64 second;

   DecodeMSTimeMD(mstime, &year, &month, &day, &hour, &minute, &second);

   switch (format)
      {
      case 0:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld%02ld%02ld%02ld%02ld%06.3lf", year, month, day, hour, minute, second);
         break;
      case 1:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld%02ld%02ld%02ld%02ld%02ld", year, month, day, hour, minute, (INT32)second);
         break;
      case 2:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld %02ld %02ld %02ld %02ld %06.3lf", year, month, day, hour, minute, second);
         break;
      case 3:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld %02ld %02ld% 02ld %02ld %02ld", year, month, day, hour, minute, (INT32)second);
         break;
      case 4:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld/%02ld/%02ld %02ld:%02ld:%06.3lf", month, day, year, hour, minute, second);
         break;
      case 5:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld", month, day, year, hour, minute, (INT32)second);
         break;
      case 6:
         sprintf(string, "%s %ld, %ld %02ld:%02ld %06.3lf", Month[month], day, year, hour, minute, second);
         break;
      case 7:
         sprintf(string, "%s %ld, %ld %02ld:%02ld %02ld", Month[month], day, year, hour, minute, (INT32)second);
         break;
      case 8:
         year = (year < 2000 ? year - 1900: year - 2000);
         doy = DayOfYear(year, month, day);
         sprintf(string, "%02ld %03ld %02ld:%02ld:%06.03lf", year, doy, hour, minute, second);
         break;
      case 9:
         sprintf(string, "%08lX", (UINT32)mstime);
         break;
      case 10:
         year = (year < 2000 ? year - 1900: year - 2000);
         doy = DayOfYear(year, month, day);
         /*
         sprintf( string, "%02ld*%03ld+%02ld:%02ld:%06.03lf",
         year, doy, hour, minute, second );
          */
         sprintf(string, "%02ld:%03ld:%02ld:%02ld:%06.03lf", year, doy, hour, minute, second);
         break;
      case 11:
         year = (year < 2000 ? year - 1900: year - 2000);
         sprintf(string, "%02ld/%02ld/%02ld %02ld:%02ld:%09.6lf", month, day, year, hour, minute, second);
         break;
      case 12:
         year = (year < 2000 ? year - 1900: year - 2000);
         doy = DayOfYear(year, month, day);
         sprintf(string, "%02ld*%03ld+%02ld:%02ld:%09.06lf", year, doy, hour, minute, second);
         break;
      }

   return (string);
   }

/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   REAL64 ParseMSTime(CHAR *string, BOOL mndy)
#else
   REAL64 ParseMSTime(strng, mndy)CHAR *string;
   BOOL mndy;

#endif
   {
   CHAR *ptr;
   INT32 year, mon, day, doy, hour, min;
   REAL64 sec, time;

   ptr = string;

   /* Default year, month, and day to current time, hour, min, and sec to 0 */
   time = SystemMSTime();
   DecodeMSTimeMD(time, &year, &mon, &day, &hour, &min, &sec);
   doy = DayOfYear(year, mon, day);
   hour = 0;
   min = 0;
   sec = 0.0;

   /* First field */
   if (*ptr)
      {
      if (mndy)
         mon = atol(ptr);
      /* Month */
      else
         {
         year = atol(ptr); /* Year */
         if (year < 70)
            year += 2000;
         else if (year < 100)
            year += 1900;
      }
      }

   /* Second field */
   ptr = MSTimeField(ptr);
   if (*ptr)
      {
      if (mndy)
         day = atol(ptr);
      /* Day */
      else
         doy = atol(ptr);
      /* Day of year */
      }

   /* Third field */
   ptr = MSTimeField(ptr);
   if (*ptr)
      {
      if (mndy)
         {
         year = atol(ptr); /* Year */
         if (year < 70)
            year += 2000;
         else if (year < 100)
            year += 1900;
         }
      else
         hour = atol(ptr);
      /* Hour */
      }

   /* Fourth field */
   ptr = MSTimeField(ptr);
   if (*ptr)
      {
      if (mndy)
         hour = atol(ptr);
      /* Hour */
      else
         min = atol(ptr);
      /* Minute */
      }

   /* Fifth field */
   ptr = MSTimeField(ptr);
   if (*ptr)
      {
      if (mndy)
         min = atol(ptr);
      /* Minute */
      else
         sec = atof(ptr);
      /* Second */
      }

   if (mndy)
      {
      /* Sixth field */
      ptr = MSTimeField(ptr);
      if (*ptr)
         {
         sec = atof(ptr); /* Second */
         }
      }
   else
      MonthDay(year, doy, &mon, &day);

   time = EncodeMSTimeMD(year, mon, day, hour, min, sec);

   return (time);
   }

/*--------------------------------------------------------------------- */
/* 15Sep04	----  (RS) make function to put packet trigger time into
format of header time .. YYYYDDDHHMMSS -> seconds since */
#ifdef ANSI_C
   REAL64 ParseEHTime(CHAR *string)
#else
   REAL64 ParseEHTime(strng)CHAR *string;
#endif
   {
   CHAR *ptr;
   INT32 year, mon, day, doy, hour, min;
   REAL64 sec, time;
   CHAR tmp_str[10];;

   ptr = string;
   /* YEAR */
   if (*ptr)
      {
      memcpy(tmp_str,ptr,4);
      tmp_str[4] = '\0';
      ptr+=4;
      year = atol(tmp_str); /* Year */
      if (year < 70)
         year += 2000;
      else if (year < 100)
         year += 1900;
 
  		/* DAY */
  		memcpy(tmp_str,ptr,3);
  		tmp_str[3] = '\0';
  		ptr+=3;
  		doy = atol(tmp_str);
  		
  		/* Hour */
  		memcpy(tmp_str,ptr,2);
  		tmp_str[2] = '\0';
  		ptr+=2;
  		hour = atol(tmp_str);
 
      /* Minute */
  		memcpy(tmp_str,ptr,2);
  		tmp_str[2] = '\0';
  		ptr+=2;
      min = atol(tmp_str);
 
      /* Second */
  		memcpy(tmp_str,ptr,2);
  		tmp_str[2] = '\0';
  		ptr+=2;
      sec = atol(tmp_str);
      }


   MonthDay(year, doy, &mon, &day);

   time = EncodeMSTimeMD(year, mon, day, hour, min, sec);

   return (time);
   }





/*--------------------------------------------------------------------- */

#ifdef ANSI_C
   CHAR *MSTimeField(CHAR *ptr)
#else
   CHAR *MSTimeField()CHAR *ptr;

#endif
   {

   if (*ptr)
      {
      while (*ptr && isdigit(*ptr))
         ptr++;

      while (*ptr && !isdigit(*ptr))
         ptr++;
      }

   return (ptr);
   }

#ifdef DEBUG_TEST

   main(int argc, char **argv)
      {
      REAL64 value;

      while (1)
         {
         value = SystemMSTime();
         printf("value = %.3lf\n", value);
         sleep(1);
         }
      }

#endif /* DEBUG_TEST */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.3  2004/09/30 19:10:52  rstavely
 * Modified write.c to setup a filename if a ET packet id received & no filename exists &
 * created routine in mstime.c to convert time in header
 *
 * Revision 1.2  2002/01/18 17:51:44  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
     */
