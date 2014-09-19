#pragma ident "$Id: soh.c,v 1.8 2007/01/23 02:50:43 dechavez Exp $"
/*======================================================================
 *
 *  ISI/IACP SOH related convenience functions
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

#ifdef ISI_SERVER

/* Send a single SOH record via IACP */

static BOOL SendStreamSoh(IACP *iacp, ISI_STREAM_SOH *soh, UINT8 *buf)
{
IACP_FRAME frame;

    frame.payload.type = ISI_IACP_STREAM_SOH;
    frame.payload.data = buf;
    frame.payload.len  = isiPackStreamSoh(frame.payload.data, soh);

    return iacpSendFrame(iacp, &frame);
}

/* Send a list of SOH records via IACP */

BOOL isiIacpSendStreamSohList(IACP *iacp, LNKLST *list)
{
LNKLST_NODE *crnt;
#define LOCAL_BUFLEN (sizeof(ISI_STREAM_SOH) * 2)
UINT8 buf[LOCAL_BUFLEN];

    if (iacp == NULL || list == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    /* send over each element of the list */

    crnt = listFirstNode(list);
    while (crnt != NULL) {
        if (!SendStreamSoh(iacp, (ISI_STREAM_SOH *) crnt->payload, buf)) return FALSE;
        crnt = listNextNode(crnt);
    }

    /* send over a NULL to signal the end of the list */

    return iacpSendNull(iacp);
}

#endif /* ISI_SERVER */

char *isiStreamSohString(ISI_STREAM_SOH *soh, char *buf)
{
char tmpbuf[256];

    buf[0] = 0;
    sprintf(buf+strlen(buf), "%5s %3s %3s", soh->name.sta, soh->name.chn, soh->name.loc);
    sprintf(buf+strlen(buf), " %5ld", soh->nseg);
    sprintf(buf+strlen(buf), " %8ld", soh->nrec);
    if (soh->tofs.value != (REAL64) ISI_UNDEFINED_TIMESTAMP) {
        sprintf(buf+strlen(buf), " %s", utilDttostr(soh->tofs.value, 0, tmpbuf));
    } else {
        sprintf(buf + strlen(buf), "           n/a        ");
    }
    if (soh->tols.value != (REAL64) ISI_UNDEFINED_TIMESTAMP) {
        sprintf(buf+strlen(buf), " %s", utilDttostr(soh->tols.value, 0, tmpbuf));
    } else {
        sprintf(buf + strlen(buf), "           n/a        ");
    }
    if (soh->nrec > 0) {
        sprintf(buf+strlen(buf), " %s", utilDttostr(soh->tslw, 8, tmpbuf));
    } else {
        sprintf(buf + strlen(buf), "      n/a      ");
    }

    return buf;
}

VOID isiPrintStreamSoh(FILE *fp, ISI_STREAM_SOH *soh)
{
char buf[1024];

    if (fp == NULL || soh == NULL) return;
    fprintf(fp, "%s\n", isiStreamSohString(soh, buf));
}

/* Receive stream SOH */

BOOL isiIacpRecvStreamSohList(IACP *iacp, LNKLST *list)
{
time_t begin, ElapsedTime;
IACP_FRAME frame;
ISI_STREAM_SOH soh;
#define LOCALBUFLEN (sizeof(ISI_STREAM_SOH)*2)
UINT8 buf[LOCALBUFLEN];

    begin = time(NULL);

    do {

        if (!iacpRecvFrame(iacp, &frame, buf, LOCALBUFLEN)) return FALSE;

        switch (frame.payload.type) {

          case ISI_IACP_STREAM_SOH:
            isiUnpackStreamSoh(frame.payload.data, &soh);
            if (!listAppend(list, &soh, sizeof(ISI_STREAM_SOH))) return FALSE;
            begin = time(NULL);
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

LNKLST *isiRequestSoh(ISI *isi)
{
LNKLST *list;
static char *fid = "isiRequestSoh";

    if (isi == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if ((list = listCreate()) == NULL) return NULL;

    if (!iacpSendEmptyMessage(isi->iacp, ISI_IACP_REQ_SOH)) {
        isiLogMsg(isi, LOG_INFO, "%s: iacpSendEmptyMessage failed", fid);
        listDestroy(list);
        return NULL;
    }

    if (!isiIacpRecvStreamSohList(isi->iacp, list)) {
        isiLogMsg(isi, LOG_INFO, "%s: isiIacpRecvStreamSohList failed", fid);
        listDestroy(list);
        return NULL;
    }

    return list;
}

/* Revision History
 *
 * $Log: soh.c,v $
 * Revision 1.8  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.7  2006/08/15 00:56:55  dechavez
 * treat nrec and nseg as signed ints in isiStreamSohString
 *
 * Revision 1.6  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.5  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.4  2003/11/19 23:48:01  dechavez
 * include util.h to calm compiler
 *
 * Revision 1.3  2003/11/13 19:22:43  dechavez
 * remove server side (#define ISI_SERVER) items from client side builds
 *
 * Revision 1.2  2003/11/03 23:15:55  dechavez
 * added isiRequestSoh(), detect ISI_UNDEFINED_TIMESTAMPs in isiStreamSohString() and print n/a's instead of time strings
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
