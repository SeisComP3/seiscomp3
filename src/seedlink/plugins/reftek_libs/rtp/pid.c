#pragma ident "$Id: pid.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Send/receive process ID's
 *
 *====================================================================*/
#include "rtp.h"

BOOL rtp_pid_decode(UINT8 *buf, INT32 *pid)
{
UINT32 ltmp; size_t llen = 4;
static CHAR *fid = "rtp_pid_decode";

    if (buf == (UINT8 *) NULL || pid == (INT32 *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        return FALSE;
    }

    memcpy((void *) &ltmp, (void *) buf, llen);
    *pid = (INT32) ntohl((u_long) ltmp);

    return TRUE;
}

BOOL rtp_pid_send(RTP *rtp)
{
UINT32 ltmp; size_t llen = 4;
UINT8 buf[4];

    ltmp = (UINT32) htonl((u_long) getpid());
    memcpy((void *) buf, (void *) &ltmp, llen);
    return rtp_send(rtp, buf, RTP_MSG_PID, llen);
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
