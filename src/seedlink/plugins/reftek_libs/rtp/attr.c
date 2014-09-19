#pragma ident "$Id: attr.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Encode, send, decode, and extract attribute structures
 *
 *====================================================================*/
#include "rtp.h"

/* encode an attribute structure */

static INT32 encode(UINT8 *buf, struct rtp_attr *attr)
{
UINT32 ltmp; size_t llen = 4;
UINT8 *ptr;

    ptr = buf;

    ltmp = (UINT32) htonl((u_long) attr->at_dasid);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_pmask);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_smask);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_timeo);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_block);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_sndbuf);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    ltmp = (UINT32) htonl((u_long) attr->at_rcvbuf);
    memcpy((void *) ptr, (void *) &ltmp, llen);
    ptr += llen;

    return (INT32) (ptr - buf);
}

/* Decode an attribute structure */

VOID rtp_attr_decode(UINT8 *buf, struct rtp_attr *attr)
{
UINT32  ltmp; size_t llen = 4;
UINT8 *ptr;
static CHAR *fid = "rtp_attr_decode";

    if (buf == (UINT8 *) NULL || attr == (struct rtp_attr *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        return;
    }

    ptr = buf;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_dasid = (UINT16) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_pmask = (UINT16) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_smask = (UINT16) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_timeo = (UINT16) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_block = (BOOL) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_sndbuf = (INT32) ntohl((u_long) ltmp);
    ptr += llen;

    memcpy((void *) &ltmp, (void *) ptr, llen);
    attr->at_rcvbuf = (INT32) ntohl((u_long) ltmp);
    ptr += llen;
}

/* Send an attribute structure */

BOOL rtp_attr_send(RTP *rtp, struct rtp_attr *attr)
{
INT32  datlen;
UINT8 msg[2*sizeof(struct rtp_attr)];
static CHAR *fid = "rtp_attr_send";

    if (rtp == (RTP *) NULL || attr == (struct rtp_attr *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return FALSE;
    }

    datlen = encode(msg, attr);

    return rtp_send(rtp, msg, RTP_MSG_ATTR, datlen);
}

/* Copy the current attribute structure to a user buffer */

BOOL rtp_getattr(RTP *rtp, struct rtp_attr *attr)
{
static CHAR *fid = "rtp_getattr";

    if (rtp == (RTP *) NULL || attr == (struct rtp_attr *) NULL) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return FALSE;
    }

    MUTEX_LOCK(&rtp->attr.at_mutex);
        *attr = rtp->attr;
    MUTEX_UNLOCK(&rtp->attr.at_mutex);

    return TRUE;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:48  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
