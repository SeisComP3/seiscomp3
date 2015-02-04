/*   Lib330 Continuity definitions
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
    1 2007-03-05 rdr Add purge_continuity and purge_thread_continuity.
*/
#ifndef libcont_h
/* Flag this file as included */
#define libcont_h
#define VER_LIBCONT 11

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

extern void restore_thread_continuity (pq330 q330, boolean pass1, string *result) ;
extern void save_continuity (pq330 q330) ;
extern void save_thread_continuity (pq330 q330) ;
extern void check_continuity (pq330 q330) ;
extern boolean restore_continuity (pq330 q330) ;
extern void purge_continuity (pq330 q330) ;
extern void purge_thread_continuity (pq330 q330) ;
extern void continuity_timer (pq330 q330) ;
#endif
