#pragma ident "$Id: ida.h,v 1.22 2008/03/05 22:46:03 dechavez Exp $"
/*======================================================================
 *
 *  IDA specific stuff
 *
 *====================================================================*/
#ifndef ida_h_included
#define ida_h_included

#include "platform.h"
#include "dbio.h"
#include "logio.h"
#include "isi.h"
#include "ida/limits.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constants */

#define IDA_REV9_TAG_0 "IDA9" /* 9.0 (original) */
#define IDA_REV9_TAG_X "Ida9" /* 9.x (x at byte 23) */

/*  Hardware system codes  */

#define IDA_MK3  0x0001
#define IDA_MK4  0x0002
#define IDA_MK5  0x0004
#define IDA_MK6A 0x0008
#define IDA_MK6B 0x0010
#define IDA_MK7A 0x0020
#define IDA_MK7B 0x0040

/*  Record types  */

#define IDA_UNKNOWN  0x0000
#define IDA_IDENT    0x0001
#define IDA_DATA     0x0002
#define IDA_CONFIG   0x0004
#define IDA_LOG      0x0008
#define IDA_CALIB    0x0010
#define IDA_EVENT    0x0020
#define IDA_OLDHDR   0x0040
#define IDA_OLDCALIB 0x0080
#define IDA_POSTHDR  0x0100
#define IDA_RESERVED 0x0200
#define IDA_DMPREC   0x0400
#define IDA_LABEL    0x0800
#define IDA_ARCEOF   0x1000
#define IDA_DASSTAT  0x2000
#define IDA_ISPLOG   0x4000
#define IDA_CORRUPT  0xffff

/*  Digitizer codes  */

#define IDA_DAS     0x01
#define IDA_ARS     0x02
#define IDA_NUMATOD 2

/*  Calibration type codes  */

#define IDA_IMPULSE  1
#define IDA_SQUARE   2
#define IDA_SINE     3
#define IDA_TRIANGLE 4
#define IDA_RANDOM   5

/*  Output devices  */

#define IDA_TAPE  0x01
#define IDA_MODEM 0x02
#define IDA_DTOA  0x04

/*  Calibration channel bitmaps  */

#define IDA_BBCAL  0x01
#define IDA_SPCAL  0x02
#define IDA_INTCAL 0x04

/*  Format codes  */

#define S_UNCOMP    0 /* uncompressed shorts                      */
#define L_UNCOMP    1 /* uncompressed longs                       */
#define IGPP_COMP   2 /* IGPP compression                         */
#define FP32_COMP   3 /* 32 bit fixed point (16:16), uncompressed */
#define FP32_UNCOMP 4 /* 32 bit fixed point (16:16), compressed   */
#define BAD_FORM    5 /* unrecognized format code                 */

/* Mode codes  */

#define CONT       0 /* continuous recording   */
#define TRIG       1 /* triggered              */
#define BAD_MODE   2 /* unrecognized mode code */

/*  OLD Possible time tag error flags  */

#define IDA_BADTQUAL         -12345 /* unrecognized time qual code */
#define TTAG_LOW_EXTTIM      0x0001 /* ext time <  TTAG_MIN_EXTTIM */
#define TTAG_BIG_EXTTIM      0x0002 /* ext time >= TTAG_MAX_EXTTIM */
#define TTAG_BAD_EXTTIM      0x0004 /* illegal external time       */
#define TTAG_BAD_OFFSET      0x0008 /* hz time < sys time          */
#define TTAG_BAD_CLKRD       0x0010 /* corrupt time from clock?    */
#define TTAG_BAD_SYSTIM      0x0020 /* beg sys >= end sys time     */
#define TTAG_LEAP_YEAR_PATCH 0x0040 /* for leap year bug applied   */

#define IDA_LOGOPER   1
#define IDA_LOGFATAL  2
#define IDA_LOGERROR  3
#define IDA_LOGWARN   4
#define IDA_LOGINFO   5
#define IDA_LOGDEBUG  6

/* Datatypes */

/*  OLD Time structure  */

