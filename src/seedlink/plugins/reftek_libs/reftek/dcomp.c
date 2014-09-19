#pragma ident "$Id: dcomp.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Decompress a C0 compressed data record.  The input are taken from
 *  the structure's 'raw' buffer, and decompressed into the structures
 *  'dcmp' buffer.  If any errors are detected then the dcerr flag is
 *  set with the appropriate value.
 *
 *  Data are uncompressed into the host native byte order.
 *
 *====================================================================*/
#include "private.h"

#define NFRM 15           /* number of frames              */
#define FLEN 64           /* number of bytes per frame     */
#define NSEQ 16           /* number of sequences per frame */
#define WLEN FLEN / NSEQ  /* number of bytes per sequence  */

VOID reftek_dcomp(struct reftek_dt *dt)
{
INT32 i, j, k, beg, end, code[NSEQ], key, val, nsamp, itmp;
UINT16 stmp;
UINT8 *frm;
union {
    UINT8 *c;
    INT16 *s;
    INT32 *i;
} ptr;

    dt->dcerr = 0;

    if (sizeof(INT32) != WLEN) {
        dt->dcerr = 1;
        return;
    }

    if (dt->format != REFTEK_FC0) {
        dt->dcerr = 2;
        return;
    }

/* Get the block start/stop values */

    ptr.c = dt->raw;

    memcpy((void *) &beg, (void *) (ptr.c + 4), (size_t) 4); LSWAP(&beg, 1);
    memcpy((void *) &end, (void *) (ptr.c + 8), (size_t) 4); LSWAP(&end, 1);

/* Loop over each frame */
/* We do not verify that the 0x00 codes are where they should be */

    val   = dt->dcmp[0] = beg;
    nsamp = 1;

    for (i = 0; i < NFRM; i++) {

        frm = dt->raw + (i * FLEN);  /* point to start of current frame */
        key = *((int *) frm);        /* codes are in first 4 bytes      */
        LSWAP(&key, 1);
        for (j = NSEQ - 1; j >= 0; j--) {
            code[j] = key & 0x03;
            key >>= 2;
        }

        for (j = 1; j < NSEQ; j++) {

            if (nsamp >= REFTEK_MAXC0) {
                dt->dcerr = 3;
                return;
            }

            ptr.c = frm + (j * 4);  /* point to current 4 byte sequence */

            switch (code[j]) {
              case 0:
                break;

              case 1:
                for (k = (nsamp == 1) ? 1 : 0; k < 4; k++) {
                    dt->dcmp[nsamp++] = (val += (int) ptr.c[k]);
                }
                break;

              case 2:
                for (k = (nsamp == 1) ? 1 : 0; k < 2; k++) {
                    stmp = ptr.s[k];
                    SSWAP(&stmp, 1);
                    dt->dcmp[nsamp++] = (val += (int) stmp);
                }
                break;

              case 3:
                if (nsamp > 1) {
                    itmp = ptr.i[0];
                    LSWAP(&itmp, 1);
                    dt->dcmp[nsamp++] = (val += itmp);
                }
                break;

              default:
                dt->dcerr = 4;
                return;
            }
        }
    }

/* Update the dt header to point to uncompressed data */

    dt->data = dt->dcmp;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:55:55  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
