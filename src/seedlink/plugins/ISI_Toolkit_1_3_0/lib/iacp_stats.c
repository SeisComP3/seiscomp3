#pragma ident "$Id: stats.c,v 1.2 2005/05/23 20:59:55 dechavez Exp $"
/*======================================================================
 * 
 *  IACP I/O stats
 *
 *====================================================================*/
#include "iacp.h"

VOID iacpIncrByteCounter(IACP_STATS *stats, int increment, BOOL AlreadyLocked)
{
    if (!AlreadyLocked) MUTEX_LOCK(stats->mutex);
        stats->nbyte += increment;
    if (!AlreadyLocked) MUTEX_UNLOCK(stats->mutex);
}

VOID iacpUpdateFrameStats(IACP_STATS *stats, UINT32 len, BOOL AlreadyLocked)
{
    if (!AlreadyLocked) MUTEX_LOCK(stats->mutex);
        ++stats->nframe;
        if (len > stats->maxlen) stats->maxlen = len;
        if (len < stats->minlen) stats->minlen = len;
        stats->sumlen += (UINT64) len;
        stats->avelen  = (UINT32) (stats->sumlen / (UINT64) stats->nframe);
        stats->ssdiff  = (UINT64) (len * len);
#ifdef WIN32
/* no conversion unsigned _int64 -> double in VC++ 6.0 */
        stats->stddev  = (UINT32) (sqrt((double)(INT64)(stats->ssdiff / stats->nframe)));
#else
        stats->stddev  = (UINT32) (sqrt((double) (stats->ssdiff / stats->nframe)));
#endif
    if (!AlreadyLocked) MUTEX_UNLOCK(stats->mutex);
}

VOID iacpInitStats(IACP_STATS *stats, MUTEX *mutex)
{
    stats->mutex = mutex;
    stats->nbyte  = 0;
    stats->nframe = 0;
    stats->maxlen = 0;
    stats->minlen = 0;
    stats->avelen = 0;
    stats->stddev = 0;
    stats->sumlen = 0;
    stats->ssdiff = 0;
}

/* Revision History
 *
 * $Log: stats.c,v $
 * Revision 1.2  2005/05/23 20:59:55  dechavez
 * WIN32 mods (05-23 update AAP)
 *
 * Revision 1.1  2005/05/06 01:04:24  dechavez
 * created
 *
 */
