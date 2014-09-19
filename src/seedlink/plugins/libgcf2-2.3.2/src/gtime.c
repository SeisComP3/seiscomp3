/*
 * ./src/gtime.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gtime.c,v 1.6 2004/10/14 09:48:36 root Exp $";

/*
 * $Log: gtime.c,v $
 * Revision 1.6  2004/10/14 09:48:36  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/20 20:09:12  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/03/18 19:09:02  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/10/30 15:34:53  root
 * added a blank line for no reason
 *
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:26  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/14 16:31:46  root
 * #
 *
 * Revision 1.2  2003/05/13 15:10:03  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:17  root
 * #
 *
 */

#include "includes.h"

#include "gtime.h"

#define SECSINDAY (24*60*60)

static inline void
set_leap (G2UTC * u)
{
  if (u->year % 4)
    {
      u->leap = 0;
    }
  else if (u->year % 100)
    {
      u->leap = 1;
    }
  else if (u->year % 400)
    {
      u->leap = 0;
    }
  else
    {
      u->leap = 1;
    }
}


G2UTC
G2GTime2UTC (G2GTime g)
{
  G2UTC u;

  int y400, y100, y4;
  int leap;

/* A large degree of infernal logic here */
/* Years run in 400 year cycles, we arrange our cycle to start in 1601 */
/* One year after a leap year, so that when we get down to individual */
/* years it is the last of the cycle of 4 that could be a leap year */

  g.day += 142034;              /* days between GSL epoch and Julian 1st Jan 1601 */

  u.wday = g.day % 7;
  u.wday++;

  y400 = g.day / 146097;        /*146097 days in 400 years */
  g.day -= (y400 * 146097);

  y100 = g.day / 36524;         /*36524 days in 100 years */
  g.day -= (y100 * 36524);

  y4 = g.day / 1461;            /*1461 days in 4 years */
  g.day -= (y4 * 1461);

  /* This may look redundant but 31 Dec in year 4 is special case */
  if (g.day < 1095)             /*1095 days in 3 years */
    {
      u.year = g.day / 365;     /*365 days in a year */
      g.day -= (365 * u.year);
    }
  else
    {
      u.year = 3;
      g.day -= 1095;
    }



  /* Now put it all back together */
  u.year += 1601;
  u.year += 4 * y4;
  u.year += 100 * y100;
  u.year += 400 * y400;

  u.jday = g.day + 1;

  set_leap (&u);

  if (!u.leap)
    {                           /*Days and months for ordinary years */
      if (u.jday < 32)
        {
          u.month = 1;
          u.mday = u.jday;
        }
      else if (u.jday < 60)
        {
          u.month = 2;
          u.mday = u.jday - 31;
        }
      else if (u.jday < 91)
        {
          u.month = 3;
          u.mday = u.jday - 59;
        }
      else if (u.jday < 121)
        {
          u.month = 4;
          u.mday = u.jday - 90;
        }
      else if (u.jday < 152)
        {
          u.month = 5;
          u.mday = u.jday - 120;
        }
      else if (u.jday < 182)
        {
          u.month = 6;
          u.mday = u.jday - 151;
        }
      else if (u.jday < 213)
        {
          u.month = 7;
          u.mday = u.jday - 181;
        }
      else if (u.jday < 244)
        {
          u.month = 8;
          u.mday = u.jday - 212;
        }
      else if (u.jday < 274)
        {
          u.month = 9;
          u.mday = u.jday - 243;
        }
      else if (u.jday < 305)
        {
          u.month = 10;
          u.mday = u.jday - 273;
        }
      else if (u.jday < 335)
        {
          u.month = 11;
          u.mday = u.jday - 304;
        }
      else
        {
          u.month = 12;
          u.mday = u.jday - 334;
        }
    }
  else
    {                           /*And leap years */
      if (u.jday < 32)
        {
          u.month = 1;
          u.mday = u.jday;
        }
      else if (u.jday < 61)
        {
          u.month = 2;
          u.mday = u.jday - 31;
        }
      else if (u.jday < 92)
        {
          u.month = 3;
          u.mday = u.jday - 60;
        }
      else if (u.jday < 122)
        {
          u.month = 4;
          u.mday = u.jday - 91;
        }
      else if (u.jday < 153)
        {
          u.month = 5;
          u.mday = u.jday - 121;
        }
      else if (u.jday < 183)
        {
          u.month = 6;
          u.mday = u.jday - 152;
        }
      else if (u.jday < 214)
        {
          u.month = 7;
          u.mday = u.jday - 182;
        }
      else if (u.jday < 245)
        {
          u.month = 8;
          u.mday = u.jday - 213;
        }
      else if (u.jday < 275)
        {
          u.month = 9;
          u.mday = u.jday - 244;
        }
      else if (u.jday < 306)
        {
          u.month = 10;
          u.mday = u.jday - 274;
        }
      else if (u.jday < 336)
        {
          u.month = 11;
          u.mday = u.jday - 305;
        }
      else
        {
          u.month = 12;
          u.mday = u.jday - 335;
        }
    }

  u.hour = g.sec / 3600;
  g.sec -= u.hour * 3600;
  u.min = g.sec / 60;
  g.sec -= u.min * 60;
  u.sec = g.sec;

  return u;
}

