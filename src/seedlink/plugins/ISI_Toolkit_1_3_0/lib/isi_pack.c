#pragma ident "$Id: pack.c,v 1.14 2008/01/25 21:49:52 dechavez Exp $"
/*======================================================================
 *
 *  Pack/Unpack ISI data objects into/out of NBO
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

/* Get the next sub-field of something which has sub-fields */

static UINT8 *GetNextField(UINT8 *start, UINT32 *type, UINT8 **data)
{
UINT8 *ptr;
UINT32 length;

    ptr = start;

/* Get the type (0 means end of input) */

    ptr += utilUnpackINT32(ptr, type);
    if (*type == ISI_TAG_EOF) return NULL;

/* Get data length and set pointer to the data */

    ptr += utilUnpackINT32(ptr, &length);
    *data = ptr;

/* Point to next field */

    return ptr + length;
}

/* Set the end-of-subfields flag */

static int TerminateInput(UINT8 *start)
{
UINT8 *ptr;

    ptr = start;
    ptr += utilPackINT32(ptr, ISI_TAG_EOF);
    return (int) (ptr - start);
}

/* Pack/unpack coordinates */

int isiPackCoords(UINT8 *start, ISI_COORDS *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackREAL32(ptr, src->lat);
    ptr += utilPackREAL32(ptr, src->lon);
    ptr += utilPackREAL32(ptr, src->elev);
    ptr += utilPackREAL32(ptr, src->depth);

    return (int) (ptr - start);
}

int isiUnpackCoords(UINT8 *start, ISI_COORDS *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackREAL32(ptr, &dest->lat);
    ptr += utilUnpackREAL32(ptr, &dest->lon);
    ptr += utilUnpackREAL32(ptr, &dest->elev);
    ptr += utilUnpackREAL32(ptr, &dest->depth);

    return (int) (ptr - start);
}

/* Pack/unpack instrument data */

int isiPackInst(UINT8 *start, ISI_INST *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackREAL32(ptr, src->calib);
    ptr += utilPackREAL32(ptr, src->calper);
    ptr += utilPackREAL32(ptr, src->hang);
    ptr += utilPackREAL32(ptr, src->vang);
    ptr += utilPackBytes(ptr, (UINT8 *) src->type, ISI_INAMLEN);

    return (int) (ptr - start);
}

int isiUnpackInst(UINT8 *start, ISI_INST *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackREAL32(ptr, &dest->calib);
    ptr += utilUnpackREAL32(ptr, &dest->calper);
    ptr += utilUnpackREAL32(ptr, &dest->hang);
    ptr += utilUnpackREAL32(ptr, &dest->vang);
    ptr += utilUnpackBytes(ptr, (UINT8 *) dest->type, ISI_INAMLEN);
    dest->type[ISI_INAMLEN] = 0;

    return (int) (ptr - start);
}

/* Pack/unpack a datum description */

int isiPackDatumDescription(UINT8 *start, ISI_DATA_DESC *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackBytes(ptr, &src->comp, 1);
    ptr += utilPackBytes(ptr, &src->type, 1);
    ptr += utilPackBytes(ptr, &src->order, 1);
    ptr += utilPackBytes(ptr, &src->size, 1);

    return (int) (ptr - start);
}

int isiUnpackDatumDescription(UINT8 *start, ISI_DATA_DESC *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackBytes(ptr, &dest->comp, 1);
    ptr += utilUnpackBytes(ptr, &dest->type, 1);
    ptr += utilUnpackBytes(ptr, &dest->order, 1);
    ptr += utilUnpackBytes(ptr, &dest->size, 1);

    return (int) (ptr - start);
}

/* Pack/unpack a sample rate */

int isiPackSrate(UINT8 *start, ISI_SRATE *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackINT16(ptr, src->factor);
    ptr += utilPackINT16(ptr, src->multiplier);

    return (int) (ptr - start);
}

int isiUnpackSrate(UINT8 *start, ISI_SRATE *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackINT16(ptr, &dest->factor);
    ptr += utilUnpackINT16(ptr, &dest->multiplier);

    return (int) (ptr - start);
}

/* Pack/unpack a time stamp */

int isiPackTstamp(UINT8 *start, ISI_TSTAMP *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackREAL64(ptr, src->value);
    ptr += utilPackUINT16(ptr, src->status);

    return (int) (ptr - start);
}

int isiUnpackTstamp(UINT8 *start, ISI_TSTAMP *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackREAL64(ptr, &dest->value);
    ptr += utilUnpackUINT16(ptr, &dest->status);

    return (int) (ptr - start);
}

/* Pack/unpack a stream name */

