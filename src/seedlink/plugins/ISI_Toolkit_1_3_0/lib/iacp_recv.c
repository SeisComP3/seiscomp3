#pragma ident "$Id: recv.c,v 1.19 2007/03/26 18:13:18 dechavez Exp $"
/*======================================================================
 * 
 * Receive a frame from the peer.
 *
 *====================================================================*/
#include "iacp.h"
#include "util.h"

#define NOT_LOCKED FALSE

/* Set various values in the handle's recv field */

static VOID SetRecvError(IACP *iacp, int value)
{
    MUTEX_LOCK(&iacp->mutex);
        iacp->recv.error = value;
    MUTEX_UNLOCK(&iacp->mutex);
}

static VOID SetRecvStatus(IACP *iacp, int value)
{
    MUTEX_LOCK(&iacp->mutex);
        iacp->recv.status = value;
    MUTEX_UNLOCK(&iacp->mutex);
}

static VOID UpdateRecvTstamp(IACP *iacp)
{
time_t now;

    now = time(NULL);
    MUTEX_LOCK(&iacp->mutex);
        iacp->recv.tstamp = now;
    MUTEX_UNLOCK(&iacp->mutex);
}

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

/* Attempt to read some bytes */

static size_t ReadBytes(IACP *iacp, int LogLevel, void *ptr, size_t count)
{
size_t ReadResult;

#ifdef  _WIN32
    ReadResult = recv(iacp->sd, ptr, count,0);
#else
    ReadResult = read(iacp->sd, ptr, count);
#endif

    if (ReadResult == 0) errno = ECONNRESET;
    SetRecvError(iacp, errno);

    if (ReadResult > 0) return ReadResult;

    SetRecvStatus(iacp, iacpErrorLevel(errno));
    logioMsg(iacp->lp, LogLevel, "%s: read: %s", iacpPeerIdent(iacp), strerror(errno));

    return 0;
}

/* Local function for doing timeout enabled reads.  This assumes that
 * the file descripter has already been set for non-blocking I/O,
 * which is done as part of iacpOpen() and iacpAccept().
 */

static BOOL read_to(IACP *iacp, UINT8 *buf, UINT32 want)
{
size_t remain;
ssize_t ReadResult;
UINT8 *ptr;
int SelectResult;
int LogLevel;
fd_set readfds;
UINT32 timeoutUS;
struct timeval timeout;
int loops;        /*count up EAGAINS */
static char *fid = "read_to";

    LogLevel = iacpGetDebug(iacp) ? LOG_INFO : LOG_DEBUG;

    if (want == 0) return TRUE;

    SelectResult = 0;
    timeoutUS = iacpGetTimeoutInterval(iacp) * USEC_PER_MSEC;
    timeout.tv_sec  = timeoutUS / USEC_PER_SEC;
    timeout.tv_usec = timeoutUS - (timeout.tv_sec * USEC_PER_SEC);

/* Mask out our file descriptor as the only one to look at */

    FD_ZERO(&readfds);
    FD_SET(iacp->sd, &readfds);

/* Read from socket until desired number of bytes acquired  */

    loops=0;
    remain = want;
    ptr    = buf;

    while (remain) {
        SelectResult = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
        SetRecvError(iacp, errno);
        if (SelectResult > 0) {
            ReadResult = ReadBytes(iacp, LogLevel, (void *) ptr, (size_t) remain);
            if (ReadResult > 0) {
                remain -= ReadResult;
                ptr    += ReadResult;
            } else if (!IgnorableError(errno)) {
                logioMsg(iacp->lp, LogLevel, "%s: %s: ReadBytes: %s, return FALSE", iacpPeerIdent(iacp), fid, strerror(errno));
                return FALSE;
            } else {
                logioMsg(iacp->lp, LOG_INFO, "%s: %s: ReadBytes: %s (IGNORED), return FALSE", iacpPeerIdent(iacp), fid, strerror(errno));
#ifndef WIN32
                /* dck - found many EAGAINs causing disconnects. Just try again after a brief delay. 
                 * If we get 10 seconds or so of continuous problem, return FALSE and re-connect()
                 */
                usleep(10000);
                loops++;
                if(loops > 1000) return FALSE;
#endif /* !WIN32 */
            }
        } else if (SelectResult == 0) {
            SetRecvError(iacp, errno=ETIMEDOUT);
            SetRecvStatus(iacp, IACP_ERR_TRANSIENT);
            return FALSE;
        } else if (!IgnorableError(errno)) {
            logioMsg(iacp->lp, LogLevel, "%s: %s: select: %s", iacpPeerIdent(iacp), fid, strerror(errno));
            SetRecvStatus(iacp, iacpErrorLevel(errno));
            return FALSE;
        } else {
            logioMsg(iacp->lp, LOG_INFO, "%s: %s: select: %s (IGNORED)", iacpPeerIdent(iacp), fid, strerror(errno));
        }
    }
    if(loops >0) logioMsg(iacp->lp, LOG_INFO, "%s: %s: loops on EAGAIN=%d", iacpPeerIdent(iacp), fid, loops);

    UpdateRecvTstamp(iacp);
    SetRecvError(iacp, errno=0);
    SetRecvStatus(iacp, IACP_ERR_NONE);
    iacpIncrByteCounter(&iacp->recv.stats, want, NOT_LOCKED);
    return TRUE;
}

