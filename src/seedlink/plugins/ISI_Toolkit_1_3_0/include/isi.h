#pragma ident "$Id: isi.h,v 1.47 2008/01/25 21:43:09 dechavez Exp $"
/*======================================================================
 *
 *  Ida System Interface library
 *
 *====================================================================*/
#ifndef isi_h_included
#define isi_h_included

#include "platform.h"
#include "iacp.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IACP payload type codes (1000 - 1999) */

#define ISI_IACP_MIN          IACP_TYPE_ISI_MIN
#define ISI_IACP_REQ_SOH      ISI_IACP_MIN +  1 /* state of health request */
#define ISI_IACP_REQ_CNF      ISI_IACP_MIN +  2 /* configuration request */
#define ISI_IACP_REQ_WFDISC   ISI_IACP_MIN +  3 /* wfdisc request */
#define ISI_IACP_REQ_FORMAT   ISI_IACP_MIN +  4 /* format part of data request */
#define ISI_IACP_REQ_COMPRESS ISI_IACP_MIN +  5 /* compress part of data request */
#define ISI_IACP_REQ_POLICY   ISI_IACP_MIN +  6 /* policy part of data request */
#define ISI_IACP_REQ_TWIND    ISI_IACP_MIN +  7 /* time window part of data request */
#define ISI_IACP_SYSTEM_SOH   ISI_IACP_MIN +  8 /* system SOH data */
#define ISI_IACP_STREAM_SOH   ISI_IACP_MIN +  9 /* stream SOH data */
#define ISI_IACP_WFDISC       ISI_IACP_MIN + 10 /* wfdisc record */
#define ISI_IACP_STREAM_CNF   ISI_IACP_MIN + 11 /* configuration data */
#define ISI_IACP_GENERIC_TS   ISI_IACP_MIN + 12 /* ISI generic time series packet */
#define ISI_IACP_RAW_PKT      ISI_IACP_MIN + 13 /* raw digitizer packet of arbitrary type */
#define ISI_IACP_REQ_SEQNO    ISI_IACP_MIN + 14 /* sequence number part of a data request */
#define ISI_IACP_REQ_OPTIONS  ISI_IACP_MIN + 15 /* data request options */
#define ISI_IACP_REQ_STREAM   ISI_IACP_MIN + 16 /* stream name, optional for seqno requests */
#define ISI_IACP_MAX          IACP_TYPE_ISI_MAX

/* Wildcards for time specifications */

#define ISI_UNDEFINED_TIMESTAMP ((REAL64) -1.0)
#define ISI_OLDEST              ((REAL64) -2.0)
#define ISI_NEWEST              ((REAL64) -3.0)
#define ISI_KEEPUP              ((REAL64) -4.0)

/* Wildcards for seqno specifications */

#define ISI_OLDEST_SEQNO_SIG       ((UINT32) (0xFFFFFFFFL))
#define ISI_NEWEST_SEQNO_SIG       ((UINT32) (0xFFFFFFFEL))
#define ISI_KEEPUP_SEQNO_SIG       ((UINT32) (0xFFFFFFFDL))
#define ISI_NEVER_SEQNO_SIG        ISI_KEEPUP_SEQNO_SIG
#define ISI_BEG_RELATIVE_SEQNO_SIG ((UINT32) (0xFFFFFFFCL))
#define ISI_END_RELATIVE_SEQNO_SIG ((UINT32) (0xFFFFFFFBL))
#define ISI_CURRENT_SEQNO_SIG      ((UINT32) (0xFFFFFFFAL))
#define ISI_LARGEST_SEQNO_SIG      ((UINT32) (0xFFFFFFF0L))

#define ISI_OLDEST_SEQNO_STRING    "oldest"
#define ISI_NEWEST_SEQNO_STRING    "newest"
#define ISI_YNGEST_SEQNO_STRING    ISI_NEWEST_SEQNO_STRING
#define ISI_KEEPUP_SEQNO_STRING    "keepup"
#define ISI_NEVER_SEQNO_STRING     ISI_KEEPUP_SEQNO_STRING

#define ISI_UNDEF_SEQNO_SIG ((UINT32) (0x00000000L))

#define ISI_LARGEST_COUNTER ((UINT32) (0xFFFFFFFFL))

#define isiIsOldestSeqno(seqno) ((seqno)->signature == ISI_OLDEST_SEQNO_SIG ? TRUE : FALSE)

#define isiIsNewestSeqno(seqno) ((seqno)->signature == ISI_NEWEST_SEQNO_SIG ? TRUE : FALSE)

#define isiIsKeepupSeqno(seqno) ((seqno)->signature == ISI_KEEPUP_SEQNO_SIG ? TRUE : FALSE)