typedef struct {
    short year;         /* Data logger's idea of the year           */
    unsigned long mult; /* System and Hz time multiplier (msec/tic) */
    unsigned long sys;  /* System time in clock tics                */
    unsigned long ext;  /* External time in seconds                 */
    unsigned long hz;   /* 1-Hz time in clock tics                  */
    double tru;         /* year + ext + (sys - hz)*mult/1000        */
    char  code;         /* Omega clock quality code                 */
    short qual;         /* Omega code mapped to internal code       */
    int error;          /* error flag (see constants above)         */
} IDA_TIME_TAG;

/* Calibration record */

typedef struct {
    int   rev;      /* datalogger format revision code              */
    int   atod;     /* internal digitizer code                      */
    short state;    /* calibration on/off flag                      */
    short func;     /* internal calibration function code           */
    short chan;     /* calibration channel bitmap                   */
    short atten;    /* DAC attenuation factor                       */
    short period;   /* period of calibration function               */
    short width;    /* width of impulse function                    */
    short interval; /* interval between impulses in msecs           */
    short poly;     /* primitive polynomial % 2 for random function */
    short seed;     /* random bit generator seed                    */
    short nchan;    /* number of entries in below array             */
    short gain[IDA_MAXCALCHAN];/* array of channel gains, int. cal */
    IDA_TIME_TAG ttag;       /* record time tag                  */
} IDA_CALIB_REC;

/* Data record */

typedef struct {
    BOOL valid;       /* TRUE if this structure is supported */
    UINT32 seqno;     /* creation sequence number */
    UINT32 tstamp;    /* creation time stamp */
} IDA_EXTRA;

#define IDA_DEFAULT_EXTRA { FALSE, 0, 0 }

typedef struct {
    int   rev;            /* datalogger format revision code          */
    int   atod;           /* internal digitizer code                  */
    int   mode;           /* internal mode code                       */
    int   form;           /* internal format code                     */
    int   nbytes;         /* number of bytes of raw data              */
    int   srate;          /* digitizer sample rate                    */
    int   decim;          /* filter decimation factor                 */
    int   wrdsiz;         /* uncompressed word size                   */
    int   ffu_shift;      /* Fels' fuck up shift for this stream      */
    unsigned long order;  /* raw data byte order                      */
    short dl_stream;      /* data logger stream code                  */
    short dl_format;      /* data logger format code                  */
    short dl_channel;     /* data logger channel code                 */
    short dl_filter;      /* data logger filter code                  */
    short nsamp;          /* number of samples                        */
    long  last_word;      /* uncompressed last sample                 */
    float sint;           /* stream sample interval (ie, decim/srate) */
    IDA_TIME_TAG beg;     /* time tag of first datum                  */
    IDA_TIME_TAG end;     /* time tag of last  datum                  */
    IDA_EXTRA extra;      /* optional seqno and timestamp */
    int rt593;            /* if non-zero, a RT593 packet */
    int subformat;        /* 9.x subformat, or 0 */
    int flags;            /* flags from rev9 byte 23 nibble */
#define IDA9_FLAG_LEAPYEAR_BUG 0x10
} IDA_DHDR;

typedef struct {
    IDA_DHDR head;
    UINT8 data[IDA_MAXDLEN];
} IDA_DREC;

/* Configration record */

typedef struct {
    short dl_stream;    /* data logger stream code                  */
    short dl_channel;   /* input channel of stream                  */
    short dl_filter;    /* filter code of filter applied to data    */
    short mode;         /* internal mode code                       */
    short output;       /* bit-map of output devices (internal map) */
    short dl_gain;      /* data logger programable gain setting     */
} IDA_MK_CONFIGURATION;

typedef struct {
    short chan;    /* bit-map of detector input channels           */
    short key;     /* event detector key channel                   */
    short sta;     /* STA length in samples                        */
    short lta;     /* LTA length in samples                        */
    short thresh;  /* detection STA/LTA ratio                      */
    long  maxlen;  /* maximum trigger length in samples            */
    short voters;  /* minimum number of voters to declare an event */
    short memory;  /* pre-event memory in records                  */
} IDA_DETECTOR;

typedef struct {
    int   rev;           /* datalogger format revision code */
    int   atod;          /* internal digitizer code         */
    long  srate;         /* digitizer sample rate           */
    long  nstream;       /* number of streams               */
    short jumpers;       /* backplane jumper settings       */
    IDA_TIME_TAG ttag;   /* record time tag                 */
    IDA_DETECTOR detect; /* event detector parameters       */
    IDA_MK_CONFIGURATION table[IDA_MAXSTREAMS]; /* config. table */
} IDA_CONFIG_REC;

