/*   Lib330 Seed Compression definitions
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
    0 2006-10-06 rdr Created
*/
#ifndef libcompress_h
/* Flag this file as included */
#define libcompress_h
#define VER_LIBCOMPRESS 1

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif

extern integer decompress_blockette (paqstruc paqs, plcq q) ;
#ifndef OMIT_SEED
extern integer compress_block (paqstruc paqs, plcq q, pcom_packet pcom) ;
extern integer build_blocks (paqstruc paqs, plcq q, pcom_packet pcom) ;
extern void no_previous (paqstruc paqs) ;
#endif

#endif
