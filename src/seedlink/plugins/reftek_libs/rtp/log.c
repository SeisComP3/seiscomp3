#pragma ident "$Id: log.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Message logging facility.
 *
 *  The rtp_loginit() function must be called prior to rtp_log().
 *
 *  MT-safe, in the crudest possible way.  Only one log file is
 *  supported.  sigh
 *
 * Revisions:
 *		Add David Chavez change to use mutex only if not using syslogd
 *====================================================================*/
#include "rtp.h"

#define STRLEN  64
#define HOSTNAMLEN 128
#define LOGBUFLEN  ((size_t) 20480)
#define DEFTIMEFMT "%Y:%j-%H:%M:%S"

static char *Tfmt = DEFTIMEFMT;

static char *LogFileName = (char*)NULL;

static char Prefix[LOGBUFLEN];

static char Hostname[HOSTNAMLEN];

static char Message[LOGBUFLEN];

static char buf1[LOGBUFLEN], buf2[LOGBUFLEN];

static char *Crnt_msg,  *Prev_msg;

static int Pid;

static int Count = 0;

static int Facility;

static UINT16 Threshold = RTP_DEFAULT_LOG;

static MUTEX Mutex = MUTEX_INITIALIZER;

static BOOL Initialized = FALSE;

static BOOL using_syslogd = FALSE;

/*====================================================================*/
/* Set the current log level */

VOID rtp_loglevel(UINT16 newlevel)
   {
   Threshold = newlevel;
} /* end rtp_loglevel() */

/*====================================================================*/
/*  Open log file  */

static FILE *open_log_file()
   {
   if (using_syslogd)
      return (FILE*)NULL;
   if (LogFileName == (CHAR*)NULL)
      return stderr;
   if (strcasecmp(LogFileName, "stderr") == 0)
      return stderr;
   if (strcasecmp(LogFileName, "stdout") == 0)
      return stdout;
   if (strcasecmp(LogFileName, "-") == 0)
      return stdout;
   return fopen(LogFileName, "a+");
} /* end open_log_file() */

/*====================================================================*/
/* Flush log (close, then reopen) OBSOLETE */

void rtp_flushlog()
   {
   return ;
} /* end rtp_flushlog() */

/*====================================================================*/
/* Initialize the log */

BOOL rtp_loginit(CHAR *file, UINT16 facility, CHAR *tfmt, CHAR *fmt, ...)
   {
   FILE *fp;
   va_list ap;
   char *ptr;

   MUTEX_LOCK(&Mutex);

   /* Should only be called once per program */

   if (Initialized)
      {
      errno = EALREADY;
      MUTEX_UNLOCK(&Mutex);
      return FALSE;
      }

   gethostname(Hostname, HOSTNAMLEN);
   Pid = getpid();

   /* Initilize system logger, if using syslogd */

   /*  Save facility, name, and time format string */

   Facility = facility;

   if (file != (CHAR*)NULL)
      {
      #ifdef HAVE_SYSLOGD
         if (strcasecmp(file, "syslogd") == 0)
            {
            openlog(NULL, LOG_CONS, Facility);
            using_syslogd = TRUE;
            }
         else
      #endif /* HAVE_SYSLOGD */
      if ((LogFileName = strdup(file)) == (char*)NULL)
         {
         MUTEX_UNLOCK(&Mutex);
         return FALSE;
         }
      if ((fp = open_log_file()) == (FILE*)NULL)
         {
         if (!using_syslogd)
         	{
         	MUTEX_UNLOCK(&Mutex);
         	return FALSE;
         	}
         /* don't close if stdout or stderr (2003-05-15 PLD) */
         }
      else if (fp != stderr && fp != stdout)
         {
         fclose(fp);
         }
      }

   if (tfmt != NULL && (Tfmt = strdup(tfmt)) == NULL)
      {
      MUTEX_UNLOCK(&Mutex);
      return FALSE;
      }

   /*  Initialize the message buffers  */

   memset((void*)buf1, 0, LOGBUFLEN);
   Crnt_msg = buf1;
   memset((void*)buf2, 0, LOGBUFLEN);
   Prev_msg = buf2;

   /*  Create the message prefix  */

   memset((void*)Prefix, 0, LOGBUFLEN);
   if (fmt != (char*)NULL)
      {
      ptr = Prefix;
      va_start(ap, fmt);
      vsprintf(ptr, fmt, ap);
      va_end(ap);
      }
   else
      {
      Prefix[0] = (char)0;
      }

   /* Initialization complete */

   Initialized = TRUE;

   MUTEX_UNLOCK(&Mutex);

   return TRUE;

} /* end rtp_loginit() */

/*====================================================================*/
/* Log a message */