/*  Identification record  */

typedef struct {
    int   rev;               /* datalogger format revision code */
    int   atod;              /* internal digitizer code */
    short atod_id;           /* ARS or DAS ident */
    short atod_rev;          /* ARS or DAS firmware rev */
    short dsp_rev;           /* DSP firmware rev (DAS only) */
    short tape_no;           /* tape sequence number */
    IDA_TIME_TAG ttag;       /* record time stamp */
    IDA_TIME_TAG beg;        /* last tape insert time */
    IDA_TIME_TAG end;        /* last tape eject  time */
    char sname[IDA_SNAMLEN]; /* station code */
    int   boot_flag;         /* if set, record was sent on boot */
} IDA_IDENT_REC;

/*  DAS status info  */

typedef struct {
    long naks;     /* number of naks received (ie, bad packets)  */
    long acks;     /* number of acks recevied (ie, packets sent) */
    long dropped;  /* number of packets dropped by DAS           */
    long timeouts; /* number of xmit timeouts with ARS/ISP       */
    long triggers; /* number of events detected                  */
    int  event;    /* event in progress flag                     */
    int  calib;    /* calibration in progress flag               */
    unsigned long seqno; /* packet sequence number               */
} IDA_DAS_STATS;

/*  A single log entry  */

typedef struct {
    int   atod;                   /* internal digitizer code  */
    int   level;                  /* log level code           */
    char preamble[IDA_MAXLOGLEN]; /* preamble to message text */
    char message[IDA_MAXLOGLEN];  /* message text             */
} IDA_LOG_ENTRY;

/*  Log record is an array of log entries  */

typedef struct {
    int   rev;            /* datalogger format revision code */
    IDA_TIME_TAG ttag; /* record time tag                 */
    int numentry;         /* number of entries               */
    IDA_LOG_ENTRY entry[IDA_MAXLOGENTRY]; /* array of entries  */
} IDA_LOG_REC;

/* Event record */

typedef struct {
    long sta;            /* detector STA at time of event */
    long lta;            /* detector LTA at time of event */
    IDA_TIME_TAG beg; /* approx. trigger on time       */
    IDA_TIME_TAG end; /* approx. trigger off time      */
    double duration;     /* approx. duration in seconds   */
} IDA_DETECTION;

typedef struct {
    int   rev;            /* datalogger format revision code */
    IDA_TIME_TAG ttag; /* record time tag                 */
    int nevent;           /* number of detections            */
    IDA_DETECTION event[IDA_MAXEVENTS]; /* array of detections  */
} IDA_EVENT_REC;

/* Old-sytle (revs 1-4) configuration table */

typedef struct {
    short nrows;  /* number of rows                  */
    short ncols;  /* number of columns               */
    short table[IDA_MAXOLDCNF]; /* the configration table */
} IDA_OLD_CONFIG;

/* Old-sytle (revs 1-4) event detector parameters */

typedef struct {
    short sta;          /* short term average time (msc)           */
    short lta;          /* long term average time (sec)            */
    short thresh_off;   /* threshold offset                        */
    short turnon_fact;  /* turn on:  STA/LTA * 100                 */
    short turnoff_fact; /* turn off: STA/LTA * 100                 */
    short min_rtime;    /* minimum no. seconds to record per event */
    short max_rtime;    /* maximum no. seconds to record per event */
    short min_trgint;   /* minimum no. seconds between events      */
    short dfg;          /* detector filter gain                    */
} IDA_OLD_DETECT;

/* Old-style (revs 1-4) header record */

