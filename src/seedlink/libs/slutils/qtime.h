/***************************************************************************** 
 * time.h
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

#ifndef QTIME_H
#define QTIME_H

#define MONTHS_FMT 6

struct EXT_TIME
  {
    int year;      /* year, eg. 2003         */
    int doy;       /* day of year (1-366)    */
    int month;     /* month (1-12)           */
    int day;       /* day of month (1-31)    */
    int hour;      /* hour (0-23)            */
    int minute;    /* minute (0-59)          */
    int second;    /* second (0-59)          */
    int usec;      /* microsecond (0-999999) */
  };

struct INT_TIME
 {
    int year;
    int second;
    int usec;
 };

#ifdef __cplusplus
extern "C"
{
#endif

//! Converts an EXT_TIME structure into an INT_TIME
struct INT_TIME ext_to_int(struct EXT_TIME et);

//! Converts an INT_TIME structure into an EXT_TIME
struct EXT_TIME int_to_ext(struct INT_TIME it);

//! Takes a day_of_year and year and computes month and day_of_month
void dy_to_mdy (int doy, int year, int *month, int *mday);

//! Takes a month, day and year and computes day_of_year
int mdy_to_doy (int month, int day, int year);

//! Adds the specified number of seconds and usecs to an INT_TIME structure
struct INT_TIME add_time (struct INT_TIME it, int secs, int usecs);

//! Adds the specified number of usecs to an INT_TIME structure
struct INT_TIME add_dtime(struct INT_TIME it, double usecs);

//! Computes the time difference of (it1 - it2) in usecs
double tdiff(struct INT_TIME it1, struct INT_TIME it2);

//! Generates a printable timestamp for the specified time
char *time_to_str (struct INT_TIME it, int fmt);

#ifdef __cplusplus
}
#endif

#endif // QTIME_H

