#pragma ident "$Id: ts.c,v 1.21 2007/01/23 02:50:43 dechavez Exp $"
/*======================================================================
 *
 * Read the next ISI_GENERIC_TS frame, ignoring all other frames and
 * taking care of reconnects and updated request parameters in case
 * of errors.
 *
 * isiReadGenericTS will return either a pointer to the next frame,
 * or NULL with status set to one of the following
 *
 * ISI_DONE  - when all requested data have been provided
 * ISI_ERROR - impossible to sustain the connection
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

VOID isiInitGenericTSHDR(ISI_GENERIC_TSHDR *src)
{
    memset(src, 0, sizeof(ISI_GENERIC_TSHDR));
}

BOOL isiInitGenericTS(ISI_GENERIC_TS *ts)
{
    if (ts == NULL) {
        errno = EINVAL;
        return FALSE;
    }
    
    ts->hdr.nbytes = 0;
    ts->data = NULL;
    ts->precious = FALSE;
    ts->nalloc = 0;

    return TRUE;
}

ISI_GENERIC_TS *isiCreateGenericTS(VOID)
{
ISI_GENERIC_TS *ts;

    if ((ts = (ISI_GENERIC_TS *) malloc(sizeof(ISI_GENERIC_TS))) == NULL) return NULL;

    if (!isiInitGenericTS(ts)) {
        free(ts);
        return NULL;
    }

    return ts;
}

ISI_GENERIC_TS *isiDestroyGenericTS(ISI_GENERIC_TS *ts)
{
    if (ts == NULL) return NULL;
    if (ts->precious) return ts;

    if (ts->nalloc > 0 && ts->data != NULL) {
        free(ts->data);
        ts->data = NULL;
        ts->nalloc = 0;
    }

    free(ts);

    return NULL;
}

static BOOL UpdateDataRequest(ISI_DATA_REQUEST *old, ISI_DATA_REQUEST *new)
{
UINT32 i;

    isiInitDataRequest(new);

    new->type = old->type;
    new->policy = old->policy;
    new->format = old->format;
    new->compress = old->compress;

    for (new->nreq = 0, i = 0; i < old->nreq; i++) if (old->req.twind[i].status == ISI_INCOMPLETE) ++new->nreq;
    new->req.twind = (ISI_TWIND_REQUEST *) malloc(new->nreq * sizeof(ISI_TWIND_REQUEST));
    if (new->req.twind == NULL) return FALSE;

    for (new->nreq = 0, i = 0; i < old->nreq; i++) {
        if (old->req.twind[i].status == ISI_INCOMPLETE) {
            memcpy(&new->req.twind[new->nreq].name, &old->req.twind[i].name, sizeof(ISI_STREAM_NAME));
            new->req.twind[new->nreq].beg = old->req.twind[i].beg;
            new->req.twind[new->nreq].end = old->req.twind[i].end;
            new->req.twind[new->nreq].status = ISI_INCOMPLETE;
            ++new->nreq;
        }
    }
    
    return TRUE;
}

static VOID UpdateRequestStartTime(ISI *isi, ISI_GENERIC_TSHDR *hdr)
{
ISI_TWIND_REQUEST *twreq;

    if ((twreq = isiLocateTwindRequest(&hdr->name, &isi->datreq)) == NULL) return;

    if (isi->datreq.policy == ISI_RECONNECT_POLICY_MIN_DELAY) {
        twreq->beg = ISI_NEWEST;
    } else {
        twreq->beg = hdr->tols.value + isiSrateToSint(&hdr->srate);
    }

    if (twreq->end >= hdr->tofs.value) twreq->status = ISI_COMPLETE;
}

static BOOL ClearDataRequest(ISI_DATA_REQUEST *datreq, BOOL retval)
{
    isiClearDataRequest(datreq);
    return retval;
}

static BOOL Reconnect(ISI *isi)
{
UINT32 error;
ISI_DATA_REQUEST datreq;
static char *fid = "isiReadGenericTS:ReadNextFrame:Reconnect";

    if (!UpdateDataRequest(&isi->datreq, &datreq)) return ClearDataRequest(&datreq, FALSE);

    while (isi->iacp == NULL) {
        isiLogMsg(isi, LOG_DEBUG, "contacting %s:%d", isi->server, isi->port);
        isi->iacp = iacpOpen(isi->server, isi->port, &isi->attr, isi->lp, isi->debug);
        if (isi->iacp == NULL) return ClearDataRequest(&datreq, FALSE);
        isiLogMsg(isi, LOG_INFO, "reconnected to %s", isi->iacp->peer.ident);
        if (!isiSendDataRequest(isi, &datreq, &error)) {
            isiLogMsg(isi, LOG_INFO, "error sending request to %s: %s", isi->iacp->peer.ident, strerror(errno));
            isi->iacp = iacpClose(isi->iacp);
            utilDelayMsec(ISI_SHUTDOWN_DELAY);
        }
    }
    
    return ClearDataRequest(&datreq, TRUE);
}

static int ReadNextFrame(ISI *isi)
{
static char *fid = "isiReadGenericTS:ReadNextFrame";

    while (1) {

        if (isi->iacp == NULL && !Reconnect(isi)) return ISI_ERROR;
        if (iacpRecvFrame(isi->iacp, &isi->frame, isi->buf, ISI_INTERNAL_BUFLEN)) return ISI_OK;

        isiLogMsg(isi, LOG_INFO, "%s: %s: %s", isi->iacp->peer.ident, fid, strerror(errno));
        isi->iacp = iacpClose(isi->iacp);
        utilDelayMsec(ISI_SHUTDOWN_DELAY);
    }
}

static BOOL CheckAllocation(ISI_GENERIC_TS *ts, UINT32 amount)
{
static char *fid = "CheckAllocation";

    if (ts == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (ts->nalloc >= amount) {
        return TRUE;
    }

    ts->data = realloc(ts->data, amount);
    if (ts->data == NULL) {
        return FALSE;
    }
    ts->nalloc = amount;

    return TRUE;
}

/*
BOOL isiCopyGenericTS(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{
    if (dest == NULL || src == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    dest->hdr = src->hdr;
    if (!CheckAllocation(dest, src->hdr.nbytes)) return FALSE;
    memcpy(dest->data, src->data, src->hdr.nbytes);

    return TRUE;
}
*/

