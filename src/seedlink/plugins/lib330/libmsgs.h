/*   Lib330 Message Definitions
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
    0 2006-09-10 rdr Created
    1 2007-01-18 rdr Add LIBMSG_STATTO.
    2 2007-03-05 rdr Add LIBMSG_CONPURGE.
    3 2008-01-09 rdr Add dump_msgqueue and AUXMSG_RECV.
    4 2008-08-19 rdr Add TCP support.
*/
#ifndef libmsgs_h
/* Flag this file as included */
#define libmsgs_h
#define VER_LIBMSGS 12

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
/* Make sure q330types.h is included */
#ifndef q330types_h
#include "q330types.h"
#endif
/* Make sure libstrucs.h is included */
#ifndef libstrucs_h
#include "libstrucs.h"
#endif

/* verbosity bitmap flags */
#define VERB_SDUMP 1 /* set in cur_verbosity to enable status dump messages */
#define VERB_RETRY 2 /* set to enable command retry messages */
#define VERB_REGMSG 4 /* Registration messages */
#define VERB_LOGEXTRA 8 /* stuff like filter delays */
#define VERB_AUXMSG 16 /* webserver & netserver messages to seed message log */
#define VERB_PACKET 32 /* packet level debug */
/* message codes */
#define LIBMSG_GENDBG 0
#define LIBMSG_PKTIN 1
#define LIBMSG_PKTOUT 2

#define LIBMSG_WINDOW 100
#define LIBMSG_USER 101
#define LIBMSG_LOGCHG 102
#define LIBMSG_TOKCHG 103
#define LIBMSG_MEMOP 104
#define LIBMSG_DECNOTFOUND 105
#define LIBMSG_FILTDLY 106
#define LIBMSG_DTOPEN 107
#define LIBMSG_LINKRST 108
#define LIBMSG_FILLJMP 109
#define LIBMSG_SEQBEG 110
#define LIBMSG_SEQOVER 111
#define LIBMSG_SEQRESUME 112
#define LIBMSG_CONTBOOT 113
#define LIBMSG_CONTFND 114
#define LIBMSG_BUFSHUT 115
#define LIBMSG_CONNSHUT 116
#define LIBMSG_NOIP 117
#define LIBMSG_CTRLDET 118
#define LIBMSG_RESTCONT 119
#define LIBMSG_DISCARD 120
#define LIBMSG_CSAVE 121
#define LIBMSG_DETECT 122
#define LIBMSG_NETSTN 123
#define LIBMSG_ZONE 124
#define LIBMSG_AVG 125
#define LIBMSG_TOTAL 126
#define LIBMSG_EPDLYCHG 127

#define LIBMSG_CREATED 200
#define LIBMSG_REGISTERED 201
#define LIBMSG_DEREGWAIT 202
#define LIBMSG_COMBO 203
#define LIBMSG_DEALLOC 204
#define LIBMSG_GPSID 205
#define LIBMSG_DEREG 206
#define LIBMSG_READTOK 207
#define LIBMSG_TOKREAD 208
#define LIBMSG_AQREM 209
#define LIBMSG_POCRECV 210
#define LIBMSG_WRCONT 211
#define LIBMSG_SOCKETOPEN 212
#define LIBMSG_DEREGTO 213
#define LIBMSG_BACK 214
#define LIBMSG_CONN 215
#define LIBMSG_Q335 216

#define LIBMSG_GPSSTATUS 300
#define LIBMSG_DIGPHASE 301
#define LIBMSG_SAVEBACKUP 302
#define LIBMSG_SCHEDSTART 303
#define LIBMSG_SCHEDEND 304
#define LIBMSG_LEAPDET 305
#define LIBMSG_SMUPHASE 306
#define LIBMSG_APWRON 307
#define LIBMSG_APWROFF 308
#define LIBMSG_PHASERANGE 309
#define LIBMSG_TIMEJMP 310
#define LIBMSG_INVBLKLTH 311
#define LIBMSG_INVSPEC 312
#define LIBMSG_INVBLK 313

