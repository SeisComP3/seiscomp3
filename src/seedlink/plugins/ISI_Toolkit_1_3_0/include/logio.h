/* $Id: logio.h,v 1.5 2006/05/17 23:21:24 dechavez Exp $ */
#ifndef logio_h_included
#define logio_h_included

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_SYSLOG
#include <syslog.h>
#else
/* Facility */
#define LOG_KERN    0
#define LOG_USER    1
#define LOG_MAIL    2
#define LOG_DAEMON  3
#define LOG_AUTH    4
#define LOG_LPR     5
#define LOG_NEWS    6
#define LOG_UUCP    7
#define LOG_CRON    8
#define LOG_LOCAL0  9
#define LOG_LOCAL1 10
#define LOG_LOCAL2 11
#define LOG_LOCAL3 12
#define LOG_LOCAL4 13
#define LOG_LOCAL5 14
#define LOG_LOCAL6 15
#define LOG_LOCAL7 16
/* Level */
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but signification condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */
#endif /* HAVE_SYSLOG */

#define LOG_WARN LOG_WARNING

#define LOGIO_MAX_PREFIX_LEN 255
#define LOGIO_MAX_PNAME_LEN  255
#define LOGIO_MAX_MSG_LEN 1023
#define LOGIO_MAX_TFMT_LEN 63
#define LOGIO_MAX_HOSTNAME_LEN 127

#define LOGIO_DEF_TFMT "%Y:%j-%H:%M:%S"

typedef struct {
    MUTEX mutex;
    BOOL syslog;
    VOID(*func) (CHAR *string);
    CHAR path[MAXPATHLEN+1];
    CHAR prefix[LOGIO_MAX_PREFIX_LEN+1];
    CHAR pname[LOGIO_MAX_PNAME_LEN+1];
    CHAR msg[LOGIO_MAX_MSG_LEN+1];
    CHAR tfmt[LOGIO_MAX_TFMT_LEN+1];
    CHAR hostname[LOGIO_MAX_HOSTNAME_LEN+1];
    int  threshold;
} LOGIO;

/* macros */

#define logioCloseFile(fp) if (fp != stdout && fp != stderr) fclose(fp)

/* Function prototypes */

/* init.c */
BOOL logioInit(LOGIO *lp, CHAR *spec, VOID(*func) (CHAR *string), CHAR *pname);

/* dump.c */
VOID logioHexDump(LOGIO *lp, int level, UINT8 *ptr, int count);

/* msg.c */
FILE *logioOpenLogFile(LOGIO *lp);
VOID logioMsg(LOGIO *lp, int level, CHAR *format, ...);

/* string.c */
int logioFacilityStringToInt(CHAR *text);
CHAR *logioIntToFacilityString(int value);
int logioSeverityStringToInt(CHAR *text);
CHAR *logioIntToSeverityString(int value);

/* threshold.c */
int logioThreshold(LOGIO *lp);
int logioSetThreshold(LOGIO *lp, int value);

/* version.c */
CHAR *logioVersionString(VOID);
VERSION *logioVersion(VOID);

#ifdef __cplusplus
}
#endif


#endif

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
 * $Log: logio.h,v $
 * Revision 1.5  2006/05/17 23:21:24  dechavez
 * added copyright notice
 *
 * Revision 1.4  2004/06/25 18:34:57  dechavez
 * C++ compatibility
 *
 * Revision 1.3  2003/10/16 17:45:01  dechavez
 * updated prototype list
 *
 * Revision 1.2  2003/06/30 18:25:58  dechavez
 * updated prototypes
 *
 * Revision 1.1  2003/06/09 23:41:06  dechavez
 * initial release
 *
 */
