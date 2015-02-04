/*   Lib330 Archival Miniseed Definitions
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
    0 2006-10-11 rdr Created
*/
#ifndef libarchive_h
/* Flag this file as included */
#define libarchive_h
#define VER_LIBARCHIVE 5

#ifndef OMIT_SEED
/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

extern void flush_archive (paqstruc paqs, plcq q) ;
extern void archive_512_record (paqstruc paqs, plcq q, pcompressed_buffer_ring pbuf) ;
extern void preload_archive (pq330 q330, boolean from330, plcq onelcq) ;
#endif

#endif
