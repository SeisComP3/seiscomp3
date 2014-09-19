/*
 * g2time.c:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char rcsid[] = "$Id: g2time.c,v 1.5 2006/08/30 18:14:03 lwithers Exp $";

/*
 * $Log: g2time.c,v $
 * Revision 1.5  2006/08/30 18:14:03  lwithers
 * *** empty log message ***
 *
 * Revision 1.4  2004/10/14 09:48:36  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/07/28 12:36:45  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/05/02 10:12:20  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/01 23:46:03  root
 * *** empty log message ***
 *
 */

#include <gcf2.h>

/*Examples for using libgcf2's time functions*/



int
main (int argc, char *argv[])
{

  printf ("%s\n", rcsid);

  {
  G2GTime y={0,0};
  G2UTC u;
  struct tm tm={0};
  time_t t;
  u=G2GTime2UTC(y);
  tm=G2UTC2tm(u) ;
  t=mktime(&tm);

  printf("%d.%d is %s",y.day,y.sec,ctime(&t));
  }

  {
  G2GTime y={2147,0};
  G2UTC u;
  struct tm tm={0};
  time_t t;
  u=G2GTime2UTC(y);
  tm=G2UTC2tm(u) ;
  t=mktime(&tm);

  printf("%d.%d is %s",y.day,y.sec,ctime(&t));
  }

  {
  G2GTime y={-2147,0};
  G2UTC u;
  struct tm tm={0};
  time_t t;
  u=G2GTime2UTC(y);
  tm=G2UTC2tm(u) ;
  t=mktime(&tm);

  printf("%d.%d is %s",y.day,y.sec,ctime(&t));
  }

  {
/*GSL digitizers and the GCF data format, store times as */
/*two integers. The number of days since the GSL Epoch, and */
/*the number of seconds since the begining of the day, this */
/*division is convient for dealing with leap seconds */
/*The G2GTime type stores time in this format */
/* 
 * typedef struct {
 *  int day;
 *  int sec;
 * } G2GTime; 
 *
 */

    G2GTime gt1 = { 1024, 30102 };
    G2GTime gt2 = { 1023, 20102 };

/* The G2GTimeCompare Macro allows you to easily compare two G2GTimes */
/* allowed operations are == < and > NB it takes pointers*/

    printf ("G2GTimeCompare(&gt1,==,&gt1)=%d\n",
            G2GTimeCompare (&gt1, ==, &gt1));
    printf ("G2GTimeCompare(&gt1,>,&gt2)=%d\n",
            G2GTimeCompare (&gt1, >, &gt2));

/*G2GTimeDiff returns arg1-arg2 in seconds, hopefully dealing */
/*with gotchas like leap seconds*/

    printf ("G2GTimeDiff(gt1,gt2)=%d\n", G2GTimeDiff (gt1, gt2));

/*G2GTimeInc adds (or subtracts) a number of seconds to */
/*a G2GTime (again hopefully taking care of leap seconds) */

    gt2 = G2GTimeInc (gt1, 1000);
    printf ("G2GTimeDiff(G2GTimeInc(gt1,1000),gt1)=%d\n",
            G2GTimeDiff (gt2, gt1));

/*G2GTimeWrap takes a pointer to a G2GTime and regularizes sec so that */
/*it lies within one day, G2GTimeWrap contains no loops*/

    gt1.day = 1023;
    gt1.sec = -102;

    printf ("Before G2GTimeWrap: gt1={%d,%d}\n", gt1.day, gt1.sec);
    G2GTimeWrap (&gt1);
    printf ("After  G2GTimeWrap: gt1={%d,%d}\n", gt1.day, gt1.sec);

  }

  {
/*UTC time is stored in a broken down time structure called */
/*G2UTC*/
/*
 * typedef struct {
 *  int year;                     1900-
 *  int jday;                     1-366
 *  int month;                    1-12
 *  int mday;                     1-31
 *  int wday;                     1-7   [1=Mon, 7=Sun]
 *  int hour;                     0-23
 *  int min;                      0-59
 *  int sec;                      0-60
 *  int leap;                     0 or 1 (is this a leap year)
 *} G2UTC;
 *
 */

    G2GTime gt = { 1024, 30102 };
    G2UTC utc;
    struct tm tm;

/*G2GTime2UTC converts a G2GTime to a G2UTC */
/*G2GTime2UTC contains no loops*/

    utc = G2GTime2UTC (gt);

    printf ("G2GTime2UTC({%d,%d})={\n", gt.day, gt.sec);
    printf (" year=%d, jday=%d, month=%d, mday=%d, wday=%d,\n",
            utc.year, utc.jday, utc.month, utc.mday, utc.wday);
    printf (" hour=%d, min=%d, sec=%d, leap=%d\n",
            utc.hour, utc.min, utc.sec, utc.leap);
    printf ("}\n");

/*G2UTC2GTime does the reverse conversion */
/*The value of leap is ignored, NB IF jday IS !=0 then */
/*G2UTC2GTime will ignore month and mday, and use jday */
/*wday is always ignored*/
/*Again G2UTC2Gtime contains no loops*/
    gt = G2UTC2GTime (utc);

    printf ("G2GUTC2GTime(G2GTime2UTC({1024,30102}))={%d,%d}\n", gt.day,
            gt.sec);

    utc = G2GTime2UTC (gt);

    printf ("G2GTime2UTC(G2GUTC2GTime(G2GTime2UTC({%d,%d})))={\n", gt.day, gt.sec);
    printf (" year=%d, jday=%d, month=%d, mday=%d, wday=%d,\n",
            utc.year, utc.jday, utc.month, utc.mday, utc.wday);
    printf (" hour=%d, min=%d, sec=%d, leap=%d\n",
            utc.hour, utc.min, utc.sec, utc.leap);
    printf ("}\n");


/*G2UTC2tm converts a G2UTC to a struct tm - nothing clever is done */
/*tm_yday is computed from jday, tm_wday from wday &c. */

    tm = G2UTC2tm (utc);
    printf ("asctime(&G2UTC2tm(G2GTime2UTC({1024,30102})))=%s",
            asctime (&tm));

/*G2tm2UTC converts a struct tm back to a G2UTC, again nothing clever */
/*is done except the the value of leap is computed from year */

    utc = G2tm2UTC (tm);
    printf ("G2tm2UTC(G2UTC2tm(G2GTime2UTC({1024,30102}))))={\n");
    printf (" year=%d, jday=%d, month=%d, mday=%d, wday=%d,\n",
            utc.year, utc.jday, utc.month, utc.mday, utc.wday);
    printf (" hour=%d, min=%d, sec=%d, leap=%d\n",
            utc.hour, utc.min, utc.sec, utc.leap);
    printf ("}\n");

/*CAVEAT COMPUTOR*/
/*whilst libc's time functions have the capbility to deal with */
/*leap seconds, they dont. time_t in unix is seconds in calendar */
/*time since the epoch - NOT the number of seconds since the epoch */
/*to avoid problems use G2GTimes to store time values, and */
/*use G2GTimeInc, and G2GTimeDiff to compute deltas */
  }

  return 0;
}
