/* $Id: platform.h,v 1.35 2008/01/16 23:29:17 dechavez Exp $ */
/*====================================================================
 *
 * Platform specific types, constants, macros, etc...
 *
 *===================================================================*/
#ifndef platform_h_included
#define platform_h_included

/* Solaris */

#ifdef SOLARIS
#   include <assert.h>
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <string.h>
#   include <memory.h>
#   include <strings.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <math.h>
#   include <dirent.h>
#   include <sys/file.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <pwd.h>
#   include <termios.h>
#   include <sys/mman.h>
#   ifdef i386
#       define X86_UNIX32
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#   define HAVE_GMTIME_R
#   define HAVE_TIOCEXCL
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOG
#   define HAVE_SVR4_IPC
#   define HAVE_GETHOSTBYADDR_R
#   define HAVE_GETHOSTBYNAME_R
#   define HAVE_INTTYPES
#   define HAVE_FLOCK
#   define HAVE_MSYNC
#   define HAVE_DIRENT
#   define HAVE_STRLCPY
#   ifdef i386
#       define X86_UNIX32
#       undef HAVE_INTTYPES /* this because all our x86's are Solaris 5.5 */
        typedef long suseconds_t; /* ditto */
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#endif /* SOLARIS */

/* Mac OSX */

#ifdef DARWIN
#   include <assert.h>
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <string.h>
#   include <memory.h>
#   include <strings.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <math.h>
#   include <dirent.h>
#   include <sys/file.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <pwd.h>
#   include <termios.h>
#   include <sys/mman.h>
#   ifdef i386
#       define X86_UNIX32
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#   define HAVE_GMTIME_R
#   define HAVE_TIOCEXCL
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOG
#   define HAVE_SVR4_IPC
#   define HAVE_INTTYPES
#   define HAVE_FLOCK
#   define HAVE_MSYNC
#   define HAVE_DIRENT
#   define HAVE_STRLCPY
#endif /* DARWIN */

/* Linux */

#ifdef LINUX
#   include <assert.h>
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <dirent.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <stdio.h>
#   include <string.h>
#   include <memory.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <math.h>
#   include <endian.h>
#   include <sys/file.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <sys/mman.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <pwd.h>
#   include <termios.h>
#   define HAVE_GMTIME_R
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOG
#   define HAVE_SVR4_IPC
#   define HAVE_TERMIOS
#   define HAVE_FLOCK
#   define HAVE_MSYNC
#   define HAVE_DIRENT
// Check for Slate ARM5 cpu with unusual double precision endian setup
#if (defined(__arm__) && (__BYTE_ORDER == __LITTLE_ENDIAN) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN))
#   define ARM_SLATE
#   define X86_UNIX32     // ARM in LITTLE_ENDIAN mode behaves almost like i386
#   define SIGWAIT_THREAD_DOES_NOT_WORK
#endif
#   ifdef __i386__
#       define X86_UNIX32
#   endif
#   ifdef __x86_64__
#       define X86_UNIX64
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#ifndef min
    #define min(x, y) ((x > y) ?y :x)
#endif
#ifndef max
    #define max(x, y) ((x < y) ?y :x)
#endif
#ifndef unix
#   define unix
#endif
#endif /* LINUX */

#ifdef BSD
#   include <assert.h>
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <dirent.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <stdio.h>
#   include <string.h>
#   include <memory.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <math.h>
#   include <sys/file.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <sys/mman.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <pwd.h>
#   include <termios.h>
#   define HAVE_GMTIME_R
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOG
#   define HAVE_TERMIOS
#   define HAVE_FLOCK
#   define HAVE_MSYNC
#   define HAVE_DIRENT
#   define HAVE_STRLCPY
#   ifdef i386
#       define X86_UNIX32
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#ifndef min
#    define min(x, y) ((x > y) ?y :x)
#endif
#ifndef max
#   define max(x, y) ((x < y) ?y :x)
#endif
#ifndef unix
#   define unix
#endif
#endif /* BSD */

/* Windows */

#ifdef WIN32
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <conio.h>
#   include <fcntl.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <share.h>
#   include <io.h>
#   include <errno.h>
#   include <signal.h>
#   include <math.h>
#   include <process.h>
#   include <limits.h>
#   include <assert.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/locking.h>
#   include <direct.h>
#   ifndef _WINDOWS_
#       include <windows.h>
#   endif
#   define X86_WIN32 /* NOTE: only support 32-bit X86! */
#endif /* WIN32 */