#define LIBMSG_BADIPADDR 400
#define LIBMSG_DATADIS 401
#define LIBMSG_PERM 402
#define LIBMSG_PIU 403
#define LIBMSG_SNR 404
#define LIBMSG_INVREG 405
#define LIBMSG_CALPROG 406
#define LIBMSG_CMDABT 407
#define LIBMSG_CONTIN 408
#define LIBMSG_DATATO 409
#define LIBMSG_CONCRC 410
#define LIBMSG_CONTNR 411
#define LIBMSG_STATTO 412
#define LIBMSG_CONPURGE 413
#define LIBMSG_WRONGPORT 414

#define LIBMSG_ROUTEFAULT 500
#define LIBMSG_CANTSEND 501
#define LIBMSG_SOCKETERR 502
#define LIBMSG_BINDERR 503
#define LIBMSG_RECVERR 504
#define LIBMSG_PARERR 505
#define LIBMSG_SNV 506
#define LIBMSG_INVTOK 507
#define LIBMSG_CMDCTRL 508
#define LIBMSG_CMDSPEC 509
#define LIBMSG_CON 510
#define LIBMSG_RETRY 511
#define LIBMSG_INVTVER 512
#define LIBMSG_SEROPEN 513
#define LIBMSG_INVLTH 514
#define LIBMSG_SEQGAP 515
#define LIBMSG_LBFAIL 516
#define LIBMSG_NILRING 517
#define LIBMSG_UNCOMP 518
#define LIBMSG_CONTERR 519
#define LIBMSG_TIMEDISC 520
#define LIBMSG_RECOMP 521
#define LIBMSG_SEGOVER 522
#define LIBMSG_TCPTUN 523
#define LIBMSG_HFRATE 524

#define LIBMSG_FIXED 600
#define LIBMSG_GPSIDS 601
#define LIBMSG_GLSTAT 602
#define LIBMSG_CLOCK 603
#define LIBMSG_BOOM 604
#define LIBMSG_GPS 605
#define LIBMSG_PLL 606
#define LIBMSG_LOG 607
#define LIBMSG_GPSCFG 608
#define LIBMSG_CHANINFO 609
#define LIBMSG_CAL 610

#define AUXMSG_SOCKETOPEN 700
#define AUXMSG_SOCKETERR 701
#define AUXMSG_BINDERR 702
#define AUXMSG_LISTENERR 703
#define AUXMSG_DISCON 704
#define AUXMSG_ACCERR 705
#define AUXMSG_CONN 706
#define AUXMSG_NOBLOCKS 707
#define AUXMSG_SENT 708
#define AUXMSG_INVADDR 709
#define AUXMSG_WEBADV 710
#define AUXMSG_RECVTO 711
#define AUXMSG_WEBLINK 712
#define AUXMSG_RECV 713
#define AUXMSG_DSS 714

#define HOSTMSG_ALL 800

#ifdef CONSTMSG
extern void libmsgadd (pq330 q330, word msgcode, const string95 *msgsuf) ;
extern void libdatamsg (pq330 q330, word msgcode, const string95 *msgsuf) ;
extern void msgadd (pq330 q330, word msgcode, longword dt, const string95 *msgsuf, boolean client) ;
#else
extern void libmsgadd (pq330 q330, word msgcode, string95 *msgsuf) ;
extern void libdatamsg (pq330 q330, word msgcode, string95 *msgsuf) ;
extern void msgadd (pq330 q330, word msgcode, longword dt, string95 *msgsuf, boolean client) ;
#endif

#ifndef OMIT_SEED
extern void dump_msgqueue (pq330 q330) ;
#endif

extern char *lib_get_msg (word code, string95 *result) ;
extern char *lib_get_errstr (enum tliberr err, string63 *result) ;
extern char *lib_get_statestr (enum tlibstate state, string63 *result) ;
extern char *showdot (longword num, string15 *result) ;
extern char *command_name (byte cmd, string95 *result) ;
extern char *lib_gps_state (enum tgps_stat gs, string63 *result) ;
extern char *lib_gps_fix (enum tgps_fix gf, string63 *result) ;
extern char *lib_pll_state (enum tpll_stat ps, string31 *result) ;
extern char *lib_acc_types (enum tacctype acctype, string31 *result) ;

#endif
