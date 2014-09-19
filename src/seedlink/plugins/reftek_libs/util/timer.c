#pragma ident "$Id: timer.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
   Program  :  Any
   Task     :  Any needing 48 bit timers
   File     :  TIMER.C
   Purpose  :  48 bit millisecond timer functions
   Host     :  CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
   Target   :  Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
   Packages :  None
   Author   :  Robert Banfill (r_banfill@reftek.com)
   Company  :  Refraction Technology, Inc.
               2626 Lombardy Lane, Suite 105
               Dallas, Texas  75220  USA
               (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
   Copyright:  (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
   Notes    :    Y2K compliant
   $Revision: 165 $
   $Logfile :  R:/cpu68000/rt422/struct/version.h_v  $
   Revised  :
      25Mar97  ---- (RLB) Initial version.

----------------------------------------------------------------------- */

/* #define TIMER48_UNIT_TEST */

#define _TIMER_C
#include "platform.h"
#include "timer.h"

/*--------------------------------------------------------------------- */
static TIMER48 Timer48;
static UINT32  Last_clock;
static MUTEX   mutex;
static struct timeb BASETIME;
static BOOL initialized = FALSE;

/*--------------------------------------------------------------------- */
static UINT32 MillisecondClock();
static VOID UpdateSystemTimer(TIMER48 *timer);

/*--------------------------------------------------------------------- */
VOID Timer48Init(VOID) 
{
    if(initialized) 
        return;

    MUTEX_INIT(&mutex); 

    ftime(&BASETIME);

    Timer48.upper = 0;
    Timer48.lower = 0;
    Timer48.interval = 0;

    Last_clock = 0;

    initialized = TRUE;
}

/*--------------------------------------------------------------------- */
static UINT32 MillisecondClock()
{
struct timeb tb;

    ftime(&tb);

    tb.time -= BASETIME.time;

    return (((UINT32)tb.time * 1000) + (UINT32)tb.millitm) - (UINT32)BASETIME.millitm;
}

/*--------------------------------------------------------------------- */
static VOID UpdateSystemTimer(TIMER48 *timer)
{

    MUTEX_LOCK(&mutex);

    /* Load system timer with the current time */
    Timer48.lower = MillisecondClock();

#if 0
    if(Timer48.lower < Last_clock)
        Timer48.upper++;
    Last_clock = Timer48.lower;
#endif

    /* If timer arg is not null, copy current time into it */
    if(timer != NULL) {
        timer->upper = Timer48.upper;
        timer->lower = Timer48.lower;
    }

    MUTEX_UNLOCK(&mutex);
}

/*---------------------------------------------------------------------
 * 48 bit millisecond timer (period is ~8920 years)
 *
 * Type timer is defined in TIMER.H.
 *
 * TIMER is a pointer to a 48 bit timer structure.
 * UINT32 interval is the interval of the timer in milliseconds.
 *
 * The maximum timer interval is 49+17:02:47.295.
 */

VOID Timer48Start( TIMER48 * timer, UINT32 interval )
{
    if(!initialized)
        Timer48Init();

    UpdateSystemTimer(timer);

    /* Store interval */
    timer->interval = interval;

    return;
}

/*---------------------------------------------------------------------
 * Restart the timer
 */

VOID Timer48Restart( TIMER48 * timer )
{
    if(!initialized)
        Timer48Init();
 
    UpdateSystemTimer(timer);

    return;
}

/*---------------------------------------------------------------------
 * Return TRUE if timer has expired, FALSE if not
 */

BOOL Timer48Expired( TIMER48 * timer )
{
 
    if(Timer48MSElapsed(timer) >= timer->interval)
        return TRUE;

    return FALSE;
}

/*---------------------------------------------------------------------
 * Returns elapsed time in milliseconds from timer start
 * Largest millisecond interval: 49+17:02:47.295
 */

UINT32 Timer48MSElapsed( TIMER48 * timer )
{
    UINT32 delta;
    UINT16 borrow;
    TIMER48 sys;

    if(!initialized)
        Timer48Init();

    UpdateSystemTimer(&sys);

    /* Compute delta */
    if((delta = sys.lower - timer->lower) > sys.lower)
        borrow = 1;
    else
        borrow = 0;

    /* If delta overflows 32 bits, return 0 */
    if(sys.upper - timer->upper - borrow != 0)
        return 0L;

    return delta;
}

/*---------------------------------------------------------------------
 * Returns elapsed time in seconds from timer start
 * Largest second interval: 136*070+06:28:15
 */

UINT32 Timer48Elapsed( TIMER48 * timer )
{
    TIMER48 delta;
    UINT32 temp;
    UINT16 borrow;
    TIMER48 sys;

    if(!initialized)
        Timer48Init();

    UpdateSystemTimer(&sys);

    /* Compute delta */
    if ( ( delta.lower = sys.lower - timer->lower ) > sys.lower )
        borrow = 1;
    else
        borrow = 0;
    delta.upper = sys.upper - timer->upper - borrow;

    /* Divide by 1024, 10 bits */
    delta.lower >>= 10;
    temp = ( UINT32 ) ( delta.upper & 0x0400 );
    temp <<= 22;
    delta.lower |= temp;

    return ( delta.lower );
}

/*---------------------------------------------------------------------
 * Pause for specified number of milliseconds
 * Largest millisecond interval: 0xFFFFFFFF = 49+17:02:47.295
 */

VOID MSPause( UINT32 interval )
{
#ifdef unix
int status;
struct timespec rqtp, rmtp;

    rqtp.tv_sec = (time_t) interval / 1000;
    rqtp.tv_nsec = (long) (1000000 * (interval % 1000));
    
    do {
        status = nanosleep(&rqtp, &rmtp);
        if (status != 0) {
            if (errno != EINTR) return;
            rqtp = rmtp;
        }
    } while (status != 0);
#else
    Sleep(interval);
#endif
    return;
}

/*---------------------------------------------------------------------
 * Return millisecond interval as formatted ASCII string
 * Largest millisecond interval: 049+17:02:47.295
 */

CHAR *MSIntervalToString( CHAR * string, UINT32 interval, UINT16 format )
{
    BOOL started;
    UINT32 temp;

    started = FALSE;
    string[0] = 0;

    if ( interval >= DAY_MS ) {
        temp = interval / DAY_MS;
        interval -= temp * DAY_MS;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && temp != 0 ) {
        started = TRUE;
        sprintf( string, "%ld %s, ", temp, ( temp == 1 ? "day" : "days" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( string, "%03ld+", temp );

    if ( interval >= HOUR_MS ) {
        temp = interval / HOUR_MS;
        interval -= temp * HOUR_MS;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) ) {
        started = TRUE;
        sprintf( &string[strlen( string )], "%ld %s, ", temp, ( temp == 1 ? "hour" : "hours" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld:", temp );

    if ( interval >= MINUTE_MS ) {
        temp = interval / MINUTE_MS;
        interval -= temp * MINUTE_MS;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) ) {
        started = TRUE;
        sprintf( &string[strlen( string )], "%ld %s, ", temp, ( temp == 1 ? "minute" : "minutes" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld:", temp );

    temp = interval / SECOND_MS;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) )
        sprintf( &string[strlen( string )], "%ld.", temp );
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld.", temp );

    interval -= temp * SECOND_MS;
    if ( format == FMT_VERBOSE )
        sprintf( &string[strlen( string )], "%03ld seconds", interval );
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%03ld", interval );

    return ( string );
}

/*---------------------------------------------------------------------
 * Return second interval as formatted ASCII string
 * Largest second interval: 136*070+06:28:15
 */

CHAR *IntervalToString( CHAR * string, UINT32 interval, UINT16 format )
{
    BOOL started;
    UINT32 temp;

    started = FALSE;
    string[0] = '\0';

    if ( interval >= YEAR ) {
        temp = interval / YEAR;
        interval -= temp * YEAR;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && temp != 0 ) {
        started = TRUE;
        sprintf( string, "%ld %s, ", temp, ( temp == 1 ? "year" : "years" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( string, "%ld*", temp );

    if ( interval >= DAY ) {
        temp = interval / DAY;
        interval -= temp * DAY;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) ) {
        started = TRUE;
        sprintf( &string[strlen( string )], "%ld %s, ", temp, ( temp == 1 ? "day" : "days" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%03ld+", temp );

    if ( interval >= HOUR ) {
        temp = interval / HOUR;
        interval -= temp * HOUR;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) ) {
        started = TRUE;
        sprintf( &string[strlen( string )], "%ld %s, ", temp, ( temp == 1 ? "hour" : "hours" ) );
    }
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld:", temp );

    if ( interval >= MINUTE ) {
        temp = interval / MINUTE;
        interval -= temp * MINUTE;
    }
    else
        temp = 0;
    if ( format == FMT_VERBOSE && ( temp != 0 || started ) )
        sprintf( &string[strlen( string )], "%ld %s, ", temp, ( temp == 1 ? "minute" : "minutes" ) );
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld:", temp );

    temp = interval;
    if ( format == FMT_VERBOSE )
        sprintf( &string[strlen( string )], "%ld seconds", temp );
    else if ( format == FMT_DEFAULT )
        sprintf( &string[strlen( string )], "%02ld", temp );

    return ( string );
}

/*---------------------------------------------------------------------*/
#if defined TIMER48_UNIT_TEST

static BOOL stop = FALSE;

static BOOL SetSignalHandlers(VOID);
static VOID CatchSignal(int sig);

int main(int argc, char *argv[])
{
    UINT32 interval, elapsed, sleep_period;
    TIMER48 timer;

    if(!SetSignalHandlers())
        exit(1);

    if(argc > 1)
        interval = strtoul(argv[1], NULL, 0);
    else
        interval = SECOND_MS * 10;

    if(argc > 2)
        sleep_period = strtoul(argv[2], NULL, 0);
    else {
        sleep_period = interval / 10;
        if(sleep_period > 100)
            sleep_period = 100;
    }

    printf("Timer interval: %u.%03u, sleep period: %u.%03u\n", 
        interval / 1000, interval % 1000, 
        sleep_period / 1000, sleep_period % 1000);

    Timer48Start(&timer, interval);
    printf("timer.upper: %u, timer.lower: %u, timer.interval: %u\n", 
        timer.upper, timer.lower, timer.interval);
    while(!stop) {
        elapsed = Timer48MSElapsed(&timer);
        if(Timer48Expired(&timer)) {
            printf("\rTimer expired: %u.%03u\n", elapsed / 1000, elapsed % 1000);
            Timer48Restart(&timer);
            printf("timer.upper: %u, timer.lower: %u, timer.interval: %u\n", 
                timer.upper, timer.lower, timer.interval);
        }
        else {
            printf("\rElapsed: %u.%03u", elapsed / 1000, elapsed % 1000);
            MSPause(sleep_period);
        }
    }

    return 0;
}

/*--------------------------------------------------------------------- */
static BOOL SetSignalHandlers(VOID)
{
	if (signal(SIGTERM, CatchSignal) == SIG_ERR) {
		perror("signal(SIGTERM)");
		return FALSE;
	}
	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		perror("signal(SIGINT)");
		return FALSE;
	}

	return TRUE;
}

/*--------------------------------------------------------------------- */
static VOID CatchSignal(int sig)
{
	switch (sig) {
	  case SIGTERM:
		fprintf(stderr, "\rCaught SIGTERM! \n");
		stop = TRUE;
		break;
	  case SIGINT:
		fprintf(stderr, "\rCaught SIGINT! \n");
		stop = TRUE;
		break;
	  default:
		fprintf(stderr, "\rCaught unexpected signal %d, ignored! \n", sig);
		break;
	}

	signal(sig, CatchSignal);
}

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2001/07/23 19:00:15  nobody
 * Removed wrap test on 48 bit timers
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
