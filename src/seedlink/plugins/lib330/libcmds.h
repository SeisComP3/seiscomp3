/*   Lib330 Command Processing headers
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
#ifndef libcmds_h
/* Flag this file as included */
#define libcmds_h
#define VER_LIBCMDS 10

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

extern void lib_start_registration (pq330 q330) ;
extern void lib_continue_registration (pq330 q330) ;
extern void lib_start_ping (pq330 q330) ;
extern void lib_timer (pq330 q330) ;
extern void purge_cmdq (pq330 q330) ;
extern void lib_command_response (pq330 q330, pbyte pb) ;
extern void start_deallocation (pq330 q330) ;
extern void new_cmd (pq330 q330, byte ncmd, word sz) ;

#endif