static BOOL DecompressINT32(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{
UINT32 newlen;

    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return FALSE;
    }

/* We can only deal with IDA compression */

    if (src->hdr.desc.comp != ISI_COMP_IDA) {
        errno = ENOTSUP;
        return FALSE;
    }

    newlen = src->hdr.nsamp * sizeof(INT32);
    if (!CheckAllocation(dest, newlen)) return FALSE;
    utilIdaExpandINT32((INT32 *) dest->data, (UINT8 *) src->data, src->hdr.nsamp);
    dest->hdr = src->hdr;
    dest->hdr.nbytes = newlen;
    dest->hdr.desc.comp = ISI_COMP_NONE;
    dest->hdr.desc.order = ISI_HOST_BYTE_ORDER;
    return TRUE;
}

static BOOL DecompressIdaINT32(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest, UINT32 newlen, UINT32 hdrlen)
{
UINT8 *input;
INT32 *output;

    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (!CheckAllocation(dest, newlen)) return FALSE;
    memcpy(dest->data, src->data, hdrlen);
    input = ((UINT8 *) src->data) + hdrlen;
    output = (INT32 *) (((UINT8 *) dest->data) + hdrlen);
    utilIdaExpandINT32(output, input, src->hdr.nsamp);

/* The data decompress into host byte order.  We need to make sure that they
 * actually end up in digitizer byte order.
 */

    if (ISI_HOST_BYTE_ORDER == ISI_ORDER_LTLENDIAN) utilSwapINT32(output, src->hdr.nsamp);

    dest->hdr = src->hdr;
    dest->hdr.nbytes = newlen;
    dest->hdr.desc.comp = ISI_COMP_NONE;
    return TRUE;
}

