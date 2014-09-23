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

#ifndef stderror_h_2000_05_15_14_04_16_jschultz_at_cnds_jhu_edu
#define stderror_h_2000_05_15_14_04_16_jschultz_at_cnds_jhu_edu

#include <errno.h>
#include <stdio.h>

#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: redefining this limit only has an effect at compile time of the stdutil library */

#ifndef STDERR_MAX_ERR_MSG_LEN 
#  define STDERR_MAX_ERR_MSG_LEN 1024
#endif

/* stderr output stream (can be set to NULL to squelch all stdutil output) */

extern FILE * stdutil_output /* = stderr */;

/* stderr error routines */

typedef enum 
{
  STDERR_RETURN,
  STDERR_EXIT,
  STDERR_ABORT

} stderr_action;

int stderr_output(stderr_action act, int errno_cpy, const char *fmt, ...);

STDINLINE const char *stderr_strerr(stdcode code);

/* error macros */

#define STDEXCEPTION(x) stderr_output(STDERR_ABORT, 0, "STDEXCEPTION: File: %s; Line: %d: %s", __FILE__, __LINE__, #x)

#if defined(STDSAFETY_CHECKS)
#  define STDSAFETY_CHECK(x) { if (!(x)) { STDEXCEPTION(safety check (x) failed); } }
#else
#  define STDSAFETY_CHECK(x) 
#endif

#if defined(STDBOUNDS_CHECKS)
#  define STDBOUNDS_CHECK(x) { if (!(x)) { STDEXCEPTION(bounds check (x) failed); } }
#else
#  define STDBOUNDS_CHECK(x)
#endif

/* error values */

#define STDESUCCESS 0
#define STDEOF EOF

#if defined(EUNKNOWN)
#  define STDEUNKNOWN EUNKNOWN
#else
#  define STDEUNKNOWN 500
#endif

#if defined(EINVAL)
#  define STDEINVAL EINVAL
#else
#  define STDEINVAL 501
#endif

#if defined(ENOMEM)
#  define STDENOMEM ENOMEM
#else
#  define STDENOMEM 502
#endif

#if defined(EACCES)
#  define STDEACCES EACCES
#else
#  define STDEACCES 503
#endif

#if defined(EBUSY)
#  define STDEBUSY EBUSY
#else
#  define STDEBUSY 504
#endif

#if defined(EPERM)
#  define STDEPERM EPERM
#else
#  define STDEPERM 505
#endif

#if defined(ENOSYS)
#  define STDENOSYS ENOSYS
#else
#  define STDENOSYS 506
#endif

#if defined(EINTR)
#  define STDEINTR EINTR
#else
#  define STDEINTR 507
#endif

#if defined(ETIMEDOUT)
#  define STDETIMEDOUT ETIMEDOUT
#else
#  define STDETIMEDOUT 508
#endif

#ifdef __cplusplus
}
#endif

#endif
