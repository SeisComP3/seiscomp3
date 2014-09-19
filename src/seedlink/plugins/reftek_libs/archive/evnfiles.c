#pragma ident "$Id: evnfiles.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
Program  : Any
Task     : Archive Library API functions.
File     : evnfiles.c
Purpose  : Event file management routines.
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
22 Sep 2004  ----  (rs) Put more error logs in if failing on write
17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#define _EVNFILES_C
#include "archive.h"

/*---------------------------------------------------------------------*/
BOOL OpenEventFileForWrite(H_ARCHIVE harchive, STREAM *stream)
   {
   BOOL append;
   CHAR filespec[MAX_PATH_LEN + 1];

   ASSERT(stream != NULL);

   if (!ValidateHandle(harchive))
      {
      ArchiveLog(ARC_LOG_ERRORS, "%s:%s", __FILE__, "Can't Validate Archive");
      return (FALSE);
      }

   _archive[harchive].last_error = ARC_NO_ERROR;
   _archive_error = ARC_NO_ERROR;

   /* Create filespec for where this packet goes */
   if (MakeEventFileSpec(stream) == NULL)
      {
      ArchiveLog(ARC_LOG_ERRORS, "%s:%s event %u on %04X:%u ", __FILE__, "Can't Make Filespec",
      	(stream->event.number < RF_MAX_EVENT_NO ? stream->event.number: 0), 
      	stream->unit, stream->stream);
      return (FALSE);
      }

   /* Create directories as needed */
   if (!CreateEventDirectory(harchive, stream))
      {
      ArchiveLog(ARC_LOG_ERRORS, "%s:%s <%s> <%s>", __FILE__, "Can't Create Directory ",
       _archive[harchive].path, stream->event.filespec);
      return (FALSE);
      }

   /* Open the file */
   append = FALSE;
   sprintf(filespec, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);
   if (IsRegularFile(filespec))
      append = TRUE;
   if ((stream->event.file = FileOpenForWrite(filespec)) == VOID_H_FILE)
      {
      _archive[harchive].last_error = ARC_FILE_IO_ERROR;
      ArchiveLog(ARC_LOG_ERRORS, "%s:%s <%s>", __FILE__, "Can't Open File", filespec);
      return (FALSE);
      }

   if (append)
      {
      if (!FileSeekEOF(stream->event.file))
         {
         _archive[harchive].last_error = ARC_FILE_IO_ERROR;
         return (FALSE);
         }
      ArchiveLog(ARC_LOG_VERBOSE, "Archive: reopen event %u on %04X:%u %s", 
      	(stream->event.number < RF_MAX_EVENT_NO ? stream->event.number: 0), 
      	stream->unit, stream->stream, stream->event.filespec);
      }
   else
      {
      ArchiveLog(ARC_LOG_VERBOSE, "Archive: new event %u on %04X:%u %s", 
      	(stream->event.number < RF_MAX_EVENT_NO ? stream->event.number: 0), 
      	stream->unit, stream->stream, stream->event.filespec);
      }

   return (TRUE);
   }

/*---------------------------------------------------------------------*/
BOOL CloseEventFile(STREAM *stream)
   {

   ASSERT(stream != NULL);

   stream->event.filespec[0] = '\0';
   stream->event.time.earliest = VOID_TIME;
   stream->event.time.latest = VOID_TIME;

   if (stream->event.file == VOID_H_FILE)
      return (TRUE);

   if (!FileClose(stream->event.file))
      {
      _archive_error = ARC_FILE_IO_ERROR;
      return (FALSE);
      }

   stream->event.file = VOID_H_FILE;

   return (TRUE);
   }

/*---------------------------------------------------------------------*/
BOOL CloseEventFileAndRename(H_ARCHIVE harchive, STREAM *stream)
   {
   BOOL error;
   CHAR oldfilespec[MAX_PATH_LEN + 1], newfilespec[MAX_PATH_LEN + 1],  *ptr;
   TIMER48 timer;
   UINT32 length;

   ASSERT(stream != NULL);

   if (!ValidateHandle(harchive))
      return (FALSE);

   _archive[harchive].last_error = ARC_NO_ERROR;
   _archive_error = ARC_NO_ERROR;

   /* Close the file */

   if (stream->event.file == VOID_H_FILE)
      return (TRUE);

   if (!FileClose(stream->event.file))
      {
      _archive[harchive].last_error = ARC_FILE_IO_ERROR;
      return (FALSE);
      }

   /* If latest time is defined... */
   if (!UndefinedTime(stream->event.time.latest))
      {

      /* Record length in milliseconds */
      length = (UINT32)(((stream->event.time.latest - stream->event.time.earliest) / SI_MILLI) + 0.5);

      /* Create old filespec */
      sprintf(oldfilespec, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);

      /* Update length portion of filespec */
      if ((ptr = strrchr(stream->event.filespec, '_')) != NULL)
         {
         sprintf(ptr, "_%08lX", length);
         sprintf(newfilespec, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);

         /* Is the new name different? */
         error = FALSE;
         if (strcmp(oldfilespec, newfilespec) != 0)
            {
            /* Avoid name collision */
            if (IsRegularFile(newfilespec))
               remove(newfilespec);
            Timer48Start(&timer, FILE_RENAME_TIMEOUT);
            while (!Timer48Expired(&timer))
               {
               if (rename(oldfilespec, newfilespec) == 0)
                  {
                  error = FALSE;
                  break;
                  }
               error = TRUE;
               MSPause(FILE_RENAME_INTERVAL);
               }
            }
         if (error)
            ArchiveLog(ARC_LOG_ERRORS, "Archive: rename: %s > %s failed!", oldfilespec, newfilespec);
         else
            ArchiveLog(ARC_LOG_VERBOSE, "Archive: rename: %s > %s", oldfilespec, newfilespec);
         }
      }

   stream->event.filespec[0] = '\0';
   stream->event.time.earliest = VOID_TIME;
   stream->event.time.latest = VOID_TIME;
   stream->event.file = VOID_H_FILE;

   return (TRUE);
   }

