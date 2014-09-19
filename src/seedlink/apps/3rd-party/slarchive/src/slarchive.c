/***************************************************************************
 * slarchive.c
 * A SeedLink client for data collection and archiving.
 *
 * Connects to a SeedLink server, collects and archives data.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified 2005.147
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <libslink.h>

#include "archive.h"

#define PACKAGE   "slarchive"
#define VERSION   "1.7-sc3"

static void packet_handler (char *msrecord, int packet_type,
			    int seqnum, int packet_size);
static int  parameter_proc (int argcount, char **argvec);
static char *getoptval (int argcount, char **argvec, int argopt);
static int  add_dsarchive(const char *path, int archivetype);
static int  set_dsbuffersize(int size);
static void term_handler (int sig);
static void print_timelog (const char *msg);
static void usage (void);

/* A chain of archive definitions */
typedef struct DSArchive_s {
  DataStream  datastream;
  struct DSArchive_s *next;
}
DSArchive;

static short int verbose  = 0;   /* flag to control general verbosity */
static short int ppackets = 0;   /* flag to control printing of data packets */
static int stateint       = 0;   /* packet interval to save statefile */
static char *statefile    = 0;	 /* state file for saving/restoring stream states */

static SLCD *slconn;	         /* connection parameters */
static DSArchive *dsarchive;

int
main (int argc, char **argv)
{
  SLpacket *slpack;
  int seqnum;
  int ptype;
  int packetcnt = 0;

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

  dsarchive = NULL;

  /* Allocate and initialize a new connection description */
  slconn = sl_newslcd();

  /* Process given parameters (command line and parameter file) */
  if (parameter_proc (argc, argv) < 0)
    {
      fprintf (stderr, "Parameter processing failed\n");
      fprintf (stderr, "Try '-h' for detailed help\n");
      return -1;
    }

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
	      if (dsarchive) {
	        DSArchive *curdsa = dsarchive;

	        while ( curdsa != NULL ) {
		  if ( curdsa->datastream.grouphash ) {
                    DataStreamGroup *curgroup, *tmp;
                    HASH_ITER(hh, curdsa->datastream.grouphash, curgroup, tmp) {
		      if ( curgroup->filed ) {
			if ( curgroup->bp ) {
			  sl_log (1, 3, "Writing data to data stream file %s\n", curgroup->filename);
			  if ( write (curgroup->filed, curgroup->buf, curgroup->bp) != curgroup->bp ) {
			    sl_log (2, 1,
				    "main: failed to write record\n");
			    return -1;
			  }
			}
			curgroup->bp = 0;
		      }
		    }
		  }
	          curdsa = curdsa->next;
	        }
	      }
	      sl_savestate (slconn, statefile);
	      packetcnt = 0;
	    }
	}
    }

  /* Do all the necessary cleanup and exit */
  if (slconn->link != -1)
    sl_disconnect (slconn);
  
  if (dsarchive) {
    DSArchive *curdsa = dsarchive;

    while ( curdsa != NULL ) {
      archstream_proc (&curdsa->datastream, NULL, 0);
      curdsa = curdsa->next;
    }    
  }

  if (statefile)
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

  if ( verbose >= 1 ) {
    /* Build a current local time string */
    dtime   = sl_dtime ();
    secfrac = (double) ((double)dtime - (int)dtime);
    ttime   = (time_t) dtime;
    timep   = localtime (&ttime);
    snprintf (timestamp, 20, "%04d.%03d.%02d:%02d:%02d.%01.0f",
	      timep->tm_year + 1900, timep->tm_yday + 1, timep->tm_hour,
	      timep->tm_min, timep->tm_sec, secfrac);
    
    sl_log (1, 1, "%s, seq %d, Received %s blockette\n",
	    timestamp, seqnum, type[packet_type]);
  }

  /* Parse data record and print requested detail if any */
  if ( sl_msr_parse (slconn->log, msrecord, &msr, 1, 0) == NULL )
    {
      sl_log (2, 0, "%s, seq %d, Received %s blockette, error: blockette discarded\n",
                          timestamp, seqnum, type[packet_type]);
      return;
    }
  
  if ( ppackets )
    sl_msr_print (slconn->log, msr, ppackets - 1);
  
  /* Process waveform data and send it on */
  if ( packet_type == SLDATA )
    {
      /* Test for a so-called end-of-detection record */
      if ( msr->fsdh.samprate_fact == 0 && msr->fsdh.num_samples == 0 )
	archflag = 0;
    }
  
  /* Write packet to all archives in archive definition chain */
  if ( dsarchive && archflag ) {
    DSArchive *curdsa = dsarchive;

    while ( curdsa != NULL ) {
      curdsa->datastream.packettype = packet_type;

      /* Limit BUD archiving to waveform data */
      if ( curdsa->datastream.archivetype == BUD &&
	   packet_type != SLDATA ) {
	curdsa = curdsa->next;
	continue;
      }

      archstream_proc (&curdsa->datastream, msr, packet_size);
      
      curdsa = curdsa->next;
    }
  }
}  /* End of packet_handler() */


