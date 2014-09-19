#pragma ident "$Id: arc_api.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive library API functions.
 File     : arc_api.h
 Purpose  : Archive API application level include file.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  15 Sep 2005	---- (pld) change COMMENT about stream numbers: max is now 9
  03 Sep 2005	---- (pld) change sample rates to float
  17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------
    Archive file specification:

    .../ArchiveName/YYYYDDD/UUUU/S/HHMMSSSSS_LLLLLLLL
                    1234123 1234 1 121212345 12345678
                    1234567 1234 1 123456789a12345678
                    123456789a123456789b123456789c123

        YYYY     = 4 digit year 
        DDD      = Day-of-year (001-366)
        UUUU     = DAS unit ID (0000-9999)
        S        = DAS stream number (1 based, 0=SOH, 1-8=Data)
        HH       = Hour of day (00-23)
        MM       = Minute of hour (00-59)
        SSSSS    = Millisecond of minute (00000-59999)
        _        = Event length field delimiter (underscore)
        LLLLLLLL = Event length as 32 bit hex milliseconds (0 to 2^32-1)

-----------------------------------------------------------------------*/

#ifndef _ARC_API_H_INCLUDED_
#define _ARC_API_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include <platform.h>
#include <recfmt.h>
#include <mstime.h>
#include <timer.h>
#include <si.h>

/* Constants ----------------------------------------------------------*/
                                          
#define ARC_PATH_LEN                 39   
#define MAX_PATH_LEN                255    /* Maximum allowable length of a file specification */
#define ARC_MAX_LOG_MESSAGE         255    /* Max length of a log message */
#define VERSION_LEN                  31
#define MAX_NAME_LEN                 63

/* Status logging options */
#define ARC_LOG_NONE                  0    /* No logging */
#define ARC_LOG_ERRORS                1    /* Errors only */
#define ARC_LOG_VERBOSE               2    /* Informational status */
#define ARC_LOG_MAXIMUM               3    /* Everything */

/* Criteria parsing stuff */
#define CRI_MAX_ELEM                  5
#define CRI_ELEM_LENGTH              32    
#define CRI_DELIMITER               ','
#define CRI_WILDCARD                '*'

/* Undefined integer types */
#ifndef VOID_INT8
#   define VOID_INT8               -127    /* For signed types on ones's complement machines */
#   define VOID_INT16            -32767    /* this is the largest negative value, on two's complement */
#   define VOID_INT32       -2147483647    /* machines, it the largest negative value plus one. */
#   define VOID_INT64 -9223372036854775807
#   define VOID_UINT8              0xFF    /* For unsigned types this is simply the largest possible */
#   define VOID_UINT16           0xFFFF    /* value for each type. */
#   define VOID_UINT32       0xFFFFFFFF
#   define VOID_UINT64 0xFFFFFFFFFFFFFFFF
#endif

/* Various undefined values */
#define VOID_TIME           -2145916800    /* Undefined or unspecified time value (1902*001+00:00:00) */
#define VOID_H_ARCHIVE      VOID_UINT32    /* Undefined archive handle value */
#define VOID_H_STREAM       VOID_UINT32    /* Undefined stream handle value */

/* Wildcards */
#define ALL_UNITS           VOID_UINT16
#define ALL_STREAMS          VOID_UINT8
#define ALL_CHANNELS        VOID_UINT16
#define ALL_TIMES             VOID_TIME

/* Errors */
#define ARC_NO_ERROR                  0    /* No error */
#define ARC_NOT_INITIALIZED           1    /* Archive library is not yet initialized */
#define ARC_INVALID_HANDLE            2    /* Handle is invalid */
#define ARC_FILE_IO_ERROR             3    /* File error, see FileLastError( ) */
#define ARC_NO_HANDLE                 4    /* No more handles, too many archives are open */
#define ARC_NO_EXIST                  5    /* Attempted to open nonexistent archive */
#define ARC_EXISTS                    6    /* Attempted create an existing archive */
#define ARC_BAD_PATH                  7    /* Path to archive does not exist */
#define ARC_NO_MEMORY                 8    /* Not enough RAM */
#define ARC_NO_DISK_SPACE             9    /* Not enough disk space */
#define ARC_NOT_OPENED               10    /* Operation attempted on unopened archive */
#define ARC_BAD_PACKET               11    /* Packet is not a valid PASSCAL packet */
#define ARC_INTERNAL_ERROR           12    /* An serious internal error occurred probably linklist error */
#define ARC_PERMISSION_DENIED        13    /* Tried to write to an archive opened read-only */
#define ARC_NOT_FOUND                14    /* Search failed */
#define ARC_BAD_CRITERIA             15    /* Invalid search criteria specified */
#define ARC_BAD_STREAM_HANDLE        16    /* Stream handle is invalid */
#define ARC_NO_STREAM_HANDLE         17    /* Too many streams open, no more handles */
#define ARC_SEQUENCE_BREAK           18    /* Event is discontinuous */
#define ARC_END_OF_DATA              19    /* End of data encountered */
#define ARC_NO_RATE                  20
#define ARC_PURGE_THREAD             21
#define ARC_N_ERRORS                 22    /* Number of defined errors */