/* Portable data types key off the platform type defined above */

#include "stdtypes.h"

/* WIN32 Threads, Sockets, and other issues... */

#ifdef WIN32

/* Window sockets deviation from Berkeley sockets stuff */

/* Redefine these standard error numbers */
#   undef  EINTR
#   define EINTR            (WSAEINTR)
#   undef  EACCES
#   define EACCES           (WSAEACCES)
#   undef  EINVAL
#   define EINVAL           (WSAEINVAL)
#   undef  ENOINT
#   define ENOINT           (WSAENOINT)
#   undef  ENOTSOCK
#   define ENOTSOCK         (WSAENOTSOCK)
/* Define these error numbers */
#   define EPROTO           (WSAEPROTOTYPE)
#   define EALREADY         (WSAEALREADY)
#   define ETIMEDOUT        (WSAETIMEDOUT)
#   define EAFNOSUPPORT     (WSAEAFNOSUPPORT)
#   define ELOOP            (WSAELOOP)
#   define ENOSR            (WSAENOBUFS)    /* No stream resources */
#   define ENOTSOCK         (WSAENOTSOCK)
#   define EPROTOTYPE       (WSAEPROTOTYPE)
#   define ECONNRESET       (WSAECONNRESET)
#   define EWOULDBLOCK      (WSAEWOULDBLOCK)
#   define ECONNABORTED     (WSAECONNABORTED)
#   define ENOTSUP          (WSAEOPNOTSUPP)
#   define EMSGSIZE         (WSAEMSGSIZE)
#   define EBADMSG          (WSAEINVAL)

#define EUSERS                   (WSAEUSERS)
#define ENOTSOCK                 (WSAENOTSOCK)
#define EDESTADDRREQ             (WSAEDESTADDRREQ)
#define EMSGSIZE                 (WSAEMSGSIZE)
#define EPROTOTYPE               (WSAEPROTOTYPE)
#define ENOPROTOOPT              (WSAENOPROTOOPT)
#define EPROTONOSUPPORT          (WSAEPROTONOSUPPORT)
#define ESOCKTNOSUPPORT          (WSAESOCKTNOSUPPORT)
#define EOPNOTSUPP               (WSAEOPNOTSUPP)
#define EPFNOSUPPORT             (WSAEPFNOSUPPORT)
#define EAFNOSUPPORT             (WSAEAFNOSUPPORT)
#define EADDRINUSE               (WSAEADDRINUSE)
#define EADDRNOTAVAIL            (WSAEADDRNOTAVAIL)
#define ENETDOWN                 (WSAENETDOWN)
#define ENETUNREACH              (WSAENETUNREACH)
#define ENETRESET                (WSAENETRESET)
#define ECONNABORTED             (WSAECONNABORTED)
#define ECONNRESET               (WSAECONNRESET)
#define ENOBUFS                  (WSAENOBUFS)
#define EISCONN                  (WSAEISCONN)
#define ENOTCONN                 (WSAENOTCONN)
#define ESHUTDOWN                (WSAESHUTDOWN)
#define ETOOMANYREFS             (WSAETOOMANYREFS)
#define ETIMEDOUT                (WSAETIMEDOUT)
#define ECONNREFUSED             (WSAECONNREFUSED)
#define EHOSTDOWN                (WSAEHOSTDOWN)
#define EHOSTUNREACH             (WSAEHOSTUNREACH)
#define EALREADY                 (WSAEALREADY)
#define EINPROGRESS              (WSAEINPROGRESS)
#define ESTALE                   (WSAESTALE)
#define EDQUOT                   (WSAEDQUOT)

struct tm *gmtime_r(const time_t *clock, struct tm *result);
#   define strtok_r( _s, _sep, _lasts )  ( *(_lasts) = strtok( (_s), (_sep) ) )
#   define rint(X) floor((X)+0.5)
#   define strtoll(x,y,z) (__int64) strtol(x,y,z)
const char *inet_ntop(int af, const void *src, char *dst, long cnt);

/* Thread sleep function, Sleep() arg is milliseconds */
#   define sleep( seconds ) (Sleep( (DWORD)((seconds) * 1000) ))
#   define YieldProcessor(  ) (Sleep( 1 ))

/* strcasecmp library function is stricmp */

#   define strcasecmp stricmp
#   define strncasecmp _strnicmp

