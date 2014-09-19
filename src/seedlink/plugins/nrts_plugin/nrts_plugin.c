/***************************************************************************** 
 * nrts_plugin.c
 *
 * SeedLink plugin for IRIS/IDA Near-Real Time System
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "isi.h"
#include "util.h"
#include "plugin.h"

#define MYVERSION "1.2 (2010.256)"

#ifndef DEFAULT_SERVER
#define DEFAULT_SERVER "localhost"
#endif

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

#define LOGMSGLEN 150

static const char *const ident_str = "SeedLink NRTS Plugin v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
static const char *const opterr_message = "Try `%s --help' for more information\n";
static const char *const help_message = 
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is used as a signature in log messages\n"
    "\n"
    "-s, --server=HOST             Server address, default is localhost\n"
    "-p, --port=PORT               Port number, default is 39136\n"
    "-m, --match=STREAM_SPEC       Streams to request, eg., \"pfo.bhz.00\"\n"
    "                              requests one specific stream and\n"
    "                              \"pfo.*.*+jts.*.*\" requests all PFO and\n"
    "                              JTS streams. Default is all streams.\n" 
    "-v                            Increase verbosity level\n"
    "    --verbosity=LEVEL         Set verbosity level\n"
    "-D, --daemon                  Daemon mode\n"
    "-V, --version                 Show version information\n"
    "-h, --help                    Show this help message\n";
#else
static const char *const opterr_message = "Try `%s -h' for more information\n";
static const char *const help_message =
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is used as a signature in log messages\n"
    "\n"
    "-s HOST        Server address, default is localhost\n"
    "-p PORT        Port number, default is 39136\n"
    "-m STREAM_SPEC Streams to request, eg., \"pfo.bhz.00\"\n"
    "               requests one specific stream and\n"
    "               \"pfo.*.*+jts.*.*\" requests all PFO and\n"
    "               JTS streams. Default is all streams.\n" 
    "-v             Increase verbosity level\n"
    "-D             Daemon mode\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

static const char *plugin_name = NULL;
static int daemon_mode = 0, daemon_init = 0;
static int verbosity = 0;

static void log_print(char *msg)
  {
    if(daemon_init)
      {
        syslog(LOG_INFO, "%s", msg);
      }
    else
      {
        time_t t = time(NULL);
        char *p = asctime(localtime(&t));
        printf("%.*s - %s: %s\n", strlen(p) - 1, p, plugin_name, msg);
        fflush(stdout);
      }
  }

static int log_printf(const char *fmt, ...)
  {
    char buf[LOGMSGLEN];
    int r;
    va_list ap;

    va_start(ap, fmt);
    r = vsnprintf(buf, LOGMSGLEN, fmt, ap);
    va_end(ap);

    log_print(buf);

    return r;
  }

static void sendTS(ISI_GENERIC_TS *ts)
  {
    char sid[PLUGIN_SIDLEN], cid[PLUGIN_CIDLEN], *p;
    int tqual, r, i;

    if(verbosity > 1)
      {
        char buf[1024];
        log_printf("%s", isiGenericTsHdrString(&ts->hdr, buf));
      }
    
    if(ts->hdr.desc.comp != ISI_COMP_NONE)
      {
        log_printf("unexpected compression %d", ts->hdr.desc.comp);
        return;
      }

    if(ts->hdr.desc.order != ISI_HOST_BYTE_ORDER)
      {
        log_printf("unexpected byte order %d", ts->hdr.desc.order);
        return;
      }

    snprintf(sid, PLUGIN_SIDLEN, "%s", ts->hdr.name.sta);
    for(p = sid; *p; ++p) *p = toupper(*p);

    snprintf(cid, PLUGIN_CIDLEN, "%s%s", ts->hdr.name.loc, ts->hdr.name.chn);
    for(p = cid; *p; ++p) *p = toupper(*p);
    
    tqual = 0;

    if(ts->hdr.tofs.status & ISI_TSTAMP_STATUS_LOCKED)
        tqual = 100;
    else if(ts->hdr.tofs.status & (ISI_TSTAMP_STATUS_AUTOINC|ISI_TSTAMP_STATUS_DERIVED))
        tqual = 10;
    
    if(ts->hdr.desc.type == ISI_TYPE_INT32)
      {
        r = send_raw_depoch(sid, cid, ts->hdr.tofs.value, 0, tqual,
          ts->data, ts->hdr.nbytes >> 2);
      }
    else if(ts->hdr.desc.type == ISI_TYPE_INT16)
      {
        int32_t *data_buf;
        if((data_buf = malloc(ts->hdr.nbytes << 1)) == NULL)
          {
            log_printf("malloc: %s", strerror(errno));
            exit(1);
          }

        for(i = 0; i < (ts->hdr.nbytes >> 1); ++i)
            data_buf[i] = ((int16_t *)ts->data)[i];

        r = send_raw_depoch(sid, cid, ts->hdr.tofs.value, 0, tqual,
          data_buf, ts->hdr.nbytes >> 1);

        free(data_buf);
      }
    else
      {
        log_printf("unexpected data type %d", ts->hdr.desc.type);
        return;
      }

    if(r < 0)
      {
        log_printf("error sending data to seedlink: %s", strerror(errno));
        exit(1);
      }
    else if(r == 0)
      {
        log_printf("error sending data to seedlink");
        exit(1);
      }
  }

static const char *get_progname(const char *argv0)
  {
    const char *p;

    if((p = strrchr(argv0, '/')) != NULL)
        return (p + 1);

    return argv0;
  }

int main (int argc, char **argv)
  {
    ISI *isi;
    ISI_PARAM par;
    ISI_DATA_REQUEST *dreq;
    ISI_GENERIC_TS *ts;
    int status;
    char *stream_spec = NULL;
    char *server = DEFAULT_SERVER;
    int port = 0; /* default */

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    struct option ops[] = 
      {
        { "server",         required_argument, NULL, 's' },
        { "port",           required_argument, NULL, 'p' },
        { "match",          required_argument, NULL, 'm' },
        { "verbosity",      required_argument, NULL, 'X' },
        { "daemon",         no_argument,       NULL, 'D' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    while((c = getopt_long(argc, argv, "s:p:m:vDVh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "s:p:m:vDVh")) != EOF)
#endif
      {
        switch(c)
          {
          case 's':
            if((server = strdup(optarg)) == NULL)
              {
                perror("strdup");
                exit(1);
              }
            
            break;
            
          case 'm':
            if((stream_spec = strdup(optarg)) == NULL)
              {
                perror("strdup");
                exit(1);
              }

            break;

          case 'p': port = atoi(optarg); break;
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'D': daemon_mode = 1; break;
          case 'V': fprintf(stdout, "%s\n", ident_str);
                    exit(0);
          case 'h': fprintf(stdout, help_message, get_progname(argv[0]));
                    exit(0);
          case '?': fprintf(stderr, opterr_message, get_progname(argv[0]));
                    exit(1);
          }
      }

    if(optind != argc - 1)
      {
        fprintf(stderr, help_message, get_progname(argv[0]));
        exit(1);
      }

    if((plugin_name = strdup(argv[optind])) == NULL)
      {
        perror("strdup");
        exit(1);
      }
    
    if(daemon_mode)
      {
        log_printf("%s started", ident_str);
        log_printf("take a look into syslog files for more messages");
        openlog(plugin_name, 0, SYSLOG_FACILITY);
        daemon_init = 1;
      }
    
    log_printf("%s started", ident_str);

    utilNetworkInit();
    isiInitDefaultPar(&par);
    isiStartLogging(&par, NULL, log_print, NULL);
    isiSetDebugFlag(&par, verbosity);

    if(port != 0)
        isiSetServerPort(&par, port);

    if ((dreq = isiAllocSimpleDataRequest(ISI_NEWEST, ISI_KEEPUP,
      stream_spec)) == NULL)
      {
        log_printf("isiAllocSimpleDataRequest: %s", strerror(errno));
        exit(1);
      }

    isiSetDatreqFormat(dreq, ISI_FORMAT_GENERIC);
    isiSetDatreqCompress(dreq, ISI_COMP_NONE);

    if ((isi = isiInitiateDataRequest(server, &par, dreq)) == NULL)
      {
        log_printf("isiInitiateDataRequest: %s", strerror(errno));
        exit(1);
      }

    isiFreeDataRequest(dreq);

    while((ts = isiReadGenericTS(isi, &status)) != NULL)
        sendTS(ts);

    if (status != ISI_DONE)
      {
        log_printf("isiReadGenericTS: %s", strerror(errno));
        return 1;
      }

    return 0;
  }

