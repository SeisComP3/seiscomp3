/*
 * ./src/gtime.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gtime.h,v 1.1 2003/05/16 10:40:27 root Exp $
 */

/*
 * $Log: gtime.h,v $
 * Revision 1.1  2003/05/16 10:40:27  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/05/14 16:31:45  root
 * #
 *
 * Revision 1.3  2003/05/13 15:10:03  root
 * #
 *
 * Revision 1.2  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:16  root
 * #
 *
 */

#ifndef __GTIME_H__
#define __GTIME_H__

#include <time.h>

typedef struct
{
  int day;
  int sec;
}
G2GTime;

/*When using a UTC time as a source, set jday to zero */
/*to use mday and month. wday and leap are always ignored */

typedef struct
{
  int year;                     /*1900-    */
  int jday;                     /*1-366    */
  int month;                    /*1-12     */
  int mday;                     /*1-31    */
  int wday;                     /*1-7   [1=Mon, 7=Sun]   */
  int hour;                     /*0-23     */
  int min;                      /*0-59    */
  int sec;                      /*0-60    */
  int leap;                     /*0 or 1    */
}
G2UTC;

extern G2UTC G2GTime2UTC (G2GTime);
extern G2GTime G2UTC2GTime (G2UTC);
extern void G2GTimeWrap (G2GTime *);
extern struct tm G2UTC2tm (G2UTC);
extern G2UTC G2tm2UTC (struct tm);
extern int G2GTimeDiff (G2GTime, G2GTime); /* arg1-arg2 in secs */
extern G2GTime G2GTimeInc (G2GTime, int);
#endif /* __GTIME_H__ */
