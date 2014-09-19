/*   Lib330 Opaque Blockette Definitions
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
#ifndef libopaque_h
/* Flag this file as included */
#define libopaque_h
#define VER_LIBOPAQUE 4

#ifndef OMIT_SEED
/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif

#ifndef libsampglob_h
#include "libsampglob.h"
#endif
extern void start_cfgblks (paqstruc paqs) ;
extern void add_cfg (paqstruc paqs, string7 *name, pointer buf, integer size,
                     integer num, byte flags) ;
extern void flush_cfgblks (paqstruc paqs, boolean final) ;
extern void process_cnp (pq330 q330, pbyte pb) ;
extern void flush_cnp (paqstruc paqs, plcq q, pcom_packet pcom) ;
#endif

#endif
