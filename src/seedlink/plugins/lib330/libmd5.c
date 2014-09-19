/*   Lib330 MD5 Routines
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
    1 2007-02-19 rdr In transform copy source array into array of longinteger
                     to handle CPU's that can't do non-aligned access (per DN).
*/
#ifndef libmd5_h
#include "libmd5.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

typedef struct {
  tdigest fdigest ;         /*the digest to be returned*/
  tdigestlongint cdigest ;  /*digest accumulator*/
  longint bitlo, bithi ;    /* number of _bits_ handled mod 2^64 */
  tbytebuf bbuf ;
  longint blen ; /*bytes in bbuf*/
} tworkingvar ;
typedef tworkingvar *pworkingvar ;

char *dig2str (t64 *d, string63 *result)
begin
  string63 s ;
  string3 s1 ;
  integer i ;
  byte *p ;
  t64 cond ;
#ifndef ENDIAN_LITTLE
  longword lw ;
  longword *plw ;
#endif

#ifdef ENDIAN_LITTLE
  cond[0] = htonl((*d)[1]) ;
  cond[1] = htonl((*d)[0]) ;
#else
  cond[0] = (*d)[0] ;
  cond[1] = (*d)[1] ;
#endif
  s[0] = 0 ;
  p = addr(cond) ;
  for (i = 0 ; i <= 7 ; i++)
    begin
      sprintf(s1, "%x", *p++) ;
      strcat(s, zpad(s1, 2)) ;
    end
#ifndef ENDIAN_LITTLE
/* MD5 algorithm expects little endian input, reverse longwords for big endian */
  for (i = 0 ; i <= 3 ; i++)
    begin
      plw = (pointer)(addr(s[i * 4])) ;
      lw = *plw ;
      lw = (lw shr 24) or ((lw and 0xFF0000) shr 8) or ((lw and 0xFF00) shl 8) or (lw shl 24) ;
      *plw = lw ;
    end
#endif
  strcpy(result, s) ;
  return result ;
end

static longint rol (longint x, longint n)
begin

  return (x shl n) or ((longword)x shr (longword)(32 - n)) ;
end

static longint ff (longint a, longint b, longint c, longint d, longint x, longint s, longint ac)
begin

  return rol (a + x + ac + ((b and c) or (not b and d)), s) + b ;
end

static longint gg (longint a, longint b, longint c, longint d, longint x, longint s, longint ac)
begin

  return rol (a + x + ac + ((b and d) or (c and not d)), s) + b ;
end

static longint hh (longint a, longint b, longint c, longint d, longint x, longint s, longint ac)
begin

  return rol (a + x + ac + (b xor c xor d), s) + b ;
end

static longint ii (longint a, longint b, longint c, longint d, longint x, longint s, longint ac)
begin

  return rol (a + x + ac + (c xor (b or not d)), s) + b ;
end