int isiPackStreamName(UINT8 *start, ISI_STREAM_NAME *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackBytes(ptr, (UINT8 *) src->sta, ISI_STALEN);
    ptr += utilPackBytes(ptr, (UINT8 *) src->chn, ISI_CHNLEN);
    ptr += utilPackBytes(ptr, (UINT8 *) src->loc, ISI_LOCLEN);

    return (int) (ptr - start);
}

int isiUnpackStreamName(UINT8 *start, ISI_STREAM_NAME *dest)
{
UINT8 *ptr;
char sta[ISI_STALEN+1], chn[ISI_CHNLEN+1], loc[ISI_LOCLEN+1];

    ptr = start;

    ptr += utilUnpackBytes(ptr, (UINT8 *) sta, ISI_STALEN);
    dest->sta[ISI_STALEN] = 0;
    ptr += utilUnpackBytes(ptr, (UINT8 *) chn, ISI_CHNLEN);
    dest->chn[ISI_CHNLEN] = 0;
    ptr += utilUnpackBytes(ptr, (UINT8 *) loc, ISI_LOCLEN);
    dest->loc[ISI_LOCLEN] = 0;
    isiStaChnLocToStreamName(sta, chn, loc, dest);

    return (int) (ptr - start);
}

/* Pack/unpack ISI_SEQNO_REQUEST */

int isiPackSeqnoRequest(UINT8 *start, ISI_SEQNO_REQUEST *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilPackBytes(ptr, (UINT8 *) src->site, ISI_SITELEN);
    ptr += isiPackSeqno(ptr, &src->beg);
    ptr += isiPackSeqno(ptr, &src->end);

    return (int) (ptr - start);
}

int isiUnpackSeqnoRequest(UINT8 *start, ISI_SEQNO_REQUEST *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += utilUnpackBytes(ptr, (UINT8 *) dest->site, ISI_SITELEN);
    dest->site[ISI_SITELEN] = 0;
    ptr += isiUnpackSeqno(ptr, &dest->beg);
    ptr += isiUnpackSeqno(ptr, &dest->end);

    return (int) (ptr - start);
}

/* Pack/unpack ISI_TWIND_REQUEST */

int isiPackTwindRequest(UINT8 *start, ISI_TWIND_REQUEST *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiPackStreamName(ptr, &src->name);
    ptr += utilPackREAL64(ptr, src->beg);
    ptr += utilPackREAL64(ptr, src->end);

    return (int) (ptr - start);
}

int isiUnpackTwindRequest(UINT8 *start, ISI_TWIND_REQUEST *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiUnpackStreamName(ptr, &dest->name);
    ptr += utilUnpackREAL64(ptr, &dest->beg);
    ptr += utilUnpackREAL64(ptr, &dest->end);

    return (int) (ptr - start);
}

/* Pack/unpack a configuration packet */

int isiPackStreamCnf(UINT8 *start, ISI_STREAM_CNF *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiPackStreamName(ptr, &src->name);
    ptr += isiPackSrate(ptr, &src->srate);
    ptr += isiPackCoords(ptr, &src->coords);
    ptr += isiPackInst(ptr, &src->inst);

    return (int) (ptr - start);
}

int isiUnpackStreamCnf(UINT8 *start, ISI_STREAM_CNF *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiUnpackStreamName(ptr, &dest->name);
    ptr += isiUnpackSrate(ptr, &dest->srate);
    ptr += isiUnpackCoords(ptr, &dest->coords);
    ptr += isiUnpackInst(ptr, &dest->inst);

    return (int) (ptr - start);
}

/* Pack/unpack a state of health packets */

int isiPackStreamSoh(UINT8 *start, ISI_STREAM_SOH *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiPackStreamName(ptr, &src->name);
    ptr += isiPackTstamp(ptr, &src->tofs);
    ptr += isiPackTstamp(ptr, &src->tols);
    ptr += utilPackREAL64(ptr, src->tslw);
    ptr += utilPackUINT32(ptr, src->nseg);
    ptr += utilPackUINT32(ptr, src->nrec);

    return (int) (ptr - start);
}

int isiUnpackStreamSoh(UINT8 *start, ISI_STREAM_SOH *dest)
{
UINT8 *ptr;

    ptr = start;

    ptr += isiUnpackStreamName(ptr, &dest->name);
    ptr += isiUnpackTstamp(ptr, &dest->tofs);
    ptr += isiUnpackTstamp(ptr, &dest->tols);
    ptr += utilUnpackREAL64(ptr, &dest->tslw);
    ptr += utilUnpackUINT32(ptr, &dest->nseg);
    ptr += utilUnpackUINT32(ptr, &dest->nrec);

    return (int) (ptr - start);
}

/* Pack/unpack a CD status structure */