static BOOL DecompressIDA5678(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{
UINT32 newlen;

#define IDA5678_HEADER_LENGTH 60
#define IDA5678_TRAILER_PADDING 4

    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return FALSE;
    }

/* We can only deal with IDA compression */

    if (src->hdr.desc.comp != ISI_COMP_IDA) {
        errno = ENOTSUP;
        return FALSE;
    }

/* We "add" the trailer padding so that the packet length works out to 1024 bytes,
 * which is how the media based packets are sized.
 */

    newlen = IDA5678_HEADER_LENGTH + (src->hdr.nsamp * sizeof(INT32)) + IDA5678_TRAILER_PADDING;
    return DecompressIdaINT32(src, dest, newlen, IDA5678_HEADER_LENGTH);
}

static BOOL DecompressIDA9(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{
UINT32 newlen;

#define IDA9_HEADER_LENGTH 64

    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return FALSE;
    }

/* We can only deal with IDA compression */

    if (src->hdr.desc.comp != ISI_COMP_IDA) {
        errno = ENOTSUP;
        return FALSE;
    }

    newlen = IDA9_HEADER_LENGTH + (src->hdr.nsamp * sizeof(INT32));
    return DecompressIdaINT32(src, dest, newlen, IDA9_HEADER_LENGTH);
}

static BOOL DecompressIDA10(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{
UINT8 format;
UINT32 newlen;

/* We include this meta-knowledge here in order to avoid having to ship the ida10
 * library with the toolkit
 */

#define IDA10_HEADER_LENGTH 64
#define IDA10_FORMAT_BYTE_OFFSET 56
#define IDA10_COMP_NONE  0
#define IDA10_TYPE_INT32 0
#define IDA10_MASK_COMP  0x03 /* - - - - - - 1 1 */
#define IDA10_MASK_TYPE  0x30 /* - - 1 1 - - - - */

    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return FALSE;
    }

/* We can only deal with IDA compression */

    if (src->hdr.desc.comp != ISI_COMP_IDA) {
        errno = ENOTSUP;
        return FALSE;
    }

/* We can only handle IDA10 packets which are uncompressed INT32's.  It's confusing
 * because what we are doing here is verifying that the packet was not "born" compressed
 * (hence  the test for IDA10_COMP_NONE).  The compression that we are removing here
 * is the ISI compression applied by the data server.  That server would not have
 * set the compressed bit in our ISI_GENERIC_TS header were the original data anything
 * other than IDA10_COMP_NONE plus IDA10_TYPE_INT32, so it is not anticipated that
 * these tests will ever fail.
 *
 */

    format = ((UINT8 *) src->data)[IDA10_FORMAT_BYTE_OFFSET];

    if ((format & IDA10_MASK_COMP) != IDA10_COMP_NONE) {
        errno = ENOTSUP;
        return FALSE;
    }
    if ((format & IDA10_MASK_TYPE) != IDA10_TYPE_INT32) {
        errno = ENOTSUP;
        return FALSE;
    }

    newlen = IDA10_HEADER_LENGTH + (src->hdr.nsamp * sizeof(INT32));
    return DecompressIdaINT32(src, dest, newlen, IDA10_HEADER_LENGTH);
}

