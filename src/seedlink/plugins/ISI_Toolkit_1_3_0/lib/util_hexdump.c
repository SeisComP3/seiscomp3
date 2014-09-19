#pragma ident "$Id: hexdump.c,v 1.1 2003/11/13 19:39:36 dechavez Exp $"
/*======================================================================
 *
 * hex dump
 *
 *====================================================================*/
#include "util.h"

#define BYTES_PER_LINE 16
#define PRINTCHAR(c) ((isascii(c) && isprint(c)) ? c : '.')

VOID utilPrintHexDump(FILE *fp, UINT8 *ptr, int count)
{
int whole_lines, num_last, line, byte, base, offset;
char buf[512];

    num_last    = count % BYTES_PER_LINE;
    whole_lines = (count - num_last) / BYTES_PER_LINE;
    for (offset = line = 0; line < whole_lines; line++) {
        sprintf(buf, "%08x", offset);
        base = line*BYTES_PER_LINE;
        for (byte = 0; byte < BYTES_PER_LINE; byte++) {
            sprintf(buf+strlen(buf)," %02x",ptr[byte+base]);
        }
        sprintf(buf+strlen(buf), " | ");
        for (byte = 0; byte < BYTES_PER_LINE; byte++) {
            sprintf(buf+strlen(buf),"%c",PRINTCHAR(ptr[byte+base]));
        }
        offset += BYTES_PER_LINE;
        fprintf(fp, "%s\n", buf);
    }
    if (num_last != 0) {
        sprintf(buf, "%08x", offset);
        base = line*BYTES_PER_LINE;
        for (byte = 0; byte < num_last; byte++) {
            sprintf(buf+strlen(buf)," %02x",ptr[byte+base]);
        }
        for (byte = num_last; byte < BYTES_PER_LINE; byte++) {
            sprintf(buf+strlen(buf),"   ");
        }
        sprintf(buf+strlen(buf)," | ");
        for (byte = 0; byte < num_last; byte++) {
            sprintf(buf+strlen(buf),"%c",PRINTCHAR(ptr[byte+base]));
        }
        fprintf(fp, "%s\n", buf);
    }
}

/* Revision History
 *
 * $Log: hexdump.c,v $
 * Revision 1.1  2003/11/13 19:39:36  dechavez
 * initial release
 *
 */
