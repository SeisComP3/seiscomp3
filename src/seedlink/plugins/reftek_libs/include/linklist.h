#pragma ident "$Id: linklist.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Generic linked list functions.
 File     : linklist.h
 Purpose  : Constants, types, globals, prototypes.
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


#ifndef _LINKLIST_H_INCLUDED_
#define _LINKLIST_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include <platform.h>

/* Constants ----------------------------------------------------------*/
#define GARBAGE_BYTE    0xCC           /* Freed memory filler */

/* Types --------------------------------------------------------------*/
/* Generic node type */
typedef struct _NODE {
    struct _NODE *prev;
    struct _NODE *next;
    UINT32 length;
    VOID *data;
} NODE;

/* Generic linked list type */
typedef struct _LIST {
    NODE head;                         /* Dummy head node */
    NODE tail;                         /* Dummy tail node */
} LIST;

/* Module globals -----------------------------------------------------*/
#if !defined _LINKLIST_C
#define _LINKLIST_C extern
#endif

/* Prototypes ---------------------------------------------------------*/
_LINKLIST_C BOOL CreateList( LIST * list );
_LINKLIST_C VOID DestroyList( LIST * list );

_LINKLIST_C NODE *FirstNode( LIST * list );
_LINKLIST_C NODE *LastNode( LIST * list );
_LINKLIST_C NODE *NextNode( NODE * node );
_LINKLIST_C NODE *PrevNode( NODE * node );

_LINKLIST_C NODE *InsertNodeAfter( NODE * node, VOID * data, UINT32 length );
_LINKLIST_C NODE *InsertNodeBefore( NODE * node, VOID * data, UINT32 length );

_LINKLIST_C NODE *DestroyNode( NODE * node );

#endif

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
