#pragma ident "$Id: open.c,v 1.5 2007/01/23 02:52:58 dechavez Exp $"
/*======================================================================
 * 
 * Establish a connection with an IACP server.  If there are connect
 * errors, then the function will either quit or retry depending on
 * the value of the retry flag in the attributes.
 *
 *====================================================================*/
#include "iacp.h"

IACP *iacpOpen(char *server, int port, IACP_ATTR *attr, LOGIO *lp, int debug)
{
IACP *iacp;
static char *fid = "iacpOpen";

/* Argument check */

    if (server == (char *) NULL || port <= 0) {
        logioMsg(lp, LOG_INFO, "%s: invalid argument(s)", fid);
        errno = EINVAL;
        return (IACP *) NULL;
    }

/* Create handle */

    if ((iacp = (IACP *) malloc(sizeof(IACP))) == NULL) {
        logioMsg(lp, LOG_INFO, "%s: malloc: %s", fid, strerror(errno));
        return iacpClose(iacp);
    }
    iacpInitHandle(iacp, server, port, attr, lp, debug);
    if (debug > 0) logioSetThreshold(lp, LOG_DEBUG);

/* Establish connection with server, including handshake */

    if (!iacpConnect(iacp)) {
        logioMsg(lp, LOG_INFO, "%s: unable to connect to %s:%d", fid, server, port);
        return iacpClose(iacp);
    }

/* Successful connection/handshake, return the handle */

    return iacp;
}

/* Revision History
 *
 * $Log: open.c,v $
 * Revision 1.5  2007/01/23 02:52:58  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.4  2005/09/30 22:16:55  dechavez
 * improved error message on iacpConnect failure
 *
 * Revision 1.3  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:44:11  dechavez
 * set logging threshold to reflect debug parameter
 *
 * Revision 1.1  2003/06/09 23:50:27  dechavez
 * initial release
 *
 */
