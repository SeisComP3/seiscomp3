/*   Lib330 Control Detector Definitions
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
#ifndef libctrldet_h
/* Flag this file as included */
#define libctrldet_h
#define VER_LIBCTRLDET 3

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

extern void expand_control_detectors (paqstruc paqs) ;
extern void evaluate_detector_stack (pq330 q330, plcq q) ;
extern enum tliberr lib_ctrlstat (pq330 q330, tctrlstat *ctrlstat) ;
#endif

#endif
