#pragma ident "$Id: win32.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
Program  : Any
Task     : Archive Library API functions.
File     : win32.c
Purpose  : Win32 specific functions for archive library.
Host     : CC, GCC, Microsoft Visual C++ 5.x
Target   : Solaris (Sparc and x86), Linux, Win32
Author   : Robert Banfill (r.banfill@reftek.com)
Company  : Refraction Technology, Inc.
2626 Lombardy Lane, Suite 105
Dallas, Texas  75220  USA
(214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
Notes    : This file provides platform independent file I/O and directory
handling functions.
$Revision: 165 $
$Logfile : R:/cpu68000/rt422/struct/version.h_v  $
Revised  :
22 Sep 2004  ----  (rs) retry file open for testing Marks problem
17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#define _PLATFORM_C
#include "archive.h"

/* Constants ----------------------------------------------------------*/
#define FILE_ERROR_STRING_LENGTH 255

/* Module globals -----------------------------------------------------*/
static UINT32 _FileLastError;

static CHAR _FileErrorString[FILE_ERROR_STRING_LENGTH + 1];

/*---------------------------------------------------------------------
File I/O functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
H_FILE FileOpenForWrite(CHAR *filespec)
   {
   H_FILE handle;
   INT16 x;

   ASSERT(filespec != 0);
   ASSERT(filespec[0] != '\0');

   /* Open for read/write if exists, if not, create */
   for (x = 0; x < 3; x++)
      {
      handle = CreateFile(filespec, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,  /* Allow others to read only */
      NULL, OPEN_ALWAYS,  /* Create if needed */
      FILE_ATTRIBUTE_NORMAL, NULL);
      if (handle != VOID_H_FILE)
         break;
      sleep(1);
      }

   _FileLastError = GetLastError();

   return (handle);
   }

/*---------------------------------------------------------------------*/
H_FILE FileOpenForRead(CHAR *filespec)
   {
   H_FILE handle;

   ASSERT(filespec != 0);
   ASSERT(filespec[0] != '\0');

   /* Open for read */
   handle = CreateFile(filespec, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,  /* Allow others to read and write */
   NULL, OPEN_EXISTING,  /* Must exist */
   FILE_ATTRIBUTE_NORMAL, NULL);

   _FileLastError = GetLastError();

   return (handle);
   }

