#pragma ident "$Id: log.c,v 1.2 2007/01/08 15:58:18 dechavez Exp $"
/*======================================================================
 *
 *  ISI library logging 
 *
 *====================================================================*/
#include "isi.h"
#include "util.h"

VOID isiLogMsgLevel(ISI *isi, int level)
{
    logioSetThreshold(isi->lp, level);
}

VOID isiLogMsg(ISI *isi, int level, char *format, ...)
{
va_list marker;
char *ptr, msgbuf[LOGIO_MAX_MSG_LEN];

    ptr = msgbuf;
    va_start(marker, format);
    vsnprintf(ptr, LOGIO_MAX_MSG_LEN, format, marker);
    va_end(marker);
    
    logioMsg(isi->lp, level, msgbuf);
}

/* Revision History
 *
 * $Log: log.c,v $
 * Revision 1.2  2007/01/08 15:58:18  dechavez
 * switch to size-bounded string operations
 *
 * Revision 1.1  2003/11/03 23:57:54  dechavez
 * Initial release
 *
 */
