/*   Lib330 host <-> native conversion definitions
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
    0 2006-09-28 rdr Created
    1 2007-01-08 hjs Prefaced some functions with lib330_ to avoid collisions
*/
#ifndef libcvrt_h
/* Flag this file as included */
#define libcvrt_h
#define VER_LIBCVRT 2

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif

extern void storebyte (pbyte *p, byte b) ;
extern void storeword (pbyte *p, word w) ;
extern void storeint16 (pbyte *p, int16 i) ;
extern void storelongword (pbyte *p, longword lw) ;
extern void storelongint (pbyte *p, longint li) ;
extern void storesingle (pbyte *p, single s) ;
extern void storestring (pbyte *p, integer fieldwidth, string *s) ;
extern void storeblock (pbyte *p, integer size, pointer psrc) ;
extern byte loadbyte (pbyte *p) ;
extern word loadword (pbyte *p) ;
extern int16 loadint16 (pbyte *p) ;
extern longword loadlongword (pbyte *p) ;
extern longint loadlongint (pbyte *p) ;
extern single loadsingle (pbyte *p) ;
extern void loadstring (pbyte *p, integer fieldwidth, string *) ;
extern void loadmac (pbyte *p, tsix *mac) ;
extern void loadblock (pbyte *p, integer size, pointer pdest) ;

#endif
