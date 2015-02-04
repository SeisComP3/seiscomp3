/*   Lib330 DSS definitions
     Copyright 2009 Certified Software Corporation

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
    0 2009-07-28 rdr Created.
*/
#ifndef libdss_h
/* Flag this file as included */
#define libdss_h
#define VER_LIBDSS 6

#ifndef OMIT_SEED

#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libseed_h
#include "libseed.h"
#endif

extern void lib_dss_start (tdss *dss, pointer ct, word host_size_limit) ;
extern void lib_dss_stop (pointer ct) ;
extern void lib_dss_timer (pointer ct) ;
extern void get_dss_server_display (pointer ct, string63 *result) ;
extern void lib_dss_continuous (pointer ct) ;
extern void lib_dss_read (pointer ct) ;

#endif
#endif
