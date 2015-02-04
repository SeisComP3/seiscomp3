/*   Lib330 Status Dump Definitions
     Copyright 2006-2010 Certified Software Corporation

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
    0 2006-10-01 rdr Created
*/
#ifndef libverbose_h
/* Flag this file as included */
#define libverbose_h
#define VER_LIBVERBOSE 6

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef q330types_h
#include "q330types.h"
#endif

extern void log_all_info (pq330 q330) ;

#ifndef OMIT_SEED
extern longword print_generated_rectotals (pq330 q330) ;
#endif

#endif
