#pragma ident "$Id: unix.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : unix.c
 Purpose  : Unix specific functions for archive library.
 Host     : CC, GCC
 Target   : POSIX
 Author   : David Chavez (dec@essw.com)
 Company  : Engineering Services & Software
 Copyright: (c) 1998 Refraction Technology, Inc. - All Rights Reserved.
 Notes    : This file provides platform independent file I/O and directory
            handling functions.
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  03 Sep 1998  ---- (DEC) First effort.

-----------------------------------------------------------------------*/
#ifndef unix
#   error "Unix specific source!"
#endif

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>

#include "util.h"
#include "archive.h"

#define NO_ERROR         0
#define ERROR_HANDLE_EOF 1
#define OTHER_ERROR      2

/* Constants ----------------------------------------------------------*/

/* Module globals -----------------------------------------------------*/
static UINT32 _FileLastError = NO_ERROR;
static int last_errno = 0;

static void update_error_status(FILE * fp)
{

    if (fp == (FILE *) NULL || ferror(fp)) {
        last_errno = errno;
        _FileLastError = OTHER_ERROR;
    }
    else if (feof(fp)) {
        last_errno = 0;
        _FileLastError = ERROR_HANDLE_EOF;
    }
    else {
        last_errno = 0;
        _FileLastError = NO_ERROR;
    }
}

/*---------------------------------------------------------------------
  File I/O functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
H_FILE FileOpenForWrite(CHAR * filespec)
{
    FILE *fp;

    assert(filespec != 0);
    assert(filespec[0] != '\0');

    if ((fp = fopen(filespec, "w+")) == (FILE *) NULL) {
        switch (errno) {
          case ENOENT:
          case ENOTDIR:
          case EACCES:
          case ENAMETOOLONG:
            _archive_error = ARC_BAD_PATH;
            break;
          default:
            _archive_error = ARC_FILE_IO_ERROR;
            update_error_status(fp);
            break;
        }
    }

    return (H_FILE) fp;
}

/*---------------------------------------------------------------------*/
H_FILE FileOpenForRead(CHAR * filespec)
{
    FILE *fp;

    assert(filespec != 0);
    assert(filespec[0] != '\0');

    if ((fp = fopen(filespec, "r")) == (FILE *) NULL) {
        switch (errno) {
          case ENOENT:
          case ENOTDIR:
          case EACCES:
          case ENAMETOOLONG:
            _archive_error = ARC_BAD_PATH;
            break;
          default:
            _archive_error = ARC_FILE_IO_ERROR;
            update_error_status(fp);
            break;
        }
    }

    return (H_FILE) fp;
}