int isiPackCdstatus(UINT8 *start, ISI_CD_STATUS *src)
{
UINT8 *ptr;

    ptr = start;
    *ptr = src->data;     ptr += 1;
    *ptr = src->security; ptr += 1;
    *ptr = src->misc;     ptr += 1;
    *ptr = src->voltage;  ptr += 1;
    ptr += utilPackREAL64(ptr, src->sync);
    ptr += utilPackUINT32(ptr, src->diff);

    return (int) (ptr - start);
}

int isiUnpackCdstatus(UINT8 *start, ISI_CD_STATUS *dest)
{
UINT8 *ptr;

    ptr = start;
    dest->data     = *ptr; ptr += 1;
    dest->security = *ptr; ptr += 1;
    dest->misc     = *ptr; ptr += 1;
    dest->voltage  = *ptr; ptr += 1;
    ptr += utilUnpackREAL64(ptr, &dest->sync);
    ptr += utilUnpackUINT32(ptr, &dest->diff);

    return (int) (ptr - start);
}

/* Pack/unpack an ISI sequence number */

int isiPackSeqno(UINT8 *start, ISI_SEQNO *src)
{
UINT8 *ptr;
static char *fid = "isiPackSeqno";

    ptr = start;
    ptr += utilPackUINT32(ptr, src->signature);
    ptr += utilPackUINT64(ptr, src->counter);

    return (int) (ptr - start);
}

int isiUnpackSeqno(UINT8 *start, ISI_SEQNO *dest)
{
UINT8 *ptr;
static char *fid = "isiPackSeqno";

    ptr = start;
    ptr += utilUnpackUINT32(ptr, &dest->signature);
    ptr += utilUnpackUINT64(ptr, &dest->counter);

    return (int) (ptr - start);
}

/* Pack/unpack a generic time series packet */

int isiPackGenericTSHDR(UINT8 *start, ISI_GENERIC_TSHDR *src)
{
UINT8 *ptr;
static char *fid = "isiPackGenericTSHDR";

    ptr = start;

    ptr += isiPackStreamName(ptr, &src->name);
    ptr += isiPackSrate(ptr, &src->srate);
    ptr += isiPackTstamp(ptr, &src->tofs);
    ptr += isiPackTstamp(ptr, &src->tols);
    ptr += isiPackCdstatus(ptr, &src->status);
    ptr += utilPackUINT32(ptr, src->nsamp);
    ptr += utilPackUINT32(ptr, src->nbytes);
    ptr += isiPackDatumDescription(ptr, &src->desc);

    return (int) (ptr - start);
}

int isiUnpackGenericTSHDR(UINT8 *start, ISI_GENERIC_TSHDR *dest)
{
UINT8 *ptr;

    ptr = start;
    ptr += isiUnpackStreamName(ptr, &dest->name);
    ptr += isiUnpackSrate(ptr, &dest->srate);
    ptr += isiUnpackTstamp(ptr, &dest->tofs);
    ptr += isiUnpackTstamp(ptr, &dest->tols);
    ptr += isiUnpackCdstatus(ptr, &dest->status);
    ptr += utilUnpackUINT32(ptr, &dest->nsamp);
    ptr += utilUnpackUINT32(ptr, &dest->nbytes);
    ptr += isiUnpackDatumDescription(ptr, &dest->desc);

    return (int) (ptr - start);
}

/* Unpack a generic TS packet, keeping data in its place */

int isiUnpackGenericTS(UINT8 *start, ISI_GENERIC_TS *dest)
{
UINT8 *ptr;

    ptr = start;
    ptr += isiUnpackGenericTSHDR(ptr, &dest->hdr);
    dest->data = (VOID *) ptr;
    ptr += dest->hdr.nbytes;

    return (int) (ptr - start);
}

/* Pack a raw digitizer packet */

static int packTaggedSiteName(UINT8 *start, char *src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_SITE_NAME));
    ptr += sizeof(INT32);
    ptr += (length = utilPackBytes(ptr, src, ISI_SITELEN));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedSeqno(UINT8 *start, ISI_SEQNO *src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_SEQNO));
    ptr += sizeof(INT32);
    ptr += (length = isiPackSeqno(ptr, src));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedDatumDescription(UINT8 *start, ISI_DATA_DESC *src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_DATA_DESC));
    ptr += sizeof(INT32);
    ptr += (length = isiPackDatumDescription(ptr, src));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedLenUsed(UINT8 *start, UINT32 src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_LEN_USED));
    ptr += sizeof(INT32);
    ptr += (length = utilPackUINT32(ptr, src));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedLenNative(UINT8 *start, UINT32 src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_LEN_NATIVE));
    ptr += sizeof(INT32);
    ptr += (length = utilPackUINT32(ptr, src));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedStatus(UINT8 *start, UINT32 src)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_RAW_STATUS));
    ptr += sizeof(INT32);
    ptr += (length = utilPackUINT32(ptr, src));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedPayload(UINT8 *start, UINT8 *src, int len)
{
INT32 length;
UINT8 *ptr, *lptr;

    ptr = start;

    lptr = (ptr += utilPackINT32(ptr, ISI_TAG_PAYLOAD));
    ptr += sizeof(INT32);
    ptr += (length = utilPackBytes(ptr, src, len));
    utilPackINT32(lptr, length);

    return (int) (ptr - start);
}

