#pragma ident "$Id: errors.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : errors.h
 Purpose  : Error strings.
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


#ifndef _ERRORS_H_INCLUDED_
#define _ERRORS_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include <platform.h>

/*---------------------------------------------------------------------*/
CHAR *archive_error_strings[ARC_N_ERRORS] = {
    "No archive error",
    "ArchiveError: Archive library has not been initialized!",
    "ArchiveError: Archive handle is invalid",
    "ArchiveError: File I/O error",
    "ArchiveError: No more handles, the maximum number of archives are open",
    "ArchiveError: Attempted to open a nonexistent archive",
    "ArchiveError: Attempted to create an existing archive",
    "ArchiveError: Path to archive does not exist",
    "ArchiveError: Not enough RAM to complete operation",
    "ArchiveError: Not enough disk space is available",
    "ArchiveError: An operation was attempted on unopened archive",
    "ArchiveError: Encountered an invalid PASSCAL recording format packet",
    "ArchiveError: A serious internal error was encountered",
    "ArchiveError: Permission is denied",
    "ArchiveError: No data matching search criteria was found",
    "ArchiveError: Invalid search criteria was specified",
    "ArchiveError: Stream handle is invalid",
    "ArchiveError: No more stream handles, too many streams are open",
    "ArchiveError: Event data is discontinuous",
    "ArchiveError: End of data encountered",
    "ArchiveError: Unable to determine sampling rate in data segment",
    "ArchiveError: Unable to start purge thread"
};

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
