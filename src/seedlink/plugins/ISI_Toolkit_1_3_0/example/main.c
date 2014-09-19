#pragma ident "$Id: main.c,v 1.26 2008/01/10 16:27:42 dechavez Exp $"
/*======================================================================
 *
 *  General purpose ISI client
 *
 *====================================================================*/
#define INCLUDE_ISI_STATIC_SEQNOS
#include "isi.h"
#include "util.h"
#ifdef INCLUDE_DL_OPTION
#include "isi/dl.h"
ISI_DL *dl     = NULL;
#endif /* INCLUDE_DL_OPTION */

#define VERBOSE_REPORT  0x00000001
#define TERSE_REPORT    0x00000002
#define LATENCY_REPORT  0x00000004
#define TIMETEAR_REPORT 0x00000008
#define VERBOSE         (flags & VERBOSE_REPORT)
#define TERSE           (flags & VERBOSE_REPORT) || (flags & TERSE_REPORT)
#define PRINT_LATENCY   (flags & LATENCY_REPORT)
#define TRACK_TIMETEARS (flags & TIMETEAR_REPORT)

extern char *VersionIdentString;

#define LOCALBUFLEN 1024

#ifndef DEFAULT_SERVER
#define DEFAULT_SERVER "localhost"
#endif /* DEFAULT_SERVER */

#define UNKNOWN 0
#define DATA    1
#define CNF     2
#define SOH     3
#define WD      4
#define RAW     5

UINT32 flags   = TERSE_REPORT;
char *server   = DEFAULT_SERVER;
char *datdir   = NULL;
static char *StandardOutput = "stdout";

#ifdef INCLUDE_DL_OPTION
static ISI_DL *OpenDiskLoop(char *dbspec, char *myname, char *site)
{
ISI_GLOB glob;
static char *defdb = ISI_DL_DEFAULT_DBID;

    if (dbspec == NULL) dbspec = defdb;

    if (!isidlSetGlobalParameters(dbspec, myname, &glob)) {
        fprintf(stderr, "isidlSetGlobalParameters: %s\n", strerror(errno));
        exit(1);
    }
    glob.flags |= ISI_DL_FLAGS_TIME_DL_OPS;

    if((dl = isidlOpenDiskLoop(&glob, site, NULL, ISI_RDWR)) == NULL) {
        fprintf(stderr, "isidlOpenDiskLoop: %s: %s\n", site, strerror(errno));
        exit(1);
    }
}

static void SetRawBegEnd(ISI_DL *dl, ISI_SEQNO *begseqno, ISI_SEQNO *endseqno)
{
ISI_SEQNO yngest;

    yngest = isidlPacketSeqno(dl, dl->sys->index.yngest);
    if (isiIsUndefinedSeqno(&yngest)) {
        *begseqno = ISI_OLDEST_SEQNO;
    } else {
        ++yngest.counter;
        *begseqno = yngest;
    }

    *endseqno = ISI_KEEPUP_SEQNO;
}
#endif /* INCLUDE_DL_OPTION */

static void WriteGenericTS(ISI_GENERIC_TS *ts)
{
FILE *fp;
char parent[MAXPATHLEN+1], path[MAXPATHLEN+1];

    if (strcmp(datdir, StandardOutput) != 0) {
        MKDIR(datdir, 0775);
        sprintf(parent, "%s/%s", datdir, ts->hdr.name.sta);
        MKDIR(parent, 0775);

        if (strlen(ts->hdr.name.loc) != 0) {
            sprintf(path, "%s/%s%s", parent, ts->hdr.name.chn, ts->hdr.name.loc);
        } else {
            sprintf(path, "%s/%s", parent, ts->hdr.name.chn);
        }

        if ((fp = fopen(path, "ab")) == NULL) {
            fprintf(stderr, "fopen: ");
            perror(path);
            exit(1);
        }
    } else {
        fp = stdout;
    }

/* non-native packets include the generic header */

    if (!isiIsNative(&ts->hdr.desc)) {
        int len;
        char buf[sizeof(ISI_GENERIC_TSHDR)*2];
        len = isiPackGenericTSHDR((UINT8 *) buf, &ts->hdr);
        if ((fwrite(buf, sizeof(UINT8), len, fp)) != (size_t) len) {
            fprintf(stderr, "fwrite: ");
            perror(path);
            exit(1);
        }
    }

/* Save the payload */

    if ((fwrite(ts->data, sizeof(UINT8), ts->hdr.nbytes, fp)) != ts->hdr.nbytes) {
        fprintf(stderr, "fwrite: ");
        perror(path);
        exit(1);
    }
    if (fp != stdout) {
        fclose(fp);
    } else {
        fflush(fp);
    }
}

