/*   Lib330 MD5 Definitions
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
    0 2006-09-30 rdr Created
*/
#ifndef libmd5_h
/* Flag this file as included */
#define libmd5_h
#define VER_LIBMD5 3

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

typedef byte tdigest[16] ;
typedef longint tdigestlongint[4] ;
typedef longint tlongintbuf[16] ;
typedef byte tbytebuf[64] ;

extern char *dig2str (t64 *d, string63 *result) ;
extern void calcmd5 (pq330 q330, string250 *chal, t128 *resp) ;
extern void init_md5_buffer (pq330 q330) ;
extern void md5_operation (pq330 q330, tmd5op *md5op) ;
#endif