#define isiIsNeverSeqno(seqno) ((seqno)->signature == ISI_NEVER_SEQNO_SIG ? TRUE : FALSE)
#define isiIsBegRelativeSeqno(seqno) ((seqno)->signature == ISI_BEG_RELATIVE_SEQNO_SIG ? TRUE : FALSE)
#define isiIsEndRelativeSeqno(seqno) ((seqno)->signature == ISI_END_RELATIVE_SEQNO_SIG ? TRUE : FALSE)
#define isiIsLargestSeqno(seqno) ((seqno)->signature == ISI_LARGEST_SEQNO_SIG && (seqno)->counter == ISI_LARGEST_COUNTER ? TRUE : FALSE)
#define isiIsUndefinedSeqno(seqno) ((seqno)->signature == ISI_UNDEF_SEQNO_SIG ? TRUE : FALSE)
#define isiIsAbsoluteSeqno(seqno) ((seqno)->signature > ISI_UNDEF_SEQNO_SIG && (seqno)->signature <= ISI_LARGEST_SEQNO_SIG ? TRUE : FALSE)

/* String lengths */

#define ISI_STALEN   7
#define ISI_CHNLEN   3
#define ISI_LOCLEN   2
#define ISI_SITELEN  ISI_STALEN
#define ISI_CHNLOCLEN (ISI_CHNLEN+ISI_LOCLEN)
#define ISI_INAMLEN 7
#define ISI_CNAMLEN 7
#define ISI_STREAM_NAME_LEN (ISI_STALEN + 1 + ISI_CHNLEN + 1 + ISI_LOCLEN)
#define ISI_SEQNO_STRING_LEN 24
#define ISI_SEQ_NO_STRING_LEN (ISI_SEQNO_STRING_LEN + 1)

#define ISI_STA_CHN_LOC_DELIMITERS ".,:;/\\"

#define ISI_BLANK_LOC "  "

/* Reconnect policy codes */

#define ISI_RECONNECT_POLICY_UNDEF     0
#define ISI_RECONNECT_POLICY_NONE      1
#define ISI_RECONNECT_POLICY_MIN_GAP   2
#define ISI_RECONNECT_POLICY_MIN_DELAY 3

#define ISI_SHUTDOWN_DELAY 5000 /* 5 seconds between isiClose() and isiOpen() */

/* isiReadGenericTS() and isiReadRawPacket() return codes */

#define ISI_OK        0
#define ISI_DONE      1
#define ISI_ERROR     2
#define ISI_BREAK     3
#define ISI_TIMEDOUT  4
#define ISI_CONNRESET 5
#define ISI_EINVAL    6
#define ISI_BADMSG    7

/* Format codes */

#define ISI_FORMAT_UNDEF   0
#define ISI_FORMAT_GENERIC 1
#define ISI_FORMAT_NATIVE  2

/* misc data types */

typedef struct {
    UINT32 signature;
    UINT64 counter;
} ISI_SEQNO;

typedef struct {
    ISI_SEQNO beg;
    ISI_SEQNO end;
} ISI_SEQNO_RANGE;

typedef struct {
    char sta[ISI_STALEN+1];
    char chn[ISI_CHNLEN+1];
    char loc[ISI_LOCLEN+1];
    char chnloc[ISI_CHNLOCLEN+1];
} ISI_STREAM_NAME;

#define ISI_NAME_WILDCARD "*"

typedef struct {
    INT16 factor;
    INT16 multiplier;
    REAL64 value;
} ISI_SRATE;

#define ISI_TSTAMP_STATUS_LOCKED  0x0001 /* set when clock is deemed locked */
#define ISI_TSTAMP_STATUS_AUTOINC 0x0002 /* set if digitizer auto-incremented clock time tag */
#define ISI_TSTAMP_STATUS_NOINIT  0x0004 /* set if digitizer has never seen a valid time */
#define ISI_TSTAMP_STATUS_SUSPECT 0x0008 /* set if digitizer thinks time might be bogus */
#define ISI_TSTAMP_STATUS_DERIVED 0x0010 /* set if application derived this via interpolation */
#define ISI_TSTAMP_STATUS_UNKNOWN 0x8000 /* no status information is available */

#define ISI_TSTAMP_INVALID (ISI_TSTAMP_STATUS_NOINIT | ISI_TSTAMP_STATUS_SUSPECT)

#define isiIsValidTimestamp(ptstamp) ((pstamp->status & ISI_TIMESTAMP_INVALID) ? FALSE : TRUE)

typedef struct {
    REAL64 value;
    UINT16 status;
} ISI_TSTAMP;

typedef struct {
    REAL32 lat;
    REAL32 lon;
    REAL32 elev;
    REAL32 depth;
} ISI_COORDS;

typedef struct {
    ISI_STREAM_NAME name; /* stream name */
    ISI_TSTAMP      tofs; /* time of oldest datum */
    ISI_TSTAMP      tols; /* time of youngest datum in disk loop */
    REAL64          tslw; /* time since last write to disk loop */
    UINT32          nseg; /* number of segments (gaps) in disk loop */
    UINT32          nrec; /* number of records received */
} ISI_STREAM_SOH;

typedef struct {
    REAL32  calib;
    REAL32  calper;
    REAL32  hang;
    REAL32  vang;
    char    type[ISI_INAMLEN+1];
} ISI_INST;

#define ISI_UNDEFINED_INST { (REAL32) 0.0, (REAL32) -1.0, (REAL32) -999.9, (REAL32) -999.9, "-" }

typedef struct {
    ISI_STREAM_NAME name;
    ISI_SRATE       srate;
    ISI_COORDS      coords;
    ISI_INST        inst;
} ISI_STREAM_CNF;

/* packet data request types */

