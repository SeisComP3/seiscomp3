/***************************************************************************** 
 * qtime.cc
 *
 * Qlib2-compatible time functions without leapsecond support
 *
 * (c) 2019 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "qtime.h"

static int DOY[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

inline bool is_leap(int year)
  {
    return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
  }

inline int year_secs(int year)
  {
    return ((is_leap(year)? 366: 365) * 24 * 3600);
  }

inline int ldoy(int year, int month)
  {
    assert(month >= 1 && month <= 12);
    return DOY[month-1] + (is_leap(year) and month >= 3);
  }

void dy_to_mdy (int doy, int year, int *month, int *mday)
  {
    *month = 1;

    while (*month < 12 && doy > ldoy(year, *month + 1))
        *month += 1;

    *mday = doy - ldoy(year, *month);
  }

int mdy_to_doy (int month, int day, int year)
  {
    return ldoy(year, month) + day;
  }

INT_TIME ext_to_int(EXT_TIME et)
  {
    INT_TIME it;

    it.year = et.year;
    it.second = (((et.doy - 1) * 24 + et.hour) * 60 + et.minute) * 60 + et.second;
    it.usec = et.usec;

    return it;
  }

EXT_TIME int_to_ext(INT_TIME it)
  {
    EXT_TIME et;
    div_t minute, hour, doy;

    minute = div(it.second, 60);
    hour = div(minute.quot, 60);
    doy = div(hour.quot, 24);

    et.year = it.year;
    et.doy = doy.quot + 1;
    et.hour = doy.rem;
    et.minute = hour.rem;
    et.second = minute.rem;
    et.usec = it.usec;

    dy_to_mdy (et.doy, et.year, &et.month, &et.day);

    return et;
  }

INT_TIME add_time (INT_TIME it, int secs, int usecs)
  {
    div_t dsecs = div(it.usec + usecs, 1000000);

    usecs = dsecs.rem;
    secs = it.second + secs + dsecs.quot;

    if (usecs < 0)
      {
        usecs += 1000000;
        secs -= 1;
      }

    while (secs < 0)
      {
        --it.year;
        secs += year_secs(it.year);
      }

    while (secs >= year_secs(it.year))
      {
        secs -= year_secs(it.year);
        ++it.year;
      }

    it.second = secs;
    it.usec = usecs;

    return it;
  }

INT_TIME add_dtime(INT_TIME it, double usecs)
  {
    long long secs = floor((it.usec + usecs) / 1000000.0);

    usecs += it.usec - secs * 1000000.0;
    secs += it.second;

    while (secs < 0)
      {
        --it.year;
        secs += year_secs(it.year);
      }

    while (secs >= year_secs(it.year))
      {
        secs -= year_secs(it.year);
        ++it.year;
      }

    it.second = secs;
    it.usec = usecs;

    return it;
  }

double tdiff(INT_TIME it1, INT_TIME it2)
  {
    long long secs = 0;

    for (int y = it2.year; y < it1.year; ++y)
        secs += year_secs(y);

    for (int y = it1.year; y < it2.year; ++y)
        secs -= year_secs(y);

    return ((secs + it1.second - it2.second) * 1000000.0 + it1.usec - it2.usec);
  }

char *time_to_str (INT_TIME it, int fmt)
  {
    static char buf[40];
    EXT_TIME et = int_to_ext(it);

    assert(fmt == MONTHS_FMT);

    snprintf(buf, 40, "%d/%02d/%02d %02d:%02d:%02d.%04d", et.year, et.month, et.day,
      et.hour, et.minute, et.second, et.usec / 100);

    return buf;
  }