static ISI_DATA_REQUEST *BuildDataRequest(int format, int compress, REAL64 begtime, REAL64 endtime, char *StreamSpec)
{
ISI_DATA_REQUEST *dreq;

    if ((dreq = isiAllocSimpleDataRequest(begtime, endtime, StreamSpec)) == NULL) {
        fprintf(stderr, "isiAllocSimpleDataRequest: %s\n", strerror(errno));
        exit(1);
    }
    isiSetDatreqFormat(dreq, format);
    isiSetDatreqCompress(dreq, compress);

    return dreq;
}

static void CompareHeaders(ISI_GENERIC_TSHDR *prev, ISI_GENERIC_TSHDR *crnt, char *buf)
{
struct {
    INT32 samples;
    REAL64 seconds;
} ttear;
REAL64 sint, sint2, increment;
char name[ISI_STREAM_NAME_LEN+1];

    isiStreamNameString(&crnt->name, name);

    sint  = isiSrateToSint(&prev->srate);
    sint2 = isiSrateToSint(&crnt->srate);
    if (sint != sint2) {
        logioMsg(NULL, LOG_INFO, "%s sample rate change from %.3lf to %.3lf\n", name, sint, sint2);
        printf("prev: %s\n", isiGenericTsHdrString(prev, (char *) buf));
        printf("crnt: %s\n", isiGenericTsHdrString(crnt, (char *) buf));
        printf("\n");
        fflush(stdout);
        return;
    }

    increment = crnt->tofs.value - prev->tols.value;
    ttear.seconds = increment - sint;
    ttear.samples = (INT32) (ttear.seconds / sint);

    if (ttear.samples != 0) {
        logioMsg(NULL, LOG_INFO, "%s time tear of %.3lf seconds (%s, %ld samples)\n", name, ttear.seconds, utilDttostr(ttear.seconds, 8, buf), ttear.samples);
        printf("prev: %s\n", isiGenericTsHdrString(prev, (char *) buf));
        printf("crnt: %s\n", isiGenericTsHdrString(crnt, (char *) buf));
        printf("\n");
        fflush(stdout);
    }
}

static ISI_GENERIC_TSHDR *PreviousHeader(ISI_GENERIC_TSHDR *target, LNKLST *head)
{
ISI_GENERIC_TSHDR *hdr;
LNKLST_NODE *crnt;

    crnt = listFirstNode(head);
    while (crnt != NULL) {
        hdr = (ISI_GENERIC_TSHDR *) crnt->payload;
        if (isiStreamNameCompare(&hdr->name, &target->name) == 0) return hdr;
        crnt = listNextNode(crnt);
    }

    return NULL;
}

static void CheckHeaders(ISI_GENERIC_TSHDR *crnt, LNKLST *head, char *buf)
{
ISI_GENERIC_TSHDR *prev;

    if ((prev = PreviousHeader(crnt, head)) != NULL) {
        CompareHeaders(prev, crnt, buf);
        memcpy(prev, crnt, sizeof(ISI_GENERIC_TSHDR));
    } else {
        listAppend(head, (void *) crnt, sizeof(ISI_GENERIC_TSHDR));
    }
}

