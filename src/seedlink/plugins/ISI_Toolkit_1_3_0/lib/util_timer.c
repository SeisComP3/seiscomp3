#pragma ident "$Id: timer.c,v 1.4 2006/05/16 00:02:11 dechavez Exp $"
/*======================================================================
 *
 *  Nanosecond timer
 *
 *====================================================================*/
#include "util.h"

VOID utilDelayMsec(UINT32 interval)
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

INT64 utilTimeStamp(void)
{
INT64 retval;
struct timeval timeval;

    gettimeofday(&timeval, NULL);

    retval = (INT64) timeval.tv_sec * NANOSEC_PER_SEC;
    retval += (INT64) timeval.tv_usec * NANOSEC_PER_USEC;

    return retval;
}

void utilStartTimer(UTIL_TIMER *timer, UINT64 interval)
{
    MUTEX_LOCK(&timer->mutex);
        timer->interval = interval;
        timer->tstamp = utilTimeStamp();
    MUTEX_UNLOCK(&timer->mutex);
}

void utilInitTimer(UTIL_TIMER *timer)
{
    MUTEX_INIT(&timer->mutex);
    utilStartTimer(timer, 0);
}

void utilResetTimer(UTIL_TIMER *timer)
{
    MUTEX_LOCK(&timer->mutex);
        timer->tstamp = utilTimeStamp();
    MUTEX_UNLOCK(&timer->mutex);
}

UINT64 utilElapsedTime(UTIL_TIMER *timer)
{
UINT64 result;

    MUTEX_LOCK(&timer->mutex);
        result = utilTimeStamp() - timer->tstamp;
    MUTEX_UNLOCK(&timer->mutex);

    return result;
}

BOOL utilTimerExpired(UTIL_TIMER *timer)
{
BOOL result;

    MUTEX_LOCK(&timer->mutex);
        result = utilTimeStamp() - timer->tstamp > timer->interval ? TRUE : FALSE;
    MUTEX_UNLOCK(&timer->mutex);

    return result;
}

/* Revision History
 *
 * $Log: timer.c,v $
 * Revision 1.4  2006/05/16 00:02:11  dechavez
 * changed utilStartTimer() interval parameter from int to UINT64
 *
 * Revision 1.3  2006/04/27 19:59:45  dechavez
 * added utilStartTimer(), utilInitTimer(), utilElapsedTime() and utilTimerExpired()
 *
 * Revision 1.2  2005/06/30 01:23:03  dechavez
 * INT64 timestamp support
 *
 * Revision 1.1  2003/10/16 16:07:54  dechavez
 * initial release
 *
 */