/* Receive an IACP frame */

BOOL iacpRecvFrame(IACP *iacp, IACP_FRAME *frame, UINT8 *buf, UINT32 buflen)
{
UINT8 *ptr;
UINT8  tmpbuf[IACP_MAX_PREAMBLE_LEN];
static char *fid = "iacpRecvFrame";

    if (iacp == (IACP *) NULL || frame == (IACP_FRAME *) NULL) {
        errno = EINVAL;
        return FALSE;
    }

    frame->seqno = 0;
    frame->payload.type = IACP_TYPE_NULL;
    frame->payload.len = 0;

/* Read the preamble */

    if (!read_to(iacp, (UINT8 *) tmpbuf, IACP_DATA_PREAMBLE_LEN)) {
        if (errno != ETIMEDOUT && errno != ECONNRESET) {
            logioMsg(iacp->lp, LOG_INFO, "%s: ERROR: read_to (data preamble): %s", fid, strerror(errno));
        }
        return FALSE;
    }

    if (memcmp(tmpbuf, IACP_TAG, IACP_TAGLEN) != 0) {
        errno = EPROTO;
        logioMsg(iacp->lp, LOG_INFO, "%s: ERROR: IACP tag missing", fid, strerror(errno));
        return FALSE;
    }

    ptr  = tmpbuf + IACP_TAGLEN;
    ptr += utilUnpackUINT32(ptr, &frame->seqno);
    ptr += utilUnpackUINT32(ptr, &frame->payload.type);
    ptr += utilUnpackUINT32(ptr, &frame->payload.len);
    iacpDumpDataPreamble(iacp, frame, "RECV preamble", tmpbuf, IACP_DATA_PREAMBLE_LEN);

/* Read the payload */

    if (frame->payload.len > buflen) {
        SetRecvError(iacp, errno=EBADMSG);
        SetRecvStatus(iacp, IACP_ERR_FATAL);
        logioMsg(iacp->lp, LOG_INFO, "%s: %s: payload too big (len=%lu, buflen=%lu)!",
            iacpPeerIdent(iacp), fid,  frame->payload.len, buflen
        );
        return FALSE;
    } else {
        frame->payload.data = buf;
        if (!read_to(iacp, frame->payload.data, frame->payload.len)) {
            logioMsg(iacp->lp, LOG_INFO, "%s: ERROR: read_to (data payload): %s", fid, strerror(errno));
            return FALSE;
        }
    }
    iacpDumpDataPayload(iacp, frame, "RECV payload");

/* Read the signature */

    if (!read_to(iacp, (UINT8 *) tmpbuf, IACP_SIG_PREAMBLE_LEN)) {
        logioMsg(iacp->lp, LOG_INFO, "%s: ERROR: read_to (sig preamble): %s", fid, strerror(errno));
        return FALSE;
    }
    ptr = tmpbuf;
    ptr += utilUnpackUINT32(ptr, &frame->auth.id);
    ptr += utilUnpackUINT32(ptr, &frame->auth.len);
    iacpDumpSignaturePreamble(iacp, frame, "RECV signature preamble", tmpbuf, IACP_SIG_PREAMBLE_LEN);
    if (frame->auth.len > IACP_MAXSIGLEN) {
        SetRecvError(iacp, errno=EBADMSG);
        SetRecvStatus(iacp, IACP_ERR_FATAL);
        logioMsg(iacp->lp, LOG_INFO, "%s: %s: signature block too big (len=%lu, IACP_MAXSIGLEN=%lu)!",
            iacpPeerIdent(iacp), fid,  frame->auth.len, IACP_MAXSIGLEN
        );
        return FALSE;
    } else {
        if (!read_to(iacp, frame->auth.sig, frame->auth.len)) {
            logioMsg(iacp->lp, LOG_INFO, "%s: ERROR: read_to (sig payload): %s", fid, strerror(errno));
            return FALSE;
        }
    }

/* Verify the data */

    iacpVerifyFrame(frame);

/* Increment the frame stats and we are done */

    iacpUpdateFrameStats(&iacp->recv.stats, frame->payload.len, NOT_LOCKED);

    return TRUE;
}

