#pragma ident "$Id: datreq.c,v 1.28 2008/01/25 23:10:01 dechavez Exp $"
/*======================================================================
 *
 *  ISI/IACP data request support
 *
 *====================================================================*/
#define INCLUDE_ISI_STATIC_SEQNOS
#include "isi.h"
#include "util.h"

/* Print the contents of a data request */

VOID isiPrintReqTime(FILE *fp, REAL64 value)
{
char sbuf[256];
static char *oldest_string = "<  beg of disk loop >";
static char *newest_string = "<  end of disk loop >";
static char *keepup_string = "<       forever     >";

    if (value == ISI_OLDEST) {
        fprintf(fp, " %s", oldest_string);
    } else if (value == ISI_NEWEST) {
        fprintf(fp, " %s", newest_string);
    } else if (value == ISI_KEEPUP) {
        fprintf(fp, " %s", keepup_string);
    } else {
        fprintf(fp, " %s", utilDttostr(value, 0, sbuf));
    }
}

VOID isiPrintTwindReq(FILE *fp, ISI_TWIND_REQUEST *req)
{
    fprintf(fp, "'%s'.'%s'.'%s'", req->name.sta, req->name.chn, req->name.loc);
    isiPrintReqTime(fp, req->beg);
    isiPrintReqTime(fp, req->end);
}

VOID isiPrintSeqnoReq(FILE *fp, ISI_SEQNO_REQUEST *req)
{
char buf[256];

    fprintf(fp, "%s, ", req->site);
    fprintf(fp, "%s, ", isiSeqnoString(&req->beg, buf));
    fprintf(fp, "%s\n", isiSeqnoString(&req->end, buf));
}

