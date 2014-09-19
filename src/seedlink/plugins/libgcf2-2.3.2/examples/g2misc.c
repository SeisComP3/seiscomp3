/*
 * g2misc.c:
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

static char rcsid[] = "$Id: g2misc.c,v 1.3 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2misc.c,v $
 * Revision 1.3  2004/05/02 10:12:20  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/05/02 00:43:39  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/01 23:46:03  root
 * *** empty log message ***
 *
 */

#include <gcf2.h>

/* Miscelleneous library functions */

int
main (int argc, char *argv[])
{
  uint32_t fish = 724193;
  char *soup = "SOUP";
  char buf[7];

  printf ("%s\n", rcsid);

/*A call to get the version of the library, the returned */
/*value is static, and valid so long as library is mapped */
/*(don't try to free it)*/

  printf ("G2GetVersion()=%s\n", G2GetVersion ());

/*The GCF protocol stores names is a base-36 format, complete */
/*with a strage form of padding, these two functions help you */
/*convert to and from it if you need to */


/*Buf needs to be at least 7 bytes, and the result _will_ be */
/*null terminated */
  G2base36toa (fish, buf);
  printf ("G2base36toa(%u,buf),  buf=%s\n", fish, buf);

/*The string passed should be capable of being represented */
/*in 32 (actually some digitizers can only handle 31) bits.*/

  printf ("G2atobase36(%s)=%d\n", soup, G2atobase36 (soup));

/*G2Sleep(int len) sleeps for len micro seconds*/

  G2Sleep (1000000);

  return 0;
}
