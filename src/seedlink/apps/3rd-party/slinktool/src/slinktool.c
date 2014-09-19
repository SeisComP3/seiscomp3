/***************************************************************************
 * A SeedLink client for data stream inspection, data collection and server
 * testing.
 *
 * Connects to a SeedLink server, configures a connection using either
 * uni or multi-station mode and collects data.  Detailed information about
 * the data received can be printed and the data can be saved to files.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified 2009.240
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WIN32
  #include <signal.h>
#endif

#include <libslink.h>

#include "slinkxml.h"
#include "archive.h"

#define PACKAGE   "slinktool"
#define VERSION   "4.1b-sc3"

/* Idle archive stream timeout */ 
#define  IDLE_ARCH_STREAM_TIMEOUT  120

static short int verbose  = 0;      /* flag to control general verbosity */
static short int pingonly = 0;      /* flag to control ping function */
static short int ppackets = 0;      /* flag to control printing of data packets */
static short int psamples = 0;      /* flag to control printing of data samples */
static int stateint       = 0;      /* packet interval to save statefile */
static char *archformat   = 0;	    /* format string for a custom structure */
static char *sdsdir       = 0;	    /* base directory for a SDS structure */
static char *buddir       = 0;	    /* base directory for a BUD structure */
static char *statefile    = 0;	    /* state file for saving/restoring the seq. no. */
static char *dumpfile     = 0;	    /* output file for data dump */
static FILE *outfile      = 0;      /* the descriptor for the dumpfile */

static SLCD * slconn;	            /* connection parameters */

/* Possible query types */
static enum
{
  SLTNoQuery,
  SLTIDQuery,
  SLTStationQuery,
  SLTStreamQuery,
  SLTGapQuery,
  SLTConnectionQuery,
  SLTGenericQuery,
  SLTKeepAliveQuery
}
slt_query = SLTNoQuery;

/* XML parser context for processing INFO responses */
static xmlParserCtxtPtr xml_parser_ctxt = NULL;

/* Functions internal to this source file */
static void packet_handler (char *msrecord, int packet_type,
			    int seqnum, int packet_size);
static int  info_handler (SLMSrecord * msr, int terminate);

static int  parameter_proc (int argcount, char **argvec);
static char *getoptval (int argcount, char **argvec, int argopt);
static void print_samples (SLMSrecord *msr);
static int  ping_server (SLCD *slconn);
static void print_stderr (const char *message);
static void report_environ ();
static void usage (void);

#ifndef WIN32
  static void term_handler (int sig);
#endif

int
main (int argc, char **argv)
{
  SLpacket * slpack;
  int seqnum;
  int ptype;
  int packetcnt = 0;

#ifndef WIN32
  /* Signal handling, use POSIX calls with standardized semantics */
  struct sigaction sa;

  sa.sa_flags   = SA_RESTART;
  sigemptyset (&sa.sa_mask);

  sa.sa_handler = term_handler;
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGQUIT, &sa, NULL);
  sigaction (SIGTERM, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction (SIGHUP, &sa, NULL);
  sigaction (SIGPIPE, &sa, NULL);
#endif

  /* Allocate and initialize a new connection description */
  slconn = sl_newslcd();
  
  /* Process given parameters (command line and parameter file) */
  if ( parameter_proc (argc, argv) < 0 )
    {
      sl_log (2, 0, "parameter processing failed.\n");
      return -1;
    }
  
  /* Print important parameters if verbose enough */
  if ( verbose >= 3 )
    report_environ ();
  
  /* Only do a ping if requested */
  if ( pingonly )
    exit (ping_server (slconn));
  
  /* Loop with the connection manager */
  while ( sl_collect (slconn, &slpack) )
    {
      ptype  = sl_packettype (slpack);
      seqnum = sl_sequence (slpack);

      packet_handler ((char *) &slpack->msrecord, ptype, seqnum, SLRECSIZE);

      if ( statefile && stateint )
        {
          if ( ++packetcnt >= stateint )
            {
              sl_savestate (slconn, statefile);
              packetcnt = 0;
            }
        }

      /* Quit if no streams and terminated INFO is received */
      if ( slconn->streams == NULL && ptype == SLINFT )
	break;
    }

  /* Shutdown */
  if ( slconn->link != -1 )
    sl_disconnect (slconn);

  if ( dumpfile )
    fclose (outfile);

  if ( buddir )
    bud_streamproc (NULL, NULL, 0, 0);

  if ( archformat )
    arch_streamproc (NULL, NULL, 0, 0, 0);

  if ( sdsdir )
    sds_streamproc (NULL, NULL, 0, 0, 0);

  if ( statefile )
    sl_savestate (slconn, statefile);

  return 0;
}  /* End of main() */


