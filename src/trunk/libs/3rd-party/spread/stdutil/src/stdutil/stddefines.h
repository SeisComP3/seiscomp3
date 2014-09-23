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

#ifndef stddefines_h_2000_10_22_18_20_30_jschultz_at_cnds_jhu_edu
#define stddefines_h_2000_10_22_18_20_30_jschultz_at_cnds_jhu_edu

#include <stddef.h>
#include <limits.h>

/* include basic, architecture-specific types and constants */

#include <stdutil/private/stdarch.h>

#ifdef __cplusplus
extern "C" {
#endif

/* define some useful basic types and constants */

typedef int                stdbool;

#define stdtrue            1
#define STDTRUE            1

#define stdfalse           0
#define STDFALSE           0

typedef int                stdcode;  /* a return code (non-zero -> abnormal operation) */

typedef signed char        stdschar;
typedef unsigned char      stduchar;

typedef unsigned short     stdushort;
typedef unsigned int       stduint;
typedef unsigned long      stdulong;

typedef signed char        stdint8;
typedef unsigned char      stduint8;

#define STDINT8_MAX        SCHAR_MAX
#define STDINT8_MIN        SCHAR_MIN
#define STDUINT8_MAX       UCHAR_MAX

typedef short              stdint16;
typedef unsigned short     stduint16;

#define STDINT16_MAX       SHRT_MAX
#define STDINT16_MIN       SHRT_MIN
#define STDUINT16_MAX      USHRT_MAX

typedef stdarch_int32      stdint32;
typedef stdarch_uint32     stduint32;

#define STDINT32_MAX       ((stdint32) ((stduint32) STDINT32_MIN - 1))
#define STDINT32_MIN       ((stdint32) ((stduint32) 0x1 << 31))
#define STDUINT32_MAX      ((stduint32) -1)

typedef stdarch_int64      stdint64;
typedef stdarch_uint64     stduint64;

#define STDINT64_MAX       ((stdint64) ((stduint64) STDINT64_MIN - 1))
#define STDINT64_MIN       ((stdint64) ((stduint64) 0x1 << 63))
#define STDUINT64_MAX      ((stduint64) -1)

typedef size_t             stdsize;
typedef stdarch_ssize      stdssize;  /* a signed version of size_t */

#define STDSSIZE_MAX       ((stdssize) ((stdsize) STDSSIZE_MIN - 1))
#define STDSSIZE_MIN       ((stdssize) ((stdsize) 0x1 << (sizeof(stdssize) * 8 - 1))
#define STDSIZE_MAX        ((stdsize) -1)

/* define comparison fcn types */

typedef stduint32          stdhcode;

typedef int                (*stdcmp_fcn)(const void *v1, const void *v2);
typedef stdhcode           (*stdhcode_fcn)(const void *v);

/* define some common, if dangerous, macros */

#define STDSWAP(x, y, t)   ((t) = (x), (x) = (y), (y) = (t))
#define STDMAX(x, y)       ((x) < (y) ? (y) : (x))
#define STDMIN(x, y)       ((x) > (y) ? (y) : (x))

/* define some useful constants */

#define STD1THOUSAND       1000
#define STD1MILLION        1000000L
#define STD1BILLION        1000000000L

/* inline rules vary too much from language to language (C89, C99,
   C++) and compiler to compiler (gcc, .NET, etc.) in non-standard
   usage (i.e. - inline in C89) for stdutil to use any one inline
   syntax.  The way to get inline to work as it should with stdutil is
   to compile all of your project in one translation unit, either
   #define or -D an appropriate value for STDINLINE and THEN #include
   <stdutil/...> which properly includes ALL of stdutil's code.
   Because all of your code will be in a single translation unit, the
   compiler can then do a good job of inline'ing stdutil fcn calls,
   which it can't do well if stdutil is a library.
*/

#ifndef STDINLINE
#  define STDINLINE
#endif

/* macros used for ensuring proper byte alignment of dynamic types in memory */

#define STDARCH_PADDING(x)     ((STDARCH_MAX_BYTE_ALIGNMENT - ((x) & (STDARCH_MAX_BYTE_ALIGNMENT - 1))) & (STDARCH_MAX_BYTE_ALIGNMENT - 1))
#define STDARCH_PADDED_SIZE(x) ((x) + STDARCH_PADDING(x))

#ifdef __cplusplus
}
#endif

#endif
