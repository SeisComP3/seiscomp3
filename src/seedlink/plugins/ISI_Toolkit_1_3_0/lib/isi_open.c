#pragma ident "$Id: open.c,v 1.6 2007/01/11 17:50:35 dechavez Exp $"
/*======================================================================
 *
 *  high level ISI connections
 *
 *====================================================================*/
#define INCLUDE_IACP_DEFAULT_ATTR
#include "isi.h"
#include "iacp.h"

static BOOL InitHandle(ISI *isi, char *server, int port, IACP_ATTR *attr, LOGIO *lp, int debug)
{
    isi->lp = lp;
    isi->debug = debug;
    isiInitDataRequest(&isi->datreq);
    isiInitIncoming(&isi->incoming);
    isi->port = port;
    isi->attr = *attr;
    isi->decompress = TRUE;
    if ((isi->ts1 = isiCreateGenericTS()) == NULL) return FALSE;
    if ((isi->ts2 = isiCreateGenericTS()) == NULL) return FALSE;
    return ((isi->server = strdup(server)) == NULL) ? FALSE : TRUE;
}

ISI *isiOpen(char *server, int port, IACP_ATTR *user_attr, LOGIO *lp, int debug)
{
ISI *isi;
IACP_ATTR attr;

    if (user_attr != NULL) {
        attr = *user_attr;
    } else {
        attr = IACP_DEFAULT_ATTR;
    }

    if ((isi = (ISI *) calloc(1, sizeof(ISI))) == NULL) return (ISI *) NULL;

    if (!InitHandle(isi, server, port, &attr, lp, debug)) return isiFree(isi);
    if ((isi->iacp = iacpOpen(server, port, user_attr, lp, debug)) == NULL) return isiFree(isi);

    return isi;

}

ISI *isiFree(ISI *isi)
{
    if (isi == NULL) return (ISI *) NULL;
    free(isi->server);
    if (isi->datreq.req.twind !=NULL) free(isi->datreq.req.twind);
    if (isi->datreq.req.seqno !=NULL) free(isi->datreq.req.seqno);
    listDestroy(&isi->incoming.list);
    isiDestroyGenericTS(isi->ts1);
    isiDestroyGenericTS(isi->ts2);
    free(isi);
    return (ISI *) NULL;
}

ISI *isiClose(ISI *isi)
{
    if (isi == NULL) return (ISI *) NULL;
    iacpClose(isi->iacp);
    return isiFree(isi);
}

/* Revision History
 *
 * $Log: open.c,v $
 * Revision 1.6  2007/01/11 17:50:35  dechavez
 * renamed all the "stream" requests to the more accurate "twind" (time window)
 *
 * Revision 1.5  2006/06/17 00:38:12  dechavez
 * calloc() handle instead of unitialized malloc()
 *
 * Revision 1.4  2005/06/24 21:40:08  dechavez
 * accomodate new design of ISI_DATA_REQUEST structure
 *
 * Revision 1.3  2004/06/04 22:49:43  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.2  2003/11/19 21:30:43  dechavez
 * added decompress, ts1, and ts2 fields to the handl
 *
 * Revision 1.1  2003/11/03 23:56:54  dechavez
 * Initial release
 *
 */
