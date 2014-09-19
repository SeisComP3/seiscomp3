#pragma ident "$Id: cmdpkt.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Encode, send, decode, command packets
 *
 *====================================================================*/
#include "rtp.h"

/* encode an RTP_CMDPKT structure */

INT32 rtp_cmdpkt_encode(UINT8 *buf, RTP_CMDPKT *cmdpkt)
{
UINT16 stmp; size_t slen = 2;
UINT8 *ptr;

    ptr = buf;

    stmp = (UINT16) htons((u_short) cmdpkt->unit);
    memcpy((void *) ptr, (void *) &stmp, slen);
    ptr += slen;

    if (cmdpkt->len > RTP_MAXMSGLEN) cmdpkt->len = RTP_MAXMSGLEN;

    stmp = (UINT16) htons((u_short) cmdpkt->len);
    memcpy((void *) ptr, (void *) &stmp, slen);
    ptr += slen;

    memcpy((void *) ptr, (void *) cmdpkt->data, cmdpkt->len);
    ptr += cmdpkt->len;

    return (INT32) (ptr - buf);
}

/* Decode an RTP_CMDPKT structure */

VOID rtp_cmdpkt_decode(UINT8 *buf, RTP_CMDPKT *cmdpkt)
{
UINT16 stmp; size_t slen = 2;
UINT8 *ptr;
static CHAR *fid = "rtp_cmdpkt_decode";

    if (buf == (UINT8 *) NULL || cmdpkt == (RTP_CMDPKT *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
    }

    ptr = buf;

    memcpy((void *) &stmp, (void *) ptr, slen);
    cmdpkt->unit = (UINT16) ntohs((u_short) stmp);
    ptr += slen;

    memcpy((void *) &stmp, (void *) ptr, slen);
    cmdpkt->len = (UINT16) ntohs((u_short) stmp);
    ptr += slen;

    if (cmdpkt->len > RTP_MAXMSGLEN) cmdpkt->len = RTP_MAXMSGLEN;

    memcpy((void *) cmdpkt->data, (void *) ptr, cmdpkt->len);

    return;
}

/* Send a RTP_CMDPKT structure */

BOOL rtp_cmdpkt_send(RTP *rtp, RTP_CMDPKT *cmdpkt)
{
INT32  datlen;
UINT8 msg[sizeof(RTP_CMDPKT)];
static CHAR *fid = "rtp_cmdpkt_send";

    if (rtp == (RTP *) NULL || cmdpkt == (RTP_CMDPKT *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return FALSE;
    }

    datlen = rtp_cmdpkt_encode(msg, cmdpkt);

    return rtp_send(rtp, msg, RTP_MSG_CMDPKT, datlen);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:48  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
