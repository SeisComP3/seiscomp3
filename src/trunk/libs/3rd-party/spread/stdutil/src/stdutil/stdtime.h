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

#ifndef stdtime_h_2003_12_18_18_09_56_jschultz_at_cnds_jhu_edu
#define stdtime_h_2003_12_18_18_09_56_jschultz_at_cnds_jhu_edu

#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  stdint64 sec;
  stdint32 nano;

} stdtime;

typedef stdint64 stdtime64;  /* time represented in nanoseconds */

STDINLINE stdcode   stdtime_now(stdtime *abs_time);
STDINLINE stdtime   stdtime_time64(stdtime64 src);

STDINLINE int       stdtime_cmp(stdtime ls, stdtime rs);
STDINLINE int       stdtime_sign(stdtime t);

STDINLINE stdtime   stdtime_neg(stdtime t);
STDINLINE stdtime   stdtime_add(stdtime ls, stdtime rs);
STDINLINE stdtime   stdtime_sub(stdtime ls, stdtime rs);

STDINLINE stdcode   stdtime64_now(stdtime64 *abs_time);
STDINLINE stdtime64 stdtime64_time(stdtime t);  /* conversion can silently over/under flow */

/* sleep fcns */

STDINLINE stdcode   stdsleep(stdtime delta, stdtime * rem);
STDINLINE stdcode   stdsleep64(stdtime64 delta, stdtime64 * rem);

#ifdef __cplusplus
}
#endif

#endif
