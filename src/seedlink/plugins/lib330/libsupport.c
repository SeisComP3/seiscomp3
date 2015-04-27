/*   Lib330 Support routines
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
    0 2006-09-12 rdr Created
    1 2006-10-29 rdr Fix posix lib_file_open to make sure file handle is returned.
    2 2006-11-28 rdr For Apple version lseek only seems to return 0 for success rather
                     than the new file position. For all versions if creating a flle,
                     truncate length to zero.
    3 2007-07-05 rdr Add some bulletproffing to zpad and jul_string.
    4 2007-08-04 rdr Add CMEX32 support.
    5 2008-01-10 rdr If file owner is specified then use baler callback to translate file
                     names and handle media access for file open, close, and delete.
    6 2009-07-30 rdr uppercase routine moved here from libtokens, renamed to lib330_upper
    7 2013-08-18 rdr Add some includes.
*/
#ifndef libsupport_h
#include "libsupport.h"
#endif
#include <ctype.h>
#include <stdio.h>
#ifndef X86_WIN32
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

const dms_type days_mth = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} ;

typedef pointer *pptr ;

pointer extend_link (pointer base, pointer add)
begin
  pptr p ;

  if (base == NIL)
    then
      base = add ; /* first in list */
    else
      begin
        p = (pptr) base ;
        while (*p != NIL)
          p = (pptr) *p ; /* find end of list */
        *p = add ; /* add to end */
      end
  return base ;
end

/* Convert pascal string to C string, modified to allow in-place operation */
void lib330_strpcopy (pchar outstring, pchar instring)
  begin
    integer i, lth ;

    lth = *instring++ ; /* pascal dynamic length byte */
    for (i = 0 ; i < lth ; i++)
      *outstring++ = *instring++ ;
    *outstring = '\0' ; /* null terminated */
  end

/* Convert C string to Pascal string */
void lib330_strpas (pchar outstring, pchar instring)
  begin
    integer lth ;
    pchar psave ;

    lth = 0 ;
    psave = outstring++ ; /* to set length */
    while (*instring)
      begin
        inc(lth) ;
        *outstring++ = *instring++ ;
      end
    *psave = lth ; /* set dynamic length */
  end

char *zpad (pchar s, integer lth)
begin
  integer len, diff ;

  len = strlen(s) ;
  diff = lth - len ;
  if (diff > 0)
    then
      begin
        memmove (addr(s[diff]), addr(s[0]), len + 1) ; /* shift existing string right */
        memset (addr(s[0]), '0', diff) ; /* add ascii zeroes at front */
      end
  return s ;
end

word day_julian (word yr, word wmth, word day)
begin
  word mth, jday ;

  jday = 0 ;
  mth = 1 ;
  while (mth < wmth)
    begin
      if (((yr mod 4) == 0) land (mth == 2))
        then
          jday = jday + 29 ;
        else
          jday = jday + days_mth[mth] ;
      inc(mth) ;
    end
  return jday + day ;
end

longint lib330_julian (tsystemtime *greg)
begin
  word year ;
  longint leap, days ;

  year = greg->wyear - 2000 ;
  leap = (year + 3) shr 2 ; /* leap years so far */
  days = (longint) year * 365 + leap ; /* number of years passed, plus leap days */
  days = days + day_julian (year, greg->wmonth, greg->wday) - 1 ;
  return days * 86400 + (longint) greg->whour * 3600 +
            (longint) greg->wminute * 60 + (longint) greg->wsecond ;
end


#ifdef X86_WIN32
double now (void)
begin
  SYSTEMTIME sgreg ;
  tsystemtime nowgreg ;
  double r ;

  GetSystemTime (addr(sgreg)) ;
  memcpy (addr(nowgreg), addr(sgreg), sizeof(tsystemtime)) ; /* copy to internal version */
  r = lib330_julian (addr(nowgreg)) ; /* get whole seconds since 2000 */
  r = r + nowgreg.wmilliseconds / 1000.0 ;
  return r ;
end

#else

#ifdef CMEX32
double now (void)
begin

  return cmex_now () ;
end

#else

