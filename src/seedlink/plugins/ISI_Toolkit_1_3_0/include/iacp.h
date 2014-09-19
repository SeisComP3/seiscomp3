#pragma ident "$Id: iacp.h,v 1.13 2007/01/11 17:46:49 dechavez Exp $"
#ifndef iacp_include_defined
#define iacp_include_defined

/* platform specific stuff */

#include "platform.h"
#include "logio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IACP_TAG "IACP"
#define IACP_TAGLEN strlen(IACP_TAG)

/* "null" subframe type */

#define IACP_TYPE_NULL                     0 /* null parameter */

/* 1 - 99: Handshake related sub-frames */
#define IACP_TYPE_HANDSHAKE                1 /* handshake */
#define IACP_TYPE_PID                      2 /* peer process id */
#define IACP_TYPE_TO                       3 /* i/o timeout */
#define IACP_TYPE_SNDSIZ                   4 /* TCP/IP send buffer len */
#define IACP_TYPE_RCVSIZ                   5 /* TCP/IP receive buffer len */

/* 100 - 199: General IACP I/O (post-handshake) related sub-frames */
#define IACP_TYPE_ALERT                  100 /* peer disconnect notification */
#define IACP_TYPE_NOP                    101 /* i/o heartbeat */
#define IACP_TYPE_ENOSUCH                102 /* rejected packet notification */

/* 999: Largest type code for general IACP use */
#define IACP_TYPE_IACP_MAX               999

/* 1000 - 1999: IDA System Interface (ISI) */
#define IACP_TYPE_ISI_MIN                1000
#define IACP_TYPE_ISI_MAX                1999

/* Alert codes used for as IACP_TYPE_ALERT  payloads */

#define IACP_EINVAL_UINT32   0xffffffff /* used to flag an invalid UINT32 */
#define IACP_ALERT_NONE               0 /* never sent */
#define IACP_ALERT_DISCONNECT         1 /* normal disconnect */
#define IACP_ALERT_REQUEST_COMPLETE   2 /* request complete */
#define IACP_ALERT_IO_ERROR           3 /* i/o error */
#define IACP_ALERT_SERVER_FAULT       4 /* server error */
#define IACP_ALERT_SERVER_BUSY        5 /* too many active connections */
#define IACP_ALERT_FAILED_AUTH        6 /* frame signature failed to verify */
#define IACP_ALERT_ACCESS_DENIED      7 /* access to server refused */
#define IACP_ALERT_REQUEST_DENIED     8 /* client request refused */
#define IACP_ALERT_SHUTDOWN           9 /* shutdown in progress */
#define IACP_ALERT_PROTOCOL_ERROR    10 /* illegal frame received */
#define IACP_ALERT_ILLEGAL_DATA      11 /* unexpected frame data */
#define IACP_ALERT_UNSUPPORTED       12 /* unsupported IACP frame type */
#define IACP_ALERT_OTHER_ERROR       99 /* other error */

/* error levels associated with I/O failures */

#define IACP_ERR_NONE      (                     (int) 0)
#define IACP_ERR_TRANSIENT (IACP_ERR_NONE      + (int) 1)
#define IACP_ERR_NONFATAL  (IACP_ERR_TRANSIENT + (int) 1)
#define IACP_ERR_FATAL     (IACP_ERR_NONFATAL  + (int) 1)

/* limits */

#define IACP_DATA_PREAMBLE_LEN 16 /* "IACP" + seqno + type + len */
#define IACP_SIG_PREAMBLE_LEN   8 /* keyid + len */
#define IACP_MAX_PREAMBLE_LEN IACP_DATA_PREAMBLE_LEN

#define IACP_MINTIMEO     30000   /* 30 sec minimum i/o timeout with server */
#define IACP_MAXSIGLEN    40      /* DSS signature  length */

/* connection attributes */

typedef struct {
    UINT32 at_timeo;   /* i/o timeout interval */
    UINT32 at_sndbuf;  /* TCP/IP send buffer size */
    UINT32 at_rcvbuf;  /* TCP/IP receive buffer size  */
    UINT32 at_maxerr;  /* largest tolerable I/O error level */
    BOOL   at_retry;   /* if TRUE, retry failed connects */
    UINT32 at_wait;    /* retry interval, msec */
    char  *at_dbgpath; /* path name for debug output */
} IACP_ATTR;

/* Handle for socket I/O */

#define IACP_DOT_DECIMAL_LEN 15
#define IACP_MAX_PEER_NAME_LEN 63

typedef struct {
    MUTEX  *mutex; /* points to IACP handle mutex */
    UINT32 nbyte;
    UINT32 nframe;
    UINT32 maxlen;
    UINT32 minlen;
    UINT32 avelen;
    UINT32 stddev;
    UINT64 sumlen;
    UINT64 ssdiff;
} IACP_STATS;