static void data(char *server, ISI_PARAM *par, int format, int compress, REAL64 begtime, REAL64 endtime, char *StreamSpec)
{
ISI *isi;
int status;
INT32 latency;
ISI_GENERIC_TS *ts;
ISI_DATA_REQUEST *dreq;
UINT8 buf[LOCALBUFLEN];
LNKLST *history;

    if ((history = listCreate()) == NULL) {
        perror("listCreate");
        return;
    }

    dreq = BuildDataRequest(format, compress, begtime, endtime, StreamSpec);

    if (VERBOSE) {
        fprintf(stderr, "Client side data request\n");
        isiPrintDatreq(stderr, dreq);
    }

    if ((isi = isiInitiateDataRequest(server, par, dreq)) == NULL) {
        if (errno == ENOENT) {
            fprintf(stderr, "can't connect to server %s, port %d\n", server, par->port);
        } else {
            perror("isiInitiateDataRequest");
        }
        exit(1);
    }

    isiFreeDataRequest(dreq);

    if (VERBOSE) {
        fprintf(stderr, "Server expanded data request\n");
        isiPrintDatreq(stderr, &isi->datreq);
    }

    while ((ts = isiReadGenericTS(isi, &status)) != NULL) {
        fprintf(stderr, "%s", isiGenericTsHdrString(&ts->hdr, (char *) buf));
        if (PRINT_LATENCY) {
            latency = (INT32) time(NULL) - (INT32) ts->hdr.tols.value;
            fprintf(stderr, " %s", utilLttostr(latency, 8, (char *) buf));
        }
        fprintf(stderr, "\n");
        if (datdir != NULL) WriteGenericTS(ts);
        if (TRACK_TIMETEARS) CheckHeaders(&ts->hdr, history, (char *) buf);
    }

    if (status != ISI_DONE) perror("isiReadGenericTS");
}

static ISI_DATA_REQUEST *BuildRawRequest(int compress, ISI_SEQNO *begseqno, ISI_SEQNO *endseqno, char *SiteSpec)
{
ISI_DATA_REQUEST *dreq;

    if ((dreq = isiAllocSimpleSeqnoRequest(begseqno, endseqno, SiteSpec)) == NULL) {
        fprintf(stderr, "isiAllocSimpleSeqnoRequest: %s\n", strerror(errno));
        exit(1);
    }
    isiSetDatreqCompress(dreq, compress);

    return dreq;
}

static BOOL ReadRawPacket(ISI *isi, ISI_RAW_PACKET *raw)
{
int status;

    while ((status = isiReadFrame(isi, TRUE)) == ISI_OK) {
        if (isi->frame.payload.type != ISI_IACP_RAW_PKT) {
            fprintf(stderr, "unexpected type %d packet ignored\n", isi->frame.payload.type);
        } else {
            isiUnpackRawPacket(isi->frame.payload.data, raw);
            return TRUE;
        }
    }

    switch (status) {
      case ISI_DONE:
        fprintf(stderr, "request complete\n");
        break;

      case ISI_BREAK:
        fprintf(stderr, "server disconnect\n");
        break;

      default:
        perror("isiReadFrame");
        break;
    }

    return FALSE;
}

static void WriteRawPacket(ISI_RAW_PACKET *raw, UINT8 *buf, UINT32 buflen)
{
FILE *fp;
char path[MAXPATHLEN+1];

    if (strcmp(datdir, StandardOutput) != 0) {
        MKDIR(datdir, 0775);
        sprintf(path, "%s/%s", datdir, raw->hdr.site);

        if ((fp = fopen(path, "ab")) == NULL) {
            fprintf(stderr, "fopen: ");
            perror(path);
            exit(1);
        }
    } else {
        fp = stdout;
    }

    if (fwrite(raw->payload, 1, raw->hdr.len.used, fp) != raw->hdr.len.used) {
        perror("fwrite");
        exit(1);
    }
    if (fp != stdout) {
        fclose(fp);
    } else {
        fflush(fp);
    }
}

static void raw(char *server, ISI_PARAM *par, int compress, ISI_SEQNO *begseqno, ISI_SEQNO *endseqno, char *SiteSpec)
{
ISI *isi;
ISI_RAW_PACKET raw;
ISI_DATA_REQUEST *dreq;
#define LOCALBUFLEN 1024
UINT8 buf[LOCALBUFLEN];
UINT64 count = 0;

    dreq = BuildRawRequest(compress, begseqno, endseqno, SiteSpec);

    if (VERBOSE) {
        fprintf(stderr, "Client side data request\n");
        isiPrintDatreq(stderr, dreq);
    }

    if ((isi = isiInitiateDataRequest(server, par, dreq)) == NULL) {
        if (errno == ENOENT) {
            fprintf(stderr, "can't connect to server %s, port %d\n", server, par->port);
        } else {
            perror("isiInitiateDataRequest");
        }
        exit(1);
    }

    isiFreeDataRequest(dreq);

    if (VERBOSE) {
        fprintf(stderr, "Server expanded data request\n");
        isiPrintDatreq(stderr, &isi->datreq);
    }

    while (ReadRawPacket(isi, &raw)) {
        ++count;
        if (TERSE) fprintf(stderr, "%s\n", isiRawHeaderString(&raw.hdr, buf));
        if (!isiDecompressRawPacket(&raw, buf, LOCALBUFLEN)) {
            fprintf(stderr, "isiDecompressRawPacket error\n");
        } else if (VERBOSE) {
            utilPrintHexDump(stderr, raw.payload, 64);
        }
        if (datdir != NULL) WriteRawPacket(&raw, buf, LOCALBUFLEN);
#ifdef INCLUDE_DL_OPTION
        if (dl != NULL) isidlWriteToDiskLoop(dl, &raw, 0);
#endif /* INCLUDE_DL_OPTION */
    }

    fprintf(stderr, "%llu packets received\n", count);
}