/* Stream cooking options for ArchiveOpenStream */
#define OPT_INCLUDE_SOH      0x00000001    /* Include state-of-health */
#define OPT_NO_DISCONTINUOUS 0x00000002    /* Abort on discontinuous data */

/* Types --------------------------------------------------------------*/

#ifdef WINNT
    typedef UINT32 H_ARCHIVE;                  /* Handle types */
    typedef UINT32 H_STREAM;
    typedef INT32  H_FIND;
#else
#   include <sys/types.h>
#   include <dirent.h>
    typedef UINT32 H_ARCHIVE;                  /* Handle types */
    typedef UINT32 H_STREAM;
    typedef DIR *  H_FIND;
#endif

typedef struct _TIME_RANGE {
    REAL64 earliest;                       /* Earliest time */
    REAL64 latest;                         /* Latest time */
} TIME_RANGE;

/* Data segment (event) */
typedef struct _EVENT {
    UINT16 unit;                           /* Unit number */
    UINT8 stream;                          /* Stream number */
    UINT16 chn_mask;                       /* Channels present mask, bit 0 = channel 1... */

    UINT16 number;                         /* Event number of current event */
    UINT16 sequence;                       /* Sequence number in current event */
    TIME_RANGE time;                       /* Data time range */
    UINT64 bytes;                          /* Size in bytes */

    H_FILE file;                           /* File handle == VOID_H_FILE if not open or error */
    CHAR filespec[MAX_PATH_LEN + 1];       /* Fully qualified event file specification */
    TIMER48 last_io;                       /* Time since last I/O operation */

    H_FIND find;                           /* Find handle */
    UINT32 attribute;                      /* Find attribute: FIND_FILE or FIND_SUBDIR */
    REAL64 yd;                             /* Year and day-of-year of earliest data time */
} EVENT;

typedef struct _STREAM {
    UINT16 unit;                           /* DAS unit ID number, 4 digits */
    UINT8 stream;                          /* Stream number, 0=SOH, 1-9=Data */
    UINT16 chn_mask;                       /* Channels present mask, bit 0 = channel 1... */
    TIME_RANGE time;                       /* Time range of data for this stream in archive */
    UINT64 bytes;                          /* Storage occupied by stream */
    REAL32 rate;                           /* Last known sampling rate, sps */
    EVENT event;                           /* Current segment */
} STREAM;

typedef struct _ARC_INFO {
    CHAR running_version[VERSION_LEN + 1]; /* Version of software currently running */
    CHAR copyright[128];                   /* Copyright notice */
    CHAR created_version[VERSION_LEN + 1]; /* Version of software at creation */
    CHAR name[MAX_NAME_LEN + 1];           /* Name of archive */
    REAL64 created;                        /* Time of creation */
    REAL64 last_update;                    /* Time of last update */
    BOOL open_for_write;                   /* Open for write flag */
    TIME_RANGE time;                       /* Gross time range of archive */
    UINT32 n_streams;                      /* Number of streams in archive */
    UINT64 max_bytes;                      /* Max allowed size of archive */
    UINT64 thres_bytes;                    /* Size at which purge commences */
    UINT64 bytes;                          /* Current size of archive */
} ARC_INFO;

/* Library API prototypes ---------------------------------------------*/

/* --- Open, close, and creation functions --- */

/* Create an archive (must not exist) */
BOOL ArchiveCreate( CHAR * path, CHAR * name, UINT64 threshold, UINT64 maximum );
/* Open and close archives (must exist) */
H_ARCHIVE ArchiveOpenForWrite( CHAR * path );
H_ARCHIVE ArchiveOpenForRead( CHAR * path );
BOOL ArchiveClose( H_ARCHIVE harchive );
VOID ArchiveCloseAll( VOID );

/* --- Packet based write functions --- */

/* Write a single packet into an archive */
BOOL ArchiveWritePacket( H_ARCHIVE harchive, RF_PACKET * packet );
BOOL ArchiveSync( H_ARCHIVE harchive );

/* --- Packet based stream reading functions --- */

/* Find and read packets from a single stream in an archive.
   Criteria must specify a unit and stream number, data is cooked */
