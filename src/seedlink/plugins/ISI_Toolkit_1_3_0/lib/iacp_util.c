#pragma ident "$Id: util.c,v 1.14 2007/01/07 17:26:36 dechavez Exp $"
/*======================================================================
 *
 *  IACP convenience functions
 *
 *====================================================================*/
#define INCLUDE_IACP_DEFAULT_ATTR
#include "iacp.h"
#include "util.h"

/* Initialize all fields in handle */

VOID iacpInitHandle(IACP *iacp, char *peer, int port, IACP_ATTR *user_attr, LOGIO *lp, int debug)
{
IACP_ATTR attr;
static char *fid = "iacpInitHandle";

    if (iacp == NULL) return;

/* Set default attributes if none provided */

    if (user_attr != (IACP_ATTR *) NULL) {
        attr = *user_attr;
    } else {
        attr = IACP_DEFAULT_ATTR;
    }

/* Silently force timeout to be at least our minimum (so we can do
 * heartbeats at half that interval)
 */

    if (attr.at_timeo < IACP_MINTIMEO) attr.at_timeo = IACP_MINTIMEO;

/* Fill it in */

    MUTEX_INIT(&iacp->mutex);
    iacp->sd = INVALID_SOCKET;
    iacp->port = port;
    iacp->connect = 0;
    iacp->attr = attr;
    iacp->seqno = 0;
    iacp->lp = lp;
    iacp->debug = debug;
    iacp->disabled = FALSE;
    iacp->recv.status = 0;
    iacp->recv.error = 0;
    iacp->send.error = 0;
    iacpInitStats(&iacp->recv.stats, &iacp->mutex);
    iacpInitStats(&iacp->send.stats, &iacp->mutex);
    if (peer != NULL) {
        if ( strlen( peer ) > MAXPATHLEN-1 ) {
            strncpy( iacp->peer.name, peer, MAXPATHLEN-1);
            iacp->peer.name[MAXPATHLEN-1] = '\0';
        } else {
            strlcpy(iacp->peer.name, peer, MAXPATHLEN);
        }
    } else {
        memset(iacp->peer.name, 0, sizeof(iacp->peer.name));
    }
    memset(iacp->peer.addr, 0, sizeof(iacp->peer.addr));
    if (attr.at_dbgpath != NULL) {
        if ((iacp->dbgfp = fopen(attr.at_dbgpath, "ab")) == NULL) {
            logioMsg(lp, LOG_WARN, "%s: fopen: %s: %s (ignored)", fid, attr.at_dbgpath, strerror(errno));
        } else {
            logioMsg(lp, LOG_INFO, "logging traffic to %s", attr.at_dbgpath);
        }
    } else {
        iacp->dbgfp = NULL;
    }
}

/* Determine error severity */

int iacpErrorLevel(int ErrorCode)
{
    switch (ErrorCode) {
#ifdef _WIN32
      case WSAEINTR:
      case WSAETIMEDOUT:
      case WSAECONNREFUSED:
      case WSAECONNRESET:
      case WSAENETDOWN:
      case WSAENETRESET:
      case WSAENETUNREACH:
      case WSAEHOSTUNREACH:
      case WSAEHOSTDOWN:
#else
      case EINTR:
      case ETIMEDOUT:
      case ECONNREFUSED:
      case ECONNRESET:
      case ENETDOWN:
      case ENETRESET:
      case ENETUNREACH:
      case EHOSTUNREACH:
      case EHOSTDOWN:
#endif
        return IACP_ERR_TRANSIENT;
    }
    return IACP_ERR_FATAL;
}

/* Get a sure to be printable peer string */

char *iacpPeerIdent(IACP *iacp)
{
static char *nil = "NULL";

    return (iacp == NULL) ? nil : iacp->peer.ident;
}

/* Disconnect with peer notification */

VOID iacpDisconnect(IACP *iacp, UINT32 cause)
{

    if (iacp == NULL) return;

    if (iacpSendAlert(iacp, cause)) iacpClose(iacp);
}

/* Send a message consisting of a UINT32 payload */

BOOL iacpSendUINT32(IACP *iacp, UINT32 type, UINT32 value)
{
UINT32 buf;
IACP_FRAME frame;

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    frame.payload.type = type;
    frame.payload.len  = sizeof(UINT32);
    frame.payload.data = (UINT8 *) &buf;
    utilPackUINT32(frame.payload.data, value);

    return iacpSendFrame(iacp, &frame);

}

/* Send a message with no payload */

BOOL iacpSendEmptyMessage(IACP *iacp, UINT32 type)
{
IACP_FRAME frame;

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    frame.payload.type = type;
    frame.payload.len  = 0;
    frame.payload.data = NULL;

    return iacpSendFrame(iacp, &frame);

}

/* Decode cause code from alert subframes */

int iacpAlertCauseCode(IACP_FRAME *frame)
{
UINT32 cause;

    if (frame == NULL || frame->payload.type != IACP_TYPE_ALERT || frame->payload.data == NULL) {
        errno = EINVAL;
        return (int) IACP_EINVAL_UINT32;
    }

    utilUnpackUINT32(frame->payload.data, &cause);
    return (int) cause;
}

/* Convert at_maxerr attribute values into string equivalents */

