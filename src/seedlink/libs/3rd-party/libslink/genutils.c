/***************************************************************************
 *
 * General utility functions.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * Originally based on the SeedLink interface of the modified Comserv in
 * SeisComP written by Andres Heinloo
 *
 * Version: 2008.028
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libslink.h"

/***************************************************************************
 * sl_dtime:
 *
 * Return a double precision version of the current time since the epoch.
 * This is the number of seconds since Jan. 1, 1970 without leap seconds.
 ***************************************************************************/
double
sl_dtime (void)
{
  /* Now just a shell for the portable version */
  return slp_dtime ();
} /* End of sl_dtime() */

/***************************************************************************
 * sl_doy2md:
 *
 * Compute the month and day-of-month from a year and day-of-year.
 *
 * Returns 0 on success and -1 on error.
 ***************************************************************************/
int
sl_doy2md (int year, int jday, int *month, int *mday)
{
  int idx;
  int leap;
  int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  /* Sanity check for the supplied year */
  if (year < 1900 || year > 2100)
  {
    sl_log_r (NULL, 2, 0, "sl_doy2md(): year (%d) is out of range\n", year);
    return -1;
  }

  /* Test for leap year */
  leap = (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) ? 1 : 0;

  /* Add a day to February if leap year */
  if (leap)
    days[1]++;

  if (jday > 365 + leap || jday <= 0)
  {
    sl_log_r (NULL, 2, 0, "sl_doy2md(): day-of-year (%d) is out of range\n", jday);
    return -1;
  }

  for (idx = 0; idx < 12; idx++)
  {
    jday -= days[idx];

    if (jday <= 0)
    {
      *month = idx + 1;
      *mday  = days[idx] + jday;
      break;
    }
  }

  return 0;
} /* End of sl_doy2md() */

/***************************************************************************
 * sl_checkversion:
 *
 * Check protocol version number against some value
 *
 * Returns:
 *  1 = version is greater than or equal to value specified
 *  0 = no protocol version is known
 * -1 = version is less than value specified
 ***************************************************************************/
int
sl_checkversion (const SLCD *slconn, float version)
{
  if (slconn->protocol_ver == 0.0)
  {
    return 0;
  }
  else if (slconn->protocol_ver >= version)
  {
    return 1;
  }
  else
  {
    return -1;
  }
} /* End of sl_checkversion() */

/***************************************************************************
 * sl_checkslcd:
 *
 * Check a SeedLink connection description (SLCD struct).
 *
 * Returns 0 if pass and -1 if problems were identified.
 ***************************************************************************/
int
sl_checkslcd (const SLCD *slconn)
{
  int retval = 0;

  if (slconn->streams == NULL && slconn->info == NULL)
  {
    sl_log_r (slconn, 2, 0, "sl_checkslconn(): stream chain AND info type are empty\n");
    retval = -1;
  }

  return retval;
} /* End of sl_checkslconn() */

/***************************************************************************
 * sl_readline:
 *
 * Read characters from a stream (specified as a file descriptor)
 * until a newline character '\n' is read and place them into the
 * supplied buffer.  Reading stops when either a newline character is
 * read or buflen-1 characters have been read.  The buffer will always
 * contain a NULL-terminated string.
 *
 * Returns the number of characters read on success and -1 on error.
 ***************************************************************************/
int
sl_readline (int fd, char *buffer, int buflen)
{
  int nread = 0;

  if (!buffer)
    return -1;

  /* Read data from stream until newline character or max characters */
  while (nread < (buflen - 1))
  {
    /* Read a single character from the stream */
    if (read (fd, buffer + nread, 1) != 1)
    {
      return -1;
    }

    /* Trap door for newline character */
    if (buffer[nread] == '\n')
    {
      break;
    }

    nread++;
  }

  /* Terminate string in buffer */
  buffer[nread] = '\0';

  return nread;
} /* End of sl_readline() */
