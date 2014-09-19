#pragma ident "$Id: accept.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Accept new client connections and complete handshake.
 *
 *====================================================================*/
#include "rtp.h"

extern BOOL set_attr(RTP *rtp, struct rtp_attr *attr); /* server supplied */

/* Server side of the handshake */

static BOOL handshake(RTP *rtp)
{
UINT16 type;
INT16 client_version;
INT32 unused;
struct rtp_attr attr;
UINT8 msgbuf[RTP_MAXMSGLEN];
static CHAR *fid = "handshake";

/* Get the client protocol version number */
 
    rtp_log(RTP_DEBUG, "%s: read client protocol version", fid);
    if ((client_version = rtp_version_recv(rtp)) < 0) {
        rtp_log(RTP_ERR, "%s: rtp_version_recv: error %d",
            fid, client_version
        );
        return FALSE;
    }
    rtp_log(RTP_DEBUG, "%s: client version is %d", fid, client_version);

/* Send back our version number */
 
    rtp_log(RTP_DEBUG, "%s: send server protcol version %d",
        fid, RTP_VERSION
    );
    if (!rtp_version_send(rtp)) {
        rtp_log(RTP_ERR, "%s: rtp_version_send failed", fid);
        return FALSE;
    }

/* If the client is a higher version, protocol says quit now */
 
    if (client_version > RTP_VERSION) {
        rtp_log(RTP_ERR, "%s: unsupported protocol version (%d)",
            fid, client_version
        );
        return FALSE;
    }
 
/* Following is specific to protocol version */
 
    if (client_version == 1) {

    /* client should begin with its process id */

        if (!rtp_recv(rtp, msgbuf, &type, &unused)) {
            rtp_log(RTP_ERR, "%s: rtp_recv failed", fid);
            return FALSE;
        }

        if (type != RTP_MSG_PID) {
            rtp_log(RTP_ERR, "%s: unexpected message type %d != %d",
                fid, type, RTP_MSG_ATTR
            );
            errno = EPROTO;
            return FALSE;
        }

        rtp_pid_decode(msgbuf, &rtp->pid);
        rtp_log(RTP_DEBUG, "%s: client process id is %d", fid, rtp->pid);

    /* which we acknowledge with our process id */

        rtp_log(RTP_DEBUG, "%s: send pid ack", fid);
        if (!rtp_pid_send(rtp)) {
            rtp_log(RTP_ERR, "%s: rtp_pid_send failed", fid);
            return FALSE;
        }

    /* client should then send its attribute request */

        if (!rtp_recv(rtp, msgbuf, &type, &unused)) {
            rtp_log(RTP_ERR, "%s: rtp_recv failed", fid);
            return FALSE;
        }

        if (type != RTP_MSG_ATTR) {
            rtp_log(RTP_ERR, "%s: unexpected message type %d != %d",
                fid, type, RTP_MSG_ATTR
            );
            errno = EPROTO;
            return FALSE;
        }

        rtp_attr_decode(msgbuf, &attr);
        rtp_log(RTP_DEBUG, "%s: client attribute request received", fid);

    /* which will load in and echo back (maybe change it, too) */

        if (!set_attr(rtp, &attr)) {
            rtp_log(RTP_ERR, "%s: set_attr failed", fid);
            rtp_break(rtp);
            return FALSE;
        }

        rtp_log(RTP_DEBUG, "%s: send attribute ack", fid);
        if (!rtp_attr_send(rtp, &attr)) {
            return FALSE;
        }

    } else {
        rtp_log(RTP_ERR, "%s: unsupported protocol version (%d)",
            fid, client_version
        );
        errno = EPROTO;
        rtp_break(rtp);
        return FALSE;
    }

    rtp_log(RTP_DEBUG, "%s: handshake complete", fid);
    return TRUE;
}

/* Get the name of one's peer */

static MUTEX mutex = MUTEX_INITIALIZER;

