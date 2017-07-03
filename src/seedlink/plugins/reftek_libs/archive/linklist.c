#pragma ident "$Id: linklist.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Generic linked list API.
 File     : linklist.c
 Purpose  : API functions.
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

#include <stdio.h>
#include <stdlib.h>
// For Mac OS X use stdlib.h instead of malloc.h
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <memory.h>

#define _LINKLIST_C
#include "linklist.h"

/* Assertion macro ----------------------------------------------------*/

/*#ifdef _DEBUG
extern VOID _ArchiveAssert( CHAR * file, UINT32 line );

#define ASSERT( f )           \
      if( f )                 \
         NULL;                \
      else                    \
         _ArchiveAssert( __FILE__, __LINE__ )
#else
#define ASSERT( f ) NULL
 #endif *//* defined DEBUG */

/* Module Globals -----------------------------------------------------*/
UINT32 _allocated_nodes = 0;

/* Local helpers ------------------------------------------------------*/
static BOOL AllocNode(NODE ** node, UINT32 length);

/*---------------------------------------------------------------------*/
BOOL CreateList(LIST * list)
{

    ASSERT(list != NULL);

    /* Link head to tail */
    list->head.prev = &list->head;
    list->head.next = &list->tail;
    list->head.length = 0;
    list->head.data = NULL;

    /* Link tail to head */
    list->tail.prev = &list->head;
    list->tail.next = &list->tail;
    list->tail.length = 0;
    list->tail.data = NULL;

    return (TRUE);
}

/*---------------------------------------------------------------------*/
VOID DestroyList(LIST * list)
{
    NODE *node;

    ASSERT(list != NULL);

    /* Release the list */
    if ((node = FirstNode(list)) != NULL)
        while ((node = DestroyNode(node)) != NULL);

    return;
}

/*---------------------------------------------------------------------*/
NODE *FirstNode(LIST * list)
{

    ASSERT(list != NULL);

    /* Is the list empty? */
    if (list->head.next == &list->tail)
        return (NULL);

    return (list->head.next);
}

/*---------------------------------------------------------------------*/
NODE *LastNode(LIST * list)
{

    ASSERT(list != NULL);

    /* Is the list empty? */
    if (list->head.next == &list->tail)
        return (NULL);

    return (list->tail.prev);
}

/*---------------------------------------------------------------------*/
NODE *NextNode(NODE * node)
{

    ASSERT(node != NULL);

    /* End of list? (is next node the tail?) */
    if (node->next->next == node->next)
        return (NULL);

    return (node->next);
}

/*---------------------------------------------------------------------*/
NODE *PrevNode(NODE * node)
{

    ASSERT(node != NULL);

    /* Top of list? (is the prev node the head?) */
    if (node->prev->prev == node->prev)
        return (NULL);

    return (node->prev);
}

/*---------------------------------------------------------------------*/
NODE *InsertNodeAfter(NODE * node, VOID * data, UINT32 length)
{
    NODE *next;

    ASSERT(node != NULL);
    ASSERT(data != NULL);

    if (!AllocNode(&next, length))
        return (NULL);

    memcpy(next->data, data, length);

    /* Insert new next node */
    next->prev = node;
    next->next = node->next;
    next->next->prev = next;
    next->prev->next = next;

    return (next);
}

/*---------------------------------------------------------------------*/
NODE *InsertNodeBefore(NODE * node, VOID * data, UINT32 length)
{
    NODE *prev;

    ASSERT(node != NULL);
    ASSERT(data != NULL);

    if (!AllocNode(&prev, length))
        return (NULL);

    memcpy(prev->data, data, length);

    /* Insert new prev node */
    prev->prev = node->prev;
    prev->next = node;
    prev->next->prev = prev;
    prev->prev->next = prev;

    return (prev);
}

/*---------------------------------------------------------------------*/
NODE *DestroyNode(NODE * node)
{
    NODE *next;

    ASSERT(node != NULL);
    ASSERT(node->next != NULL);
    ASSERT(node->prev != NULL);

    /* Save pointer to next node, this will be NULL if this is the last node */
    next = NextNode(node);

    /* Unlink this node */
    node->prev->next = node->next;
    node->next->prev = node->prev;

    /* Release data memory */
    if (node->data != NULL) {

#ifdef _DEBUG
        memset(node->data, GARBAGE_BYTE, (size_t) node->length);
#endif

        free(node->data);
        node->data = NULL;
        node->length = 0;
    }

    /* Release node memory */

#ifdef _DEBUG
    memset(node, GARBAGE_BYTE, sizeof(NODE));
#endif

    free(node);
    node = NULL;

    _allocated_nodes--;

    return (next);
}

/*---------------------------------------------------------------------*/
/* Local helpers */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
BOOL AllocNode(NODE ** node, UINT32 length)
{

    ASSERT(node != NULL);

    (*node) = (NODE *) malloc(sizeof(NODE));

    if ((*node) == NULL)
        return (FALSE);

    (*node)->length = length;

    /* Allocate data memeory */
    if (length == 0)
        (*node)->data = NULL;
    else {
        (*node)->data = malloc((size_t) length);
        if ((*node)->data == NULL) {
            free((*node));
            (*node) = NULL;
            return (FALSE);
        }
    }

    _allocated_nodes++;

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