/***************************************************************************
 * packet_handler:
 * Process a received packet based on packet type.
 ***************************************************************************/
static void
packet_handler (char *msrecord, int packet_type, int seqnum, int packet_size)
{
  static SLMSrecord * msr = NULL;

  double dtime;			/* Epoch time */
  double secfrac;		/* Fractional part of epoch time */
  time_t ttime;			/* Integer part of epoch time */
  char   timestamp[20];
  struct tm *timep;
  int    archflag = 1;

  /* The following is dependent on the packet type values in libslink.h */
  char *type[]  = { "Data", "Detection", "Calibration", "Timing",
		    "Message", "General", "Request", "Info",
                    "Info (terminated)", "KeepAlive" };

  /* Build a current local time string */
  dtime   = sl_dtime ();
  secfrac = (double) ((double)dtime - (int)dtime);
  ttime   = (time_t) dtime;
  timep   = localtime (&ttime);
  snprintf (timestamp, 20, "%04d.%03d.%02d:%02d:%02d.%01.0f",
	    timep->tm_year + 1900, timep->tm_yday + 1, timep->tm_hour,
	    timep->tm_min, timep->tm_sec, secfrac);

  /* Process waveform data and send it on */
  if ( packet_type == SLDATA )
    {
      sl_log (1, 1, "%s, seq %d, Received %s blockette\n",
	      timestamp, seqnum, type[packet_type]);

      /* Parse data record and print requested detail if any */
      if ( psamples )
	sl_msr_parse (slconn->log, msrecord, &msr, 1, 1);
      else
	sl_msr_parse (slconn->log, msrecord, &msr, 1, 0);

      if ( ppackets )
        sl_msr_print (slconn->log, msr, ppackets - 1);

      if ( psamples )
	print_samples (msr);

      /* Test for a so-called end-of-detection record */
      if ( msr->fsdh.samprate_fact == 0 && msr->fsdh.num_samples == 0 )
	archflag = 0;

      /* Write packet to BUD structure if requested */
      if ( buddir && archflag )
	{
	  if (bud_streamproc (buddir, msr, packet_size,
			      IDLE_ARCH_STREAM_TIMEOUT))
	    sl_log (2, 0, "cannot write data to BUD at %s\n", buddir);
	}
    }
  else if ( packet_type == SLINF ||  packet_type == SLINFT )
    {
      int terminate;

      sl_log (1, 1, "%s, seq %d, Received %s blockette\n",
	      timestamp, seqnum, type[packet_type]);

      terminate = ( packet_type == SLINFT );

      sl_msr_parse (slconn->log, msrecord, &msr, 0, 0);
      
      if ( info_handler (msr, terminate) == -2 )
	{
	  sl_log (2, 1, "processing of INFO packet failed\n");
	}

      archflag = 0;
    }
  else if ( packet_type == SLKEEP )
    {
      sl_log (2, 0, "keepalive packet received by packet_handler?!?\n");

      archflag = 0;
    }
  else
    {
      sl_log (1, 1, "%s, seq %d, Received %s blockette\n",
	      timestamp, seqnum, type[packet_type]);
    }

  /* Write packet to dumpfile if defined */
  if ( dumpfile )
    {
      if (fwrite (msrecord, packet_size, 1, outfile) == 0)
	sl_log (2, 0, "fwrite(): error writing data to %s\n", dumpfile);
    }

  /* Write packet to an archive if requested */
  if ( archformat && archflag )
    {
      if (arch_streamproc (archformat, msr, packet_size, packet_type,
			   IDLE_ARCH_STREAM_TIMEOUT))
	sl_log (2, 0, "cannot write data to archive\n");

    }

  /* Write packet to an SDS archive if requested */
  if ( sdsdir && archflag )
    {
      if (sds_streamproc (sdsdir, msr, packet_size, packet_type,
			  IDLE_ARCH_STREAM_TIMEOUT))
	sl_log (2, 0, "cannot write data to SDS at %s\n", sdsdir);

    }  
}  /* End of packet_handler() */