/*---------------------------------------------------------------------*/
CHAR *MakeEventFileSpec(STREAM *stream)
   {
   INT32 year, doy, hour, minute;
   REAL64 second;
   UINT32 millisecond, length;

   ASSERT(stream != NULL);

   if (UndefinedTime(stream->event.time.earliest))
      {
      stream->event.filespec[0] = '\0';
      return (stream->event.filespec);
      }

   /* Earliest time */
   DecodeMSTimeDOY(stream->event.time.earliest, &year, &doy, &hour, &minute, &second);
   millisecond = (UINT32)((second / SI_MILLI) + 0.5);

   /* If latest time is defined... */
   /*
   if (!UndefinedTime(stream->event.time.latest))
   length = (UINT32) (((stream->event.time.latest - stream->event.time.earliest) / SI_MILLI) + 0.5);
   else
    */
   length = 0;

   /* Filespec relative to base path */
   sprintf(stream->event.filespec, "%04ld%03ld%c%04X%c%01u%c%02ld%02ld%05lu_%08lx", year, doy, PATH_DELIMITER, stream->unit, PATH_DELIMITER, stream->stream, PATH_DELIMITER, hour, minute, millisecond, length);

   return (stream->event.filespec);
   }

/*---------------------------------------------------------------------*/
BOOL EventFileTimeRange(CHAR *eventfilespec, TIME_RANGE *range)
   {
   CHAR filespec[MAX_PATH_LEN + 1],  *ptr;
   long year, doy, hour, minute, millisecond;
   unsigned long length;

   ASSERT(eventfilespec != NULL);
   ASSERT(range != NULL);

   /* Make a copy of filespec because we'll destroy it below */
   strcpy(filespec, eventfilespec);

   /* Length in milliseconds */
   if ((ptr = strrchr(filespec, '_')) == NULL)
      return (FALSE);
   sscanf(ptr, "_%8lx", &length);
   *ptr = '\0';

   /* Hour, minute, and second */
   if ((ptr = strrchr(filespec, PATH_DELIMITER)) == NULL)
      ptr = filespec;
   else
      ptr++;
   sscanf(ptr, "%2ld%2ld%5ld", &hour, &minute, &millisecond);

   /* Year and day-of-year */
   if (!TrimPath(filespec, 3))
      return (FALSE);
   if ((ptr = strrchr(filespec, PATH_DELIMITER)) == NULL)
      ptr = filespec;
   else
      ptr++;
   sscanf(ptr, "%4ld%3ld", &year, &doy);

   range->earliest = EncodeMSTimeDOY(year, doy, hour, minute, (REAL64)millisecond *SI_MILLI);
   range->latest = range->earliest + ((REAL64)length *SI_MILLI);

   return (TRUE);
   }

/*---------------------------------------------------------------------*/
BOOL CreateEventDirectory(H_ARCHIVE harchive, STREAM *stream)
   {
   CHAR path[MAX_PATH_LEN + 1];

   ASSERT(stream != NULL);

   if (!ValidateHandle(harchive))
      return (FALSE);

   /* Day directory */
   sprintf(path, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);
   TrimPath(path, 3);
   if (!IsDirectory(path))
      {
      if (!CreatePath(path))
         {
         _archive_error = ARC_FILE_IO_ERROR;
         return (FALSE);
         }
      }

   /* Unit directory */
   sprintf(path, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);
   TrimPath(path, 2);
   if (!IsDirectory(path))
      {
      if (!CreatePath(path))
         {
         _archive_error = ARC_FILE_IO_ERROR;
         return (FALSE);
         }
      }

   /* Stream directory */
   sprintf(path, "%s%c%s", _archive[harchive].path, PATH_DELIMITER, stream->event.filespec);
   TrimPath(path, 1);
   if (!IsDirectory(path))
      {
      if (!CreatePath(path))
         {
         _archive_error = ARC_FILE_IO_ERROR;
         return (FALSE);
         }
      }

   return (TRUE);
   }

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.4  2004/10/08 14:19:21  rstavely
 * Made changes to evnfiles.c to log error better when writing to archive fails.
 * Made changes to win32.c to try 3 times to open file if it fails.
 * Made David Chavez changes to do mutex lock for syslogd in log.c
 *
 * Revision 1.3  2002/01/18 17:53:21  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
    */
