#pragma ident "$Id: dt.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Decode a DT record
 *
	Revised  :
		29Sep05	(PLD) enable Steim2 decoding
		03Sep05  (pld) add initial support for Steim2 decoding
 *====================================================================*/
#include "private.h"

/* Offsets to the various pieces */

#define EVTN_OFF  16  /* event number           */
#define STRM_OFF  18  /* stream id              */
#define CHAN_OFF  19  /* channel id             */
#define NSMP_OFF  20  /* number of samples      */
#define FRMT_OFF  23  /* data format descriptor */
#define UDAT_OFF  24  /* uncompressed data      */
#define CDAT_OFF  64  /* uncompressed data      */

BOOL reftek_dt(struct reftek_dt *dest, UINT8 *src, BOOL convert)
{
UINT8 *dptr;
UINT16 i, ncopy;
INT16 *sbuf;

/* Load the common header */

    reftek_com(src, &dest->exp, &dest->unit, &dest->seqno, &dest->tstamp);

/* Load the record specific parts */

    dest->evtno  = (UINT16) utilBcdToUint32(src + EVTN_OFF, 4, 0); 
    dest->stream = (UINT16) utilBcdToUint32(src + STRM_OFF, 2, 0);
    dest->chan   = (UINT16) utilBcdToUint32(src + CHAN_OFF, 2, 0);
    dest->nsamp  = (UINT16) utilBcdToUint32(src + NSMP_OFF, 4, 0);

/* Figure out the data type */

    switch (utilBcdToUint32(src + FRMT_OFF, 2, 0)) {
      case 16:
        dest->format = REFTEK_F16;
        dptr = src + UDAT_OFF;
        ncopy = 1000;
        break;

      case 32:
        dest->format = REFTEK_F32;
        dptr = src + UDAT_OFF;
        ncopy = 1000;
        break;

      case 120:
        dest->format = REFTEK_FC0;
        dptr = src + CDAT_OFF;
        ncopy = 960;
        break;

      case 122:
        dest->format = REFTEK_FC2;
        dptr = src + CDAT_OFF;
        ncopy = 960;
        break;

      default:
        errno = EINVAL;
        return FALSE;
    }

/* Copy raw data */

    memcpy((void *) dest->raw, (void *) dptr, (size_t) ncopy);

/* Convert to host ordered INT32's if requested */

    if (convert) {
        switch (dest->format) {

          case REFTEK_F16:
            sbuf = (INT16 *) dest->raw;
            SSWAP((UINT16 *) sbuf, dest->nsamp);
            for (i = 0; i < dest->nsamp; i++) dest->dcmp[i] = (INT32) sbuf[i];
            dest->data  = dest->dcmp;
            dest->dcerr = FALSE;
            break;

          case REFTEK_F32:
            dest->data  = (INT32 *) dest->raw;
            dest->dcerr = FALSE;
            LSWAP(dest->data, dest->nsamp);
            break;

          case REFTEK_FC0:
            dest->dcerr = !decode_steim(
                (VOID *) src, (INT16 *) &dest->nsamp, dest->dcmp, &dest->dcmp_prv
            );
            dest->data = dest->dcmp;
            break;

          case REFTEK_FC2:
            dest->dcerr = !decode_steim2(
                (VOID *) src, (INT16 *) &dest->nsamp, dest->dcmp, &dest->dcmp_prv
            );
            dest->data = dest->dcmp;
            break;

          default:
            errno = EINVAL;
            return FALSE;
        }

    } else {
        dest->data = (INT32 *) NULL;
    }

    return TRUE;

}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:49  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.3  2005/09/03 21:52:30  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.2  2002/01/18 17:55:56  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