#   define vsnprintf _vsnprintf
#   define snprintf  _snprintf

#   define getpid _getpid

#   define ftime _ftime

#   ifdef PATH_MAX
#      define MAXPATHLEN PATH_MAX
#   endif
typedef int socklen_t;
typedef int pid_t;
typedef unsigned short mode_t;
typedef long suseconds_t;
#define getdtablesize() FD_SETSIZE

#ifndef bzero
#define bzero(dst,len) memset(dst,0,len)
#endif

int kill(pid_t pid, int sig);
int gettimeofday(struct timeval *tp, void *tzp);

#ifndef MAXHOSTNAMELEN
#define   MAXHOSTNAMELEN 64
#endif

#ifndef M_I86
#define M_I86
#endif

#ifndef WINNT
#define WINNT
#endif

/* some fcntl stuff */

#define F_DUPFD    0    /* dup */
#define F_GETFD    1    /* get f_flags */
#define F_SETFD    2    /* set f_flags */
#define F_GETFL    3    /* more flags (cloexec) */
#define F_SETFL    4
#define F_GETLK    5
#define F_SETLK    6
#define F_SETLKW   7

#define F_RDLCK    0
#define F_WRLCK    1
#define F_UNLCK    2

/* some SIGXX defs */
#define SIGHUP     1
/* 2 is used for SIGINT on windows */
#define SIGQUIT    3
/* 4 is used for SIGILL on windows */
#define SIGTRAP    5
#define SIGIOT     6
#define SIGBUS     7
/* 8 is used for SIGFPE on windows */
#define SIGKILL    9
#define SIGUSR1    10
/* 11 is used for SIGSEGV on windows */
#define SIGUSR2    12
#define SIGPIPE    13
#define SIGALRM    14
/* 15 is used for SIGTERM on windows */
#define SIGSTKFLT  16
#define SIGCHLD    17 
#define SIGCONT    18
#define SIGSTOP    19
#define SIGTSTP    20
/* 21 is used for SIGBREAK on windows */
/* 22 is used for SIGABRT on windows */
#define SIGTTIN    23
#define SIGTTOU    24
#define SIGURG     25
#define SIGXCPU    26
#define SIGXFSZ    27
#define SIGVTALRM  28
#define SIGPROF    29
#define SIGWINCH   30
#define SIGIO      31
#ifndef S_ISCHR
#  define S_ISCHR(m) ((m & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISREG
#  define S_ISREG(m) ((m & S_IFMT) == S_IFREG)
#endif
 
#define ftruncate(fd, length) (_chsize( (fd), (length) ))

#endif

#ifdef HAVE_MSYNC
#   define MSYNC(addr, len) msync(addr, len, MS_SYNC)
#elif defined WIN32
#   define MSYNC(addr, len) FlushViewOfFile(addr, len)
#else
#   define MSYNC(addr, len)
#endif


#ifndef HAVE_STRLCPY
#ifdef __cplusplus
extern "C" {
#endif
char *strlcpy (char *dst, const char *src, size_t len);
#ifdef __cplusplus
}
#endif
#endif /* ! HAVE_STRLCPY */


/* Macros for mutual exclusion portability */

#ifdef HAVE_POSIX_THREADS
#   include <pthread.h>
    typedef pthread_mutex_t MUTEX;
#   define MUTEX_LOCK(arg)   pthread_mutex_lock(arg)
#   define MUTEX_UNLOCK(arg) pthread_mutex_unlock(arg)
#   define MUTEX_INIT(arg)   pthread_mutex_init((arg), NULL)
#   define MUTEX_TRYLOCK(arg) (pthread_mutex_trylock(arg) == 0 ? TRUE : FALSE)
#   define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#   define MUTEX_DESTROY(arg)  pthread_mutex_destroy(arg)
#elif defined WIN32
    typedef HANDLE MUTEX;
#   define MUTEX_LOCK(arg)    WaitForSingleObject( *(arg), INFINITE )
#   define MUTEX_TRYLOCK(arg) (WaitForSingleObject( *(arg), 0 ) == WAIT_TIMEOUT ? FALSE : TRUE)
#   define MUTEX_UNLOCK(arg) ReleaseMutex( *(arg) )
#   define MUTEX_INIT(arg)   (*(arg) = CreateMutex( NULL, FALSE, NULL ))
#   define MUTEX_DESTROY(arg) CloseHandle( *(arg) )
#   define MUTEX_INITIALIZER NULL
#else
    typedef int MUTEX;
