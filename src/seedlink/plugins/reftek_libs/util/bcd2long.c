#pragma ident "$Id: bcd2long.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * convert from BCD to a UINT32
 *
 * Modified from Bruce Crawford's bcd2int function.
 *
 *====================================================================*/
#include "util.h"

UINT32 utilBcdToUint32(UINT8 *input, UINT16 numDigits, UINT16 nibble)
{
int digit, decimal, sum;

    sum = 0;

    for (digit = 0 ; digit < numDigits; digit++)
    {
        if (nibble)
            decimal = *input & 0xF;
        else
            decimal = ((int) (*input & 0xF0)) >> 4;

        sum = sum * 10 + decimal;

        if (nibble)
            input++;

        nibble = ! nibble;
    }

    return (sum);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:51:44  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
