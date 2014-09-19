/*   Lib330 host <-> native conversion routines
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
    1 2006-10-29 rdr Add including my header file.
*/
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

void storebyte (pbyte *p, byte b)
begin

  **p = b ;
  inc(*p) ;
end

void storeword (pbyte *p, word w)
begin

#ifdef ENDIAN_LITTLE
  w = htons(w) ;
#endif
  memcpy(*p, addr(w), 2) ;
  incn(*p, 2) ;
end

void storeint16 (pbyte *p, int16 i)
begin

#ifdef ENDIAN_LITTLE
  i = htons(i) ;
#endif
  memcpy(*p, addr(i), 2) ;
  incn(*p, 2) ;
end

void storelongword (pbyte *p, longword lw)
begin

#ifdef ENDIAN_LITTLE
  lw = htonl(lw) ;
#endif
  memcpy(*p, addr(lw), 4) ;
  incn(*p, 4) ;
end

void storelongint (pbyte *p, longint li)
begin

#ifdef ENDIAN_LITTLE
  li = htonl(li) ;
#endif
  memcpy(*p, addr(li), 4) ;
  incn(*p, 4) ;
end

void storesingle (pbyte *p, single s)
begin
  longint li ;

  memcpy(addr(li), addr(s), 4) ;
#ifdef ENDIAN_LITTLE
  li = htonl(li) ;
#endif
  memcpy(*p, addr(li), 4) ;
  incn(*p, 4) ;
end

void storestring (pbyte *p, integer fieldwidth, string *s)
begin
  integer lth ;

  lth = strlen(s) + 1 ;
  if (lth > fieldwidth)
    then
      begin
        lth = fieldwidth ;
        (*s)[lth - 1] = 0 ; /* shorten too long string */
      end
  lib330_strpas ((pchar) *p, s) ;
  incn(*p, lth) ;
  if (fieldwidth > lth)
    then
      begin
        memset(*p, 0, fieldwidth - lth) ;
        incn(*p, fieldwidth - lth) ;
      end
end

void storemac (pbyte *p, tsix *mac)
begin

#ifdef ENDIAN_LITTLE
  storeword (*p, (*mac)[2]) ;
  storeword (*p, (*mac)[1]) ;
  storeword (*p, (*mac)[0]) ;
#else
  storeword (*p, (*mac)[0]) ;
  storeword (*p, (*mac)[1]) ;
  storeword (*p, (*mac)[2]) ;
#endif
end

void storeblock (pbyte *p, integer size, pointer psrc)
begin

  memcpy(*p, psrc, size) ;
  incn(*p, size) ;
end

byte loadbyte (pbyte *p)
begin
  byte temp ;

  temp = **p ;
  incn(*p, 1) ;
  return temp ;
end

word loadword (pbyte *p)
begin
  word w ;

  memcpy(addr(w), *p, 2) ;
#ifdef ENDIAN_LITTLE
  w = ntohs(w) ;
#endif
  incn(*p, 2) ;
  return w ;
end

int16 loadint16 (pbyte *p)
begin
  int16 i ;

  memcpy(addr(i), *p, 2) ;
#ifdef ENDIAN_LITTLE
  i = ntohs(i) ;
#endif
  incn(*p, 2) ;
  return i ;
end

longword loadlongword (pbyte *p)
begin
  longword lw ;

  memcpy(addr(lw), *p, 4) ;
#ifdef ENDIAN_LITTLE
  lw = ntohl(lw) ;
#endif
  incn(*p, 4) ;
  return lw ;
end

longint loadlongint (pbyte *p)
begin
  longint li ;

  memcpy(addr(li), *p, 4) ;
#ifdef ENDIAN_LITTLE
  li = ntohl(li) ;
#endif
  incn(*p, 4) ;
  return li ;
end

single loadsingle (pbyte *p)
begin
  longint li ;
  single s ;

  memcpy(addr(li), *p, 4) ;
#ifdef ENDIAN_LITTLE
  li = ntohl(li) ;
#endif
  memcpy(addr(s), addr(li), 4) ;
  incn(*p, 4) ;
  return s ;
end

void loadstring (pbyte *p, integer fieldwidth, string *s)
begin

  memcpy(s, *p, fieldwidth) ;
  lib330_strpcopy(s, s) ; /* this routine allows in-place operation */
  incn(*p, fieldwidth) ;
end

void loadmac (pbyte *p, tsix *mac)
begin

#ifdef ENDIAN_LITTLE
  (*mac)[2] = loadword (p) ;
  (*mac)[1] = loadword (p) ;
  (*mac)[0] = loadword (p) ;
#else
  (*mac)[0] = loadword (p) ;
  (*mac)[1] = loadword (p) ;
  (*mac)[2] = loadword (p) ;
#endif
end

void loadblock (pbyte *p, integer size, pointer pdest)
begin

  memcpy(pdest, *p, size) ;
  incn(*p, size) ;
end

