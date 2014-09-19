#pragma ident "$Id: sc.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Decode a SC packet
 *
 *====================================================================*/
#include "private.h"

BOOL reftek_sc(struct reftek_sc *dest, UINT8 *src)
{
REAL32 value, factor;
CHAR temp[16], *token[2];
UINT8 *off, *ptr;
UINT16 i, ndx;
INT32 ntoken;

/* Load the common header */

    reftek_com(src, &dest->exp, &dest->unit, &dest->seqno, &dest->tstamp);

/* Load the record specific parts */

    dest->nchan = 0;
    for (i = 0; i < 5; i++) {
        off = src + 202 + (i * 146);

        ptr = off + 0;
        memcpy((void *) temp, (void *) ptr, (size_t) 2);
        temp[2] = 0;
        util_strtrm(temp);
        if (strlen(temp) > 0) {
            ndx = dest->nchan;
            dest->chan[ndx].num = atoi(temp);

            ptr = off + 2;
            strncpy((char *) dest->chan[ndx].name, (char *) ptr, 10);
            dest->chan[ndx].name[10] = 0;
            util_strtrm(dest->chan[ndx].name);

            ptr = off + 70;
            strncpy((char *) temp, (char *) ptr, 4);
            temp[4] = 0;
            util_strtrm(temp);
            dest->chan[ndx].gain = (REAL32) atof(temp);

            ptr = off + 74;
            strncpy((char *) dest->chan[ndx].model, (char *) ptr, 12);
            dest->chan[ndx].model[12] = 0;
            util_strtrm(dest->chan[ndx].model);

            ptr = off + 86;
            strncpy((char *) dest->chan[ndx].sn, (char *) ptr, 12);
            dest->chan[ndx].sn[12] = 0;
            util_strtrm(dest->chan[ndx].sn);

            ptr = off + 138;
            strncpy((char *) temp, (char *) ptr, 8);
            temp[8] = 0;
            util_strtrm(temp);
            ntoken = util_parse(temp, token, " ", 2, 0);
            if (ntoken == 2) {
                value = (REAL32) atof(token[0]);
                if (strcasecmp(token[1], "mV") == 0) {
                    factor = 1000.0 * 65536.0;
                } else if (strcasecmp(token[1], "uV") == 0) {
                    factor = 1000000.0 * 65536.0;
                } else if (strcasecmp(token[1], "V") == 0) {
                    factor = 1.0 * 65536.0;
                } else {
                    factor = 1.0;
                    value  = -12345.0;
                }
            } else {
                factor = 1.0;
                value  = -12345.0;
            }
            dest->chan[ndx].scale = value / factor;

            ++dest->nchan;
        }
    }

    return TRUE;

}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:55:57  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
