#pragma ident "$Id: raw.c,v 1.7 2005/09/30 22:11:35 dechavez Exp $"
/*======================================================================
 *
 * Support for ISI_RAW_PACKET stuff
 *
 *====================================================================*/
#define INCLUDE_ISI_STATIC_SEQNOS
#define INCLUDE_STATIC_DATA_DESC
#include "isi.h"
#include "util.h"
#include "zlib.h"

BOOL isiInitRawHeader(ISI_RAW_HEADER *hdr)
{
    if (hdr == NULL) {
        errno = EINVAL;
        return FALSE;
    }
    
    memset(hdr->site, 0, ISI_SITELEN);
    hdr->seqno = ISI_UNDEFINED_SEQNO;
    hdr->desc = ISI_DEFAULT_DATA_DESC;
    hdr->len.used = 0;
    hdr->len.native = 0;
    hdr->status = ISI_RAW_STATUS_OK;

    return TRUE;
}

BOOL isiInitRawPacket(ISI_RAW_PACKET *raw, VOID *buf, UINT32 buflen)
{
    if (raw == NULL) {
        errno = EINVAL;
        return FALSE;
    }
    
    isiInitRawHeader(&raw->hdr);
    if (buf != NULL) {
        raw->payload = buf;
    } else {
        if ((raw->payload = (VOID *) malloc(buflen)) == NULL) return FALSE;
    }
    raw->hdr.len.payload = buflen;

    return TRUE;
}

BOOL isiDecompressRawPacket(ISI_RAW_PACKET *raw, UINT8 *buf, UINT32 buflen)
{
int status;
uLong nbytes;
static char *fid = "isiDecompressRawPacket";

    if (raw == NULL || buf == NULL || buflen == 0) {
        errno = EINVAL;
        return FALSE;
    }

/* uncompressed packets need no attention */

    if (raw->hdr.desc.comp == ISI_COMP_NONE) return TRUE;

/* we only support gzip compression, and since all client implementations
 * only request gzip compression, this should not be a problem until
 * someone decides to write their own implementation... not likely
 */
    if (raw->hdr.desc.comp != ISI_COMP_GZIP) {
        errno = ENOTSUP;
        return FALSE;
    }

/* Do the decompresion */

    nbytes = (uLong) buflen;
    status = uncompress((Bytef *) buf, &nbytes, (Bytef *) raw->payload, (uLong) raw->hdr.len.used);
    if (status != Z_OK) return FALSE;

    raw->payload = buf;
    raw->hdr.len.payload = buflen;
    raw->hdr.len.used  = (UINT32) nbytes;
    raw->hdr.desc.comp = ISI_COMP_NONE;

    return TRUE;
}

BOOL isiCompressRawPacket(ISI_RAW_PACKET *raw, UINT8 *buf, UINT32 buflen)
{
int status;
uLong nbytes;
static char *fid = "isiCompressRawPacket";

    if (raw == NULL || buf == NULL || buflen == 0) {
        errno = EINVAL;
        return FALSE;
    }

/* already compressed packets are left alone */

    if (raw->hdr.desc.comp != ISI_COMP_NONE) return TRUE;

/* We will only gzip compress, regardless of what they ask */

    nbytes = (uLong) buflen;
    status = compress2((Bytef *) buf, &nbytes, (Bytef *) raw->payload, (uLong) raw->hdr.len.used, Z_BEST_COMPRESSION);
    if (status != Z_OK) return FALSE;

    if (nbytes < raw->hdr.len.used) {
        raw->payload = buf;
        raw->hdr.len.used  = (UINT32) nbytes;
        raw->hdr.desc.comp = ISI_COMP_GZIP;
    }

    return TRUE;
}

void isiDestroyRawPacket(ISI_RAW_PACKET *raw)
{
    if (raw == NULL) return;
    if (raw->payload != NULL) free(raw->payload);
    free(raw);
}

ISI_RAW_PACKET *isiAllocateRawPacket(UINT32 buflen)
{
ISI_RAW_PACKET *raw;

    if ((raw = (ISI_RAW_PACKET *) malloc(sizeof(ISI_RAW_PACKET))) == NULL) return NULL;
    isiInitRawHeader(&raw->hdr);
    if ((raw->payload = (VOID *) malloc(buflen)) == NULL) {
        isiDestroyRawPacket(raw);
        return NULL;
    }
    raw->hdr.len.payload = buflen;

    return raw;
}

/* Revision History
 *
 * $Log: raw.c,v $
 * Revision 1.7  2005/09/30 22:11:35  dechavez
 * removed tabs
 *
 * Revision 1.6  2005/09/10 21:36:56  dechavez
 * added isiAllocateRawPacket(), isiDestroyRawPacket(), added support for NULL
 * buf pointer for isiInitRawPacket()
 *
 * Revision 1.5  2005/07/26 00:15:25  dechavez
 * added support for ISI_TAG_RAW_STATUS
 *
 * Revision 1.4  2005/07/06 15:27:57  dechavez
 * removed uneeded "precious" stuff, uneeded isiCopyRawPacket() and added
 * isiCompressRawPacket() and isiDecompressRawPacket()
 *
 * Revision 1.3  2005/06/24 21:35:40  dechavez
 * mods, additions, subtractions as part of getting isidl to work
 *
 * Revision 1.2  2005/06/10 15:48:36  dechavez
 * updated comments
 *
 * Revision 1.1  2005/05/06 00:48:54  dechavez
 * initial release
 *
 */
