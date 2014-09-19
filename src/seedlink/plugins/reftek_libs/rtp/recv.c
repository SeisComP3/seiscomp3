#pragma ident "$Id: recv.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Receive a message from the peer.  The whole routine is mutex locked,
 * so multiple threads can issue receive requests independent of each
 * other (but why would you want to?).
 *
 *====================================================================*/
#include "rtp.h"

#ifdef WINNT 
typedef SSIZE_T ssize_t;
#endif

/* Local function for doing timeout enabled reads.  This assumes that
 * the file descripter has already been set for non-blocking I/O,
 * which is done as part of rtp_open() and rtp_accept().
 */

static BOOL read_to(RTP *rtp, UINT8 *buf, INT32 want, INT32 to)
{
size_t remain;
ssize_t got;
UINT8 *ptr;
int  error;
fd_set readfds;
struct timeval timeout;
static CHAR *fid = "read_to";

    if (want == 0) return TRUE;

    error           = 0;
    timeout.tv_sec  = to;
    timeout.tv_usec = 0;

/* Mask out our file descriptor as the only one to look at */

#ifndef FD_SETSIZE
#define FD_SETSIZE  getdtablesize()
#endif
    FD_ZERO(&readfds);
    FD_SET(rtp->sd, &readfds);
    
/*  Read from socket until desired number of bytes acquired  */

    remain = want;
    ptr    = buf;

    while (remain) {
        error = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
        if (error == 0) {
            errno = ETIMEDOUT;
            rtp->rcv.error = RTP_ERR_TRANSIENT;
            rtp_log(RTP_DEBUG, "%s:%hu timed out", rtp->peer, rtp->port);
            return FALSE;
        } else if (error < 0) {
            if (errno != EINTR && errno != EAGAIN) {
                rtp_log(RTP_ERR, "%s:%hu select: %s", 
                    rtp->peer, rtp->port, strerror(errno)
                );
                rtp->rcv.error = RTP_ERR_FATAL;
                return FALSE;
            }
        } else {
#ifdef WINNT
            got = recv(rtp->sd, ptr, remain, 0);
            if (got == SOCKET_ERROR) {
                if (WSAGetLastError() == WSAECONNRESET) {
                    got = 0;
                } else {
                    got = -1;
                }
            }
#else
            got = read(rtp->sd, (void *) ptr, (size_t) remain);
#endif
            if (got > 0) {
                remain -= got;
                ptr    += got;
            } else if (got == 0) {
                errno = ECONNRESET; /* actually it's EOF */
                rtp_log(RTP_ERR, "%s:%hu read: %s",
                    rtp->peer, rtp->port, strerror(errno)
                );
                rtp->rcv.error = RTP_ERR_TRANSIENT;
                return FALSE;
#ifdef WINNT
            } else if (WSAGetLastError() != 0) {
                rtp_log(RTP_DEBUG, "%s:%hu recv: %d",
                    rtp->peer, rtp->port, WSAGetLastError()
                );
                rtp->rcv.error = RTP_ERR_FATAL;
                return FALSE;
            } else if (WSAGetLastError() == 0) {
                rtp_log(RTP_DEBUG, "%s:%hu recv: %d (ignored)",
                    rtp->peer, rtp->port, WSAGetLastError()
                );
#endif
            } else if (errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR) {
                rtp_log(RTP_ERR, "%s:%hu read: error %d",
                    rtp->peer, rtp->port, errno
                );
                rtp->rcv.error = RTP_ERR_FATAL;
                return FALSE;
            } else {
                rtp_log(RTP_DEBUG, "%s:%hu read: error %d (ignored)",
                    rtp->peer, rtp->port, errno
                );
            }
        }
    }

    rtp->rcv.error = RTP_ERR_NONE;
    return TRUE;
}

#ifndef DRAINLEN
#define DRAINLEN 32
#endif

