#pragma ident "$Id: com.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Decode the common header of every RefTek packet.
 *
 *====================================================================*/
#include "private.h"

/* Offsets to the various pieces */

#define EXPN_OFF     2  /* experiment number */
#define YEAR_OFF     3  /* year              */
#define UNIT_OFF     4  /* unit id           */
#define TIME_OFF     6  /* time stamp        */
#define SEQN_OFF    14  /* sequence number   */

VOID reftek_com(
    UINT8 *src, UINT16 *exp, UINT16 *unit, UINT16 *seqno, REAL64 *tstamp
) {
UINT16 yr, da, hr, mn, sc, ms, stmp;

    yr = (UINT16) utilBcdToUint32(src + YEAR_OFF,   2, 0); 
    yr += (yr < 88) ? 2000 : 1900;  /* WARNING: fix this before 2099! */

    da = (UINT16) utilBcdToUint32(src + TIME_OFF,   3, 0);
    hr = (UINT16) utilBcdToUint32(src + TIME_OFF+1, 2, 1);
    mn = (UINT16) utilBcdToUint32(src + TIME_OFF+2, 2, 1);
    sc = (UINT16) utilBcdToUint32(src + TIME_OFF+3, 2, 1);
    ms = (UINT16) utilBcdToUint32(src + TIME_OFF+4, 3, 1);

    *exp  = (UINT16) utilBcdToUint32(src + EXPN_OFF,   2, 0); 
	memcpy(&stmp, src + UNIT_OFF, 2);
	*unit = (UINT16) ntohs(stmp);
	
    *seqno  = (UINT16) utilBcdToUint32(src + SEQN_OFF,   4, 0);
    *tstamp = util_ydhmsmtod(yr, da, hr, mn, sc, ms);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:55:55  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