static void DisplaySohReport(ISI_SOH_REPORT *report)
{
int i;

    if (VERBOSE) printf(
        "  Sta Chn Loc  Nseg     Nrec  Oldest Sample Time   "
        "Youngest Sample Time     Last Write\n"
    );

    for (i = 0; i < (int) report->nentry; i++) isiPrintStreamSoh(stdout, &report->entry[i]);
}

static void soh(char *server, ISI_PARAM *par)
{
ISI_SOH_REPORT *report;

    if ((report = isiSoh(server, par)) == NULL) {
        perror("isiSoh");
        exit(1);
    }

    DisplaySohReport(report);
    isiFreeSoh(report);
}

static void cnf(char *server, ISI_PARAM *par)
{
int i;
ISI_CNF_REPORT *report;

    if ((report = isiCnf(server, par)) == NULL) {
        perror("isiCnf");
        exit(1);
    }

    if (VERBOSE) printf(
        "  Sta Chn Loc    Sint    Lat       Long        Elev"
        "     Depth     Calib      Calper    Hang    Vang   Inst\n"
    );

    for (i = 0; i < (int) report->nentry; i++) isiPrintStreamCnf(stdout, &report->entry[i]);
    isiFreeCnf(report);
}

static void wd(char *server, ISI_PARAM *par, int maxdur)
{
int i;
ISI_WFDISC_REPORT *report;

    if ((report = isiWfdisc(server, par, maxdur)) == NULL) {
        perror("isiWfdisc");
        exit(1);
    }

    for (i = 0; i < (int) report->nentry; i++) printf("%s\n", report->entry[i]);
    isiFreeWfdisc(report);
}

