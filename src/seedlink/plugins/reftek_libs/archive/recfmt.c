#pragma ident "$Id: recfmt.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Generic recording format functions.
 File     : recfmt.c
 Purpose  : Management of recording format packets.
 Host     : CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
 Target   : Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
 Author	  : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#define _RECFMT_C
#include "archive.h"

/* Local prototypes --------------------------------------------------- */
VOID CopyString(CHAR * dest, CHAR * src, INT16 leng);
VOID GetPacketTime(RF_PACKET * pac, RF_HEADER * hdr);
VOID ucDecode(CHAR * string, UINT8 * val);
REAL64 JSec2MSTime(INT16 year, REAL64 jsec);
CHAR *ListDRTime(UINT32 sec, UINT16 msec);

/*--------------------------------------------------------------------- */
UINT8 RFPacketType(RF_PACKET * pac)
{
    UINT8 i;

    for (i = 1; i <= MAX_RF_PACKET_TYPE; i++) {
        if (PacketCodes[i][0] == pac->hdr.type[0] &&
            PacketCodes[i][1] == pac->hdr.type[1]) {
            return (i);
        }
    }

    return (0);
}

/*--------------------------------------------------------------------- */
BOOL IsDataPacket(UINT8 type)
{

    return (type == EH || type == DT || type == ET);
}

/*--------------------------------------------------------------------- */
UINT16 ExpectedSeqNumber(UINT16 seq)
{

    if (seq >= RF_MAX_SEQ_NO)
        return (0);
    else
        return (seq + 1);

}

/*--------------------------------------------------------------------- */
REAL64 RFPacketTime(RF_PACKET * pac)
{
    INT32 doy, hr, min, sec, msec, year;
    UINT8 buf[32];

    /* Returns packet time stamp as MS_TIME */

    /* Decode day, hour, minute... */
    sscanf(UnBCD(buf, pac->hdr.time, 6), "%3ld%2ld%2ld%2ld%3ld",
        &doy, &hr, &min, &sec, &msec);

    /* Decode year */
    sscanf(UnBCD(buf, &pac->hdr.year, 1), "%2ld", &year);
    if (year < 80)
        year += 2000;
    else
        year += 1900;

    /* Convert to MS_TIME */
    return (EncodeMSTimeDOY(year, doy, hr, min, (REAL64) sec + ((REAL64) msec * 0.001)));
}

/*--------------------------------------------------------------------- */
VOID RFDecodeHeader(RF_PACKET * pac, RF_HEADER * hdr)
{
    UINT8 buf[32];
    int value;

    hdr->type = RFPacketType(pac);

    if (hdr->type == NO_PACKET) {
        memset(hdr, 0, sizeof(RF_HEADER));
        return;
    }

    hdr->time = RFPacketTime(pac);

    Decode(UnBCD(buf, &pac->hdr.exp, 1), "%2d", &value);
    hdr->exp = (UINT16) value;
    hdr->unit = ntohs(pac->hdr.unit);
    Decode(UnBCD(buf, pac->hdr.bpp, 2), "%4d", &value);
    hdr->bytes = (UINT16) value;
    Decode(UnBCD(buf, pac->hdr.packet, 2), "%4d", &value);
    hdr->seq = (UINT16) value;

    hdr->evn_no = VOID_UINT16;
    hdr->stream = VOID_UINT8;
    hdr->channel = VOID_UINT8;
    hdr->length = VOID_UINT16;
    hdr->data_type = VOID_UINT8;

    if (hdr->type == DT || hdr->type == EH ||
        hdr->type == ET || hdr->type == NH || hdr->type == NT) {
        Decode(UnBCD(buf, pac->pac.dt.event_num, 2), "%4d", &value);
        hdr->evn_no = (UINT16) value;
        Decode(UnBCD(buf, &pac->pac.dt.stream, 1), "%2d", &value);
        hdr->stream = (UINT8) value + 1;
    }

    if (hdr->type == DT) {
        Decode(UnBCD(buf, &pac->pac.dt.channel, 1), "%2d", &value);
        hdr->channel = (UINT8) value + 1;
        Decode(UnBCD(buf, pac->pac.dt.len, 2), "%4d", &value);
        hdr->length = (UINT16) value;
        if (pac->pac.dt.data_type == 0xC0)
            hdr->data_type = DT_COMP;
        else if (pac->pac.dt.data_type == 0x32)
            hdr->data_type = DT_32BIT;
        else if (pac->pac.dt.data_type == 0x16)
            hdr->data_type = DT_16BIT;
    }

    return;
}

/*--------------------------------------------------------------------- */
VOID CopyString(CHAR * dest, CHAR * src, INT16 leng)
{
    INT16 i;

    strncpy(dest, src, leng);

    dest[leng - 1] = '\0';
    for (i = leng - 2; i >= 0; i--) {
        if (isalnum(dest[i]))
            break;
        dest[i] = '\0';
    }

    return;
}

/*--------------------------------------------------------------------- */
CHAR *UnBCD(BCD * dest, BCD * source, INT16 sbytes)
{
    INT16 i;

    for (i = 0; i < sbytes; i++) {
        dest[i * 2] = (source[i] >> 4) + 0x30;
        dest[i * 2 + 1] = (source[i] & 0x0F) + 0x30;
    }

    return ((CHAR *) dest);
}

/*--------------------------------------------------------------------- */
/* Convert year and julian seconds (seconds of year) to mstime */
/*--------------------------------------------------------------------- */
REAL64 JSec2MSTime(INT16 year, REAL64 jsec)
{
    INT32 doy, hour, minute;

    /* Day of year */
    doy = (INT32) (jsec / DAY) + 1;
    jsec -= (REAL64) doy *DAY;

    /* Hours */
    hour = (INT16) (jsec / HOUR);
    jsec -= (REAL64) hour *HOUR;

    /* Minutes */
    minute = (INT16) (jsec / MINUTE);
    jsec -= (REAL64) minute *MINUTE;

    /* MSTIME */
    return (EncodeMSTimeDOY(year, doy, hour, minute, jsec));
}

/* -------------------------------------------------------------------- */
VOID Decode(CHAR * string, CHAR * fmt, VOID * val)
{
    CHAR buf[128];
    int wid;

    /* Scan unterminated ASCII field to numeric value */
    /* Must provide width in format, e.g., "%4d". */

    sscanf(fmt, "%%%d", &wid);
    memset(buf, '\0', 128);

    strncpy(buf, string, wid);
    sscanf(buf, fmt, val);

    return;
}

/* -------------------------------------------------------------------- */
VOID ucDecode(CHAR * string, UINT8 * val)
{
    CHAR buf[3];
    UINT16 temp;

    /* Scan unterminated ASCII field to unsigned char value */

    strncpy(buf, string, 2);
    buf[2] = '\0';
    sscanf(buf, "%u", &temp);
    *val = (UINT8) temp;

    return;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:53:22  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
