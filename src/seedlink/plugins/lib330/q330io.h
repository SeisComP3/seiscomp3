/*   Lib330 Q330 I/O Communications routine headers
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
    0 2006-09-29 rdr Created
*/
#ifndef q330io_h
/* Flag this file as included */
#define q330io_h
#define VER_Q330IO 13

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

extern void close_sockets (pq330 q330) ;
#ifndef OMIT_NETWORK
extern boolean open_sockets (pq330 q330, boolean both, boolean fromback) ;
extern void read_cmd_socket (pq330 q330) ;
extern void read_data_socket (pq330 q330) ;
extern void tcp_error (pq330 q330, string95 *msgsuf) ;
#endif

#ifndef OMIT_SERIAL
extern void send_packet (pq330 q330, integer lth, word toport, word fromport) ;
extern boolean open_serial (pq330 q330) ;
extern void read_from_serial (pq330 q330) ;
extern void inject_packet (pq330 q330, pbyte payload, byte protocol, longword srcaddr,
                           longword destaddr, word srcport, word destport, word datalength,
                           longword seq, longword ack, word window, byte flags) ;
#endif

#endif
