/**************************************************************************

  naqs_plugin.c:  A SeedLink plugin for collecting data from a Nanometrics
                  NaqsServer (NAQS).

  Based on the example code (dsClient.cpp) distrubuted by Nanometrics.
  The original copyright notice is below.  Notification was given that
  "there are no restrictions regarding the use of the dsClient.cpp snippet"
  (Neil Spriggs, Nanometrics, 7-2-2002).

  Slapped together by
  Chad Trabant ORFEUS/EC-Project MEREDIAN

  --------- Original notice ---------
  File:  dsClient.c

  Description:  Example DataStream client program to connect to NaqsServer

  Copyright 1999 Nanometrics, Inc.  All rights reserved.

  The purpose of this code is to demonstrate how to communicate with the
  NaqsServer datastream service.  It is written for Windows 95 or NT, but
  may be easily modified to run on other platforms.

  It connects to the datastream service, requests data for a single
  channel, and prints out some info about each data packet received.
  It can request and receive time-series, state-of-health or serial data.

  The requested channel name and the host name and port name for the
  datastream service are input as command-line parameters.
  By default, the program connects to port 28000 on the local machine.
  
  Note that all data received from the datastream server are in network
  byte order (most-significant byte first), EXCEPT compressed data packets.
  Compressed data packets are forwarded without modification from the 
  originating instrument; these packets are in LSB-first order.

  This source code is distributed as documentation in support of Nanometrics 
  NaqsServer data streams.  As documentation, Nanometrics offers no support
  and/or warranty for this product.  In particular, Nanometrics provides no
  support and/or warranty for any software developed using part or all of
  this source code.

 ------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "naqs_utils.h"

#define VERSION "2006.310"

/* Can only request up to MAXCHANREQ number of Channels */
#define MAXCHANREQ 1000

/* For a linked list of strings, as filled by clientStrParse() */
typedef struct strlist {
  char *element;
  struct strlist *next;
} strlist;

/* Some globals */
static char *progName = 0;
static char *naqsChannels = 0;   /* List of channels provided by user */
static char *dumpFile = 0;       /* Append all received stuff to dumpFile */
static char *naqsAddr = 0;       /* Address of NAQS */
static int naqsPort = 0;         /* Port to connect to, default is 28000 */
static int sampRate = 0;         /* Samp. rate to request, 0 = original */
static int shortTermComp = 0;    /* Short-term-completion time, def. 60 sec */
static int Verbose = 0;          /* How verbose to be */
static int NSocket = -1;         /* The file descriptor for the conn. */
static strlist *requestList;     /* Parsed, linked-list, of naqsChannels */

/* It's ugly, but simplifies things greatly */
ChannelList channelList;

/* Functions in this source file */
static int processMessage(MessageHeader*);
static int matchDataChannels(int[], int, char*);
static int clientConfig(int, char**);
static void clientReportEnviron();
static void clientShutdown();
static void clientDummyHandler();
static int clientStrParse(char*, char*, struct strlist**);
static int clientMatches(char*, char*);
static void clientUsage();

int
main (int argc, char* argv[])
{
  int rc = 0;
  FILE *outfile;
  MessageHeader messageHeader;

  /* Signal handling, use POSIX calls with standardized semantics */
  struct sigaction sa;

  sa.sa_handler = clientDummyHandler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, NULL);

  sa.sa_handler = clientShutdown;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  progName = argv[0];

  /* Process command line arguments */
  if ( clientConfig(argc, argv) < 0 || argc < 2 ) {
    clientUsage();
    exit(1);
  }

  /* Print important parameters if verbose enough */
  if ( Verbose >= 3 ) clientReportEnviron();

  /* Open dumpFile if requested */
  if ( dumpFile ) {
    outfile = fopen(dumpFile, "a+");
  }

  /* Initialize the channel list length */
  channelList.length = 0;

  /* Main loop */
  for (;;)
  {
    /* Open a TCP socket */
    NSocket = openSocket(naqsAddr, naqsPort);

    if ( NSocket >= 0 ) {
      /* Send the Connect message */
      rc = sendConnectMessage(NSocket);

      /* Continuously read from the stream */
      while ( rc == SOCKET_OK ) {
	if ( receiveHeader( NSocket, &messageHeader) != SOCKET_OK )
	  break;
	rc = processMessage( &messageHeader );
      }
    }

    gen_log(0,0, "Lost connection to NAQS!\n");
    close (NSocket);
    channelList.length = 0;
    sleep (3);
  }

  return 0;
}