#define ISI_REQUEST_TYPE_UNDEF 0
#define ISI_REQUEST_TYPE_SEQNO 1
#define ISI_REQUEST_TYPE_TWIND 2

/* compression codes */

#define ISI_COMP_UNDEF  0
#define ISI_COMP_NONE   1
#define ISI_COMP_IDA    2
#define ISI_COMP_STEIM1 3
#define ISI_COMP_STEIM2 4
#define ISI_COMP_GZIP   5

/* uncompressed packet data type codes */

#define ISI_TYPE_UNDEF  0
#define ISI_TYPE_INT8   1
#define ISI_TYPE_INT16  2
#define ISI_TYPE_INT32  3
#define ISI_TYPE_INT64  4
#define ISI_TYPE_REAL32 5
#define ISI_TYPE_REAL64 6
#define ISI_TYPE_NATIVE 10
#define ISI_TYPE_IDA8   (ISI_TYPE_NATIVE + 1)
#define ISI_TYPE_IDA10  (ISI_TYPE_NATIVE + 2)
#define ISI_TYPE_IDA6   (ISI_TYPE_NATIVE + 3)
#define ISI_TYPE_IDA7   (ISI_TYPE_NATIVE + 4)
#define ISI_TYPE_IDA9   (ISI_TYPE_NATIVE + 5)
#define ISI_TYPE_IDA5   (ISI_TYPE_NATIVE + 6)
#define ISI_TYPE_QDPLUS (ISI_TYPE_NATIVE + 7) /* native QDP plus unit id header */
#define ISI_TYPE_MSEED  (ISI_TYPE_NATIVE + 8) /* mini-SEED */

/* byte order codes */

#define ISI_ORDER_UNDEF     (UINT8) -1
#define ISI_ORDER_LTLENDIAN (UINT8) LTL_ENDIAN_BYTE_ORDER
#define ISI_ORDER_BIGENDIAN (UINT8) BIG_ENDIAN_BYTE_ORDER

/* datum description */

typedef struct {
    UINT8 comp;  /* ISI_COMP_x, defined above */
    UINT8 type;  /* ISI_TYPE_x, defined above */
    UINT8 order; /* ISI_ORDER_x, defined above */
    UINT8 size;  /* uncompressed data sample size in bytes */
} ISI_DATA_DESC;

#ifdef INCLUDE_STATIC_DATA_DESC
static ISI_DATA_DESC ISI_DEFAULT_DATA_DESC = { ISI_COMP_UNDEF, ISI_TYPE_UNDEF, ISI_ORDER_UNDEF, 0 };
#endif

/* type tags for self describing structures */

#define ISI_TAG_EOF        0
#define ISI_TAG_SITE_NAME  1
#define ISI_TAG_SEQNO      2
#define ISI_TAG_DATA_DESC  3
#define ISI_TAG_LEN_USED   4
#define ISI_TAG_LEN_NATIVE 5
#define ISI_TAG_PAYLOAD    6
#define ISI_TAG_RAW_STATUS 7

#define ISI_RAW_STATUS_OK      0
#define ISI_RAW_STATUS_SUSPECT 1

/* raw digitizer packets (this can evolve via the use of type tags) */

typedef struct {
    char site[ISI_SITELEN+1]; /* source site name */
    ISI_SEQNO seqno;          /* source sequence number */
    ISI_DATA_DESC desc;       /* payload descriptor */
    UINT32 status;            /* unpack status, etc */
    struct {
        UINT32 payload;       /* number of bytes _allocated_ to payload */
        UINT32 used;          /* number of bytes _used_ in payload */
        UINT32 native;        /* length payload should decompress to */
    } len;
} ISI_RAW_HEADER;

typedef struct {
    ISI_RAW_HEADER hdr; /* packet header */
    UINT8 *payload;     /* points to nbytes bytes of data described by desc */
} ISI_RAW_PACKET;

/* values for the ISI_TWIND_REQUEST status field */

#define ISI_COMPLETE   1
#define ISI_INCOMPLETE 2

/* Sequence number request */

typedef struct {
    char site[ISI_SITELEN+1]; /* source site name */
    ISI_SEQNO beg;            /* starting sequence number */
    ISI_SEQNO end;            /* ending sequence number */
    int status;               /* internal use only, never sent over the wire */
} ISI_SEQNO_REQUEST;

/* Time window request */

typedef struct {
    ISI_STREAM_NAME name;
    REAL64 beg;
    REAL64 end;
    int status; /* internal use only, never sent over the wire */
} ISI_TWIND_REQUEST;

/* Data request options */

#define ISI_DEFAULT_OPTIONS           0x00000000

/* Data request */

typedef struct {
    UINT32 type;       /* request type (seqno, stream) */
    UINT32 policy;     /* reconnect policy */
    UINT32 format;     /* delivery format */
    UINT32 compress;   /* telemetry compression */
    UINT32 options;    /* other options (nothing currently defined) */
    LNKLST slist;      /* optional list of desired streams (used only by seqno requests) */
    UINT32 nreq;       /* number of request entries to follow */
    struct {
        ISI_TWIND_REQUEST *twind;
        ISI_SEQNO_REQUEST *seqno;
    } req;
} ISI_DATA_REQUEST;

