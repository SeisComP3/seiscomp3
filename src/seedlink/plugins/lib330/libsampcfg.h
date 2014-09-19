/*   Lib330 time series configuration definitions
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
#ifndef libsampcfg_h
/* Flag this file as included */
#define libsampcfg_h
#define VER_LIBSAMPCFG 6

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif

extern void clear_sg (paqstruc paqs) ;
extern void deallocate_sg (paqstruc paqs) ;
extern pointer allocate_aqstruc (tcontext ownedby) ;
extern void clear_calstat (pq330 q330) ;
extern void set_gaps (plcq q) ;
extern void deallocate_dplcqs (pq330 q330) ;
extern void init_lcq (paqstruc paqs) ;
extern void init_dplcq (paqstruc paqs, plcq pl, boolean newone) ;
extern void init_dplcqs (paqstruc paqs) ;
extern void verify_mapping (pq330 q330) ;
extern char *realtostr (double r, integer digits, string31 *result) ;
extern longword secsince (void) ;
extern enum tliberr lib_commevents (pq330 q330, tcommevents *commevents) ;
extern enum tliberr lib_getdpcfg (pq330 q330, tdpcfg *dpcfg) ;

#ifndef OMIT_SEED
extern enum tliberr lib_lcqstat (pq330 q330, tlcqstat *lcqstat) ;
extern void lib_setcommevent (pq330 q330, integer number, boolean seton) ;
#endif

#endif
