/*   Lib330 Support routines definitions
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
    1 2007-01-08 hjs prefaced some functions with lib330 to avoid collisions
*/
#ifndef libsupport_h
/* Flag this file as included */
#define libsupport_h
#define VER_LIBSUPPORT 4

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif

/* lib_file_open modes bit values */
#define LFO_CREATE 1 /* create new file, overwrite any existing file */
#define LFO_OPEN 2 /* open existing file */
#define LFO_READ 4 /* allow reading */
#define LFO_WRITE 8 /* allow writing */

extern const dms_type days_mth ;

extern pointer extend_link (pointer base, pointer add) ;
extern void lib330_strpcopy (pchar outstring, pchar instring) ;
extern void lib330_strpas (pchar outstring, pchar instring) ;
extern char *zpad (pchar s, integer lth) ;
extern double now (void) ;
extern longint lib_round (double r) ;
extern word day_julian (word yr, word wmth, word day) ;
extern longint lib330_julian (tsystemtime *greg) ;
extern void day_gregorian (word yr, word jday, word *mth, word *day) ;
extern char *jul_string (longint jul, pchar result) ;
extern char *packet_time (longint jul, pchar result) ;
extern void lib330_gregorian (longint jul, tsystemtime *greg) ;
extern char *showsn (t64 *val, string31 *result) ;
extern longword getip (pchar s, boolean *domain) ;
extern word newrand (word *sum) ;
extern tfile_handle lib_file_open (pchar path, integer mode) ;
extern void lib_file_close (tfile_handle desc) ;
extern boolean lib_file_seek (tfile_handle desc, integer offset) ;
extern boolean lib_file_read (tfile_handle desc, pointer buf, integer size) ;
extern boolean lib_file_write (tfile_handle desc, pointer buf, integer size) ;
extern void lib_file_delete (pchar path) ;
extern integer lib_file_size (tfile_handle desc) ;
#endif