ISI_GENERIC_TS *isiDecompressGenericTS(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest)
{

    if (dest == NULL || src == NULL) {
        errno = EINVAL;
        return NULL;
    }

/* Deal with uncompressed data (either leave alone or byte swap) */

    if (src->hdr.desc.comp == ISI_COMP_NONE) {
        if (src->hdr.desc.order == ISI_HOST_BYTE_ORDER) return src;
        switch (src->hdr.desc.type) {
          case ISI_TYPE_INT8:
            src->hdr.desc.order = ISI_HOST_BYTE_ORDER;
          case ISI_TYPE_IDA5:
          case ISI_TYPE_IDA6:
          case ISI_TYPE_IDA7:
          case ISI_TYPE_IDA8:
          case ISI_TYPE_IDA9:
          case ISI_TYPE_IDA10:
          case ISI_TYPE_QDPLUS:
          case ISI_TYPE_MSEED:
            return src;
        }
        switch (src->hdr.desc.type) {
          case ISI_TYPE_INT16:
            utilSwapINT16((UINT16 *) src->data, src->hdr.nsamp);
            break;
          case ISI_TYPE_INT32:
            utilSwapINT32((UINT32 *) src->data, src->hdr.nsamp);
            break;
          case ISI_TYPE_INT64:
            utilSwapINT64((UINT64 *) src->data, src->hdr.nsamp);
            break;
          case ISI_TYPE_REAL32:
          case ISI_TYPE_REAL64:
            break;
          default:
            errno = EINVAL;
            return NULL;
        }
        src->hdr.desc.order = ISI_HOST_BYTE_ORDER;
        return src;
    }

/* Otherwise, decompress */

    switch (src->hdr.desc.type) {

      case ISI_TYPE_INT32:
        return DecompressINT32(src, dest) ? dest : NULL;

      case ISI_TYPE_IDA5:
      case ISI_TYPE_IDA6:
      case ISI_TYPE_IDA7:
      case ISI_TYPE_IDA8:
        return DecompressIDA5678(src, dest) ? dest : NULL;

      case ISI_TYPE_IDA9:
        return DecompressIDA9(src, dest) ? dest : NULL;

      case ISI_TYPE_IDA10:
        return DecompressIDA10(src, dest) ? dest : NULL;

      default:
        errno = ENOTSUP;
    }
    return NULL;
}

