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

#ifndef stdfd_h_2003_10_20_13_49_08_jschultz_at_cnds_jhu_edu
#define stdfd_h_2003_10_20_13_49_08_jschultz_at_cnds_jhu_edu

#include <stdio.h>
#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  int    fd;
  FILE * stream;

} stdfd;

#define STDFD_INIT() { -1, NULL }

typedef enum 
{
  STDFD_READ_ONLY = 2000,     /* rb  */
  STDFD_READ_WRITE_EXISTING,  /* rb+ */
  STDFD_WRITE_ONLY,           /* wb  */
  STDFD_READ_WRITE_NEW,       /* wb+ */
  STDFD_APPEND_ONLY,          /* ab  */
  STDFD_READ_APPEND           /* ab+ */

} stdfd_access_type;          /* as per fopen(): redefined for static enum check warnings */

typedef enum 
{
  STDSEEK_SET = SEEK_SET,
  STDSEEK_CUR = SEEK_CUR,
  STDSEEK_END = SEEK_END

} stdfd_whence;               /* as per fseek(): redefined for static enum check warnings */

/* File Descriptor Operations */

STDINLINE stdcode stdfd_open(stdfd *fd, const char *path, stdfd_access_type mode);
STDINLINE stdcode stdfd_close(stdfd *fd);

STDINLINE stdcode stdfd_read(stdfd *fd, void *ptr, stdsize nsize, stdsize nmemb, stdsize *num);
STDINLINE stdcode stdfd_write(stdfd *fd, const void *ptr, stdsize nsize, stdsize nmemb, stdsize *num);

STDINLINE stdcode stdfd_flush(stdfd *fd);
STDINLINE stdcode stdfd_sync(stdfd *fd);

STDINLINE stdcode stdfd_seek(stdfd *fd, long offset, stdfd_whence whence);
STDINLINE stdcode stdfd_tell(stdfd *fd, long *pos);

STDINLINE stdbool stdfd_eof(stdfd *fd);
STDINLINE stdbool stdfd_err(stdfd *fd);
STDINLINE void    stdfd_clr_err(stdfd *fd);

STDINLINE stdcode stdfd_trylock(stdfd *fd);
STDINLINE stdcode stdfd_unlock(stdfd *fd);

/* File Operations */

STDINLINE stdcode stdfile_unlink(const char * path);

#ifdef __cplusplus
}
#endif

#endif