double now (void)
begin
#define DIFF2000_1970 ((23 * 365) + (7 * 366)) /* Difference between 1970 and 2000 references */
  struct timeval tp;
  struct timezone tzp;
  double r ;

  gettimeofday (addr(tp), addr(tzp)) ;
  r = ((double) tp.tv_sec + ((double) tp.tv_usec / 1000000.0)) ;
  return r - DIFF2000_1970 * 86400.0 ;
end
#endif
#endif

longint lib_round (double r)
begin
  longint result ;

  if (r >= 0.0)
    then
      result = r + 0.5 ;
    else
      result = r - 0.5 ;
  return result ;
end

void day_gregorian (word yr, word jday, word *mth, word *day)
begin
  word dim ;

  *mth = 1 ;
  repeat
    dim = days_mth[*mth] ;
    if (((yr mod 4) == 0) land (*mth == 2))
      then
        dim = 29 ;
    if (jday <= dim)
      then
        break ;
    jday = jday - dim ;
    inc(*mth) ;
  until (jday > 400)) ; /* just in case */
  *day = jday ;
end

void lib330_gregorian (longint jul, tsystemtime *greg)
begin
  longint quads, days, subday, yeartemp ;

  days = jul div 86400 ;
  subday = jul - (days * 86400) ;
  greg->whour = subday div 3600 ;
  subday = subday - ((longint) greg->whour * 3600) ;
  greg->wminute = subday div 60 ;
  greg->wsecond = subday - (greg->wminute * 60) ;
  yeartemp = 2000 ;
  quads = days div 1461 ; /* 0-3 groups */
  incn(yeartemp, quads shl 2) ;
  decn(days, quads * 1461) ;
  if (days >= 366)
    then
      begin
        inc(yeartemp) ;
        days = days - 366 ; /* remove leap year part of group */
        while (days >= 365)
          begin
            inc(yeartemp) ;
            days = days - 365 ;
          end
      end
  greg->wyear = yeartemp ;
  day_gregorian (greg->wyear, days + 1, addr(greg->wmonth), addr(greg->wday)) ;
end

char *jul_string (longint jul, pchar result)
begin
  tsystemtime g ;
  string7 s1 ;

  if (jul < 0)
    then
      strcpy(result, "Invalid Time       ") ;
    else
      begin
        lib330_gregorian (jul, addr(g)) ;
        result[0] = 0 ;
        sprintf(result, "%d", g.wyear) ;
        strcat(result, "-") ;
        sprintf(s1, "%d", g.wmonth) ;
        zpad(s1, 2) ;
        strcat(result, s1) ;
        strcat(result, "-") ;
        sprintf(s1, "%d", g.wday) ;
        zpad(s1, 2) ;
        strcat(result, s1) ;
        strcat(result, " ") ;
        sprintf(s1, "%d", g.whour) ;
        zpad(s1, 2) ;
        strcat(result, s1) ;
        strcat(result, ":") ;
        sprintf(s1, "%d", g.wminute) ;
        zpad(s1, 2) ;
        strcat(result, s1) ;
        strcat(result, ":") ;
        sprintf(s1, "%d", g.wsecond) ;
        zpad(s1, 2) ;
        strcat(result, s1) ;
      end
  return result ;
end

char *packet_time (longint jul, pchar result)
begin
  tsystemtime g ;
  string7 s1 ;

  lib330_gregorian (jul, addr(g)) ;
  strcpy(result, "[") ;
  sprintf(s1, "%d", g.wminute) ;
  zpad(s1, 2) ;
  strcat(result, s1) ;
  strcat(result, ":") ;
  sprintf(s1, "%d", g.wsecond) ;
  zpad(s1, 2) ;
  strcat(result, s1) ;
  strcat(result, "] ") ;
  return result ;
end ;

char *showsn (t64 *val, string31 *result)
begin
  string15 s1, s2 ;

  sprintf(s1, "%X", (*val)[0]) ;
  zpad(s1, 8) ;
  sprintf(s2, "%X", (*val)[1]) ;
  zpad(s2, 8) ;
#ifdef ENDIAN_LITTLE
  strcpy(result, s2) ;
  strcat(result, s1) ;
#else
  strcpy(result, s1) ;
  strcat(result, s2) ;
