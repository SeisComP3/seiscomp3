#pragma ident "$Id: string.c,v 1.26 2008/01/25 21:48:59 dechavez Exp $"
/*======================================================================
 *
 *  ISI related string functions
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

char *isiReqStatusString(int value)
{
static char    *unknown = "UNKNOWN";
static char   *complete = "complete";
static char *incomplete = "incomplete";

    switch (value) {
      case ISI_COMPLETE:    return complete;
      case ISI_INCOMPLETE: return incomplete;
    }

    return unknown;
}

char *isiFormatString(int value)
{
static char  *unknown = "UNKNOWN";
static char    *undef = "undefined";
static char  *generic = "generic";
static char   *native = "native";

    switch (value) {
      case ISI_FORMAT_UNDEF:   return undef;
      case ISI_FORMAT_GENERIC: return generic;
      case ISI_FORMAT_NATIVE:  return native;
    }

    return unknown;

}

char *isiCompressString(int value)
{
static char *unknown = "UNKNOWN";
static char   *undef = "undefined";
static char    *none = "uncompressed";
static char     *ida = "idacmp";
static char  *steim1 = "steim1";
static char  *steim2 = "steim2";
static char    *gzip = "gzip";

    switch (value) {
      case ISI_COMP_UNDEF:  return undef;
      case ISI_COMP_NONE:   return none;
      case ISI_COMP_IDA:    return ida;
      case ISI_COMP_STEIM1: return steim1;
      case ISI_COMP_STEIM2: return steim2;
      case ISI_COMP_GZIP:   return gzip;
    }

    return unknown;

}

char *isiRequestTypeString(int value)
{
static char *unknown = "UNKNOWN";
static char   *undef = "undefined";
static char   *seqno = "sequence number";
static char   *twind = "time window";

    switch (value) {
      case ISI_REQUEST_TYPE_UNDEF: return undef;
      case ISI_REQUEST_TYPE_SEQNO: return seqno;
      case ISI_REQUEST_TYPE_TWIND: return twind;
    }

    return unknown;

}

char *isiPolicyString(int value)
{
static char   *unknown = "UNKNOWN";
static char     *undef = "undefined";
static char      *none = "none";
static char   *min_gap = "minimal gap";
static char *min_delay = "minimal delay";

    switch (value) {
      case ISI_RECONNECT_POLICY_UNDEF:     return undef;
      case ISI_RECONNECT_POLICY_NONE:      return none;
      case ISI_RECONNECT_POLICY_MIN_GAP:   return min_gap;
      case ISI_RECONNECT_POLICY_MIN_DELAY: return min_delay;
    }

    return unknown;

}

char *isiDatumTypeString(int value)
{
static char *unknown = "??????";
static char   *undef = "undef ";
static char    *int8 = "int8  ";
static char   *int16 = "int16 ";
static char   *int32 = "int32 ";
static char   *int64 = "int64 ";
static char  *real32 = "real32";
static char  *real64 = "real64";
static char    *ida5 = "ida5  ";
static char    *ida6 = "ida6  ";
static char    *ida7 = "ida7  ";
static char    *ida8 = "ida8  ";
static char    *ida9 = "ida9  ";
static char   *ida10 = "ida10 ";
static char  *qdplus = "qdplus";
static char   *mseed = "mseed ";

    switch (value) {
      case ISI_TYPE_UNDEF:  return undef;
      case ISI_TYPE_INT8:   return int8;
      case ISI_TYPE_INT16:  return int16;
      case ISI_TYPE_INT32:  return int32;
      case ISI_TYPE_REAL32: return real32;
      case ISI_TYPE_REAL64: return real64;
      case ISI_TYPE_IDA5:   return ida5;
      case ISI_TYPE_IDA6:   return ida6;
      case ISI_TYPE_IDA7:   return ida7;
      case ISI_TYPE_IDA8:   return ida8;
      case ISI_TYPE_IDA9:   return ida9;
      case ISI_TYPE_IDA10:  return ida10;
      case ISI_TYPE_QDPLUS: return qdplus;
      case ISI_TYPE_MSEED:  return mseed;
    }

    return unknown;
}

BOOL isiStringToStreamName(char *string, ISI_STREAM_NAME *stream)
{
int ntoken = 0;
#define STA_CHN_LOC_TOKEN_COUNT 3
#define STA_CHN_TOKEN_COUNT    (STA_CHN_LOC_TOKEN_COUNT - 1)
char *copy, *token[STA_CHN_LOC_TOKEN_COUNT];
static char *BlankLoc = "  ";

    if (string == NULL || stream == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if ((copy = strdup(string)) == NULL) return FALSE;
    ntoken = utilParse(copy, token, ISI_STA_CHN_LOC_DELIMITERS, STA_CHN_LOC_TOKEN_COUNT, 0);
    if (ntoken == STA_CHN_TOKEN_COUNT) {
        token[STA_CHN_TOKEN_COUNT] = BlankLoc;
    } else if (ntoken != STA_CHN_LOC_TOKEN_COUNT) {
        free(copy);
        return FALSE;
    }

    strlcpy(stream->sta, token[0], ISI_STALEN+1);
    strlcpy(stream->chn, token[1], ISI_CHNLEN+1);
    strlcpy(stream->loc, token[2], ISI_LOCLEN+1);

    free(copy);
    return TRUE;
}

char *isiStreamNameString(ISI_STREAM_NAME *stream, char *string)
{
static char mt_unsafe[ISI_STREAM_NAME_LEN+1];

    if (string == NULL) string = mt_unsafe;

    if (stream->chnloc[0] != 0) {
        sprintf(string, "%s:%s", stream->sta, stream->chnloc);
    } else if (strlen(stream->loc) != 0 && strcmp(stream->loc, ISI_BLANK_LOC) != 0) {
        sprintf(string, "%4s.%3s.%s", stream->sta, stream->chn, stream->loc);
    } else {
        sprintf(string, "%4s.%3s   ", stream->sta, stream->chn);
    }
    return string;
}

char *isiTstampString(ISI_TSTAMP *tstamp, char *buf)
{
    utilDttostr(tstamp->value, 0, buf);
    sprintf(buf+strlen(buf), " 0x%04x", tstamp->status);
    return buf;
}

char *isiDescString(ISI_DATA_DESC *desc, char *buf)
{
    buf[0] = 0;
    sprintf(buf+strlen(buf), "%s", isiCompressString(desc->comp));
    sprintf(buf+strlen(buf), ":%s", isiDatumTypeString(desc->type));
    if (!isiIsNative(desc)) {
        sprintf(buf+strlen(buf), ":%s", desc->order == ISI_ORDER_BIGENDIAN ? "nbo" : "obn");
    }
    return buf;
}

char *isiGenericTsHdrString(ISI_GENERIC_TSHDR *hdr, char *buf)
{
char tbuf[128];

    buf[0] = 0;
    isiStreamNameString(&hdr->name, buf);
    sprintf(buf+strlen(buf), " %s", isiTstampString(&hdr->tofs, tbuf));
    sprintf(buf+strlen(buf), " %s", isiTstampString(&hdr->tols, tbuf));
    sprintf(buf+strlen(buf), " %6.3lf", isiSrateToSint(&hdr->srate));
    sprintf(buf+strlen(buf), " %3d", hdr->nsamp);
    sprintf(buf+strlen(buf), " %4d", hdr->nbytes);
    sprintf(buf+strlen(buf), " %s", isiDescString(&hdr->desc, tbuf));
    return buf;
}

char *isiRawHeaderString(ISI_RAW_HEADER *hdr, char *buf)
{
int percent;
char tbuf[128];

    buf[0] = 0;
    sprintf(buf+strlen(buf), "%5s", hdr->site);
    sprintf(buf+strlen(buf), " %s", isiSeqnoString(&hdr->seqno, tbuf));
    sprintf(buf+strlen(buf), " %s", isiDescString(&hdr->desc, tbuf));
    sprintf(buf+strlen(buf), " %5lu", hdr->len.used);
    sprintf(buf+strlen(buf), " %5lu", hdr->len.native);
    if (hdr->desc.comp != ISI_COMP_NONE) {
        percent = 100 - (int) (((float) hdr->len.used / (float) hdr->len.native) * 100.0);
        sprintf(buf+strlen(buf), " %2d%%", percent);
    }
    return buf;
}

char *isiRawPacketString(ISI_RAW_PACKET *raw, char *buf)
{
char tbuf[128];

    buf[0] = 0;
    sprintf(buf+strlen(buf), "%s", isiRawHeaderString(&raw->hdr, tbuf));
    return buf;
}

char *isiSeqnoString(ISI_SEQNO *seqno, char *buf)
{
static char *oldest_string = "[    beg of disk loop    ]";
static char *newest_string = "[    end of disk loop    ]";
static char *keepup_string = "[        forever         ]";
static char *undef_string  = "[    undefined seqno     ]";
static char mt_unsafe[ISI_SEQ_NO_STRING_LEN+1];

    if (buf == NULL) buf = mt_unsafe;

    if (isiIsOldestSeqno(seqno)) {
        sprintf(buf, "%s", oldest_string);
    } else if (isiIsNewestSeqno(seqno)) {
        sprintf(buf, "%s", newest_string);
    } else if (isiIsKeepupSeqno(seqno)) {
        sprintf(buf, "%s", keepup_string);
    } else if (isiIsUndefinedSeqno(seqno)) {
        sprintf(buf, "%s", undef_string);
    } else {
        sprintf(buf, "%08lx", seqno->signature);
        sprintf(buf+strlen(buf), " %08lx", (UINT32) (seqno->counter >> 32));
        sprintf(buf+strlen(buf), " %08lx", (UINT32) seqno->counter & 0x00000000ffffffff);
    }

    return buf;
}

char *isiTwindRequestString(ISI_TWIND_REQUEST *req, char *buf)
{
char tbuf[64];

    buf[0] = 0;
    sprintf(buf+strlen(buf), "'%s'.'%s'.'%s'", req->name.sta, req->name.chn, req->name.loc);
    sprintf(buf+strlen(buf), " %s", isiRequestTimeString(req->beg, tbuf));
    sprintf(buf+strlen(buf), " %s", isiRequestTimeString(req->end, tbuf));

    return buf;
}

char *isiSeqnoRequestString(ISI_SEQNO_REQUEST *req, char *buf)
{
char tbuf[64];

    buf[0] = 0;
    sprintf(buf+strlen(buf), "%s", req->site);
    sprintf(buf+strlen(buf), " %s", isiSeqnoString(&req->beg, tbuf));
    sprintf(buf+strlen(buf), " %s", isiSeqnoString(&req->end, tbuf));

    return buf;
}

char *isiRequestTimeString(REAL64 value, char *buf)
{
char tbuf[256];
static char *oldest_string = "<  beg of disk loop >";
static char *newest_string = "<  end of disk loop >";
static char *keepup_string = "<       forever     >";

    if (value == ISI_OLDEST) {
        sprintf(buf, "%s", oldest_string);
    } else if (value == ISI_NEWEST) {
        sprintf(buf, "%s", newest_string);
    } else if (value == ISI_KEEPUP) {
        sprintf(buf, "%s", keepup_string);
    } else {
        sprintf(buf, "%s", utilDttostr(value, 0, tbuf));
    }

    return buf;
}

char *isiDatatypeString(ISI_DATA_DESC *desc)
{
static char *s2 = "s2";
static char *i2 = "i2";
static char *s4 = "s4";
static char *i4 = "i4";
static char *t4 = "t4";
static char *t8 = "t8";
static char *null = " -";

    if (desc == NULL) return null;

    if (desc->comp != ISI_COMP_NONE) return null;

    if (desc->order == ISI_ORDER_BIGENDIAN) {
        switch (desc->type) {
          case ISI_TYPE_INT16:  return s2;
          case ISI_TYPE_INT32:  return s4;
          case ISI_TYPE_REAL32: return t4;
          case ISI_TYPE_REAL64: return t8;
        }
    } else {
        switch (desc->type) {
          case ISI_TYPE_INT16:  return i2;
          case ISI_TYPE_INT32:  return i4;
          case ISI_TYPE_REAL32: return t4;
          case ISI_TYPE_REAL64: return t8;
        }
    }

    return null;
}

/* Revision History
 *
 * $Log: string.c,v $
 * Revision 1.26  2008/01/25 21:48:59  dechavez
 * removed tabs
 *
 * Revision 1.25  2007/10/31 16:52:06  dechavez
 * replaced string memcpy with strlcpy
 *
 * Revision 1.24  2007/05/17 20:33:58  dechavez
 * allow for NULL destination string in isiStreamNameString()
 *
 * Revision 1.23  2007/01/11 17:50:35  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.22  2006/12/13 21:53:52  dechavez
 * add optional internal buffer to isiSeqnoString for non-MT use,
 * changed formatting of non-numeric strings
 *
 * Revision 1.21  2006/11/10 06:56:38  dechavez
 * REAL64 support
 *
 * Revision 1.20  2006/07/07 17:26:39  dechavez
 * fixed bug in isiSeqnoString with 3-part seqno's under Linux
 *
 * Revision 1.19  2006/06/19 19:06:29  dechavez
 * support ISI_TYPE_IDA[567] and ISI_TYPE_MSEED
 *
 * Revision 1.18  2006/06/12 21:26:18  dechavez
 * changed isiSeqnoString's undef to "undefined seqno" so it could be
 * recognized as a sequence number
 *
 * Revision 1.17  2006/06/07 22:15:11  dechavez
 * made static TypeString public isiDataTypeString()
 *
 * Revision 1.16  2006/06/02 20:51:22  dechavez
 * added ISI_TYPE_QDPLUS to TypeString
 *
 * Revision 1.15  2006/03/13 22:23:06  dechavez
 * changed format of seqno string
 *
 * Revision 1.14  2006/02/09 00:09:59  dechavez
 * Renamed 'oldchn' field of ISI_STREAM_NAME to 'chnloc'
 *
 * Revision 1.13  2005/08/26 18:38:53  dechavez
 * added ISI_TYPE_IDA9 support
 *
 * Revision 1.12  2005/07/26 00:17:54  dechavez
 * removed isiDLStateString(), isiIndexString() (to isidl) added isiDatatypeString()
 *
 * Revision 1.11  2005/07/06 15:30:26  dechavez
 * support for new len field in raw packet header
 *
 * Revision 1.10  2005/06/30 01:26:55  dechavez
 * added isiDLStateString(), isiSeqnoString(), isiStreamRequestString(), isiSeqnoRequestString()
 *
 * Revision 1.9  2005/06/24 21:32:59  dechavez
 * added isiRequestTypeString(), isiRawHeaderString(), isiRawPacketString(), isiIndexString()
 *
 * Revision 1.8  2005/05/06 00:58:35  dechavez
 * renamed isiStreamString() to isiStreamNameString() and added isiStringToStreamName()
 *
 * Revision 1.7  2005/01/28 01:49:24  dechavez
 * print at least 3 char chan field in isiStreamString()
 *
 * Revision 1.6  2004/06/21 19:46:44  dechavez
 * recognize blank loc in isiStreamString()
 *
 * Revision 1.5  2003/11/26 21:20:52  dechavez
 * fixed order test in isiDescString
 *
 * Revision 1.4  2003/11/19 23:48:01  dechavez
 * include util.h to calm compiler
 *
 * Revision 1.3  2003/11/04 19:59:38  dechavez
 * added isiReqStatusString(), changed isiStreamString to use sta/chan/loc if
 * oldchn is missing
 *
 * Revision 1.2  2003/11/03 23:14:06  dechavez
 * removed sample size from isiDescString(), added nbytes to isiGenericTsHdrString()
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
