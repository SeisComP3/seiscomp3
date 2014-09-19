#pragma ident "$Id: send.c,v 1.16 2007/02/08 23:10:45 dechavez Exp $"
/*======================================================================
 * 
 * Send a frame to the peer.
 *
 *====================================================================*/
#include "iacp.h"
#include "util.h"

#define ALREADY_LOCKED TRUE

/* Some errors should be ignored */

static BOOL IgnorableError(int error)
{
    switch (error) {
      case EWOULDBLOCK:
#if (EWOULDBLOCK != EAGAIN)
      case EAGAIN:
#endif
      case EINTR:
        return TRUE;
    }

    return FALSE;
}

/* Local function for doing timeout enabled writes.  This assumes that
 * the file descripter has already been set for non-blocking I/O,
 * which is done as part of iacpOpen() for the clients, and iacpAccept()
 * for the servers.
 */

static BOOL write_to(IACP *iacp, UINT8 *buf, UINT32 want)
{
size_t remain;
ssize_t WriteResult;
UINT8 *ptr;
int SelectResult;
int LogLevel;
UINT32 timeoutUS;
fd_set writefds;
struct timeval timeout;
static char *fid = "write_to";

    if (want == 0) return TRUE;

    LogLevel = iacp->debug ? LOG_INFO : LOG_DEBUG;

    if (buf == NULL) {
        errno = EINVAL;
        iacp->send.error = errno;
        logioMsg(iacp->lp, LogLevel, "%s: NULL buf!", iacpPeerIdent(iacp), fid);
        return FALSE;
    }

    SelectResult = 0;
    timeoutUS = iacp->attr.at_timeo * USEC_PER_MSEC;
    timeout.tv_sec  = timeoutUS / USEC_PER_SEC;
    timeout.tv_usec = timeoutUS - (timeout.tv_sec * USEC_PER_SEC);

/* Mask out our file descriptor as the only one to look at */

    FD_ZERO(&writefds);
    FD_SET(iacp->sd, &writefds);
    
/*  Write to socket until desired number of bytes sent */

    remain = want;
    ptr    = buf;

    while (remain) {
        SelectResult = select(FD_SETSIZE, NULL, &writefds, NULL, &timeout);
        iacp->send.error = errno;
        if (SelectResult == 0) {
            errno = ETIMEDOUT;
            iacp->send.error = errno;
            logioMsg(iacp->lp, LogLevel, "%s: %s: select: %s: to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
            return FALSE;
        } else if (SelectResult < 0) {
            if (!IgnorableError(errno)) {
                logioMsg(iacp->lp, LogLevel, "%s: %s: select: %s: to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
                return FALSE;
            } else {
                logioMsg(iacp->lp, LOG_INFO, "%s: %s: select: %s (IGNORED): to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
            }
        } else {
            WriteResult = send(iacp->sd, (void *) ptr, remain, 0);
            iacp->send.error = errno;
            if (WriteResult > 0) {
                remain -= WriteResult;
                ptr    += WriteResult;
            } else if (WriteResult == 0) {
                errno = ECONNRESET;
                iacp->send.error = errno;
                logioMsg(iacp->lp, LogLevel, "%s: %s: send: %s: to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
                return FALSE;
            } else if (!IgnorableError(errno)) {
                logioMsg(iacp->lp, LogLevel, "%s: %s: send: %s: to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
                return FALSE;
            } else {
                logioMsg(iacp->lp, LOG_INFO, "%s: %s: send: %s (IGNORED): to=%d.%06d", iacpPeerIdent(iacp), fid, strerror(errno), timeout.tv_sec, timeout.tv_usec);
            }
        }
    }

    iacp->send.tstamp = time(NULL);
    iacp->send.error = 0;
    iacpIncrByteCounter(&iacp->send.stats, want, ALREADY_LOCKED);
    return TRUE;
}

static BOOL ReleaseLock(IACP *iacp, BOOL result)
{
    if (result == FALSE) iacp->disabled = TRUE;
    MUTEX_UNLOCK(&iacp->mutex);
    return result;
}

/* Send an IACP frame */

BOOL iacpSendFrame(IACP *iacp, IACP_FRAME *frame)
{
int LogLevel;
UINT8 *ptr;
UINT8 tmpbuf[IACP_MAX_PREAMBLE_LEN];
static char *fid = "iacpSendFrame";

    if (iacp == (IACP *) NULL || frame == (IACP_FRAME *) NULL) {
        errno = EINVAL;
        return FALSE;
    }

    LogLevel = iacp->debug ? LOG_INFO : LOG_DEBUG;

    MUTEX_LOCK(&iacp->mutex); /* Only one thread at a time can be here! */
    if (iacp->disabled) return ReleaseLock(iacp, FALSE);

    /* build and send payload preamble */

    frame->seqno = iacp->seqno++;

    ptr = tmpbuf;
    ptr += utilPackBytes(ptr, (UINT8 *) IACP_TAG, IACP_TAGLEN);
    ptr += utilPackUINT32(ptr, frame->seqno);
    ptr += utilPackUINT32(ptr, frame->payload.type);
    ptr += utilPackUINT32(ptr, frame->payload.len);
    iacpDumpDataPreamble(iacp, frame, "SEND preamble", tmpbuf, IACP_DATA_PREAMBLE_LEN);
    if (!write_to(iacp, tmpbuf, IACP_DATA_PREAMBLE_LEN)) {
        logioMsg(iacp->lp, LogLevel, "%s: %s: error sending %ld byte message preamble: %s", iacpPeerIdent(iacp), fid, IACP_DATA_PREAMBLE_LEN, strerror(errno));
        return ReleaseLock(iacp, FALSE);
    }

    /* send payload */

    iacpDumpDataPayload(iacp, frame, "SEND payload");
    if (!write_to(iacp, frame->payload.data, frame->payload.len)) {
        logioMsg(iacp->lp, LogLevel, "%s: %s: error sending %ld byte message: %s", iacpPeerIdent(iacp), fid, frame->payload.len, strerror(errno));
        return ReleaseLock(iacp, FALSE);
    }

    /* sign frame and build and send signature preamble */

    iacpSignFrame(iacp, frame);
    ptr = tmpbuf;
    ptr += utilPackUINT32(ptr, frame->auth.id);
    ptr += utilPackUINT32(ptr, frame->auth.len);
    iacpDumpSignaturePreamble(iacp, frame, "SEND signature preamble", tmpbuf, IACP_SIG_PREAMBLE_LEN);
    if (!write_to(iacp, tmpbuf, IACP_SIG_PREAMBLE_LEN)) {
        logioMsg(iacp->lp, LogLevel, "%s: %s: error sending %ld byte signature preamble: %s", iacpPeerIdent(iacp), fid, IACP_SIG_PREAMBLE_LEN, strerror(errno));
        return ReleaseLock(iacp, FALSE);
    }

    /* send the signature */

    if (!write_to(iacp,  frame->auth.sig, frame->auth.len)) {
        logioMsg(iacp->lp, LogLevel, "%s: %s: error sending %ld byte signature: %s", iacpPeerIdent(iacp), fid, frame->auth.len, strerror(errno));
        return ReleaseLock(iacp, FALSE);
    }

    iacpUpdateFrameStats(&iacp->send.stats, frame->payload.len, ALREADY_LOCKED);

    return ReleaseLock(iacp, TRUE);
}

/* Revision History
 *
 * $Log: send.c,v $
 * Revision 1.16  2007/02/08 23:10:45  dechavez
 * use send() instead of write()
 *
 * Revision 1.15  2007/01/23 02:52:58  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.14  2006/09/29 17:47:18  dechavez
 * set disabled flag on I/O errors, check for flag and fail immediately if set,
 * log each write_to failure
 *
 * Revision 1.13  2005/10/10 23:38:04  dechavez
 * removed debug tracers
 *
 * Revision 1.12  2005/09/30 22:56:14  dechavez
 * debug tracers added
 *
 * Revision 1.11  2005/06/29 18:28:26  dechavez
 * removed explicit USEC_PER_SEC defines in favor of those coming from util.h
 *
 * Revision 1.10  2005/05/06 01:05:22  dechavez
 * added support for IACP_STATS
 *
 * Revision 1.9  2005/02/11 17:52:10  dechavez
 * win32 portability mods (aap)
 *
 * Revision 1.8  2005/01/28 01:56:15  dechavez
 * wrap all of iacpSendFrame() with handle mutex, use new dump debug funcs
 *
 * Revision 1.7  2004/01/29 18:45:50  dechavez
 * cleaned up logic in read/write loops, log ignored errors
 *
 * Revision 1.6  2003/12/04 23:21:50  dechavez
 * added EWOULDBLOCK to list of ignored select failures
 *
 * Revision 1.5  2003/11/19 23:38:31  dechavez
 * tuned debug levels, added some casts to calm compiler
 *
 * Revision 1.4  2003/11/04 20:03:15  dechavez
 * adjust timeout setup to account for switch from sec to msec
 *
 * Revision 1.3  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:47:52  dechavez
 * log error messages instead of error codes
 *
 * Revision 1.1  2003/06/09 23:50:27  dechavez
 * initial release
 *
 */
