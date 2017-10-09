/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

#ifndef INC_ARCH
#define INC_ARCH

/*
 * Each record in this file represents an architecture.
 * Each record contains the following fields:
 *
 *      #define         INTSIZE{16,32,64}
 *      #define         ARCH_SCATTER_{CONTROL,ACCRIGHTS,NONE}
 *      #define         ARCH_ENDIAN{0x00000000,0x80000080}
 *      #define         LOC_INLINE { __inline__ or blank }
 *      #define         ARCH_SCATTER_SIZE { sys dependent variable }
 *      #define         HAVE_GOOD_VARGS ( exists if true )
 *      #define         HAVE_LRAND48 ( exists if true )
 *      #define         HAVE_STDINT_H   ( exists if true --currently glibc2.1 needs it )
 *      typedef         {sys dependent type} sockopt_len_t;
 *      #define         ERR_TIMEDOUT    EAGAIN
 *      #define         sock_errno { errno or WSAGetLastError() for windows }
 *      #define         sock_strerror { strerror or sock_strerror for windows }
 *      #define         sock_set_errno { sock_unix_set_errno or WSASetLastError for windows }
 */

#undef          INTSIZE32
#undef          INTSIZE64
#undef          INTSIZE16

#ifndef ARCH_PC_WIN95
/* If we aren't using windows... we can use autoconf */

#  include "config.h"

#  ifdef WORDS_BIGENDIAN
#    define ARCH_ENDIAN 0x00000000
#  else
#    define ARCH_ENDIAN 0x80000080
#  endif
  
#  define LOC_INLINE __inline__
  
#  ifndef __GNUC__
#    define __inline__ inline
#  endif

/* Need to add special cases, SUNOS gets 64, IRIX gets 512 */  
#  ifdef MSG_MAXIOVLEN
#    define ARCH_SCATTER_SIZE MSG_MAXIOVLEN
#  else
#    define ARCH_SCATTER_SIZE 1024
#  endif

#  define HAVE_GOOD_VARGS
  
#  ifndef ERR_TIMEDOUT
#    define ERR_TIMEDOUT ETIMEDOUT
#  endif
  
#  ifndef RAND_MAX
#    define RAND_MAX 2147483647
#  endif
  
#  define sock_errno        errno
#  define sock_strerror     strerror
#  define sock_set_errno(a) (errno = (a)) 
  
#  ifndef byte
#    define byte u_int8_t
#  endif

#  ifndef int16
#    define int16 int16_t
#  endif

#  ifndef int16u
#    define int16u u_int16_t
#  endif

#  ifndef int32
#    define int32 int32_t
#  endif

#  ifndef int32u
#    define int32u u_int32_t
#  endif
  
#else
/* We are using windows... */
#  define INTSIZE32
#  define ARCH_SCATTER_NONE
#  define ARCH_ENDIAN 0x80000080
#  define LOC_INLINE      
#  define BADCLOCK
#  define HAVE_GOOD_VARGS

/* Windows now has a strerror function and if we do not use it 
 * compile errors occur with shared DLL libraries. 
 */
#  define HAVE_STRERROR
#  define ARCH_SCATTER_SIZE 64
#  define ERR_TIMEDOUT      EAGAIN
#  define sock_errno        WSAGetLastError()
#  define sock_set_errno    WSASetLastError
#  define MAXPATHLEN        _MAX_PATH
#  define snprintf          _snprintf
#  define alloca            _alloca

/* Sockets are not file descriptors on windows so they need a special close function. */
#  define close closesocket

/* Windows defines a default size of 64. However, the size of fd_set array for select
 * can be raised by defining a larger constant before including windows.h winsock.h
 */
#  define FD_SETSIZE 1024

#  include <ws2tcpip.h>   /* after definition of FD_SETSIZE! */

#  define HAVE_SOCKLEN_T 1
typedef int sockopt_len_t;

/* System location of spread.conf file */
#  define         SPREAD_ETCDIR   "/etc"

