#pragma ident "$Id: create.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : create.c
 Purpose  : Creation functions.
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

#define _CREATE_C
#include "archive.h"

/*---------------------------------------------------------------------*/
BOOL ArchiveCreate(CHAR * path, CHAR * name, UINT64 thres_bytes, UINT64 max_bytes)
{
    CHAR qualified[MAX_PATH_LEN + 1];
    STATE state;

    if (path == NULL) {
        _archive_error = ARC_BAD_PATH;
        return (FALSE);
    }

    /* Fully qualify the path */
    QualifyPath(qualified, path);

    /* Create base directory if needed */
    if (!IsDirectory(qualified)) {
        if (!TrimPath(qualified, 1)) {
            _archive_error = ARC_BAD_PATH;
            return (FALSE);
        }
        if (!IsDirectory(qualified)) {
            _archive_error = ARC_BAD_PATH;
            return (FALSE);
        }
        else {
            QualifyPath(qualified, path);
            if (!CreatePath(qualified)) {
                _archive_error = ARC_BAD_PATH;
                return (FALSE);
            }
        }
    }

    /* State filespec */
    sprintf(qualified, "%s%c%s", qualified, PATH_DELIMITER, STATE_FILENAME);
    if (IsRegularFile(qualified)) {
        _archive_error = ARC_EXISTS;
        return (FALSE);
    }

    InitState(&state);

    /* Setup diskspace limits */
    state.max_bytes = max_bytes;
    state.thres_bytes = thres_bytes;

    if (name != NULL && name[0] != '\0') {
        if (strlen(name) >= MAX_NAME_LEN)
            name[MAX_NAME_LEN] = '\0';
        strcpy(state.name, name);
    }

    if (!CreateState(qualified, &state))
        return (FALSE);

    return (TRUE);
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
