#pragma ident "$Id: version.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Send/receive protocol version numbers.
 *
 *====================================================================*/
#include "rtp.h"

INT16 rtp_version_recv(RTP *rtp)
{
UINT16 type;
INT32  zero = 0;

    if (!rtp_recv(rtp, (UINT8 *) NULL, &type, &zero)) return (INT16) -1;
    return (INT16) type;
}

BOOL rtp_version_send(RTP *rtp)
{
UINT16 type;

    type = (UINT16) RTP_VERSION;
    return rtp_send(rtp, (UINT8 *) NULL, type, 0);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:50  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
