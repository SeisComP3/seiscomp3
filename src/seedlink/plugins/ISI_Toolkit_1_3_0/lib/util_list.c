#pragma ident "$Id: list.c,v 1.10 2008/01/25 21:46:42 dechavez Exp $"
/*======================================================================
 *
 *  General linked list management.  
 *
 *  Includes pieces lifted essentially intact from Robert Banfill's
 *  linked list library (thanks!).
 *
 *  MT unsafe!
 *
 *====================================================================*/
#include "util.h"
#include "list.h"

VOID listPrintNode(LNKLST_NODE *node)
{
    printf("node=0x%08x:",    node);
    printf(" prev=0x%08x",    node->prev);
    printf(" next=0x%08x",    node->next);
    printf(" length=%lu",     node->length);
    printf(" payload=0x%08x", node->payload);
    printf(" parent=0x%08x",  node->payload);
    printf("\n");
}

VOID listPrintList(LNKLST *list)
{
LNKLST_NODE *crnt;

    printf("head: "); listPrintNode(&list->head);
    printf("tail: "); listPrintNode(&list->tail);
    crnt = listFirstNode(list);
    while (crnt != NULL) {
        listPrintNode(crnt);
        crnt = listNextNode(crnt);
    }
}

static VOID DestroyArrayView(LNKLST *list)
{
    if (list->array != NULL) {
        free(list->array);
        list->array = NULL;
    }
}

static VOID IncrementNodeCount(LNKLST *list)
{
    ++list->count;
    DestroyArrayView(list);
}

static VOID DecrementNodeCount(LNKLST *list)
{
    --list->count;
    DestroyArrayView(list);
}

LNKLST_NODE *listDestroyNode(LNKLST_NODE *crnt)
{
LNKLST *parent;
LNKLST_NODE *next;

    if (crnt == NULL || crnt->next == NULL || crnt->prev == NULL) {
        errno = EINVAL;
        return NULL;
    }

    parent = (LNKLST *) crnt->parent;

    /* save pointer to next node */

    next = listNextNode(crnt); /* returns NULL if no more */

    /* remove the node from the list */

    crnt->prev->next = crnt->next;
    crnt->next->prev = crnt->prev;

    /* delete it */

    if (crnt->payload != NULL) free(crnt->payload);
    free(crnt);
    DecrementNodeCount(parent);

    /* return pointer to next node */

    return next;
}

static VOID *NewNode(VOID *parent, VOID *payload, UINT32 length)
{
LNKLST_NODE *new;

    new = (LNKLST_NODE *) malloc(sizeof(LNKLST_NODE));
    if (new == NULL) return NULL;

    new->prev = NULL;
    new->next = NULL;
    new->length = length;
    if (new->length > 0) {
        new->payload = malloc(new->length);
        if (new->payload == NULL) return NULL;
        memcpy(new->payload, payload, length);
    } else {
        new->payload = NULL;
    }
    new->parent = parent;
    IncrementNodeCount((LNKLST *) new->parent);

    return new;
}

static VOID InitializeList(LNKLST *list)
{
    list->head.prev = &list->head;
    list->head.next = &list->tail;
    list->head.length = 0;
    list->head.payload = NULL;
    list->head.parent = (VOID *) list;

    list->tail.prev = &list->head;
    list->tail.next = &list->tail;
    list->tail.length = 0;
    list->tail.payload = NULL;
    list->tail.parent = (VOID *) list;

    list->count = 0;
    list->array = NULL;
}

