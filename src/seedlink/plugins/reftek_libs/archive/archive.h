#pragma ident "$Id: archive.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : archive.h
 Purpose  : Constants, types, globals, prototypes for archive library.
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
  15 Sep 2005	---- (pld) bug fix in stream template for change in rate
  03 Sep 2005	---- (pld) change sample rates to float
  17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#ifndef _ARCHIVE_H_INCLUDED_
#define _ARCHIVE_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include <stddef.h>

#include <arc_api.h>
#include "linklist.h"
#include "version.h"
#include "util.h"

/* Constants ----------------------------------------------------------*/
#define MAX_OPEN_ARCHIVES     16            /* Max number of open archives */
#define MAX_OPEN_STREAMS      64            /* Max number of open streams */

#define STATE_FILENAME      "archive.sta"

#define FILE_IDLE_TIMEOUT  60000            /* Event files are idle after this much time in ms */
#define FILE_OPEN_TIMEOUT   5000            /* Retry open for this long */
#define FILE_OPEN_INTERVAL   100            /* Retry open at this interval */
#define FILE_RENAME_TIMEOUT 5000            /* Retry rename for this long */
#define FILE_RENAME_INTERVAL 100            /* Retry rename at this interval */
#define UPDATE_INTERVAL     5000            /* Update state file on write interval in ms */

#define FIND_FILE      _A_NORMAL            /* FileFind function attributes */
#define FIND_SUBDIR    _A_SUBDIR

#define VOID_RATE                  0.0F     /* Undefined sampling rate */
#define VOID_UNIT           VOID_UINT16
#define VOID_STREAM          VOID_UINT8
#define VOID_CHANNEL        VOID_UINT16
#define VOID_EVENT_NO       VOID_UINT16
#define VOID_SEQ_NO         VOID_UINT16

#define STA_STARTING                  0
#define STA_IN_PROGRESS               1
#define STA_FINISHED                  2

#define ARC_CLOSED                    0
#define ARC_READ                      1
#define ARC_WRITE                     2

/* Types --------------------------------------------------------------*/

/* Serialization template */
typedef struct _TEMPLATE {
    size_t offset;
    size_t length;
    BOOL swap;
} TEMPLATE;

/* Archive state */
typedef struct _STATE {
    BOOL write;                             /* Open for write access flag */
    CHAR version[VERSION_LEN + 1];          /* Version of software at creation */
    REAL64 created;                         /* Time of creation */
    REAL64 last_update;                     /* Time of last update */
    CHAR name[MAX_NAME_LEN + 1];            /* Name of archive */
    TIME_RANGE time;                        /* Gross time range of archive */
    UINT32 n_streams;                       /* Number of streams in archive */
    UINT64 max_bytes;                       /* Max size of archive */
    UINT64 thres_bytes;                     /* Size at which purge commences */
    UINT64 bytes;                           /* Current size of archive */
} STATE;

typedef struct _PURGE {
    BOOL active;
    BOOL stop;
    MUTEX mutex;
    SEMAPHORE semaphore;
    THREAD thread_id;
    H_ARCHIVE handle;
} PURGE;

typedef struct _ARCHIVE {
    STATE state;                            /* Archive state */
    LIST streams;                           /* Stream list */
    STREAM *stream;                         /* Current stream */
    CHAR path[MAX_PATH_LEN - ARC_PATH_LEN + 1]; /* Path to archive */
    BOOL access;                            /* Current access, ARC_CLOSED, ARC_READ, ARC_WRITE */
    PURGE purge;
    BOOL dirty;                             /* State is dirty */
    UINT32 last_error;                      /* Last error encountered on archive */
    CHAR filespec[MAX_PATH_LEN + 1];        /* State filespec */
    H_FILE file;                            /* State file handle */
    TIMER48 update;                         /* State file write update timer */
    FILE_STAT stat;                         /* State file stat info */
    MUTEX mutex;
} ARCHIVE;