#   define MUTEX_LOCK(arg)
#   define MUTEX_UNLOCK(arg)
#   define MUTEX_INIT(arg)
#   define MUTEX_INITIALIZER 0
#endif                                 /* HAVE_POSIX_THREADS */

/* Macros for semaphore portability */

#ifdef HAVE_POSIX_SEMAPHORES

#   include <semaphore.h>
    typedef sem_t SEMAPHORE;
#   define SEM_INIT(id, init, max) sem_init((id), 0, (unsigned int) (init))
#   define SEM_POST(id)            sem_post(id)
#   define SEM_WAIT(id)            sem_wait(id)
#   define SEM_TRYWAIT(id)         sem_trywait(id)
#   define SEM_DESTROY(id)         sem_destroy(id)

#elif defined WIN32

    typedef HANDLE SEMAPHORE;
#   define SEM_INIT(id, init, max) (*(id) = CreateSemaphore(NULL, (init), (max), NULL))
#   define SEM_POST(id)        ReleaseSemaphore( *(id), 1, NULL )
#   define SEM_WAIT(id)        WaitForSingleObject( *(id), INFINITE )
#   define SEM_WAIT_TIMEOUT(id, timeout) \
           (WaitForSingleObject( *(id), (DWORD)((timeout) * 1000) ) == WAIT_TIMEOUT ? -1 : 0) 
#   define SEM_TRYWAIT(id)    (WaitForSingleObject( *(id), 0 ) == WAIT_TIMEOUT ? -1 : 0)
#   define SEM_DESTROY(id)     CloseHandle( *(id) )

#else

    typedef int SEMAPHORE;
#   define SEM_INIT(arg1, arg2, arg3)
#   define SEM_POST(arg)
#   define SEM_WAIT(arg)
#   define SEM_TRYWAIT(arg)
#endif                                 /* HAVE_POSIX_SEMAPHORES */

/* Macros for socket portability (sort of...) */

#if defined unix
    typedef int SOCKET;
#   define INVALID_SOCKET -1
#   ifndef INADDR_NONE
#       define INADDR_NONE    -1
#   endif
#   define SOCKET_ERROR INVALID_SOCKET
#   define RECVFROM(a,b,c,d,e,f)     \
    (INT32) recvfrom(                \
        (int)                   (a), \
        (void *)                (b), \
        (size_t)                (c), \
        (int)                   (d), \
        (struct sockaddr *)     (e), \
        (int *)                 (f)  \
    )
#   define SENDTO(a,b,c,d,e,f)       \
    (INT32) sendto(                  \
        (int)                   (a), \
        (void *)                (b), \
        (size_t)                (c), \
        (int)                   (d), \
        (struct sockaddr *)     (e), \
        (int)                   (f)  \
    )
#elif defined WIN32
#   define RECVFROM(a,b,c,d,e,f)     \
    (INT32) recvfrom(                \
        (SOCKET)                (a), \
        (char FAR *)            (b), \
        (int)                   (c), \
        (int)                   (d), \
        (struct sockaddr FAR *) (e), \
        (int FAR *)             (f)  \
    )
#   define SENDTO(a,b,c,d,e,f)       \
    (INT32) sendto(                  \
        (SOCKET)                (a), \
        (char FAR *)            (b), \
        (int)                   (c), \
        (int)                   (d), \
        (struct sockaddr FAR *) (e), \
        (int)                   (f)  \
    )
#endif /* unix */

/* Macros for multi-threaded portability */

#ifdef HAVE_POSIX_SEMAPHORES

#   include <pthread.h>
    typedef pthread_t THREAD;
    typedef void *THREAD_FUNC;
#   define THREAD_CREATE(tp,fp,ap) (pthread_create((tp),NULL,(fp),(ap)) ? FALSE : TRUE)
#   define THREAD_JOIN(tp)         pthread_join(tp, NULL)
#   define THREAD_SELF()           pthread_self()
#   define THREAD_EXIT(sp)         pthread_exit((sp))
#   define THREAD_ERRNO            (errno)
#   define THREAD_SIGSETMASK(a,b,c) pthread_sigmask(a, b, c)

#elif defined WIN32

    typedef unsigned long  THREAD;
