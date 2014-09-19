#pragma ident "$Id: init.c,v 1.10 2007/10/31 17:10:08 dechavez Exp $"
/*======================================================================
 *
 * Initialize the logging facility.
 *
 * The caller must specify either spec or func, leaving the other NULL.
 *
 * spec = pathname for logging to a file
 *        "-" or "stdout" for logging to stdout
 *        "stderr" for logging to stderr
 *        "syslog:FACILITY" for logging via syslog using specified 
 *        facility
 *
 * func = pointer to user supplied function for delivering the 
 *        message string: void func(char *string)
 *
 * pname = name of calling process
 *
 * Returns a TRUE if successful, otherwise FALSE.
 *
 * Examples:
 *
 * LOGIO lp; // user provides handle space
 *
 * To log to stdout:  status = logioInit(&lp, "-", NULL, argv[0]);
 * To log via syslog: status = logioInit(&lp, "syslog:LOG_LOCAL0", NULL, argv[0]);
 * To log via user supplied function:
 *                     void myfunc(char *string)
 *                     {
 *                           printf("%s\n", string);
 *                     }
 *
 *                     status = logioInit(&lp, NULL, myfunc, argv[0]);
 *
 *====================================================================*/
#include "logio.h"

#define SYSLOG_PREFIX "syslog"
#define SYSLOGD_PREFIX "syslogd"

static BOOL IsAbsolutePath(CHAR *path)
{
    if (path[0] == '/') return TRUE;
    if (strcasecmp(path, "-") == 0) return TRUE;
    if (strcasecmp(path, "stdout") == 0) return TRUE;
    if (strcasecmp(path, "stderr") == 0) return TRUE;
    return FALSE;
}

static BOOL Fail(LOGIO *lp)
{
    free(lp);
    return FALSE;
}

#ifdef HAVE_SYSLOG
static BOOL UseSyslog(LOGIO *lp, CHAR *facilityString)
{
#ifndef DEFAULT_FACILITY
#define DEFAULT_FACILITY LOG_USER
#endif /* !DEFAULT_FACILITY */
int facility;

    lp->syslog = TRUE;
    if (strlen(facilityString) == 0) {
        facility = DEFAULT_FACILITY;
    } else if ((facility = logioFacilityStringToInt(facilityString)) < 0) {
        errno = EINVAL;
        return Fail(lp);
    }
    openlog(NULL, LOG_CONS, facility);

    return TRUE;
}

#else

static BOOL UseSyslog(LOGIO *lp, CHAR *facilityString)
{
    errno = EINVAL;
    return Fail(lp);
}

#endif /* !HAVE_SYSLOG */

static BOOL UseFile(LOGIO *lp, CHAR *path)
{
FILE *fp;
char cwd[MAXPATHLEN+1];

    strlcpy(lp->tfmt, LOGIO_DEF_TFMT, LOGIO_MAX_TFMT_LEN+1);

    if (IsAbsolutePath(path)) {
        strlcpy(lp->path, path, MAXPATHLEN+1);
    } else {
        if (getcwd(cwd, MAXPATHLEN) == NULL) return Fail(lp);
        snprintf(lp->path, MAXPATHLEN+1, "%s/%s", cwd, path);
    }

    if ((fp = logioOpenLogFile(lp)) == (FILE *) NULL) return FALSE;
    logioCloseFile(fp);

    return TRUE;
}

static BOOL UseSpec(LOGIO *lp, CHAR *spec)
{
    if (strncasecmp(spec, SYSLOGD_PREFIX, strlen(SYSLOGD_PREFIX)) == 0) {
        return UseSyslog(lp, spec+1+strlen(SYSLOGD_PREFIX));
    } else if (strncasecmp(spec, SYSLOG_PREFIX, strlen(SYSLOG_PREFIX)) == 0) {
        return UseSyslog(lp, spec+1+strlen(SYSLOG_PREFIX));
    } else {
        return UseFile(lp, spec);
    }
}

static BOOL UseFunc(LOGIO *lp, VOID(*func) (CHAR *string))
{
    lp->syslog = FALSE;
    lp->func   = func;
    return TRUE;
}

BOOL logioInit(LOGIO *lp, CHAR *spec, VOID(*func) (CHAR *string), CHAR *pname)
{
static char *DefaultSpec = "-";


/* NULL lp is OK, just means we won't do anything */

    if (lp == NULL) return TRUE;

/* Must specify either spec OR func */

    if (spec != NULL && func != NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (spec == NULL && func == NULL) spec = DefaultSpec;

    lp->syslog = FALSE;
    lp->func   = NULL;
    memset(lp->path, 0, MAXPATHLEN + 1);
    MUTEX_INIT(&lp->mutex);

    if (pname != NULL) {
        strlcpy(lp->pname, pname, LOGIO_MAX_PNAME_LEN+1);
    } else {
        lp->pname[0] = 0;
    }

    gethostname(lp->hostname, LOGIO_MAX_HOSTNAME_LEN);
    lp->prefix[0] = 0;
    lp->threshold = LOG_INFO;

    return (spec != NULL) ? UseSpec(lp, spec) : UseFunc(lp, func);
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
 * $Log: init.c,v $
 * Revision 1.10  2007/10/31 17:10:08  dechavez
 * replaced string memcpy with strlcpy
 *
 * Revision 1.9  2007/05/01 18:19:12  dechavez
 * fixed bug parsing facility from syslogd:facility specifiers
 *
 * Revision 1.8  2007/04/18 23:06:13  dechavez
 * fixed bug resulting in spurious "syslog" files lying about when facility was
 * not explicilty specified by user (LOG_USER is now the default facility)
 *
 * Revision 1.7  2007/01/07 18:10:58  dechavez
 * snprintf() instead of sprintf()
 *
 * Revision 1.6  2006/05/17 23:25:11  dechavez
 * added copyright notice
 *
 * Revision 1.5  2006/02/09 19:27:46  dechavez
 * permit NULL lp in logioInit()
 *
 * Revision 1.4  2005/05/25 22:52:13  dechavez
 * fixed up Win32/Unix usage of UseSyslog
 *
 * Revision 1.3  2005/05/25 22:38:39  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.2  2003/12/22 20:15:32  dechavez
 * allow specifying syslog with syslogd:facility
 *
 * Revision 1.1  2003/06/30 18:24:35  dechavez
 * initial release
 *
 */
