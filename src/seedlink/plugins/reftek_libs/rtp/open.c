#pragma ident "$Id: open.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * Establish a connection with the RTP server.  If there are connect
 * errors, then the function will either quit or retry depending on
 * the value of the retry flag in the attributes.
 *
 *====================================================================*/
#include "rtp.h"

/* Client side of the handshake */

static BOOL handshake(RTP *rtp)
{
UINT16 type;
INT16 server_version;
INT32 unused;
UBYTE msgbuf[RTP_MAXMSGLEN];
static CHAR *fid = "handshake";

/* Send over protocol version number */
 
    rtp_log(RTP_DEBUG, "%s: send version number %d", fid, RTP_VERSION);
    if (!rtp_version_send(rtp)) {
        rtp_log(RTP_DEBUG, "%s: rtp_version_send failed", fid);
        return FALSE;
    }
 
/* Read back server ack */
 
    rtp_log(RTP_DEBUG, "%s: read server ack", fid);
    if ((server_version = rtp_version_recv(rtp)) < RTP_VERSION) {
        if (errno == 0) {
            errno = EPROTO;
            rtp_log(RTP_ERR, "%s: server protocol version (%d) is too low",
                fid, server_version
            );
        } else {
            rtp_log(RTP_DEBUG, "%s: rtp_version_recv failed", fid);
        }
        return FALSE;
    }
    rtp_log(RTP_DEBUG, "%s: server replies with %d", fid, server_version);

/* Following is specific to protocol version */

    if (RTP_VERSION == 1) {

    /* send over our process ID */

        rtp_log(RTP_DEBUG, "%s: send over process ID", fid);
        if (!rtp_pid_send(rtp)) {
            rtp_log(RTP_ERR, "%s: rtp_pid_send failed", fid);
            return FALSE;
        }

    /* read back server ack */

        if (!rtp_recv(rtp, msgbuf, &type, &unused)) {
            rtp_log(RTP_ERR, "%s: rtp_recv failed", fid);
            return FALSE;
        }

        if (type != RTP_MSG_PID) {
            errno = EPROTO;
            rtp_log(RTP_ERR, "%s: expected %s, got %s",
                fid, RTP_MSG_PID, type
            );
            return FALSE;
        }

        rtp_pid_decode(msgbuf, &rtp->pid);
        rtp_log(RTP_DEBUG, "%s: server process ID is", fid, rtp->pid);

    /* send over connection attribute request */

        rtp_log(RTP_DEBUG, "%s: send over attributes", fid);
        if (!rtp_attr_send(rtp, &rtp->attr)) {
            rtp_log(RTP_ERR, "%s: rtp_attr_send failed", fid);
            return FALSE;
        }

    /* read back server ack */

        if (!rtp_recv(rtp, msgbuf, &type, &unused)) {
            rtp_log(RTP_ERR, "%s: rtp_recv failed", fid);
            return FALSE;
        }

        if (type != RTP_MSG_ATTR) {
            errno = EPROTO;
            rtp_log(RTP_ERR, "%s: expected %s, got %s",
                fid, RTP_MSG_ATTR, type
            );
            return FALSE;
        }

        rtp_attr_decode(msgbuf, &rtp->attr);
        rtp_log(RTP_DEBUG, "%s: server replies OK", fid);

    } else {
        errno = EPROTO;
        rtp_log(RTP_ERR, "%s: unsupported version number", fid);
        return FALSE;
    }

/* We are connected and ready to go */

    rtp_log(RTP_DEBUG, "%s: handshake complete", fid);
    return TRUE;
}

static RTP *failed(RTP *rtp, int sd)
{
    if (rtp != (RTP *) NULL) {
        if (rtp->peer != (CHAR *) NULL) {
            free(rtp->peer);
        }
        if (rtp->soh.sh_stat != (struct rtp_stat *) NULL) {
            free(rtp->soh.sh_stat);
        }
    }
    shutdown(sd, 2);
#ifndef WINNT
    close(sd);
#else
    closesocket(sd);
#endif
    return (RTP *) NULL;
}

#ifndef WINNT
extern int h_errno;
#endif