#   define THREAD_FUNC void
#   define THREAD_CREATE(tp,fp,ap) \
((*(tp) = (THREAD) _beginthread((fp),0,(void*)(ap))) == (THREAD) -1 ? FALSE : TRUE)
#   define THREAD_JOIN(tp)         WaitForSingleObject((tp), INFINITE)
#   define THREAD_SELF()           GetCurrentThreadId()
#   define THREAD_EXIT(sp)         _endthread()
#   define THREAD_ERRNO           WSAGetLastError()

#else

    typedef int THREAD;
    typedef void THREAD_FUNC;
#   define THREAD_CREATE(tp,fp,ap)
#   define THREAD_JOIN(tp)
#   define THREAD_SELF()
#   define THREAD_EXIT(sv)
#   define THREAD_ERRNO
#endif                                 /* HAVE_POSIX_SEMAPHORES */

/* Syslog facility */

#ifdef HAVE_SYSLOGD
#   include <syslog.h>
#endif                                 /* HAVE_SYSLOGD */

/* System V message queues */

#ifdef HAVE_SVR4_IPC
#   include <sys/msg.h>
#endif                                 /* HAVE_SVR4_IPC */

/* File I/O function types */

#if defined(WIN32)
    typedef HANDLE H_FILE;
    typedef struct _stat FILE_STAT;
#   define VOID_H_FILE     INVALID_HANDLE_VALUE
#   define PATH_DELIMITER '\\'
#else
    typedef FILE * H_FILE;
    typedef struct stat FILE_STAT;
#   define VOID_H_FILE ((FILE *) NULL)
#   define PATH_DELIMITER '/'
#endif /* WIN32 */

/* Other types */

#if defined(WIN32)
    typedef struct _timeb TIMEB;
#else
    typedef struct timeb TIMEB;
#endif

/* Compile time discovery of host byte order (additions welcome) */

#ifndef LTL_ENDIAN_BYTE_ORDER
#define LTL_ENDIAN_BYTE_ORDER 0  /* VAX, 80x86   */
#endif

#ifndef BIG_ENDIAN_BYTE_ORDER
#define BIG_ENDIAN_BYTE_ORDER 1  /* Sun, MC680x0 */
#endif

#if defined(X86_16BIT) || defined(X86_WIN32) || defined(X86_UNIX32) || defined(X86_UNIX64)
#   ifndef LTL_ENDIAN_HOST
#       define LTL_ENDIAN_HOST
#   endif
#   define NATIVE_BYTE_ORDER LTL_ENDIAN_BYTE_ORDER
#elif defined(SPARC_UNIX32)
#   ifndef BIG_ENDIAN_HOST
#       define BIG_ENDIAN_HOST
#   endif
#   define NATIVE_BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
#endif

#ifndef NATIVE_BYTE_ORDER
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#       define ENDIAN_LITTLE
#       define NATIVE_BYTE_ORDER LTL_ENDIAN_BYTE_ORDER
#       define LTL_ENDIAN_HOST
#   endif
#   if __BYTE_ORDER == __BIG_ENDIAN
#       define ENDIAN_BIG
#       define NATIVE_BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
#       define BIG_ENDIAN_HOST
#   endif
#endif

#ifndef NATIVE_BYTE_ORDER
#   error "failed to determine host byte order"
#endif

/* Some useful constants */

#ifndef MAXPATHLEN
#   define MAXPATHLEN 255
#endif

#ifndef INET_ADDRSTRLEN
#   define INET_ADDRSTRLEN 16
#endif

#if !defined(UNSUPPORTED) && defined(ENOTSUP)
#define UNSUPPORTED ENOTSUP
#endif

#if !defined(UNSUPPORTED) && defined(ENOSYS)
#define UNSUPPORTED ENOSYS
#endif

#if !defined(UNSUPPORTED)
#define UNSUPPORTED -1
#endif

/* Assertion macro */

#ifndef ASSERT
#if defined DEBUG
#   include <assert.h>
#   define ASSERT(expression) assert(expression);
#else
#   define ASSERT(expression) NULL
#endif                                 /* defined DEBUG */
#endif /* !ASSERT */

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_SYNC
#define O_SYNC 0
#endif

/* MKDIR macro    */
#ifdef WIN32
#define MKDIR(name, perm) mkdir(name)
#else
#define MKDIR(name, perm) mkdir(name, perm)
#endif

#ifdef WIN32
#define STRCPY(dest, src) lstrcpy(dest, src)
#else
#define STRCPY(dest, src) strcpy(dest, src)
#endif

 /* Crude attempt to handle the different errno's on various systems */

