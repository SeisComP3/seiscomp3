/*
 * g2ints.c:
 *
 * examples of how to use the endian-aware non-aligned
 * integer functions.
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char rcsid[] = "$Id: g2ints.c,v 1.1 2004/05/01 23:46:03 root Exp $";

/*
 * $Log: g2ints.c,v $
 * Revision 1.1  2004/05/01 23:46:03  root
 * *** empty log message ***
 *
 */

#include <gcf2.h>

/* libgcf2 makes sure that all the usual integer types are defined */
/* (u)int8/16/32, and provides these utility functions to convert */
/* between the unaligned network byte order that GCF uses and */
/* the host's integer types */

int
main (int argc, char *argv[])
{
  printf ("%s\n", rcsid);

  {
    uint8_t buf[] = { 0x81 };
    int val;

    val = G2uint8 (buf);
    printf ("G2uint8(0x81)=%d (%x)\n", val, val);
    val = G2int8 (buf);
    printf ("G2int8(0x81)=%d\n", val);
  }

  {
    uint8_t buf[] = { 0x81, 0x02 };
    int val;

    val = G2uint16 (buf);
    printf ("G2uint16(0x81,0x02)=%d (%x)\n", val, val);
    val = G2int16 (buf);
    printf ("G2int16(0x81,0x02)=%d\n", val);
  }

  {
    uint8_t buf[] = { 0x81, 0x02, 0x03 };
    int val;

    val = G2uint24 (buf);
    printf ("G2uint24(0x81,0x02,0x03)=%d (%x)\n", val, val);
    val = G2int24 (buf);
    printf ("G2int24(0x81,0x02,0x03)=%d\n", val);
  }

  {
    uint8_t buf[] = { 0x81, 0x02, 0x03, 0x04 };
    int val;
    unsigned int uval;

    uval = G2uint32 (buf);
    printf ("G2uint32(0x81,0x02,0x03,0x04)=%u (%x)\n", uval, uval);
    val = G2int32 (buf);
    printf ("G2int32(0x81,0x02,0x03,0x04)=%d\n", val);
  }

  return 0;
}