/* All the stuff needed to establish a connection on one handy structure */

typedef struct {
    int port;
    IACP_ATTR attr;
    int debug;
    LOGIO *lp;
    LOGIO logio;
    BOOL decompress;
} ISI_PARAM;

/* reports */

typedef struct {
    UINT32 nentry;
    ISI_STREAM_SOH *entry;
} ISI_SOH_REPORT;

typedef struct {
    UINT32 nentry;
    ISI_STREAM_CNF *entry;
} ISI_CNF_REPORT;

typedef struct {
    UINT32 nentry;
    char **entry;
} ISI_WFDISC_REPORT;

/* channel status for compatibility with CD1.1 */

typedef struct {
    UINT8  data;
    UINT8  security;
    UINT8  misc;
    UINT8  voltage;
    REAL64 sync;
    UINT32 diff;
} ISI_CD_STATUS;

/* generic time series packet */

typedef struct {
    ISI_STREAM_NAME name;   /* sta.chn.loc */
    ISI_SRATE       srate;  /* sample rate */
    ISI_TSTAMP      tofs;   /* time of first sample */
    ISI_TSTAMP      tols;   /* time of last sample */
    ISI_CD_STATUS   status; /* CD1.1 channel status data */
    UINT32          nsamp;  /* number of samples */
    ISI_DATA_DESC   desc;   /* datum description */
    UINT32          nbytes; /* number of data bytes */
} ISI_GENERIC_TSHDR;

typedef struct {
    ISI_GENERIC_TSHDR hdr;
    VOID  *data;
    UINT32 nalloc;  /* number of bytes allocated to data */
    BOOL precious;  /* if true then the structure should not be deleted */
} ISI_GENERIC_TS;

/* used to store incoming multi-part messages */

typedef struct {
    UINT32 type;
    LNKLST list;
    SEMAPHORE semaphore;
} ISI_INCOMING;

/* handle for simplified interface */

#define ISI_INTERNAL_BUFLEN 32768

typedef struct {
    char *server;
    int port;
    IACP_ATTR attr;
    LOGIO *lp;
    int debug;
    IACP *iacp;
    UINT8 buf[ISI_INTERNAL_BUFLEN];
    IACP_FRAME frame;
    ISI_DATA_REQUEST datreq;
    ISI_INCOMING incoming;
    ISI_GENERIC_TS *ts1;
    ISI_GENERIC_TS *ts2;
    BOOL decompress;
#define ISI_FLAG_NOP    0x00
#define ISI_FLAG_BREAK  0x01
    UINT8 flag;  /* used to pass events from the low level I/O routines */
    UINT32 alert; /* IACP code sent following a server alert */
} ISI;

/* Macros */

#define isiIsNative(desc) ((desc)->type > ISI_TYPE_NATIVE)
#define isiStaChnToStreamName(sta, chn, name) isiStaChnLocToStreamName(sta, chn, "  ", name)

#ifdef LTL_ENDIAN_HOST
#define ISI_HOST_BYTE_ORDER ISI_ORDER_LTLENDIAN
#else
#define ISI_HOST_BYTE_ORDER ISI_ORDER_BIGENDIAN
#endif

/* Defaults */

#define ISI_DEFAULT_PORT             39136
#define ISI_DEFAULT_HOST             "localhost"
#define ISI_DEFAULT_REQUEST_TYPE     ISI_REQUEST_TYPE_UNDEF
#define ISI_DEFAULT_RECONNECT_POLICY ISI_RECONNECT_POLICY_MIN_GAP
#define ISI_DEFAULT_FORMAT           ISI_FORMAT_GENERIC
#define ISI_DEFAULT_STREAMSPEC       "*.*.*"
#define ISI_DEFAULT_SITESPEC         "*"
#define ISI_DEFAULT_SEQNOSPEC        "*"
#define ISI_DEFAULT_COMP             ISI_COMP_GZIP
#define ISI_DEFAULT_BEGTIME          ISI_NEWEST
#define ISI_DEFAULT_ENDTIME          ISI_KEEPUP
#define ISI_DEFAULT_BEG_SEQNO_SIG    ISI_NEWEST_SEQNO_SIG
#define ISI_DEFAULT_END_SEQNO_SIG    ISI_KEEPUP_SEQNO_SIG

#ifdef INCLUDE_ISI_STATIC_SEQNOS
static ISI_SEQNO ISI_DEFAULT_BEG_SEQNO = { ISI_DEFAULT_BEG_SEQNO_SIG, 0 };
static ISI_SEQNO ISI_DEFAULT_END_SEQNO = { ISI_DEFAULT_END_SEQNO_SIG, 0 };
static ISI_SEQNO ISI_UNDEFINED_SEQNO   = { ISI_UNDEF_SEQNO_SIG,       0 };
static ISI_SEQNO ISI_OLDEST_SEQNO      = { ISI_OLDEST_SEQNO_SIG,      0 };
static ISI_SEQNO ISI_NEWEST_SEQNO      = { ISI_NEWEST_SEQNO_SIG,      0 };
static ISI_SEQNO ISI_KEEPUP_SEQNO      = { ISI_KEEPUP_SEQNO_SIG,      0 };
static ISI_SEQNO ISI_NEVER_SEQNO       = { ISI_NEVER_SEQNO_SIG,      0 };
static ISI_SEQNO ISI_LARGEST_SEQNO     = { ISI_LARGEST_SEQNO_SIG, ISI_LARGEST_COUNTER };
#endif /* INCLUDE_ISI_DEFAULT_SEQNO */

