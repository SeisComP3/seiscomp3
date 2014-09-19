/*
 * Simple time routines so the Quanterra time routines do not
 * need to be present to develop Seedlink plugins using send_log().
 *
 * no guarantees how this time might differ from what the
 * Seedlink server thinks is local time
 *
 * Chad Trabant/ORFEUS Data Center
 */

#include <time.h>
#include "timedef.h"

INT_TIME *set_it(INT_TIME *it)
{
  time_t epochsec;
  struct tm *local_time;
  int ysec;

  time(&epochsec);
  local_time = localtime(&epochsec);

  /* seconds into the year */
  ysec = 86400 * local_time->tm_yday;
  ysec += 3600 * local_time->tm_hour;
  ysec += 60 * local_time->tm_min;
  ysec += local_time->tm_sec;

  it->year = local_time->tm_year;
  it->second = ysec;
  /* no microsecond info from localtime() */
  it->usec = 0;

  return it;
}
