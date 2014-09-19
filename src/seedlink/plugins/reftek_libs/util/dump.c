#pragma ident "$Id: dump.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Pretty binary, hexadecimal, octal dumps.
 *
 *====================================================================*/
#include <ctype.h>
#include "util.h"

#define printchar(c) ((isascii(c) && isprint(c)) ? c : '.')

static CHAR hex_format[] = "%2x:";
static CHAR heX_format[] = "%2X:";
static CHAR dec_format[] = "%10d:";
static CHAR oct_format[] = "%12o:";

static VOID bin_print(FILE *fp, UINT8 value)
{
INT16 i;
UINT8 mask;

    for (i = 7; i >= 4; i--) {
        mask = 1 << i;
        fprintf(fp, "%c", mask & value ? '1' : '0');
    }
    fprintf(fp, " ");
    for (i = 3; i >= 0; i--) {
        mask = 1 << i;
        fprintf(fp, "%c", mask & value ? '1' : '0');
    }
    fprintf(fp, "   ");
}

/**********************************************************************/

VOID util_bindmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase)
{
INT32 whole_lines;
INT32 num_last;
INT32 line, byte, base;
CHAR *format;
INT32 bytes_per_line = 4;

    switch (obase) {
        case 'x':           format = hex_format; break;
        case 'X':           format = heX_format; break;
        case 'd': case 'D': format = dec_format; break;
        case 'o': case 'O': format = oct_format; break;
        default:            format = hex_format;
    }

    num_last    = count % bytes_per_line;
    whole_lines = (count - num_last) / bytes_per_line;
    for (line = 0; line < whole_lines; line++) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < bytes_per_line; byte++) {
            bin_print(fp, ptr[byte+base]);
        }
        off += bytes_per_line;
        fprintf(fp, "\n");
    }
    if (num_last != 0) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < num_last; byte++) {
            bin_print(fp, ptr[byte+base]);
        }
        fprintf(fp, "\n");
    }
}

/**********************************************************************/

VOID util_hexdmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase)
{
INT32 whole_lines;
INT32 num_last;
INT32 line, byte, base;
CHAR *format;
INT32 bytes_per_line = 16;

    switch (obase) {
        case 'x':           format = hex_format; break;
        case 'X':           format = heX_format; break;
        case 'd': case 'D': format = dec_format; break;
        case 'o': case 'O': format = oct_format; break;
        default:            format = hex_format;
    }

    num_last    = count % bytes_per_line;
    whole_lines = (count - num_last) / bytes_per_line;
    for (line = 0; line < whole_lines; line++) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < bytes_per_line; byte++) {
            fprintf(fp, " %02x",ptr[byte+base]);
        }
        fprintf(fp, " | ");
        for (byte = 0; byte < bytes_per_line; byte++) {
            fprintf(fp, "%c",printchar(ptr[byte+base]));
        }
        off += bytes_per_line;
        fprintf(fp, "\n");
    }
    if (num_last != 0) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < num_last; byte++) {
            fprintf(fp, " %02x",ptr[byte+base]);
        }
        for (byte = num_last; byte < bytes_per_line; byte++) {
            fprintf(fp, "   ");
        }
        fprintf(fp, " | ");
        for (byte = 0; byte < num_last; byte++) {
            fprintf(fp, "%c",printchar(ptr[byte+base]));
        }
        fprintf(fp, "\n");
    }
}

/**********************************************************************/

VOID util_octdmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase)
{
INT32 whole_lines;
INT32 num_last;
INT32 line, byte, base;
CHAR *format;
INT32 bytes_per_line = 8;

    switch (obase) {
        case 'x':           format = hex_format; break;
        case 'X':           format = heX_format; break;
        case 'd': case 'D': format = dec_format; break;
        case 'o': case 'O': format = oct_format; break;
        default:            format = hex_format;
    }

    num_last    = count % bytes_per_line;
    whole_lines = (count - num_last) / bytes_per_line;
    for (line = 0; line < whole_lines; line++) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < bytes_per_line; byte++) {
            fprintf(fp, "  %03o",ptr[byte+base]);
        }
        fprintf(fp, " | ");
        for (byte = 0; byte < bytes_per_line; byte++) {
            fprintf(fp, "%c",printchar(ptr[byte+base]));
        }
        off += bytes_per_line;
        fprintf(fp, "\n");
    }
    if (num_last != 0) {
        fprintf(fp, format, off);
        base = line*bytes_per_line;
        for (byte = 0; byte < num_last; byte++) {
            fprintf(fp, "  %03o",ptr[byte+base]);
        }
        for (byte = num_last; byte < bytes_per_line; byte++) {
            fprintf(fp, "     ");
        }
        fprintf(fp, " | ");
        for (byte = 0; byte < num_last; byte++) {
            fprintf(fp, "%c",printchar(ptr[byte+base]));
        }
        fprintf(fp, "\n");
    }
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
