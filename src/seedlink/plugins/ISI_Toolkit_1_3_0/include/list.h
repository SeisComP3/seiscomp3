#pragma ident "$Id: list.h,v 1.6 2008/01/25 21:41:54 dechavez Exp $"
#ifndef list_include_defined
#define list_include_defined

/* platform specific stuff */

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* datatypes */

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    UINT32 length;
    void *payload;
    void *parent;
} LNKLST_NODE;

typedef struct {
    LNKLST_NODE head;
    LNKLST_NODE tail;
    UINT32 count;
    VOID **array;
    BOOL FreeMe;
} LNKLST;

/* Function prototypes */

/* list.c */
BOOL listInit(LNKLST *lp);
LNKLST *listCreate(VOID);
LNKLST_NODE *listFirstNode(LNKLST *lp);
LNKLST_NODE *listLastNode(LNKLST *lp);
LNKLST_NODE *listNextNode(LNKLST_NODE *crnt);
LNKLST_NODE *listPrevNode(LNKLST_NODE *crnt);
LNKLST_NODE *listInsertAfter(LNKLST_NODE *crnt, VOID *payload, UINT32 length);
LNKLST_NODE *listInsertBefore(LNKLST_NODE *crnt, VOID *payload, UINT32 length);
BOOL listAppend(LNKLST *lp, VOID *payload, UINT32 length);
LNKLST_NODE *listRemoveNode(LNKLST *list, LNKLST_NODE *node);
LNKLST_NODE *listDestroyNode(LNKLST_NODE *node);
VOID listDestroy(LNKLST *lp);
VOID listClear(LNKLST *list);
BOOL listSetArrayView(LNKLST *lp);
VOID listDump(FILE *fp, LNKLST *list);
BOOL listCopy(LNKLST *dest, LNKLST *src);

/* version.c */
char *listVersionString(VOID);
VERSION *listVersion(VOID);

#ifdef __cplusplus
}
#endif

#endif

/* Revision History
 *
 * $Log: list.h,v $
 * Revision 1.6  2008/01/25 21:41:54  dechavez
 * updated prototypes
 *
 * Revision 1.5  2008/01/07 20:35:03  dechavez
 * added listDestroyNode() prototype
 *
 * Revision 1.4  2006/02/14 17:05:04  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.3  2005/07/26 00:26:36  dechavez
 * added listRemoveNode() and listClear() prototypes
 *
 * Revision 1.2  2004/06/25 18:34:56  dechavez
 * C++ compatibility
 *
 * Revision 1.1  2003/10/16 18:35:09  dechavez
 * initial release
 *
 */