VOID rtp_log(UINT16 level, CHAR *fmt, ...)
   {
   INT32 i;
   va_list ap;
   CHAR *ptr;
   FILE *fp;
   static time_t ltime;
   static CHAR preamble[LOGBUFLEN];
   static CHAR timstr[STRLEN];
   BOOL echo;

   MUTEX_LOCK(&Mutex);

   /* If we haven't been initialized, don't do anything */

   if (!Initialized)
      {
      MUTEX_UNLOCK(&Mutex);
      return ;
      }

   /* Set echo flag & clear echo bit from verbosity level (2003-05-15 PLD) */
   echo = level &RTP_LOG_ECHO;
   level &= (~RTP_LOG_ECHO);

   /* If our verbosity level is too low for this message, ignore it */

   if (Threshold < level)
      {
      MUTEX_UNLOCK(&Mutex);
      return ;
      }

   /*  Build the message string  */

   memset((void*)Crnt_msg, 0, LOGBUFLEN);
   if (using_syslogd)
      {
      sprintf(Crnt_msg, "%s[%d]: ", Prefix, Pid);
      ptr = Crnt_msg + strlen(Crnt_msg);
      }
   else
      {
      ptr = Crnt_msg;
      }

   va_start(ap, fmt);
   vsprintf(ptr, fmt, ap);
   va_end(ap);

   /*  Strip off any trailing newlines */

   for (i = strlen(Crnt_msg) - 1; i >= 0 && Crnt_msg[i] == '\n'; i--)
      {
      Crnt_msg[i] = 0;
      }

   /*  If using syslogd then we are done  */

   #ifdef HAVE_SYSLOGD
      if (using_syslogd)
         {
         syslog(LOG_INFO, Crnt_msg);
         MUTEX_UNLOCK(&Mutex);
         return ;
         }
   #endif /* HAVE_SYSLOGD */

   /*  Don't print duplicate messages  */

   if (strcmp(Crnt_msg, Prev_msg) == 0)
      {
      ++Count;
      MUTEX_UNLOCK(&Mutex);
      return ;
      }

   /*  Build the message preamble  */

   ltime = time(NULL);

   if (strftime(timstr, STRLEN, Tfmt, localtime(&ltime)))
      {
      sprintf(preamble, "%s ", timstr);
      }
   else
      {
      sprintf(preamble, "CAN'T DETERMINE TIME STRING! ");
      }

   sprintf(preamble + strlen(preamble), "%s ", Hostname);
   sprintf(preamble + strlen(preamble), "%s", Prefix);
   sprintf(preamble + strlen(preamble), "[%d]", Pid);

   /*  open the file & echo the message (2003-05-15 PLD)  */

   fp = open_log_file();
   if (echo && (fp != stderr) && (fp != stdout))
      printf("%s %s\n", preamble, Crnt_msg);

   /*  Print the message if file is open (2003-05-15 PLD)  */

   if (fp != (FILE*)NULL)
      {
      if (Count > 1)
         {
         fprintf(fp, "%s ", preamble);
         fprintf(fp, "previous message repeated %d times\n", Count);
         Count = 0;
         }
      fprintf(fp, "%s %s\n", preamble, Crnt_msg);
      fflush(fp);
      if (fp != stderr && fp != stdout)
         fclose(fp);

      /*  Save this message for comparison with the next one  */

      ptr = Prev_msg;
      Prev_msg = Crnt_msg;
      Crnt_msg = ptr;
      }

   MUTEX_UNLOCK(&Mutex);
   return ;
} /* end rtp_log() */

#ifdef DEBUG_TEST
   /*====================================================================*/
   /* Test the log functions */

   main(int argc, char **argv)
      {
      int i;

      if (argc != 2)
         {
         fprintf(stderr, "usage: %s logfilename\n", argv[0]);
         exit(1);
         }

      if (strcmp(argv[1], "syslogd") == 0)
         {
         rtp_loginit(argv[1], LOG_LOCAL1, NULL, argv[0]);
         }
      else
         {
         rtp_loginit(argv[1], 0, NULL, argv[0]);
         }

      for (i = 0; i < 1024; i++)
         rtp_log(RTP_INFO, "duplicate message\n");

      while (1)
         {
         rtp_log(RTP_INFO, "log entry %d\n", i++);
         sleep(1);
         }
   } /* end main() */
#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.5  2004/10/08 14:19:21  rstavely
 * Made changes to evnfiles.c to log error better when writing to archive fails.
 * Made changes to win32.c to try 3 times to open file if it fails.
 * Made David Chavez changes to do mutex lock for syslogd in log.c
 *
 * Revision 1.4  2003/05/22 18:24:04  pdavidson
 * Fix: rtp_init() closing stderr,stdout. Fix: rtp_log() mutex unlock. Add: rtp_log() echo option.
 *
 * Revision 1.4  2003/05/19 14:17:23  pdavidson
 * Fix: rtp_loginit() closing stdout,stderr.  Fix: rtp_log() posbl exit w/out mutex unlock.  Add: log echo.
 *
 * Revision 1.3  2003/01/30 04:49:32  dchavez
 * test open log file in rtp_loginit() and return FALSE if it fails
 *
 * Revision 1.2  2002/01/18 17:57:48  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
     */