/***************************************************************************
 * info_handler:
 * Process XML-based INFO packets.
 *
 * Returns:
 * -2 = Errors
 * -1 = XML is terminated
 *  0 = XML is not terminated
 ***************************************************************************/
static int
info_handler (SLMSrecord * msr, int terminate)
{
  int preturn;

  char *xml_chunk = (char *) msr->msrecord + msr->fsdh.begin_data;
  int xml_size = msr->fsdh.num_samples;

  /* Init XML parser context if it has not been done yet */
  if ( xml_parser_ctxt == NULL )
    {
      if ( (xml_parser_ctxt = xml_begin ()) == NULL )
	{
	  sl_log (2, 0, "xml_begin(): memory allocation error\n");
	  return -2;
	}
    }

  /* Check for an error condition */
  if ( ! strncmp(msr->fsdh.channel, "ERR", 3) )
    {
      sl_log (2, 0, "INFO type requested is not enabled\n");
      return -2;
    }

  /* Parse the XML */
  preturn = xml_parse_chunk (xml_parser_ctxt, xml_chunk, xml_size, terminate);

  if ( preturn != XML_ERR_OK )
    {
      sl_log (2, 0, "XML parse error %d\n", preturn);
      return -2;
    }

  if ( terminate || preturn )
    {
      xmlDocPtr doc = xml_end (xml_parser_ctxt);
      xml_parser_ctxt = NULL;

      switch (slt_query)
	{
	case SLTIDQuery:
	  prtinfo_identification (doc);
	  break;
	case SLTStationQuery:
	  prtinfo_stations (doc);
	  break;
	case SLTStreamQuery:
	  prtinfo_streams (doc);
	  break;
	case SLTGapQuery:
	  prtinfo_gaps (doc);
	  break;
	case SLTConnectionQuery:
	  prtinfo_connections (doc);
	  break;
	case SLTGenericQuery:
	  xmlDocFormatDump (stdout, doc, 1);
	  break;
	default:
	  break;
	}
      
      slt_query = SLTNoQuery;
      xml_free_doc (doc);
      return -1;
    }

  return preturn;
}  /* End of info_handler() */