/*---------------------------------------------------------------------*/
BOOL FileClose(H_FILE handle)
   {
   BOOL returnvalue;

   if (handle == VOID_H_FILE)
      return (TRUE);

   returnvalue = TRUE;

   if (!CloseHandle(handle))
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileWrite(H_FILE handle, VOID *data, UINT32 *nbytes)
   {
   BOOL returnvalue;
   UINT32 byteswritten;

   ASSERT(handle != VOID_H_FILE);
   ASSERT(data != NULL);
   ASSERT(nbytes != NULL);
   ASSERT(*nbytes > 0);

   returnvalue = TRUE;

   if (!WriteFile(handle, data,  *nbytes, &byteswritten, NULL))
      returnvalue = FALSE;

   if (*nbytes != byteswritten)
      returnvalue = FALSE;

   _FileLastError = GetLastError();
   *nbytes = byteswritten;

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileRead(H_FILE handle, VOID *data, UINT32 *nbytes)
   {
   BOOL returnvalue;
   UINT32 bytesread;

   ASSERT(handle != VOID_H_FILE);
   ASSERT(data != NULL);
   ASSERT(nbytes != NULL);
   ASSERT(*nbytes > 0);

   returnvalue = TRUE;

   if (!ReadFile(handle, data,  *nbytes, &bytesread, NULL))
      {
      returnvalue = FALSE;
      _FileLastError = GetLastError();
      }

   if (*nbytes != bytesread)
      {
      _FileLastError = ERROR_HANDLE_EOF;
      returnvalue = FALSE;
      }

   *nbytes = bytesread;

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileFlush(H_FILE handle)
   {
   BOOL returnvalue;

   ASSERT(handle != VOID_H_FILE);

   returnvalue = TRUE;

   if (FlushFileBuffers(handle) == 0)
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileRewind(H_FILE handle)
   {
   BOOL returnvalue;

   ASSERT(handle != VOID_H_FILE);

   returnvalue = TRUE;

   if (SetFilePointer(handle, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileSeekEOF(H_FILE handle)
   {
   BOOL returnvalue;

   ASSERT(handle != VOID_H_FILE);

   returnvalue = TRUE;

   if (SetFilePointer(handle, 0, NULL, FILE_END) == 0xFFFFFFFF)
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileSeekAbsolute(H_FILE handle, INT32 offset)
   {
   BOOL returnvalue;

   ASSERT(handle != VOID_H_FILE);

   returnvalue = TRUE;

   if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == 0xFFFFFFFF)
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
UINT32 FilePosition(H_FILE handle)
   {
   UINT32 position;

   ASSERT(handle != VOID_H_FILE);

   position = SetFilePointer(handle, 0, NULL, FILE_CURRENT);

   if ((_FileLastError = GetLastError()) != NO_ERROR)
      return (VOID_UINT32);

   return (position);
   }

/*---------------------------------------------------------------------*/
BOOL FileSeekRelative(H_FILE handle, INT32 offset)
   {
   BOOL returnvalue;

   ASSERT(handle != VOID_H_FILE);

   returnvalue = TRUE;

   if (SetFilePointer(handle, offset, NULL, FILE_CURRENT) == 0xFFFFFFFF)
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL FileAtEOF(VOID)
   {

   if (_FileLastError == ERROR_HANDLE_EOF)
      return (TRUE);

   return (FALSE);
   }

/*---------------------------------------------------------------------*/
BOOL FileError(VOID)
   {

   if (_FileLastError != NO_ERROR && _FileLastError != ERROR_HANDLE_EOF)
      return (TRUE);

   return (FALSE);
   }

/*---------------------------------------------------------------------*/
UINT32 FileLastError(VOID)
   {

   return (_FileLastError);
   }

/*---------------------------------------------------------------------*/
CHAR *FileErrorString(UINT32 error)
   {

   if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  /* Default language */
   _FileErrorString, FILE_ERROR_STRING_LENGTH, NULL) == 0)
      strcpy(_FileErrorString, "Unable to get error description");

   return (_FileErrorString);
   }

/*---------------------------------------------------------------------
Directory and file testing functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
VOID QualifyPath(CHAR *qualified, CHAR *pathname)
   {
   CHAR *ptr;

   ASSERT(qualified != NULL);
   ASSERT(pathname != NULL);

   /* Don't qualify UNC's (first two characters are "\\" or "//") */
   if (strncmp(pathname, "\\\\", 2) != 0 && strncmp(pathname, "//", 2) != 0)
      _fullpath(qualified, pathname, MAX_PATH_LEN);
   else
      strncpy(qualified, pathname, MAX_PATH_LEN);

   /* Use UNIX style path delimiters */
   ptr = qualified;
   while (*ptr)
      {
      if (*ptr == '\\')
         *ptr = PATH_DELIMITER;
      ptr++;
      }

   return ;
   }

/*---------------------------------------------------------------------*/
BOOL CreatePath(CHAR *pathname)
   {
   BOOL returnvalue;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   returnvalue = TRUE;

   if (!CreateDirectory(pathname, NULL))
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL DestroyPath(CHAR *pathname)
   {
   BOOL returnvalue;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   returnvalue = TRUE;

   if (!RemoveDirectory(pathname))
      returnvalue = FALSE;

   _FileLastError = GetLastError();

   return (returnvalue);
   }

/*---------------------------------------------------------------------*/
BOOL TrimPath(CHAR *pathname, UINT16 levels)
   {
   CHAR *ptr;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   while (levels > 0)
      {
      /* Find last path delimiter */
      ptr = strrchr(pathname, PATH_DELIMITER);

      /* If not found, error */
      if (ptr == NULL)
         return (FALSE);

      /* Terminate at delimiter */
      *ptr = '\0';

      /* Catch the root directory */
      if (ptr - pathname >= 1)
         {
         if (*(ptr - 1) == ':')
            {
            *ptr = PATH_DELIMITER;
            *(ptr + 1) = '\0';
            }
         }

      levels--;
      }

   return (TRUE);
   }

/*---------------------------------------------------------------------*/
BOOL IsRegularFile(CHAR *pathname)
   {
   struct _stat buf;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   /* Return TRUE if pathname is a regular file, FALSE if file doesn't exist
   or isn't regular. */

   if (_stat(pathname, &buf) == 0)
      {
      if (buf.st_mode &_S_IFREG)
         return (TRUE);
   }

   return (FALSE);
   }

/*---------------------------------------------------------------------*/
BOOL IsDirectory(CHAR *pathname)
   {
   struct _stat buf;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   /* Return TRUE in pathname is a directory FALSE if it doesn't exist or
   isn't a directory. */

   if (_stat(pathname, &buf) == 0)
      {
      if (buf.st_mode &_S_IFDIR)
         return (TRUE);
   }

   return (FALSE);
   }

/*---------------------------------------------------------------------*/
UINT64 DiskSpace(CHAR *pathname)
   {
   ULARGE_INTEGER myspace, disksize, freespace;

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');

   if (!GetDiskFreeSpaceEx(pathname, &myspace, &disksize, &freespace))
      return (0);

   return (myspace.QuadPart);
   }

/*---------------------------------------------------------------------*/
BOOL GetFileStat(CHAR *pathname, FILE_STAT *stat)
   {

   ASSERT(pathname != NULL);
   ASSERT(pathname[0] != '\0');
   ASSERT(stat != NULL);

   if (_stat(pathname, stat) != 0)
      {
      _archive_error = ARC_BAD_PATH;
      return (FALSE);
      }

   return (TRUE);
   }

/*---------------------------------------------------------------------
File and directory find functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
BOOL FileFindFirst(EVENT *event)
   {
   struct _finddata_t finddata;

   ASSERT(event != NULL);

   if ((event->find = _findfirst(event->filespec, &finddata)) != (H_FIND) - 1)
      {
      do
         {
         if (event->attribute == FIND_FILE)
            {
            if (finddata.attrib &_A_SUBDIR || finddata.attrib &_A_HIDDEN || finddata.attrib &_A_SYSTEM)
               continue;
         }
         else if (!(finddata.attrib &event->attribute))
            continue;

         event->bytes = (UINT64)finddata.size;
         strcpy(event->filespec, finddata.name);
         return (TRUE);

         }
      while ((_findnext(event->find, &finddata)) == 0);
      }

   return (FALSE)
      ;
   }

/*---------------------------------------------------------------------*/
BOOL FileFindNext(EVENT *event)
   {
   struct _finddata_t finddata;

   ASSERT(event != NULL);

   while (_findnext(event->find, &finddata) == 0)
      {
      if (event->attribute == FIND_FILE && finddata.attrib &_A_SUBDIR)
         continue;
      else if (event->attribute == FIND_SUBDIR && !(finddata.attrib &_A_SUBDIR))
         continue;

      event->bytes = (UINT64)finddata.size;
      strcpy(event->filespec, finddata.name);
      return (TRUE);
   }

   return (FALSE);
   }

/*---------------------------------------------------------------------*/
BOOL FileFindClose(EVENT *event)
   {

   ASSERT(event != NULL);

   _findclose(event->find);

   return (FALSE);
   }

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2004/10/08 14:19:21  rstavely
 * Made changes to evnfiles.c to log error better when writing to archive fails.
 * Made changes to win32.c to try 3 times to open file if it fails.
 * Made David Chavez changes to do mutex lock for syslogd in log.c
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
      */