ISI_GENERIC_TS *isiReadGenericTS(ISI *isi, int *status)
{
UINT32 cause;
ISI_GENERIC_TS *DecompressedFrame;
#ifdef DEBUG
UINT32 ulval;
ISI_TWIND_REQUEST twreq;
#endif /* DEBUG */
static char *fid = "isiReadGenericTS";

    if (status == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (isi == NULL) {
        errno = EINVAL;
        *status = ISI_ERROR;
        return NULL;
    }

    while (1) {

        if (isiGetFlag(isi) == ISI_FLAG_BREAK) {
            errno = 0;
            *status = ISI_BREAK;
            return NULL;
        }

        if ((*status = ReadNextFrame(isi)) != ISI_OK) return NULL;

        switch (isi->frame.payload.type) {
          case ISI_IACP_GENERIC_TS:
            isiUnpackGenericTS(isi->frame.payload.data, isi->ts1);
            UpdateRequestStartTime(isi, &isi->ts1->hdr);
            if (isi->decompress) {
                DecompressedFrame = isiDecompressGenericTS(isi->ts1, isi->ts2);
                *status = (DecompressedFrame == NULL) ? ISI_ERROR : ISI_OK;
                return DecompressedFrame;
            } else {
                *status = ISI_OK;
                return isi->ts1;
            }
          case IACP_TYPE_ALERT:
            cause = iacpAlertCauseCode(&isi->frame);
            isiLogMsg(isi, LOG_DEBUG, "%s: %s: %s", fid, isi->iacp->peer.ident, iacpAlertString(cause));
            isi->iacp = iacpClose(isi->iacp);
            if (cause == IACP_ALERT_REQUEST_COMPLETE) {
                *status = ISI_DONE;
                return NULL;
            }
            utilDelayMsec(ISI_SHUTDOWN_DELAY);
            break;
          case IACP_TYPE_NOP:
            isiLogMsg(isi, LOG_DEBUG, "%s: %s: heartbeat", fid, isi->iacp->peer.ident);
            break;
#ifdef DEBUG
          case ISI_IACP_REQ_FORMAT:
            utilUnpackUINT32(isi->frame.payload.data, &ulval);
            printf("%s: %s: format = %lu", fid, isi->iacp->peer.ident, ulval);
            break;
          case ISI_IACP_REQ_COMPRESS:
            utilUnpackUINT32(isi->frame.payload.data, &ulval);
            printf("%s: %s: compress = %lu", fid, isi->iacp->peer.ident, ulval);
            break;
          case ISI_IACP_REQ_POLICY:
            utilUnpackUINT32(isi->frame.payload.data, &ulval);
            printf("%s: %s: policy = %lu", fid, isi->iacp->peer.ident, ulval);
            break;
          case ISI_IACP_REQ_STREAM:
            isiUnpackTwindRequest(isi->frame.payload.data, &twreq);
            isiPrintTwindReq(stdout, &twreq);
            break;
#endif /* DEBUG */
          default:
            isiLogMsg(isi, LOG_DEBUG, "%s: %s: frame type %lu ignored", fid, isi->iacp->peer.ident, isi->frame.payload.type);
        }
    }
}

/* Revision History
 *
 * $Log: ts.c,v $
 * Revision 1.21  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.20  2007/01/11 17:50:35  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.19  2006/06/19 19:06:55  dechavez
 * support ISI_TYPE_IDA[567]
 *
 * Revision 1.18  2006/06/12 21:29:09  dechavez
 * fixed unitialized new->type on reconnect in UpdateDataRequest (aap)
 *
 * Revision 1.17  2005/08/26 18:38:22  dechavez
 * added ISI_TYPE_IDA9 support
 *
 * Revision 1.16  2005/06/24 21:31:52  dechavez
 * accomodate new design of ISI_DATA_REQUEST structure
 *
 * Revision 1.15  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.14  2005/05/06 01:00:48  dechavez
 * added isiInitGenericTSHDR(), use the new concise field names in ISI_GENERIC_TS
 *
 * Revision 1.13  2004/09/28 22:55:44  dechavez
 * added conditional DEBUG support
 *
 * Revision 1.12  2004/06/10 17:16:46  dechavez
 * check (new) ISI handle flag and return with status ISI_BREAK if set
 *
 * Revision 1.11  2004/01/29 18:35:01  dechavez
 * use calloc instead of malloc, tune logging messages
 *
 * Revision 1.10  2003/12/11 22:23:39  dechavez
 * removed a memory leak in Reconnect() by freeing the streams allocated
 * in UpdateDataRequest(), cleaned up isiDecompressGenericTS() decompress
 * switch, calling type specific functions.
 *
 * Revision 1.9  2003/12/10 05:56:32  dechavez
 * byte swap decompressed native digitizer packet data from host into original
 * byte order, if needed
 *
 * Revision 1.8  2003/11/26 21:22:53  dechavez
 * fixed error in setting byte order in isiDecompressGenericTS()
 *
 * Revision 1.7  2003/11/26 19:39:17  dechavez
 * made error message less verbose
 *
 * Revision 1.6  2003/11/25 20:35:32  dechavez
 * added delays after each socket close, not just graceful closures
 *
 * Revision 1.5  2003/11/21 20:01:32  dechavez
 * removed inline declarations, fixed byte order for decompressed INT32's on
 * little endian systems
 *
 * Revision 1.4  2003/11/19 23:47:12  dechavez
 * added casts for compiler and removed void pointer arithmetic to determine
 * offsets for decompressing native data
 *
 * Revision 1.3  2003/11/19 21:29:43  dechavez
 * added isiCreateGenericTS(), isiDestroyGenericTS(), isiInitGenericTS(),
 * isiDecompressGenericTS(), isiCopyGenericTS(), and changed isiReadGenericTS()
 * to use ISI_GENERIC_TS in the handle for producing (possibly) decompressed and
 * native ordered packets
 *
 * Revision 1.2  2003/11/04 19:57:58  dechavez
 * delay ISI_SHUTDOWN_DELAY msecs following socket close and subsequent reconnect
 * attempt, fixed error in setting value of updated request start time
 *
 * Revision 1.1  2003/11/03 23:52:29  dechavez
 * Initial release
 *
 */
