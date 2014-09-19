#pragma ident "$Id: stdtypes.h,v 1.11 2005/05/13 17:31:58 dechavez Exp $"
/*======================================================================
 *
 * Portable data types
 *
 *====================================================================*/
#ifndef stdtypes_h_included
#define stdtypes_h_included

#if defined (X86_WIN32)

#include <windows.h>

typedef long ssize_t;

 /* Characters */
typedef char CHAR;

 /* Boolean values */
 /* BOOL is defined by the Win32 API in windows.h */

 /* Signed and unsigned 8 bit integers */
#ifndef INT8
typedef signed __int8 INT8;
#endif
#ifndef UINT8
typedef unsigned __int8 UINT8;
#endif

 /* 16 bit integer values */
typedef signed __int16 INT16;
typedef unsigned __int16 UINT16;

 /* 32 bit integer values */
/*typedef signed __int32 INT32;*/
/*typedef unsigned __int32 UINT32;*/

 /* 64 bit integers */
typedef signed __int64 INT64;
typedef unsigned __int64 UINT64;

 /* 32 bit IEEE 754 Real */
typedef float REAL32;

 /* 64 bit IEEE 754 Real */
typedef double REAL64;

 /* 80 bit IEEE 754 Real */
typedef long double REAL80;

#elif defined(X86_UNIX32) || defined(SPARC_UNIX32)

/* Void type */
typedef void VOID;

/* Characters */
typedef char CHAR;

#ifdef HAVE_INTTYPES

#include <inttypes.h>

 /* Signed and unsigned 8 bit integers */
typedef int8_t INT8;
typedef uint8_t UINT8;

 /* 16 bit integer values */
typedef int16_t INT16;
typedef uint16_t UINT16;

 /* 32 bit integer values */
typedef int32_t INT32;
typedef uint32_t UINT32;

 /* 64 bit integers */
typedef int64_t INT64;
typedef uint64_t UINT64;

#else

 /* Signed and unsigned 8 bit integers */
typedef char INT8;
typedef unsigned char UINT8;

 /* 16 bit integer values */
typedef short INT16;
typedef unsigned short UINT16;

 /* 32 bit integer values */
typedef long INT32;
typedef unsigned long UINT32;

 /* 64 bit integers */
typedef long long INT64;
typedef unsigned long long UINT64;

#endif /* !HAVE_INTTYPES */

 /* 32 bit IEEE 754 Real */
typedef float REAL32;

 /* 64 bit IEEE 754 Real */
typedef double REAL64;

 /* 80 bit IEEE 754 Real */
typedef long double REAL80;

 /* Boolean values */
typedef INT8 BOOL;

#else

#include<sys/types.h>

/* Void type */
typedef void VOID;

/* Characters */
typedef char CHAR;

 /* Signed and unsigned 8 bit integers */
typedef int8_t INT8;
typedef u_int8_t UINT8;

 /* 16 bit integer values */
typedef int16_t INT16;
typedef u_int16_t UINT16;

 /* 32 bit integer values */
typedef int32_t INT32;
typedef u_int32_t UINT32;

 /* 64 bit integers */
typedef int64_t INT64;
typedef u_int64_t UINT64;

 /* 32 bit IEEE 754 Real */
typedef float REAL32;

 /* 64 bit IEEE 754 Real */
typedef double REAL64;

 /* 80 bit IEEE 754 Real */
typedef long double REAL80;

 /* Boolean values */
typedef INT8 BOOL;

#endif

#ifndef TRUE
#    define TRUE  ((BOOL) 1)
#endif

#ifndef FALSE
#    define FALSE ((BOOL) 0)
#endif

/* Version is Major.Minor.Patch */

typedef struct {
    int major;
    int minor;
    int patch;
} VERSION;

#endif /* stdtypes_h_included */

/* Revision History
 *
 * $Log: stdtypes.h,v $
 * Revision 1.11  2005/05/13 17:31:58  dechavez
 * win32 INT8 defined conditionally
 *
 * Revision 1.10  2005/04/04 20:25:14  dechavez
 * added comment to final #endif
 *
 * Revision 1.9  2004/06/24 19:07:18  dechavez
 * removed INT32 and UINT32 typedefs for WIN32 (aap)
 *
 * Revision 1.8  2004/06/04 22:49:18  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.7  2002/02/21 22:30:59  dec
 * added Win32 types and use inttypes for Unix if available
 *
 */
