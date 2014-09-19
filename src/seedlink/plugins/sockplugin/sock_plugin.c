/*
 * sock_plugin.c:
 * A SeedLink plugin to collect data from a network socket.
 *
 * This code connects to a TCP socket, collects any data that is written
 * by the serving process, checks for a 1000 blockette and passes 
 * records on to SeedLink.
 *
 * The serving process could be a LISS server or anything else that
 * is serving 512-byte MiniSEED records, e.g. the 'tdatasock' ComServ
 * client.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "seedutils.h"

#define PACKAGE "sock_plugin"
#define VERSION "2006.310"

unsigned int stopsig = 0;
int verbose = 0;
int ignoreeof = 0;
int port = 4000;
char *host = '\0';

int seed_size = 512;      /* OK default for now */
unsigned char buf[1024];  /* The data buffer */

int lprintf(int level, const char *fmt, ...);

void usage(int argc,char **argv)
{

  fprintf(stderr,"\n");
  fprintf(stderr,"Usage: %s [options] -s host\n", PACKAGE);
  fprintf(stderr,"Version %s\n", VERSION);
  fprintf(stderr,"\n");
  fprintf(stderr,"Required:\n");
  fprintf(stderr,"    -s host    host serving data, required\n\n");
  fprintf(stderr,"Options:\n");
  fprintf(stderr,"    -p port    port number, default is 4000\n");
  fprintf(stderr,"    -m match   regular expression to match source name\n");
  fprintf(stderr,"                using a 'net_sta_loc_chan' convention,\n");
  fprintf(stderr,"                i.e. 'IU_KONO_00_BH?' or 'GE_WLF_.*'\n");
  fprintf(stderr,"    -v         verbose, print diagnostic messages\n");
  fprintf(stderr,"                if two are used, each packet is reported\n");
  fprintf(stderr,"    -n code    force network code (after matching), 1 or 2 chars\n");
  fprintf(stderr,"    -E         ignore EOF - keep reconnecting to port\n");
  fprintf(stderr,"    -r secs    sleep time in seonds before reconnecting\n");
  fprintf(stderr,"                after an EOF or connect failure, implies\n");
  fprintf(stderr,"                the -E flag, default is 30 seconds\n");
  fprintf(stderr,"\n");

  exit(1);
}

int main(int argc,char **argv)
{

  extern char *optarg;
  extern int optind;
  char *match = '\0';
  char *netcode = 0;
  int errflg=0;
  int i, c;
  int sleeptime = 30;

  if (argc < 2) {
    errflg++;
  }

  while ((c = getopt(argc, argv, "vEr:p:s:m:n:")) != EOF)
    switch (c) {
    case 'v':
      verbose++;
      break;
    case 'E':
      ignoreeof++;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'm':
      match = (char *) strdup(optarg);
      break;
    case 'n':
      if ( strlen(optarg) > 2 ) {
	lprintf(0,"Network code can only be 1 or 2 characters");
	exit (1);
      }
      netcode = (char *) malloc (3);
      sprintf (netcode, "%-2.2s", optarg);
      netcode[2] = '\0';
      break;
    case 's':
      host = (char *) strdup(optarg);
      break;
    case 'r':
      sleeptime = atoi(optarg);
      ignoreeof++;
      break;
    default:
      errflg++;
    }

  if ( errflg ) 
    usage(argc,argv);

  if ( host == '\0' ) {
    lprintf(0,"Remote hostname is required! Use the -s argument");
    exit(1);
  }

  lprintf(2,"Run time parameters:");
  lprintf(2,"verbose:   %d", verbose);
  if ( netcode )
    lprintf(1,"Forcing network code: '%s'", netcode);
  lprintf(2,"ignoreeof: %d", ignoreeof);
  lprintf(2,"host: %s", host);
  lprintf(2,"port: %d", port);

  while ( 1 ) {
    i = process_telemetry(host, port, buf, match, netcode, seed_size);
    if (!ignoreeof && !i) break;
    sleep(sleeptime);
  }

  exit(0);

}

/* A generic log message print handler */
int lprintf(int level, const char *fmt, ...)
{
  int r = 0;
  char message[100];
  char timestr[100];
  va_list argptr;
  time_t loc_time;

  if ( level <= verbose ) {

    va_start(argptr, fmt);

    /* Build local time string and cut off the newline */
    time(&loc_time);
    strcpy(timestr, asctime(localtime(&loc_time)));
    timestr[strlen(timestr) - 1] = '\0';

    r = vsnprintf(message, sizeof(message), fmt, argptr);

    printf("%s - sock_plugin: %s\n", timestr, message);
    fflush(stdout);
  }
  
  return r;
}