typedef struct {
    char sta[ISI_STALEN+1];
    char chan[ISI_CNAMLEN+1];
    int status;
} ISI_SITECHANFLAG;

typedef struct {
    int sf_private;
} ISI_SYS_FLAGS;

typedef struct {
    char sta[ISI_STALEN+1];
    ISI_SYS_FLAGS flags;
} ISI_SYSTEM;

/* Function prototypes */

/* cnf.c */
char *isiStreamCnfString(ISI_STREAM_CNF *cnf, char *buf);
VOID isiPrintStreamCnf(FILE *fp, ISI_STREAM_CNF *cnf);
BOOL isiIacpSendStreamCnfList(IACP *iacp, LNKLST *list);
BOOL isiIacpRecvStreamCnfList(IACP *iacp, LNKLST *list);
LNKLST *isiRequestCnf(ISI *isi);

/* datreq.c */
VOID isiPrintReqTime(FILE *fp, REAL64 value);
VOID isiPrintTwindReq(FILE *fp, ISI_TWIND_REQUEST *req);
VOID isiPrintSeqnoReq(FILE *fp, ISI_SEQNO_REQUEST *req);
VOID isiPrintDatreq(FILE *fp, ISI_DATA_REQUEST *datreq);
VOID isiLogDatreq(LOGIO *lp, int level, ISI_DATA_REQUEST *datreq);
VOID isiSetDatreqType(ISI_DATA_REQUEST *datreq, UINT32 value);
VOID isiSetDatreqPolicy(ISI_DATA_REQUEST *datreq, UINT32 value);
VOID isiSetDatreqFormat(ISI_DATA_REQUEST *datreq, UINT32 value);
VOID isiSetDatreqCompress(ISI_DATA_REQUEST *datreq, UINT32 value);
UINT32 isiGetDatreqCompress(ISI_DATA_REQUEST *datreq);
VOID isiSetDatreqOptions(ISI_DATA_REQUEST *datreq, UINT32 value);
UINT32 isiGetDatreqOptions(ISI_DATA_REQUEST *datreq);
VOID isiInitDataRequest(ISI_DATA_REQUEST *req);
VOID isiClearDataRequest(ISI_DATA_REQUEST *req);
ISI_TWIND_REQUEST *isiLocateTwindRequest(ISI_STREAM_NAME *target, ISI_DATA_REQUEST *datreq);
ISI_SEQNO_REQUEST *isiLocateSeqnoRequest(char *target, ISI_DATA_REQUEST *datreq);
BOOL isiIacpSendDataRequest(IACP *iacp, ISI_DATA_REQUEST *datreq);
UINT32 isiAppendTwindReq(ISI_INCOMING *incoming, IACP_FRAME *frame);
UINT32 isiAppendSeqnoReq(ISI_INCOMING *incoming, IACP_FRAME *frame);
UINT32 isiAppendSeqnoStreamName(ISI_DATA_REQUEST *datreq, IACP_FRAME *frame);
BOOL isiCopyStreamListToDataRequest(LNKLST *list, ISI_DATA_REQUEST *output);
BOOL isiCopySeqnoListToDataRequest(LNKLST *list, ISI_DATA_REQUEST *output);
BOOL isiSendDataRequest(ISI *isi, ISI_DATA_REQUEST *datreq, UINT32 *error);
ISI_DATA_REQUEST *isiFreeDataRequest(ISI_DATA_REQUEST *datreq);
ISI_DATA_REQUEST *isiAllocSimpleDataRequest(REAL64 beg, REAL64 end, char *spec);
ISI_DATA_REQUEST *isiAllocSimpleSeqnoRequest(ISI_SEQNO *beg, ISI_SEQNO *end, char *spec);
ISI_DATA_REQUEST *isiAllocDefaultDataRequest(VOID);
ISI_DATA_REQUEST *isiAllocDefaultSeqnoRequest(VOID);

/* ezio.c */
BOOL isiInitDefaultPar(ISI_PARAM *par);
ISI_PARAM *isiAllocDefaultParam(VOID);
VOID isiSetServerPort(ISI_PARAM *par, int port);
int isiGetServerPort(ISI_PARAM *par);
VOID isiSetTimeout(ISI_PARAM *par, int value);
VOID isiSetTcpBuflen(ISI_PARAM *par, int sndbuf, int rcvbuf);
VOID isiSetRetryFlag(ISI_PARAM *par, BOOL value);
VOID isiSetRetryInterval(ISI_PARAM *par, int value);
VOID isiSetDebugFlag(ISI_PARAM *par, int value);
BOOL isiSetDbgpath(ISI_PARAM *par, char *value);
BOOL isiStartLogging(ISI_PARAM *par, char *spec, VOID(*func) (char *string), char *pname);
BOOL isiSetLog(ISI_PARAM *par, LOGIO *lp);
ISI *isiInitiateDataRequest(char *server, ISI_PARAM *par, ISI_DATA_REQUEST *dreq);
ISI_SOH_REPORT *isiSoh(char *server, ISI_PARAM *par);
VOID isiFreeSoh(ISI_SOH_REPORT *report);
ISI_CNF_REPORT *isiCnf(char *server, ISI_PARAM *par);
VOID isiFreeCnf(ISI_CNF_REPORT *report);
ISI_WFDISC_REPORT *isiWfdisc(char *server, ISI_PARAM *par, int maxdur);
VOID isiFreeWfdisc(ISI_WFDISC_REPORT *report);

