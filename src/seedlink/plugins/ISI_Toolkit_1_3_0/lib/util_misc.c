#pragma ident "$Id: misc.c,v 1.16 2007/09/14 19:24:54 dechavez Exp $"
/*======================================================================
 *
 *  Misc helper functions for cross platform convenience
 *
 *====================================================================*/
#include "util.h"

/* Windows version */

#ifdef WIN32

BOOL utilNetworkInit(VOID)
{
WSADATA lpWSAData;

    return WSAStartup(MAKEWORD(2,2), &lpWSAData) == 0 ? TRUE : FALSE;
}

VOID utilSetNonBlockingSocket(int sd)
{
unsigned long wl = 1; 

    ioctlsocket(sd, FIONBIO, &wl);

}

SOCKET utilCloseSocket(int sd)
{
    if (sd != INVALID_SOCKET) closesocket(sd);
    return INVALID_SOCKET;
}

BOOL utilCheckStat(char *path, mode_t mode)
{
BOOL status;
struct _stat buf;

    if (path == (char *) NULL) return FALSE;

    if (_stat(path, &buf) != 0) {
        status = FALSE;
    } else if (buf.st_mode & mode) {
        status = TRUE;
    } else {
        status = FALSE;
    }

    return status;
}

static BOOL Win32SetHostAddr(struct sockaddr_in *in_addr, char *host, int port)
{
BOOL status;
unsigned long addr;
struct hostent *hp;

    memset((void *) in_addr, 0, sizeof(struct sockaddr_in));
    in_addr->sin_family = AF_INET;
    in_addr->sin_port = htons((UINT16) port);

    if (host == NULL) {
        in_addr->sin_addr.s_addr = htonl(INADDR_ANY);
        return TRUE;
    }

/* Begin by assuming host name is given in text form */

    hp = gethostbyname(host);
    
/* If that fails, try again assuming host name is in dot decimal form */

    if (hp == NULL && (addr = inet_addr(host)) != -1) {
        hp = gethostbyaddr((char *) &addr, sizeof(addr),AF_INET);
    }

    if (hp == (struct hostent *) NULL) {
        errno = ENOENT;
        status = FALSE;
    } else {
        memcpy(&in_addr->sin_addr, hp->h_addr, hp->h_length);
        status = TRUE;
    }

    return status;
}

#else

/* Unix version */

VOID utilSetNonBlockingSocket(int sd)
{
    fcntl(sd, F_SETFL, O_NONBLOCK);
    return;
}

SOCKET utilCloseSocket(int sd)
{
    if (sd != INVALID_SOCKET) close(sd);
    return INVALID_SOCKET;
}

BOOL utilCheckStat(char *path, mode_t mode)
{
BOOL status;
struct stat buf;

    if (path == (char *) NULL) return FALSE;

    if (stat(path, &buf) != 0) {
        status = FALSE;
    } else if (buf.st_mode & mode) {
        status = TRUE;
    } else {
        status = FALSE;
    }

    return status;
}

BOOL utilDeleteFile(char *path)
{

    if (path == (char *) NULL) return FALSE;

    return unlink(path) == 0 ? TRUE : FALSE;
}

#endif /* unix */

/* Get socket address via specification by either name or dot decimal address */

#ifdef SOLARIS

static BOOL SolarisSetHostAddr(struct sockaddr_in *in_addr, char *host, int port)
{
BOOL status;
unsigned long addr;
struct hostent *hp, result;
int h_errno;
char buffer[MAXPATHLEN];

    memset((void *) in_addr, 0, sizeof(struct sockaddr_in));
    in_addr->sin_family = AF_INET;
    in_addr->sin_port = htons((UINT16) port);

    if (host == NULL) {
        in_addr->sin_addr.s_addr = htonl(INADDR_ANY);
        return TRUE;
    }

/* Begin by assuming host name is given in text form */

    hp = gethostbyname_r(host, &result, buffer, MAXPATHLEN, &h_errno);
    
/* If that fails, try again assuming host name is in dot decimal form */

    if (hp == NULL && (addr = inet_addr(host)) != -1) {
        hp = gethostbyaddr_r(
            (char *) &addr, sizeof(addr),
            AF_INET,
            &result,
            buffer,
            MAXPATHLEN,
            &h_errno
        );
    }

    if (hp == (struct hostent *) NULL) {
        errno = ENOENT;
        status = FALSE;
    } else {
        memcpy(&in_addr->sin_addr, hp->h_addr, hp->h_length);
        status = TRUE;
    }

    return status;
}

#elif defined LINUX

