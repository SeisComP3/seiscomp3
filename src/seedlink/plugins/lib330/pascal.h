/*   Pascal compatability includes.
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

dit History:
   Ed Date       By  Changes
   -- ---------- --- ---------------------------------------------------
    0 2006-09-09 rdr Created
*/
#ifndef platform_h
#include "platform.h"
#endif

#ifndef pascal_h
#define pascal_h

#define begin {
#define end }
#define land &&
#define lor ||
#define lnot !
#define div /
#define mod %

#define and &
#define or |
#define xor ^
#define not ~

#define then
#define NIL 0
#define addr &
#define shl <<
#define shr >>
#define repeat do {
#define until } while (lnot
#define inc(val) (val)++
#define dec(val) (val)--
#define incn(val,off) (val = val + (off))
#define decn(val,off) (val = val - (off))

#endif