#endif
  return result ;
end

longword getip (pchar s, boolean *domain)
begin
typedef longword *plword ;
  longword ip ;
#ifdef X86_WIN32
  struct hostent *phost ;
  char **listptr ;
  struct in_addr *ptr ;
#else
  struct hostent *phost ;
  char **listptr ;
  struct in_addr *ptr ;
#endif

  if (strcmp(s, "255.255.255.255") == 0)
    then
      begin /* this is the same as INADDR_NONE */
        domain = FALSE ;
        return 0xFFFFFFFF ;
      end
#ifdef X86_WIN32
  ip = inet_addr(s) ;
  if (ip == INADDR_NONE)
#else
  ip = inet_addr(s) ;
  if (ip == INADDR_NONE)
#endif
    then
      begin /* try to look up */
        *domain = TRUE ;
#ifdef X86_WIN32
        phost = gethostbyname(s) ;
        if (phost)
          then
            begin
              listptr = phost->h_addr_list ;
              ptr = (struct in_addr *) *listptr ;
              ip = ptr->s_addr ;
            end
#else
#ifdef CMEX32
        phost = NIL ;
#else
        phost = gethostbyname(s) ;
        if (phost)
          then
            begin
              listptr = phost->h_addr_list ;
              ptr = (struct in_addr *) *listptr ;
              ip = ptr->s_addr ;
            end
#endif
#endif
      end
    else
      *domain = FALSE ;
#ifdef ENDIAN_LITTLE
  ip = ntohl (ip) ;
#endif
  return ip ;
end

word newrand (word *sum)
begin

  *sum = (*sum * 765) + 13849 ;
  return *sum ;
end

/* upshift a C string */
char *lib330_upper (pchar s)
begin
  size_t i ;

  for (i = 0 ; i < strlen(s) ; i++)
    s[i] = toupper (s[i]) ;
  return s ;
end

longint baler_file (pfile_owner powner, enum tfileacc_type fatype, pstring fname,
                    integer opt1, integer opt2)
begin
  tfileacc_call fc ;

  fc.owner = powner ;
  fc.fileacc_type = fatype ;
  fc.fname = fname ;
  fc.opt1 = opt1 ;
  fc.opt2 = opt2 ;
  fc.response = 0 ;
  powner->call_fileacc (addr(fc)) ;
  return fc.response ;
end

#ifdef X86_WIN32

tfile_handle lib_file_open (pfile_owner powner, pchar path, integer mode)
begin
  longword rwmode, openmode ;
  HANDLE cf ;

  if (mode and LFO_CREATE)
    then
      openmode = CREATE_ALWAYS ;
    else
      openmode = OPEN_EXISTING ;
  if (mode and LFO_WRITE)
    then
      rwmode = GENERIC_WRITE ;
    else
      rwmode = 0 ;
  if (mode and LFO_READ)
    then
      rwmode = rwmode or GENERIC_READ ;
  if (powner)
    then
      cf = (HANDLE) baler_file (powner, FAT_OPEN, path, rwmode, openmode) ;
    else
      cf = CreateFile (path, rwmode, 0, NIL, openmode, FILE_ATTRIBUTE_NORMAL, 0) ;
  if (cf == INVALID_HANDLE_VALUE)
    then
      return INVALID_FILE_HANDLE ;
    else
      return cf ;
end

void lib_file_close (pfile_owner powner, tfile_handle desc)
begin

  if (powner)
    then
      baler_file (powner, FAT_CLOSE, NIL, (integer) desc, 0) ;
    else
      CloseHandle (desc) ;
end

boolean lib_file_seek (pfile_owner powner, tfile_handle desc, integer offset)
begin

  if (powner)
    then
      return (baler_file (powner, FAT_SEEK, NIL, (integer) desc, offset)) ;
    else
      return (SetFilePointer (desc, offset, NIL, FILE_BEGIN) == 0xFFFFFFFF) ;
end

boolean lib_file_read (pfile_owner powner, tfile_handle desc, pointer buf, integer size)
begin
  unsigned long numread ;

  if (powner)
    then
      numread = baler_file (powner, FAT_READ, buf, (integer) desc, size) ;
    else
      ReadFile (desc, buf, size, addr(numread), NIL) ;
  return ((integer)numread != size) ;