static int packTaggedRawHeader(UINT8 *start, ISI_RAW_HEADER *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += packTaggedSiteName(ptr, src->site);
    ptr += packTaggedSeqno(ptr, &src->seqno);
    ptr += packTaggedDatumDescription(ptr, &src->desc);
    ptr += packTaggedLenUsed(ptr, src->len.used);
    ptr += packTaggedLenNative(ptr, src->len.native);
    if (src->status != ISI_RAW_STATUS_OK) ptr += packTaggedStatus(ptr, src->status);

    return (int) (ptr - start);
}

int isiPackRawPacket(UINT8 *start, ISI_RAW_PACKET *src)
{
UINT8 *ptr;

    ptr = start;

    ptr += packTaggedRawHeader(ptr, &src->hdr);
    ptr += packTaggedPayload(ptr, src->payload, (int) src->hdr.len.used);

    ptr += TerminateInput(ptr);

    return (int) (ptr - start);
}

int isiBuildPackRawPacket(UINT8 *start, ISI_RAW_HEADER *hdr, UINT8 *data)
{
UINT8 *ptr;

    ptr = start;

    ptr += packTaggedRawHeader(ptr, hdr);
    ptr += packTaggedPayload(ptr, data, (int) hdr->len.used);

    ptr += TerminateInput(ptr);

    return (int) (ptr - start);
}

/* Unpack a raw digitizer packet, keeping payload in place */

void isiUnpackRawPacket(UINT8 *start, ISI_RAW_PACKET *dest)
{
UINT8 *mainPtr, *ptr;
UINT32 type;

    dest->hdr.status = ISI_RAW_STATUS_OK; /* by default */
    mainPtr = start;
    while ((mainPtr = GetNextField(mainPtr, &type, &ptr)) != NULL) {
        switch (type) {
          case ISI_TAG_SITE_NAME:
            utilUnpackBytes(ptr, (UINT8 *) dest->hdr.site, ISI_SITELEN);
            dest->hdr.site[ISI_SITELEN] = 0;
            break;

          case ISI_TAG_DATA_DESC:
            isiUnpackDatumDescription(ptr, &dest->hdr.desc);
            break;

          case ISI_TAG_RAW_STATUS:
            utilUnpackUINT32(ptr, &dest->hdr.status);
            break;

          case ISI_TAG_SEQNO:
            isiUnpackSeqno(ptr, &dest->hdr.seqno);
            break;

          case ISI_TAG_LEN_USED:
            utilUnpackUINT32(ptr, &dest->hdr.len.used);
            break;

          case ISI_TAG_LEN_NATIVE:
            utilUnpackUINT32(ptr, &dest->hdr.len.native);
            break;

          case ISI_TAG_PAYLOAD:
            dest->payload = ptr;
            break;
        }
    }
}

/* Revision History
 *
 * $Log: pack.c,v $
 * Revision 1.14  2008/01/25 21:49:52  dechavez
 * build chnloc field in isiUnpackStreamName()
 *
 * Revision 1.13  2007/01/11 17:50:35  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.12  2006/02/09 00:10:45  dechavez
 * Renamed 'oldchn' field of ISI_STREAM_NAME to 'chnloc'
 *
 * Revision 1.11  2005/08/26 18:39:51  dechavez
 * fixed nasty bug in unpacking ISI_RAW_HEADER
 *
 * Revision 1.10  2005/07/26 00:15:01  dechavez
 * added support for ISI_TAG_RAW_STATUS
 *
 * Revision 1.9  2005/07/06 15:29:56  dechavez
 * support for new len field in raw packet header
 *
 * Revision 1.8  2005/06/30 01:29:38  dechavez
 * added isiPackRawPacketHeaderAndData()
 *
 * Revision 1.7  2005/06/24 21:39:33  dechavez
 * support for updated (hdr/payload) packets and redesigned ISI_DATA_REQUEST structure
 *
 * Revision 1.6  2005/06/01 00:16:19  dechavez
 * fixed oldchn intialization error in isiUnpackStreamName()
 *
 * Revision 1.5  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.4  2005/05/06 00:50:59  dechavez
 * added tagged field and ISI_SEQNO and ISI_RAW_PACKET support
 *
 * Revision 1.3  2003/11/19 23:49:14  dechavez
 * cast utilPackBytes/utilUnpackBytes args to calm compiler
 *
 * Revision 1.2  2003/11/03 23:18:50  dechavez
 * added isiUnpackGenericTS()
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
