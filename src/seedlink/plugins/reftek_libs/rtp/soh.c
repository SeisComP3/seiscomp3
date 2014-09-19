#pragma ident "$Id: soh.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Encode, send, decode, SOH information (EVENTUALLY)
 *
 *====================================================================*/
#include "rtp.h"

/* encode an SOH structure */

static INT32 encode(UINT8 *buf, struct rtp_soh *soh)
{
    return 0;
}

/* Decode an SOH structure */

VOID rtp_soh_decode(UINT8 *buf, struct rtp_soh *soh)
{
static CHAR *fid = "rtp_soh_decode";

    if (buf == (UINT8 *) NULL || soh == (struct rtp_soh *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
    }

    return;
}

/* Send a SOH structure */

BOOL rtp_soh_send(RTP *rtp, struct rtp_soh *soh)
{
static CHAR *fid = "rtp_soh_send";

    if (rtp == (RTP *) NULL || soh == (struct rtp_soh *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return FALSE;
    }

    return TRUE;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:49  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