static void help(char *myname)
{
static char *VerboseHelp = 
"The arguments in [ square brackets ] are optional:\n"
"\n"
"           -v - turns on verbose isi output\n"
"           -q - turns off all commentary\n"
"server=string - sets the name of the server to the specified string.\n"
"     port=int - sets the port number\n"
"   log=string - turns on library logging using. 'string' is either the\n"
"                name of the log file, '-' for stdout, or 'syslogd:facility'\n"
"    debug=int - sets the debug level (0, 1, 2)\n"
"\n"
"You must specify ONE of the arguments in { curly brackets }.\n"
"\n"
"    cnf - configuration report\n"
"    soh - state of health report\n"
"     wd - disk loop wfdisc dump\n"
"   data - waveform request\n"
"    raw - raw data request\n"
"\n"
"Data feeds are done using either the 'data' or 'raw' argument, depending on if\n"
"you want to access the NRTS (time-based) or ISI (sequence number based) disk\n"
"loops, respectively.  By default, a 'data' request defaults to all streams,\n"
"starting with the most current data and continuing indefinitely.  The optional\n"
"'=spec' suffix allows you to select specific streams.  The format of the 'spec'\n"
"string is dot delimited 'sta.chn.loc'.  If you use wild cards, be sure to quote\n"
"the string so the shell won't get confused.  You may string together multiple\n"
"spec strings, delimited by a plus (sta1.chn.loc+sta2.chn.loc, etc).  You can\n"
"specify start and end times for a 'data' request using optional 'beg=str' and\n"
"'end=str' arguments.  The format of the 'str' string is the desired date and\n"
"time as YYYY:DDD-hh:mm:ss.  You can truncate from the right, in which case the\n"
"missing fields are assumed to be zero (except for a missing DDD, which is\n"
"assumed to be 001).  For example:\n"
"\n"
"isi data=\"pfo.*.*\" beg=2007:001-10:00 end=2007:002-12:34\n"
"\n"
"A 'raw' feed defaults to all packets from all data sources, starting with the\n"
"most recent packet and continuing indefinitely.  The optional '=spec' suffix\n"
"allows you to select specific data sources.  For station systems this is not\n"
"usually needed, but it is if requesting data from a hub and you only want to\n"
"get packets from a particular station.  You can specify start and end points\n"
"of the request using the optional 'beg=str' and 'end=str' arguments.  In this\n"
"case the format of the 'str' string is the desired sequence number given in\n"
"hex. For example:\n"
"\n"
"isi raw=pfo beg=45b566150000000000a613a0 end=45b5661500000000009ea391\n"
"\n"
"Both the 'data' and 'raw' requests have an optional 'datdir' argument that can\n"
"be used to to save the data frames in the indicated directory.  Without 'datdir'\n"
"then all you get are packet header summaries printed to stderr.  You can set\n"
"the datdir to \"-\" or \"stdout\" to write the data to standard output.  For\n"
"example\n"
"\n"
"isi server=idahub raw=pfo datdir=- | ida10 -v -noseqno\n"
"\n"
"The following options apply to the 'data' runs:\n"
"\n"
"           -l - include latency (data option)\n"
"           -t - note time tears (data option)\n";

    fprintf(stderr,"usage: %s ", myname);
    fprintf(stderr, "[ -v -q server=string port=int log=string debug=int ] ");
    fprintf(stderr, "{ data[=spec] [datdir=path beg=str end=str] | raw[=spec] [datdir=path beg=str end=str] | cnf | soh | wd }\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "%s", VerboseHelp);
    fprintf(stderr, "\n");
    fprintf(stderr, "default server is `%s'\n", DEFAULT_SERVER);
    exit(1);

}

int main (int argc, char **argv)
{
int i, request, port, debug;
ISI_PARAM par;
char *req = NULL;
char *log;
char *StreamSpec = NULL;
char *SiteSpec = NULL;
char *begstr = NULL;
char *endstr = NULL;
REAL64 begtime = ISI_NEWEST;
REAL64 endtime = ISI_KEEPUP;
ISI_SEQNO begseqno = ISI_NEWEST_SEQNO;
ISI_SEQNO endseqno = ISI_KEEPUP_SEQNO;
char *tmpstr;
int maxdur    = 0;
int compress  = ISI_COMP_NONE;
int format    = ISI_FORMAT_GENERIC;
char *dbspec  = NULL;

    utilNetworkInit();
    isiInitDefaultPar(&par);

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "server=", strlen("server=")) == 0) {
            server = argv[i] + strlen("server=");
        } else if (strncmp(argv[i], "port=", strlen("port=")) == 0) {
            port = atoi(argv[i] + strlen("port="));
            isiSetServerPort(&par, port);
        } else if (strncmp(argv[i], "beg=", strlen("beg=")) == 0) {
            begstr = argv[i] + strlen("beg=");
        } else if (strncmp(argv[i], "end=", strlen("end=")) == 0) {
            endstr = argv[i] + strlen("end=");
        } else if (strncmp(argv[i], "format=", strlen("format=")) == 0) {
            tmpstr = argv[i] + strlen("format=");
            if (strcasecmp(tmpstr, "native") == 0) {
                format = ISI_FORMAT_NATIVE;
            } else if (strcasecmp(tmpstr, "generic") == 0) {
                format = ISI_FORMAT_GENERIC;
            } else {
                fprintf(stderr, "%s: unsupported format type '%s'\n", argv[0], tmpstr);
                help(argv[0]);
            }
        } else if (strncmp(argv[i], "comp=", strlen("comp=")) == 0) {
            tmpstr = argv[i] + strlen("comp=");
            if (strcasecmp(tmpstr, "none") == 0) {
                compress = ISI_COMP_NONE;
            } else if (strcasecmp(tmpstr, "ida") == 0) {
                compress = ISI_COMP_IDA;
            } else if (strcasecmp(tmpstr, "gzip") == 0) {
                compress = ISI_COMP_GZIP;
            } else {
                fprintf(stderr, "%s: unsupported comp type '%s'\n", argv[0], tmpstr);
                help(argv[0]);
            }
        } else if (strncmp(argv[i], "datdir=", strlen("datdir=")) == 0) {
            datdir = argv[i] + strlen("datdir=");
            if (strcmp(datdir, "-") == 0) datdir = StandardOutput;
#ifdef INCLUDE_DL_OPTION
        } else if (strncmp(argv[i], "db=", strlen("db=")) == 0) {
            dbspec = argv[i] + strlen("db=");
        } else if (strncasecmp(argv[i], "dl=", strlen("dl=")) == 0) {
            dl = OpenDiskLoop(dbspec, argv[0], argv[i]+strlen("dl="));
#endif /* INCLUDE_DL_OPTION */
        } else if (strncmp(argv[i], "debug=", strlen("debug=")) == 0) {
            debug = atoi(argv[i] + strlen("debug="));
            isiSetDebugFlag(&par, debug);
        } else if (strncmp(argv[i], "maxdur=", strlen("maxdur=")) == 0) {
            maxdur = atoi(argv[i] + strlen("maxdur="));
        } else if (strncmp(argv[i], "log=", strlen("log=")) == 0) {
            log = argv[i] + strlen("log=");
            isiStartLogging(&par, log, NULL, argv[0]);
        } else if (strncmp(argv[i], "dbgpath=", strlen("dbgpath=")) == 0) {
            isiSetDbgpath(&par, argv[i] + strlen("dbgpath="));
        } else if (strcmp(argv[i], "-v") == 0) {
            flags |= VERBOSE_REPORT;
        } else if (strcmp(argv[i], "-q") == 0) {
            flags &= ~(TERSE_REPORT | VERBOSE_REPORT);
        } else if (strcmp(argv[i], "-l") == 0) {
            flags |= LATENCY_REPORT;
        } else if (strcmp(argv[i], "-t") == 0) {
            flags |= TIMETEAR_REPORT;
        } else if (req == NULL) {
            req = argv[i];
        } else {
           fprintf(stderr, "%s: unrecognized argument: '%s'\n", argv[0], argv[i]);
            help(argv[0]);
        }
    }

    if (datdir != NULL && strcmp(datdir, StandardOutput) == 0) flags &= ~VERBOSE_REPORT;
    if (req == NULL) help(argv[0]);

    if (VERBOSE) fprintf(stderr, "%s %s\n", argv[0], VersionIdentString);

    request = UNKNOWN;
    if (strcasecmp(req, "data") == 0) {
        request = DATA;
        StreamSpec = NULL;
    } else if (strncasecmp(req, "data=", strlen("data=")) == 0) {
        request = DATA;
        StreamSpec = req + strlen("data=");
        if (strlen(StreamSpec) == 0) StreamSpec = NULL;
    } else if (strcasecmp(req, "raw") == 0) {
        request = RAW;
        SiteSpec = NULL;
    } else if (strncasecmp(req, "raw=", strlen("raw=")) == 0) {
        request = RAW;
        SiteSpec = req + strlen("raw=");
        if (strlen(SiteSpec) == 0) SiteSpec = NULL;
    } else if (strcmp(req, "cnf") == 0) {
        request = CNF;
    } else if (strcmp(req, "soh") == 0) {
        request = SOH;
    } else if (strcmp(req, "wd") == 0) {
        request = WD;
    } else if (strcmp(req, "wfdisc") == 0) {
        request = WD;
    } else {
        help(argv[0]);
    }

    switch (request) {
      case DATA: 
        if (VERBOSE) fprintf(stderr, "%s data request\n", server);
        if (begstr != NULL) begtime = utilAttodt(begstr);
        if (endstr != NULL) endtime = utilAttodt(endstr);
        data(server, &par, format, compress, begtime, endtime, StreamSpec);
        break;
      case RAW: 
        if (VERBOSE) fprintf(stderr, "%s raw data request\n", server);
#ifdef INCLUDE_DL_OPTION
        if (dl != NULL) SetRawBegEnd(dl, &begseqno, &endseqno);
#endif /* INCLUDE_DL_OPTION */
        if (begstr != NULL && !isiStringToSeqno(begstr, &begseqno)) {
            fprintf(stderr, "illegal beg seqno '%s'\n", begstr);
            exit(1);
        }
        if (endstr != NULL && !isiStringToSeqno(endstr, &endseqno)) {
            fprintf(stderr, "illegal end seqno '%s'\n", endstr);
            exit(1);
        }
        raw(server, &par, compress, &begseqno, &endseqno, SiteSpec);
        break;
      case CNF:
        if (VERBOSE) fprintf(stderr, "%s configuration request\n", server);
        cnf(server, &par);
        break;
      case SOH:
        if (VERBOSE) fprintf(stderr, "%s state of health request\n", server);
        soh(server, &par);
        break;
      case  WD:
        if (VERBOSE) fprintf(stderr, "%s wfdisc request\n", server);
        wd(server, &par, maxdur);
        break;
      default:
        help(argv[0]);
    }

    exit(0);
}