#ifndef ENOTSUP
#ifdef EOPNOTSUPP
#define ENOTSUP EOPNOTSUPP
#else
#define ENOTSUP EINVAL
#endif
#endif /* ENOTSUP */

#ifndef EBADMSG
#ifdef EMSGSIZE
#define EBADMSG EMSGSIZE
#else
#define EBADMSG EINVAL
#endif
#endif /* EBADMSG */

#ifndef EPROTO
#ifdef EPROTONOSUPPORT
#define EPROTO EPROTONOSUPPORT
#else
#define EPROTO EINVAL
#endif
#endif /* EPROTO */

#endif                                 /* platform_include_defined */

/* Revision History
 *
 * $Log: platform.h,v $
 * Revision 1.35  2008/01/16 23:29:17  dechavez
 * added MUTEX_DESTROY for HAVE_POSIX_THREADS builds
 *
 * Revision 1.34  2008/01/14 16:52:26  dechavez
 * defined SIGWAIT_THREAD_DOES_NOT_WORK for ARM_SLATE systems
 *
 * Revision 1.33  2008/01/07 22:30:34  dechavez
 * 12/24/2007 akimov additions
 *
 * Revision 1.32  2007/10/17 18:31:29  dechavez
 * added test for Slate ARM5 cpu (fshelly)
 *
 * Revision 1.31  2007/10/05 19:28:29  dechavez
 * added conditionals to support Slate boxs (fakes i386)
 *
 * Revision 1.30  2007/06/28 20:35:47  dechavez
 * added DARWIN support
 *
 * Revision 1.29  2007/06/28 19:39:23  dechavez
 * added HAVE_STRLCPY support
 *
 * Revision 1.28  2007/02/20 02:17:20  dechavez
 * aap (2007-02-19)
 *
 * Revision 1.27  2007/01/04 23:31:21  dechavez
 * Changes to accomodate OpenBSD builds
 *
 * Revision 1.26  2006/12/12 22:36:21  dechavez
 * added HAVE_DIRENT
 *
 * Revision 1.25  2006/06/17 01:24:49  dechavez
 * added WIN32 strtoll macro
 *
 * Revision 1.24  2006/06/02 20:24:03  dechavez
 * added MSYNC macro
 *
 * Revision 1.23  2005/10/11 17:04:58  dechavez
 * 10-11-2005-aap
 *
 * Revision 1.22  2005/10/10 23:46:29  dechavez
 * made byte order macros cross-consistent
 *
 * Revision 1.21  2005/10/01 00:25:02  dechavez
 * NATIVE_BYTE_ORDER macro and some 09-30-2005 aap win32 updates
 *
 * Revision 1.20  2005/09/13 00:30:56  dechavez
 * added suseconds_t for x86 Solaris, include <pwd.h> for linux
 *
 * Revision 1.19  2005/05/25 23:55:49  dechavez
 * removed uneccesary (extra) MKDIR macro
 *
 * Revision 1.18  2005/05/25 23:54:11  dechavez
 * Changes to calm Visual C++ warnings
 *
 * Revision 1.17  2005/05/25 18:26:10  dechavez
 * use HAVE_FLOCK
 *
 * Revision 1.16  2005/05/23 20:58:59  dechavez
 * added gmtime_r macro for win32 (not MT safe however) (05-23 update AAP)
 *
 * Revision 1.15  2005/03/23 21:16:01  dechavez
 * updated includes, added Unix error codes to windows, fixed THREAD_JOIN (aap)
 *
 * Revision 1.14  2005/02/10 18:48:47  dechavez
 * added bzero() macro for win32 and MKDIR and STRCPY macros for both, fixed PATH_DELIMITER
 *
 * Revision 1.13  2004/07/26 22:36:55  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.12  2004/06/24 18:58:16  dechavez
 * added dirent.h to SOLARIS
 *
 * Revision 1.11  2004/06/24 17:01:10  dechavez
 * updated WIN32 support (aap)
 *
 * Revision 1.10  2004/06/04 22:49:18  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.9  2003/10/16 17:44:46  dechavez
 * added HAVE_INTTYPES for Solaris x86, various other useful constants
 *
 * Revision 1.8  2003/06/11 20:56:25  dechavez
 * add SEM_DESTROY and THREAD_SIGSETMASK macros
 *
 * Revision 1.7  2003/06/09 23:40:39  dechavez
 * replaced with ESSW equivalent
 *
 */
