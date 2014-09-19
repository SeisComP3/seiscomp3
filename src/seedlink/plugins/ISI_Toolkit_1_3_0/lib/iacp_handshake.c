#pragma ident "$Id: handshake.c,v 1.5 2007/01/23 02:52:58 dechavez Exp $"
/*======================================================================
 * 
 * IACP handshake
 *
 *====================================================================*/
#include "iacp.h"
#include "util.h"

#define HANDSHAKE_PARAM_COUNT  5 /* id, timeout, sndbuf, rcvbuf, slop */
#define HANDSHAKE_PARAM_LEN (3 * sizeof(UINT32)) /* type, len, value */
#define HANDSHAKE_BUFLEN (sizeof(UINT32) + (HANDSHAKE_PARAM_COUNT * HANDSHAKE_PARAM_LEN))

static int UnpackParameter(IACP *iacp, UINT8 *start, int type, int len)
{
UINT8 *ptr;

    ptr = start;

    switch (type) {
      case IACP_TYPE_PID:
        utilUnpackUINT32(ptr, &iacp->peer.pid);
        break;
      case IACP_TYPE_TO:
        utilUnpackUINT32(ptr, &iacp->attr.at_timeo);
        break;
      case IACP_TYPE_SNDSIZ:
        utilUnpackUINT32(ptr, &iacp->attr.at_sndbuf);
      case IACP_TYPE_RCVSIZ:
        utilUnpackUINT32(ptr, &iacp->attr.at_rcvbuf);
        break;
      default:
        logioMsg(iacp->lp, LOG_WARN, "skip unknown handshake parameter: type %lu, len=%lu", type, len);
    }
    return len;
}

/* Build the handshake frame */

static void BuildHandshakeFrame(IACP *iacp, IACP_FRAME *frame, UINT8 *buf)
{
UINT8 *ptr;
UINT32 count;

/* start off with handshake ident */

    frame->payload.type = IACP_TYPE_HANDSHAKE;

/* skip the parameter count for now */

    count = 0;
    frame->payload.data = buf;
    ptr = frame->payload.data + sizeof(UINT32);

/* add process id */

    ptr += utilPackUINT32(ptr, IACP_TYPE_PID);
    ptr += utilPackUINT32(ptr, sizeof(UINT32));
    ptr += utilPackUINT32(ptr, (UINT32) getpid());
    ++count;

/* add i/o timeout interval */

    ptr += utilPackUINT32(ptr, IACP_TYPE_TO);
    ptr += utilPackUINT32(ptr, sizeof(UINT32));
    ptr += utilPackUINT32(ptr, iacp->attr.at_timeo);
    ++count;

/* add TCP/IP send buffer size */

    ptr += utilPackUINT32(ptr, IACP_TYPE_SNDSIZ);
    ptr += utilPackUINT32(ptr, sizeof(UINT32));
    ptr += utilPackUINT32(ptr, iacp->attr.at_sndbuf);
    ++count;

/* add TCP/IP receive buffer size */

    ptr += utilPackUINT32(ptr, IACP_TYPE_RCVSIZ);
    ptr += utilPackUINT32(ptr, sizeof(UINT32));
    ptr += utilPackUINT32(ptr, iacp->attr.at_rcvbuf);
    ++count;

/* insert the count at the front of the payload */

    utilPackUINT32(frame->payload.data, count);

/* set payload length */

    frame->payload.len = (UINT32) (ptr - frame->payload.data);
}

/* Decode handshake frame */