/* Process all received messages */
static int
processMessage(MessageHeader *messageHeader)
{
  int rc;

  /* If a channel list request the provided channels */
  if (messageHeader->type == CHANNEL_LIST)
  {

    rc = receiveChannelList(NSocket, &channelList, messageHeader->length);

    if (rc == SOCKET_OK)
    {
      int found = 0, newfound = 0;
      int keyList[MAXCHANREQ];
      struct strlist *requestPtr;

      /* Traverse requestList, finding channel name matches */
      requestPtr = requestList;

      while ( requestPtr != 0 ) {
	newfound = matchDataChannels( keyList, found, requestPtr->element );

	/* Check to see if we matched anything this time */
	if ( newfound == found ) {
	  gen_log(0,1, "No matching channels found for %s\n",
		  requestPtr->element);
	}
	else {
	  found = newfound;
	}

	requestPtr = requestPtr->next;
      }

      /* Clean-up the regex matcher */
      clientMatches(NULL, NULL);

      if ( ! found ) {
	gen_log(1,0, "No requested data streams are available!\n");
	exit(1);
      }
      else { /* Request the matched channels */

	rc = requestDataChannels(NSocket, keyList, found,
				 sampRate, shortTermComp);

	if ( rc != SOCKET_OK )
	  {
	    gen_log(1,0, "Error requesting channels!\n");
	  }
      }
    }

    return rc;
  }

  /* If it is an Error message, receive it and print it out */
  else if (messageHeader->type == ERROR_MSG)
  {
    return receiveError(NSocket, messageHeader->length);
  }

  /* If it is a Terminate message, receive it, print it,
     and return SOCKET_ERROR to exit loop */
  else if (messageHeader->type == TERMINATE_MSG)
  {
    receiveTermination(NSocket, messageHeader->length);
    return SOCKET_ERROR;
  }

  /* If it is uncompressed data, receive it */
  else if (messageHeader->type == DECOMPRESSED_DATA)
  {
    return receiveData(NSocket, messageHeader->length);
  }

  /* If it is compressed data, read it and complain */
  else if (messageHeader->type == COMPRESSED_DATA)
  {
    gen_log(1,0, "Cannot process compressed data!, length = %ld\n",
	   messageHeader->length);
    return flushBytes(NSocket, messageHeader->length);
  }

  /* If it is anything else, just read it to keep in sync */
  else
  {
    gen_log(1,0, "Unrecognized message, type = %ld, length = %ld\n",
	   messageHeader->type, messageHeader->length);
    return flushBytes(NSocket, messageHeader->length);
  }
}

/* Find all data Channels that match the regexp and populate the keyList */
static int
matchDataChannels( int keyList[], int found, char *regex )
{
  int length = channelList.length;
  int ich = 0;
  int type, match;

  for (ich = 0; ich < length; ich++)
  {
    type = dataType(channelList.channel[ich].key);
    match = clientMatches(channelList.channel[ich].name, regex);

    /* Check if it's a data (time series) channel and a regex match */
    if ( type == TIMSER_TYPE && match )
    {
      if ( found >= MAXCHANREQ )
      {
	gen_log(0,0, "Overflow of keyList, too many requested Channels\n");
	exit(1);
      }
      keyList[found++] = channelList.channel[ich].key;
    }
  }
  return found;
}


/* clientConfig(): Process command line arguments and general setup. */
static int
clientConfig( int argcount, char **argvec )
{
  int error = 0;
  int optind;

  /* Setup defaults */
  naqsPort = 28000;
  shortTermComp = 60;
  Verbose = 0;

  if (argcount <= 1) error++;

  /* Process all but last command line argument */
  for(optind=1 ; optind < argcount ; optind++) {
    if (strncmp(argvec[optind], "-v", 2) == 0) {
      Verbose += strspn(&argvec[optind][1],"v");
    }
    else if (strcmp(argvec[optind], "-d") == 0) {
      dumpFile = argvec[++optind];
    }
    else if (strcmp(argvec[optind], "-p") == 0) {
      naqsPort = atoi(argvec[++optind]);
    }
    else if (strcmp(argvec[optind], "-c") == 0) {
      naqsChannels = argvec[++optind];
    }
    else if (strcmp(argvec[optind], "-n") == 0) {
      naqsAddr = argvec[++optind];
    }
    else if (strcmp(argvec[optind], "-s") == 0) {
      sampRate = atoi(argvec[++optind]);
    }
    else if (strcmp(argvec[optind], "-t") == 0) {
      shortTermComp = atoi(argvec[++optind]);
    }
  }

  if ( ! naqsAddr ) {
      gen_log(1,0, "No NAQS server provided with the -n flag!\n");
      error++;
  }

  /* Initialize the verbosity for the gen_log function */
  gen_log(-1, Verbose);

  /* Report the program version */
  gen_log(0,0, "%s version: %s\n", progName, VERSION);

  /* If errors then report the usage message and quit */
  if ( error )
    return -1;

  /* Translate the selection string following '-s' */
  if ( naqsChannels ) {
    if ( clientStrParse(naqsChannels, ",", &requestList) == 0 ) {
      gen_log(1,0, "Error parsing Channels list: %s\n\n", naqsChannels);
      return -1;
    }
  }
  else {  /* No 'naqsChannels' provided */
    gen_log(1,0, "No channels specified, this is required!\n\n");
    return -1;
  }

  return 0;
} /* End of clientConfig() */