H_STREAM ArchiveOpenStream( H_ARCHIVE harchive, STREAM * criteria, UINT32 options );
BOOL ArchiveCloseStream( H_STREAM hstream );
BOOL ArchiveReadPacket( H_STREAM hstream, RF_PACKET * packet );
BOOL ArchiveReadError( H_STREAM hstream );
BOOL ArchiveEndOfData( H_STREAM hstream );
BOOL ArchiveStreamTimeRange( H_STREAM hstream, TIME_RANGE * range );

/* --- Stream enumeration functions --- */

/* List of streams available in archive */
STREAM *ArchiveFirstStream( H_ARCHIVE harchive, VOID ** node );
STREAM *ArchiveLastStream( H_ARCHIVE harchive, VOID ** node );
STREAM *ArchiveNextStream( VOID ** node );
STREAM *ArchivePrevStream( VOID ** node );

/* --- Event functions --- */

/* Find raw event files */
BOOL ArchiveFindFirstEvent( H_ARCHIVE harchive, STREAM * criteria, EVENT * event );
BOOL ArchiveFindNextEvent( H_ARCHIVE harchive, STREAM * criteria, EVENT * event );
/* Remove a single event from archive */
BOOL ArchiveRemoveEvent( H_ARCHIVE harchive, EVENT * event );
/* Remove all events from archive that are outside of specified time range */
BOOL ArchiveTrim( H_ARCHIVE harchive, TIME_RANGE range );

/* --- Utility functions --- */

/* Init the library for use */
VOID ArchiveInitAPI( VOID );
/* Configure logging */
VOID ArchiveLogFunction( VOID( *function ) ( CHAR * string ) );
VOID ArchiveLogLevel( UINT32 level );
/* Various info about an open archive, see ARC_INFO structure above */
BOOL ArchiveInformation( H_ARCHIVE harchive, ARC_INFO * info );
VOID ArchiveLogInfo( ARC_INFO * info );
/* Error descriptions */
UINT32 ArchiveErrorNumber( H_ARCHIVE harchive );
CHAR *ArchiveErrorString( UINT32 error_number );

/* Parse user specified criteria in stream structure */
BOOL ArchiveParseCriteria( STREAM * criteria, CHAR * string );
CHAR * ArchiveFormatCriteria( CHAR * string, STREAM * criteria );

/* --------------------------------------------------------------------
  Search criteria specification:

    unit|*,stream|*,channels|*,yy:ddd:hh:mm:ss.sss|*,yy:ddd:hh:mm:ss.sss|+ss.sss|*

    A search specification consists of five comma separated fields.  These are: DAS unit
    ID, stream of DAS, channel(s) of stream, earliest time, and latest time or length
    as seconds relative to earliest time.  Any field may be empty or contain a wildcard (*). 
    An empty field is interpreted as a wildcard.  Whitespace characters are not allowed
    at any point within the specification.  Leading and trailing zeros are insignificant
    in all fields, including fields within time fields.  If the year is specified as two 
    digits within time fields, the century is assumed to be 2000 for years 0 through
    69 and 1900 for 70 through 99. If time fields are empty, day-of-year defaults to 
    today and hour, minute, and seconds default to zero.
    
    The following are examples of valid criteria specifications.  Each group are 
    equivalent specifications with the last of each group being the most compact 
    possible specification.

    7377,1,123,1998:202:12:00:00.000,1998:202:12:01:00.000
    7377,1,123,1998:202:12,1998:202:12:1
    7377,1,123,98:202:12,98:202:12:1
    7377,1,123,98:202:12:00:00.000,+60.000
    7377,1,123,98:202:12,+60

    The previous examples get channels 1, 2, and 3 for stream 1 of unit 7377 starting 
    at hour 12 on day 202, 1998 and ending at 12:01.

    *,1,*,2001:202:12:00:00.000,2001:202:12:01:00.001
    *,1,*,01:202:12,01:202:12:01.001
    *,1,*,01:202:12,+60.001
    ,1,,2001:202:12,2001:202:12:01.001
    ,1,,01:202:12,01:202:12:01.001
    ,1,,01:202:12,+60.001

    The previous examples get all stream 1 channels from all units starting at hour 12 
    on day 202 of 2001 and ending at 12:01.001.

    7377:1,*,1998:202:00:00:00.000,*
    7377:1,*,1998:202
    7377:1,,98

    The previous examples get all data starting from 0 hours today (assuming that 
    today is day 202) for all channels of stream 1 on unit 7377.

    *,*,1,*,2004:01:00:00:00.000
    *,*,1,*,04:01
    ,,1,,4:1

    The previous examples get all data up to midnight on January 1st 2004 for stream 
    1 for all units in the archive.

-----------------------------------------------------------------------*/

#endif                                     /* _ARC_API_H_INCLUDED_ */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:56  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.3  2005/09/03 21:52:29  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.2  2001/07/23 18:39:35  nobody
 * Cleanup, a few addtions for 1.9.11
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