RTP *rtp_open(
    CHAR *host, UINT16 port, struct rtp_attr *user_attr, UINT16 maxerr
) {
static MUTEX mutex = MUTEX_INITIALIZER;
unsigned long addr;
struct hostent *hp;
struct sockaddr_in peer_addr;
struct sockaddr *name;
int namelen, sd, error_level;
struct rtp_attr attr;
int val, ilen;
RTP *rtp = (RTP *) NULL;
static char *fid = "rtp_open";

#ifdef WINNT
    if (mutex == MUTEX_INITIALIZER) {
        rtp_log(RTP_DEBUG, "%s: local mutex initialized", fid);
        MUTEX_INIT(&mutex);
    }
#endif

/* Argument check */

    if (host == (char *) NULL || port <= 0) {
        rtp_log(RTP_ERR, "%s: null input(s)", fid);
        errno = EINVAL;
        return (RTP *) NULL;
    }

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

/*  Address of server is taken from host name  */

     MUTEX_LOCK(&mutex);
        hp = gethostbyname(host);
        if (hp == (struct hostent *) NULL) {
            if (h_errno != HOST_NOT_FOUND) {
                rtp_log(RTP_ERR, "%s: gethostbyname: %s: %d", 
#ifdef WINNT
                    fid, host, WSAGetLastError()
#else
                    fid, host, h_errno
#endif /* WINNT */
                );
                return (RTP *) NULL;
            }

        /* try again assuming server name is in dot decimal form */

            if ((addr = inet_addr(host)) == -1) {
                rtp_log(RTP_ERR, "%s: inet_addr: %s", fid, strerror(errno));
                return (RTP *) NULL;
            } else {
                hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
            }
        }
    MUTEX_UNLOCK(&mutex);

    if (hp == (struct hostent *) NULL) {
        rtp_log(RTP_ERR, "%s: can't get address of host `%s'", fid, host);
        return (RTP *) NULL;
    }

/* Establish connection with server */

    memcpy(&peer_addr.sin_addr, hp->h_addr, hp->h_length);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port   = htons((u_short) port);
    name    = (struct sockaddr *) &peer_addr;
    namelen = (int) sizeof(struct sockaddr_in);
    val     = 1;
    ilen    = sizeof(int);

    sd = INVALID_SOCKET;
    while (sd == INVALID_SOCKET) {

    /* Create socket */

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            rtp_log(RTP_ERR, "%s: socket: %s", fid, strerror(errno));
            return (RTP *) NULL;
        }

    /* Set socket options.  These are really suggestions so we don't
     * care if they work or not and therefore don't test for failure.
     * Ignorance is bliss.
     */

        setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, ilen);

        if (attr.at_sndbuf > 0) {
            setsockopt(
                sd, SOL_SOCKET, SO_SNDBUF, (char *) &attr.at_sndbuf, ilen
            );
            rtp_log(RTP_DEBUG, "%s: socket sndbuf set to %d",
                fid, attr.at_sndbuf
            );
        }

        if (attr.at_rcvbuf > 0) {
            setsockopt(
                sd, SOL_SOCKET, SO_RCVBUF, (char *) &attr.at_rcvbuf, ilen
            );
            rtp_log(RTP_DEBUG, "%s: socket scvbuf set to %d",
                fid, attr.at_sndbuf
            );
        }

    /* Do the connection */
    
        if (connect(sd, name, namelen) != 0) {       
#ifndef WINNT
            rtp_log(RTP_ERR, "%s: connect: %s", fid, strerror(errno));
            close(sd);
#else
            rtp_log(RTP_ERR, "%s: connect: error %d", 
                fid, WSAGetLastError()
            );
            closesocket(sd);
#endif /* WINNT */
            sd = INVALID_SOCKET;
            switch (errno) {
              case EINTR:
              case ETIMEDOUT:
                error_level = RTP_ERR_TRANSIENT;
                break;
              case EACCES:
              case EAFNOSUPPORT:
              case EINVAL:
              case ELOOP:
              case ENOENT:
              case ENOSR:
              case ENOTDIR:
              case ENOTSOCK:
              case EPROTOTYPE:
                error_level = RTP_ERR_FATAL;
                rtp_log(RTP_DEBUG, "%s: error_level = FATAL", fid);
              default:
                error_level = RTP_ERR_NONFATAL;
                rtp_log(RTP_DEBUG, "%s: error_level = NONFATAL", fid);
                break;
            }
        }

    /* In case of failure, check retry policy and act accordingly */

        if (sd == INVALID_SOCKET) {
            if (error_level > maxerr) {
                rtp_log(RTP_DEBUG, "%s: FAIL: error_level(%d) > maxerr(%d)",
                    fid, error_level, maxerr
                );
                return (RTP *) NULL;
            }
            rtp_log(RTP_DEBUG, "%s: sleep(60)", fid);
            sleep(60);
        }
    }

/* At this point we have a working connection... build our handle */

    rtp = (RTP *) malloc(sizeof(RTP));
    if (rtp == (RTP *) NULL) {
        rtp_log(RTP_ERR, "%s: malloc: %s", fid, strerror(errno));
        return failed(rtp, sd);
    }
    rtp->peer = (CHAR *) NULL;
    rtp->soh.sh_stat = (struct rtp_stat *) NULL;

    if ((rtp->peer = strdup(host)) == (CHAR *) NULL) {
        rtp_log(RTP_ERR, "%s: strdup: %s", fid, strerror(errno));
        return failed(rtp, sd);
    }

    rtp->sd     = sd;
    rtp->port   = port;
    rtp->attr   = attr;
    MUTEX_INIT(&rtp->rcv.mutex);
    MUTEX_INIT(&rtp->snd.mutex);
    MUTEX_INIT(&rtp->attr.at_mutex);

    rtp->soh.sh_tstamp = -1;
    rtp->soh.sh_nslot  = RTP_DEFNSLOT;
    rtp->soh.sh_ndas   = 0;
    rtp->soh.sh_index  = -1;
    rtp->soh.sh_stat   = (struct rtp_stat *)
                         malloc(sizeof(struct rtp_stat)*rtp->soh.sh_nslot);
    if (rtp->soh.sh_stat == (struct rtp_stat *) NULL) {
        rtp_log(RTP_ERR, "%s: malloc: %s", fid, strerror(errno));
        return failed(rtp, sd);
    }
    MUTEX_INIT(&rtp->soh.sh_mutex);


/* Set sd to non-blocking, so we can later use select() on it */

#ifndef WINNT /* NT sets non-blocking by default */
    if (fcntl(rtp->sd, F_SETFL, O_NONBLOCK) == -1) {
        rtp_log(RTP_ERR, "%s: fcntl: %s", fid, strerror(errno));
        return failed(rtp, sd);
    }
#endif

/* Do the handshake */

    if (!handshake(rtp)) {
        rtp_log(RTP_ERR, "%s: handshake failed", fid);
        rtp_close(rtp);
        return (RTP *) NULL;
    }

/* Successful handshake, return the handle */

    return rtp;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:48  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