BOOL rtp_recv(RTP *rtp, UINT8 *buf, UINT16 *type, INT32 *datlen)
{
UINT16 stmp; size_t slen = 2;
UINT32 ltmp; size_t llen = 4;
INT32 to;
static CHAR *fid = "rtp_recv";
#ifdef DEBUG_RTP_IO
static char prtbuf[4096];
int i;
#endif

    if (rtp == (RTP *) NULL) {
        rtp_log(RTP_ERR, "%s: null rtp!", fid);
        errno = EINVAL;
        return FALSE;
    }

    to = rtp_timeout(rtp);
    MUTEX_LOCK(&rtp->rcv.mutex);

    /* Get 2-bytes of message type */

#ifdef DEBUG_RTP_IO
        prtbuf[0] = 0;
        sprintf(prtbuf, "RTPD-> [");
#endif
        if (!read_to(rtp, (UINT8 *) &stmp, (INT32) slen, to)) {
            MUTEX_UNLOCK(&rtp->rcv.mutex);
            if (errno != ETIMEDOUT) {
                rtp_log(RTP_ERR, "%s: read_to (type) failed", fid);
            }
            return FALSE;
        }
#ifdef DEBUG_RTP_IO
        sprintf(prtbuf+strlen(prtbuf), " %04X", stmp);
#endif

        *type = (UINT16) ntohs(stmp);

    /* Get 4-bytes of data length */

        if (!read_to(rtp, (UINT8 *) &ltmp, (INT32) llen, to)) {
            MUTEX_UNLOCK(&rtp->rcv.mutex);
            if (errno != ETIMEDOUT) {
                rtp_log(RTP_ERR, "%s: read_to(length) failed", fid);
            }
            return FALSE;
        }
#ifdef DEBUG_RTP_IO
        sprintf(prtbuf+strlen(prtbuf), " %08x]", ltmp);
#endif

        *datlen = (INT32) ntohl(ltmp);
        if (*datlen == 0) {
            MUTEX_UNLOCK(&rtp->rcv.mutex);
            return TRUE;
        }

        if (buf == (UINT8 *) NULL) {
            errno = EINVAL;
            rtp->rcv.error = RTP_ERR_FATAL;
            rtp_log(RTP_ERR, "%s: null buf!", fid);
            return FALSE;
        }

        if (!read_to(rtp, buf, *datlen, to)) {
            MUTEX_UNLOCK(&rtp->rcv.mutex);
            if (errno != ETIMEDOUT) {
                rtp_log(RTP_ERR, "%s: read_to(message) failed", fid);
            }
            return FALSE;
        }
#ifdef DEBUG_RTP_IO
        for (i = 0; i < *datlen; i++) {
            sprintf(prtbuf+strlen(prtbuf), " %02x", buf[i]);
        }
        rtp_log(RTP_INFO, "%s", prtbuf);
#endif

    MUTEX_UNLOCK(&rtp->rcv.mutex);

/* All done */

    return TRUE;
}

/* For clients that want to be 100% ignorant of the protocol,
 * get the next DAS packet (internally dispose of all other types).
 */

BOOL rtp_daspkt(RTP *rtp, UINT8 *buf, INT32 *datlen)
{
size_t slen = 2;
size_t llen = 4;
UINT16 type;
static CHAR *fid = "rtp_daspkt";

    if (rtp == (RTP *) NULL || buf == (UINT8 *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)!", fid);
        errno = EINVAL;
        return FALSE;
    }

    while (1) {

    /* Get the next message from the server */

        if (!rtp_recv(rtp, buf, &type, datlen)) {
            rtp_log(RTP_DEBUG, "%s: rtp_recv failed", fid);
            return FALSE;
        }

    /* Deal with it */

        if (type == RTP_MSG_REFTEK) {
            return TRUE;
        } else if (type == RTP_MSG_NOP && !rtp->attr.at_block) {
            rtp_log(RTP_DEBUG, "HEARTBEAT received");
            *datlen = 0;
            return TRUE;
        } else if (type == RTP_MSG_CMDPKT) {
            rtp_log(RTP_DEBUG, "COMMAND packet received/ignored");
        } else if (type == RTP_MSG_ATTR) {
            rtp_log(RTP_DEBUG, "ATTR received");
            rtp_attr_decode(buf, &rtp->attr);
        } else if (type == RTP_MSG_SOH) {
            rtp_log(RTP_DEBUG, "SOH received");
            rtp_soh_decode(buf, &rtp->soh);
        } else if (type == RTP_MSG_BUSY) {
            rtp_log(RTP_DEBUG, "BUSY signal received");
            errno = ECONNRESET;
            rtp->rcv.error = RTP_ERR_TRANSIENT;
            return FALSE;
        } else if (type == RTP_MSG_BREAK) {
            rtp_log(RTP_DEBUG, "BREAK received");
            errno = ECONNRESET;
            rtp->rcv.error = RTP_ERR_NONFATAL;
            return FALSE;
        } else if (type == RTP_MSG_FAULT) {
            rtp_log(RTP_ERR, "server fault!");
            errno = ECONNABORTED;
            rtp->rcv.error = RTP_ERR_FATAL;
            return FALSE;
        }
    }
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.5  2003/06/16 20:53:26  dchavez
 * use FD_SETSIZE macro instead of getdtablesize(), if applicable
 *
 * Revision 1.4  2003/05/22 20:29:52  phil
 * Fix: change declaration of 'ssize_t' for WINNT.
 *
 * Revision 1.3  2003/05/22 18:21:57  pdavidson
 * Fix: use 'ssize_t got' in 'read_to()'.
 *
 * Revision 1.3  2003/05/19 13:54:42  pdavidson
 * Fix 'read_to() declaration of 'got' from size_t to ssize_t.
 *
 * Revision 1.2  2002/01/18 17:57:49  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