/* Report state of global variables, for testing */
static void
clientReportEnviron()
{
  gen_log(0,0,"Verbose:\t%d\n",Verbose);

  if (naqsAddr) gen_log(0,0,"naqsAddr:\t%s\n",naqsAddr);
  else gen_log(0,0,"'naqsAddr' not defined\n");

  gen_log(0,0,"naqsPort:\t%d\n",naqsPort);

  if (naqsChannels) gen_log(0,0,"naqsChannels:\t%s\n",naqsChannels);
  else gen_log(0,0,"'naqsChannels' not defined\n");

  if (dumpFile) gen_log(0,0,"dumpFile:\t%s\n",dumpFile);
  else gen_log(0,0,"'dumpFile' not defined\n");

} /* End of clientReportEnviron() */


/* Do any needed cleanup and exit */
static void
clientShutdown(int sig)
{
  close( NSocket );
  exit( sig );
} /* End of clientShutdown() */

/* Empty signal handler routine */
static void
clientDummyHandler(int sig)
{ }

/* clientStrParse(): splits a 'string' on 'delim' and puts each part
   into a linked list pointed to by 'list' (a pointer to a pointer to a 
   struct).  The last struct has it's 'next' set to 0.  All elements
   are NULL terminated strings.
   If both 'string' and 'delim' are NULL then the linked list is
   traversed and the memory used is freed.

   Returns the number of elements added to the list, or 0 when freeing
   the linked list.
*/
static int
clientStrParse(char *string, char *delim, struct strlist **list)
{
  char *beg;  /* beginning of element */
  char *del;  /* delimiter */
  int stop = 0;
  int count = 0;
  int total;

  struct strlist *curlist;
  struct strlist *tmplist;

  if ( string != NULL && delim != NULL ) {
    total = strlen(string);
    beg = string;

    while ( !stop ) {

      /* Find delimiter */
      del = strstr(beg, delim);

      /* Delimiter not found or empty */
      if ( del == NULL || strlen(delim) == 0 ) {
        del = string + strlen(string);
        stop = 1;
      }

      tmplist = (struct strlist *) malloc( sizeof(strlist) );
      tmplist->next = 0;
      
      tmplist->element = (char *) malloc( del - beg + 1 );
      strncpy(tmplist->element, beg, (del - beg));
      tmplist->element[(del-beg)] = '\0';

      /* Add this to the list */
      if ( count++ == 0 ) {
        curlist = tmplist;
        *list = curlist;
      }
      else {
        curlist->next = tmplist;
        curlist = curlist->next;
      }
      
      /* Update 'beg' */
      beg = ( del + strlen(delim) );
      if ( (beg - string) > total ) {
        break;
      }
    }

    return count;
  }
  else {
    curlist = *list;
    while ( curlist != NULL ) {
      tmplist = curlist->next;
      free(curlist);
      curlist = tmplist;
    }
    return 0;
  }
} /* End of clientStrParse() */


/* Check for regular expression match.
 * Return values: 1 == match, 0 == no match
 * If both arguments are NULL then any previously alloc'd stuff
 * is set free.
 */
static int
clientMatches( char *source, char *regex )
{
  static regex_t *pgm = 0;
  int retcode = 0;

  if ( regex != '\0' ) {
    if ( pgm == 0 ) {
      pgm = (regex_t *) malloc(sizeof(*pgm));
    }
    if ( regcomp(pgm, regex, REG_EXTENDED|REG_NOSUB ) != 0) {
      fprintf(stderr,"Can't compile regular expression '%s'", regex );
    }
    if ( regexec ( pgm, source, (size_t) 0, NULL, 0) == 0 ) {
      retcode = 1;
    }
  }
  /* Free the previously compiled expression if passed NULLs */
  if ( regex == '\0' && source == '\0' ) {
    if ( pgm != 0 ) {
      regfree(pgm);
    }
  }

  return retcode;
}

/* Print a usage message and exit */
static void
clientUsage(void)
{
  fprintf(stderr,"\n"
 "Usage: naqs_plugin [-d dumpfile] [-p port] [-s samprate] [-t stc] [-v]\n"
 "                   <-c Channels> <-n NAQS>\n\n"
 "  -d dumpfile   Append all received packets to this 'dumpfile'\n"
 "  -p port       Specify the NAQS control/data port, default 28000\n"
 "  -v            Be more verbose, can be used multiple times\n"
 "  -s samprate   Sampling rate to request, 0 = original (default)\n"
 "  -t stc        Short-term-completion time, default 60 seconds (max. 600)\n"
 "  -c Channels   Regular expression(s) to match Channel(s) to request,\n"
 "                  multiple expressions separated by commas, required\n"
 "  -n NAQS       Address of server, required\n\n");

  exit (1);
} /* End of clientUsage() */
