#pragma ident "$Id: rtp.h 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * Defines, templates, and prototypes for draft RTP support library.
 *
 * This is the "front end" view of communication.
 *
	Revised  :
		14Sep05	(PLD) change max streams from 8 to 9
		03Sep05  (pld) add support for FD packets
 *====================================================================*/
#ifndef rtp_include_defined
#define rtp_include_defined

/* platform specific stuff */

#include "platform.h"

/* misc. constants */

#define RTP_DEFAULT_PORT 2543

/* This defines the protocol we'll follow... hopefully it will
 * never need to be changed.
 */
 
#define RTP_VERSION 1
#define RTP_PREAMBLE_LEN ((INT32) (sizeof(UINT16)+sizeof(INT32)))

/* misc. limits */
 
//#define RTP_MAXSTREAM    9 /* maximum number of streams per DAS     */
#define RTP_MAXSTREAM   10 /* PLDSTRM: really want just 9 */
#define RTP_MAXCHAN     16 /* maximum number of channels per stream */
#define RTP_MINTIMEO    30 /* minimum i/o timeout with server       */
#define RTP_DASPAKLEN 1024 /* DAS packet length                     */
#define RTP_MAXMSGLEN RTP_DASPAKLEN
#define RTP_UNITSTRLEN 5

#define RTP_MINPMASKBUFLEN 64 /* for use with rtp_decode_pmask */
#define RTP_MINSMASKBUFLEN 64 /* for use with rtp_decode_smask */

/* Message/command types */

#define RTP_MSG_REFTEK  0  /* a Reftek (ie, from DAS) packet         */
#define RTP_MSG_CMDPKT  1  /* an RTP command packet                  */
#define RTP_MSG_NOP     2  /* heartbeat message                      */
#define RTP_MSG_ATTR    3  /* connection attribute message           */
#define RTP_MSG_SOH     4  /* state of health request/reply          */
#define RTP_MSG_START   5  /* start forwarding packets               */
#define RTP_MSG_STOP    6  /* stop forwarding packets                */
#define RTP_MSG_FLUSH   7  /* flush buffered but undelivered packets */
#define RTP_MSG_BREAK   8  /* break connection                       */
#define RTP_MSG_BUSY    9  /* server busy message                    */
#define RTP_MSG_FAULT  10  /* server fault                           */
#define RTP_MSG_PID    11  /* peer pid message                       */

/* packet types (for packet mask) */

#define RTP_PMASK_SPEC 0x0001 /* "special" packets */
#define RTP_PMASK_AD   0x0002
#define RTP_PMASK_CD   0x0004
#define RTP_PMASK_DS   0x0008
#define RTP_PMASK_DT   0x0010
#define RTP_PMASK_EH   0x0020
#define RTP_PMASK_ET   0x0040
#define RTP_PMASK_OM   0x0080
#define RTP_PMASK_SH   0x0100
#define RTP_PMASK_SC   0x0200
#define RTP_PMASK_CMND 0x0400 /* command packets */
#define RTP_PMASK_FD   0x0800

#define RTP_PMASK_ALL  0xffff
#define RTP_PMASK_NONE 0x0000

/* error levels associated with read failures */

#define RTP_ERR_NONE      (                    (UINT16) 0)
#define RTP_ERR_TRANSIENT (RTP_ERR_NONE      + (UINT16) 1)
#define RTP_ERR_NONFATAL  (RTP_ERR_TRANSIENT + (UINT16) 2)
#define RTP_ERR_FATAL     (RTP_ERR_NONFATAL  + (UINT16) 3)

/* Connection attributes */

struct rtp_attr {
    MUTEX at_mutex;  /* for protection                       */
    UINT32 at_dasid;  /* das "mask"                           */
    UINT32 at_pmask;  /* packet mask                          */
    UINT32 at_smask;  /* stream mask                          */
    INT32  at_timeo;  /* i/o timeout interval (for rtpd i/o)  */
    INT32  at_sndbuf; /* TCP/IP transmit buffer size          */
    INT32  at_rcvbuf; /* TCP/IP receive  buffer size          */
    BOOL  at_block;  /* application level block/noblock flag */
};

/* Default attributes */

#ifndef RTP_DEFDASID
#define RTP_DEFDASID 0 /* all DASes */
#endif