typedef struct _READS {
    H_ARCHIVE harchive;                     /* Handle of archive */
    STREAM criteria;                        /* Stream search criteria */
    UINT32 options;                         /* Search options */
    EVENT event;                            /* Event file information */
    UINT16 state;                           /* Event state */
    TIME_RANGE time;                        /* Event start and end times */
    REAL32 rate;                            /* Sampling rate */
    UINT16 out_evn;                         /* Outbound sequence number */
    UINT16 out_seq;                         /* Current outbound sequence number */
    UINT16 channels;                        /* Active channels */
    UINT16 in_seq;                          /* Sequence number of last packet */
    H_FILE file;                            /* Event file handle */
    RF_PACKET eh;                           /* EH packet for curent event */
} READS;

/* Globals ------------------------------------------------------------*/
#ifdef _DECLARE_GLOBALS
TEMPLATE _state_template[] = {
    {offsetof(STATE, write), sizeof(BOOL), TRUE},
    {offsetof(STATE, version), VERSION_LEN + 1, FALSE},
    {offsetof(STATE, created), sizeof(REAL64), TRUE},
    {offsetof(STATE, last_update), sizeof(REAL64), TRUE},
    {offsetof(STATE, name), MAX_NAME_LEN + 1, FALSE},
    {offsetof(STATE, time.earliest), sizeof(REAL64), TRUE},
    {offsetof(STATE, time.latest), sizeof(REAL64), TRUE},
    {offsetof(STATE, n_streams), sizeof(UINT32), TRUE},
    {offsetof(STATE, max_bytes), sizeof(UINT64), TRUE},
    {offsetof(STATE, thres_bytes), sizeof(UINT64), TRUE},
    {offsetof(STATE, bytes), sizeof(UINT64), TRUE},
    {VOID_UINT32, VOID_UINT32, FALSE}
};
TEMPLATE _stream_template[] = {
    {offsetof(STREAM, unit), sizeof(UINT16), TRUE},
    {offsetof(STREAM, stream), sizeof(UINT8), TRUE},
    {offsetof(STREAM, chn_mask), sizeof(UINT16), TRUE},
    {offsetof(STREAM, time.earliest), sizeof(REAL64), TRUE},
    {offsetof(STREAM, time.latest), sizeof(REAL64), TRUE},
    {offsetof(STREAM, bytes), sizeof(UINT64), TRUE},
    {offsetof(STREAM, rate), sizeof(REAL32), TRUE},
    {VOID_UINT32, VOID_UINT32, FALSE}
};
H_ARCHIVE _n_archives = 0;
ARCHIVE _archive[MAX_OPEN_ARCHIVES];
H_STREAM _n_reads = 0;
READS _reads[MAX_OPEN_STREAMS];
UINT32 _archive_error = ARC_NO_ERROR;

#   undef _DECLARE_GLOBALS
#else
extern TEMPLATE _state_template[];
extern TEMPLATE _stream_template[];
extern H_ARCHIVE _n_archives;
extern ARCHIVE _archive[];
extern H_STREAM _n_reads;
extern READS _reads[];
extern UINT32 _archive_error;

#endif

/* Prototypes ---------------------------------------------------------*/

/* archive.c */
VOID ArchiveLog(UINT32 level, CHAR * format,...);
BOOL ValidateHandle(H_ARCHIVE handle);
H_ARCHIVE GetNextHandle(VOID);
BOOL CreateEventDirectory(H_ARCHIVE handle, STREAM * stream);
BOOL UndefinedTime(REAL64 time);
INT32 CompareYD(REAL64 time1, REAL64 time2);
VOID InitArchive(H_ARCHIVE handle);
VOID UpdateTimeRange(TIME_RANGE * range, REAL64 time);

/* write.c */
BOOL WritePacket(H_ARCHIVE handle, STREAM * stream, RF_HEADER * hdr, RF_PACKET * packet);

/* purge.c */
VOID InitPurge(PURGE *purge);
BOOL PurgeOldestEvent(H_ARCHIVE harchive);
VOID DestroyPurge(PURGE *purge);
THREAD_FUNC PurgeThread(VOID *argument);

/* rate.c */
BOOL DeriveRate(H_ARCHIVE handle, STREAM * stream, RF_HEADER * hdr, RF_PACKET * rfp);
REAL32 DecodeRFRate(RF_PACKET * packet);
BOOL IsValidRate(REAL32 rate);
VOID DestroyStash(VOID);

