/* Copyright (c) 2000-2009, The Johns Hopkins University
 * All rights reserved.
 *
 * The contents of this file are subject to a license (the ``License'').
 * You may not use this file except in compliance with the License. The
 * specific language governing the rights and limitations of the License
 * can be found in the file ``STDUTIL_LICENSE'' found in this 
 * distribution.
 *
 * Software distributed under the License is distributed on an AS IS 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. 
 *
 * The Original Software is:
 *     The Stdutil Library
 * 
 * Contributors:
 *     Creator - John Lane Schultz (jschultz@cnds.jhu.edu)
 *     The Center for Networking and Distributed Systems
 *         (CNDS - http://www.cnds.jhu.edu)
 */ 

#if defined(_WIN32)
#  include <windows.h>
#  include <sys/types.h>
#  include <sys/timeb.h>
#else
#  include <time.h>
#  include <sys/time.h>
#endif

#include <stdutil/stderror.h>
#include <stdutil/stdtime.h>

#ifdef __cplusplus
extern "C" {
#endif

/* stdtime: sec == STDINT64_MIN -> nano == 0
            sec <  0            -> -STD1BILLION < nano <= 0
            sec == 0            -> -STD1BILLION < nano < STD1BILLION
            sec >  0            -> 0 <= nano < STD1BILLION
*/

#define STDTIME_IS_LEGAL(t) ( ((t).sec != STDINT64_MIN || (t).nano == 0) && \
			      (((t).sec >= 0 && (t).nano >= 0 && (t).nano < STD1BILLION) || \
			       ((t).sec <= 0 && (t).nano <= 0 && (t).nano > -STD1BILLION)) )

/************************************************************************************************
 * stdtime: Get the current system time since UTC 00:00:00, Jan 1, 1970.
 ***********************************************************************************************/

STDINLINE stdcode stdtime_now(stdtime *abs_time)
#if defined(_WIN32)
{
  struct _timeb t;

  _ftime(&t);

  abs_time->sec  = t.time;
  abs_time->nano = t.millitm * STD1MILLION;

  return STDESUCCESS;
}
#else
{
  stdcode        ret = STDESUCCESS;
  struct timeval t;

  if (gettimeofday(&t, NULL) != STDESUCCESS) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
    goto stdtime_now_end;
  }

  abs_time->sec  = t.tv_sec;
  abs_time->nano = t.tv_usec * STD1THOUSAND;

stdtime_now_end:
  return ret;
}
#endif

/************************************************************************************************
 * stdtime_time64: Return a stdtime created from a stdtime_time64.
 *
 * NOTE: '/' and '%' aren't uniformly defined for negative operands so we negate as needed
 ***********************************************************************************************/

STDINLINE stdtime stdtime_time64(stdtime64 src)
{
  stdtime64 cpy = (src >= 0 ? src : -src);
  stdtime   ret;

  ret.sec  = cpy / STD1BILLION;
  ret.nano = (stdint32) (cpy % STD1BILLION);

  if (src < 0) {
    ret.sec  = -ret.sec;
    ret.nano = -ret.nano;
  }

  return ret;
}

/************************************************************************************************
 * stdtime_cmp: Compare two stdtimes.
 ***********************************************************************************************/

STDINLINE int stdtime_cmp(stdtime ls, stdtime rs)
{
  STDSAFETY_CHECK(STDTIME_IS_LEGAL(ls) && STDTIME_IS_LEGAL(rs));

  if (ls.sec == rs.sec) {

    if (ls.nano == rs.nano) {
      return 0;
    }

    return (ls.nano < rs.nano ? -1 : 1);
  }

  return (ls.sec < rs.sec ? -1 : 1);
}

/************************************************************************************************
 * stdtime_sign: Get the sign (-1: negative, 0: none, 1: positive) of a stdtime.
 ***********************************************************************************************/

STDINLINE int stdtime_sign(stdtime t) 
{
  stdtime t2 = { 0, 0 };

  return stdtime_cmp(t, t2);
}
	
/************************************************************************************************
 * stdtime_neg: Return the negation of a stdtime.
 ***********************************************************************************************/

STDINLINE stdtime stdtime_neg(stdtime t) 
{
  STDSAFETY_CHECK(STDTIME_IS_LEGAL(t));

  t.sec  = -t.sec;
  t.nano = -t.nano;

  return t;
}

/************************************************************************************************
 * stdtime_add: Return the addition of two stdtimes.
 ***********************************************************************************************/