#ifndef RTP_DEFPMASK
#define RTP_DEFPMASK (RTP_PMASK_ALL & ~RTP_PMASK_SPEC) /* all but special */
#endif

#ifndef RTP_DEFSMASK
#define RTP_DEFSMASK 0xff /* all streams */
#endif

#ifndef RTP_DEFTIMEO
#define RTP_DEFTIMEO RTP_MINTIMEO 
#endif

#ifndef RTP_DEFBLOCK
#define RTP_DEFBLOCK 1 /* block at the application level */
#endif

#ifndef RTP_DEFSNDBUF
#define RTP_DEFSNDBUF 0 /* use OS default */
#endif

#ifndef RTP_DEFRCVBUF
#define RTP_DEFRCVBUF 0 /* use OS default */
#endif

static struct rtp_attr RTP_DEFAULT_ATTR = {
    MUTEX_INITIALIZER,
    RTP_DEFDASID,
    RTP_DEFPMASK,
    RTP_DEFSMASK,
    RTP_DEFTIMEO,
    RTP_DEFSNDBUF,
    RTP_DEFRCVBUF,
    RTP_DEFBLOCK
};

/* Per-DAS state of health statistics */

struct rtp_stat {
    UINT16 st_id;    /* DAS id     */
    UINT32 st_addr;  /* IP address */
    struct {
        INT32 front; /* time last packet was received from frontend */
        INT32 back;  /* time last packet was received from backend  */
    } st_last;
};

struct rtp_soh {
    MUTEX  sh_mutex;          /* for protection                 */
    UINT32 sh_tstamp;         /* server time stamp              */
    INT32  sh_nslot;          /* number of slots allocated      */
    INT32  sh_ndas;           /* number of defined DASes        */
    INT32  sh_index;          /* index of next struct to report */
    struct rtp_stat *sh_stat; /* array of info to report        */
};

/* Handle for all frontend I/O with rtpd */

#define RTP_DEFNSLOT 32 /* up to 32 connected DASes before realloc */

struct rtp_handle {
    SOCKET sd;             /* socket descriptor           */
    UINT16  port;            /* port number                 */
    CHAR *peer;            /* remote peer name or address */
    CHAR *addr;            /* remote peer x.x.x.x address */
    INT32 pid;             /* remote peer process id      */
    struct rtp_attr attr;  /* connection attributes       */
    struct rtp_soh soh;    /* most recent state of health */
    struct {
        MUTEX mutex; /* for MT reads     */
        UINT16 error; /* last read status */
    } rcv;
    struct {
        MUTEX mutex;                 /* for MT writes     */
        UINT8 buf[RTP_PREAMBLE_LEN]; /* outgoing preamble */
    } snd;
};
typedef struct rtp_handle RTP;

/* An RTP command packet */

typedef struct rtp_cmdpkt {
    UINT16 unit; /* unit id this command is destined for (0 => all) */
    UINT16 len;  /* number of bytes of command data to follow       */
    UINT8  data[RTP_MAXMSGLEN]; /* the command data                 */
} RTP_CMDPKT;

/* Log levels for the logging facility */

#define RTP_LOG_ERR     ((UINT16) 0x0001)
#define RTP_LOG_WARN    ((UINT16) 0x0002)
#define RTP_LOG_INFO    ((UINT16) 0x0004)
#define RTP_LOG_DEBUG   ((UINT16) 0x0008)
#define RTP_LOG_ECHO    ((UINT16) 0x8000)
#define RTP_LOG_DEFAULT (RTP_LOG_ERR | RTP_LOG_WARN | RTP_LOG_INFO)
#define RTP_LOG_VERBOSE (RTP_LOG_ERR | RTP_LOG_WARN | RTP_LOG_INFO | RTP_LOG_DEBUG)

#define RTP_ERR         RTP_LOG_ERR
#define RTP_WARN        RTP_LOG_WARN
#define RTP_INFO        RTP_LOG_INFO
#define RTP_DEBUG       RTP_LOG_DEBUG
#define RTP_DEFAULT_LOG RTP_LOG_DEFAULT
#define RTP_VERBOSE_LOG RTP_LOG_VERBOSE

/* Macros */