char *iacpMaxerrString(UINT32 value)
{
static char *none      = "NONE";
static char *transient = "TRANSIENT";
static char *nonfatal  = "NON-FATAL";
static char *fatal     = "FATAL";
static char *unknown   = "UNKNOWN?";

    switch (value) {
      case IACP_ERR_NONE:
        return none;
      case IACP_ERR_TRANSIENT:
        return transient;
      case IACP_ERR_NONFATAL:
        return nonfatal;
      case IACP_ERR_FATAL:
        return fatal;
      default:
        return unknown;
    }
}

VOID iacpPrintAttr(FILE *fp, IACP_ATTR *attr)
{

    fprintf(fp, "Socket I/O timeout, msecs (at_timeo)  = %lu\n", attr->at_timeo);
    fprintf(fp, "TCP/IP transmit buffer    (at_sndbuf) = %lu\n", attr->at_sndbuf);
    fprintf(fp, "TCP/IP receive  buffer    (at_rcvbuf) = %lu\n", attr->at_rcvbuf);
    fprintf(fp, "I/O error tolerance       (at_maxerr) = %s\n", iacpMaxerrString(attr->at_maxerr));
    fprintf(fp, "Retry flag                (at_retry)  = %s\n", utilBoolToString(attr->at_retry));
    fprintf(fp, "Retry interval, msecs     (at_wait)   = %lu\n", attr->at_wait);

}

#define TSLEN 64
#define TSFMT "%Y:%j-%H:%M:%S"

VOID iacpDumpDataPreamble(IACP *iacp, IACP_FRAME *frame, char *prefix, UINT8 *raw, UINT32 len)
{
time_t now;
char tstamp[TSLEN];

    if (iacp == NULL) return;
    if (iacp->dbgfp == NULL) return;
    if (frame == NULL) return;

    now = time(NULL);
    if (strftime(tstamp, TSLEN, TSFMT, localtime(&now))) fprintf(iacp->dbgfp, "%s: ", tstamp);
    fprintf(iacp->dbgfp, "%s peer=%s seqno=%lu type=%lu payload len=%lu\n",
        prefix, iacpPeerIdent(iacp), frame->seqno, frame->payload.type, frame->payload.len
    );
    utilPrintHexDump(iacp->dbgfp, raw, len);
    fflush(iacp->dbgfp);
}

VOID iacpDumpDataPayload(IACP *iacp, IACP_FRAME *frame, char *prefix)
{
time_t now;
char tstamp[TSLEN];

    if (iacp == NULL) return;
    if (iacp->dbgfp == NULL) return;
    if (frame == NULL) return;

    now = time(NULL);
    if (strftime(tstamp, TSLEN, TSFMT, localtime(&now))) fprintf(iacp->dbgfp, "%s: %s (%lu bytes)\n", tstamp, prefix, frame->payload.len);
    utilPrintHexDump(iacp->dbgfp, frame->payload.data, frame->payload.len);
    fflush(iacp->dbgfp);
}

VOID iacpDumpSignaturePreamble(IACP *iacp, IACP_FRAME *frame, char *prefix, UINT8 *raw, UINT32 len)
{
time_t now;
char tstamp[TSLEN];

    if (iacp == NULL) return;
    if (iacp->dbgfp == NULL) return;
    if (frame == NULL) return;

    now = time(NULL);
    if (strftime(tstamp, TSLEN, TSFMT, localtime(&now))) fprintf(iacp->dbgfp, "%s: ", tstamp);
    fprintf(iacp->dbgfp, "%s peer=%s seqno=%lu id=%lu len=%lu\n",
        prefix, iacpPeerIdent(iacp), frame->seqno, frame->auth.id, frame->auth.len
    );
    utilPrintHexDump(iacp->dbgfp, raw, len);
    fflush(iacp->dbgfp);
}

/* Revision History
 *
 * $Log: util.c,v $
 * Revision 1.14  2007/01/07 17:26:36  dechavez
 * strlcpy() instead of strcpy()
 *
 * Revision 1.13  2005/09/30 22:17:35  dechavez
 * check for NULL iacp in iacpDisconnect()
 *
 * Revision 1.12  2005/05/25 22:37:39  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.11  2005/05/06 01:05:22  dechavez
 * added support for IACP_STATS
 *
 * Revision 1.10  2005/03/23 21:20:31  dechavez
 * fixed memory protection violation (on some platforms) (aap)
 *
 * Revision 1.9  2005/01/28 01:54:40  dechavez
 * added dbgfp and at_dbgpath support, fixed attr initialization error in
 * iacpInitHandle, and added new functions iacpDumpDataPreamble(),
 * iacpDumpDataPayload(), and iacpDumpSignaturePreamble
 *
 * Revision 1.8  2004/06/04 22:49:39  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.7  2004/01/29 18:44:14  dechavez
 * added iacpErrorLevel()
 *
 * Revision 1.6  2003/12/22 18:48:38  dechavez
 * iacpPeerIdent() NULL arg returns static string instead of NULL ptr
 *
 * Revision 1.5  2003/11/19 23:37:21  dechavez
 * fixed iacpInitHandle declaration
 *
 * Revision 1.4  2003/11/04 20:01:22  dechavez
 * included msec units in iacpPrintAttr()
 *
 * Revision 1.3  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:49:43  dechavez
 * Add iacpSendAlert and rework iacpDisconnect to use it.
 * Added iacpSendENOSUCH, iacpSendNull, iacpRetryInterval, iacpMaxError,
 * iacpRetryEnabled, and iacpSetRetryFlag
 *
 * Revision 1.1  2003/06/09 23:50:27  dechavez
 * initial release
 *
 */
