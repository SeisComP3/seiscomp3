#pragma ident "$Id: peer.c,v 1.11 2007/10/30 21:55:24 dechavez Exp $"
/*======================================================================
 *
 *  Get name and address of remote peer.
 *
 *====================================================================*/
#include "util.h"

char *util_peer(int sd, char *buffer, int buflen)
{
socklen_t addrlen;
#ifndef _WIN32
int h_errno;
struct hostent result;
#endif
struct sockaddr_in cli_addr;
struct hostent *hp;
static char *error = "<util_getpeer failed>";

    if (buflen < (int) strlen(error)) return (char *) NULL;
    strlcpy(buffer, error, buflen);

    addrlen = sizeof(cli_addr);
    cli_addr.sin_family = AF_INET;
    if (getpeername(sd, (struct sockaddr *)&cli_addr, &addrlen) != 0) {
        return (char *) NULL;
    }
#ifdef SOLARIS
    hp = gethostbyaddr_r( (char *) &cli_addr.sin_addr, sizeof(struct in_addr), cli_addr.sin_family, &result, buffer, MAXPATHLEN, &h_errno);
#else
    hp = gethostbyaddr(
        (char *) &cli_addr.sin_addr,
        sizeof(struct in_addr),
        cli_addr.sin_family
    );
#endif
    if (hp != NULL) {
        strlcpy(buffer, hp->h_name, buflen);
    } else {
        strlcpy(buffer, inet_ntoa(cli_addr.sin_addr), buflen);
    }

    return buffer;
}

char *utilPeerAddr(int sd, char *buffer, int buflen)
{
socklen_t addrlen;
char *dotdecimal;
struct sockaddr_in cli_addr;

    if (buflen < INET_ADDRSTRLEN) return NULL;
    addrlen = sizeof(cli_addr);
    if (getpeername(sd, (struct sockaddr *)&cli_addr, &addrlen) != 0) return NULL;

    dotdecimal = inet_ntoa(cli_addr.sin_addr);
    if (dotdecimal == NULL) return NULL;
    strlcpy(buffer, dotdecimal, INET_ADDRSTRLEN);
#ifdef INET_NTOA_ALLOCATES_MEMORY
    free(dotdecimal);
#endif /* INET_NTOA_ALLOCATES_MEMORY */

    return buffer;
}

char *utilPeerName(int sd, char *buffer, int buflen)
{
socklen_t addrlen;
#ifndef _WIN32
int h_errno;
#endif /* !_WIN32 */
#ifdef HAVE_GETHOSTBYADDR_R
struct hostent result;
#endif /* HAVE_GETHOSTBYADDR_R */
struct sockaddr_in cli_addr;
struct hostent *hp;

    if (buflen < MAXPATHLEN) return NULL;

    addrlen = sizeof(cli_addr);
    cli_addr.sin_family = AF_INET;
    if (getpeername(sd, (struct sockaddr *)&cli_addr, &addrlen) != 0) return NULL;

#ifdef HAVE_GETHOSTBYADDR_R
    hp = gethostbyaddr_r(
        (char *) &cli_addr.sin_addr,
        sizeof(struct in_addr),
        cli_addr.sin_family,
        &result,
        buffer,
        buflen,
        &h_errno
    );
#else
    hp = gethostbyaddr(
        (char *) &cli_addr.sin_addr,
        sizeof(struct in_addr),
        cli_addr.sin_family
    );
#endif

    if (hp != NULL) {
        strlcpy(buffer, hp->h_name, buflen);
    } else {
        utilPeerAddr(sd, buffer, buflen);
    }

    return buffer;
}

int utilPeerPort(int sd)
{
socklen_t addrlen;
struct sockaddr_in cli_addr;

    addrlen = sizeof(cli_addr);
    if (getpeername(sd, (struct sockaddr *)&cli_addr, &addrlen) != 0) return -1;

    return (int) ntohs(cli_addr.sin_port);
}


/* Revision History
 *
 * $Log: peer.c,v $
 * Revision 1.11  2007/10/30 21:55:24  dechavez
 * replace string memcpy w/ strlcpy
 *
 * Revision 1.10  2007/06/28 18:53:26  dechavez
 * use socklen_t instead of int
 *
 * Revision 1.9  2007/01/07 17:40:07  dechavez
 * strlcpy() instead of strcpy()
 *
 * Revision 1.8  2005/05/25 22:58:52  dechavez
 * cleared up some Unix/Windows conflicts
 *
 * Revision 1.7  2005/05/25 22:41:46  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.6  2005/05/06 01:08:09  dechavez
 * fixed benign #endif;
 *
 * Revision 1.5  2004/06/04 22:49:46  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.4  2004/01/29 18:23:02  dechavez
 * don't free inet_ntoa() return value in utilPeerAddr().  inet_ntoa is
 * not reentrant
 *
 * Revision 1.3  2003/06/09 23:59:27  dechavez
 * added utilPeerAddr(), utilPeerName(), utilPeerPort()
 *
 * Revision 1.2  2001/10/24 23:27:44  dec
 * use cli_addr. instead of cli_addrp->
 *
 * Revision 1.1.1.1  2000/02/08 20:20:41  dec
 * import existing IDA/NRTS sources
 *
 */
