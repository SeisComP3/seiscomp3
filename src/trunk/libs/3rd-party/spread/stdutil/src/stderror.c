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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <stdutil/stddefines.h>
#include <stdutil/stderror.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************************
 * stdutil_output: Output stream used for all library error msgs.
 ***********************************************************************************************/

FILE * stdutil_output = (FILE*) 0x1;  /* magic number because stderr is not a constant */

/************************************************************************************************
 * stderr_output: Print a message to stdutil_output and flush it.  If
 * errno_copy is non-zero, also print error msg from strerror.  Return
 * # of characters written to output stream.
 *
 * NOTE: I'd prefer to use snprintf and vsnprintf but they aren't part of C89.
 ***********************************************************************************************/

STDINLINE int stderr_output(stderr_action act, int errno_copy, const char *fmt, ...) 
{
  char    buf[STDERR_MAX_ERR_MSG_LEN + 1];
  int     ret1 = 0;
  int     ret2 = 0;
  va_list ap;

  if (stdutil_output != NULL) {
    va_start(ap, fmt);

    ret1      = vsprintf(buf, fmt, ap);  /* write the msg */
    ret1      = STDMAX(ret1, 0);         /* zero out any error */
    buf[ret1] = 0;                       /* ensure termination */

    if (errno_copy != 0) {
      ret2             = sprintf(buf + ret1, ": %s", strerror(errno_copy));   /* errno msg */
      ret2             = STDMAX(ret2, 0);                                     /* zero out */
      buf[ret1 + ret2] = 0;                                                   /* termination */
    }

    if (stdutil_output == (FILE*) 0x1) {
      stdutil_output = stderr;
    }

    fprintf(stdutil_output, "%s\r\n", buf);
    fflush(stdutil_output);

    ret1 += 2;                                                                /* +2 for \r\n */
    va_end(ap);
  }

  if (act == STDERR_EXIT) {
    exit(-1);
  }

  if (act == STDERR_ABORT) {
    abort();
  }

  return ret1 + ret2;
}

/************************************************************************************************
 * stderr_strerr: Returns a constant string in response to a StdUtil
 * error code.  Some StdUtil fcns can return system specific codes.
 * In that case this fcn will return a "Unknown Error Code (system
 * error code)" string and you should consult your system specific
 * error lookup service.
 ***********************************************************************************************/

STDINLINE const char *stderr_strerr(stdcode code)
{
  const char * ret;

  switch (code) {
  case STDESUCCESS:
    ret = "Success";
    break;

  case STDEUNKNOWN:
    ret = "Unknown Error";
    break;

  case STDEINVAL:
    ret = "Invalid Argument";
    break;

  case STDENOMEM:
    ret = "Memory Allocation Failed";
    break;

  case STDEACCES:
    ret = "Permission Denied";
    break;

  case STDEBUSY:
    ret = "Resource Busy";
    break;

  case STDEPERM:
    ret = "Operation Not Permitted";
    break;

  case STDENOSYS:
    ret = "Functionality Not Implemented";
    break;

  case STDEINTR:
    ret = "Operation Interrupted";
    break;

  default:
    ret = "Unknown Error Code (system error code)";
    break;
  }

  return ret;
}

#ifdef __cplusplus
}
#endif
