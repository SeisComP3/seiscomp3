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

#include <string.h>
#include <netdb.h>

#ifndef ARCH_PC_WIN95
#  include <net/if.h>
#  include <sys/ioctl.h>
#  include <netinet/in.h>
#  ifdef sun
#    include <sys/sockio.h>  /* for SIOCGIFCONF */
#  endif
#endif

#include "ip_enum.h"
#include "spu_alarm.h"

#define MAX_IF 8192     /* max interfaces to look up */
#define MIN_IF 10       /* must be able to look up at least this many */

#ifdef ARCH_PC_WIN95
typedef INTERFACE_INFO If_info;
#else
typedef struct ifreq If_info;
#  define closesocket(s) close(s)
#endif

#if defined(_SIZEOF_ADDR_IFREQ)
#  define NEXT_IF(ifr)  (struct ifreq*)((char*)(ifr) + _SIZEOF_ADDR_IFREQ(*(ifr)))
#else
/* Assume fixed-length IFR's.  WARNING: this may not be true for
   all systems!  If it has ifr_addr.sa_len, then we need to compute
   ifr + sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len
*/
#  define NEXT_IF(ifr)  ((ifr) + 1)
#endif

struct Ip_Array 
{
  int32u *ips;
  int     size;
  int     next;
};

static void Add_ip(struct Ip_Array *array, int32u ip)
{
  void *new_array;

  if (array->next >= array->size) {

    array->size = (array->size == 0 ? 4 : 2 * array->size);

    if ((new_array = realloc(array->ips, array->size * sizeof(int32u))) == NULL) {
      Alarm(EXIT, "Ip_enum_all: Failed to allocate memory\n");
    }

    array->ips = (int32u*) new_array;
  }

  array->ips[array->next++] = ip;
}

int32u *Ip_enum_all()
{
  struct Ip_Array array = { 0, 0, 0 };

  struct hostent *host_ptr;
  char            machine_name[256] = { 0 };
  int32u          addr;

  int             s;
  void           *buffer;
  int             bufferSize;
  int             n;

#ifdef  ARCH_PC_WIN95
  DWORD           bytesReturned;
  INTERFACE_INFO *ifr;
#else
  struct ifconf   ifc;
  struct ifreq   *end;
  struct ifreq   *ifr;
#endif

  /* get IPs from hostname */

  if (gethostname(machine_name, sizeof(machine_name)) == 0) {
    host_ptr = gethostbyname(machine_name);

    if (host_ptr != NULL && host_ptr->h_addrtype == AF_INET && host_ptr->h_length == 4) {
      
      for (n = 0; host_ptr->h_addr_list[n] != NULL; n++) {
	
	memcpy(&addr, host_ptr->h_addr_list[n], sizeof(addr));
	
	if (addr != 0) {
	  Add_ip(&array, addr);
	}
      }
    }
  }

  /* get IPs from interfaces */

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) >= 0) {

    bufferSize = sizeof(If_info) * MAX_IF;
    if ((buffer = malloc(bufferSize)) == NULL) {
      Alarm(EXIT, "Ip_enum_all: Failed to allocate memory\n");
    }

#ifdef ARCH_PC_WIN95
    if (WSAIoctl(s, SIO_GET_INTERFACE_LIST, 0, 0, buffer, bufferSize,
		 &bytesReturned, 0, 0) == SOCKET_ERROR) {
      Alarm(EXIT, "Ip_enum_all: Error getting interface list: %d\n", sock_errno);
    }
    
    ifr = (INTERFACE_INFO*)buffer;

    for (n = bytesReturned / sizeof(INTERFACE_INFO); n; ifr++, n--) {
      struct sockaddr_in *sa = &ifr->iiAddress.AddressIn;

      if (sa->sin_family == AF_INET && sa->sin_addr.s_addr != 0) {
	Add_ip(&array, sa->sin_addr.s_addr);
      }
    }

#else   /* !ARCH_PC_WIN95 */

    ifc.ifc_len = bufferSize;
    ifc.ifc_buf = buffer;

    while (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
#  ifdef __linux__
      /* Workaround for a failure to allocate kernel memory
	 of the same size as our 'buffer'. */
      if (errno == ENOMEM) {
	if (bufferSize / 2 >= MIN_IF * (int)sizeof(struct ifreq)) {
	  bufferSize /= 2;

	  ifc.ifc_len = bufferSize;
	  ifc.ifc_buf = buffer;
	  continue;
	}
      }
#  endif
      Alarm(EXIT, "Ip_enum_all: Error enumerating IP addresses: %d\n", sock_errno);
    }

    end = (struct ifreq*)(ifc.ifc_buf + ifc.ifc_len - (sizeof(struct ifreq) - 1));

    for (ifr = ifc.ifc_req;  ifr < end; ifr = NEXT_IF(ifr)) {
      struct sockaddr_in *sa = (struct sockaddr_in*)&ifr->ifr_addr;

      if (sa->sin_family == AF_INET && sa->sin_addr.s_addr != 0) {
	Add_ip(&array, sa->sin_addr.s_addr);
      }
    }
#endif

    closesocket(s);
    free(buffer);
  }

  Add_ip(&array, 0);   /* 0.0.0.0 ends the array */

  return array.ips;
}
