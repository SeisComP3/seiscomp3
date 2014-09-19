/***************************************************************************
 * logging.c
 *
 * Log handling routines for libslink
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified: 2005.332
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "libslink.h"

void sl_loginit_main (SLlog * logp, int verbosity,
		      void (*log_print)(const char*), const char * logprefix,
		      void (*diag_print)(const char*), const char * errprefix);

int sl_log_main (SLlog * logp, int level, int verb, va_list * varlist);

/* Initialize the global logging parameters */
SLlog gSLlog = {NULL, NULL, NULL, NULL, 0};


/***************************************************************************
 * sl_loginit:
 *
 * Initialize the global logging parameters.
 *
 * See sl_loginit_main() description for usage.
 ***************************************************************************/
void
sl_loginit (int verbosity,
	    void (*log_print)(const char*), const char * logprefix,
	    void (*diag_print)(const char*), const char * errprefix)
{
  sl_loginit_main(&gSLlog, verbosity, log_print, logprefix, diag_print, errprefix);
}  /* End of sl_loginit() */


/***************************************************************************
 * sl_loginit_r:
 *
 * Initialize SLCD specific logging parameters.  If the logging parameters
 * have not been initialized (slconn->log == NULL) new parameter space will
 * be allocated.
 *
 * See sl_loginit_main() description for usage.
 ***************************************************************************/
void
sl_loginit_r (SLCD * slconn, int verbosity,
	      void (*log_print)(const char*), const char * logprefix,
	      void (*diag_print)(const char*), const char * errprefix)
{
  if ( ! slconn )
    return;

  if ( slconn->log == NULL )
    {
      slconn->log = (SLlog *) malloc (sizeof(SLlog));

      slconn->log->log_print = NULL;
      slconn->log->logprefix = NULL;
      slconn->log->diag_print = NULL;
      slconn->log->errprefix = NULL;
      slconn->log->verbosity = 0;
    }

  sl_loginit_main(slconn->log, verbosity, log_print, logprefix, diag_print, errprefix);
}  /* End of sl_loginit_r() */


/***************************************************************************
 * sl_loginit_rl:
 *
 * Initialize SLlog specific logging parameters.  If the logging parameters
 * have not been initialized (log == NULL) new parameter space will
 * be allocated.
 *
 * See sl_loginit_main() description for usage.
 *
 * Returns a pointer to the created/re-initialized SLlog struct.
 ***************************************************************************/
SLlog *
sl_loginit_rl (SLlog * log, int verbosity,
	       void (*log_print)(const char*), const char * logprefix,
	       void (*diag_print)(const char*), const char * errprefix)
{
  SLlog *logp;

  if ( log == NULL )
    {
      logp = (SLlog *) malloc (sizeof(SLlog));

      logp->log_print = NULL;
      logp->logprefix = NULL;
      logp->diag_print = NULL;
      logp->errprefix = NULL;
      logp->verbosity = 0;
    }
  else
    {
      logp = log;
    }

  sl_loginit_main (logp, verbosity, log_print, logprefix, diag_print, errprefix);

  return logp;
}  /* End of sl_loginit_rl() */


/***************************************************************************
 * sl_loginit_main:
 *
 * Initialize the logging subsystem.  Given values determine how sl_log()
 * and sl_log_r() emit messages.
 *
 * This function modifies the logging parameters in the passed SLlog.
 *
 * Any log/error printing functions indicated must except a single
 * argument, namely a string (const char *).  The sl_log() and
 * sl_log_r() functions format each message and then pass the result
 * on to the log/error printing functions.
 *
 * If the log/error prefixes have been set they will be pre-pended to the
 * message.
 *
 * Use NULL for the function pointers or the prefixes if they should not
 * be changed from previously set or default values.  The default behavior
 * of the logging subsystem is given in the example below.
 *
 * Example: sl_loginit_main (0, (void*)&printf, NULL, (void*)&printf, "error: ");
 ***************************************************************************/
void
sl_loginit_main (SLlog * logp, int verbosity,
		 void (*log_print)(const char*), const char * logprefix,
		 void (*diag_print)(const char*), const char * errprefix)
{
  if ( ! logp )
    return;

  logp->verbosity = verbosity;

  if ( log_print )
    logp->log_print = log_print;

  if ( logprefix )
    {
      if ( strlen(logprefix) >= MAX_LOG_MSG_LENGTH )
	{
	  sl_log_rl (logp, 2, 0, "log message prefix is too large\n");
	}
      else
	{
	  logp->logprefix = logprefix;
	}
    }

  if ( diag_print )
    logp->diag_print = diag_print;

  if ( errprefix )
    {
      if ( strlen(errprefix) >= MAX_LOG_MSG_LENGTH )
	{
	  sl_log_rl (logp, 2, 0, "error message prefix is too large\n");
	}
      else
	{
	  logp->errprefix = errprefix;
	}
    }

  return;
}  /* End of sl_loginit_main() */


/***************************************************************************
 * sl_log:
 *
 * A wrapper to sl_log_main() that uses the global logging parameters.
 *
 * See sl_log_main() description for return values.
 ***************************************************************************/
int
sl_log (int level, int verb, ...)
{
  int retval;
  va_list varlist;
  
  va_start (varlist, verb);

  retval = sl_log_main (&gSLlog, level, verb, &varlist);

  va_end (varlist);

  return retval;
}  /* End of sl_log() */