/*---------------------------------------------------------------------*/
BOOL FileClose(H_FILE handle)
{

    if (handle == VOID_H_FILE)
        return TRUE;

    if (fclose((FILE *) handle) != 0) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileWrite(H_FILE handle, VOID * data, UINT32 * nbytes)
{
    UINT32 byteswritten;

    assert(handle != VOID_H_FILE);
    assert(data != NULL);
    assert(nbytes != NULL);
    assert(*nbytes > 0);

    byteswritten = (UINT32) fwrite(
        (void *) data,
        sizeof(char),
        (size_t) (*nbytes),
        (FILE *) handle
        );

    if (*nbytes != byteswritten) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileRead(H_FILE handle, VOID * data, UINT32 * nbytes)
{
    UINT32 bytesread;

    assert(handle != VOID_H_FILE);
    assert(data != NULL);
    assert(nbytes != NULL);
    assert(*nbytes > 0);

    bytesread = (UINT32) fread(
        (void *) data,
        sizeof(char),
        (size_t) (*nbytes),
        (FILE *) handle
        );

    if (*nbytes != bytesread) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileFlush(H_FILE handle)
{

    assert(handle != VOID_H_FILE);

    if (fflush((FILE *) handle) != 0) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileRewind(H_FILE handle)
{

    assert(handle != VOID_H_FILE);

    clearerr((FILE *) handle);
    rewind((FILE *) handle);
    if (ferror((FILE *) handle)) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileSeekEOF(H_FILE handle)
{

    assert(handle != VOID_H_FILE);

    if (fseek((FILE *) handle, (long) 0, SEEK_END) != 0) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileSeekAbsolute(H_FILE handle, INT32 offset)
{

    assert(handle != VOID_H_FILE);

    if (fseek((FILE *) handle, (long) offset, SEEK_SET) != 0) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL FileSeekRelative(H_FILE handle, INT32 offset)
{

    assert(handle != VOID_H_FILE);

    if (fseek((FILE *) handle, (long) offset, SEEK_CUR) != 0) {
        update_error_status((FILE *) handle);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
UINT32 FilePosition(H_FILE handle)
{
    long tell;

    assert(handle != VOID_H_FILE);

    if ((tell = ftell((FILE *) handle)) < 0) {
        update_error_status((FILE *) handle);
        return (VOID_UINT32);
    }

    return (UINT32) tell;
}

/*---------------------------------------------------------------------*/
BOOL FileAtEOF(VOID)
{

    return _FileLastError == ERROR_HANDLE_EOF ? TRUE : FALSE;
}

/*---------------------------------------------------------------------*/
BOOL FileError(VOID)
{

    return _FileLastError == OTHER_ERROR ? TRUE : FALSE;
}

/*---------------------------------------------------------------------*/
UINT32 FileLastError(VOID)
{

    return (UINT32) last_errno;
}

/*---------------------------------------------------------------------*/
CHAR *FileErrorString(UINT32 error)
{

    return strerror((int) error);
}

/*---------------------------------------------------------------------
  Directory and file testing functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
VOID QualifyPath(CHAR * qualified, CHAR * pathname)
{
    CHAR *ptr;

    assert(qualified != NULL);
    assert(pathname != NULL);
    strncpy(qualified, pathname, MAX_PATH_LEN);
    return;
}

/*---------------------------------------------------------------------*/
BOOL CreatePath(CHAR * pathname)
{

    assert(pathname != NULL);
    assert(pathname[0] != '\0');

    if (mkdir((char *) pathname, 0775) != 0) {
        update_error_status((FILE *) NULL);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL DestroyPath(CHAR * pathname)
{

    assert(pathname != NULL);
    assert(pathname[0] != '\0');

    if (rmdir(pathname) != 0) {
        update_error_status((FILE *) NULL);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------*/
BOOL TrimPath(CHAR * pathname, UINT16 levels)
{
    CHAR *ptr;

    assert(pathname != NULL);
    assert(pathname[0] != '\0');

    while (levels > 0) {
        /* Find last path delimiter */
        ptr = strrchr(pathname, PATH_DELIMITER);

        /* If not found, error */
        if (ptr == NULL)
            return (FALSE);

        /* Terminate at delimiter */
        *ptr = '\0';

        /* Catch the root directory */
        if (ptr - pathname >= 1) {
            if (*(ptr - 1) == ':') {
                *ptr = PATH_DELIMITER;
                *(ptr + 1) = '\0';
            }
        }

        levels--;
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL IsRegularFile(CHAR * pathname)
{
    struct stat buf;

    assert(pathname != NULL);
    assert(pathname[0] != '\0');

    /* Return TRUE if pathname is a regular file, FALSE if file doesn't exist
       or isn't regular. */

    if (stat(pathname, &buf) == 0) {
        if (buf.st_mode & S_IFREG)
            return (TRUE);
    }

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL IsDirectory(CHAR * pathname)
{
    struct stat buf;

    assert(pathname != NULL);
    assert(pathname[0] != '\0');

    /* Return TRUE in pathname is a directory FALSE if it doesn't exist or
       isn't a directory. */

    if (stat(pathname, &buf) == 0) {
        if (buf.st_mode & S_IFDIR)
            return (TRUE);
    }

    return (FALSE);
}

/*---------------------------------------------------------------------*/
UINT64 DiskSpace(CHAR * pathname)
{
    struct stat sbuf;
    struct ustat ubuf;

    if (stat((char *) pathname, &sbuf) != 0)
        return 0;
    if (ustat(sbuf.st_dev, &ubuf) != 0)
        return 0;

    return (UINT64) ubuf.f_tfree * (UINT64) 512;

}

/*---------------------------------------------------------------------*/
BOOL GetFileStat(CHAR * pathname, FILE_STAT * statbuf)
{

    assert(pathname != NULL);
    assert(pathname[0] != '\0');
    assert(stat != NULL);

    if (stat((char *) pathname, (struct stat *) statbuf) != 0) {
        _archive_error = ARC_BAD_PATH;
        return (FALSE);
    }

    return (TRUE);
}

/*---------------------------------------------------------------------
  File and directory find functions...
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
BOOL FileFindFirst(EVENT * event)
{
    struct _finddata_t finddata;

    assert(event != NULL);

    if ((event->find = (H_FIND) util_findfirst(event->filespec, &finddata)) != (H_FIND) - 1) {
        do {
            if (event->attribute == FIND_FILE) {
                if (finddata.attrib & _A_SUBDIR ||
                    finddata.attrib & _A_HIDDEN ||
                    finddata.attrib & _A_SYSTEM)
                    continue;
            }
            else if (finddata.attrib != event->attribute)
                continue;

            event->bytes = (UINT64) finddata.size;
            strcpy(event->filespec, finddata.name);
            return (TRUE);

        } while ((util_findnext((INT32) event->find, &finddata)) == 0);
    }

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL FileFindNext(EVENT * event)
{
    struct _finddata_t finddata;

    assert(event != NULL);

    while (util_findnext((INT32) event->find, &finddata) == 0) {
        if (event->attribute == FIND_FILE && finddata.attrib & _A_SUBDIR)
            continue;
        else if (event->attribute == FIND_SUBDIR && !(finddata.attrib & _A_SUBDIR))
            continue;

        event->bytes = (UINT64) finddata.size;
        strcpy(event->filespec, finddata.name);
        return (TRUE);
    }

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL FileFindClose(EVENT * event)
{

    assert(event != NULL);

    util_findclose((INT32) event->find);

    return (FALSE);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