typedef struct {
    int sd;                         /* socket descriptor */
    time_t connect;                 /* time connection was established */
    int port;                       /* port number */
    struct {
        char addr[INET_ADDRSTRLEN]; /* remote peer IP address */
        char name[MAXPATHLEN];      /* remote peer name */
        UINT32 pid;                 /* remote peer process id */
        char ident[MAXPATHLEN];     /* pid@name */
    } peer;
    IACP_ATTR attr;                 /* connection attributes */
    struct {
        int error;                  /* last read errno */
        int status;                 /* last read status */
        time_t  tstamp;             /* time of last read */
        IACP_STATS stats;           /* statistics */
    } recv;
    struct {
        int error;                  /* last write errno */
        time_t  tstamp;             /* time of last write */
        IACP_STATS stats;           /* statistics */
    } send;
    UINT32 seqno;                   /* outbound sequence number */
    MUTEX mutex;
    LOGIO *lp;                      /* logging facility handle */
    int debug;                      /* debug level */
    BOOL disabled;                  /* used by server, TRUE when shutting down */
    FILE *dbgfp;                    /* for debugging traffic */
} IACP;

/* an IACP message */

typedef struct {
    UINT32 type;  /* message type */
    UINT32 len;   /* message length */
    UINT8 *data;  /* message */
} IACP_PAYLOAD;

/* an opaque IACP frame */

typedef struct {
    UINT32 seqno;                   /* sequence number */
    IACP_PAYLOAD payload;           /* the message */
    struct {
        BOOL verified;              /* TRUE if signature verified */
        UINT32 id;                  /* public key identifer */
        UINT32 len;                 /* length of signature */
        UINT8  sig[IACP_MAXSIGLEN]; /* DSS signature over data */
    } auth;
} IACP_FRAME;

/* various defaults */

#ifndef IACP_DEF_AT_TIMEO
#define IACP_DEF_AT_TIMEO IACP_MINTIMEO 
#endif

#ifndef IACP_DEF_AT_SNDBUF
#define IACP_DEF_AT_SNDBUF 0 /* use OS default */
#endif

#ifndef IACP_DEF_AT_RCVBUF
#define IACP_DEF_AT_RCVBUF 0 /* use OS default */
#endif

#ifndef IACP_DEF_AT_MAXERR
#define IACP_DEF_AT_MAXERR IACP_ERR_NONFATAL
#endif

#ifndef IACP_DEF_AT_RETRY
#define IACP_DEF_AT_RETRY TRUE
#endif

#ifndef IACP_DEF_AT_WAIT
#define IACP_DEF_AT_WAIT 10000
#endif

#ifndef IACP_DEF_AT_DBPATH
#define IACP_DEF_AT_DBPATH NULL
#endif

#ifdef INCLUDE_IACP_DEFAULT_ATTR
static IACP_ATTR IACP_DEFAULT_ATTR = {
    IACP_DEF_AT_TIMEO,
    IACP_DEF_AT_SNDBUF,
    IACP_DEF_AT_RCVBUF,
    IACP_DEF_AT_MAXERR,
    IACP_DEF_AT_RETRY,
    IACP_DEF_AT_WAIT,
    IACP_DEF_AT_DBPATH
};
#endif /* INCLUDE_IACP_DEFAULT_ATTR */

/* macros */

#define iacpHeartbeat(iacp) iacpSendEmptyMessage(iacp, IACP_TYPE_NOP)
#define iacpSendNull(iacp) iacpSendEmptyMessage(iacp, IACP_TYPE_NULL)
#define iacpSendAlert(iacp, code) iacpSendUINT32(iacp, IACP_TYPE_ALERT, code)
#define iacpSendENOSUCH(iacp, code) iacpSendUINT32(iacp, IACP_TYPE_ENOSUCH, code)
#define iacpLiveConnection(iacp) (iacp->sd != INVALID_SOCKET)

/* function prototypes */

/* accept.c */
IACP *iacpAccept(IACP *server);

/* alert.c */
void iacpBuildAlertFrame(IACP_FRAME *frame, UINT32 code);

/* auth.c */
BOOL iacpAuthInit(IACP *iacp);
BOOL iacpSignFrame(IACP *iacp, IACP_FRAME *frame);
BOOL iacpVerifyFrame(IACP_FRAME *frame);
BOOL iacpVerified(IACP_FRAME *frame);

/* close.c */
IACP *iacpClose(IACP *iacp);

/* connect.c */
BOOL iacpConnect(IACP *iacp);
BOOL iacpReconnect(IACP *iacp);

/* free.c */
IACP *iacpFree(IACP *iacp);