typedef struct {
    int   rev;                 /* datalogger format revision code    */
    char  id;                  /* head record counter                */
    char  sname[7];            /* station name                       */
    char  dl_rev[6];           /* firmware revision code             */
    char  das_id[8];           /* DAS ident                          */
    char  ars_id[8];           /* ARS ident                          */
    short year;                /* data logger's idea of year         */
    unsigned long ext_time;    /* Absolute (external) time tag       */
    IDA_OLD_CONFIG config;     /* configuration table                */
    short numtap;              /* number of tape drives              */
    short reclen;              /* tape record length                 */
    short rpf;                 /* records per tape file              */
    short fpt;                 /* maximum number of files per tape   */
    IDA_OLD_DETECT bb_trig;    /* BB event detector parameters       */
    IDA_OLD_DETECT sp_trig;    /* SP event detector parameters       */
    short mt1_errors[3];       /* tape unit 1 error counts           */
    short mt2_errors[3];       /* tape unit 2 error counts           */
    short ars_errors[3];       /* data transmission error counts     */
    short das_errors[4];       /* data acquisition  error counts     */
    short ars_reboots;         /* number of ARS reboots              */
    short das_reboots;         /* number of DAS reboots              */
} IDA_OLD_HEADER;

/* Old-style (revs 1-4) calibration record */

typedef struct {
    int   rev;  /* datalogger format revision code */
    short unsupported;
} IDA_OLD_CALIB_REC;

/* Channel map entry (MK7 digitizers) */

typedef struct {
    int ccode;
    int fcode;
    int tcode;
    char chn[ISI_CHNLEN+1];
    char loc[ISI_LOCLEN+1];
} IDA_CHNLOCMAP;

#define IDA_REV_DESC_LEN 64
typedef struct {
    int value;
    char description[IDA_REV_DESC_LEN+1];
} IDA_REV;

typedef struct {
    char factory;
    int  internal;
} IDA_TQUAL;

typedef struct {
    short chan;
    short filt;
    int stream;
} IDA_CFS;

typedef struct {
    char *site;
    char mapname[IDA_MNAMLEN+1];
    LNKLST *chnlocmap;
    LNKLST *tqual;
    IDA_REV rev;
    DBIO *db;
    LOGIO *lp;
    LNKLST *cfs;
    int flags;
} IDA;

/* Macros */

#ifndef leap_year
#define leap_year(i) ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0)
#endif
#ifndef dysize
#define dysize(i) (365 + leap_year(i))
#endif

#ifdef M_I86 /*  Need to byte-swap on PC's, defined by MS C compiler */
#define SWAB
#endif

#ifdef vax  /*  Need to byte-swap on Vax's  */
#define SWAB
#endif

#ifdef i386  /*  Need to byte-swap on PC's  */
#define SWAB
#endif

#ifdef SWAB
#define SSWAB(a, b) util_sswap((short *) (a), (long) (b))
#define LSWAB(a, b) util_lswap((long *)  (a), (long) (b))
#else
#define SSWAB(a, b)
#define LSWAB(a, b)
#endif /* ifdef SWAB */

/* Prototyypes */

/* cfs.c */
int idaFakeStreamNumber(IDA *ida, short chan, short filt);

/* chnlocmap.c */
BOOL idaSetChnLocMap(IDA *ida, char *name, UINT32 when);
char *idaChnlocName(IDA *ida, int ccode, int fcode, int tcode, char *buf, int buflen);
void idaBuildStreamName(IDA *ida, IDA_DHDR *hdr, ISI_STREAM_NAME *name);

/* cnfrec.c */
int ida_cnfrec(IDA *ida, IDA_CONFIG_REC *dest, UINT8 *src);

/* crec.c */
int ida_crec(IDA *ida, IDA_CALIB_REC *dest, UINT8 *src);

/* dcmp.c */
INT32 ida_dcmp(INT32 *dest, UINT8 *src, int nsamp);

/* dhead.c */
int ida_dhead(IDA *ida, IDA_DHDR *dest, UINT8 *src);

/* dhlen.c */
void ida_dhlen(int rev, int *head, int *data);

/* drec.c */
int ida_drec(IDA *ida, IDA_DREC *dest, UINT8 *src, int shift, int swab);

/* evtrec.c */
int ida_evtrec(IDA *ida, IDA_EVENT_REC *dest, UINT8 *src);

/* ext1hzoff.c */
double ida_ext1hzoff(IDA_TIME_TAG *tag);

/* filltime.c */
void ida_filltime(IDA *ida, IDA_TIME_TAG *ttag);

/* format.c */
int idaPacketFormat(UINT8 *buf);

