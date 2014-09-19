#pragma ident "$Id: read.c,v 1.6 2006/09/29 19:45:39 dechavez Exp $"
/*======================================================================
 *
 * Read one frame, returning one of the following
 *
 * ISI_OK    - when frame was read OK
 * ISI_ERROR - when read failed
 * ISI_DONE  - when server disconnected normally
 * ISI_BREAK - when server disconnected, isi->alert has cause code
 *
 *
 *====================================================================*/
#include "isi.h"

int isiReadFrame(ISI *isi, BOOL SkipHeartbeats)
{
static char *fid = "isiReadFrame";

    if (isi == NULL) return ISI_EINVAL;

    while (iacpRecvFrame(isi->iacp, &isi->frame, isi->buf, ISI_INTERNAL_BUFLEN)) {
        switch (isi->frame.payload.type) {
          case IACP_TYPE_NOP:
            isiLogMsg(isi, LOG_DEBUG, "HEARTBEAT received");
            if (!SkipHeartbeats) return ISI_OK;
            break;
          case IACP_TYPE_ALERT:
            isi->alert = iacpAlertCauseCode(&isi->frame);
            return (isi->alert == IACP_ALERT_REQUEST_COMPLETE) ? ISI_DONE : ISI_BREAK;
          default:
            return ISI_OK;
        }
    }

    if (isi->iacp->recv.error == ETIMEDOUT) {
        return ISI_TIMEDOUT;
    } else if (isi->iacp->recv.error == ECONNRESET) {
        return ISI_CONNRESET;
    } else if (isi->iacp->recv.error == EBADMSG) {
        return ISI_BADMSG;
    }

    logioMsg(isi->lp, LOG_INFO, "%s: unexpected IACP receive error code %d", fid, isi->iacp->recv.error);
    logioMsg(isi->lp, LOG_INFO, "%s: seqno=%lu, type=%lu, len=%lu", fid, isi->frame.seqno, isi->frame.payload.type, isi->frame.payload.len);
    if (isi->frame.payload.len > 0) logioMsg(isi->lp, LOG_INFO, isi->frame.payload.data, isi->frame.payload.len);
    return ISI_ERROR;
}

/* Revision History
 *
 * $Log: read.c,v $
 * Revision 1.6  2006/09/29 19:45:39  dechavez
 * added test for EBADMSG
 *
 * Revision 1.5  2006/06/26 22:37:35  dechavez
 * check for and return ISI_TIMEDOUT and ISI_CONNRESET conditions
 *
 * Revision 1.4  2005/10/10 23:43:31  dechavez
 * debug tracers removed
 *
 * Revision 1.3  2005/09/30 22:54:39  dechavez
 * debug tracers added
 *
 * Revision 1.2  2005/06/30 01:29:07  dechavez
 * debugged isiReadFrame()
 *
 * Revision 1.1  2005/06/10 15:45:13  dechavez
 * initial release
 *
 */
