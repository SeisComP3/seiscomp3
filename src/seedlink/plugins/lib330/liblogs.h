/*   Lib330 Message Log definitions
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
    1 2006-11-30 rdr Fix definition to be liblogs_h, not liblog_h.
    2 2007-01-08 hjs prefaced some functions with lib330_ to avoid collisions
*/
#ifndef liblogs_h
/* Flag this file as included */
#define liblogs_h
#define VER_LIBLOGS 3

#ifndef OMIT_SEED
/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif

enum tclock_exception {CE_VALID, CE_DAILY, CE_JUMP} ;

extern void lib330_padright (string *s, pchar b, integer fld) ;
extern void log_clock (pq330 q330, enum tclock_exception clock_exception, string95 *jump_amount) ;
extern void finish_log_clock (pq330 q330) ;
extern void logevent (paqstruc paqs, pdet_packet det, tonset_mh *onset) ;
extern void log_cal (pq330 q330, pbyte pb) ;
extern void log_message (pq330 q330, string *msg) ;
extern void flush_messages (paqstruc paqs) ;
extern void flush_timing (paqstruc paqs) ;
#endif

#endif
