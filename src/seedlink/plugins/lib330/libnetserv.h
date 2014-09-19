/*   Lib330 Netserver (LISS) definitions
     Copyright 2006 Certified Software Corporation

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
    1 2007-03-07 rdr pbuf declaration fixed for lib_ns_send.
*/
#ifndef libnetserv_h
/* Flag this file as included */
#define libnetserv_h
#define VER_LIBNETSERV 2

#ifndef OMIT_SEED
/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
/* Make sure libseed.h is included */
#ifndef libseed_h
#include "libseed.h"
#endif

#define MAX_NETWHITE 10
#define MAX_NS_BUFFERS 9800 /* 5.0MB as shown in station manager */

typedef completed_record tnsbuf[MAX_NS_BUFFERS] ;
typedef struct {
  longword lowip, highip ;
} twhitelist ;
typedef struct { /* creation parameters for one netserver */
  word ns_port ; /* TCP port number */
  integer server_number ;
  integer whitecount ;
  integer sync_time ;
  integer record_count ; /* number of records allocated */
  pointer stnctx ; /* station context */
  tnsbuf *nsbuf ; /* pointer to circular buffer */
  twhitelist whitelist[MAX_NETWHITE] ;
} tns_par ;

extern pointer lib_ns_start (tns_par *nspar) ;
extern void lib_ns_stop (pointer ct) ;
extern void lib_ns_send (pointer ct, pcompleted_record pbuf) ;

#endif
#endif
