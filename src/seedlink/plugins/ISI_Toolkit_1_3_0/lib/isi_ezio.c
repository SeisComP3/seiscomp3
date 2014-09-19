#pragma ident "$Id: ezio.c,v 1.16 2007/01/23 02:50:43 dechavez Exp $"
/*======================================================================
 *
 *  Functions to allow a very simple API to access ISI data
 *
 *====================================================================*/
#define INCLUDE_IACP_DEFAULT_ATTR
#include "isi.h"

BOOL isiInitDefaultPar(ISI_PARAM *par)
{

    if (par == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    par->port = ISI_DEFAULT_PORT;
    par->attr = IACP_DEFAULT_ATTR;
    par->debug = 0;
    par->lp = NULL;
    par->decompress = TRUE;

    return TRUE;
}

static ISI *OpenWithPar(char *server, ISI_PARAM *par)
{
ISI *isi;
ISI_PARAM defaultPar;

    if (server == NULL) server = ISI_DEFAULT_HOST;

    if (par == NULL) {
        par = &defaultPar;
        isiInitDefaultPar(par);
    }

    isi = isiOpen(server, par->port, &par->attr, par->lp, par->debug);

    if (isi == NULL) return NULL;

    isi->decompress = par->decompress;

    return isi;
}

ISI_PARAM *isiAllocDefaultParam(VOID)
{
ISI_PARAM *par;

    par = (ISI_PARAM *) malloc(sizeof(ISI_PARAM));
    if (par == NULL) return NULL;
    isiInitDefaultPar(par);
    return par;
}

VOID isiSetServerPort(ISI_PARAM *par, int port)
{
    if (par == NULL) return;
    par->port = port;
}

int isiGetServerPort(ISI_PARAM *par)
{
    if (par == NULL) {
        errno = EINVAL;
        return -1;
    }
    return par->port;
}

VOID isiSetTimeout(ISI_PARAM *par, int value)
{
    if (par == NULL) return;
    par->attr.at_timeo = value;
}

VOID isiSetTcpBuflen(ISI_PARAM *par, int sndbuf, int rcvbuf)
{
    if (par == NULL) return;
    par->attr.at_sndbuf = (UINT32) sndbuf;
    par->attr.at_rcvbuf = (UINT32) rcvbuf;
}

VOID isiSetRetryFlag(ISI_PARAM *par, BOOL value)
{
    if (par == NULL) return;
    par->attr.at_retry = value;
}

VOID isiSetRetryInterval(ISI_PARAM *par, int value)
{
    if (par == NULL) return;
    par->attr.at_wait = (UINT32) value;
}

VOID isiSetDebugFlag(ISI_PARAM *par, int value)
{
    if (par == NULL) return;
    par->debug = (UINT32) value;
}

BOOL isiSetDbgpath(ISI_PARAM *par, char *value)
{
    if (par == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if ((par->attr.at_dbgpath = strdup(value)) == NULL) return FALSE;
    return TRUE;
}

BOOL isiSetLog(ISI_PARAM *par, LOGIO *value)
{
    if (par == NULL) return TRUE;
    par->lp =  value;
    return TRUE;
}

BOOL isiStartLogging(ISI_PARAM *par, char *spec, VOID(*func) (char *string), char *pname)
{
    if (!logioInit(&par->logio, spec, func, pname)) return FALSE;
    par->lp = &par->logio;
    if (par->debug < 1) par->debug = 0;
    if (par->debug > 0) logioSetThreshold(par->lp, LOG_DEBUG);
    return TRUE;
}

ISI *isiInitiateDataRequest(char *server, ISI_PARAM *par, ISI_DATA_REQUEST *dreq)
{
ISI *isi;
UINT32 cause;
BOOL defaultDataRequest = FALSE;
static char *fid = "isiInitiateDataRequest";

    if ((isi = OpenWithPar(server, par)) == NULL) return NULL;

    if (dreq == NULL) {
        if ((dreq = isiAllocDefaultDataRequest()) == NULL) return NULL;
        defaultDataRequest = TRUE;
    }

    if (!isiSendDataRequest(isi, dreq, &cause)) {
        isiLogMsg(isi, LOG_INFO, "%s: isiSendDataRequest: %s", fid, strerror(errno));
        isiClose(isi);
        if (defaultDataRequest) isiFreeDataRequest(dreq);
        return NULL;
    }
#ifdef DEBUG
    printf("%s\n", fid);
    isiPrintDatreq(stdout, dreq);
#endif /* DEBUG */

    return isi;
}

ISI_SOH_REPORT *isiSoh(char *server, ISI_PARAM *par)
{
int i;
ISI *isi;
LNKLST *list;
ISI_SOH_REPORT *report;

    if ((isi = OpenWithPar(server, par)) == NULL) return NULL;
    list = isiRequestSoh(isi);
    isiClose(isi);

    if (list == NULL) return NULL;
    if (!listSetArrayView(list)) {
        listDestroy(list);
        return NULL;
    }

    report = (ISI_SOH_REPORT *) malloc(sizeof(ISI_SOH_REPORT));
    if (report == NULL) {
        listDestroy(list);
        return NULL;
    }
    report->entry = (ISI_STREAM_SOH *) malloc(list->count * sizeof(ISI_STREAM_SOH));
    if (report->entry == NULL) {
        free(report);
        listDestroy(list);
        return NULL;
    }

    for (i = 0; i < (int) list->count; i++) {
        memcpy(&report->entry[i], list->array[i], sizeof(ISI_STREAM_SOH));
    }
    report->nentry = list->count;

    listDestroy(list);

    return report;
}

VOID isiFreeSoh(ISI_SOH_REPORT *report)
{
    if (report == NULL) return;
    if (report->entry == NULL) return;
    free(report->entry);
    free(report);
}

ISI_CNF_REPORT *isiCnf(char *server, ISI_PARAM *par)
{
int i;
ISI *isi;
LNKLST *list;
ISI_CNF_REPORT *report;

    if ((isi = OpenWithPar(server, par)) == NULL) return NULL;
    list = isiRequestCnf(isi);
    isiClose(isi);

    if (list == NULL) return NULL;
    if (!listSetArrayView(list)) {
        listDestroy(list);
        return NULL;
    }

    report = (ISI_CNF_REPORT *) malloc(sizeof(ISI_CNF_REPORT));
    if (report == NULL) {
        listDestroy(list);
        return NULL;
    }
    report->entry = (ISI_STREAM_CNF *) malloc(list->count * sizeof(ISI_STREAM_CNF));
    if (report->entry == NULL) {
        free(report);
        listDestroy(list);
        return NULL;
    }

    for (i = 0; i < (int) list->count; i++) {
        memcpy(&report->entry[i], list->array[i], sizeof(ISI_STREAM_CNF));
    }
    report->nentry = list->count;

    listDestroy(list);

    return report;
}

VOID isiFreeCnf(ISI_CNF_REPORT *report)
{
    if (report == NULL) return;
    if (report->entry == NULL) return;
    free(report->entry);
    free(report);
}

ISI_WFDISC_REPORT *isiWfdisc(char *server, ISI_PARAM *par, int maxdur)
{
int i;
ISI *isi;
LNKLST *list;
ISI_WFDISC_REPORT *report;

    if ((isi = OpenWithPar(server, par)) == NULL) return NULL;
    list = isiRequestWfdisc(isi, maxdur);
    isiClose(isi);

    if (list == NULL) return NULL;
    if (!listSetArrayView(list)) {
        listDestroy(list);
        return NULL;
    }

    report = (ISI_WFDISC_REPORT *) malloc(sizeof(ISI_WFDISC_REPORT));
    if (report == NULL) {
        listDestroy(list);
        return NULL;
    }
    report->entry = (char **) malloc(list->count * sizeof(char *));
    if (report->entry == NULL) {
        free(report);
        listDestroy(list);
        return NULL;
    }

    for (i = 0; i < (int) list->count; i++) report->entry[i] = NULL;

    for (i = 0; i < (int) list->count; i++) {
        if ((report->entry[i] = strdup((char *) list->array[i])) == NULL) {
            listDestroy(list);
            for (i = 0; i < (int) list->count; i++) if (report->entry[i] != NULL) free(report->entry[i]);
            free(report->entry);
            free(report);
            return NULL;
        }
    }
    report->nentry = list->count;

    return report;
}

VOID isiFreeWfdisc(ISI_WFDISC_REPORT *report)
{
int i;

    if (report == NULL) return;
    if (report->entry == NULL) return;
    for (i = 0; i < (int) report->nentry; i++) free(report->entry[i]);
    free(report->entry);
    free(report);
}

/* Revision History
 *
 * $Log: ezio.c,v $
 * Revision 1.16  2007/01/23 02:50:43  dechavez
 * changed LOG_ERR messages to LOG_INFO
 *
 * Revision 1.15  2006/06/26 22:36:01  dechavez
 * removed unreferenced local variables, added missing return value to isiSetLog
 *
 * Revision 1.14  2006/02/14 17:05:23  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.13  2005/06/10 15:50:29  dechavez
 * renamed isiSetLogging to isiStartLogging, added isiSetLog and replaced
 * log field in ISI_PARAM with lp
 *
 * Revision 1.12  2005/05/25 22:38:18  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.11  2005/01/28 01:49:45  dechavez
 * added isiSetDbgpath()
 *
 * Revision 1.10  2004/09/28 22:55:44  dechavez
 * added conditional DEBUG support
 *
 * Revision 1.9  2004/06/24 18:05:59  dechavez
 * fixed bug in isiWfdisc error handling
 *
 * Revision 1.8  2004/01/29 18:36:06  dechavez
 * added isiFreeSoh(), isiFreeCnf(), isiFreeWfdisc()
 *
 * Revision 1.7  2003/12/10 05:51:08  dechavez
 * cosmetic clean up
 *
 * Revision 1.6  2003/11/26 19:40:01  dechavez
 * set logging threshold to LOG_DEBUG when debugging flag is set in isiSetLogging()
 *
 * Revision 1.5  2003/11/25 20:36:07  dechavez
 * added isiGetServerPort()
 *
 * Revision 1.4  2003/11/19 23:51:00  dechavez
 * Fixed log initialization error in isiSetLogging()
 *
 * Revision 1.3  2003/11/19 21:32:22  dechavez
 * added support for decompress parameter, changed isiSoh(), isiCnf(), and
 * isiWfdisc() to return report structures
 *
 * Revision 1.2  2003/11/13 19:24:43  dechavez
 * replaced isiDataRequest() with isiInitiateDataRequest(), added isiSoh(),
 * isiCnf() and isiWfdisc()
 *
 * Revision 1.1  2003/11/03 23:55:11  dechavez
 * Initial release (nothing tested yet)
 *
 */