#define RTP_HEX_UNIT_ID(unit) ((unit) >= 0x9000) /* to support the RT130 */

#define rtp_message(hndl, type) rtp_send((hndl),(UINT8 *)NULL,type,0)

#define rtp_fault(hndl)  (rtp_message((hndl), RTP_MSG_FAULT))
#define rtp_busy(hndl)   (rtp_message((hndl), RTP_MSG_BUSY))
#define rtp_break(hndl)  (rtp_message((hndl), RTP_MSG_BREAK))
#define rtp_start(hndl)  (rtp_message((hndl), RTP_MSG_START))
#define rtp_stop(hndl)   (rtp_message((hndl), RTP_MSG_STOP))
#define rtp_reqsoh(hndl) (rtp_message((hndl), RTP_MSG_SOH))
#define rtp_hbeat(hndl)  (rtp_message((hndl), RTP_MSG_NOP))
#define rtp_flush(hndl)  (rtp_message((hndl), RTP_MSG_FLUSH))

#define rtp_setattr(hndl, ptr) (rtp_attr_send((hndl), (ptr)))

#define rtp_sohtime(hndl) ((hndl)->soh.sh_tstamp)

/* Function prototypes */

RTP *rtp_server(UINT16 port, UINT16 backlog, struct rtp_attr *user_attr);
RTP *rtp_accept(RTP *server);
RTP *rtp_open(CHAR *host, UINT16 port, struct rtp_attr *user_attr, UINT16 maxerr);
VOID rtp_close(RTP *rtp);
 
BOOL rtp_recv(RTP *rtp, UINT8 *buf, UINT16 *type, INT32 *datlen);
BOOL rtp_send(RTP *rtp, UINT8 *buf, UINT16 type, INT32 datlen);
BOOL rtp_daspkt(RTP *rtp, UINT8 *buf, INT32 *msglen);
 
UINT16 rtp_setstat(RTP *rtp);
struct rtp_stat *rtp_getstat(RTP *rtp);
 
VOID rtp_attr_decode(UINT8 *buf, struct rtp_attr *attr);
BOOL rtp_attr_send(RTP *rtp, struct rtp_attr *attr);
 
BOOL rtp_loginit(CHAR *file, UINT16 facility, CHAR *tfmt, CHAR *fmt, ...);
VOID rtp_log(UINT16 level, CHAR *fmt, ...);
VOID rtp_loglevel(UINT16 newlevel);
VOID rtp_flushlog();
 
VOID rtp_soh_decode(UINT8 *buf, struct rtp_soh *soh);
BOOL rtp_soh_send(RTP *rtp, struct rtp_soh *soh);
 
INT16 rtp_version_recv(RTP *rtp);
BOOL  rtp_version_send(RTP *rtp);

char *rtp_strunit(UINT16 unit, char *buf);
BOOL rtp_getattr(RTP *, struct rtp_attr *);
INT32 rtp_timeout(RTP *);
UINT16 rtp_errno(RTP *rtp);
BOOL  rtp_want(RTP *rtp, UINT8 *pkt);

CHAR *rtp_decode_pmask(UINT32 pmask, CHAR *buf);
BOOL rtp_encode_pmask(CHAR **token, UINT16 ntok, UINT32 *pmask);
CHAR *rtp_decode_smask(UINT32 smask, CHAR *buf);
BOOL rtp_encode_smask(CHAR **token, UINT16 ntok, UINT32 *smask);

INT32 rtp_cmdpkt_encode(UINT8 *buf, RTP_CMDPKT *cmdpkt);
VOID rtp_cmdpkt_decode(UINT8 *buf, RTP_CMDPKT *pkt);
BOOL rtp_cmdpkt_send(RTP *rtp, RTP_CMDPKT *pkt);

BOOL rtp_pid_decode(UINT8 *buf, INT32 *pid);
BOOL rtp_pid_send(RTP *rtp);

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:35:59  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.5  2005/09/03 21:52:29  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.4  2004/01/08 17:23:26  pdavidson
 * Corrected handling of Unit ID numbers.
 *
 * Revision 1.3  2003/05/22 18:07:16  pdavidson
 * Add RTP_LOG_ECHO constant and update other log constants.
 *
 * Revision 1.2  2002/01/18 17:49:01  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
