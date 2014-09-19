#pragma ident "$Id: set.c,v 1.1 2003/10/16 16:33:46 dechavez Exp $"
/*======================================================================
 *
 *  Insert parameters into the handle in an MT-safe fashion
 *
 *====================================================================*/
#include "iacp.h"

VOID iacpSetDebug(IACP *iacp, int value)
{
    if (iacp == NULL) return;

    MUTEX_LOCK(&iacp->mutex);
        iacp->debug = value;
    MUTEX_UNLOCK(&iacp->mutex);
}

VOID iacpSetLogio(IACP *iacp, LOGIO *logio)
{
    if (iacp == NULL) return;

    MUTEX_LOCK(&iacp->mutex);
        iacp->lp = logio;
    MUTEX_UNLOCK(&iacp->mutex);
}

VOID iacpSetRetryFlag(IACP *iacp, BOOL value)
{
    if (iacp == NULL) return;

    MUTEX_LOCK(&iacp->mutex);
        iacp->attr.at_retry = value;
    MUTEX_UNLOCK(&iacp->mutex);
}

VOID iacpSetDisabled(IACP *iacp, BOOL value)
{
    if (iacp == NULL) return;

    MUTEX_LOCK(&iacp->mutex);
        iacp->disabled = value;
    MUTEX_UNLOCK(&iacp->mutex);
}

/* Revision History
 *
 * $Log: set.c,v $
 * Revision 1.1  2003/10/16 16:33:46  dechavez
 * initial release
 *
 */