/***************************************************************************
 * sl_log_r:
 *
 * A wrapper to sl_log_main() that uses the logging parameters in a
 * supplied SLCD. If the supplied pointer is NULL the global logging
 * parameters will be used.
 *
 * See sl_log_main() description for return values.
 ***************************************************************************/
int
sl_log_r (const SLCD * slconn, int level, int verb, ...)
{
  int retval;
  va_list varlist;
  SLlog *logp;

  if ( ! slconn )
    logp = &gSLlog;
  else if ( ! slconn->log )
    logp = &gSLlog;
  else
    logp = slconn->log;
  
  va_start (varlist, verb);
  
  retval = sl_log_main (logp, level, verb, &varlist);

  va_end (varlist);

  return retval;
}  /* End of sl_log_r() */


/***************************************************************************
 * sl_log_rl:
 *
 * A wrapper to sl_log_main() that uses the logging parameters in a
 * supplied SLlog.  If the supplied pointer is NULL the global logging
 * parameters will be used.
 *
 * See sl_log_main() description for return values.
 ***************************************************************************/
int
sl_log_rl (SLlog * log, int level, int verb, ...)
{
  int retval;
  va_list varlist;
  SLlog *logp;

  if ( ! log )
    logp = &gSLlog;
  else
    logp = log;
  
  va_start (varlist, verb);
  
  retval = sl_log_main (logp, level, verb, &varlist);

  va_end (varlist);

  return retval;
}  /* End of sl_log_rl() */


/***************************************************************************
 * sl_log_main:
 *
 * A standard logging/printing routine.
 *
 * This routine acts as a central message facility for the all of the
 * libslink functions.
 *
 * The function uses logging parameters specified in the supplied
 * SLlog.
 * 
 * This function expects 3+ arguments, message level, verbosity level,
 * fprintf format, and fprintf arguments.  If the verbosity level is
 * less than or equal to the set verbosity (see sl_loginit_main()),
 * the fprintf format and arguments will be printed at the appropriate
 * level.
 *
 * Three levels are recognized:
 * 0  : Normal log messages, printed using log_print with logprefix
 * 1  : Diagnostic messages, printed using diag_print with logprefix
 * 2+ : Error messagess, printed using diag_print with errprefix
 *
 * This function builds the log/error message and passes to it as a
 * string (const char *) to the functions defined with sl_loginit() or
 * sl_loginit_r().  If the log/error printing functions have not been
 * defined messages will be printed with fprintf, log messages to
 * stdout and error messages to stderr.
 *
 * If the log/error prefix's have been set with sl_loginit() or
 * sl_loginit_r() they will be pre-pended to the message.
 *
 * All messages will be truncated to the MAX_LOG_MSG_LENGTH, this includes
 * any set prefix.
 *
 * Returns the number of characters formatted on success, and a
 * a negative value on error.
 ***************************************************************************/
int
sl_log_main (SLlog * logp, int level, int verb, va_list * varlist)
{
  static char message[MAX_LOG_MSG_LENGTH];
  int retvalue = 0;
  
  message[0] = '\0';

  if (verb <= logp->verbosity)
    {
      int presize;
      const char *format;

      format = va_arg (*varlist, const char *);

      if ( level >= 2 )  /* Error message */
	{
	  if ( logp->errprefix != NULL )
	    {
	      strncpy (message, logp->errprefix, MAX_LOG_MSG_LENGTH);
	    }
	  else
	    {
	      strncpy (message, "error: ", MAX_LOG_MSG_LENGTH);
	    }

	  presize = strlen(message);
	  retvalue = vsnprintf (&message[presize],
				MAX_LOG_MSG_LENGTH - presize,
				format, *varlist);

	  message[MAX_LOG_MSG_LENGTH - 1] = '\0';

	  if ( logp->diag_print != NULL )
	    {
	      logp->diag_print ((const char *) message);
	    }
	  else
	    {
	      fprintf(stderr, "%s", message);
	    }
	}
      else if ( level == 1 )  /* Diagnostic message */
	{
	  if ( logp->logprefix != NULL )
	    {
	      strncpy (message, logp->logprefix, MAX_LOG_MSG_LENGTH);
	    }

	  presize = strlen(message);
	  retvalue = vsnprintf (&message[presize],
				MAX_LOG_MSG_LENGTH - presize,
				format, *varlist);

	  message[MAX_LOG_MSG_LENGTH - 1] = '\0';

	  if ( logp->diag_print != NULL )
	    {
	      logp->diag_print ((const char *) message);
	    }
	  else
	    {
	      fprintf(stderr, "%s", message);
	    }
	}
      else if ( level == 0 )  /* Normal log message */
	{
	  if ( logp->logprefix != NULL )
	    {
	      strncpy (message, logp->logprefix, MAX_LOG_MSG_LENGTH);
	    }

	  presize = strlen(message);
	  retvalue = vsnprintf (&message[presize],
				MAX_LOG_MSG_LENGTH - presize,
				format, *varlist);

	  message[MAX_LOG_MSG_LENGTH - 1] = '\0';

	  if ( logp->log_print != NULL )
	    {
	      logp->log_print ((const char *) message);
	    }
	  else
	    {
	      fprintf(stdout, "%s", message);
	    }
	}
    }

  return retvalue;
}  /* End of sl_log_main() */
