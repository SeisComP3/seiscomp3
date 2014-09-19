#pragma ident "$Id: close.c,v 1.5 2005/10/10 23:38:04 dechavez Exp $"
/*======================================================================
 * 
 * Close a connection, and free all resources.
 *
 *====================================================================*/
#include "iacp.h"
#include "util.h"

IACP *iacpClose(IACP *iacp)
{
static char *fid = "iacpClose";

    if (iacp == (IACP *) NULL) return NULL;

/* Shutdown the connection */

    if (iacp->sd != INVALID_SOCKET ) {
        shutdown(iacp->sd, 2);
        utilCloseSocket(iacp->sd);
    }
    logioMsg(iacp->lp, LOG_DEBUG, "%s: %s: connection closed", iacpPeerIdent(iacp), fid);

/* Free resources */

    if (iacp->dbgfp != NULL) {
        fclose(iacp->dbgfp);
        iacp->dbgfp = NULL;
    }
    return iacpFree(iacp);
}

/* Revision History
 *
 * $Log: close.c,v $
 * Revision 1.5  2005/10/10 23:38:04  dechavez
 * removed debug tracers
 *
 * Revision 1.4  2005/09/30 22:56:14  dechavez
 * debug tracers added
 *
 * Revision 1.3  2005/01/28 01:58:57  dechavez
 * dbgfp support
 *
 * Revision 1.2  2003/11/19 23:41:01  dechavez
 * include util.h to calm compiler
 *
 * Revision 1.1  2003/06/09 23:50:26  dechavez
 * initial release
 *
 */
