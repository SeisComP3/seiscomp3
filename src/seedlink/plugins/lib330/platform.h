/*   Platform specific system includes and definitions
     Copyright 2006-2007 Certified Software Corporation

    This file is part of Lib330

    Lib330 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Lib330 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lib330; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Edit History:
   Ed Date       By  Changes
   -- ---------- --- ---------------------------------------------------
    0 2006-09-10 rdr Created
    1 2006-11-01 hjs Added support for linux and solaris
    2 2007-07-28 rdr Add AVR32/CMEX32 support.
    3 2007-08-21 rdr/jms add BALER44 (ARM-LINUX) support
    4 2007-09-04 rdr Fix syntax error in BALER44 section starting at line 41.
    5 2008-01-29 rdr Make integer same size as pointer.
    6 2009-09-17 rdr Add DOUBLE_HYBRID_ENDIAN for platforms with mixed endians
                     for double types.
    7 2010-02-18 fcs Slate computer needs same platform settings as BALER44
    8 2012-02-08 dsn/rdr Added configuration for ARM-LINUX Big Endian (ARMEB).
    9 2013-08-18 rdr For Unix definitions move OMIT_SERIAL conditional to after unistd.h.
*/
#ifndef platform_h
#define platform_h

#include <stdio.h>
#include <math.h>

#if defined(__SVR4) && defined(__sun)
#  define solaris
#  include <sys/filio.h>
#endif

#if defined(BALER44) || (defined(__arm__) && defined(linux))
  #define ARM_UNIX32
#endif

#if defined(ARM_UNIX32)
#ifndef __ARMEB__
#    define ENDIAN_LITTLE
#    define DOUBLE_HYBRID_ENDIAN
#endif
#  include <sys/time.h>
#elif defined(linux) || defined(solaris)
#  if defined(__i386__) || defined(__x86_64__)
#    define X86_UNIX32
#    define ENDIAN_LITTLE
#  elif defined(__x86_64__)
#    define X86_UNIX64
#    define ENDIAN_LITTLE
#  else
#    define SPARC_UNIX32
#  endif
#  include <sys/time.h>
#  define OMIT_SERIAL
#endif


#if defined (X86_WIN32)

#include <winsock2.h>
#include <windows.h>
#include <winbase.h>

#define boolean unsigned __int8 /* 8 bit unsigned, 0 or non-zero */
#define shortint __int8 /* 8 bit signed */
#define byte unsigned __int8 /* 8 bit unsigned */
#define int16 __int16 /* 16 bit signed */
#define word unsigned __int16 /* 16 bit unsigned */
#define longint __int32 /* 32 bit signed */
#define longword unsigned __int32 /* 32 bit unsigned */
#define integer __int32 /* 32 bit signed */
#define uninteger unsigned __int32 /* 32 bit unsigned */
#define single float /* 32 bit floating point */
typedef HANDLE tfile_handle ;
typedef struct _stat tfile_state ;
#define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
#define INVALID_IO_HANDLE INVALID_HANDLE_VALUE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ECONNRESET WSAECONNRESET
#define ENOBUFS WSAENOBUFS
#define EINPROGRESS WSAEINPROGRESS
#define ECONNABORTED WSAECONNABORTED
#define ENDIAN_LITTLE

#elif defined(X86_UNIX32) || defined(SPARC_UNIX32) || defined(X86_UNIX64) || defined(ARM_UNIX32)

#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h>     // required for ARM Linux. maybe for others as well.
#ifndef OMIT_SERIAL
#include <sys/termios.h>
#endif

#define boolean uint8_t /* 8 bit unsigned, 0 or non-zero */
#define shortint int8_t /* 8 bit signed */
#define byte uint8_t /* 8 bit unsigned */
#define int16 int16_t /* 16 bit signed */
#define word uint16_t /* 16 bit unsigned */
#define longint int32_t /* 32 bit signed */
#define longword uint32_t /* 32 bit unsigned */
#if defined(X86_UNIX64) || defined(__x86_64__)
#define integer int64_t /* 64 bit signed, same size as pointer */
#define uninteger uint64_t /* 64 bit unsigned, same size as pointer */
#else
#define integer int32_t /* 32 bit signed, same size as pointer */
#define uninteger uint32_t /* 32 bit unsigned, same size as pointer */
#endif
#define single float /* 32 bit floating point */
typedef integer tfile_handle ;
typedef struct stat tfile_state ;
#define INVALID_FILE_HANDLE -1
#define INVALID_IO_HANDLE -1
#define FALSE 0
#define TRUE 1

#elif defined(__APPLE__)

#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netdb.h>
#include <signal.h>

#ifndef OMIT_SERIAL
#include <fcntl.h>
#include <sys/termios.h>
#endif

#define boolean uint8_t /* 8 bit unsigned, 0 or non-zero */
#define shortint int8_t /* 8 bit signed */
#define byte uint8_t /* 8 bit unsigned */
#define int16 int16_t /* 16 bit signed */
#define word uint16_t /* 16 bit unsigned */
#define longint int32_t /* 32 bit signed */
#define longword uint32_t /* 32 bit unsigned */
#if defined(__x86_64__)
#define integer int64_t /* 64 bit signed, same size as pointer */
#define uninteger uint64_t /* 64 bit unsigned, same size as pointer */
#else
#define integer int32_t /* 32 bit signed, same size as pointer */
#define uninteger uint32_t /* 32 bit unsigned, same size as pointer */
#endif
#define single float /* 32 bit floating point */
typedef integer tfile_handle ;
typedef struct stat tfile_state ;
#define INVALID_FILE_HANDLE -1
#define INVALID_IO_HANDLE -1
#define FALSE 0
#define TRUE 1
#ifdef __LITTLE_ENDIAN__
#define ENDIAN_LITTLE
#endif

#elif defined(CMEX32)

#include <string.h>
#include <stdlib.h>

#define boolean unsigned char /* 8 bit unsigned, 0 or non-zero */
#define shortint signed char /* 8 bit signed */
#define byte unsigned char /* 8 bit unsigned */
#define int16 short /* 16 bit signed */
#define word unsigned short /* 16 bit unsigned */
#define longint long /* 32 bit signed */
#define longword unsigned long /* 32 bit unsigned */
#define integer int /* 32 bit signed */
#define uninteger unsigned int /* 32 bit unsigned */
#define single float /* 32 bit floating point */
#define FALSE 0
#define TRUE 1
#define INVALID_FILE_HANDLE -1
#define INVALID_IO_HANDLE -1
#if defined(__AVR32_AP7000__)
#define CPU_HZ 20000000 /* Osc0 direct */
#elif defined(__AVR32_UC3A0512__)
#ifdef FULLBLAST
#define CPU_HZ 24000000 /* PLL0 */
#else
#define CPU_HZ 12000000 /* Osc0 direct */
#endif
#endif
typedef integer tfile_handle ;
typedef void *pvoid ;

#include "cmexnix.h"

#endif

/* at least on solaris, this is undefined */
#ifndef INADDR_NONE
#  define INADDR_NONE (longword) -1
#endif

#endif
