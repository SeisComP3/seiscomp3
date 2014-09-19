/***************************************************************************
 * slplatform.h:
 * 
 * Platform specific headers.  This file provides a basic level of platform
 * portability.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License (GNU-LGPL) for more details.  The
 * GNU-LGPL and further information can be found here:
 * http://www.gnu.org/
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified: 2008.078
 ***************************************************************************/

#ifndef SLPLATFORM_H
#define SLPLATFORM_H 1

#ifdef __cplusplus
extern "C" {
#endif


  /* Portability to the XScale (ARM) architecture
   * requires a packed attribute in certain places
   * but this only works with GCC for now.
   */

#if defined (__GNUC__)
  #define SLP_PACKED __attribute__ ((packed))
#else
  #define SLP_PACKED
#endif  

  /* Make some guesses about the system libraries based
   * on the architecture.  Currently the assumptions are:
   * Linux => glibc2 (SLP_GLIBC2)
   * Sun => Solaris (SLP_SOLARIS)
   * WIN32 => WIN32 and Windows Sockets 2 (SLP_WIN32)
   * Apple => Mac OS X (SLP_DARWIN)
   */

#if defined(__linux__) || defined(__linux)
  #define SLP_GLIBC2 1

  #include <stdlib.h>
  #include <unistd.h>
  #include <stdarg.h>
  #include <inttypes.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <sys/time.h>
   
#elif defined(__sun__) || defined(__sun)
  #define SLP_SOLARIS 1

  #include <stdlib.h>
  #include <unistd.h>
  #include <stdarg.h>
  #include <inttypes.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <sys/time.h>

#elif defined(WIN32)
  #define SLP_WIN32 1

  #include <windows.h>
  #include <stdarg.h>
  #include <winsock.h>
  #include <io.h>

  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strncasecmp _strnicmp

  typedef signed char int8_t;
  typedef unsigned char uint8_t;
  typedef signed short int int16_t;
  typedef unsigned short int uint16_t;
  typedef signed int int32_t;
  typedef unsigned int uint32_t;
  typedef signed __int64 int64_t;
  typedef unsigned __int64 uint64_t;

#elif defined(__APPLE__)
  #define SLP_DARWIN 1

  #include <stdlib.h>
  #include <unistd.h>
  #include <stdarg.h>
  #include <inttypes.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <sys/time.h>

#else
  typedef signed char int8_t;
  typedef unsigned char uint8_t;
  typedef signed short int int16_t;
  typedef unsigned short int uint16_t;
  typedef signed int int32_t;
  typedef unsigned int uint32_t;
  typedef signed long long int64_t;
  typedef unsigned long long uint64_t;

#endif

extern int slp_sockstartup (void);
extern int slp_sockconnect (int sock, struct sockaddr * inetaddr, int addrlen);
extern int slp_sockclose (int sock);
extern int slp_socknoblock (int sock);
extern int slp_noblockcheck (void);
extern int slp_getaddrinfo (char * nodename, char * nodeport, 
			    struct sockaddr * addr, size_t * addrlen);
extern int slp_openfile (const char *filename, char perm);
extern const char *slp_strerror(void);
extern double slp_dtime(void);
extern void slp_usleep(unsigned long int useconds);

#ifdef __cplusplus
}
#endif
 
#endif /* SLPLATFORM_H */