/* Revision History
 *
 * $Log: recv.c,v $
 * Revision 1.19  2007/03/26 18:13:18  dechavez
 * Retry all ignorable errors in read_to() at 10 msec intervals for up to 10 seconds - Dave Ketchum
 *
 * Revision 1.18  2007/01/23 02:52:58  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.17  2006/09/29 17:48:39  dechavez
 * init frame stuff on entry
 *
 * Revision 1.16  2005/10/10 23:36:33  dechavez
 * removed debug tracers
 *
 * Revision 1.15  2005/10/06 22:04:16  dechavez
 * more debug tracers added
 *
 * Revision 1.14  2005/09/30 22:56:14  dechavez
 * debug tracers added
 *
 * Revision 1.13  2005/06/29 18:28:25  dechavez
 * removed explicit USEC_PER_SEC defines in favor of those coming from util.h
 *
 * Revision 1.12  2005/05/06 01:05:22  dechavez
 * added support for IACP_STATS
 *
 * Revision 1.11  2005/03/23 21:23:47  dechavez
 * set errno and handle error flags with single call
 *
 * Revision 1.10  2005/01/28 01:57:53  dechavez
 * read in chunks which mimic send.c, as aid in debugging
 *
 * Revision 1.9  2004/06/04 22:49:39  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.8  2004/01/29 18:45:50  dechavez
 * cleaned up logic in read/write loops, log ignored errors
 *
 * Revision 1.7  2003/12/04 23:23:57  dechavez
 * added EWOULDBLOCK to list of ignored select failures, fixed bug causing
 * unneeded failures for ignored error codes
 *
 * Revision 1.6  2003/11/19 23:38:58  dechavez
 * tuned debug levels
 *
 * Revision 1.5  2003/11/04 20:03:15  dechavez
 * adjust timeout setup to account for switch from sec to msec
 *
 * Revision 1.4  2003/11/04 00:32:06  dechavez
 * tuned logging verbosity
 *
 * Revision 1.3  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:47:24  dechavez
 * rework retry loop to allow for changes in retry attribute to be respected,
 * rework error handling logic and log error messages instead of error codes
 *
 * Revision 1.1  2003/06/09 23:50:27  dechavez
 * initial release
 *
 */