STDINLINE stdtime stdtime_add(stdtime ls, stdtime rs)
{
  stdtime ret;

  STDSAFETY_CHECK(STDTIME_IS_LEGAL(ls) && STDTIME_IS_LEGAL(rs));

  /* do the addition */ 

  ret.sec  = ls.sec  + rs.sec;
  ret.nano = ls.nano + rs.nano;

  /* ensure -STD1BILLION < nano < STD1BILLION */

  if (ret.nano >= STD1BILLION) {
    ++ret.sec;
    ret.nano -= STD1BILLION;

  } else if (ret.nano <= -STD1BILLION) {
    --ret.sec;
    ret.nano += STD1BILLION;
  }

  /* check for non-agreement in signs */

  if (ret.sec > 0) {

    if (ret.nano < 0) {
      --ret.sec;
      ret.nano += STD1BILLION;
    }

  } else if (ret.sec != 0) {

    if (ret.nano > 0) {
      ++ret.sec;
      ret.nano -= STD1BILLION;
    }
  }

  /* check for nasty edge case and wrap around if not exactly minimum representation */

  if (ret.sec == STDINT64_MIN && ret.nano != 0) {
    --ret.sec;
    ret.nano += STD1BILLION;
  }

  return ret;
}

/************************************************************************************************
 * stdtime_sub: Return the difference of two stdtimes.
 ***********************************************************************************************/

STDINLINE stdtime stdtime_sub(stdtime ls, stdtime rs)
{
  return stdtime_add(ls, stdtime_neg(rs));
}

/************************************************************************************************
 * stdtime64_now: Get the current system time since UTC 00:00:00, Jan 1, 1970.
 ***********************************************************************************************/

STDINLINE stdcode stdtime64_now(stdtime64 *abs_time)
{
  stdtime t;
  stdcode ret = stdtime_now(&t);

  if (ret == STDESUCCESS) {
    *abs_time = stdtime64_time(t);
  }

  return ret;
}

/************************************************************************************************
 * stdtime64_time: Return a stdtime64 from a stdtime.
 ***********************************************************************************************/

STDINLINE stdtime64 stdtime64_time(stdtime t)
{
  STDSAFETY_CHECK(STDTIME_IS_LEGAL(t));

  return (stdtime64) t.sec * STD1BILLION + t.nano;
}

/************************************************************************************************
 * stdsleep: Sleep for a period of time.  On error, remndr will be
 * filled if non-NULL.
 ***********************************************************************************************/

STDINLINE stdcode stdsleep(stdtime delta, stdtime *remndr)
#  if defined(_WIN32)
{
  stdcode ret        = STDESUCCESS;
  stdtime start_time = { 0, 0 };
  stdtime end_time   = { 0, 0 };
  DWORD   milli_sleep;

  if (delta.sec < 0 || delta.nano < 0 || delta.nano >= STD1BILLION) {
    ret = STDEINVAL;
    goto stdsleep_fail;
  }

  /* conditional +1 for possible nanosecond division trunctation */

  milli_sleep = (DWORD) (delta.sec * STD1THOUSAND + delta.nano / STD1MILLION) + (delta.nano != 0 ? 1 : 0); 

  Sleep(milli_sleep);

  goto stdsleep_end;

  /* error handling and return */

 stdsleep_fail:
  if (remndr != NULL) {
    *remndr = delta;
  }

 stdsleep_end:
  return ret;
}
#else
{
  stdcode         ret;
  struct timespec d;
  struct timespec r;

  if (delta.sec < 0 || delta.nano < 0 || delta.nano >= STD1BILLION) {
    ret = STDEINVAL;
    goto stdsleep_fail;
  }

  d.tv_sec  = (time_t) delta.sec;
  d.tv_nsec = delta.nano;

  if ((ret = nanosleep(&d, &r)) != STDESUCCESS) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);

    if (remndr != NULL) {
      remndr->sec  = r.tv_sec;
      remndr->nano = r.tv_nsec;
    }
  }

  goto stdsleep_end;

  /* error handling and return */

 stdsleep_fail:
  if (remndr != NULL) {
    *remndr = delta;
  }

 stdsleep_end:
  return ret;
}
#endif

/************************************************************************************************
 * stdsleep64: Sleep for a number of nanoseconds.
 ***********************************************************************************************/

STDINLINE stdcode stdsleep64(stdtime64 delta, stdtime64 *remndr)
{
  stdcode  ret;
  stdtime  t_delta = stdtime_time64(delta);
  stdtime  t_remndr;

  ret = stdsleep(t_delta, (remndr != NULL ? &t_remndr : NULL));

  if (ret != STDESUCCESS && remndr != NULL) {
    *remndr = stdtime64_time(t_remndr);
  }

  return ret;
}

#ifdef __cplusplus
}
#endif
