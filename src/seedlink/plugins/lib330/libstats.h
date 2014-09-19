/*   Lib330 Statistics headers
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
    0 2006-09-29 rdr Created
*/
#ifndef libstats_h
/* Flag this file as included */
#define libstats_h
#define VER_LIBSTATS 3

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

extern void add_status (pq330 q330, enum tacctype acctype, longword count) ;
extern void lib_stats_timer (pq330 q330) ;
extern void update_op_stats (pq330 q330) ;
extern void update_gps_stats (pq330 q330) ;
extern void process_gps_phase_change (pq330 q330, byte newphase, word coldreason) ;
#endif
