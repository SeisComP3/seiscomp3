#pragma ident "$Id: errors.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : errors.c
 Purpose  : Error number and string functions.
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
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#define _ERRORS_C
#include "archive.h"
#include "errors.h"

/*---------------------------------------------------------------------*/
UINT32 ArchiveErrorNumber(H_ARCHIVE handle)
{

    /* Undefined handle returns global error */
    if (handle > MAX_OPEN_ARCHIVES)
        return (_archive_error);

    if (_archive[handle].last_error == ARC_NO_ERROR)
        return (_archive_error);

    return (_archive[handle].last_error);
}

/*---------------------------------------------------------------------*/
CHAR *ArchiveErrorString(UINT32 error)
{

    if (error >= ARC_N_ERRORS)
        error = 0;

    /* Catch and report file I/O errors */
    if (error == ARC_FILE_IO_ERROR)
        return (FileErrorString(FileLastError()));

    return (archive_error_strings[error]);
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