G2GTime
G2UTC2GTime (G2UTC u)
{
  G2GTime g;
  int y400;
  int y100;
  int y4;

  static int const mdays[] =
    { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  static int const lmdays[] =
    { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

  set_leap (&u);
  if (!u.jday)
    {
      if (u.leap)
        {
          u.jday = u.mday + lmdays[u.month];
        }
      else
        {
          u.jday = u.mday + mdays[u.month];
        }
    }

  /*See forwards code for an explanation */

  u.year -= 1601;
  y400 = u.year / 400;
  u.year -= y400 * 400;
  y100 = u.year / 100;
  u.year -= y100 * 100;
  y4 = u.year / 4;
  u.year -= y4 * 4;

  

  g.day = u.jday - 1;

  g.day += u.year * 365;
  g.day += y4 * 1461;
  g.day += y100 * 36524;
  g.day += y400 * 146097;
  g.day -= 142034;              /* days between GSL epoch and Julian 1st Jan 1601 */


  g.sec = u.sec;
  g.sec += u.min * 60;
  g.sec += u.hour * 3600;

  return g;
}

struct tm
G2UTC2tm (G2UTC u)
{
  struct tm t = { 0 };

  t.tm_sec = u.sec;
  t.tm_min = u.min;
  t.tm_hour = u.hour;
  t.tm_mday = u.mday;
  t.tm_mon = u.month - 1;
  t.tm_year = u.year - 1900;
  t.tm_wday = u.wday % 7;
  t.tm_yday = u.jday - 1;
  t.tm_isdst = 0;

  return t;
}


G2UTC
G2tm2UTC (struct tm t)
{
  G2UTC u;

  u.sec = t.tm_sec;
  u.min = t.tm_min;
  u.hour = t.tm_hour;
  u.mday = t.tm_mday;
  u.month = t.tm_mon + 1;
  u.year = t.tm_year + 1900;
  u.wday = ((t.tm_wday + 6) % 7) + 1;
  u.jday = t.tm_yday + 1;

  set_leap (&u);

  return u;
}

/*FIXME: leap seconds */
int
G2GTimeDiff (G2GTime g1, G2GTime g2)
{
  int s;
  g1.day -= g2.day;
  g1.sec -= g2.sec;
  g1.sec += SECSINDAY * g1.day; /*FIXME: */
  return g1.sec;
}

/*FIXME: leap seconds */
G2GTime
G2GTimeInc (G2GTime g, int i)
{
  long int d, s = g.sec;
  s += (long int) i;

  if (s >= SECSINDAY)
    {
      d = s / SECSINDAY;
      s -= d * SECSINDAY;

      g.day += (int) d;
      g.sec = (int) s;

      return g;
    }
  else if (s < 0)
    {
      d = s / SECSINDAY;
      d--;
      s -= d * SECSINDAY;
      g.day += (int) d;
      g.sec = (int) s;
      return g;
    }

  g.sec = (int) s;
  return g;

}

/*FIXME: leap seconds */
void
G2GTimeWrap (G2GTime * g)
{
  long int d, s = g->sec;

  if (s > SECSINDAY)
    {
      d = s / SECSINDAY;
      s -= d * SECSINDAY;

      g->day += (int) d;
      g->sec = (int) s;

      return;
    }
  else if (s < 0)
    {
      d = s / SECSINDAY;
      d--;
      s -= d * SECSINDAY;
      g->day += (int) d;
      g->sec = (int) s;
      return;

    }
  return;
}


