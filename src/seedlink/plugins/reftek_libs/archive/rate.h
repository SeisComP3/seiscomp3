#pragma ident "$Id: rate.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : rate.h
 Purpose  : Sampling derivation stuff.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  03 Oct 2005	---- (pld) change sample rates to REAL32, add 0.1
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#ifndef _RATE_H_INCLUDED_
#define _RATE_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include "archive.h"

/*---------------------------------------------------------------------*/
typedef struct _STASH_PACKET {
    RF_HEADER hdr;
    RF_PACKET rfp;
} STASH_PACKET;

typedef struct _STASH_LIST {
    UINT16 unit;
    UINT8 stream;
    UINT16 n_packets;
    LIST packets;
} STASH_LIST;

/*---------------------------------------------------------------------*/
/* Table of all valid sampling rates on 72A series DAS's */

#define N_RATES 17
REAL32 valid_rate[N_RATES] = {
    1000.0,
    500.0,
    250.0,
    200.0,
    125.0,
    100.0,
    50.0,
    40.0,
    25.0,
    20.0,
    10.0,
    8.0,
    5.0,
    4.0,
    2.0,
    1.0,
    0.1F
};

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2005/09/03 21:52:30  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