static void transform (pointer paccum, pointer pbuffer)
begin
  longint a, b, c, d ;
  tlongintbuf *pbuf ;
  tdigestlongint *pacc ;
  longint ibuf[16] ;

  /* copy contents of pbuffer to ibuf for alignment purposes. */
  memcpy (addr(ibuf), (pointer) pbuffer, 16 * sizeof(longint));
  pacc = paccum ;
  pbuf = addr(ibuf) ;
  a = (*pacc)[0] ;
  b = (*pacc)[1] ;
  c = (*pacc)[2] ;
  d = (*pacc)[3] ;
  a = ff(a, b, c, d, (*pbuf)[ 0], 7, 0xd76aa478) ; /* 1 */
  d = ff(d, a, b, c, (*pbuf)[ 1], 12, 0xe8c7b756) ; /* 2 */
  c = ff(c, d, a, b, (*pbuf)[ 2], 17, 0x242070db) ; /* 3 */
  b = ff(b, c, d, a, (*pbuf)[ 3], 22, 0xc1bdceee) ; /* 4 */
  a = ff(a, b, c, d, (*pbuf)[ 4], 7, 0xf57c0faf) ; /* 5 */
  d = ff(d, a, b, c, (*pbuf)[ 5], 12, 0x4787c62a) ; /* 6 */
  c = ff(c, d, a, b, (*pbuf)[ 6], 17, 0xa8304613) ; /* 7 */
  b = ff(b, c, d, a, (*pbuf)[ 7], 22, 0xfd469501) ; /* 8 */
  a = ff(a, b, c, d, (*pbuf)[ 8], 7, 0x698098d8) ; /* 9 */
  d = ff(d, a, b, c, (*pbuf)[ 9], 12, 0x8b44f7af) ; /* 10 */
  c = ff(c, d, a, b, (*pbuf)[10], 17, 0xffff5bb1) ; /* 11 */
  b = ff(b, c, d, a, (*pbuf)[11], 22, 0x895cd7be) ; /* 12 */
  a = ff(a, b, c, d, (*pbuf)[12], 7, 0x6b901122) ; /* 13 */
  d = ff(d, a, b, c, (*pbuf)[13], 12, 0xfd987193) ; /* 14 */
  c = ff(c, d, a, b, (*pbuf)[14], 17, 0xa679438e) ; /* 15 */
  b = ff(b, c, d, a, (*pbuf)[15], 22, 0x49b40821) ; /* 16 */

  a = gg(a, b, c, d, (*pbuf)[ 1], 5, 0xf61e2562) ; /* 17 */
  d = gg(d, a, b, c, (*pbuf)[ 6], 9, 0xc040b340) ; /* 18 */
  c = gg(c, d, a, b, (*pbuf)[11], 14, 0x265e5a51) ; /* 19 */
  b = gg(b, c, d, a, (*pbuf)[ 0], 20, 0xe9b6c7aa) ; /* 20 */
  a = gg(a, b, c, d, (*pbuf)[ 5], 5, 0xd62f105d) ; /* 21 */
  d = gg(d, a, b, c, (*pbuf)[10], 9, 0x02441453) ; /* 22 */
  c = gg(c, d, a, b, (*pbuf)[15], 14, 0xd8a1e681) ; /* 23 */
  b = gg(b, c, d, a, (*pbuf)[ 4], 20, 0xe7d3fbc8) ; /* 24 */
  a = gg(a, b, c, d, (*pbuf)[ 9], 5, 0x21e1cde6) ; /* 25 */
  d = gg(d, a, b, c, (*pbuf)[14], 9, 0xc33707d6) ; /* 26 */
  c = gg(c, d, a, b, (*pbuf)[ 3], 14, 0xf4d50d87) ; /* 27 */
  b = gg(b, c, d, a, (*pbuf)[ 8], 20, 0x455a14ed) ; /* 28 */
  a = gg(a, b, c, d, (*pbuf)[13], 5, 0xa9e3e905) ; /* 29 */
  d = gg(d, a, b, c, (*pbuf)[ 2], 9, 0xfcefa3f8) ; /* 30 */
  c = gg(c, d, a, b, (*pbuf)[ 7], 14, 0x676f02d9) ; /* 31 */
  b = gg(b, c, d, a, (*pbuf)[12], 20, 0x8d2a4c8a) ; /* 32 */

  a = hh(a, b, c, d, (*pbuf)[ 5], 4, 0xfffa3942) ; /* 33 */
  d = hh(d, a, b, c, (*pbuf)[ 8], 11, 0x8771f681) ; /* 34 */
  c = hh(c, d, a, b, (*pbuf)[11], 16, 0x6d9d6122) ; /* 35 */
  b = hh(b, c, d, a, (*pbuf)[14], 23, 0xfde5380c) ; /* 36 */
  a = hh(a, b, c, d, (*pbuf)[ 1], 4, 0xa4beea44) ; /* 37 */
  d = hh(d, a, b, c, (*pbuf)[ 4], 11, 0x4bdecfa9) ; /* 38 */
  c = hh(c, d, a, b, (*pbuf)[ 7], 16, 0xf6bb4b60) ; /* 39 */
  b = hh(b, c, d, a, (*pbuf)[10], 23, 0xbebfbc70) ; /* 40 */
  a = hh(a, b, c, d, (*pbuf)[13], 4, 0x289b7ec6) ; /* 41 */
  d = hh(d, a, b, c, (*pbuf)[ 0], 11, 0xeaa127fa) ; /* 42 */
  c = hh(c, d, a, b, (*pbuf)[ 3], 16, 0xd4ef3085) ; /* 43 */
  b = hh(b, c, d, a, (*pbuf)[ 6], 23, 0x04881d05) ; /* 44 */
  a = hh(a, b, c, d, (*pbuf)[ 9], 4, 0xd9d4d039) ; /* 45 */
  d = hh(d, a, b, c, (*pbuf)[12], 11, 0xe6db99e5) ; /* 46 */
  c = hh(c, d, a, b, (*pbuf)[15], 16, 0x1fa27cf8) ; /* 47 */
  b = hh(b, c, d, a, (*pbuf)[ 2], 23, 0xc4ac5665) ; /* 48 */

  a = ii(a, b, c, d, (*pbuf)[ 0], 6, 0xf4292244) ; /* 49 */
  d = ii(d, a, b, c, (*pbuf)[ 7], 10, 0x432aff97) ; /* 50 */
  c = ii(c, d, a, b, (*pbuf)[14], 15, 0xab9423a7) ; /* 51 */
  b = ii(b, c, d, a, (*pbuf)[ 5], 21, 0xfc93a039) ; /* 52 */
  a = ii(a, b, c, d, (*pbuf)[12], 6, 0x655b59c3) ; /* 53 */
  d = ii(d, a, b, c, (*pbuf)[ 3], 10, 0x8f0ccc92) ; /* 54 */
  c = ii(c, d, a, b, (*pbuf)[10], 15, 0xffeff47d) ; /* 55 */
  b = ii(b, c, d, a, (*pbuf)[ 1], 21, 0x85845dd1) ; /* 56 */
  a = ii(a, b, c, d, (*pbuf)[ 8], 6, 0x6fa87e4f) ; /* 57 */
  d = ii(d, a, b, c, (*pbuf)[15], 10, 0xfe2ce6e0) ; /* 58 */
  c = ii(c, d, a, b, (*pbuf)[ 6], 15, 0xa3014314) ; /* 59 */
  b = ii(b, c, d, a, (*pbuf)[13], 21, 0x4e0811a1) ; /* 60 */
  a = ii(a, b, c, d, (*pbuf)[ 4], 6, 0xf7537e82) ; /* 61 */
  d = ii(d, a, b, c, (*pbuf)[11], 10, 0xbd3af235) ; /* 62 */
  c = ii(c, d, a, b, (*pbuf)[ 2], 15, 0x2ad7d2bb) ; /* 63 */
  b = ii(b, c, d, a, (*pbuf)[ 9], 21, 0xeb86d391) ; /* 64 */
  incn((*pacc)[0], a) ;
  incn((*pacc)[1], b) ;
  incn((*pacc)[2], c) ;
  incn((*pacc)[3], d) ;
