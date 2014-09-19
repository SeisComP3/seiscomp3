#pragma ident "$Id: swap.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Byte-swapping utilities.
 *
 *  util_lswap:  byte swap an array of longs
 *  util_sswap:  byte swap an array of shorts
 *  util_iftovf: convert IEEE floats into VAX floats.
 *  util_vftoif: convert VAX floats into IEEE floats.
 *
 *====================================================================*/
#include "util.h"

#define MANTISSA_MASK ((unsigned long)(0x00FFFFFF))
#define MANTISSA_SIZE (24)

/**********************************************************************/

VOID util_lswap(UINT32 *input, INT32 number)
{
UINT16 s_temp[2];
UINT16 temp;
INT32 i;

    util_sswap((UINT16 *) input, number*2);
    for (i = 0; i < number; i++) {
        memcpy((void *) s_temp, (void *) (input + i), (size_t) 4);
        temp      = s_temp[0];
        s_temp[0] = s_temp[1];
        s_temp[1] = temp;
        memcpy(input + i, s_temp, 4);
    }
}

/**********************************************************************/

VOID util_sswap(UINT16 *input, INT32 number)
{
CHAR byte[2];
CHAR temp;
INT32 i;

    for (i = 0; i < number; i++) {
        memcpy((void *) byte, (void *) (input + i), (size_t) 2);
        temp = byte[0];
        byte[0] = byte[1];
        byte[1] = temp;
        memcpy(input + i, byte, 2);
    }
}

/**********************************************************************/

VOID util_iftovf(UINT32 *input, INT32 number)
{
UINT32 mantissa, exponent;
INT32 i;

#ifdef LTL_ENDIAN_HOST
    util_lswap((UINT32 *) input, number);
#endif

    for (i = 0; i < number; i++) {
        mantissa = input[i] & MANTISSA_MASK;
        exponent = (((input[i] >> MANTISSA_SIZE) + 1) << MANTISSA_SIZE);
        input[i] = mantissa | exponent;
    }
    util_sswap((UINT16 *)input, number*2);

#ifdef LTL_ENDIAN_HOST
    util_lswap((UINT32 *) input, number);
#endif

}

/**********************************************************************/

VOID util_vftoif(UINT32 *input, INT32 number)
{
UINT32 mantissa, exponent;
INT32 i;

#ifdef LTL_ENDIAN_HOST
    util_lswap((UINT32 *) input, number);
#endif

    for (i = 0; i < number; i++) {
        if (input[i] != 0) {
            mantissa = input[i] & MANTISSA_MASK;
            exponent = (((input[i]>>MANTISSA_SIZE)-1)<<MANTISSA_SIZE);
            input[i] = mantissa | exponent;
        }
    }


#ifdef LTL_ENDIAN_HOST
    util_lswap((UINT32 *) input, number);
#endif
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:51:45  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
