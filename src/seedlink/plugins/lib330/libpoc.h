/*   Lib330 POC Receiver definitions
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
    0 2006-09-10 rdr Created
*/
#ifndef libpoc_h
/* Flag this file as included */
#define libpoc_h
#define VER_LIBPOC 0

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#include <errno.h>

typedef struct { /* Format of callback parameter */
  t64 serial_number ; /* Q330's serial port */
  longword ip_address ; /* new dynamic IP address */
  word base_port ; /* port translation may have changed it */
  word data_port ; /* for data port */
} tpoc_recvd ;
enum tpocstate {PS_NEWPOC, PS_CONNRESET} ;
typedef void (*tpocproc)(enum tpocstate pocstate, tpoc_recvd *poc_recv) ;
typedef struct { /* parameters for POC receiver */
  word poc_port ; /* UDP port to listen on */
  tpocproc poc_callback ; /* procedure to call when poc received */
} tpoc_par ;

extern pointer lib_poc_start (tpoc_par *pp) ;
extern void lib_poc_stop (pointer ctr) ;

#endif