/***************************************************************************
 * parameter_proc:
 *
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure.
 ***************************************************************************/
static int
parameter_proc (int argcount, char **argvec)
{
  int futurecontflag= 0;  /* continuous future check flag */
  int futurecont    = 2;  /* continuous future check overlap */
  int futureinitflag= 0;  /* initial future check (opening files) flag */
  int futureinit    = 2;  /* initial future check (opening files) overlap */
  int idletimeout   = 300; /* idle stream timeout */
  int error = 0;

  char *streamfile  = 0;   /* stream list file for configuring streams */
  char *multiselect = 0;
  char *selectors   = 0;
  char *timewin     = 0;
  char *tptr;

  SLstrlist *timelist;	   /* split the time window arg */

  if (argcount <= 1)
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
      else if (strncmp (argvec[optind], "-p", 2) == 0)
        {
          ppackets += strspn (&argvec[optind][1], "p");
        }
      else if (strncmp (argvec[optind], "-Fc", 3) == 0)
        {
	  futurecontflag = 1;
	  if ( (tptr = strchr(argvec[optind], ':')) )
	    futurecont = atoi(tptr+1);
        }
      else if (strncmp (argvec[optind], "-Fi", 3) == 0)
        {
	  futureinitflag = 1;
	  if ( (tptr = strchr(argvec[optind], ':')) )
	    futureinit = atoi(tptr+1);
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
      else if (strcmp (argvec[optind], "-i") == 0)
	{
	  idletimeout = atoi (getoptval(argcount, argvec, optind++));
	}
      else if (strcmp (argvec[optind], "-A") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), ARCH) == -1 )
	    return -1;
	}
	  else if (strcmp (argvec[optind], "-B") == 0)
	{
	  if ( set_dsbuffersize(atoi (getoptval(argcount, argvec, optind++))) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-SDS") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), SDS) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-BUD") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), BUD) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-DLOG") == 0)
	{
	  if ( add_dsarchive(getoptval(argcount, argvec, optind++), DLOG) == -1 )
	    return -1;
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
  sl_loginit (verbose, &print_timelog, NULL, &print_timelog, NULL);

  /* Set stdout (where logs go) to always flush after a newline */
  setvbuf(stdout, NULL, _IOLBF, 0);

  /* Report the program version */
  sl_log (1, 1, "%s version: %s\n", PACKAGE, VERSION);

  /* If errors then report the usage message and quit */
  if (error)
    {
      usage ();
      exit (1);
    }
  
  /* Load the stream list from a file if specified */
  if ( streamfile )
    sl_read_streamlist (slconn, streamfile, selectors);

  /* Split the time window argument */
  if ( timewin )
    {

      if (strchr (timewin, ':') == NULL)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  return -1;
	}

      if (sl_strparse (timewin, ":", &timelist) > 2)
	{
	  sl_log (2, 0, "time window not in begin:[end] format\n");
	  sl_strparse (NULL, NULL, &timelist);
	  return -1;
	}

      if (strlen (timelist->element) == 0)
	{
	  sl_log (2, 0, "time window must specify a begin time\n");
	  sl_strparse (NULL, NULL, &timelist);
	  return -1;
	}

      slconn->begin_time = strdup (timelist->element);

      timelist = timelist->next;

      if (timelist != 0)
	{
	  slconn->end_time = strdup (timelist->element);

	  if (timelist->next != 0)
	    {
	      sl_log (2, 0, "malformed time window specification\n");
	      sl_strparse (NULL, NULL, &timelist);
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
  else if ( !streamfile )
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
  
  /* If no archiving is specified print a warning */
  if ( !dsarchive ) {
    sl_log (1, 0, "WARNING: no archiving method was specified\n");
  }
  /* Otherwise fill in the global parameters for each entry */
  else {
    DSArchive *curdsa = dsarchive;
    while ( curdsa != NULL ) {
      curdsa->datastream.idletimeout = idletimeout;
      curdsa->datastream.futurecontflag = futurecontflag;
      curdsa->datastream.futurecont = futurecont;
      curdsa->datastream.futureinitflag = futureinitflag;
      curdsa->datastream.futureinit = futureinit;
      curdsa = curdsa->next;
    }
  }

  return 0;
}  /* End of parameter_proc() */


/***************************************************************************
 * getoptval:
 *
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

  if ( (argopt+1) < argcount && *argvec[argopt+1] != '-' )
    return argvec[argopt+1];

  fprintf (stderr, "Option %s requires a value\n", argvec[argopt]);
  exit (1);
}  /* End of getoptval() */


/***************************************************************************
 * add_dsarchive():
 * Add entry to the data stream archive chain.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
add_dsarchive( const char *path, int archivetype )
{
  DSArchive *newdsa;

  newdsa = (DSArchive *) malloc (sizeof (DSArchive));
  
  if ( newdsa == NULL ) {
    sl_log (2, 0, "cannot allocate memory for new archive definition\n");
    return -1;
  }

  /* Setup new entry and add it to the front of the chain */
  newdsa->datastream.path = strdup(path);
  newdsa->datastream.archivetype = archivetype;
  newdsa->datastream.grouphash = NULL;

  if ( newdsa->datastream.path == NULL ) {
    sl_log (2, 0, "cannot allocate memory for new archive path\n");
    return -1;
  }

  newdsa->next = dsarchive;
  dsarchive = newdsa;

  return 0;
}  /* End of add_dsarchive() */


/***************************************************************************
 * set_dsbuffersize():
 * Sets the buffer size of the archive
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
set_dsbuffersize( int size )
{
  if ( size < 0 ) return -1;
  DS_BUFSIZE = size * 512;
  return 0;
}


/***************************************************************************
 * term_handler:
 * Signal handler routine to controll termination.
 ***************************************************************************/
static void
term_handler (int sig)
{
  sl_terminate (slconn);
}


/***************************************************************************
 * print_timelog:
 * Log message print handler used with sl_loginit() and sl_log().  Prefixes
 * a local time string to the message before printing.
 ***************************************************************************/
static void
print_timelog (const char *msg)
{
  char timestr[100];
  time_t loc_time;
  
  /* Build local time string and cut off the newline */
  time(&loc_time);
  strcpy(timestr, asctime(localtime(&loc_time)));
  timestr[strlen(timestr) - 1] = '\0';
  
  fprintf (stdout, "%s - %s", timestr, msg);
}


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
	   " -p              print details of data packets, multiple flags can be used\n"
	   " -nd delay       network re-connect delay (seconds), default 30\n"
	   " -nt timeout     network timeout (seconds), re-establish connection if no\n"
	   "                   data/keepalives are received in this time, default 600\n"
	   " -k interval     send keepalive (heartbeat) packets this often (seconds)\n"
	   " -x sfile[:int]  save/restore stream state information to this file\n"
	   " -i timeout      idle stream entries might be closed (seconds), default 300\n"
	   " -d             configure the connection in dial-up mode\n"
	   " -b             configure the connection in batch mode\n"
	   " -Fi[:overlap]   Initially check (existing files) that data records are newer\n"
	   " -Fc[:overlap]   Continuously check that data records are newer\n"
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
	   " ## Data archiving options ##\n"
	   " -A format       save all received records is a custom file structure\n"
	   " -B count        set record buffer size to count*512, default 1000\n"
	   " -SDS  SDSdir    save all received records in a SDS file structure\n"
	   " -BUD  BUDdir    save all received data records in a BUD file structure\n"
	   " -DLOG DLOGdir   save all received data records in an old-style\n"
	   "                   SeisComP/datalog file structure\n"
	   "\n"
	   " [host][:][port] Address of the SeedLink server in host:port format\n"
	   "                  Default host is 'localhost' and default port is '18000'\n\n");

}  /* End of usage() */