/* log.c */
VOID isiLogMsgLevel(ISI *isi, int level);
VOID isiLogMsg(ISI *isi, int level, char *format, ...);

/* netutil.c */
VOID isiInitIncoming(ISI_INCOMING *incoming);
VOID isiResetIncoming(ISI_INCOMING *incoming);
VOID isiSetFlag(ISI *isi, UINT8 value);
UINT8 isiGetFlag(ISI *isi);
BOOL isiGetIacpStats(ISI *isi, IACP_STATS *send, IACP_STATS *recv);

/* open.c */
ISI *isiOpen(char *server, int port, IACP_ATTR *user_attr, LOGIO *lp, int debug);
ISI *isiFree(ISI *isi);
ISI *isiClose(ISI *isi);

/* pack.c */
int isiPackCoords(UINT8 *start, ISI_COORDS *src);
int isiUnpackCoords(UINT8 *start, ISI_COORDS *dest);
int isiPackInst(UINT8 *start, ISI_INST *src);
int isiUnpackInst(UINT8 *start, ISI_INST *dest);
int isiPackDatumDescription(UINT8 *start, ISI_DATA_DESC *src);
int isiUnpackDatumDescription(UINT8 *start, ISI_DATA_DESC *dest);
int isiPackSrate(UINT8 *start, ISI_SRATE *src);
int isiUnpackSrate(UINT8 *start, ISI_SRATE *dest);
int isiPackTstamp(UINT8 *start, ISI_TSTAMP *src);
int isiUnpackTstamp(UINT8 *start, ISI_TSTAMP *dest);
int isiPackStreamName(UINT8 *start, ISI_STREAM_NAME *src);
int isiUnpackStreamName(UINT8 *start, ISI_STREAM_NAME *dest);
int isiPackSeqnoRequest(UINT8 *start, ISI_SEQNO_REQUEST *src);
int isiUnpackSeqnoRequest(UINT8 *start, ISI_SEQNO_REQUEST *dest);
int isiPackTwindRequest(UINT8 *start, ISI_TWIND_REQUEST *src);
int isiUnpackTwindRequest(UINT8 *start, ISI_TWIND_REQUEST *src);
int isiPackStreamCnf(UINT8 *start, ISI_STREAM_CNF *src);
int isiUnpackStreamCnf(UINT8 *start, ISI_STREAM_CNF *dest);
int isiPackStreamSoh(UINT8 *start, ISI_STREAM_SOH *src);
int isiUnpackStreamSoh(UINT8 *start, ISI_STREAM_SOH *dest);
int isiPackCdstatus(UINT8 *start, ISI_CD_STATUS *src);
int isiPackSeqno(UINT8 *start, ISI_SEQNO *src);
int isiUnpackSeqno(UINT8 *start, ISI_SEQNO *dest);
int isiUnpackCdstatus(UINT8 *start, ISI_CD_STATUS *dest);
int isiPackGenericTSHDR(UINT8 *start, ISI_GENERIC_TSHDR *src);
int isiUnpackGenericTSHDR(UINT8 *start, ISI_GENERIC_TSHDR *dest);
int isiUnpackGenericTS(UINT8 *start, ISI_GENERIC_TS *dest);
int isiPackRawPacket(UINT8 *start, ISI_RAW_PACKET *src);
void isiUnpackRawPacket(UINT8 *start, ISI_RAW_PACKET *dest);

/* raw.c */
BOOL isiInitRawHeader(ISI_RAW_HEADER *hdr);
BOOL isiInitRawPacket(ISI_RAW_PACKET *raw, VOID *buf, UINT32 buflen);
BOOL isiDecompressRawPacket(ISI_RAW_PACKET *raw, UINT8 *buf, UINT32 buflen);
BOOL isiCompressRawPacket(ISI_RAW_PACKET *raw, UINT8 *buf, UINT32 buflen);
void isiDestroyRawPacket(ISI_RAW_PACKET *raw);
ISI_RAW_PACKET *isiAllocateRawPacket(UINT32 buflen);

/* read.c */
int isiReadFrame(ISI *isi, BOOL SkipHeartbeats);

/* seqno.c */
VOID isiPrintSeqno(FILE *fp, ISI_SEQNO *seqno);
BOOL isiStringToSeqno(char *string, ISI_SEQNO *seqno);
BOOL isiSeqnoEQ(ISI_SEQNO *a, ISI_SEQNO *b);
BOOL isiSeqnoLT(ISI_SEQNO *a, ISI_SEQNO *b);
BOOL isiSeqnoLE(ISI_SEQNO *a, ISI_SEQNO *b);
BOOL isiSeqnoGT(ISI_SEQNO *a, ISI_SEQNO *b);
BOOL isiSeqnoGE(ISI_SEQNO *a, ISI_SEQNO *b);
int isiCompareSeqno(ISI_SEQNO *a, ISI_SEQNO *b);
ISI_SEQNO isiIncrSeqno(ISI_SEQNO *seqno);
BOOL isiWriteSeqno(FILE *fp, ISI_SEQNO *seqno);
BOOL isiReadSeqno(FILE *fp, ISI_SEQNO *seqno);

