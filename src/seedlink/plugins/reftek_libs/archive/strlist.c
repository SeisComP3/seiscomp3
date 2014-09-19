#pragma ident "$Id: strlist.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : strlist.c
 Purpose  : Stream list API functions.
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

#define _STRLIST_C
#include "archive.h"

/*---------------------------------------------------------------------*/
STREAM *ArchiveFirstStream(H_ARCHIVE harchive, VOID ** ptr)
{
    NODE *node;

    ASSERT(ptr != NULL);

    if (!ValidateHandle(harchive))
        return (NULL);

    if ((node = FirstNode(&_archive[harchive].streams)) != NULL) {
        if (node->length == sizeof(STREAM)) {
            *ptr = node;
            return ((STREAM *) node->data);
        }
    }

    return (NULL);
}

/*---------------------------------------------------------------------*/
STREAM *ArchiveLastStream(H_ARCHIVE harchive, VOID ** ptr)
{
    NODE *node;

    ASSERT(ptr != NULL);

    if (!ValidateHandle(harchive))
        return (NULL);

    if ((node = LastNode(&_archive[harchive].streams)) != NULL) {
        if (node->length == sizeof(STREAM)) {
            *ptr = node;
            return ((STREAM *) node->data);
        }
    }

    return (NULL);
}

/*---------------------------------------------------------------------*/
STREAM *ArchiveNextStream(VOID ** ptr)
{
    NODE *node;

    ASSERT(ptr != NULL);

    node = (NODE *) * ptr;

    if ((node = NextNode(node)) != NULL) {
        if (node->length == sizeof(STREAM)) {
            *ptr = node;
            return ((STREAM *) node->data);
        }
    }

    return (NULL);
}

/*---------------------------------------------------------------------*/
STREAM *ArchivePrevStream(VOID ** ptr)
{
    NODE *node;

    ASSERT(ptr != NULL);

    node = (NODE *) * ptr;

    if ((node = PrevNode(node)) != NULL) {
        if (node->length == sizeof(STREAM)) {
            *ptr = node;
            return ((STREAM *) node->data);
        }
    }

    return (NULL);
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
