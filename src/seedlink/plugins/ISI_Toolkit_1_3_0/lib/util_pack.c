#pragma ident "$Id: pack.c,v 1.6 2007/10/17 18:33:48 dechavez Exp $"
/*======================================================================
 *
 * Pack/unpack various things for network I/O.
 *
 *====================================================================*/
#include "util.h"

/* Pack/Unpack bytes */

int utilPackBytes(UINT8 *ptr, UINT8 *string, int length)
{
    memcpy(ptr, string, length);
    return length;
}

int utilUnpackBytes(UINT8 *ptr, UINT8 *string, int length)
{
    memcpy(string, ptr, length);
    return length;
}

/* Pack/Unpack 16 bit integer values */

int utilPackUINT16(UINT8 *ptr, UINT16 value)
{
UINT16 sval;

    sval = htons(value);
    memcpy(ptr, &sval, 2);

    return 2;
}

int utilUnpackUINT16(UINT8 *ptr, UINT16 *out)
{
UINT16 sval;

    memcpy(&sval, ptr, 2);
    *out = ntohs(sval);

    return 2;
}

int utilPackINT16(UINT8 *ptr, INT16 value)
{
UINT16 sval;

    sval = htons((UINT16) value);
    memcpy(ptr, &sval, 2);

    return 2;
}

int utilUnpackINT16(UINT8 *ptr, INT16 *out)
{
UINT16 sval;

    memcpy(&sval, ptr, 2);
    *out = (INT16) ntohs(sval);

    return 2;
}

/* Pack/Unpack 32 bit integer values */

int utilPackUINT32(UINT8 *ptr, UINT32 value)
{
UINT32 lval;

    lval = htonl(value);
    memcpy(ptr, &lval, 4);

    return 4;
}

int utilUnpackUINT32(UINT8 *ptr, UINT32 *out)
{
UINT32 lval;

    memcpy(&lval, ptr, 4);
    *out = ntohl(lval);

    return 4;
}

int utilPackINT32(UINT8 *ptr, INT32 value)
{
UINT32 lval;

    lval = htonl((UINT32) value);
    memcpy(ptr, &lval, 4);

    return 4;
}

int utilUnpackINT32(UINT8 *ptr, INT32 *out)
{
UINT32 lval;

    memcpy(&lval, ptr, 4);
    *out = (INT32) ntohl(lval);

    return 4;
}

/* Pack/Unpack 64 bit ARM double precision values */

static int SwapArmREAL64(UINT8 *src, UINT8 *dest)
{
    // First 4 bytes contain swapped exponent/20 bit mantissa
    dest[0] = src[3];
    dest[1] = src[2];
    dest[2] = src[1];
    dest[3] = src[0];

    // Last 4 bytes contain swapped 32 bit mantissa
    dest[4] = src[7];
    dest[5] = src[6];
    dest[6] = src[5];
    dest[7] = src[4];

    return 8;
}

/* Pack/Unpack 64 bit integer values */

int utilPackUINT64(UINT8 *ptr, UINT64 value)
{
UINT64 lval, *ptr64;

#ifdef LTL_ENDIAN_HOST
    lval = value;
    ptr64 = &lval;
    utilSwapINT64(ptr64, 1);
#else
    ptr64 = &value;
#endif

    memcpy(ptr, ptr64, 8);

    return 8;
}

int utilUnpackUINT64(UINT8 *ptr, UINT64 *out)
{
UINT64 lval;

    memcpy(&lval, ptr, 8);
#ifdef LTL_ENDIAN_HOST
    utilSwapINT64(&lval, 1);
#endif
    *out = lval;

    return 8;
}

int utilPackINT64(UINT8 *ptr, INT64 value)
{
INT64 lval, *ptr64;

#ifdef LTL_ENDIAN_HOST
    lval = value;
    ptr64 = &lval;
    utilSwapINT64((UINT64 *) ptr, 1);
#else
    ptr64 = &value;
#endif

    memcpy(ptr, ptr64, 8);

    return 8;
}

int utilUnpackINT64(UINT8 *ptr, INT64 *out)
{
INT64 lval;

    memcpy(&lval, ptr, 8);
#ifdef LTL_ENDIAN_HOST
    utilSwapINT64((UINT64 *) &lval, 1);
#endif
    *out = lval;

    return 8;
}

/* Pack/Unpack 32 bit float values */

int utilPackREAL32(UINT8 *ptr, REAL32 value)
{
union {
    REAL32 f;
    UINT32 l;
} val;
UINT32 lval;

    val.f = value;
    lval  = htonl(val.l);
    memcpy(ptr, &lval, 4);

    return 4;

}

int utilUnpackREAL32(UINT8 *ptr, REAL32 *out)
{
union {
    REAL32 f;
    UINT32 l;
} val;
UINT32 lval;

    memcpy(&lval, ptr, 4);
    val.l = htonl(lval);
    *out = val.f;

    return 4;
}

/* Pack/Unpack 64 bit float values */

int utilPackREAL64(UINT8 *ptr, REAL64 value)
{
union {
    REAL64 f;
    UINT64 l;
} val;

    val.f = value;

#ifndef ARM_SLATE
    return utilPackUINT64(ptr, val.l);
#else
    return SwapArmREAL64((UINT8 *)&val.f, ptr);
#endif
}

int utilUnpackREAL64(UINT8 *ptr, REAL64 *out)
{
union {
    REAL64 f;
    UINT64 l;
} val;

#ifndef ARM_SLATE
    utilUnpackUINT64(ptr, &val.l);
#else
    SwapArmREAL64(ptr, (UINT8 *)&val.f);
#endif
    *out = val.f;

    return 8;
}

/* Revision History
 *
 * $Log: pack.c,v $
 * Revision 1.6  2007/10/17 18:33:48  dechavez
 * added support for Slate ARM5 cpu REAL64s (fshelly)
 *
 * Revision 1.5  2003/10/16 15:47:47  dechavez
 * added utilPackREAL64(), utilUnpackREAL64(), fixed case sensitivity error in
 * various function names revealed for LTL_ENDIAN_HOST builds
 *
 * Revision 1.4  2003/06/09 23:52:23  dechavez
 * added 64-bit integers
 *
 * Revision 1.3  2003/05/23 19:47:37  dechavez
 * changed naming style to match type (eg, INT16 instead of Int16)
 *
 * Revision 1.2  2001/05/07 22:39:11  dec
 * added type sensitive pack/unpack routines for UINT16, INT16, UINT32, INT32
 * (removes the need to cast signed values in the original version).
 *
 * Revision 1.1.1.1  2000/02/08 20:20:41  dec
 * import existing IDA/NRTS sources
 *
 */
