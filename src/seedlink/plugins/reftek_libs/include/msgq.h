#pragma ident "$Id: msgq.h 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * Defines, templates, and prototypes for the portable message queue
 * libary.
 *
 *====================================================================*/
#ifndef msgq_include_defined
#define msgq_include_defined

/* platform specific stuff */

#include "platform.h"

/* Flags for message queue I/O */
 
#define MSGQ_WAIT   ((UINT16) 0)
#define MSGQ_NOWAIT ((UINT16) 1)

/* typedefs */

typedef struct msgq_msg {
    UINT8 *data;
    INT32 len;
    INT32 maxlen;
    struct msgq_msg *next;
} MSGQ_MSG;
 
typedef struct msg_queue {
    MSGQ_MSG  head;  /* the messages        */
    MUTEX     mp;    /* for protection      */
    SEMAPHORE sp;    /* for synchronization */
    INT32     nfree; /* for optimization    */
    INT32     lowat; /* for optimization    */
    INT32     hiwat; /* for optimization    */
} MSGQ;

/* function prototypes */

BOOL msgq_init(MSGQ *queue, INT32 nelem, INT32 maxelem, INT32 maxlen);
MSGQ_MSG *msgq_get(MSGQ *queue, UINT16 flag);
VOID msgq_put(MSGQ *queue, MSGQ_MSG *new);
INT32 msgq_nfree(MSGQ *queue);
INT32 msgq_lowat(MSGQ *queue);
INT32 msgq_hiwat(MSGQ *queue);
BOOL msgq_chkmsg(MSGQ_MSG *msg);
VOID msgq_flush(MSGQ *full, MSGQ *empty);

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:49:00  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