/* handle.c */
void idaInitHandle(IDA *ida);
void idaClearHandle(IDA *ida);
IDA *idaDestroyHandle(IDA *handle);
IDA *idaCreateHandle(char *sta, int rev, char *map, DBIO *db, LOGIO *lp, UINT32 flags);

/* idrec.c */
int ida_idrec(IDA *ida, IDA_IDENT_REC *dest, UINT8 *src);

/* logrec.c */
int ida_logrec(IDA *ida, IDA_LOG_REC *dest, UINT8 *src);

/* rtype.c */
int ida_rtype(UINT8 *buf, int rev);

/* sampok.c */
int ida_sampok(IDA_DHDR *dhead);

/* srec.c */
int ida_srec(IDA *ida, IDA_DAS_STATS *dest, UINT8 *src);

/* timerr.c */
int ida_timerr(IDA_DHDR *crnt, IDA_DHDR *prev, int *sign, unsigned long *errptr);
unsigned long ida_offerr(IDA_DHDR *dhead, int *sign, unsigned long *expected);

/* timstr.c */
char *sys_timstr(unsigned long tics, unsigned long mult, char *buf);
char *ext_timstr(unsigned long sec, char *buf);
char *tru_timstr(IDA_TIME_TAG *tag, char *buf);

/* version.c */
char *isidaVersionString(void);
VERSION *isidaVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* ida_h_included */

/* Revision History
 *
 * $Log: ida.h,v $
 * Revision 1.22  2008/03/05 22:46:03  dechavez
 * added subformat and flags to IDA_DHDR, defined TTAG_LEAP_YEAR_PATCH flag bit
 *
 * Revision 1.21  2008/01/16 23:29:38  dechavez
 * updated timstr.c prototypes
 *
 * Revision 1.20  2007/06/01 19:01:09  dechavez
 * defined IDA_EXTRA and replaced explicit structure for extra field in IDA_DHDR with same
 *
 * Revision 1.19  2007/01/25 20:25:49  dechavez
 * IDA9.x (aka RT593) support
 *
 * Revision 1.18  2006/02/14 17:05:04  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.17  2006/02/09 00:05:33  dechavez
 * Introduction of the IDA handle and all the associated changes stemming from
 * that.  To many changes to list.  Moved out IDADB, changed struct xxx to
 * typedef IDA_xxx.
 *
 * Revision 1.16  2005/09/06 18:23:51  dechavez
 * removed ; from end of IDA_REV9_TAG definition
 *
 * Revision 1.15  2005/09/06 18:22:08  dechavez
 * updated prototyypes, added IDA_REV9_TAG
 *
 * Revision 1.14  2005/08/26 18:11:29  dechavez
 * added ida rev9 extra stuff (seqno and tstamp) to data_head
 *
 * Revision 1.13  2005/07/26 00:31:46  dechavez
 * added idaInitDB() and cleaned up integration with DBIO, a bit
 *
 * Revision 1.12  2005/06/10 15:21:40  dechavez
 * added IDA_SERIAL_PORT (however not using it yet)
 *
 * Revision 1.11  2005/05/25 22:46:47  dechavez
 * updated prototypes
 *
 * Revision 1.10  2004/06/25 18:34:56  dechavez
 * C++ compatibility
 *
 * Revision 1.9  2004/06/24 16:56:27  dechavez
 * removed unecessary system includes
 *
 * Revision 1.8  2004/06/21 19:37:40  dechavez
 * changed time_tag "true" field to "tru"
 *
 * Revision 1.7  2004/04/23 00:37:19  dechavez
 * dbio support
 *
 * Revision 1.6  2003/10/16 17:50:03  dechavez
 * added some new general datatypes for future use, updated prototypes
 *
 * Revision 1.5  2002/02/21 22:32:54  dec
 * increase max channel name length to 8 chars
 *
 * Revision 1.4  2001/05/20 15:56:01  dec
 * switch to platform.h MUTEX
 *
 * Revision 1.3  2001/05/07 22:28:11  dec
 * added platform.h, removed CHAR references
 *
 * Revision 1.2  2000/03/16 06:13:14  dec
 * Added sintmap to idadb
 *
 * Revision 1.1.1.1  2000/02/08 20:20:22  dec
 * import existing IDA/NRTS sources
 *
 */