static BOOL peer_info(RTP *rtp)
{
int addrlen;
struct sockaddr_in cli_addr, *cli_addrp;
struct hostent *hp;
static CHAR *fid = "peer_info";

    addrlen = sizeof(cli_addr);
    cli_addrp = &cli_addr;
    if (getpeername(rtp->sd, (struct sockaddr *)cli_addrp, &addrlen) != 0) {
        rtp_log(RTP_ERR, "%s: getpeername: %s", fid, strerror(errno));
        return FALSE;
    }
    rtp->addr = (CHAR *) strdup(inet_ntoa(cli_addrp->sin_addr));
    MUTEX_LOCK(&mutex);
        hp = gethostbyaddr(
            (char *) &cli_addrp->sin_addr,
            sizeof(struct in_addr),
            cli_addrp->sin_family
        );
        if (hp != NULL) {
            rtp->peer = (CHAR *) strdup(hp->h_name);
        } else {
            rtp->peer = (CHAR *) strdup(rtp->addr);
        }
    MUTEX_UNLOCK(&mutex);

    if (rtp->peer == (CHAR *) NULL) {
        rtp_log(RTP_ERR, "%s: strdup: %s", fid, strerror(errno));
        return FALSE;
    }

    rtp->port = (UINT16) ntohs(cli_addr.sin_port);

    return TRUE;
}

/* Accept incoming connection */

RTP *rtp_accept(RTP *server)
{
RTP *rtp;
struct sockaddr_in cli_addr;
int client, len = sizeof(cli_addr);
static CHAR *fid = "rtp_accept";
 
/* Create/fill the handle */
 
    if ((rtp = (RTP *) malloc(sizeof(RTP))) == (RTP *) NULL) {
        rtp_log(RTP_ERR, "%s: malloc: %s", fid, strerror(errno));
        return (RTP *) NULL;
    }

    rtp->sd   = server->sd;
    rtp->port = server->port;
    rtp->attr = server->attr;
    rtp->soh.sh_tstamp = -1;
    rtp->soh.sh_nslot  = RTP_DEFNSLOT;
    rtp->soh.sh_ndas   = 0;
    rtp->soh.sh_index  = -1;
    rtp->soh.sh_stat   = (struct rtp_stat *)
                         malloc(sizeof(struct rtp_stat)*rtp->soh.sh_nslot);
    if (rtp->soh.sh_stat == (struct rtp_stat *) NULL) {
        rtp_log(RTP_ERR, "%s: malloc: %s", fid, strerror(errno));
        free(rtp);
        return (RTP *) NULL;
    }

/* Accept a new connection */

    client = INVALID_SOCKET;
    while (client == INVALID_SOCKET) {
        client = accept(server->sd, (struct sockaddr *) &cli_addr, &len);
        if (client == INVALID_SOCKET && errno != EINTR) {
            rtp_log(RTP_ERR, "%s: accept: %s", fid, strerror(errno));
            free(rtp->soh.sh_stat);
            free(rtp);
            return (RTP *) NULL;
        }
    }

/* Complete the handle */

    rtp->sd = client;
    MUTEX_INIT(&rtp->rcv.mutex);
    MUTEX_INIT(&rtp->snd.mutex);
    if (!peer_info(rtp)) {
        rtp_log(RTP_DEBUG, "%s: peer_info failed", fid);
        free(rtp->soh.sh_stat);
        free(rtp);
        return (RTP *) NULL;
    }

/* Windows NT defaults non-blocking */

#ifndef WINNT
    if (fcntl(rtp->sd, F_SETFL, O_NONBLOCK) == -1) {
        rtp_log(RTP_ERR, "%s: fcntl: %s", fid, strerror(errno));
        free(rtp->soh.sh_stat);
        free(rtp);
        return (RTP *) NULL;
    }
#endif
    rtp_log(RTP_DEBUG, "%s: incoming connection from %s:%hu",
        fid, rtp->peer, rtp->port
    );

/* Do the handshake */

    if (!handshake(rtp)) {
        rtp_log(RTP_ERR, "%s: handshake failed", fid);
        free(rtp->soh.sh_stat);
        free(rtp);
        return (RTP *) NULL;
    }

/* Return handle for this connection */

    return rtp;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:47  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
