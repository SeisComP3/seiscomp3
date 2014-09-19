#pragma ident "$Id: send.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Send a message to the peer.  The whole routine is mutex locked, so
 * multiple threads can send messages independent of each other.
 *
 *====================================================================*/
#include "rtp.h"

/* Local function for doing timeout enabled writes.  This assumes that
 * the file descripter has already been set for non-blocking I/O,
 * which is done as part of rtp_open() for the clients, and rtp_accept()
 * for the servers.
 */

static BOOL write_to(RTP *rtp, UINT8 *buf, INT32 want, INT32 to)
{
INT32 remain, got;
UINT8 *ptr;
int  error, width;
fd_set writefds;
struct timeval timeout;
static CHAR *fid = "write_to";

    error           = 0;
    timeout.tv_sec  = to;
    timeout.tv_usec = 0;

/* Mask out our file descriptor as the only one to look at */

#ifndef WINNT
#   ifdef FD_SETSIZE
        width = FD_SETSIZE;
#   else
        width = getdtablesize();
#   endif
#else
    width = 0;
#endif

    FD_ZERO(&writefds);
    FD_SET(rtp->sd, &writefds);
    
/*  Write to socket until desired number of bytes sent */

    remain = want;
    ptr    = buf;

    while (remain) {
        error = select(width, NULL, &writefds, NULL, &timeout);
        if (error == 0) {
            errno = ETIMEDOUT;
            rtp_log(RTP_ERR, "%s:%hu select: %s", 
                rtp->peer, rtp->port, strerror(errno)
            );
            return FALSE;
        } else if (error < 0) {
            if (errno != EINTR && errno != EAGAIN) {
                rtp_log(RTP_ERR, "%s:%hu select: %s", 
                    rtp->peer, rtp->port, strerror(errno)
                );
                return FALSE;
            }
        } else {
#ifdef WINNT
            got = send(rtp->sd, ptr, remain, 0);
            if (got == SOCKET_ERROR) got = -1;
#else
            got = write(rtp->sd, (void *) ptr, (size_t) remain);
#endif
            if (got >= 0) {
                remain -= got;
                ptr    += got;
            } else if (
                errno != EWOULDBLOCK &&
                errno != EAGAIN      &&
                errno != EINTR
            ) {
                rtp_log(RTP_ERR, "%s:%hu write: %s", 
                    rtp->peer, rtp->port, strerror(errno)
                );
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* Send a message */

BOOL rtp_send(RTP *rtp, UINT8 *msg, UINT16 type, INT32 datlen)
{
INT32  to;
UINT16 stmp; size_t slen = 2;
UINT32 ltmp; size_t llen = 4;
static CHAR *fid = "rtp_send";
#ifdef DEBUG_RTP_IO
int i;
static char prtbuf[1024];
#endif

    if (rtp == (RTP *) NULL || (datlen > 0 && msg == (UINT8 *) NULL)) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return FALSE;
    }

    if (datlen < 0) {
        rtp_log(RTP_ERR, "%s: illegal datlen (%d)", fid, datlen);
        errno = EINVAL;
        return FALSE;
    }

    MUTEX_LOCK(&rtp->snd.mutex);

    /* First 2-bytes of every message is the type code */

        stmp = (UINT16) htons((u_short) type);
        memcpy((void *) rtp->snd.buf, (void *) &stmp, slen);

    /* Insert the 4-byte data length following the type code */

        ltmp = (UINT32) htonl((u_long) datlen);
        memcpy((void *) (rtp->snd.buf + 2), (void *) &ltmp, llen);

    /* Send the preamble */

#ifdef DEBUG_RTP_IO
        prtbuf[0] = 0;
        sprintf(prtbuf, "->RTPD: [");
        for (i = 0; i < RTP_PREAMBLE_LEN; i++) {
            sprintf(prtbuf+strlen(prtbuf), " %02x", rtp->snd.buf[i]);
        }
        sprintf(prtbuf+strlen(prtbuf), " ]");
#endif
        to = rtp_timeout(rtp);
        if (!write_to(rtp, rtp->snd.buf, RTP_PREAMBLE_LEN, to)) {
            rtp_log(RTP_ERR, "%s: write_to (preamble) failed", fid);
            MUTEX_UNLOCK(&rtp->snd.mutex);
            return FALSE;
        }

    /* Send the message data, if we have any */

#ifdef DEBUG_RTP_IO
        for (i = 0; i < datlen; i++) {
            sprintf(prtbuf+strlen(prtbuf), " %02x", msg[i]);
        }
#endif
        if (datlen) {
            if (!write_to(rtp, msg, datlen, to)) {
                rtp_log(RTP_ERR, "%s: write_to (message) failed", fid);
                MUTEX_UNLOCK(&rtp->snd.mutex);
                return FALSE;
            }
        }
#ifdef DEBUG_RTP_IO
        rtp_log(RTP_INFO, "%s", prtbuf);
#endif

    MUTEX_UNLOCK(&rtp->snd.mutex);

    return TRUE;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.3  2003/06/16 20:53:26  dchavez
 * use FD_SETSIZE macro instead of getdtablesize(), if applicable
 *
 * Revision 1.2  2002/01/18 17:57:49  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
