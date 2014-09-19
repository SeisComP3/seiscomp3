#pragma ident "$Id: msg.c,v 1.10 2007/09/14 19:31:38 dechavez Exp $"
/*======================================================================
 *
 * Log a message string
 *
 *====================================================================*/
#include "logio.h"

FILE *logioOpenLogFile(LOGIO *lp)
{
    if (strcasecmp(lp->path, "stderr") == 0) return stderr;
    if (strcasecmp(lp->path, "stdout") == 0) return stdout;
    if (strcasecmp(lp->path, "-") == 0)      return stdout;
    return fopen(lp->path, "a+");
}

#define TSLEN 64
#define PID_STR_LEN 64
#define PIDAMBLE_LEN LOGIO_MAX_PREFIX_LEN + LOGIO_MAX_PNAME_LEN + PID_STR_LEN + 3
#define TMPBUF_LEN   (PIDAMBLE_LEN + LOGIO_MAX_MSG_LEN)

static void DeliverMessage(LOGIO *lp, int level)
{
FILE *fp;
time_t now;
char timestamp[TSLEN];
char pidamble[PIDAMBLE_LEN];
#ifdef HAVE_SYSLOG
char tmpbuf[TMPBUF_LEN];
#endif /* HAVE_SYSLOG */
#if defined LINUX || defined BSD
static char *fmtstr = "[%d:%x]";
#else
static char *fmtstr = "[%d:%u]";
#endif

/* using user supplied function */

    if (lp->func != NULL) {
        (*lp->func)(lp->msg);
        return;
    }

/* build pidamble */

    pidamble[0] = 0;
    if (strlen(lp->prefix) != 0) snprintf(pidamble+strlen(pidamble), PIDAMBLE_LEN-strlen(pidamble), "%s ", lp->prefix);
    if (strlen(lp->pname) != 0) snprintf(pidamble+strlen(pidamble), PIDAMBLE_LEN-strlen(pidamble), "%s", lp->pname);
    snprintf(pidamble+strlen(pidamble), PIDAMBLE_LEN-strlen(pidamble), fmtstr, (int) getpid(), THREAD_SELF());

/* using syslog */

    if (lp->syslog) {
#ifdef HAVE_SYSLOG
        snprintf(tmpbuf, TMPBUF_LEN, "%s %s", pidamble, lp->msg);
        syslog(level, tmpbuf);
#endif /* HAVE_SYSLOG */
        return;
    }

/* logging to a file */

    if ((fp = logioOpenLogFile(lp)) == NULL) return;

/* timestamp */

    now = time(NULL);
    if (strftime(timestamp, TSLEN, lp->tfmt, localtime(&now))) fprintf(fp, "%s ", timestamp);

/* pidamble */

    fprintf(fp, "%s ", pidamble);

/* message */

    fprintf(fp, "%s\n", lp->msg);
    logioCloseFile(fp);
}

VOID logioMsg(LOGIO *lp, int level, CHAR *format, ...)
{
int i, maxlen;
va_list marker;
CHAR *ptr;
LOGIO DefaultLogio;

/* NULL handle OK, we'll just dump to stdout */

    if (lp == NULL) {
        logioInit(&DefaultLogio, "stdout", NULL, "logioMsg");
        lp = &DefaultLogio;
     }

/* Ignore the message if it is beyond our threshold */

    if (level > logioThreshold(lp)) return;

/* Build and deliver the message string */

    MUTEX_LOCK(&lp->mutex);
        memset((void *) lp->msg, 0, LOGIO_MAX_MSG_LEN);

        ptr = lp->msg+strlen(lp->msg);
        maxlen = LOGIO_MAX_MSG_LEN+1 - strlen(lp->msg);
        va_start(marker, format);
        vsnprintf(ptr, maxlen, format, marker);
        va_end(marker);

        for (i = strlen(lp->msg) - 1; i >= 0 && lp->msg[i] == '\n'; i--) lp->msg[i] = 0;
        DeliverMessage(lp, level);
    MUTEX_UNLOCK(&lp->mutex);
}

/*-----------------------------------------------------------------------+
 |                                                                       |
 | Copyright (C) 2006 Regents of the University of California            |
 |                                                                       |
 | This software is provided 'as-is', without any express or implied     |
 | warranty.  In no event will the authors be held liable for any        |
 | damages arising from the use of this software.                        |
 |                                                                       |
 | Permission is granted to anyone to use this software for any purpose, |
 | including commercial applications, and to alter it and redistribute   |
 | it freely, subject to the following restrictions:                     |
 |                                                                       |
 | 1. The origin of this software must not be misrepresented; you must   |
 |    not claim that you wrote the original software. If you use this    |
 |    software in a product, an acknowledgment in the product            |
 |    documentation of the contribution by Project IDA, UCSD would be    |
 |    appreciated but is not required.                                   |
 | 2. Altered source versions must be plainly marked as such, and must   |
 |    not be misrepresented as being the original software.              |
 | 3. This notice may not be removed or altered from any source          |
 |    distribution.                                                      |
 |                                                                       |
 +-----------------------------------------------------------------------*/

/* Revision History
 *
 * $Log: msg.c,v $
 * Revision 1.10  2007/09/14 19:31:38  dechavez
 * eliminated extra prefix
 *
 * Revision 1.9  2007/01/10 20:21:35  dechavez
 * fixed 2.2.4 bug causing messages to be truncated after preamble
 *
 * Revision 1.8  2007/01/07 18:11:20  dechavez
 * snprintf instead of sprintf(), BSD builds print hex tid
 *
 * Revision 1.7  2006/05/20 01:32:34  dechavez
 * print Linux thread id's as hex since they can be so huge
 *
 * Revision 1.6  2006/05/17 23:25:11  dechavez
 * added copyright notice
 *
 * Revision 1.5  2005/10/17 21:16:53  dechavez
 * Fixed LOG_ERR and LOG_WARN filter that was here, not is syslogd (duh).  So, syslog msg's
 * back to being logged at specified level again
 *
 * Revision 1.4  2005/10/14 01:33:38  dechavez
 * syslog messages all logged at LOG_INFO, since LOG_ERR was getting filtered by syslogd
 *
 * Revision 1.3  2005/07/26 00:34:57  dechavez
 * log to stdout if called with uninitialized handle
 *
 * Revision 1.2  2003/12/22 18:45:51  dechavez
 * include pname[pid:tid] in syslog messages
 *
 * Revision 1.1  2003/06/09 23:48:00  dechavez
 * initial release
 *
 */