/* get.c */
time_t iacpGetStartTime(IACP *iacp);
int iacpGetSendError(IACP *iacp);
int iacpGetSendError(IACP *iacp);
time_t iacpGetSendTstamp(IACP *iacp);
int iacpGetRecvError(IACP *iacp);
int iacpGetRecvStatus(IACP *iacp);
time_t iacpGetRecvTstamp(IACP *iacp);
int iacpGetDebug(IACP *iacp);
UINT32 iacpGetTimeoutInterval(IACP *iacp);
IACP_ATTR *iacpGetAttr(IACP *iacp, IACP_ATTR *attr);
UINT32 iacpGetRetryInterval(IACP *iacp);
UINT32 iacpGetMaxError(IACP *iacp);
BOOL iacpGetRetry(IACP *iacp);
BOOL iacpGetDisabled(IACP *iacp);
IACP_STATS *iacpGetRecvStats(IACP *iacp, IACP_STATS *dest);
IACP_STATS *iacpGetSendStats(IACP *iacp, IACP_STATS *dest);

/* handshake.c */
BOOL iacpClientHandshake(IACP *iacp);
BOOL iacpServerHandshake(IACP *iacp);

/* open.c */
IACP *iacpOpen(char *host, int port, IACP_ATTR *user_attr, LOGIO *lp, int debug);

/* recv.c */
BOOL iacpRecvFrame(IACP *iacp, IACP_FRAME *frame, UINT8 *buf, UINT32 buflen);

/* send.c */
BOOL iacpSendFrame(IACP *iacp, IACP_FRAME *frame);

/* server.c */
IACP *iacpServer(int port, IACP_ATTR *user_attr, LOGIO *lp, int debug);

/* set.c */
VOID iacpSetDebug(IACP *iacp, int value);
VOID iacpSetLogio(IACP *iacp, LOGIO *logio);
VOID iacpSetRetryFlag(IACP *iacp, BOOL value);
VOID iacpSetDisabled(IACP *iacp, BOOL value);

/* stats.c */
VOID iacpIncrByteCounter(IACP_STATS *stats, int increment, BOOL AlreadyLocked);
VOID iacpUpdateFrameStats(IACP_STATS *stats, UINT32 len, BOOL AlreadyLocked);
 VOID iacpInitStats(IACP_STATS *stats, MUTEX *mutex);

/* string.c */
char *iacpAlertString(UINT32 code);

/* util.c */
VOID iacpInitHandle(IACP *iacp, char *peer, int port, IACP_ATTR *user_attr, LOGIO *lp, int debug);
int iacpErrorLevel(int ErrorCode);
char *iacpPeerIdent(IACP *iacp);
VOID iacpDisconnect(IACP *iacp, UINT32 cause);
BOOL iacpSendUINT32(IACP *iacp, UINT32 type, UINT32 value);
BOOL iacpSendEmptyMessage(IACP *iacp, UINT32 type);
int iacpAlertCauseCode(IACP_FRAME *frame);
char *iacpMaxerrString(UINT32 value);
VOID iacpPrintAttr(FILE *fp, IACP_ATTR *attr);
VOID iacpDumpDataPreamble(IACP *iacp, IACP_FRAME *frame, char *prefix, UINT8 *raw, UINT32 len);
VOID iacpDumpDataPayload(IACP *iacp, IACP_FRAME *frame, char *prefix);
VOID iacpDumpSignaturePreamble(IACP *iacp, IACP_FRAME *frame, char *prefix, UINT8 *raw, UINT32 len);

/* version.c */
BOOL iacpVersionSend(IACP *iacp, IACP_FRAME *frame);
char *iacpVersionString(VOID);

#ifdef __cplusplus
}
#endif

#endif

/* Revision History
 *
 * $Log: iacp.h,v $
 * Revision 1.13  2007/01/11 17:46:49  dechavez
 * added IACP_ALERT_UNSUPPORTED
 *
 * Revision 1.12  2005/05/06 01:06:38  dechavez
 * 1.7.0 (added support for IACP_STATS)
 *
 * Revision 1.11  2005/01/28 01:46:35  dechavez
 * added dbgpath attribute and dbgfp handle field, updated prototypes
 *
 * Revision 1.10  2004/06/25 18:34:56  dechavez
 * C++ compatibility
 *
 * Revision 1.9  2004/06/18 21:08:52  dechavez
 * removed at_myport
 *
 * Revision 1.8  2004/06/17 19:58:36  dechavez
 * added at_myport attribute
 *
 * Revision 1.7  2004/01/29 18:31:57  dechavez
 * added more prototypes
 *
 * Revision 1.6  2003/11/19 23:43:55  dechavez
 * updated prototypes
 *
 * Revision 1.5  2003/11/04 20:05:07  dechavez
 * changed at_timeo and at_wait units to msec
 *
 * Revision 1.4  2003/11/03 23:10:02  dechavez
 * added IACP_ALERT_OTHER_ERROR and iacpLiveConnection()
 *
 * Revision 1.3  2003/10/16 16:32:53  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:57:40  dechavez
 * increased IACP_MAXSUBFRAME_DATA_LEN to 32K, removed macros, added new
 * function prototypes and tags
 *
 * Revision 1.1  2003/06/09 23:43:26  dechavez
 * initial release
 *
 */
