#pragma ident "$Id: utils.c,v 1.16 2008/02/03 17:20:17 dechavez Exp $"
/*======================================================================
 *
 *  Misc convenience functions
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"
#ifdef ISI_SERVER
#include "ida.h"
#include "ida10.h"
#include "liss.h"
#endif /* ISI_SERVER */

/* Convert sample interval to ISI srate */

ISI_SRATE *isiSintToSrate(REAL64 sint, ISI_SRATE *srate)
{
    if (srate == (ISI_SRATE *) NULL) {
        errno = EINVAL;
        return srate;
    }

    srate->value = 1.0 / sint;

    if (sint >= 1.0) {
        srate->factor = (INT16) -sint;
        srate->multiplier = 1;
    } else {
        srate->factor = (INT16) rint((REAL64) 1.0 / sint);
        srate->multiplier = 1;
    }

    return srate;
}

/* Convert ISI srate structure to sample interval */

REAL64 isiSrateToSint(ISI_SRATE *srate)
{
    if (srate->factor > 0 && srate->multiplier > 0) {
        return 1.0 / (REAL64) srate->factor * (REAL64) srate->multiplier;
    } else if (srate->factor > 0 && srate->multiplier < 0) {
        return (REAL64) -srate->multiplier / (REAL64) srate->factor;
    } else if (srate->factor < 0 && srate->multiplier > 0) {
        return (REAL64) -srate->factor / (REAL64) srate->multiplier;
    } else {
        return (REAL64) srate->factor / (REAL64) srate->multiplier;
    }
}

/* Compare ISI_STREAM_NAMES */

static int CompareWildStrings(char *a, char *b, BOOL wild)
{
    if (wild) {
        if (strcmp(a, ISI_NAME_WILDCARD) == 0) return 0;
        if (strcmp(b, ISI_NAME_WILDCARD) == 0) return 0;
    }

    return strcmp(a, b);
}

static int CompareWildNames(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b, BOOL wild)
{
int result;

    if ((result = CompareWildStrings(a->sta, b->sta, wild)) != 0) return result;
    if ((result = CompareWildStrings(a->chn, b->chn, wild)) != 0) return result;
    if ((result = CompareWildStrings(a->loc, b->loc, wild)) != 0) return result;

    return 0;
}

int isiStreamNameCompare(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b)
{
    return CompareWildNames(a, b, FALSE);
}

int isiStreamNameCompareWild(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b)
{
    return CompareWildNames(a, b, TRUE);
}

BOOL isiStreamNameMatch(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b)
{
    return isiStreamNameCompare(a, b) == 0 ? TRUE : FALSE;
}

/* Build an ISI_STREAM_NAME from basic parts */

VOID isiStaChnLocToStreamName(char *sta, char *chn, char *loc, ISI_STREAM_NAME *name)
{
    if (sta == NULL || chn == NULL || name == NULL) return;

    strlcpy(name->sta, sta, ISI_STALEN+1);
    if (loc == NULL || strcmp(loc, ISI_BLANK_LOC) == 0) {
        if (strlen(chn) == 5) {
            memcpy(name->chn, chn, 3);
            name->chn[3] = 0;
            memcpy(name->loc, chn+3, 2);
            name->loc[2] = 0;
        } else {
            strlcpy(name->chn, chn, ISI_CHNLEN+1);
            strlcpy(name->loc, ISI_BLANK_LOC, ISI_LOCLEN+1);
        }
        strlcpy(name->chnloc, chn, ISI_CHNLOCLEN+1);
    } else {
        strlcpy(name->chn, chn, ISI_CHNLEN+1);
        strlcpy(name->loc, loc, ISI_LOCLEN+1);
        snprintf(name->chnloc, ISI_CHNLOCLEN+1, "%s%s", name->chn, name->loc);
    }
}

/* Compare site names */

int isiSiteNameCompareWild(char *a, char *b)
{
    return CompareWildStrings(a, b, TRUE);
}

/* Take a string of "+" delimited stream specifiers and turn into a list
 * of ISI_STREAM_NAMES
 */

LNKLST *isiExpandStreamNameSpecifier(char *StreamSpec)
{
LNKLST *list, *TokenList;
LNKLST_NODE *crnt;
ISI_STREAM_NAME name;
static char *DefaultSpec = ISI_DEFAULT_STREAMSPEC;
static char *fid = "isiExpandStreamNameSpecifier";

    if (StreamSpec == NULL) StreamSpec = DefaultSpec;

    if ((list = listCreate()) == NULL) return NULL;
    if ((TokenList = utilStringTokenList(StreamSpec, "+", 0)) == NULL) return NULL;

    crnt = listFirstNode(TokenList);
    while (crnt != NULL) {
        if (!isiStringToStreamName((char *) crnt->payload, &name)) {
            listDestroy(TokenList);
            listDestroy(list);
            return NULL;
        }
        if (!listAppend(list, &name, sizeof(ISI_STREAM_NAME))) {
            listDestroy(TokenList);
            listDestroy(list);
            return NULL;
        }
        crnt = listNextNode(crnt);
    }

    if (!listSetArrayView(list)) {
        listDestroy(TokenList);
        listDestroy(list);
        return NULL;
    }

    listDestroy(TokenList);
    return list;

}

/* Take a string of "+" delimited stream specifiers and turn into a list of site names */

