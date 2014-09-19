#pragma ident "$Id: connect.c,v 1.14 2007/01/23 02:52:58 dechavez Exp $"
/*======================================================================
 * 
 * Connect to server with retry
 *
 *====================================================================*/
#include "iacp.h"
#include "util.h"

static VOID SetErrorLevel(int ErrorCode, int *ErrorLevel, int *LogLevel)
{
    *ErrorLevel = iacpErrorLevel(ErrorCode);
    *LogLevel = (*ErrorLevel == IACP_ERR_TRANSIENT) ? LOG_DEBUG : LOG_INFO;
}

BOOL iacpConnect(IACP *iacp)
{
int ErrorLevel, LogLevel;
int keepalive = 1;
int ilen = sizeof(int);
unsigned int RetryInterval;
struct sockaddr_in serverAddr;
static char *fid = "iacpConnect";

    if (iacp == (IACP *) NULL) {
        errno = EINVAL;
        return FALSE;
    }

/* Set the server address */

    if (!utilSetHostAddr(&serverAddr, iacp->peer.name, iacp->port)) {
        logioMsg(iacp->lp, LOG_INFO, "%s: server %s: %s", fid, iacp->peer.name, strerror(errno));
        return FALSE;
    }

    iacp->sd = INVALID_SOCKET;
    while (1) {

/* create the socket */

        if ((iacp->sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            logioMsg(iacp->lp, LOG_INFO, "%s: socket: %s", fid, strerror(errno));
            return FALSE;
        }

/* set initial socket options */

        setsockopt(iacp->sd, SOL_SOCKET, SO_KEEPALIVE, (char *) &keepalive, ilen);
        if (iacp->attr.at_sndbuf > 0) {
            setsockopt(iacp->sd, SOL_SOCKET, SO_SNDBUF, (char *) &iacp->attr.at_sndbuf, ilen);
        }
        if (iacp->attr.at_rcvbuf > 0) {
            setsockopt(iacp->sd, SOL_SOCKET, SO_RCVBUF, (char *) &iacp->attr.at_rcvbuf, ilen);
        }

/* attempt to connect */

        if (connect(iacp->sd,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr_in)) < 0) {
            SetErrorLevel(errno, &ErrorLevel, &LogLevel);
            logioMsg(iacp->lp, LogLevel, "%s: connect: %s", fid, strerror(errno));
            iacp->sd = utilCloseSocket(iacp->sd);
        }

/* if connected OK, update handle and attempt the handshake */

        if (iacp->sd != INVALID_SOCKET) {
            utilSetNonBlockingSocket(iacp->sd);
            iacp->connect = time(NULL);
            utilPeerAddr(iacp->sd, iacp->peer.addr, INET_ADDRSTRLEN);
            utilPeerName(iacp->sd, iacp->peer.name, MAXPATHLEN);
            sprintf(iacp->peer.ident, "pid?@", iacp->peer.name);
            if (iacpClientHandshake(iacp)) return TRUE;
            SetErrorLevel(errno, &ErrorLevel, &LogLevel);
        }

/* must have encountered an error, either in connect or handshake */

        if ((UINT32) ErrorLevel > iacpGetMaxError(iacp)) {
            return FALSE;
        } else {
            iacp->sd = utilCloseSocket(iacp->sd);
            RetryInterval = iacpGetRetryInterval(iacp);
            logioMsg(iacp->lp, LOG_DEBUG, "%s: delay %lu msec", fid, RetryInterval);
            utilDelayMsec(RetryInterval);
        }
    }
}

BOOL iacpReconnect(IACP *iacp)
{
    iacp->sd = utilCloseSocket(iacp->sd);
    return iacpConnect(iacp);
}

/* Revision History
 *
 * $Log: connect.c,v $
 * Revision 1.14  2007/01/23 02:52:58  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.13  2007/01/07 17:26:15  dechavez
 * fixed bug preserving errno in iacpConnect
 * CVeS: ----------------------------------------------------------------------
 *
 * Revision 1.12  2005/09/30 22:14:37  dechavez
 * fixed error message on utilSetHostAddr failure
 *
 * Revision 1.11  2005/05/25 22:37:39  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.10  2004/06/18 21:11:27  dechavez
 * removed at_myport support
 *
 * Revision 1.9  2004/06/17 20:01:27  dechavez
 * set client side port with new at_myport attribute
 *
 * Revision 1.8  2004/01/29 18:46:39  dechavez
 * use iacpErrorLevel() to test for severity
 *
 * Revision 1.7  2003/12/22 18:48:25  dechavez
 * expanded list of transient error conditions
 *
 * Revision 1.6  2003/11/19 23:40:50  dechavez
 * include util.h to calm compiler
 *
 * Revision 1.5  2003/11/04 20:04:38  dechavez
 * use utilDelayMsec(RetryInterval) instead of sleep(), units changed to msec
 *
 * Revision 1.4  2003/11/04 00:33:08  dechavez
 * use utilCloseSocket() to reset value if sd in IACP handle
 *
 * Revision 1.3  2003/10/16 16:36:31  dechavez
 * Many bug fixes and enhancements, to numerous to mention.
 *
 * Revision 1.2  2003/06/30 18:43:18  dechavez
 * log error messages instead of error codes
 *
 * Revision 1.1  2003/06/09 23:50:26  dechavez
 * initial release
 *
 */
