#pragma ident "$Id: platform.h 1228 2008-05-21 14:46:47Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Any.
 File     : platform.h
 Purpose  : Platform specific types, constants, macros, etc...
 Host     : CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
 Target   : Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 1228 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#ifndef platform_include_defined
#define platform_include_defined

/* Command line defines are expected to be one of SOLARIS, WINNT, DOS
 * (so far).  But, we'll allow leading underscore as some people seem
 * to like those.
 */

#ifdef _SOLARIS
#   ifndef SOLARIS
#       define SOLARIS
#   endif
#endif

#ifdef _WINNT
#   ifndef WINNT
#       define WINNT
#   endif
#endif

#ifdef NT
#   ifndef WINNT
#       define WINNT
#   endif
#endif

#ifdef WIN32
#   ifndef WINNT
#       define WINNT
#   endif
#endif

#ifdef _WIN32
#   ifndef WINNT
#       define WINNT
#   endif
#endif

#ifdef _LINUX
#   ifndef LINUX
#       define LINUX
#   endif
#endif

#ifdef _DOS
#   ifndef DOS
#       define DOS
#   endif
#endif

/* Solaris */

#ifdef SOLARIS
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOGD
#   define HAVE_SVR4_IPC
#   ifdef i386
#       define X86_UNIX32
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#endif                                 /* SOLARIS */

/* Linux */

#ifdef LINUX
#   include <stdio.h>
#   include <time.h>
#   include <ctype.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <errno.h>
#   include <signal.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   define HAVE_POSIX_THREADS
#   define HAVE_POSIX_SEMAPHORES
#   define HAVE_SYSLOGD
#   define HAVE_SVR4_IPC
#   ifdef __i386__
#       define X86_UNIX32
#   endif
#   ifdef __x86_64__
#       define X86_UNIX64
#   endif
#   ifdef sparc
#       define SPARC_UNIX32
#   endif
#endif                                 /* LINUX */

/* Windows NT - Assume 32 bit X86 platform for now... will want to
 *              ifdef these once I know what to look for.
 */

#ifdef WINNT
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
#   include <process.h>
#   include <limits.h>
#   include <assert.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <sys/stat.h>
#   include <windows.h>
#   define X86_WIN32
#endif                                 /* WINNT */

/* DOS will never die */

#ifdef DOS
#   define X86_16BIT
#endif                                 /* DOS */

/* Portable data types key off the platform type defined above */

#include "stdtypes.h"

/* Windows NT Threads, Sockets, and other issues... */

#ifdef WINNT

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
#   define timeb _timeb

#   ifdef PATH_MAX
#      define MAXPATHLEN PATH_MAX
#   endif

#endif

/* Macros for mutual exclusion portability */

#ifdef HAVE_POSIX_THREADS
#   include <pthread.h>
    typedef pthread_mutex_t MUTEX;
#   define MUTEX_LOCK(arg)   pthread_mutex_lock(arg)
#   define MUTEX_UNLOCK(arg) pthread_mutex_unlock(arg)
#   define MUTEX_INIT(arg)   pthread_mutex_init((arg), NULL)
#   define MUTEX_TRYLOCK(arg) (pthread_mutex_trylock(arg) == 0 ? TRUE : FALSE)
#   define MUTEX_DESTROY(arg)  pthread_mutex_destroy(arg);
#   define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#elif defined WINNT
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
#   define MUTEX_DESTROY(arg)
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

#elif defined WINNT

    typedef HANDLE SEMAPHORE;
#   define SEM_INIT(id, init, max) (*(id) = CreateSemaphore(NULL, (init), (max), NULL))
#   define SEM_POST(id)        ReleaseSemaphore( *(id), 1, NULL )
#   define SEM_WAIT(id)        WaitForSingleObject( *(id), INFINITE )
#   define SEM_WAIT_TIMEOUT(id, timeout) \
	       (WaitForSingleObject( *(id), (DWORD)((timeout) * 1000) ) == WAIT_TIMEOUT ? -1 : 0) 
#   define SEM_TRYWAIT(id)    (WaitForSingleObject( *(id), 0 ) == WAIT_TIMEOUT ? -1 : 0)
#	define SEM_DESTROY(id)     CloseHandle( *(id) );

#else

    typedef int SEMAPHORE;
#   define SEM_INIT(arg1, arg2, arg3)
#   define SEM_POST(arg)
#   define SEM_WAIT(arg)
#   define SEM_TRYWAIT(arg)
#   define SEM_DESTROY(arg)
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
#elif defined WINNT
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
#   define THREAD_JOIN(tp)         pthread_join(*(tp),NULL)
#   define THREAD_SELF()           pthread_self()
#   define THREAD_EXIT(sp)         pthread_exit((sp))
#   define THREAD_ERRNO            (errno)

#elif defined WINNT

    typedef HANDLE THREAD;
#   define THREAD_FUNC void
#   define THREAD_CREATE(tp,fp,ap) \
((*(tp) = (THREAD) _beginthread((fp),0,(void*)(ap))) == (THREAD) -1 ? FALSE : TRUE)
#   define THREAD_JOIN(tp)         WaitForSingleObject(*(tp), INFINITE)
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

#if defined(WINNT)
    typedef HANDLE H_FILE;
    typedef struct _stat FILE_STAT;
#   define VOID_H_FILE     INVALID_HANDLE_VALUE
#   define PATH_DELIMITER '/'
#else
    typedef FILE * H_FILE;
    typedef struct stat FILE_STAT;
#   define VOID_H_FILE ((FILE *) NULL)
#   define PATH_DELIMITER '/'
#endif /* WINNT */

/* Other types */

#if defined(WINNT)
    typedef struct _timeb TIMEB;
#else
    typedef struct timeb TIMEB;
#endif
/* Compile time discovery of host byte order (additions welcome) */

#if defined(X86_16BIT) || defined(X86_WIN32) || defined(X86_UNIX32) || defined(X86_UNIX64)
#   ifndef LTL_ENDIAN_HOST
#       define LTL_ENDIAN_HOST
#   endif
#elif defined(SPARC_UNIX32)
#   ifndef BIG_ENDIAN_HOST
#       define BIG_ENDIAN_HOST
#   endif
#endif

/* Some old code has ANSI_C ifdefs */

#if (defined(__STDC__) || defined(WINNT)) && !defined ANSI_C
#   define ANSI_C
#endif

/* Some useful constants */

#ifndef MAXPATHLEN
#   define MAXPATHLEN 255
#endif

/* Assertion macro ----------------------------------------------------*/
#if defined _DEBUG
#   include <assert.h>
#   define ASSERT(expression) assert(expression);
#else
#	define ASSERT(expression) NULL
#endif                                 /* defined DEBUG */

#endif                                 /* platform_include_defined */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.3  2001/07/23 19:22:18  nobody
 * Fixed THREAD_JOIN macro
 *
 * Revision 1.2  2001/07/23 18:39:35  nobody
 * Cleanup, a few addtions for 1.9.11
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
