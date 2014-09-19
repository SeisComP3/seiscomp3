#pragma ident "$Id: cnf.c,v 1.6 2007/01/23 02:50:43 dechavez Exp $"
/*======================================================================
 *
 *  ISI_STREAM_CNF convenience functions
 *
 *====================================================================*/
#include "isi.h"

#ifdef ISI_SERVER

/* Send a single configuration record via IACP */

static BOOL SendStreamCnf(IACP *iacp, ISI_STREAM_CNF *cnf, UINT8 *buf)
{
IACP_FRAME frame;

    frame.payload.type = ISI_IACP_STREAM_CNF;
    frame.payload.data = buf;
    frame.payload.len  = isiPackStreamCnf(frame.payload.data, cnf);

    return iacpSendFrame(iacp, &frame);
}

/* Send a list of configuration records via IACP */

BOOL isiIacpSendStreamCnfList(IACP *iacp, LNKLST *list)
{
LNKLST_NODE *crnt;
#define LOCAL_BUFLEN (sizeof(ISI_STREAM_CNF) * 2)
UINT8 buf[LOCAL_BUFLEN];

    if (iacp == NULL || list == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    /* send over each element of the list */

    crnt = listFirstNode(list);
    while (crnt != NULL) {
        if (!SendStreamCnf(iacp, (ISI_STREAM_CNF *) crnt->payload, buf)) return FALSE;
        crnt = listNextNode(crnt);
    }

    /* send over a NULL to signal the end of the list */

    return iacpSendNull(iacp);
}

#endif /* ISI_SERVER */

char *isiStreamCnfString(ISI_STREAM_CNF *cnf, char *buf)
{

    buf[0] = 0;
    sprintf(buf+strlen(buf), "%5s %3s %3s", cnf->name.sta, cnf->name.chn, cnf->name.loc);
    sprintf(buf+strlen(buf), " %7.3lf", isiSrateToSint(&cnf->srate));
    sprintf(buf+strlen(buf), " %9.4f", cnf->coords.lat);
    sprintf(buf+strlen(buf), " %9.4f", cnf->coords.lon);
    sprintf(buf+strlen(buf), " %9.2f", cnf->coords.elev);
    sprintf(buf+strlen(buf), " %9.2f", cnf->coords.depth);
    sprintf(buf+strlen(buf), " %11.4e", cnf->inst.calib);
    sprintf(buf+strlen(buf), " %11.4e", cnf->inst.calper);
    sprintf(buf+strlen(buf), " %7.2f", cnf->inst.hang);
    sprintf(buf+strlen(buf), " %7.2f", cnf->inst.vang);
    sprintf(buf+strlen(buf), " %s", cnf->inst.type);

    return buf;
}

VOID isiPrintStreamCnf(FILE *fp, ISI_STREAM_CNF *cnf)
{
char buf[1024];

    if (fp == NULL || cnf == NULL) return;
    fprintf(fp, "%s\n", isiStreamCnfString(cnf, buf));
}

/* Receive stream configuration */

BOOL isiIacpRecvStreamCnfList(IACP *iacp, LNKLST *list)
{
time_t begin, ElapsedTime;
IACP_FRAME frame;
ISI_STREAM_CNF cnf;
#define LOCALBUFLEN (sizeof(ISI_STREAM_CNF)*2)
UINT8 buf[LOCALBUFLEN];

    begin = time(NULL);

    do {

        if (!iacpRecvFrame(iacp, &frame, buf, LOCALBUFLEN)) return FALSE;

        switch (frame.payload.type) {

          case ISI_IACP_STREAM_CNF:
            isiUnpackStreamCnf(frame.payload.data, &cnf);
            if (!listAppend(list, &cnf, sizeof(ISI_STREAM_CNF))) return FALSE;
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

LNKLST *isiRequestCnf(ISI *isi)
{
LNKLST *list;
static char *fid = "isiRequestCnf";

    if (isi == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if ((list = listCreate()) == NULL) return NULL;

    if (!iacpSendEmptyMessage(isi->iacp, ISI_IACP_REQ_CNF)) {
        isiLogMsg(isi, LOG_INFO, "%s: iacpSendEmptyMessage failed", fid);
        listDestroy(list);
        return NULL;
    }

    if (!isiIacpRecvStreamCnfList(isi->iacp, list)) {
        isiLogMsg(isi, LOG_INFO, "%s: isiIacpRecvStreamCnfList failed", fid);
        listDestroy(list);
        return NULL;
    }

    return list;
}

/* Revision History
 *
 * $Log: cnf.c,v $
 * Revision 1.6  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.5  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.4  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.3  2003/11/13 19:22:22  dechavez
 * remove server side (#define ISI_SERVER) items from client side builds
 *
 * Revision 1.2  2003/11/03 23:54:18  dechavez
 * added isiRequestCnf()
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