/* Use winsock constants since we are dealing with sockets
 * Note: If we ever need file IO with errno's we will have conflicts
 * since the WSA version and the basic E versions may not have the same
 * number. Right now we don't need the E versions for windows so we just
 * use the WSA versions.
 */
#  undef EINTR
#  define EINTR WSAEINTR

#  undef EAGAIN
#  define EAGAIN WSAEWOULDBLOCK

#  undef EWOULDBLOCK
#  define EWOULDBLOCK WSAEWOULDBLOCK

#  undef EINPROGRESS
#  define EINPROGRESS WSAEINPROGRESS

#  undef EALREADY
#  define EALREADY WSAEALREADY

/* Windows does not define MAXHOSTNAMELEN, so we define it here to a reasonable host name limit */
#define MAXHOSTNAMELEN 256

/* byte is already defined as a typedef to unsigned char on Windows XP (and probably earlier) so do not define 
 * #ifndef byte
 * #define byte unsigned char
 * #endif
 */

#  ifndef int16
#    define int16 short
#  endif

#  ifndef int16u
#    define int16u unsigned short
#  endif

#  ifndef int32
#    define int32 int
#  endif

#  ifndef int32u
#    define int32u unsigned int
#  endif

#  ifndef UINT32_MAX
#    define UINT32_MAX UINT_MAX
#  endif

#  ifndef INT32_MAX
#    define INT32_MAX INT_MAX
#  endif

#  ifndef int64_t
#    define int64_t __int64
#  endif

#  define PRId64 "I64d"

/* Declare functions from arch.c */
char *sock_strerror(int err);

#endif /* ARCH_PC_WIN95 */

/* Pick which rand version to use */
#ifdef HAVE_LRAND48
#  define get_rand lrand48
#else
#  define get_rand rand
#endif

/* Useful CPP macros to make strings from #defines */  
#define Q(x)    # x
#define QQ(x)   Q(x)

#define         ENDIAN_TYPE             0x80000080

#define         Get_endian( t )      ( (t) &  ENDIAN_TYPE )
#define         Set_endian( t )      ( ( (t) & ~ENDIAN_TYPE ) | ARCH_ENDIAN )
#define         Same_endian( t )     ( ( (t) & ENDIAN_TYPE ) == ARCH_ENDIAN )
#define         Clear_endian( t )    ( (t) & ~ENDIAN_TYPE )

#ifndef         Flip_int16
#  define       Flip_int16( t )      ( ( ((t) >> 8) & 0x00ff) | ( ((t) << 8) & 0xff00) )
#endif

#ifndef         Flip_int32
#  define       Flip_int32( t )      ( ( ((t) >> 24) & 0x000000ff) | ( ((t) >> 8) & 0x0000ff00) | ( ((t) << 8) & 0x00ff0000) | ( ((t) << 24) & 0xff000000) )
#endif

#ifndef         Flip_int64
#  define       Flip_int64( t ) ( \
  (((t) & ((int64_t) 0xff <<  0)) << 56) | (((t) & ((int64_t) 0xff <<  8)) << 40) | \
  (((t) & ((int64_t) 0xff << 16)) << 24) | (((t) & ((int64_t) 0xff << 24)) <<  8) | \
  (((t) & ((int64_t) 0xff << 32)) >>  8) | (((t) & ((int64_t) 0xff << 40)) >> 24) | \
  (((t) & ((int64_t) 0xff << 48)) >> 40) | (((t) & ((int64_t) 0xff << 56)) >> 56) )
#endif
 

#define         channel                 int
#define         mailbox                 int

typedef struct  dummy_membership_id {
        int32   proc_id;
        int32   time;
} membership_id;

typedef struct  dummy_group_id {
        membership_id   memb_id;
        int32           index;
} group_id;

/* 
 * General Useful Types
 */

#ifndef __cplusplus
/*
 * May want to just undef it as shown below. Test to see what we want. 
 * work around bad/different bool definitions in system headers
 * #undef bool     
 */
typedef         short           bool;
#endif

#ifndef TRUE
#  define         TRUE            1
#endif

#ifndef FALSE
#  define         FALSE           0
#endif

#endif  /* INC_ARCH */