/* soh.c */
char *isiStreamSohString(ISI_STREAM_SOH *soh, char *buf);
VOID isiPrintStreamSoh(FILE *fp, ISI_STREAM_SOH *soh);
BOOL isiIacpSendStreamSohList(IACP *iacp, LNKLST *list);
BOOL isiIacpRecvStreamSohList(IACP *iacp, LNKLST *list);
LNKLST *isiRequestSoh(ISI *isi);

/* string.c */
char *isiReqStatusString(int value);
char *isiFormatString(int value);
char *isiCompressString(int value);
char *isiRequestTypeString(int value);
char *isiPolicyString(int value);
char *isiDatumTypeString(int value);
BOOL isiStringToStreamName(char *string, ISI_STREAM_NAME *stream);
char *isiStreamNameString(ISI_STREAM_NAME *stream, char *string);
char *isiTstampString(ISI_TSTAMP *tstamp, char *buf);
char *isiDescString(ISI_DATA_DESC *desc, char *buf);
char *isiGenericTsHdrString(ISI_GENERIC_TSHDR *hdr, char *buf);
char *isiRawHeaderString(ISI_RAW_HEADER *hdr, char *buf);
char *isiRawPacketString(ISI_RAW_PACKET *raw, char *buf);
char *isiSeqnoString(ISI_SEQNO *seqno, char *buf);
char *isiTwindRequestString(ISI_TWIND_REQUEST *req, char *buf);
char *isiSeqnoRequestString(ISI_SEQNO_REQUEST *req, char *buf);
char *isiRequestTimeString(REAL64 value, char *buf);
char *isiDatatypeString(ISI_DATA_DESC *desc);

/* ts.c */
VOID isiInitGenericTSHDR(ISI_GENERIC_TSHDR *src);
BOOL isiInitGenericTS(ISI_GENERIC_TS *ts);
ISI_GENERIC_TS *isiCreateGenericTS(VOID);
ISI_GENERIC_TS *isiDestroyGenericTS(ISI_GENERIC_TS *ts);
BOOL isiCopyGenericTS(ISI_GENERIC_TS *src, ISI_GENERIC_TS *dest);
ISI_GENERIC_TS *isiReadGenericTS(ISI *isi, int *status);

/* utils.c */
ISI_SRATE *isiSintToSrate(REAL64 sint, ISI_SRATE *srate);
REAL64 isiSrateToSint(ISI_SRATE *srate);
int isiStreamNameCompare(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b);
int isiStreamNameCompareWild(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b);
BOOL isiStreamNameMatch(ISI_STREAM_NAME *a, ISI_STREAM_NAME *b);
VOID isiStaChnLocToStreamName(char *sta, char *chn, char *loc, ISI_STREAM_NAME *name);
int isiSiteNameCompareWild(char *a, char *b);
LNKLST *isiExpandStreamNameSpecifier(char *StreamSpec);
LNKLST *isiExpandSeqnoSiteSpecifier(char *SiteSpec);
#ifdef ISI_SERVER
ISI_STREAM_NAME *isiRawPacketStreamName(ISI_STREAM_NAME *name, ISI_RAW_PACKET *raw, void *ida);
#endif /* ISI_SERVER */

/* version.c */
char *isiVersionString(VOID);
VERSION *isiVersion(VOID);

/* wfdisc.c */
BOOL isiIacpSendWfdiscList(IACP *iacp, LNKLST *list);
BOOL isiIacpRecvWfdiscList(IACP *iacp, LNKLST *list);
LNKLST *isiRequestWfdisc(ISI *isi, int maxdur);

#ifdef __cplusplus
}
#endif

#endif

