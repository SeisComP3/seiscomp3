#pragma ident "$Id: get.c,v 1.2 2005/05/06 01:05:22 dechavez Exp $"
/*======================================================================
 *
 *  Retrive parameters from the handle in an MT-safe fashion
 *
 *====================================================================*/
#include "iacp.h"

time_t iacpGetStartTime(IACP *iacp)
{
time_t retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return 0;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->connect;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

int iacpGetSendError(IACP *iacp)
{
int retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return -1;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->send.error;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

time_t iacpGetSendTstamp(IACP *iacp)
{
time_t retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return -1;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->send.tstamp;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

int iacpGetRecvError(IACP *iacp)
{
int retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return -1;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->recv.error;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

int iacpGetRecvStatus(IACP *iacp)
{
int retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return -1;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->recv.status;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

time_t iacpGetRecvTstamp(IACP *iacp)
{
time_t retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return -1;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->recv.tstamp;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

int iacpGetDebug(IACP *iacp)
{
int retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return 0;
    }
    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->debug;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

UINT32 iacpGetTimeoutInterval(IACP *iacp)
{
UINT32 retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return 0xFFFFFFFF;
    }

    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->attr.at_timeo;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

IACP_ATTR *iacpGetAttr(IACP *iacp, IACP_ATTR *attr)
{
    if (iacp == NULL || attr == NULL) {
        errno = EINVAL;
        return NULL;
    }

    MUTEX_LOCK(&iacp->mutex);
        *attr = iacp->attr;
    MUTEX_UNLOCK(&iacp->mutex);

    return attr;
}

UINT32 iacpGetRetryInterval(IACP *iacp)
{
UINT32 retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return 0xFFFFFFFF;
    }

    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->attr.at_wait;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

UINT32 iacpGetMaxError(IACP *iacp)
{
UINT32 retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return 0xFFFFFFFF;
    }

    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->attr.at_maxerr;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

BOOL iacpGetRetry(IACP *iacp)
{
BOOL retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->attr.at_retry;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

BOOL iacpGetDisabled(IACP *iacp)
{
BOOL retval;

    if (iacp == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    MUTEX_LOCK(&iacp->mutex);
        retval = iacp->disabled;
    MUTEX_UNLOCK(&iacp->mutex);

    return retval;
}

static IACP_STATS *CopyStats(IACP_STATS *src, IACP_STATS *dest)
{
    MUTEX_LOCK(src->mutex);
        *dest = *src;
    MUTEX_UNLOCK(src->mutex);

    return dest;
}

IACP_STATS *iacpGetRecvStats(IACP *iacp, IACP_STATS *dest)
{
    if (iacp == NULL || dest == NULL) {
        errno = EINVAL;
        return NULL;
    }

    return CopyStats(&iacp->recv.stats, dest);
}

IACP_STATS *iacpGetSendStats(IACP *iacp, IACP_STATS *dest)
{
    if (iacp == NULL || dest == NULL) {
        errno = EINVAL;
        return NULL;
    }

    return CopyStats(&iacp->send.stats, dest);
}

/* Revision History
 *
 * $Log: get.c,v $
 * Revision 1.2  2005/05/06 01:05:22  dechavez
 * added support for IACP_STATS
 *
 * Revision 1.1  2003/10/16 16:33:46  dechavez
 * initial release
 *
 */
