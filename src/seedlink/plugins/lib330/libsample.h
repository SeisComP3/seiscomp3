/*   Lib330 Time Series data routine definitions
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
#ifndef libsample_h
/* Flag this file as included */
#define libsample_h
#define VER_LIBSAMPLE 11

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif

extern longint sex (longint l) ;
extern longint bsex (longint l) ;
extern void process_one (pq330 q330, longint data) ;
extern void process_comp (pq330 q330, pbyte p, integer size) ;
extern void process_mult (pq330 q330, pbyte psave, longword seq) ;
extern void process_variable (pq330 q330, integer sps, integer dly5ms, longint data) ;
extern longint seqspread (longword new_, longword last) ;
extern word translate_clock (tclock *qclock, word qual, word loss) ;

#ifndef OMIT_SEED
extern void finish_record (paqstruc paqs, plcq q, pcom_packet pcom) ;
extern void flush_lcq (paqstruc paqs, plcq q, pcom_packet pcom) ;
extern void flush_lcqs (paqstruc paqs) ;
extern void flush_dplcqs (pq330 q330) ;
extern void add_blockette (paqstruc paqs, plcq q, pword pw, double time) ;
extern void send_to_client (paqstruc paqs, plcq q, pcompressed_buffer_ring pbuf, byte dest) ;
extern void build_separate_record (paqstruc paqs, plcq q, pword pw,
                                   double time, enum tpacket_class pclass) ;
#endif

#endif