static BOOL LinuxSetHostAddr(struct sockaddr_in *in_addr, char *host, int port)
{
BOOL status;
unsigned long addr;
struct hostent hpp;
struct hostent *hp=&hpp, *result;
int h_errno;
char buffer[MAXPATHLEN];

    memset((void *) in_addr, 0, sizeof(struct sockaddr_in));
    in_addr->sin_family = AF_INET;
    in_addr->sin_port = htons((UINT16) port);

    if (host == NULL) {
        in_addr->sin_addr.s_addr = htonl(INADDR_ANY);
        return TRUE;
    }

/* Begin by assuming host name is given in text form */

    gethostbyname_r(host, hp, buffer, MAXPATHLEN, &result, &h_errno);
    
/* If that fails, try again assuming host name is in dot decimal form */

    if (hp == NULL && (addr = inet_addr(host)) != -1) {
        gethostbyaddr_r(
            (char *) &addr,
            sizeof(addr),
            AF_INET,
            hp,
            buffer,
            MAXPATHLEN,
            &result,
            &h_errno
        );
    }

    if (hp == (struct hostent *) NULL) {
        errno = ENOENT;
        status = FALSE;
    } else {
        memcpy(&in_addr->sin_addr, hp->h_addr, hp->h_length);
        status = TRUE;
    }

    return status;
}

#else

static BOOL UnixSetHostAddr(struct sockaddr_in *in_addr, char *host, int port)
{
BOOL status;
unsigned long addr;
struct hostent *hp;

    memset((void *) in_addr, 0, sizeof(struct sockaddr_in));
    in_addr->sin_family = AF_INET;
    in_addr->sin_port = htons((UINT16) port);

    if (host == NULL) {
        in_addr->sin_addr.s_addr = htonl(INADDR_ANY);
        return TRUE;
    }

/* Begin by assuming host name is given in text form */

    hp = gethostbyname(host);
    
/* If that fails, try again assuming host name is in dot decimal form */

    if (hp == NULL && (addr = inet_addr(host)) != -1) {
        hp = gethostbyaddr((char *) &addr, sizeof(addr),AF_INET);
    }

    if (hp == (struct hostent *) NULL) {
        errno = ENOENT;
        status = FALSE;
    } else {
        memcpy(&in_addr->sin_addr, hp->h_addr, hp->h_length);
        status = TRUE;
    }

    return status;
}

#endif /* unix */

BOOL utilSetHostAddr(struct sockaddr_in *in_addr, char *host, int port)
{
#ifdef SOLARIS
    return SolarisSetHostAddr(in_addr, host, port);
#elif defined LINUX
    return LinuxSetHostAddr(in_addr, host, port);
#elif defined WIN32
    return Win32SetHostAddr(in_addr, host, port);
#else
    return UnixSetHostAddr(in_addr, host, port);
#endif
}

/* Revision History
 *
 * $Log: misc.c,v $
 * Revision 1.16  2007/09/14 19:24:54  dechavez
 * added utilDeleteFile() to unix build
 *
 * Revision 1.15  2007/06/28 19:41:23  dechavez
 * removed extra *hp from linux builds
 *
 * Revision 1.14  2007/01/04 18:00:45  dechavez
 * separated things into cleaner OS specific sections
 *
 * Revision 1.13  2006/06/14 23:58:10  dechavez
 * fixed error in Linux gethostbyaddr_r call
 *
 * Revision 1.12  2006/05/16 00:05:29  dechavez
 * allow for NULL hostname in utilSetHostAddr() (INADDR_ANY)
 *
 * Revision 1.11  2005/08/26 18:14:21  dechavez
 * added utilDirectoryExists()
 *
 * Revision 1.10  2005/06/30 01:22:19  dechavez
 * set ENOENT errno when utilSetHostAddr() unable to determine address
 *
 * Revision 1.9  2005/05/25 22:58:52  dechavez
 * cleared up some Unix/Windows conflicts
 *
 * Revision 1.8  2005/05/25 22:41:46  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.7  2005/03/23 21:30:06  dechavez
 *  Linux portability mods (aap)
 *
 * Revision 1.6  2004/06/10 17:18:38  dechavez
 * fixed utilSetNonBlockingSocket() for WIN32
 *
 * Revision 1.5  2004/06/08 18:45:56  dechavez
 * ifdef h_error out of utilSetHostAddr() for Windows builds (aap)
 *
 * Revision 1.4  2004/06/04 22:49:46  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.3  2003/11/04 00:35:50  dechavez
 * utilCloseSocket() changed to return INVALID_SOCKET
 *
 * Revision 1.2  2003/10/16 15:50:52  dechavez
 * added tests for invalid arguments
 *
 * Revision 1.1  2003/06/10 00:05:41  dechavez
 * initial release
 *
 */
