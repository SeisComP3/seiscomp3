#pragma ident "$Id: serialize.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : serialize.c
 Purpose  : Serialized file I/O functions.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    : These functions serialize a structure into a packed, network
            byte order (big endian) stream of bytes which is read from,
            or written to, a file.
            Both functions return the number of bytes seen by the filesystem.
            They return VOID_UINT32 in the event of an error.
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#define _SERIALIZE_C
#include "archive.h"

/*---------------------------------------------------------------------*/
UINT32 SerializedWrite(VOID * str, TEMPLATE * temp, H_FILE file)
{
    UINT8 *ptr;
    UINT32 i, length, actual;

#ifdef LTL_ENDIAN_HOST
    UINT8 swap_buffer[8];
#endif

    ASSERT(str != NULL);
    ASSERT(temp != NULL);

    actual = 0;

    if(file == VOID_H_FILE)
		return actual;

    i = 0;
    while (temp[i].offset != VOID_UINT32) {

        ptr = (UINT8 *) str + temp[i].offset;

#ifdef LTL_ENDIAN_HOST
        if (temp[i].swap) {
            switch (temp[i].length) {
              case 2:
                swap_buffer[0] = ptr[1];
                swap_buffer[1] = ptr[0];
                ptr = swap_buffer;
                break;
              case 4:
                swap_buffer[0] = ptr[3];
                swap_buffer[1] = ptr[2];
                swap_buffer[2] = ptr[1];
                swap_buffer[3] = ptr[0];
                ptr = swap_buffer;
                break;
              case 8:
                swap_buffer[0] = ptr[7];
                swap_buffer[1] = ptr[6];
                swap_buffer[2] = ptr[5];
                swap_buffer[3] = ptr[4];
                swap_buffer[4] = ptr[3];
                swap_buffer[5] = ptr[2];
                swap_buffer[6] = ptr[1];
                swap_buffer[7] = ptr[0];
                ptr = swap_buffer;
                break;
            }
        }
#endif

        length = (UINT32) temp[i].length;
        if (!FileWrite(file, ptr, &length)) {
            _archive_error = ARC_FILE_IO_ERROR;
            return (VOID_UINT32);
        }
        actual += length;

        i++;
    }

    return (actual);
}

/*---------------------------------------------------------------------*/
UINT32 SerializedRead(VOID * str, TEMPLATE * temp, H_FILE file)
{
    UINT8 *ptr;
    UINT32 i, length, actual;

#ifdef LTL_ENDIAN_HOST
    UINT8 hold;
#endif

    ASSERT(str != NULL);
    ASSERT(temp != NULL);

    actual = 0;

    if(file == VOID_H_FILE)
		return actual;

    i = 0;
    while (temp[i].offset != VOID_UINT32) {

        ptr = (UINT8 *) str + temp[i].offset;

        length = (UINT32) temp[i].length;
        if (!FileRead(file, ptr, &length)) {
            _archive_error = ARC_FILE_IO_ERROR;
            return (VOID_UINT32);
        }
        actual += length;

#ifdef LTL_ENDIAN_HOST
        if (temp[i].swap) {
            switch (temp[i].length) {
              case 2:
                hold = ptr[0];
                ptr[0] = ptr[1];
                ptr[1] = hold;
                break;
              case 4:
                hold = ptr[0];
                ptr[0] = ptr[3];
                ptr[3] = hold;
                hold = ptr[1];
                ptr[1] = ptr[2];
                ptr[2] = hold;
                break;
              case 8:
                hold = ptr[0];
                ptr[0] = ptr[7];
                ptr[7] = hold;
                hold = ptr[1];
                ptr[1] = ptr[6];
                ptr[6] = hold;
                hold = ptr[2];
                ptr[2] = ptr[5];
                ptr[5] = hold;
                hold = ptr[3];
                ptr[3] = ptr[4];
                ptr[4] = hold;
                break;
            }
        }
#endif

        i++;
    }

    return (actual);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/05/20 19:05:40  nobody
 * Repaired invalid assertion and increased simulation packet rate to 4
 * packets/second.
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