/***************************************************************************
 * parameter_proc:
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
parameter_proc (int argcount, char **argvec)
{
  int error = 0;
  int optind;

  char *streamfile  = 0;	/* stream list file for configuring streams */
  char *multiselect = 0;
  char *selectors   = 0;
  char *timewin     = 0;
  char *tptr;

  SLstrlist *timelist;		/* split the time window arg */

  if ( argcount <= 1 )
    error++;

  /* Process all command line arguments */
  for (optind = 1; optind < argcount; optind++)
    {
      if (strcmp (argvec[optind], "-V") == 0)
        {
          fprintf(stderr, "%s version: %s\n", PACKAGE, VERSION);
          exit (0);
        }
      else if (strcmp (argvec[optind], "-h") == 0)
        {
          usage();
          exit (0);
        }
      else if (strncmp (argvec[optind], "-v", 2) == 0)
	{
	  verbose += strspn (&argvec[optind][1], "v");
	}
      else if (strcmp (argvec[optind], "-P") == 0)
        {
          pingonly = 1;
        }
      else if (strncmp (argvec[optind], "-p", 2) == 0)
        {
          ppackets += strspn (&argvec[optind][1], "p");
        }
      else if (strcmp (argvec[optind], "-u") == 0)
        {
          psamples = 1;
        }
      else if (strcmp (argvec[optind], "-d") == 0)
	{
	  slconn->dialup = 1;
	}
      else if (strcmp (argvec[optind], "-b") == 0)
	{
	  slconn->batchmode = 1;
	}
      else if (strcmp (argvec[optind], "-nt") == 0)
	{
	  slconn->netto = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-nd") == 0)
	{
	  slconn->netdly = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-k") == 0)
	{
	  slconn->keepalive = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-o") == 0)
	{
	  dumpfile = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-A") == 0)
	{
	  archformat = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-SDS") == 0)
	{
	  sdsdir = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-BUD") == 0)
	{
	  buddir = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-l") == 0)
	{
	  streamfile = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-s") == 0)
	{
	  selectors = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-S") == 0)
	{
	  multiselect = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-x") == 0)
	{
	  statefile = getoptval(argcount, argvec, optind++);
	}
      else if (strcmp (argvec[optind], "-i") == 0)
	{
	  if ( sl_request_info (slconn, getoptval(argcount, argvec, optind++)) == 0 )
	    slt_query = SLTGenericQuery;
	}
      else if (strcmp (argvec[optind], "-I") == 0)
	{
	  if (sl_request_info (slconn, "ID") == 0)
	    slt_query = SLTIDQuery;
	}
      else if (strcmp (argvec[optind], "-L") == 0)
	{
	  if (sl_request_info (slconn, "STATIONS") == 0)
	    slt_query = SLTStationQuery;
	}
      else if (strcmp (argvec[optind], "-Q") == 0)
	{
	  if (sl_request_info (slconn, "STREAMS") == 0)
	    slt_query = SLTStreamQuery;
	}
      else if (strcmp (argvec[optind], "-G") == 0)
	{
	  if (sl_request_info (slconn, "GAPS") == 0)
	    slt_query = SLTGapQuery;
	}
      else if (strcmp (argvec[optind], "-C") == 0)
	{
	  if (sl_request_info (slconn, "CONNECTIONS") == 0)
	    slt_query = SLTConnectionQuery;
	}
      else if (strcmp (argvec[optind], "-tw") == 0)
	{
	  timewin = getoptval(argcount, argvec, optind++);
	}
      else if (strncmp (argvec[optind], "-", 1) == 0)
        {
          fprintf (stderr, "Unknown option: %s\n", argvec[optind]);
          exit (1);
        }
      else if (!slconn->sladdr)
        {
          slconn->sladdr = argvec[optind];
        }
      else
        {
          fprintf (stderr, "Unknown option: %s\n", argvec[optind]);
          exit (1);
        }
    }

  /* Make sure a server was specified */
  if ( ! slconn->sladdr )
    {
      fprintf(stderr, "No SeedLink server specified\n\n");
      fprintf(stderr, "%s version %s\n\n", PACKAGE, VERSION);
      fprintf(stderr, "Usage: %s [options] [host][:][port]\n\n", PACKAGE);
      fprintf(stderr, "Try '-h' for detailed help\n");
      exit (1);
    }

  /* Initialize the verbosity for the sl_log function */
  sl_loginit (verbose, NULL, NULL, NULL, NULL);

  /* Open dumpfile if requested */
  if (dumpfile)
    {
      if (!strcmp (dumpfile, "-"))
	{
	  /* Re-direct all messages to standard error */
	  sl_loginit (verbose, &print_stderr, NULL, &print_stderr, NULL);

	  outfile = stdout;
	  setvbuf (stdout, NULL, _IONBF, 0);
	}
      else if ((outfile = fopen (dumpfile, "a+b")) != NULL)
	{
	  setvbuf (outfile, NULL, _IONBF, 0);
	}
      else
	{
	  sl_log (2, 0, "cannot open dumpfile: %s\n", dumpfile);
	  exit (1);
	}
    }

  /* Report the program version */
  sl_log (1, 1, "%s version: %s\n", PACKAGE, VERSION);

  /* If errors then report the usage message and quit */
  if (error)
    {
      usage ();
      exit (1);
    }
  
  /* Make sure we print basic packet details if printing samples */
  if ( psamples && ppackets == 0 )
    ppackets = 1;
  
  /* Load the stream list from a file if specified */
  if ( streamfile )
    sl_read_streamlist (slconn, streamfile, selectors);
  
  /* Split the time window argument */
  if ( timewin )
    {
      SLstrlist *timeptr;
      
      if (strchr (timewin, ':') == NULL)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  return -1;
	}
      
      if (sl_strparse (timewin, ":", &timelist) > 2)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  return -1;
	}
      
      timeptr = timelist;
      
      if (strlen (timeptr->element) == 0)
	{
	  sl_log (2, 0, "time window must specify a begin time\n");
	  return -1;
	}

      slconn->begin_time = strdup (timeptr->element);
      
      timeptr = timeptr->next;

      if (timeptr != 0)
	{
	  slconn->end_time = strdup (timeptr->element);

	  if (timeptr->next != 0)
	    {
	      sl_log (2, 0, "malformed time window specification\n");
	      return -1;

	    }
	}
      
      /* Free the parsed list */
      sl_strparse (NULL, NULL, &timelist);
    }
  
  /* Parse the 'multiselect' string following '-S' */
  if ( multiselect )
    {
      if ( sl_parse_streamlist (slconn, multiselect, selectors) == -1 )
	return -1;
    }
  else if ( slconn->streams == NULL && slconn->info == NULL )
    {		         /* No 'streams' array, assuming uni-station mode */
      sl_setuniparams (slconn, selectors, -1, 0);
    }

  /* Attempt to recover sequence numbers from state file */
  if (statefile)
    {
      /* Check if interval was specified for state saving */
      if ((tptr = strchr (statefile, ':')) != NULL)
        {
          char *tail;
           
          *tptr++ = '\0';
           
          stateint = (unsigned int) strtoul (tptr, &tail, 0);
           
          if ( *tail || (stateint < 0 || stateint > 1e9) )
            {
              sl_log (2, 0, "state saving interval specified incorrectly\n");
              return -1;
            }
        }

      if (sl_recoverstate (slconn, statefile) < 0)
	{
	  sl_log (2, 0, "state recovery failed\n");
	}
    }

  return 0;
}  /* End of parameter_proc() */


/***************************************************************************
 * getoptval:
 * Return the value to a command line option; checking that the value is 
 * itself not an option (starting with '-') and is not past the end of
 * the argument list.
 *
 * argcount: total arguments in argvec
 * argvec: argument list
 * argopt: index of option to process, value is expected to be at argopt+1
 *
 * Returns value on success and exits with error message on failure
 ***************************************************************************/
static char *
getoptval (int argcount, char **argvec, int argopt)
{
  if ( argvec == NULL || argvec[argopt] == NULL ) {
    fprintf (stderr, "getoptval(): NULL option requested\n");
    exit (1);
  }
  
  /* Special case of '-o -' usage */
  if ( (argopt+1) < argcount && strcmp (argvec[argopt], "-o") == 0 )
    if ( strcmp (argvec[argopt+1], "-") == 0 )
      return argvec[argopt+1];
  
  if ( (argopt+1) < argcount && *argvec[argopt+1] != '-' )
    return argvec[argopt+1];

  fprintf (stderr, "Option %s requires a value\n", argvec[argopt]);
  exit (1);

  return NULL; /* To stop compiler warnings about no return */
}  /* End of getoptval() */


/***************************************************************************
 * print_samples:
 * Print samples in the supplied SLMSrecord with a simple format.
 ***************************************************************************/
static void
print_samples (SLMSrecord *msr)
{
  int line, lines, col, cnt;
  
  if ( msr->datasamples != NULL ) {
    lines = (msr->numsamples / 6) + 1;
    
    for ( cnt = 0, line = 0; line < lines; line++ )
      {
	for ( col = 0; col < 6 ; col ++ )
	  {
	    if ( cnt < msr->numsamples )
	      sl_log (0, 0, "%10d  ", *(msr->datasamples + cnt++));
	  }
	sl_log (0, 0, "\n");
      }
  }
  
  return;
}  /* End of print_samples() */


/***************************************************************************
 * ping_server:
 *
 * Ping a server and print the server ID and site.
 *
 * Returns 0 on success, and 1 on failure.
 ***************************************************************************/
static int
ping_server (SLCD *slconn)
{
  char serverid[100];
  char site[100];
  int retval;
  
  retval = sl_ping (slconn, serverid, site);
  
  if ( retval == 0 )
    {
      sl_log (0, 0, "%s\n%s\n", serverid, site);
    }
  else if ( retval == -1 )
    {
      sl_log (1, 0, "Bad response from server, not SeedLink?\n");
      retval = 1;
    }
  else if ( retval == -2 )
    {
      sl_log (1, 0, "Could not open network connection\n");
      retval = 1;
    }

  return retval;
}  /* End of ping_server() */


/***************************************************************************
 * print_stderr:
 * Print the given message to standard error.
 ***************************************************************************/
static void
print_stderr (const char *message)
{
  fprintf (stderr, "%s", message);
  return;
}


/***************************************************************************
 * report_environ:
 * Report (print) the state of global variables, intended for testing.
 ***************************************************************************/
static void
report_environ ()
{
  SLstream *curstream;

  sl_log (1, 0, "verbose:\t%d\n", verbose);
  sl_log (1, 0, "pingonly:\t%d\n", pingonly);

  if (dumpfile)
    sl_log (1, 0, "dumpfile:\t%s\n", dumpfile);
  else
    sl_log (1, 0, "'dumpfile' not defined\n");

  if (archformat)
    sl_log (1, 0, "archformat:\t%s\n", archformat);
  else
    sl_log (1, 0, "'archformat' not defined\n");

  if (sdsdir)
    sl_log (1, 0, "sdsdir:\t%s\n", sdsdir);
  else
    sl_log (1, 0, "'sdsdir' not defined\n");

  if (buddir)
    sl_log (1, 0, "buddir:\t%s\n", buddir);
  else
    sl_log (1, 0, "'buddir' not defined\n");

  if (statefile)
    sl_log (1, 0, "statefile:\t%s\n", statefile);
  else
    sl_log (1, 0, "'statefile' not defined\n");

  if (slconn->sladdr)
    sl_log (1, 0, "sladdr:\t%s\n", slconn->sladdr);
  else
    sl_log (1, 0, "'slconn->sladdr' not defined\n");

  if (slconn->begin_time)
    sl_log (1, 0, "slconn->begin_time:\t%s\n", slconn->begin_time);
  else
    sl_log (1, 0, "'slconn->begin_time' not defined\n");
  if (slconn->end_time)
    sl_log (1, 0, "slconn->end_time:\t%s\n", slconn->end_time);
  else
    sl_log (1, 0, "'slconn->end_time' not defined\n");

  sl_log (1, 0, "slconn->dialup:\t%d\n", slconn->dialup);
  sl_log (1, 0, "slconn->multistation:\t%d\n", slconn->multistation);

  if (slconn->info)
    sl_log (1, 0, "slconn->info:\t%s\n", slconn->info);
  else
    sl_log (1, 0, "'slconn->info' not defined\n");

  sl_log (1, 0, "keepalive:\t%d\n", slconn->keepalive);
  sl_log (1, 0, "nettimeout:\t%d\n", slconn->netto);
  sl_log (1, 0, "netdelay:\t%d\n", slconn->netdly);

  sl_log (1, 0, "slconn->protocol_ver:\t%f\n", slconn->protocol_ver);
  sl_log (1, 0, "slconn->link:\t%d\n", slconn->link);

  curstream = slconn->streams;

  sl_log (1, 0, "'streams' array:\n");
  while (curstream != NULL)
    {
      if (curstream->net)
	sl_log (1, 0, "Sta - net: %s\n", curstream->net);
      else
	sl_log (1, 0, "'net' not defined\n");

      if (curstream->sta)
	sl_log (1, 0, "Sta - sta: %s\n", curstream->sta);
      else
	sl_log (1, 0, "'sta' not defined\n");

      if (curstream->selectors)
	sl_log (1, 0, "Sta - selectors: %s\n", curstream->selectors);
      else
	sl_log (1, 0, "'selectors' not defined\n");

      sl_log (1, 0, "Sta - seqnum: %d\n", curstream->seqnum);

      if (curstream->timestamp)
	sl_log (1, 0, "Sta - timestamp: %s\n", curstream->timestamp);
      else
	sl_log (1, 0, "'timestamp' not defined\n");

      curstream = curstream->next;
    }
}  /* End of report_environ() */


#ifndef WIN32
/***************************************************************************
 * term_handler:
 * Signal handler routine to set the termination flag.
 ***************************************************************************/
static void
term_handler (int sig)
{
  sl_terminate(slconn);
}
#endif


/***************************************************************************
 * usage:
 * Print the usage message and exit.
 ***************************************************************************/
static void
usage (void)
{
  fprintf (stderr, "%s version %s\n\n", PACKAGE, VERSION);
  fprintf (stderr, "Usage: %s [options] [host][:][port]\n\n", PACKAGE);
  fprintf (stderr,
	   " ## General program options ##\n"
	   " -V              report program version\n"
	   " -h              show this usage message\n"
	   " -v              be more verbose, multiple flags can be used\n"
	   " -P              ping the server, report the server ID and exit\n"
	   " -p              print details of data packets, multiple flags can be used\n"
	   " -u              print unpacked samples of data packets\n\n"
	   " -nd delay       network re-connect delay (seconds), default 30\n"
	   " -nt timeout     network timeout (seconds), re-establish connection if no\n"
	   "                   data/keepalives are received in this time, default 600\n"
	   " -k interval     send keepalive (heartbeat) packets this often (seconds)\n"
	   " -x sfile[:int]  save/restore stream state information to this file\n"
	   " -d              configure the connection in dial-up mode\n"
	   " -b              configure the connection in batch mode\n"
	   "\n"
	   " ## Data stream selection ##\n"
	   " -s selectors    selectors for uni-station or default for multi-station mode\n"
	   " -l listfile     read a stream list from this file for multi-station mode\n"
           " -S streams      define a stream list for multi-station mode\n"
	   "   'streams' = 'stream1[:selectors1],stream2[:selectors2],...'\n"
	   "        'stream' is in NET_STA format, for example:\n"
	   "        -S \"IU_KONO:BHE BHN,GE_WLF,MN_AQU:HH?.D\"\n\n"
	   " -tw begin:[end]  (requires SeedLink >= 3)\n"
	   "        specify a time window in year,month,day,hour,min,sec format\n"
	   "        example: -tw 2002,08,05,14,00,00:2002,08,05,14,15,00\n"
	   "        the end time is optional, but the colon must be present\n"
	   "\n"
	   " ## Data saving options ##\n"
	   " -o dumpfile     write all received records to this file\n"
	   " -A format       save all received records is a custom file structure\n"
	   " -SDS SDSdir     save all received records in a SDS file structure\n"
	   " -BUD BUDdir     save all received data records in a BUD file structure\n"
	   "\n"
	   " ## Data server  information ## (requires SeedLink >= 3)\n"
	   " -i type         send info request, type is one of the following:\n"
	   "                   ID, CAPABILITIES, STATIONS, STREAMS, GAPS, CONNECTIONS, ALL\n"
	   "                   the returned raw XML is displayed when using this option\n"
	   " -I              print formatted server id and version\n"
	   " -L              print formatted station list (if supported by server)\n"
	   " -Q              print formatted stream list (if supported by server)\n"
	   " -G              print formatted gap list (if supported by server)\n"
	   " -C              print formatted connection list (if supported by server)\n"
	   "\n"
	   " [host][:][port] Address of the SeedLink server in host:port format\n"
	   "                   Default host is 'localhost' and default port is '18000'\n");

}  /* End of usage() */
