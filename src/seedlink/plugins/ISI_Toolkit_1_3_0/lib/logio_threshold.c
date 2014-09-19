#pragma ident "$Id: threshold.c,v 1.7 2007/10/31 17:10:08 dechavez Exp $"
/*======================================================================
 *
 * Manipulate logging threshold
 *
 *====================================================================*/
#include "logio.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

int logioThreshold(LOGIO *lp)
{
int retval;

    if (lp == NULL) {
        errno = EINVAL;
        return -1;
    }

    MUTEX_LOCK(&lp->mutex);
        retval = lp->threshold;
    MUTEX_UNLOCK(&lp->mutex);

    return retval;
}

int logioSetThreshold(LOGIO *lp, int value)
{

    if (lp == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (logioIntToSeverityString(value) == NULL) return -1;
    MUTEX_LOCK(&lp->mutex);
        lp->threshold = value;
    MUTEX_UNLOCK(&lp->mutex);

    return value;
}

VOID logioSetPrefix(LOGIO *lp, CHAR *prefix)
{

    if (lp == NULL || prefix == NULL) return;

    MUTEX_LOCK(&lp->mutex);
        strlcpy(lp->prefix, prefix, LOGIO_MAX_PREFIX_LEN+1);
    MUTEX_UNLOCK(&lp->mutex);
}

BOOL logioIsAbsolutePath(CHAR *path)
{
    if (path == NULL) return FALSE;
    if (path[0] == '/') return TRUE;
    if (strcasecmp(path, "-") == 0) return TRUE;
    if (strcasecmp(path, "stdout") == 0) return TRUE;
    if (strcasecmp(path, "stderr") == 0) return TRUE;
    return FALSE;
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
 * $Log: threshold.c,v $
 * Revision 1.7  2007/10/31 17:10:08  dechavez
 * replaced string memcpy with strlcpy
 *
 * Revision 1.6  2006/08/15 00:58:56  dechavez
 * define min() macro if necessary
 *
 * Revision 1.5  2006/07/18 20:45:49  dechavez
 * copy no more than strlen(prefix) bytes in logioSetPrefix (aa)
 *
 * Revision 1.4  2006/05/17 23:25:11  dechavez
 * added copyright notice
 *
 * Revision 1.3  2005/07/26 18:39:10  dechavez
 * check for NULL prefix in logioSetPrefix()
 *
 * Revision 1.2  2003/11/19 23:42:42  dechavez
 * better argument checking
 *
 * Revision 1.1  2003/06/09 23:48:00  dechavez
 * initial release
 *
 */
