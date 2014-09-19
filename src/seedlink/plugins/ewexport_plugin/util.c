/***************************************************************************
 * network.c
 *
 * General utility routines
 *
 * Written by Chad Trabant, ORFEUS
 *
 * modified: 2009.040
 ***************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

/***************************************************************************
 *  A generic logging/printing routine
 *  This function works in two modes:
 *  1 - Initialization, expecting 2 arguments with the first (level)
 *      being -1 and the second being verbosity.  This will set the
 *      verbosity for all future calls, the default is 0.  Can be used
 *      to change the verbosity at any time. I.e. 'gen_log(-1,2);'
 *      
 *  2 - expecting 3+ arguments, log level, verbosity level, printf
 *      format, and printf arguments.  If the verbosity level is less
 *      than or equal to the set verbosity (see mode 1), the printf
 *      format and arguments will be printed at the appropriate log
 *      level. I.e. 'gen_log(0, 0, "error: %s", result);'
 *      
 *  Returns the new verbosity if using mode 1.
 *  Returns the number of characters formatted on success, and a
 *    a negative value on error if using mode 2.
 ***************************************************************************/
extern int
gen_log(int level, int verb, ... )
{
  static int staticverb = 0;
  int retvalue = -1;
  
  if ( level == -1 ) {
    staticverb = verb;
    retvalue = staticverb;
  }
  else if (verb <= staticverb) {
    char message[200];
    char timestr[200];
    char *format;
    va_list listptr;
    time_t loc_time;
 
    va_start(listptr, verb);
    format = va_arg(listptr, char *);
 
    /* Build local time string and cut off the newline */
    time(&loc_time);
    strcpy(timestr, asctime(localtime(&loc_time)));
    timestr[strlen(timestr) - 1] = '\0';
 
    retvalue = vsnprintf(message, 200, format, listptr);
 
    if ( level == 1 ) {
      printf("%s - ewexport_plugin: error: %s",timestr, message);
    }
    else {
      printf("%s - ewexport_plugin: %s", timestr, message);
    }
 
    fflush(stdout);
    va_end(listptr);
  }
 
  return retvalue;
} /* End of gen_log() */


/***************************************************************************
 * safe_usleep():
 *
 * Sleep for a given number of microseconds using nanosleep which will not
 * affect threads/signals.
 ***************************************************************************/
extern void
safe_usleep (unsigned long int useconds)
{
  struct timespec treq, trem;
  
  treq.tv_sec = (time_t) (useconds / 1e6);
  treq.tv_nsec = (long) ((useconds * 1e3) - (treq.tv_sec * 1e9));
  
  nanosleep (&treq, &trem);
} /* End of safe_usleep() */

