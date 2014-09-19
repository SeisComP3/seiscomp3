#pragma ident "$Id: type.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Determine record type
 *
	Revised  :
		03Sep05  (pld) add support for FD packets
					(pld) change sample rates to float
 *====================================================================*/
#include "reftek.h"

#define SOH   0x01
#define ATTEN 0x80
#define RT130_CMND 0x84
#define RT130_RESP 0x85

UINT16 reftek_type(UINT8 *raw)
{
    if (memcmp(raw, "AD", 2) == 0) return REFTEK_AD;
    if (memcmp(raw, "CD", 2) == 0) return REFTEK_CD;
    if (memcmp(raw, "DS", 2) == 0) return REFTEK_DS;
    if (memcmp(raw, "DT", 2) == 0) return REFTEK_DT;
    if (memcmp(raw, "EH", 2) == 0) return REFTEK_EH;
    if (memcmp(raw, "ET", 2) == 0) return REFTEK_ET;
    if (memcmp(raw, "OM", 2) == 0) return REFTEK_OM;
    if (memcmp(raw, "SC", 2) == 0) return REFTEK_SC;
    if (memcmp(raw, "SH", 2) == 0) return REFTEK_SH;
    if (memcmp(raw, "FD", 2) == 0) return REFTEK_FD;

    if (raw[0] == SOH   || raw[0] == RT130_RESP) return REFTEK_SPEC;
    if (raw[0] == ATTEN || raw[0] == RT130_CMND) return REFTEK_CMND;

    return 0;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:30  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.4  2005/09/03 21:52:32  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.3  2002/02/05 22:32:19  nobody
 * added support for RT130 command and response frames
 *
 * Revision 1.2  2002/01/18 17:55:59  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
