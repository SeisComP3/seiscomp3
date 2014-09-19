#pragma ident "$Id: dump.c,v 1.3 2007/01/07 18:10:58 dechavez Exp $"
/*======================================================================
 *
 *  Hex dumps to log output
 *
 *====================================================================*/
#include "logio.h"

#define BYTES_PER_LINE 16
#define PRINTCHAR(c) ((isascii(c) && isprint(c)) ? c : '.')

VOID logioHexDump(LOGIO *lp, int level, UINT8 *ptr, int count)
{
int whole_lines, num_last, line, byte, base, offset;
#define FIXED_BUFLEN 512
char buf[FIXED_BUFLEN];

    num_last    = count % BYTES_PER_LINE;
    whole_lines = (count - num_last) / BYTES_PER_LINE;
    for (offset = line = 0; line < whole_lines; line++) {
        snprintf(buf, FIXED_BUFLEN, "%08x", offset);
        base = line*BYTES_PER_LINE;
        for (byte = 0; byte < BYTES_PER_LINE; byte++) {
            snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), " %02x",ptr[byte+base]);
        }
        sprintf(buf+strlen(buf), " | ");
        for (byte = 0; byte < BYTES_PER_LINE; byte++) {
            snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), "%c",PRINTCHAR(ptr[byte+base]));
        }
        offset += BYTES_PER_LINE;
        logioMsg(lp, level, "%s", buf);
    }
    if (num_last != 0) {
        snprintf(buf, FIXED_BUFLEN, "%08x", offset);
        base = line*BYTES_PER_LINE;
        for (byte = 0; byte < num_last; byte++) {
            snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), " %02x",ptr[byte+base]);
        }
        for (byte = num_last; byte < BYTES_PER_LINE; byte++) {
            snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), "   ");
        }
        snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), " | ");
        for (byte = 0; byte < num_last; byte++) {
            snprintf(buf+strlen(buf), FIXED_BUFLEN-strlen(buf), "%c",PRINTCHAR(ptr[byte+base]));
        }
        logioMsg(lp, level, "%s", buf);
    }
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
 * $Log: dump.c,v $
 * Revision 1.3  2007/01/07 18:10:58  dechavez
 * snprintf() instead of sprintf()
 *
 * Revision 1.2  2006/05/17 23:25:11  dechavez
 * added copyright notice
 *
 * Revision 1.1  2003/10/16 16:57:51  dechavez
 * initial release
 *
 */
