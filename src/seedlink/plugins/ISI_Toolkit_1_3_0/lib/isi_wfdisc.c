#pragma ident "$Id: wfdisc.c,v 1.9 2007/10/31 16:45:30 dechavez Exp $"
/*======================================================================
 *
 *  wfdisc related convenience functions
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

#ifdef ISI_SERVER
#include "cssio.h"

/* Send a single wfdisc record via IACP */

static BOOL SendWfdisc(IACP *iacp, WFDISC *wfdisc, UINT8 *buf)
{
IACP_FRAME frame;

    frame.payload.type = ISI_IACP_WFDISC;
    frame.payload.data = buf;
    cssioWfdiscString(wfdisc, (char *) frame.payload.data);
    frame.payload.len = strlen((char *) frame.payload.data);

    return iacpSendFrame(iacp, &frame);
}

/* Send a list of wfdisc records via IACP */

BOOL isiIacpSendWfdiscList(IACP *iacp, LNKLST *list)
{
LNKLST_NODE *crnt;
#define LOCAL_BUFLEN WFDISC_SIZE
UINT8 buf[LOCAL_BUFLEN];

    if (iacp == NULL || list == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    /* send over each element of the list */

    crnt = listFirstNode(list);
    while (crnt != NULL) {
        if (!SendWfdisc(iacp, (WFDISC *) crnt->payload, buf)) return FALSE;
        crnt = listNextNode(crnt);
    }

    /* send over a NULL to signal the end of the list */

    return iacpSendNull(iacp);
}

#else

#define WFDISC_SIZE 284

#endif /* !ISI_SERVER */

/* Receive wfdiscs */

BOOL isiIacpRecvWfdiscList(IACP *iacp, LNKLST *list)
{
time_t begin, ElapsedTime;
IACP_FRAME frame;
char string[WFDISC_SIZE];
#define LOCALBUFLEN (WFDISC_SIZE * 2)
UINT8 buf[LOCALBUFLEN];

    begin = time(NULL);

    do {

        if (!iacpRecvFrame(iacp, &frame, buf, LOCALBUFLEN)) return FALSE;

        switch (frame.payload.type) {

          case ISI_IACP_WFDISC:
            if (frame.payload.len == WFDISC_SIZE-1) {
                strlcpy(string, frame.payload.data, WFDISC_SIZE);
                if (!listAppend(list, string, WFDISC_SIZE)) return FALSE;
                begin = time(NULL);
            }
            break;

          case IACP_TYPE_NULL:
            return listSetArrayView(list);
            break;

          case IACP_TYPE_NOP:
            break;

          case IACP_TYPE_ENOSUCH:
            errno = UNSUPPORTED;
            return FALSE;

          case IACP_TYPE_ALERT:
            errno = ECONNABORTED;
            return FALSE;
        }

        ElapsedTime = time(NULL) - begin;

    } while (ElapsedTime < (time_t) iacpGetTimeoutInterval(iacp));

    errno = ETIMEDOUT;
    return FALSE;
}

LNKLST *isiRequestWfdisc(ISI *isi, int maxdur)
{
LNKLST *list;
static char *fid = "isiRequestWfdisc";

    if (isi == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if ((list = listCreate()) == NULL) return NULL;

    if (!iacpSendUINT32(isi->iacp, ISI_IACP_REQ_WFDISC, maxdur)) {
        isiLogMsg(isi, LOG_INFO, "%s: iacpSendUINT32 failed", fid);
        listDestroy(list);
        return NULL;
    }

    if (!isiIacpRecvWfdiscList(isi->iacp, list)) {
        isiLogMsg(isi, LOG_INFO, "%s: isiIacpRecvWfdiscList failed", fid);
        listDestroy(list);
        return NULL;
    }

    return list;
}

/* Revision History
 *
 * $Log: wfdisc.c,v $
 * Revision 1.9  2007/10/31 16:45:30  dechavez
 * replaced string memcpy with strlcpy
 *
 * Revision 1.8  2007/02/09 18:00:23  dechavez
 * ignore (instead of fail) wfdisc records that are the wrong length
 *
 * Revision 1.7  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.6  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.5  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.4  2003/12/10 05:52:29  dechavez
 * various explicit casts to calm solaris cc
 *
 * Revision 1.3  2003/11/13 19:22:56  dechavez
 * remove server side (#define ISI_SERVER) items from client side builds
 *
 * Revision 1.2  2003/11/03 23:10:50  dechavez
 * added isiRequestWfdisc()
 *
 * Revision 1.1  2003/10/16 15:38:53  dechavez
 * Initial release
 *
 */