static BOOL DecodePeerFrame(char *fid, int LogLevel, IACP *iacp, IACP_FRAME *frame)
{
int ilen = sizeof(int);
UINT8 *ptr;
UINT32 i, count, type, len;

/* This had better be a handshake frame */

    if (frame->payload.type != IACP_TYPE_HANDSHAKE) {
        logioMsg(iacp->lp, LogLevel, "%s: protocol error (not IACP_TYPE_HANDSHAKE)", fid);
        return FALSE;
    }

/* Read all the parameters */

    ptr = frame->payload.data;
    ptr += utilUnpackUINT32(ptr, &count);

    for (i = 0; i < count; i++) {
        ptr += utilUnpackUINT32(ptr, &type);
        ptr += utilUnpackUINT32(ptr, &len);
        ptr += UnpackParameter(iacp, ptr, type, len);
    }

/* Match peer TCP/IP buffer lengths, if given */

    if (iacp->attr.at_sndbuf > 0) {
        setsockopt(
            iacp->sd,
            SOL_SOCKET,
            SO_SNDBUF,
            (char *) &iacp->attr.at_sndbuf,
            ilen
        );
        setsockopt(
            iacp->sd,
            SOL_SOCKET,
            SO_RCVBUF,
            (char *) &iacp->attr.at_sndbuf,
            ilen
        );
    }

    if (iacp->attr.at_rcvbuf > 0) {
        setsockopt(
            iacp->sd,
            SOL_SOCKET,
            SO_SNDBUF,
            (char *) &iacp->attr.at_rcvbuf,
            ilen
        );
        setsockopt(
            iacp->sd,
            SOL_SOCKET,
            SO_RCVBUF,
            (char *) &iacp->attr.at_rcvbuf,
            ilen
        );
    }

/* Update peer ident */

    sprintf(iacp->peer.ident, "pid%lu@%s", iacp->peer.pid, iacp->peer.name);

    return TRUE;
}

/* Client side of the handshake */

BOOL iacpClientHandshake(IACP *iacp)
{
int LogLevel;
IACP_FRAME frame;
UINT8 buf[HANDSHAKE_BUFLEN];
static char *fid = "iacpClientHandshake";

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    LogLevel = iacpGetDebug(iacp) ? LOG_INFO : LOG_DEBUG;

/* Build a frame with our attributes */

    BuildHandshakeFrame(iacp, &frame, buf);
    
/* Send it to the server */

    if (!iacpSendFrame(iacp, &frame)) {
        logioMsg(iacp->lp, LogLevel, "%s: iacpSendFrame failed", fid);
        return FALSE;
    }
 
/* Read back the server response */

    if (!iacpRecvFrame(iacp, &frame, buf, HANDSHAKE_BUFLEN)) {
        logioMsg(iacp->lp, LogLevel, "%s: iacpRecvFrame failed", fid);
        return FALSE;
    }

/* Decode/verify response */

    if (!DecodePeerFrame(fid, LogLevel, iacp, &frame)) return FALSE;

/* Handshake complete */

    return TRUE;
}

/* Server side of the handshake */

BOOL iacpServerHandshake(IACP *iacp)
{
int LogLevel;
IACP_FRAME frame;
UINT8 buf[HANDSHAKE_BUFLEN];
static char *fid = "iacpServerHandshake";

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    LogLevel = iacpGetDebug(iacp) ? LOG_INFO : LOG_DEBUG;
    
/* Client sends initial frame */

    if (!iacpRecvFrame(iacp, &frame, buf, HANDSHAKE_BUFLEN)) {
        logioMsg(iacp->lp, LogLevel, "%s: iacpRecvFrame failed", fid);
        return FALSE;
    }

/* Decode/verify response */

    if (!DecodePeerFrame(fid, LogLevel, iacp, &frame)) return FALSE;

/* Reply with a frame with final attributes */

    BuildHandshakeFrame(iacp, &frame, buf);

    if (!iacpSendFrame(iacp, &frame)) {
        logioMsg(iacp->lp, LogLevel, "%s: iacpSendFrame failed", fid);
        return FALSE;
    }
 
/* Handshake complete */

    return TRUE;
}

/* Revision History
 *
 * $Log: handshake.c,v $
 * Revision 1.5  2007/01/23 02:52:58  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.4  2003/12/04 23:51:09  dechavez
 * removed test for negative parameter count in handshake (unsigned ints
 * are always non-negative)
 *
 * Revision 1.3  2003/11/19 23:40:04  dechavez
 * unpack UINT32's with proper unpacker (benign)
 *
 * Revision 1.2  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.1  2003/06/09 23:50:26  dechavez
 * initial release
 *
 */
