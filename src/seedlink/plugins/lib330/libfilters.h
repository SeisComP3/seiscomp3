/*   Lib330 Filter Definitions
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
#ifndef libfilters_h
/* Flag this file as included */
#define libfilters_h
#define VER_LIBFILTERS 2

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif

#ifndef OMIT_SEED
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
typedef double *pdouble ;

extern void load_firfilters (pq330 q330, paqstruc paqs) ;
extern void append_firfilters (pq330 q330, paqstruc paqs, pfilter src) ;
extern pfir_packet create_fir (pq330 q330, pfilter src) ;
extern piirfilter create_iir (pq330 q330, piirdef src, integer points) ;
extern void average (paqstruc paqs, pavg_packet pavg, tfloat s, tfloat samp, plcq q) ;
extern void allocate_lcq_filters (paqstruc paqs, plcq q) ;
extern tfloat mac_and_shift (pfir_packet pf) ;
extern pfilter find_fir (paqstruc paqs, byte num) ;
extern piirdef find_iir (paqstruc paqs, byte num) ;
extern double multi_section_filter (piirfilter resp, double s) ;
extern void calc_section (tsection_base *sect) ;
extern void bwsectdes (pdouble a, pdouble b, integer npoles, boolean high, tfloat ratio) ;
#endif

#endif