/* Revision History
 *
 * $Log: main.c,v $
 * Revision 1.26  2008/01/10 16:27:42  dechavez
 * Added TERSE_REPORT flag (set by default), added -q option which clears both
 * TERSE_REPORT and VERBOSE_REPORT, raw feed header displayed if either is set
 *
 * Revision 1.25  2007/06/28 19:45:51  dechavez
 * Made help even more verbose
 *
 * Revision 1.24  2007/06/01 19:27:20  dechavez
 * 1.10.1
 *
 * Revision 1.23  2007/05/17 22:20:26  dechavez
 * Added -t option to track time tears
 *
 * Revision 1.22  2007/03/26 21:37:33  dechavez
 * Print headers on vanilla "data" request (got lost in 1.9.0?)
 *
 * Revision 1.21  2007/02/20 02:24:30  dechavez
 * Added conditional dl option, fixed up verbosity handling
 *
 * Revision 1.20  2007/01/25 20:55:35  dechavez
 * added standard output option for data feeds
 *
 * Revision 1.19  2006/08/15 01:25:47  dechavez
 * Added -l option for including latency reports in data requests
 *
 * Revision 1.18  2005/09/10 22:10:00  dechavez
 * Hex dump raw headers when -v option is given, improve reporting on reason
 * for quitting when server sends an alert
 *
 * Revision 1.17  2005/07/06 15:57:32  dechavez
 * changed behavior of raw option to mimic that of data (ie, only print header
 * by default and output data only if datdir is given)
 *
 * Revision 1.16  2005/06/30 01:45:42  dechavez
 * raw (seqno) support tested
 *
 * Revision 1.15  2005/06/24 21:55:38  dechavez
 * untested checkpoint with preliminary support for raw (seqno) requests
 *
 * Revision 1.14  2005/06/10 15:39:15  dechavez
 * Rename isiSetLogging() to isiStartLogging()
 *
 * Revision 1.13  2005/05/25 23:54:11  dechavez
 * Changes to calm Visual C++ warnings
 *
 * Revision 1.12  2005/01/28 02:00:54  dechavez
 * added dbgpath option
 *
 * Revision 1.11  2004/09/29 18:22:51  dechavez
 * corrected help message typo
 *
 * Revision 1.10  2004/04/26 21:22:41  dechavez
 * improved(?) help message, made uncompressed data the default
 *
 * Revision 1.9  2004/01/29 19:08:41  dechavez
 * use new isiFreeSoh(), isiFreeCnf(), isiFreeWfdisc()
 *
 * Revision 1.8  2003/12/23 00:12:40  dechavez
 * print out default in help message
 * allow for compile time override of default server define
 *
 * Revision 1.7  2003/12/11 22:27:10  dechavez
 * Changed data() to include building and freeing data request
 * Relink with libisi 1.4.7
 *
 * Revision 1.6  2003/11/21 20:27:26  dechavez
 * Changed default server to idahub (bad idea?) and corrected test for
 * isiReadGenericTS() failure cause
 *
 * Revision 1.5  2003/11/19 23:52:57  dechavez
 * print server name and operation when verbose option selected
 *
 * Revision 1.4  2003/11/19 21:35:55  dechavez
 * Rework to use simplified ISI API (libisi 1.4.0), and to include header in generic
 * data disk dumps
 *
 * Revision 1.3  2003/11/13 19:44:35  dechavez
 * Rework to use simplified ISI API (libisi 1.3.0)
 *
 * Revision 1.2  2003/11/04 00:03:24  dechavez
 * Rework to use ISI handle and related functions from version 1.1.0 of the library
 *
 * Revision 1.1  2003/10/16 18:05:23  dechavez
 * initial release
 *
 */