end

void resetbuffer (pworkingvar workingvar)
begin

  workingvar->bitlo = 0 ;
  workingvar->bithi = 0 ;
  workingvar->blen = 0 ;
  workingvar->cdigest[0] = 0x67452301 ;
  workingvar->cdigest[1] = 0xefcdab89 ;
  workingvar->cdigest[2] = 0x98badcfe ;
  workingvar->cdigest[3] = 0x10325476 ;
end

static void update (pworkingvar workingvar, pointer chkbuf, longint len)
begin
  pbyte bufptr ;
  longint left ;

  if ((workingvar->bitlo + ((longint)len shl 3)) < workingvar->bitlo)
    then
      inc(workingvar->bithi) ;
  incn(workingvar->bitlo, (longint)len shl 3) ;
  incn(workingvar->bithi, (longint)len shr 29) ;

  bufptr = chkbuf ;
  if (workingvar->blen > 0)
    then
      begin
        left = 64 - workingvar->blen ;
        if (left > len)
          then
            left = len ;
        memcpy (addr(workingvar->bbuf[workingvar->blen]), bufptr , left) ;
        incn(workingvar->blen, left) ;
        incn(bufptr, left) ;
        if (workingvar->blen < 64)
          then
            return ;
        transform(addr(workingvar->cdigest), addr(workingvar->bbuf)) ;
        workingvar->blen = 0 ;
        decn(len, left) ;
      end
  while (len >= 64)
    begin
      transform(addr(workingvar->cdigest), bufptr) ;
      incn(bufptr, 64) ;
      decn(len, 64) ;
    end
  if (len > 0)
    then
      begin
        workingvar->blen = len ;
        memcpy (addr(workingvar->bbuf[0]), bufptr, workingvar->blen) ;
      end
end

void init_md5_buffer (pq330 q330)
begin

  getbuf (q330, addr(q330->md5buf), sizeof(tworkingvar)) ;
end

void calcmd5 (pq330 q330, string250 *chal, t128 *resp)
begin
  tbytebuf workbuf ;
  longint worklen ;
  pworkingvar workingvar ;
  longword lw ;
  integer i ;
  tlongintbuf *plbuf ;

  workingvar = q330->md5buf ;
  resetbuffer (workingvar) ;
  update(workingvar, chal, strlen(chal)) ;
  memcpy(addr(workingvar->fdigest), addr(workingvar->cdigest), sizeof(tdigest)) ;
  memcpy(addr(workbuf), addr(workingvar->bbuf), workingvar->blen) ; /*make copy of buffer*/
  /*pad out to block of form (0..55, bitlo, bithi)*/
#ifdef ENDIAN_LITTLE
  workbuf[workingvar->blen] = 0x80 ;
  worklen = workingvar->blen + 1 ;
#else
  workbuf[workingvar->blen] = 0 ;
  workbuf[workingvar->blen + 1] = 0 ;
  workbuf[workingvar->blen + 2] = 0 ;
  workbuf[workingvar->blen + 3] = 0x80 ;
  worklen = workingvar->blen + 4 ;
#endif
  if (worklen > 56)
    then
      begin
        memset(addr(workbuf[worklen]), 0, 64 - worklen) ;
        transform(addr(workingvar->fdigest), addr(workbuf)) ;
        worklen = 0 ;
      end
  memset(addr(workbuf[worklen]), 0, 56 - worklen) ;
  plbuf = addr(workbuf) ;
  (*plbuf)[14] = workingvar->bitlo ;
  (*plbuf)[15] = workingvar->bithi ;
  transform (addr(workingvar->fdigest), addr(workbuf)) ;
  memcpy(resp, addr(workingvar->fdigest), sizeof(t128)) ;
/* For big endian htonl won't work, for little endian this getline cancelled by storelongword */
  for (i = 0 ; i <= 3 ; i++)
    begin
      lw = (*resp)[i] ;
      lw = (lw shr 24) or ((lw and 0xFF0000) shr 8) or ((lw and 0xFF00) shl 8) or (lw shl 24) ;
      (*resp)[i] = lw ;
    end
end