VOID isiPrintDatreq(FILE *fp, ISI_DATA_REQUEST *datreq)
{
int i;

    fprintf(fp, "*** beg ISI_DATA_REQUEST ***\n");
    fprintf(fp, "       type = %lu (%s)\n", datreq->type, isiRequestTypeString(datreq->type));
    fprintf(fp, "     policy = %lu (%s)\n", datreq->policy, isiPolicyString(datreq->policy));
    fprintf(fp, "     format = %lu (%s)\n", datreq->format, isiFormatString(datreq->format));
    fprintf(fp, "   compress = %lu (%s)\n", datreq->compress, isiCompressString(datreq->compress));
    fprintf(fp, "    options = 0x%08xu\n", datreq->options);
    fprintf(fp, "       nreq = %lu\n", datreq->nreq);
    if (datreq->type == ISI_REQUEST_TYPE_TWIND) {
        for (i = 0; i < (int) datreq->nreq; i++) {
            fprintf(fp, "   %d: ", i+1);
            isiPrintTwindReq(fp, &datreq->req.twind[i]);
            fprintf(fp, "\n");
        }
    } else if (datreq->type == ISI_REQUEST_TYPE_SEQNO) {
        for (i = 0; i < (int) datreq->nreq; i++) {
            fprintf(fp, "   %d: ", i+1);
            isiPrintSeqnoReq(fp, &datreq->req.seqno[i]);
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "*** end ISI_DATA_REQUEST ***\n");
}

VOID isiLogDatreq(LOGIO *lp, int level, ISI_DATA_REQUEST *datreq)
{
int i;
char buf[256];

    logioMsg(lp, level, "*** beg ISI_DATA_REQUEST ***");
    logioMsg(lp, level, "       type = %lu (%s)", datreq->type, isiRequestTypeString(datreq->type));
    logioMsg(lp, level, "     policy = %lu (%s)", datreq->policy, isiPolicyString(datreq->policy));
    logioMsg(lp, level, "     format = %lu (%s)", datreq->format, isiFormatString(datreq->format));
    logioMsg(lp, level, "   compress = %lu (%s)", datreq->compress, isiCompressString(datreq->compress));
    logioMsg(lp, level, "    options = 0x%08x", datreq->options);
    logioMsg(lp, level, "       nreq = %lu\n", datreq->nreq);
    if (datreq->type == ISI_REQUEST_TYPE_TWIND) {
        for (i = 0; i < (int) datreq->nreq; i++) {
            logioMsg(lp, level, "   %d: %s", i+1, isiTwindRequestString(&datreq->req.twind[i], buf));
        }
    } else if (datreq->type == ISI_REQUEST_TYPE_SEQNO) {
        for (i = 0; i < (int) datreq->nreq; i++) {
            logioMsg(lp, level, "   %d: %s", i+1, isiSeqnoRequestString(&datreq->req.seqno[i], buf));
        }
    }
    logioMsg(lp, level, "*** end ISI_DATA_REQUEST ***");
}

VOID isiSetDatreqType(ISI_DATA_REQUEST *datreq, UINT32 value)
{
    if (datreq == NULL) return;
    datreq->type = value;
}

VOID isiSetDatreqPolicy(ISI_DATA_REQUEST *datreq, UINT32 value)
{
    if (datreq == NULL) return;
    datreq->policy = value;
}

VOID isiSetDatreqFormat(ISI_DATA_REQUEST *datreq, UINT32 value)
{
    if (datreq == NULL) return;
    datreq->format = value;
}

VOID isiSetDatreqCompress(ISI_DATA_REQUEST *datreq, UINT32 value)
{
    if (datreq == NULL) return;
    datreq->compress = value;
}

UINT32 isiGetDatreqCompress(ISI_DATA_REQUEST *datreq)
{
    if (datreq == NULL) {
        errno = EINVAL;
        return ISI_COMP_UNDEF;
    }

    return datreq->compress;
}

VOID isiSetDatreqOptions(ISI_DATA_REQUEST *datreq, UINT32 value)
{
    if (datreq == NULL) return;
    datreq->options = value;
}

UINT32 isiGetDatreqOptions(ISI_DATA_REQUEST *datreq)
{
    if (datreq == NULL) {
        errno = EINVAL;
        return ISI_DEFAULT_OPTIONS;
    }

    return datreq->options;
}

/* initialize a data request structure */

VOID isiInitDataRequest(ISI_DATA_REQUEST *req)
{
static char *fid = "isiInitDataRequest";

    if (req == NULL) return;

    req->type      = ISI_DEFAULT_REQUEST_TYPE;
    req->policy    = ISI_DEFAULT_RECONNECT_POLICY;
    req->format    = ISI_DEFAULT_FORMAT;
    req->compress  = ISI_DEFAULT_COMP;
    req->options   = ISI_DEFAULT_OPTIONS;
    req->nreq      = 0;
    req->req.twind = NULL;
    req->req.seqno = NULL;
    listInit(&req->slist);
}

/* clear a data request structure */

VOID isiClearDataRequest(ISI_DATA_REQUEST *req)
{
static char *fid = "isiClearDataRequest";

    if (req == NULL) return;

    if (req->nreq != 0) {
        if (req->req.twind != NULL) {
            free(req->req.twind);
            req->req.twind = NULL;
        }
        if (req->req.seqno != NULL) {
            free(req->req.seqno);
            req->req.seqno = NULL;
        }
    }
    req->nreq = 0;
    listClear(&req->slist);

    isiInitDataRequest(req);
}

/* Find a specific time window request */

ISI_TWIND_REQUEST *isiLocateTwindRequest(ISI_STREAM_NAME *target, ISI_DATA_REQUEST *datreq)
{
UINT32 i;

    if (target == NULL || datreq == NULL || datreq->req.twind == NULL) {
        errno = EINVAL;
        return NULL;
    }

    for (i = 0; i < datreq->nreq; i++) {
        if (isiStreamNameMatch(&datreq->req.twind[i].name, target)) return &datreq->req.twind[i];
    }

    return NULL;
}

/* Find a specific seqno request */

ISI_SEQNO_REQUEST *isiLocateSeqnoRequest(char *target, ISI_DATA_REQUEST *datreq)
{
UINT32 i;

    if (target == NULL || datreq == NULL || datreq->req.seqno == NULL) {
        errno = EINVAL;
        return NULL;
    }

    for (i = 0; i < datreq->nreq; i++) {
        if (strcmp(datreq->req.seqno[i].site, target)) return &datreq->req.seqno[i];
    }

    return NULL;
}

/* send a data request */

static BOOL SendTwindRequest(IACP *iacp, ISI_TWIND_REQUEST *req)
{
IACP_FRAME frame;
#define LOCAL_BUFLEN1 (sizeof(ISI_TWIND_REQUEST) * 2)
UINT8 buf[LOCAL_BUFLEN1];
static char *fid = "SendTwindRequest";

    memset(buf, 0xee, LOCAL_BUFLEN1);

    frame.payload.type = ISI_IACP_REQ_TWIND;
    frame.payload.data = buf;
    frame.payload.len  = isiPackTwindRequest(frame.payload.data, req);

    return iacpSendFrame(iacp, &frame);
}

static BOOL SendSeqnoRequest(IACP *iacp, ISI_SEQNO_REQUEST *req)
{
IACP_FRAME frame;
#define LOCAL_BUFLEN2 (sizeof(ISI_SEQNO_REQUEST) * 2)
UINT8 buf[LOCAL_BUFLEN2];

    memset(buf, 0xee, LOCAL_BUFLEN2);

    frame.payload.type = ISI_IACP_REQ_SEQNO;
    frame.payload.data = buf;
    frame.payload.len  = isiPackSeqnoRequest(frame.payload.data, req);

    return iacpSendFrame(iacp, &frame);
}

static BOOL SendStreamName(IACP *iacp, ISI_STREAM_NAME *name)
{
IACP_FRAME frame;
#define LOCAL_BUFLEN3 (sizeof(ISI_STREAM_NAME) * 2)
UINT8 buf[LOCAL_BUFLEN3];

    memset(buf, 0xee, LOCAL_BUFLEN3);

    frame.payload.type = ISI_IACP_REQ_STREAM;
    frame.payload.data = buf;
    frame.payload.len  = isiPackStreamName(frame.payload.data, name);

    return iacpSendFrame(iacp, &frame);
}

BOOL isiIacpSendDataRequest(IACP *iacp, ISI_DATA_REQUEST *datreq)
{
UINT32 i;
LNKLST_NODE *crnt;
static char *fid = "isiIacpSendDataRequest";

    if (!iacpSendUINT32(iacp, ISI_IACP_REQ_FORMAT, datreq->format)) return FALSE;
    if (!iacpSendUINT32(iacp, ISI_IACP_REQ_COMPRESS, datreq->compress)) return FALSE;
    if (datreq->options != ISI_DEFAULT_OPTIONS && !iacpSendUINT32(iacp, ISI_IACP_REQ_OPTIONS, datreq->options)) return FALSE;
    if (!iacpSendUINT32(iacp, ISI_IACP_REQ_POLICY, datreq->policy)) return FALSE;
    if (datreq->type == ISI_REQUEST_TYPE_TWIND) {
        for (i = 0; i < datreq->nreq; i++) {
            if (!SendTwindRequest(iacp, &datreq->req.twind[i])) return FALSE;
        }
    } else if (datreq->type == ISI_REQUEST_TYPE_SEQNO) {
        for (i = 0; i < datreq->nreq; i++) {
            if (!SendSeqnoRequest(iacp, &datreq->req.seqno[i])) return FALSE;
        }
        crnt = listFirstNode(&datreq->slist);
        while (crnt != NULL) {
            if (!SendStreamName(iacp, (ISI_STREAM_NAME *) crnt->payload)) return FALSE;
            crnt = listNextNode(crnt);
        }
    }
    isiLogDatreq(iacp->lp, LOG_DEBUG, datreq);
    return iacpSendUINT32(iacp, 0, IACP_TYPE_NULL);
}

/* Append a time window request to the incoming request packet */

UINT32 isiAppendTwindReq(ISI_INCOMING *incoming, IACP_FRAME *frame)
{
ISI_TWIND_REQUEST req;

    if (incoming == NULL || frame == NULL) {
        errno = EINVAL;
        return IACP_ALERT_OTHER_ERROR;
    }

/* Make sure peer isn't mixing request types */

    if (incoming->list.count == 0) {
        incoming->type = frame->payload.type;
    } else if (incoming->type != frame->payload.type) {
        return IACP_ALERT_PROTOCOL_ERROR;
    }

/* Decode the message and append it to the list */

    isiUnpackTwindRequest(frame->payload.data, &req);
    if (!listAppend(&incoming->list, &req, sizeof(ISI_TWIND_REQUEST))) return IACP_ALERT_SERVER_FAULT;

    return IACP_ALERT_NONE;
}

/* Append a seqno request to the incoming request packet */

UINT32 isiAppendSeqnoReq(ISI_INCOMING *incoming, IACP_FRAME *frame)
{
ISI_SEQNO_REQUEST req;

    if (incoming == NULL || frame == NULL) {
        errno = EINVAL;
        return IACP_ALERT_OTHER_ERROR;
    }

/* Make sure peer isn't mixing request types */

    if (incoming->list.count == 0) {
        incoming->type = frame->payload.type;
    } else if (incoming->type != frame->payload.type) {
        return IACP_ALERT_PROTOCOL_ERROR;
    }

/* Decode the message and append it to the list */

    isiUnpackSeqnoRequest(frame->payload.data, &req);
    if (!listAppend(&incoming->list, &req, sizeof(ISI_SEQNO_REQUEST))) return IACP_ALERT_SERVER_FAULT;

    return IACP_ALERT_NONE;
}

/* Append a stream name to an incoming seqno request packet */

UINT32 isiAppendSeqnoSlist(ISI_DATA_REQUEST *datreq, IACP_FRAME *frame)
{
ISI_STREAM_NAME name;
static char *fid = "isiAppendSeqnoSlist";

    if (datreq == NULL || frame == NULL) {
        errno = EINVAL;
        return IACP_ALERT_OTHER_ERROR;
    }

/* Decode the message and append it to the list */

    isiUnpackStreamName(frame->payload.data, &name);
    if (!listAppend(&datreq->slist, &name, sizeof(ISI_STREAM_NAME))) return IACP_ALERT_SERVER_FAULT;

    return IACP_ALERT_NONE;
}

/* Copy a list of time window requests into a data request array */

BOOL isiCopyTwindListToDataRequest(LNKLST *list, ISI_DATA_REQUEST *output)
{
UINT32 i;
ISI_TWIND_REQUEST *entry;
static char *fid = "isiCopyTwindListToDataRequest";

    if (!listSetArrayView(list)) return FALSE;
    if ((output->nreq = list->count) == 0) return TRUE;

    output->req.twind = (ISI_TWIND_REQUEST *) malloc(list->count * sizeof(ISI_TWIND_REQUEST));
    if (output->req.twind == NULL) return FALSE;

    for (i = 0; i < output->nreq; i++) {
        entry = (ISI_TWIND_REQUEST *) list->array[i];
        memcpy(&output->req.twind[i].name, &entry->name, sizeof(ISI_STREAM_NAME));
        output->req.twind[i].beg = entry->beg;
        output->req.twind[i].end = entry->end;
    }
    return TRUE;
}

/* Copy a list of seqno requests into a data request array */

BOOL isiCopySeqnoListToDataRequest(LNKLST *list, ISI_DATA_REQUEST *output)
{
UINT32 i;
ISI_SEQNO_REQUEST *entry;

    if (!listSetArrayView(list)) return FALSE;
    if ((output->nreq = list->count) == 0) return TRUE;

    output->req.seqno = (ISI_SEQNO_REQUEST *) malloc(list->count * sizeof(ISI_SEQNO_REQUEST));
    if (output->req.seqno == NULL) return FALSE;

    for (i = 0; i < output->nreq; i++) {
        entry = (ISI_SEQNO_REQUEST *) list->array[i];
        memcpy(&output->req.seqno[i].site, &entry->site, sizeof(ISI_SEQNO_REQUEST));
        output->req.seqno[i].beg = entry->beg;
        output->req.seqno[i].end = entry->end;
    }
    return TRUE;
}

/* Client level data request (with acknowledgement) */

static BOOL ReceiveDataRequestAcknowledgment(ISI *isi, UINT32 *error)
{
UINT32 i;
BOOL finished;
static char *fid = "isiSendDataRequest:ReceiveDataRequestAcknowledgment";

    isiResetIncoming(&isi->incoming);
    isiInitDataRequest(&isi->datreq);

/* Read response into linked list */

    finished = FALSE;
    while (!finished) {
        if (iacpRecvFrame(isi->iacp, &isi->frame, isi->buf, ISI_INTERNAL_BUFLEN)) {
            switch (isi->frame.payload.type) {
              case ISI_IACP_REQ_FORMAT:
                utilUnpackUINT32(isi->frame.payload.data, &isi->datreq.format);
                break;
              case ISI_IACP_REQ_COMPRESS:
                utilUnpackUINT32(isi->frame.payload.data, &isi->datreq.compress);
                break;
              case ISI_IACP_REQ_OPTIONS:
                utilUnpackUINT32(isi->frame.payload.data, &isi->datreq.options);
                break;
              case ISI_IACP_REQ_POLICY:
                utilUnpackUINT32(isi->frame.payload.data, &isi->datreq.policy);
                break;
              case ISI_IACP_REQ_TWIND:
                *error = isiAppendTwindReq(&isi->incoming, &isi->frame);
                if (*error != IACP_ALERT_NONE) {
                    isiLogMsg(isi, LOG_INFO, "%s: isiAppendTwindReq: %s", fid, strerror(errno));
                    return FALSE;
                }
                isi->datreq.type = ISI_REQUEST_TYPE_TWIND;
                break;
              case ISI_IACP_REQ_SEQNO:
                *error = isiAppendSeqnoReq(&isi->incoming, &isi->frame);
                if (*error != IACP_ALERT_NONE) {
                    isiLogMsg(isi, LOG_INFO, "%s: isiAppendSeqnoReq: %s", fid, strerror(errno));
                    return FALSE;
                }
                isi->datreq.type = ISI_REQUEST_TYPE_SEQNO;
                break;
              case ISI_IACP_REQ_STREAM:
                *error = isiAppendSeqnoSlist(&isi->datreq, &isi->frame);
                if (*error != IACP_ALERT_NONE) {
                    isiLogMsg(isi, LOG_INFO, "%s: isiAppendSeqnoSlist: %s", fid, strerror(errno));
                    return FALSE;
                }
                break;
              case IACP_TYPE_NULL:
                finished = TRUE;
                break;
              case IACP_TYPE_ALERT:
                finished = TRUE;
                *error = iacpAlertCauseCode(&isi->frame);
                break;
              case IACP_TYPE_NOP:
                break;
              default:
                isiLogMsg(isi, LOG_INFO, "%s: unexpected type %lu frame received", fid, isi->frame.payload.type);
                *error = IACP_ALERT_PROTOCOL_ERROR;
                return FALSE;
            }
        } else {
            isiLogMsg(isi, LOG_INFO, "%s: iacpRecvFrame: %s", fid, strerror(errno));
            *error = IACP_ALERT_IO_ERROR;
            return FALSE;
        }
    }

/* Copy from linked list to internal data request buffer */

    if (isi->datreq.type == ISI_REQUEST_TYPE_TWIND) {
        if (!isiCopyTwindListToDataRequest(&isi->incoming.list, &isi->datreq)) {
            isiLogMsg(isi, LOG_INFO, "%s: isiCopyTwindListToDataRequest: %s", fid, strerror(errno));
            *error = IACP_ALERT_OTHER_ERROR;
            return FALSE;
        }
        for (i = 0; i < isi->datreq.nreq; i++) isi->datreq.req.twind[i].status = ISI_INCOMPLETE;
    } else if (isi->datreq.type == ISI_REQUEST_TYPE_SEQNO) {
        if (!isiCopySeqnoListToDataRequest(&isi->incoming.list, &isi->datreq)) {
            isiLogMsg(isi, LOG_INFO, "%s: isiCopySeqnoListToDataRequest: %s", fid, strerror(errno));
            *error = IACP_ALERT_OTHER_ERROR;
            return FALSE;
        }
        for (i = 0; i < isi->datreq.nreq; i++) isi->datreq.req.seqno[i].status = ISI_INCOMPLETE;
    }

    return TRUE;
}

BOOL isiSendDataRequest(ISI *isi, ISI_DATA_REQUEST *datreq, UINT32 *error)
{
static char *fid = "isiSendDataRequest";

    if (error == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (isi == NULL || datreq == NULL) {
        *error = IACP_ALERT_OTHER_ERROR;
        errno = EINVAL;
        return FALSE;
    }

    if (!isiIacpSendDataRequest(isi->iacp, datreq)) {
        isiLogMsg(isi, LOG_INFO, "%s: isiIacpSendDataRequest failed", fid);
        *error = IACP_ALERT_IO_ERROR;
        return FALSE;
    }

    if (!ReceiveDataRequestAcknowledgment(isi, error)) {
        isiLogMsg(isi, LOG_INFO, "%s: ReceiveDataRequestAcknowledgment failed", fid);
        return FALSE;
    }

    return TRUE;
}

ISI_DATA_REQUEST *isiFreeDataRequest(ISI_DATA_REQUEST *datreq)
{
static char *fid = "isiFreeDataRequest";

    if (datreq == NULL) return NULL;
    isiClearDataRequest(datreq);
    free(datreq);
    return NULL;
}

static ISI_DATA_REQUEST *AllocSimpleTwindRequest(char *StreamSpec)
{
UINT32 i;
LNKLST *list;
ISI_DATA_REQUEST *datreq;
static char *fid = "AllocSimpleTwindRequest";

    datreq = (ISI_DATA_REQUEST *) calloc(1, sizeof(ISI_DATA_REQUEST));
    if (datreq == NULL) return NULL;

    isiInitDataRequest(datreq);
    datreq->type = ISI_REQUEST_TYPE_TWIND;
    if ((list = isiExpandStreamNameSpecifier(StreamSpec)) == NULL) return NULL;

    datreq->nreq = list->count;
    datreq->req.twind = (ISI_TWIND_REQUEST *) malloc(datreq->nreq * sizeof(ISI_TWIND_REQUEST));
    if (datreq->req.twind == NULL) return isiFreeDataRequest(datreq);

    for (i = 0; i < list->count; i++) {
        datreq->req.twind[i].name = *((ISI_STREAM_NAME *) list->array[i]);
        datreq->req.twind[i].beg  = ISI_UNDEFINED_TIMESTAMP;
        datreq->req.twind[i].end  = ISI_UNDEFINED_TIMESTAMP;
    }

    listDestroy(list);
    return datreq;
}

ISI_DATA_REQUEST *isiAllocSimpleDataRequest(REAL64 beg, REAL64 end, char *StreamSpec)
{
UINT32 i;
ISI_DATA_REQUEST *datreq;
static char *fid = "isiAllocSimpleDataRequest";

    if ((datreq = AllocSimpleTwindRequest(StreamSpec)) == NULL) return NULL;

    for (i = 0; i < datreq->nreq; i++) {
        datreq->req.twind[i].beg  = beg;
        datreq->req.twind[i].end  = end;
    }

    return datreq;
}

static ISI_DATA_REQUEST *AllocSimpleSeqnoRequest(char *SiteSpec)
{
UINT32 i;
LNKLST *list;
ISI_DATA_REQUEST *datreq;
static char *fid = "AllocSimpleSeqnoRequest";

    datreq = (ISI_DATA_REQUEST *) calloc(1, sizeof(ISI_DATA_REQUEST));
    if (datreq == NULL) return NULL;

    isiInitDataRequest(datreq);
    datreq->type = ISI_REQUEST_TYPE_SEQNO;
    datreq->format = ISI_FORMAT_NATIVE;
    if ((list = isiExpandSeqnoSiteSpecifier(SiteSpec)) == NULL) return NULL;

    datreq->nreq = list->count;
    datreq->req.seqno = (ISI_SEQNO_REQUEST *) malloc(datreq->nreq * sizeof(ISI_SEQNO_REQUEST));
    if (datreq->req.seqno == NULL) return isiFreeDataRequest(datreq);

    for (i = 0; i < list->count; i++) {
        strlcpy(datreq->req.seqno[i].site, (char *) list->array[i], ISI_SITELEN+1);
        datreq->req.seqno[i].beg  = ISI_UNDEFINED_SEQNO;
        datreq->req.seqno[i].end  = ISI_UNDEFINED_SEQNO;
    }

    listDestroy(list);
    return datreq;
}

ISI_DATA_REQUEST *isiAllocSimpleSeqnoRequest(ISI_SEQNO *beg, ISI_SEQNO *end, char *SiteSpec)
{
UINT32 i;
ISI_DATA_REQUEST *datreq;
static char *fid = "isiAllocSimpleSeqnoRequest";

    if ((datreq = AllocSimpleSeqnoRequest(SiteSpec)) == NULL) return NULL;

    for (i = 0; i < datreq->nreq; i++) {
        if (beg != NULL) {
            datreq->req.seqno[i].beg = *beg;
        } else {
            datreq->req.seqno[i].beg = ISI_DEFAULT_BEG_SEQNO;
        }
        if (end != NULL) {
            datreq->req.seqno[i].end = *end;
        } else {
            datreq->req.seqno[i].end = ISI_DEFAULT_END_SEQNO;
        }
    }

    return datreq;
}

ISI_DATA_REQUEST *isiAllocDefaultDataRequest(VOID)
{
    return isiAllocSimpleDataRequest(ISI_DEFAULT_BEGTIME, ISI_DEFAULT_ENDTIME, ISI_DEFAULT_STREAMSPEC);
}

ISI_DATA_REQUEST *isiAllocDefaultSeqnoRequest(VOID)
{
    return isiAllocSimpleSeqnoRequest(&ISI_DEFAULT_BEG_SEQNO, &ISI_DEFAULT_END_SEQNO, ISI_DEFAULT_SITESPEC);
}

/* Revision History
 *
 * $Log: datreq.c,v $
 * Revision 1.28  2008/01/25 23:10:01  dechavez
 * don't send data request options if they are the default (this avoids breaking peers that don't know about this field)
 *
 * Revision 1.27  2008/01/25 21:51:51  dechavez
 * added support for ISI_IACP_REQ_STREAM and ISI_IACP_REQ_OPTIONS, new functions
 * isiGetDatreqOptions(), isiSetDatreqOptions(), isiAppendSeqnoSlist()
 *
 * Revision 1.26  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.25  2007/01/11 17:50:35  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.24  2007/01/08 15:56:35  dechavez
 * fixed "free same pointer twice" bug
 *
 * Revision 1.23  2006/06/26 22:36:29  dechavez
 * removed unreferenced local variables
 *
 * Revision 1.22  2006/06/21 18:35:30  dechavez
 * fixed multi-free bug in isiInitDataRequest (stimulated when called by isiClearDataRequest)
 *
 * Revision 1.21  2006/06/12 21:23:57  dechavez
 * calloc() datreq in AllocSimpleXRequest(), so that isiInitDataRequest()
 * can detect and free any stream/seqno pointers in the req field.
 *
 * Revision 1.20  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.19  2005/10/26 23:16:11  dechavez
 * added isiSetDatreqType()
 *
 * Revision 1.18  2005/10/10 23:44:11  dechavez
 * fixed potential infinite loop in ReceiveDataRequestAcknowledgment
 *
 * Revision 1.17  2005/06/30 01:33:15  dechavez
 * checkpoint, debugged seqno requests
 *
 * Revision 1.16  2005/06/24 21:41:43  dechavez
 * accomodate new design of ISI_DATA_REQUEST structure
 *
 * Revision 1.15  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.14  2005/05/06 00:53:21  dechavez
 * support for sequence number based requests added
 *
 * Revision 1.13  2004/09/28 22:55:44  dechavez
 * added conditional DEBUG support
 *
 * Revision 1.12  2004/06/21 19:47:43  dechavez
 * print name components in quotes in isiPrintStreamReq() (helps debugging)
 *
 * Revision 1.11  2004/06/10 17:35:34  dechavez
 * allow for sta.chn strings in isiBuildStreamName()
 *
 * Revision 1.10  2004/06/04 22:49:43  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.9  2004/01/29 18:40:25  dechavez
 * init SendStreamRequest() buf with 0xee's (debug aid)
 *
 * Revision 1.8  2003/12/11 22:19:23  dechavez
 * changed isiFreeDataRequest() to free streams via isiClearDataRequest()
 * instead of its own explicit free()
 *
 * Revision 1.7  2003/12/10 05:50:34  dechavez
 * added IACP_TYPE_NOP support to isiSendDataRequest()
 *
 * Revision 1.6  2003/12/04 23:31:57  dechavez
 * added IACP_TYPE_ALERT support to ReceiveDataRequestAcknowledgment
 *
 * Revision 1.5  2003/11/25 20:36:35  dechavez
 * added isiGetDatreqCompress()
 *
 * Revision 1.4  2003/11/21 20:03:23  dechavez
 * fixed typo in testing for isiCopyStreamListToDataRequest malloc failure,
 * removed some left over debugging printfs
 *
 * Revision 1.3  2003/11/19 23:50:46  dechavez
 * added return value for EINVAL isiAppendStreamReq call
 *
 * Revision 1.2  2003/11/03 23:53:45  dechavez
 * added isiSetDatreqPolicy(), isiSetDatreqFormat(), isiSetDatreqCompress(),
 * isiSendDataRequest(), isiClearDataRequest(), isiLocateStreamRequest(),
 * isiFreeDataRequest(), isiAllocSimpleDataRequest(), isiAllocDefaultDataRequest()
 * Data request's are now acknowledged with server expanded requests
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