LNKLST *isiExpandSeqnoSiteSpecifier(char *SiteSpec)
{
LNKLST *TokenList;
static char *DefaultSpec = ISI_DEFAULT_SITESPEC;

    if (SiteSpec == NULL) SiteSpec = DefaultSpec;
    if ((TokenList = utilStringTokenList(SiteSpec, "+", 0)) == NULL) return NULL;

    if (!listSetArrayView(TokenList)) {
        listDestroy(TokenList);
        listDestroy(TokenList);
        return NULL;
    }
    return TokenList;
}

#ifdef ISI_SERVER

/* Extract the ISI_STREAM_NAME, if any, from a raw packet */

static ISI_STREAM_NAME *GetIda10StreamName(ISI_STREAM_NAME *name, ISI_RAW_PACKET *raw)
{
IDA10_TSHDR tshdr;

    if (ida10Type(raw->payload) != IDA10_TYPE_TS) return NULL;

    ida10UnpackTSHdr(raw->payload, &tshdr);
    isiStaChnToStreamName(tshdr.sname, tshdr.cname, name);

    return name;
}

static ISI_STREAM_NAME *GetIdaStreamName(ISI_STREAM_NAME *name, ISI_RAW_PACKET *raw, IDA *ida)
{
IDA_DHDR dhdr;
static char *fid = "GetIdaStreamName";

    if (raw->payload[0] != 0x44) return NULL;
    if (raw->payload[1] != 0xBB) return NULL;

    ida_dhead(ida, &dhdr, raw->payload);
    idaBuildStreamName(ida, &dhdr, name);

    return name;
}

static ISI_STREAM_NAME *GetMseedStreamName(ISI_STREAM_NAME *name, ISI_RAW_PACKET *raw)
{
LISS_PKT pkt;

    if (!lissUnpackMiniSeed(&pkt, raw->payload, 0)) return NULL;
    if (pkt.status != LISS_PKT_OK) return NULL;

    strlcpy(name->sta, pkt.fsdh.staid, ISI_STALEN+1); util_lcase(name->sta);
    strlcpy(name->chn, pkt.fsdh.chnid, ISI_CHNLEN+1); util_lcase(name->chn);
    strlcpy(name->loc, pkt.fsdh.locid, ISI_LOCLEN+1); util_lcase(name->loc);
    sprintf(name->chnloc, "%s%s", name->chn, name->loc);
    utilTrimString(name->chnloc);

    return name;
}

ISI_STREAM_NAME *isiRawPacketStreamName(ISI_STREAM_NAME *name, ISI_RAW_PACKET *raw, void *ida)
{
    if (name == NULL || raw == NULL) {
        errno = EINVAL;
        return NULL;
    }

    switch (raw->hdr.desc.type) {
      case ISI_TYPE_IDA5:
      case ISI_TYPE_IDA6:
      case ISI_TYPE_IDA7:
      case ISI_TYPE_IDA8:
      case ISI_TYPE_IDA9:
        if (ida == NULL) return NULL;
        return GetIdaStreamName(name, raw, (IDA *) ida);
      case ISI_TYPE_IDA10:
        return GetIda10StreamName(name, raw);
      case ISI_TYPE_MSEED:
        return GetMseedStreamName(name, raw);
    }
    return NULL;
}

#endif /* ISI_SERVER */

/* Revision History
 *
 * $Log: utils.c,v $
 * Revision 1.16  2008/02/03 17:20:17  dechavez
 * ensure isiRawPacketStreamName() returns NULL for non-data packets
 *
 * Revision 1.15  2008/01/25 21:48:24  dechavez
 * added isiRawPacketStreamName()
 *
 * Revision 1.14  2007/10/31 16:48:55  dechavez
 * replaced string memcpy with strlcpy, sprintf with snprintf
 *
 * Revision 1.13  2007/01/08 15:58:18  dechavez
 * switch to size-bounded string operations
 *
 * Revision 1.12  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.11  2006/02/09 00:09:12  dechavez
 * moved iacp functions into newutil.c, renamed 'oldchn' field of ISI_STREAM_NAME to 'chnloc'
 *
 * Revision 1.10  2005/06/30 01:25:26  dechavez
 * fixed isiExpandSeqnoSiteSpecifier()
 *
 * Revision 1.9  2005/06/24 21:30:18  dechavez
 * added isiSiteNameCompareWild(), isiExpandSeqnoSiteSpecifier()
 *
 * Revision 1.8  2005/05/06 00:52:18  dechavez
 * added isiGetIacpStats() and isiExpandStreamNameSpecifier()
 *
 * Revision 1.7  2004/06/24 17:47:50  dechavez
 * accept both NULL and blank loc in isiStaChnLocToStreamName()
 *
 * Revision 1.6  2004/06/21 19:45:22  dechavez
 * use blank loc in isiStaChnLocToStreamName()
 *
 * Revision 1.5  2004/06/10 17:15:30  dechavez
 * added isiSetFlag() and isiGetFlag()
 *
 * Revision 1.4  2003/11/19 23:45:40  dechavez
 * made isiStaChnToStreamName a macro (in isi.h)
 *
 * Revision 1.3  2003/11/08 00:40:07  dechavez
 * cleaned up comments
 *
 * Revision 1.2  2003/11/03 23:12:11  dechavez
 * added isiInitIncoming() and isiResetIncoming()
 *
 * Revision 1.1  2003/10/16 15:38:52  dechavez
 * Initial release
 *
 */
