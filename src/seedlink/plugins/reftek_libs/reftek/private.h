#pragma ident "$Id: private.h 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Prototypes for revision specific functions.
 *
 * Revision history:
 		29Sep2005	(pld) add encode_steim2()
 						(pld) add parm to encode functions
 		03Sep2005	(pld) add decode_steim2()
 *====================================================================*/
#ifndef reftek_private_included
#define reftek_private_included

#include "reftek.h"
#include "steim.h"         /* Steim structures and constants */

#ifndef BIG_ENDIAN_HOST
#   define REVERSE_BYTE_ORDER
#   define LSWAP(ptr, count) util_lswap((UINT32 *) ptr, count)
#   define SSWAP(ptr, count) util_sswap((UINT16 *) ptr, count)
#else
#   undef  REVERSE_BYTE_ORDER
#   define LSWAP(ptr, count) 
#   define SSWAP(ptr, count) 
#endif /* BIG_ENDIAN_HOST */

VOID reftek_com(UINT8 *src, UINT16 *exp, UINT16 *unit, UINT16 *seqno, REAL64 *tstamp);
VOID reftek_dcomp(struct reftek_dt *dt);

INT16 encode_steim ( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev );
BOOL  decode_steim ( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev );

INT16 encode_steim2( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev );
BOOL  decode_steim2( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev );

#endif /* reftek_private_included */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:40  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.3  2005/09/03 21:52:31  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.2  2002/01/18 17:55:57  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