end

boolean lib_file_write (pfile_owner powner, tfile_handle desc, pointer buf, integer size)
begin
  unsigned long numwrite ;

  if (powner)
    then
      numwrite = baler_file (powner, FAT_WRITE, buf, (integer) desc, size) ;
    else
      WriteFile (desc, buf, size, addr(numwrite), NIL) ;
  return ((integer)numwrite != size) ;
end

void lib_file_delete (pfile_owner powner, pchar path)
begin

  if (powner)
    then
      baler_file (powner, FAT_DEL, path, 0, 0) ;
    else
      DeleteFile (path) ;
end

integer lib_file_size (pfile_owner powner, tfile_handle desc)
begin

  if (powner)
    then
      return baler_file (powner, FAT_SIZE, NIL, (integer) desc, 0) ;
    else
      return GetFileSize (desc, NIL) ;
end

#else

#ifdef CMEX32
/* returns negative number for error */
tfile_handle lib_file_open (pfile_owner powner, pchar path, integer mode)
begin
  tfile_handle cf ;

  return (cmexopen (path, mode)) ;
end

#else
/* returns negative number for error */
tfile_handle lib_file_open (pfile_owner powner, pchar path, integer mode)
begin
  tfile_handle cf ;
  integer flags ;
  mode_t rwmode ;

  if (mode and LFO_CREATE)
    then
      flags = O_CREAT or O_TRUNC ;
    else
      flags = 0 ;
  if (mode and LFO_WRITE)
    then
      if (mode and LFO_READ)
        then
          flags = flags or O_RDWR ;
        else
          flags = flags or O_WRONLY ;
  else if (mode and LFO_READ)
    then
      flags or O_RDONLY ;
  rwmode = S_IRUSR or S_IWUSR or S_IRGRP or S_IROTH ;
  if (powner)
    then
      cf = baler_file (powner, FAT_OPEN, path, rwmode, flags) ;
    else
      cf = open (path, flags, rwmode) ;
  return cf ;
end
#endif

void lib_file_close (pfile_owner powner, tfile_handle desc)
begin

  if (powner)
    then
      baler_file (powner, FAT_CLOSE, NIL, desc, 0) ;
    else
      close (desc) ;
end

boolean lib_file_seek (pfile_owner powner, tfile_handle desc, integer offset)
begin
  off_t result, long_offset ;

  long_offset = offset ;
  if (powner)
    then
      result = baler_file (powner, FAT_SEEK, NIL, (integer) desc, offset) ;
    else
      result = lseek(desc, long_offset, SEEK_SET) ;
#ifdef __APPLE__
  return (result < 0) ;
#else
  return (result != long_offset) ;
#endif
end

boolean lib_file_read (pfile_owner powner, tfile_handle desc, pointer buf, integer size)
begin
  integer numread ;

  if (powner)
    then
      numread = baler_file (powner, FAT_READ, buf, (integer) desc, size) ;
    else
      numread = read (desc, buf, size) ;
  return (numread != size) ;
end

boolean lib_file_write (pfile_owner powner, tfile_handle desc, pointer buf, integer size)
begin
  integer numwrite ;

  if (powner)
    then
      numwrite = baler_file (powner, FAT_WRITE, buf, (integer) desc, size) ;
    else
      numwrite = write(desc, buf, size) ;
  return (numwrite != size) ;
end

void lib_file_delete (pfile_owner powner, pchar path)
begin

  if (powner)
    then
      baler_file (powner, FAT_DEL, path, 0, 0) ;
    else
      remove (path) ;
end

#ifdef CMEX32
integer lib_file_size (pfile_owner powner, tfile_handle desc)
begin

  return cmexsize (desc) ;
end

#else

integer lib_file_size (pfile_owner powner, tfile_handle desc)
begin
  struct stat sb ;

  if (powner)
    then
      return baler_file (powner, FAT_SIZE, NIL, (integer) desc, 0) ;
    else
      begin
        fstat(desc, addr(sb)) ;
        return sb.st_size ;
      end
end
#endif

#endif
