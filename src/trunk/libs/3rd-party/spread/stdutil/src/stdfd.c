/* Copyright (c) 2000-2009, The Johns Hopkins University
 * All rights reserved.
 *
 * The contents of this file are subject to a license (the ``License'').
 * You may not use this file except in compliance with the License. The
 * specific language governing the rights and limitations of the License
 * can be found in the file ``STDUTIL_LICENSE'' found in this 
 * distribution.
 *
 * Software distributed under the License is distributed on an AS IS 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. 
 *
 * The Original Software is:
 *     The Stdutil Library
 * 
 * Contributors:
 *     Creator - John Lane Schultz (jschultz@cnds.jhu.edu)
 *     The Center for Networking and Distributed Systems
 *         (CNDS - http://www.cnds.jhu.edu)
 */ 

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdutil/stderror.h>
#include <stdutil/stdfd.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  include <io.h>
#  include <sys/locking.h>
#  define STDFILENO(s) _fileno(s)
#  define STDFSYNC(fd) _commit(fd)
#else
#  include <unistd.h>
#  define STDFILENO(s) fileno(s)
#  define STDFSYNC(fd) fsync(fd)
#endif

/************************************************************************************************
 * stdfd_open: Open a file descriptor.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_open(stdfd *fd, const char *path, stdfd_access_type mode)
{
  stdcode      ret = STDESUCCESS;
  const char * fopen_mode;

  switch (mode) {
  case STDFD_READ_ONLY:
    fopen_mode = "rb";
    break;

  case STDFD_READ_WRITE_EXISTING:
    fopen_mode = "rb+";
    break;

  case STDFD_WRITE_ONLY:
    fopen_mode = "wb";
    break;

  case STDFD_READ_WRITE_NEW:
    fopen_mode = "wb+";
    break;

  case STDFD_APPEND_ONLY:
    fopen_mode = "ab";
    break;

  case STDFD_READ_APPEND:
    fopen_mode = "ab+";
    break;

  default:
    ret = STDEINVAL;
    goto stdfd_open_end;
  }

  if ((fd->stream = fopen(path, fopen_mode)) != NULL) {
    fd->fd = STDFILENO(fd->stream);

  } else {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

 stdfd_open_end:
  return ret;
}

/************************************************************************************************
 * stdfd_close: Close a file descriptor.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_close(stdfd *fd)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (fclose(fd->stream) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  fd->stream = NULL;
  fd->fd     = -1;

  return ret;
}

/************************************************************************************************
 * stdfd_read: Read 'nmemb' entries of 'nsize' bytes each from a file
 * descriptor into 'ptr.'
 ***********************************************************************************************/

STDINLINE stdcode stdfd_read(stdfd *fd, void *ptr, stdsize nsize, stdsize nmemb, stdsize *num)
{
  stdcode ret;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if ((*num = fread(ptr, nsize, nmemb, fd->stream)) == nmemb) {
    ret = STDESUCCESS;

  } else if (feof(fd->stream)) {
    ret = STDEOF;

  } else if (ferror(fd->stream)) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);

  } else {  
    ret = STDEINTR;  /* wrong count but not EOF or error? alert user! */
  }

  return ret;
}

/************************************************************************************************
 * stdfd_write: Write 'nmemb' entries of 'nsize' bytes each from 'ptr'
 * to a file descriptor.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_write(stdfd *fd, const void *ptr, stdsize nsize, stdsize nmemb, stdsize *num)
{
  stdcode ret;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if ((*num = fwrite(ptr, nsize, nmemb, fd->stream)) == nmemb) {
    ret = STDESUCCESS;

  } else if (feof(fd->stream)) {
    ret = STDEOF;

  } else if (ferror(fd->stream)) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);

  } else {  
    ret = STDEINTR;  /* wrong count but not EOF or error? alert user! */
  }

  return ret;
}

/************************************************************************************************
 * stdfd_flush: Flush any user space data on a file descriptor to the OS.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_flush(stdfd *fd)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (fflush(fd->stream) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}

/************************************************************************************************
 * stdfd_sync: Flush user space data to the OS and force the OS to
 * flush all data to disk.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_sync(stdfd *fd)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (fflush(fd->stream) != 0 || STDFSYNC(fd->fd) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}

/************************************************************************************************
 * stdfd_seek: Move the read/write head position of a file descriptor.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_seek(stdfd *fd, long offset, stdfd_whence whence)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (fseek(fd->stream, offset, whence) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}

/************************************************************************************************
 * stdfd_tell: Get the read/write head position of a file descriptor.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_tell(stdfd *fd, long *pos)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if ((*pos = ftell(fd->stream)) == -1) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}

/************************************************************************************************
 * stdfd_eof: Return whether or not the EOF indicator on a file descriptor is set.
 ***********************************************************************************************/

STDINLINE stdbool stdfd_eof(stdfd *fd)
{
  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  return (feof(fd->stream) != 0);
}

/************************************************************************************************
 * stdfd_err: Return whether or not an error indicator on a file descriptor is set.
 ***********************************************************************************************/

STDINLINE stdbool stdfd_err(stdfd *fd)
{
  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  return (ferror(fd->stream) != 0);
}

/************************************************************************************************
 * stdfd_clr_err: Clear any error indicator on a file descriptor.
 ***********************************************************************************************/

STDINLINE void stdfd_clr_err(stdfd *fd)
{
  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  clearerr(fd->stream);
}

/************************************************************************************************
 * stdfd_trylock: Try to acquire an advisory lock on a file
 * descriptor's underlying file.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_trylock(stdfd *fd)
#if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (_locking(fd->fd, _LK_NBLCK, 1) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}
#else
{
  struct flock lock = { 0 };
  stdcode ret       = STDESUCCESS;

  lock.l_type = F_WRLCK;

  if (fcntl(fd->fd, F_SETLK, &lock) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);    
  }

  return ret;
}
#endif

/************************************************************************************************
 * stdfd_unlock: Release an advisory lock on a file descriptor's
 * underlying file.
 ***********************************************************************************************/

STDINLINE stdcode stdfd_unlock(stdfd *fd)
#if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(fd->stream != NULL && fd->fd >= 0);

  if (_locking(fd->fd, _LK_UNLCK, 1) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }  

  return ret;
}
#else
{
  struct flock lock = { 0 };
  stdcode ret       = STDESUCCESS;

  lock.l_type = F_UNLCK;
  
  if (fcntl(fd->fd, F_SETLK, &lock) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
 }
#endif

/************************************************************************************************
 * stdfile_unlink: Erase a file.
 ***********************************************************************************************/

STDINLINE stdcode stdfile_unlink(const char *path)
{
  stdcode ret = STDESUCCESS;

  if (remove(path) != 0) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}

#ifdef __cplusplus
}
#endif