/* Revision History
 *
 * $Log: isi.h,v $
 * Revision 1.47  2008/01/25 21:43:09  dechavez
 * added support for ISI_IACP_REQ_OPTIONS and ISI_IACP_REQ_STREAM, added
 * options and slist to ISI_DATA_REQUEST
 *
 * Revision 1.46  2007/06/21 22:47:25  dechavez
 * added 'derived' clock status
 *
 * Revision 1.45  2007/04/18 22:45:13  dechavez
 * added isiWriteSeqno() and isiReadSeqno() prototypes
 *
 * Revision 1.44  2007/01/11 17:46:27  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.43  2007/01/04 22:02:38  dechavez
 * fixed typedf typo
 *
 * Revision 1.42  2007/01/04 21:51:57  dechavez
 * added missing size qualifiers to large constants, introduced ISI_SITECHANFLAG
 *
 * Revision 1.41  2006/12/13 21:51:30  dechavez
 * defined ISI_SEQ_NO_STRING_LEN for the space delimited versions
 *
 * Revision 1.40  2006/12/12 22:37:31  dechavez
 * added "never" sequence number (equivalent to "keepup"), defined macros for
 * the abstract sequence numbers, defined ISI_SEQNO_RANGE datatype
 *
 * Revision 1.39  2006/11/10 06:25:11  dechavez
 * defined (finally) generic time status bits
 *
 * Revision 1.38  2006/09/29 20:02:02  dechavez
 * defined ISI_EINVAL and ISI_BADMSG
 *
 * Revision 1.37  2006/08/30 18:09:28  judy
 * added ISI_SYSTEM and ISI_SYS_FLAGS
 *
 * Revision 1.36  2006/06/26 22:39:40  dechavez
 * Defined ISI_TIMEDOUT and ISI_CONNRESET.  Explicitly ast ISI_UNDEFINED_INST members.
 * Updated prototypes.
 *
 * Revision 1.35  2006/06/19 18:33:16  dechavez
 * defined ISI_TYPE_MSEED
 *
 * Revision 1.34  2006/06/07 22:12:08  dechavez
 * updated prototypes
 *
 * Revision 1.33  2006/05/17 23:20:46  dechavez
 * added IDA5 and QDPLUS type codes
 *
 * Revision 1.32  2006/02/14 17:05:04  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.31  2006/02/09 00:00:28  dechavez
 * changed "oldchn" field in ISI_STREAM_NAME to "chnloc", updated prototypes
 *
 * Revision 1.30  2005/12/09 21:10:57  dechavez
 * removed seqno generation rules (moved to isi/dl.h flags)
 *
 * Revision 1.29  2005/10/26 23:14:54  dechavez
 * updated prototypes
 *
 * Revision 1.28  2005/10/11 22:43:41  dechavez
 * changed ISI_DATA_REQUEST req field to a struct
 *
 * Revision 1.27  2005/10/10 23:46:29  dechavez
 * made byte order macros cross-consistent
 *
 * Revision 1.26  2005/09/10 21:34:05  dechavez
 * updated prototypes
 *
 * Revision 1.25  2005/08/26 18:10:26  dechavez
 * added ISI_TYPE_IDA9 definition
 *
 * Revision 1.24  2005/07/26 00:20:20  dechavez
 * 2.0.0 (split off disk loop I/O stuff to isi/dl.h)
 *
 * Revision 1.23  2005/07/06 15:32:56  dechavez
 * 1.10.2 checkpoint
 *
 * Revision 1.22  2005/06/30 01:33:35  dechavez
 * 1.10.1 checkpoint
 *
 * Revision 1.21  2005/06/24 21:43:20  dechavez
 * version 1.10.0 mods
 *
 * Revision 1.20  2005/06/10 15:21:03  dechavez
 * checkpoint while developing ISI_RAW_PACKET disk loops and protocols
 *
 * Revision 1.19  2005/05/25 22:49:03  dechavez
 * updated prototypes
 *
 * Revision 1.18  2005/05/25 18:26:10  dechavez
 * use HAVE_FLOCK
 *
 * Revision 1.17  2005/05/25 00:26:45  dechavez
 * checkpoint build with working ISI disk loop locks
 *
 * Revision 1.16  2005/05/13 17:20:09  dechavez
 * checkpoint with early disk loop support
 *
 * Revision 1.15  2005/05/06 01:02:20  dechavez
 * 1.7.0
 *
 * Revision 1.14  2005/01/28 01:46:57  dechavez
 * updated prototypes
 *
 * Revision 1.13  2004/06/25 18:34:56  dechavez
 * C++ compatibility
 *
 * Revision 1.12  2004/06/21 19:43:05  dechavez
  defined types for ida rev's 6 and 7, blank loc constant and changed
 * isiStaChnToStreamName() macro to use blank loc instead of NULL
 *
 * Revision 1.11  2004/06/10 17:14:01  dechavez
 * added isiSetFlag() and isiGetFlag()
 *
 * Revision 1.10  2004/01/29 18:31:47  dechavez
 * added more prototypes
 *
 * Revision 1.9  2003/12/04 23:40:28  dechavez
 * renamed isiNativeByteOrder() macro with ISI_HOST_BYTE_ORDER
 *
 * Revision 1.8  2003/11/25 20:33:55  dechavez
 * added new prototypes
 *
 * Revision 1.7  2003/11/19 23:44:14  dechavez
 * updated prototypes, added isiStaChnToStreamName macro
 *
 * Revision 1.6  2003/11/19 21:23:51  dechavez
 * added bookkeeping hooks for buffer managment to ISI_GENERIC_TS, added
 * ISI_GENERIC_TS buffers to the ISI handle, added report structures
 *
 * Revision 1.5  2003/11/13 19:25:29  dechavez
 * updated prototypes
 *
 * Revision 1.4  2003/11/04 19:54:33  dechavez
 * added ISI_SHUTDOWN_DELAY, updated prototypes
 *
 * Revision 1.3  2003/11/04 00:39:02  dechavez
 * updated prototypes
 *
 * Revision 1.2  2003/11/03 23:08:52  dechavez
 * introduced ISI handle, ISI_PARAM structure, added additional prototypes,
 * consolidated defaults
 *
 * Revision 1.1  2003/10/16 15:38:40  dechavez
 * initial release
 *
 */