BOOL listInit(LNKLST *list)
{
    if (list == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    InitializeList(list);
    list->FreeMe = FALSE;

    return TRUE;
}

LNKLST *listCreate()
{
LNKLST *list;

    if ((list = (LNKLST *) malloc(sizeof(LNKLST))) == NULL) return NULL;

    if (!listInit(list)) {
        free(list);
        return NULL;
    }

    list->FreeMe = TRUE;

    return list;
}

LNKLST_NODE *listFirstNode(LNKLST *list)
{
LNKLST_NODE *retval;

    if (list == NULL) return NULL;

    if (list->head.next == &list->tail) {
        retval = NULL; /* empty list */
    } else {
        retval = list->head.next;
    }

    return retval;
}

LNKLST_NODE *listLastNode(LNKLST *list)
{
LNKLST_NODE *retval;

    if (list == NULL) return NULL;

    if (list->head.next == &list->tail) {
        retval = NULL; /* empty list */
    } else {
        retval = list->tail.prev;
    }

    return retval;
}

LNKLST_NODE *listNextNode(LNKLST_NODE *crnt)
{
LNKLST_NODE *retval;

    if (crnt == NULL) return NULL;

    if (crnt->next->next == crnt->next) {
        retval = NULL; /* end of list */
    } else {
        crnt = crnt->next;
        retval = crnt;
    }

    return retval;
}

LNKLST_NODE *listPrevNode(LNKLST_NODE *crnt)
{
LNKLST_NODE *retval;

    if (crnt == NULL) return NULL;

    if (crnt->prev->prev == crnt->prev) {
        retval = NULL; /* begining of list */
    } else {
        retval = crnt->prev;
    }

    return retval;
}

LNKLST_NODE *listInsertAfter(LNKLST_NODE *crnt, VOID *payload, UINT32 length)
{
LNKLST_NODE *new, *next;

    if (crnt == NULL || payload == NULL) return NULL;

    if ((new = NewNode(crnt->parent, payload, length)) == NULL) return NULL;

    next = crnt->next;

    new->prev = crnt;
    new->next = crnt->next;

    crnt->next = new;
    next->prev = new;

    return new;
}

LNKLST_NODE *listInsertBefore(LNKLST_NODE *crnt, VOID *payload, UINT32 length)
{
LNKLST_NODE *new, *prev;

    if (crnt == NULL || payload == NULL) return NULL;

    if ((new = NewNode(crnt->parent, payload, length)) == NULL) return NULL;

    prev = crnt->prev;

    new->prev = crnt->prev;
    new->next = crnt;

    crnt->prev = new;
    prev->next = new;

    return new;
}

BOOL listAppend(LNKLST *list, VOID *payload, UINT32 length)
{
LNKLST_NODE *crnt;

    if (length == 0) return TRUE; /* ignore empty payloads */

    crnt = &list->head;
    while (crnt->next != &list->tail) {
        crnt = crnt->next;
    }
    return (listInsertAfter(crnt, payload, length) == NULL) ? FALSE : TRUE;
}

LNKLST_NODE *listRemoveNode(LNKLST *list, LNKLST_NODE *node)
{
    if (list == NULL || node == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (node->parent != list) { /* sanity check */
        errno = EINVAL;
        return NULL;
    }
    
    return listDestroyNode(node);
}

VOID listDestroy(LNKLST *list)
{
LNKLST_NODE *crnt;

    if (list == NULL) return;
    if ((crnt = listFirstNode(list)) == NULL) return;
    while ((crnt = listDestroyNode(crnt)) != NULL);

    if (list->array != NULL) free(list->array);
    list->array = NULL;
    list->count = 0;

    if (list->FreeMe == TRUE) free(list);
}

VOID listClear(LNKLST *list)
{
BOOL FreeMe;

    if (list == NULL) return;

    FreeMe = list->FreeMe;
    list->FreeMe = FALSE;
    listDestroy(list);
    InitializeList(list);
    list->FreeMe = FreeMe;
}

BOOL listSetArrayView(LNKLST *list)
{
UINT32 count;
LNKLST_NODE *crnt;

    if (list == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (list->count == 0) return TRUE;

    if (list->array != NULL) return TRUE;

    list->array = (VOID **) malloc(list->count * sizeof(VOID *));
    if (list->array == NULL) return FALSE;

    count = 0;
    crnt = listFirstNode(list);
    while (crnt != NULL) {
        if (count == list->count) { /* should never occur! */
            errno = EINVAL;
            free(list->array);
            return FALSE;
        }
        list->array[count++] = crnt->payload;
        crnt = listNextNode(crnt);
    }
    if (count != list->count) { /* should never occur! */
        errno = EINVAL;
        free(list->array);
        return FALSE;
    }

    return TRUE;
}

VOID listDump(FILE *fp, LNKLST *list)
{
UINT32 count = 0;
LNKLST_NODE *crnt;

    crnt = listFirstNode(list);
    while (crnt != NULL) {
        fprintf(fp, "%lu: len=%lu\n", count++, crnt->length);
        utilPrintHexDump(fp, crnt->payload, crnt->length);
        crnt = listNextNode(crnt);
    }
}

/* Copy a list */

BOOL listCopy(LNKLST *dest, LNKLST *src)
{
LNKLST_NODE *crnt;

    if (dest == NULL || src == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    listClear(dest);

    crnt = listFirstNode(src);
    while (crnt != NULL) {
        if (!listAppend(dest, crnt->payload, crnt->length)) return FALSE;
        crnt = listNextNode(crnt);
    }

    return TRUE;
}

#ifdef DEBUG_TEST

void WalkList(LNKLST *list)
{
UINT32 count = 0;
LNKLST_NODE *crnt;

    printf("WalkList, count = %lu\n", list->count);

    crnt = listFirstNode(list);
    while (crnt != NULL) {
        printf("%lu: '%s' len=%lu\n", count++, crnt->payload, crnt->length);
        crnt = listNextNode(crnt);
    }
}

void PrintArray(LNKLST *list)
{
UINT32 i;
char *payload;

    if (!listSetArrayView(list)) {
        perror("listSetArrayView");
        exit(1);
    }

    printf("PrintArray, count = %lu\n", list->count);

    for (i = 0; i < list->count; i++) {
        payload = list->array[i];
        printf("%lu: '%s' len=%lu\n", i, payload, strlen(payload));
    }
}

static VOID TestDestroy(LNKLST_NODE *crnt)
{
LNKLST *list;

    list = crnt->parent;
    printf("Destroy node with payload '%s'\n", crnt->payload);
    listDestroyNode(crnt);
    WalkList(list);
    PrintArray(list);
}

int main(int argc, char **argv)
{
int i, count = 0;
LNKLST list;
LNKLST_NODE *crnt;
static char string[MAXPATHLEN];

    if (argc > 1) {
        count = atoi(argv[1]);
    } else {
        fprintf(stderr, "usage: %s count\n", argv[0]);
        exit(1);
    }

    if (count < 1) {
        fprintf(stderr, "%s: bad count\n", argv[0]);
        exit(1);
    }

    listInit(&list);
    for (i = 0; i < count; i++) {
        sprintf(string, "this is element %d", i);
        if (!listAppend(&list, string, strlen(string))) {
            perror("listAppend");
            exit(1);
        }
    }

    WalkList(&list);
    PrintArray(&list);

    crnt = listFirstNode(&list);
    if (crnt == NULL) {
        fprintf(stderr, "unexpected NULL return from listFirstNode\n");
        exit(1);
    }
    crnt = listNextNode(crnt);
    TestDestroy(crnt);
    crnt = listNextNode(crnt);
    crnt = listNextNode(crnt);
    TestDestroy(crnt);

    printf("destroy entire list\n");
    listDestroy(&list);
    WalkList(&list);
    PrintArray(&list);
}

#endif /* DEBUG_TEST */

/* Revision History
 *
 * $Log: list.c,v $
 * Revision 1.10  2008/01/25 21:46:42  dechavez
 * added listCopy()
 *
 * Revision 1.9  2008/01/07 20:34:25  dechavez
 * made formerly static DestroyNode() public listDestroyNode()
 *
 * Revision 1.8  2006/02/14 17:05:30  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.7  2005/11/03 23:14:41  dechavez
 * removed unreferenced local variables
 *
 * Revision 1.6  2005/07/26 00:27:17  dechavez
 * added listRemoveNode() and listClear()
 *
 * Revision 1.5  2004/06/24 17:07:24  dechavez
 * removed unused variables (aap)
 *
 * Revision 1.4  2004/06/04 22:49:46  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.3  2003/11/19 23:30:38  dechavez
 * included util.h to calm certain compilers
 *
 * Revision 1.2  2003/11/04 00:37:19  dechavez
 * improved error checking in listCreate() and listSetArrayView()
 *
 * Revision 1.1  2003/10/16 16:06:55  dechavez
 * initial release (imported from ex-liblist)
 *
 */
