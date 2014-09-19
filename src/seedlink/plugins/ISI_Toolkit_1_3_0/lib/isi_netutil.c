#pragma ident "$Id: netutil.c,v 1.1 2006/02/09 00:09:20 dechavez Exp $"
/*======================================================================
 *
 *  Additional convenience functions useful in networked apps
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

/* Initialize incoming multipart message buffer */

VOID isiInitIncoming(ISI_INCOMING *incoming)
{
    incoming->type = 0;
    listInit(&incoming->list);
    SEM_INIT(&incoming->semaphore, 0, 1);
}

/* Reset incoming multipart message buffer */

VOID isiResetIncoming(ISI_INCOMING *incoming)
{
    if (incoming == NULL) return;

    incoming->type = 0;
    listDestroy(&incoming->list);
    listInit(&incoming->list);
    SEM_TRYWAIT(&incoming->semaphore);
}

/* Set/Get ISI handle flag */

VOID isiSetFlag(ISI *isi, UINT8 value)
{
    if (isi == NULL) return;
    isi->flag = value;
}

UINT8 isiGetFlag(ISI *isi)
{
    if (isi == NULL) return ISI_FLAG_NOP;
    return isi->flag;
}

BOOL isiGetIacpStats(ISI *isi, IACP_STATS *send, IACP_STATS *recv)
{
    if (isi == NULL || send == NULL || recv == NULL) return FALSE;
    if (!iacpGetSendStats(isi->iacp, send)) return FALSE;
    if (!iacpGetRecvStats(isi->iacp, recv)) return FALSE;

    return TRUE;
}

/* Revision History
 *
 * $Log: netutil.c,v $
 * Revision 1.1  2006/02/09 00:09:20  dechavez
 * initial release
 *
 */
