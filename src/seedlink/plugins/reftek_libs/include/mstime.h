#pragma ident "$Id: mstime.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Any program needing time functions
 File     : MSTIME.H
 Purpose  : Y2K compliant time handling
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
    21May98 ---- (RLB) First effort.

-------------------------------------------------------------------- */

#ifndef _MSTIME_H
#define _MSTIME_H

#ifdef _WIN32
#	include <sys\types.h>
#	include <sys\timeb.h>
#	include <time.h>
#	include <math.h>
#	include <stdio.h>
#	include <string.h>
#endif

/* Constants ---------------------------------------------------------- */

/* 1 January 1970 */
#define BASEJDN         2440588

/* milliseconds based time units */

#ifndef MSECOND_MS
#   define MSECOND_MS   1L
#   define SECOND_MS    (MSECOND_MS * 1000L)
#   define MINUTE_MS    (SECOND_MS * 60L)
#   define HOUR_MS      (MINUTE_MS * 60L)
#   define DAY_MS       (HOUR_MS * 24L)
#endif

/* Seconds based time units */

#ifndef SECOND
#   define SECOND       1L
#   define MINUTE       (SECOND * 60L)
#   define HOUR         (MINUTE * 60L)
#   define DAY          (HOUR * 24L)
#   define YEAR         (DAY * 365L)
#endif

#ifndef _MSTIME_C
#define _MSTIME_C extern
#endif

/* Types -------------------------------------------------------------- */
typedef struct _MSTIME_COMP {
    INT32 year;
    INT32 doy;
    INT32 month;
    INT32 day;
    INT32 hour;
    INT32 minute;
    REAL64 second;
} MSTIME_COMP;

/* Prototypes --------------------------------------------------------- */

#ifdef ANSI_C
_MSTIME_C REAL64 SystemMSTime( VOID );
_MSTIME_C REAL64 EncodeMSTimeMD( INT32 year, INT32 month, INT32 day, INT32 hour, INT32 minute, REAL64 second );
_MSTIME_C VOID   DecodeMSTimeMD( REAL64 mstime, INT32 * year, INT32 * month, INT32 * day, INT32 * hour, INT32 * minute, REAL64 * second );
_MSTIME_C REAL64 EncodeMSTimeDOY( INT32 year, INT32 doy, INT32 hour, INT32 minute, REAL64 second );
_MSTIME_C VOID   DecodeMSTimeDOY( REAL64 mstime, INT32 * year, INT32 * doy, INT32 * hour, INT32 * minute, REAL64 * second );
_MSTIME_C VOID   DecomposeMSTime( REAL64 mstime, MSTIME_COMP * components );
_MSTIME_C CHAR  *FormatMSTime( CHAR * string, REAL64 mstime, INT32 format );
_MSTIME_C REAL64 ParseMSTime( CHAR * string, BOOL mndy );
_MSTIME_C INT32  DayOfYear( INT32 year, INT32 month, INT32 day );
_MSTIME_C BOOL   MonthDay( INT32 year, INT32 doy, INT32 * month, INT32 * day );
_MSTIME_C BOOL   IsLeapYear( INT32 year );
_MSTIME_C REAL64 ParseEHTime( CHAR * string);

#else
_MSTIME_C REAL64 SystemMSTime(  );
_MSTIME_C REAL64 EncodeMSTimeMD(  );
_MSTIME_C VOID   DecodeMSTimeMD(  );
_MSTIME_C REAL64 EncodeMSTimeDOY(  );
_MSTIME_C VOID   DecodeMSTimeDOY(  );
_MSTIME_C VOID   DecomposeMSTime(  );
_MSTIME_C CHAR  *FormatMSTime(  );
_MSTIME_C REAL64 ParseMSTime(  );
_MSTIME_C INT32  DayOfYear(  );
_MSTIME_C BOOL   MonthDay(  );
_MSTIME_C BOOL   IsLeapYear(  );
_MSTIME_C REAL64 ParseEHTime(  );

#endif

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2004/09/30 19:10:52  rstavely
 * Modified write.c to setup a filename if a ET packet id received & no filename exists &
 * created routine in mstime.c to convert time in header
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
