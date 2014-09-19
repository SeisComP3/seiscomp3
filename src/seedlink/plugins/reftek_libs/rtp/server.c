#pragma ident "$Id: server.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Set self up as a server listening on the specified port.
 *
 *====================================================================*/
#include "rtp.h"

RTP *rtp_server(UINT16 port, UINT16 backlog, struct rtp_attr *user_attr)
{
RTP *rtp;
struct rtp_attr attr;
struct sockaddr_in serv_addr;
static char *fid = "rtp_server";

/* Get default attributes */
 
    if (user_attr != (struct rtp_attr *) NULL) {
        attr = *user_attr;
    } else {
        attr = RTP_DEFAULT_ATTR;
    }
 
/* Silently force timeout to be at least our minimum (so we can do
 * heartbeats at half that interval)
 */
 
    if (attr.at_timeo < RTP_MINTIMEO) attr.at_timeo = RTP_MINTIMEO;

/* Create/fill the handle */
 
    if ((rtp = (RTP *) malloc(sizeof(RTP))) == (RTP *) NULL) {
        rtp_log(RTP_ERR, "%s: malloc: %s", fid, strerror(errno));
        return (RTP *) NULL;
    }

    rtp->port = port;
    rtp->attr = attr;

/* Create socket and bind */

    if ((rtp->sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        rtp_log(RTP_ERR, "%s: socket: %s", fid, strerror(errno));
        free(rtp);
        return (RTP *) NULL;
    }
    memset((void *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
 
    if (bind(
        rtp->sd,
        (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)
    ) != 0) {
        rtp_log(RTP_ERR, "%s: bind: %s", fid, strerror(errno));
        free(rtp);
        return (RTP *) NULL;
    }
 
/* Start listening for connectinos */
 
    if (listen(rtp->sd, (int) backlog) != 0) {
        rtp_log(RTP_ERR, "%s: listen: %s", fid, strerror(errno));
        free(rtp);
        return (RTP *) NULL;
    }

/* Return handle for this connection */

    return rtp;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:49  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
