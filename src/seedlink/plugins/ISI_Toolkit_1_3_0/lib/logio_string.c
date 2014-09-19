#pragma ident "$Id: string.c,v 1.3 2006/05/17 23:25:11 dechavez Exp $"
/*======================================================================
 *
 * String/contstant conversions
 *
 *====================================================================*/
#include "logio.h"

typedef struct {
    int value;
    char *text;
} LOGIO_MAP;

static LOGIO_MAP FacilityMap[] = {
    { LOG_KERN,   "KERN"   },
    { LOG_USER,   "USER"   },
    { LOG_MAIL,   "MAIL"   },
    { LOG_DAEMON, "DAEMON" },
    { LOG_AUTH,   "AUTH"   },
    { LOG_LPR,    "LPR"    },
    { LOG_NEWS,   "NEWS"   },
    { LOG_UUCP,   "UUCP"   },
    { LOG_CRON,   "CRON"   },
    { LOG_LOCAL0, "LOCAL0" },
    { LOG_LOCAL1, "LOCAL1" },
    { LOG_LOCAL2, "LOCAL2" },
    { LOG_LOCAL3, "LOCAL3" },
    { LOG_LOCAL4, "LOCAL4" },
    { LOG_LOCAL5, "LOCAL5" },
    { LOG_LOCAL6, "LOCAL6" },
    { LOG_LOCAL7, "LOCAL7" },
    { LOG_KERN,   "LOG_KERN"   },
    { LOG_USER,   "LOG_USER"   },
    { LOG_MAIL,   "LOG_MAIL"   },
    { LOG_DAEMON, "LOG_DAEMON" },
    { LOG_AUTH,   "LOG_AUTH"   },
    { LOG_LPR,    "LOG_LPR"    },
    { LOG_NEWS,   "LOG_NEWS"   },
    { LOG_UUCP,   "LOG_UUCP"   },
    { LOG_CRON,   "LOG_CRON"   },
    { LOG_LOCAL0, "LOG_LOCAL0" },
    { LOG_LOCAL1, "LOG_LOCAL1" },
    { LOG_LOCAL2, "LOG_LOCAL2" },
    { LOG_LOCAL3, "LOG_LOCAL3" },
    { LOG_LOCAL4, "LOG_LOCAL4" },
    { LOG_LOCAL5, "LOG_LOCAL5" },
    { LOG_LOCAL6, "LOG_LOCAL6" },
    { LOG_LOCAL7, "LOG_LOCAL7" },
    { -1,          NULL        }
};

static LOGIO_MAP SeverityMap[] = {
    { LOG_EMERG,   "EMERG"   },
    { LOG_ALERT,   "ALERT"   },
    { LOG_CRIT,    "CRIT"    },
    { LOG_ERR,     "ERR"     },
    { LOG_WARNING, "WARNING" },
    { LOG_NOTICE,  "NOTICE"  },
    { LOG_INFO,    "INFO"    },
    { LOG_DEBUG,   "DEBUG"   },
    { LOG_EMERG,   "LOG_EMERG"   },
    { LOG_ALERT,   "LOG_ALERT"   },
    { LOG_CRIT,    "LOG_CRIT"    },
    { LOG_ERR,     "LOG_ERR"     },
    { LOG_WARNING, "LOG_WARNING" },
    { LOG_NOTICE,  "LOG_NOTICE"  },
    { LOG_INFO,    "LOG_INFO"    },
    { LOG_DEBUG,   "LOG_DEBUG"   },
    { -1,           NULL         }
};

int logioFacilityStringToInt(char *text)
{
int i;

    for (i = 0; FacilityMap[i].text != NULL; i++) {
        if (strcasecmp(text, FacilityMap[i].text) == 0) return FacilityMap[i].value;
    }

    return -1;
}

char *logioIntToFacilityString(int value)
{
int i;

    for (i = 0; FacilityMap[i].value !=-1; i++) {
        if (FacilityMap[i].value == value) return FacilityMap[i].text;
    }

    return NULL;
}

int logioSeverityStringToInt(char *text)
{
int i;

    for (i = 0; SeverityMap[i].text != NULL; i++) {
        if (strcasecmp(text, SeverityMap[i].text) == 0) return SeverityMap[i].value;
    }

    return -1;
}

char *logioIntToSeverityString(int value)
{
int i;

    for (i = 0; SeverityMap[i].value !=-1; i++) {
        if (SeverityMap[i].value == value) return SeverityMap[i].text;
    }

    return NULL;
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
 * $Log: string.c,v $
 * Revision 1.3  2006/05/17 23:25:11  dechavez
 * added copyright notice
 *
 * Revision 1.2  2003/11/04 22:06:34  dechavez
 * allow for tags w/o the log_ prefix
 *
 * Revision 1.1  2003/06/09 23:48:00  dechavez
 * initial release
 *
 */