/* evnfiles.c */
BOOL OpenEventFileForWrite(H_ARCHIVE handle, STREAM * stream);
BOOL CloseEventFile(STREAM * stream);
BOOL CloseEventFileAndRename(H_ARCHIVE handle, STREAM * stream);
CHAR *MakeEventFileSpec(STREAM * stream);
BOOL EventFileTimeRange(CHAR * eventfilespec, TIME_RANGE * range);
BOOL CreateEventDirectory(H_ARCHIVE handle, STREAM * stream);

/* streams.c */
BOOL UpdateStreamTimeInfo(H_ARCHIVE handle, STREAM * stream, TIME_RANGE * range);
STREAM *LookupStream(H_ARCHIVE harchive, UINT16 unit, UINT8 stream);
STREAM *CreateStream(H_ARCHIVE harchive, UINT16 unit, UINT8 stream);
VOID InitStream(STREAM * stream);

/* serialize.c */
UINT32 SerializedWrite(VOID * str, TEMPLATE * temp, H_FILE file);
UINT32 SerializedRead(VOID * str, TEMPLATE * temp, H_FILE file);

/* state.c */
VOID InitState(STATE * state);
BOOL CreateState(CHAR * filespec, STATE * state);
BOOL OpenStateForWrite(H_ARCHIVE handle);
BOOL OpenStateForRead(H_ARCHIVE handle);
BOOL SynchronizeState(H_ARCHIVE handle);
BOOL WriteState(H_ARCHIVE handle);
BOOL ReadState(H_ARCHIVE handle);

/* read.c */
BOOL ReadPacket(H_FILE file, RF_HEADER * hdr, RF_PACKET * packet);
BOOL UnReadPacket(H_FILE file);
BOOL ValidateStreamHandle(H_STREAM hstream);
VOID InitRead(H_STREAM hstream);

/* find.c */
VOID InitEvent(EVENT * event);
BOOL GetEventFileTimeRange(EVENT * event, TIME_RANGE * range);

/* Platform specific functions defined in win32.c, solaris.c, or linux.c */
H_FILE FileOpenForWrite(CHAR * filespec);
H_FILE FileOpenForRead(CHAR * filespec);
BOOL FileClose(H_FILE handle);
BOOL FileWrite(H_FILE handle, VOID * data, UINT32 * nbytes);
BOOL FileRead(H_FILE handle, VOID * data, UINT32 * nbytes);
BOOL FileFlush(H_FILE handle);
BOOL FileRewind(H_FILE handle);
BOOL FileSeekEOF(H_FILE handle);
BOOL FileSeekAbsolute(H_FILE handle, INT32 offset);
BOOL FileSeekRelative(H_FILE handle, INT32 offset);
UINT32 FilePosition(H_FILE handle);
BOOL FileError(VOID);
BOOL FileAtEOF(VOID);
UINT32 FileLastError(VOID);
CHAR *FileErrorString(UINT32 error);

VOID QualifyPath(CHAR * qualified, CHAR * pathname);
BOOL CreatePath(CHAR * pathname);
BOOL DestroyPath(CHAR * pathname);
BOOL TrimPath(CHAR * pathname, UINT16 levels);

BOOL IsRegularFile(CHAR * pathname);
BOOL IsDirectory(CHAR * pathname);
BOOL GetFileStat(CHAR * pathname, FILE_STAT * stat);

UINT64 DiskSpace(CHAR * pathname);

BOOL FileFindFirst(EVENT * event);
BOOL FileFindNext(EVENT * event);
BOOL FileFindClose(EVENT * event);

/* Assertion macro ----------------------------------------------------*/
/*
#ifdef _DEBUG
VOID _ArchiveAssert( CHAR * file, UINT32 line );
#define ASSERT( f )        \
      if( f )                 \
         NULL;                \
      else                    \
         _ArchiveAssert( __FILE__, __LINE__ )
#else
#define ASSERT( f ) NULL
 #endif *//* defined DEBUG */

#endif                                      /* _ARCHIVE_H_INCLUDED_ */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:07  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.3  2005/09/03 21:52:30  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
