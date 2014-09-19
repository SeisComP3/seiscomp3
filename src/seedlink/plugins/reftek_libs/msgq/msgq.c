#pragma ident "$Id: msgq.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Simple minded message queue to handle buffering of data between
 *  threads.  This implementation uses the mutex and semaphore macros.
 *
 *  A message is pulled off a queue with msgq_get, and put on a queue
 *  with msgq_put().  Use msgq_init() to initialize a queue and fill it
 *  it with pointers to pre-sized buffers.
 *
 *  The simple minded part is that here there is no notion of message
 *  type.  All messages are UBYTE pointers, and it is assumed that the
 *  application knows what to do with them, and won't clobber memory.
 *  Furthermore, there is no way provided to delete these queues.  This
 *  is basically because up to now the applications that use these want
 *  to have the queues around forever and so it hasn't been an issue.
 *
 *====================================================================*/
#include "msgq.h"
#include "rtp.h"  /* for logging facility */
#include "util.h" /* for hex dump */

#define TESTLEN  ((size_t) 4)
#define TESTCHAR 'e'

/* Initialize a queue with pre-sized buffers */

BOOL msgq_init(MSGQ *queue, INT32 nelem, INT32 maxelem, INT32 maxlen)
{
INT32 i;
MSGQ_MSG *crnt, *newm;

    MUTEX_INIT(&queue->mp);
    SEM_INIT(&queue->sp, nelem, maxelem);
    queue->head.next = (MSGQ_MSG *) NULL;
    queue->head.data = (UBYTE *) NULL;
    queue->nfree = nelem;
    queue->lowat = nelem;
    queue->hiwat = 0;

    crnt = &queue->head;
    for (i = 0; i < nelem; i++) {

    /* create the message object */

        newm = (MSGQ_MSG *) malloc(sizeof(MSGQ_MSG));
        if (newm == (MSGQ_MSG *) NULL) return FALSE;

    /* initialize the message object */

        newm->maxlen = maxlen;
        newm->data   = (UBYTE *) malloc(maxlen+TESTLEN);
        newm->len    = 0;
        if (newm->data == (UBYTE *) NULL) return FALSE;
        memset(
            (void *) newm->data, (int) TESTCHAR, (size_t) maxlen+TESTLEN
        );
        newm->next = (MSGQ_MSG *) NULL;

    /* add it to the end of the list */

        crnt->next = newm;
        crnt       = crnt->next;
    }

    return TRUE;
}

/* Pull the next message off the queue */

MSGQ_MSG *msgq_get(MSGQ *queue, UINT16 flag)
{
MSGQ_MSG *head, *msg;
static char *fid = "msgq_get";

    if (queue == (MSGQ *) NULL) {
        rtp_log(RTP_ERR, "%s: ABORT - NULL queue pointer received!", fid);
        abort();
    }

    if (flag == MSGQ_WAIT) {
        SEM_WAIT(&queue->sp);
    } else if (SEM_TRYWAIT(&queue->sp) != 0) {
        return (MSGQ_MSG *) NULL;
    }

    MUTEX_LOCK(&queue->mp);
        head = &queue->head;
        if (head->next == (MSGQ_MSG *) NULL) {
            rtp_log(RTP_ERR, "%s: ABORT - QUEUE MANAGMENT IS HOSED!", fid);
            abort();
        }
        msg = head->next;
        head->next = msg->next;
        if (--queue->nfree < queue->lowat) queue->lowat = queue->nfree;
    MUTEX_UNLOCK(&queue->mp);

    return msg;
}

VOID msgq_put(MSGQ *queue, MSGQ_MSG *newm)
{
MSGQ_MSG *crnt;

    MUTEX_LOCK(&queue->mp);
        crnt = &queue->head;
        while (crnt->next != (MSGQ_MSG *) NULL) crnt = crnt->next;
        crnt->next = newm;
        newm->next = (MSGQ_MSG *) NULL;
        SEM_POST(&queue->sp);
        if (++queue->nfree > queue->hiwat) queue->hiwat = queue->nfree;
    MUTEX_UNLOCK(&queue->mp);
}

INT32 msgq_nfree(MSGQ *queue)
{
INT32 retval;

    MUTEX_LOCK(&queue->mp);
        retval = queue->nfree;
    MUTEX_UNLOCK(&queue->mp);

    return retval;
}

INT32 msgq_lowat(MSGQ *queue)
{
INT32 retval;

    MUTEX_LOCK(&queue->mp);
        retval = queue->lowat;
    MUTEX_UNLOCK(&queue->mp);

    return retval;
}

INT32 msgq_hiwat(MSGQ *queue)
{
INT32 retval;

    MUTEX_LOCK(&queue->mp);
        retval = queue->hiwat;
    MUTEX_UNLOCK(&queue->mp);

    return retval;
}

/* Flush a queue */

VOID msgq_flush(MSGQ *full, MSGQ *empty)
{
MSGQ_MSG *msg;

    while ((msg = msgq_get(full, MSGQ_NOWAIT)) != (MSGQ_MSG *) NULL) {
        msgq_put(empty, msg);
    }
}

/* Check for message overruns */

BOOL msgq_chkmsg(MSGQ_MSG *msg)
{
static UBYTE tbuf[TESTLEN] = {TESTCHAR, TESTCHAR, TESTCHAR, TESTCHAR};
static CHAR *fid = "msgq_chkmsg";

    if (msg == (MSGQ_MSG *) NULL) {
        rtp_log(RTP_ERR, "ERROR: %s: msg == NULL!", fid);
        errno = EINVAL;
        return FALSE;
    }

    if (msg->maxlen <= 0) {
        rtp_log(RTP_ERR, "ERROR: %s: msg->maxlen=%ld <= 0!",
            fid, msg->maxlen
        );
        errno = EINVAL;
        return FALSE;
    }

    if (msg->data == (UBYTE *) NULL) {
        rtp_log(RTP_ERR, "ERROR: %s: msg->data == NULL!", fid);
        errno = EINVAL;
        return FALSE;
    }

    if (memcmp(
        (void *) (msg->data + msg->maxlen), 
        (void *) tbuf,
        (size_t) TESTLEN
    ) != 0) {
        rtp_log(RTP_ERR, "ERROR: %s: buffer overflow (maxlen=%ld)!",
            fid, msg->maxlen
        );
        util_hexdmp(
            stdout, (unsigned char *) msg->data, (long) msg->maxlen, 0, 'x'
        );
        return FALSE;
    }

    if (msg->len < 0 || msg->len > msg->maxlen) {
        rtp_log(RTP_ERR, "ERROR: %s: msg->len=%ld < 0 or > %ld!",
            fid, msg->len, msg->maxlen
        );
        return FALSE;
    }

    return TRUE;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:50:56  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
