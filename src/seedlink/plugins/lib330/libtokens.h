/*   Lib330 DP Token definitions
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
    0 2006-10-01 rdr Created
*/
#ifndef libtokens_h
/* Flag this file as included */
#define libtokens_h
#define VER_LIBTOKENS 3

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif

#define MAXCFG 7884 /* actual number of characters allowed */
#define T_VER 0 /* Token Version */
#define DEFLTH 37 /* Number of valid bytes in deftok */

/* Fixed Length Tokens */
#define TF_NOP 0 /* nothing */
#define TF_VERSION 1 /* Token Version Number */
#define TF_NET_STAT 2 /* Network and Station ID */
#define TF_NETSERV 3 /* Netserver Port */
#define TF_DSS 4 /* DSS Parameters */
#define TF_WEB 5 /* DP Webserver */
#define TF_CLOCK 6 /* Clock Processing */
#define TF_MT 7 /* Log and Timing record ID's */
#define TF_CFG 8 /* How to write configuration information */
#define TF_DATASERV 9 /* Dataserver Port */
/* 8 Bit Length Field */
#define T1_LCQ 128 /* Logical Channel Queue */
#define T1_IIR 129 /* IIR Filter */
#define T1_FIR 130 /* FIR Filter */
#define T1_CTRL 131 /* Control Detector */
#define T1_MHD 132 /* Murdock Hutt Detector */
#define T1_THRD 133 /* Threshold Detector */
#define T1_NONCOMP 134 /* Non-compliant DP */
#define T1_DPLCQ 135 /* DP Logical Channel Queue */
/* 16 bit Length Field */
#define T2_CNAMES 192 /* Comm Event Names */
#define T2_ALERT 193 /* Email Alert */
#define T2_OPAQUE 194 /* Opaque Configuration */
#define OVERHEAD 10 /* Structure overhead */
typedef struct {
  tseed_net network ;
  tseed_stn station ;
} tstation ;

#ifndef OMIT_SEED
typedef struct { /* threshold detector definition */
  byte num ; /* detector number */
  byte filtnum ; /* filter number */
  byte iw ; /* iw parameter */
  byte nht ; /* nht parameter */
  longint fhi ; /* fhi/filhi parameter */
  longint flo ; /* flo/fillo parameter */
  word wa ; /* wa parameter */
  word spare ; /* needed for this compiler */
} tdet_thr ;
typedef struct { /* murdock-hutt detector definition */
  byte num ; /* detector number */
  byte filtnum ; /* filter number */
  byte iw ; /* iw parameter */
  byte nht ; /* nht parameter */
  longint fhi ; /* fhi/filhi parameter */
  longint flo ; /* flo/fillo parameter */
  word wa ; /* wa parameter */
  word spare ; /* needed for this compiler */
  word tc ; /* tc number */
  byte x1_2 ; /* x1 parameter / 2 */
  byte x2_2 ; /* x2 parameter / 2 */
  byte x3_2 ; /* x3 parameter / 2 */
  byte xx ; /* xx parameter */
  byte av ; /* av parameter */
} tdet_mh ;
#endif

extern void cfg_start (pq330 q330) ;
extern void read_q330_cfg (pq330 q330, pbyte pb) ;
extern void decode_cfg (pq330 q330) ;
extern void set_loc_name (plcq q) ;

#endif
