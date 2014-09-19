/*
 * g2net.c:
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

static char rcsid[] = "$Id: g2net.c,v 1.2 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2net.c,v $
 * Revision 1.2  2004/05/02 10:12:20  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/02 00:43:39  root
 * *** empty log message ***
 *
 */

/* Example program using the network calls in libgcf2 */

int
main (int argc, char *argv)
{
#if 0
  extern int G2NetLookup (char *namecolonport, G2NetAddr * a);
  /*Lookup an address,ret 0 on success */
  extern int G2NetPort (G2Net *);
  /*Get port number */
  extern G2Net *G2NetOpenUDP (int port);
  /*Open a UDP port */
  extern G2Net *G2NetOpenUDPB (int port, int local);
  /*Open a UDP port for broadcasting */
  extern G2Net *G2NetOpenTCPC (G2NetAddr * remote);
  /*Open a TCP client port and connect to remote */
  extern G2Net *G2NetOpenTCPCNB (G2NetAddr * remote);
  /*Open a TCP client port and connect to remote
   *non blocking call, call the next to get the 
   *result*/
  extern int G2NetOpenTCPCNBE (G2Net *);
  /*Get result, if non zero, G2Net will
   *have been freed*/
  extern G2Net *G2NetOpenTCPS (int port);
  /*Open a TCP client server port */

  extern int G2NetOpenUDPTCP (int port, G2Net ** tcp, G2Net ** udp);
  /*Attempt to open a TCP server port,
   *and a UDP port on the same number*/

  extern void G2NetClose (G2Net *);
  /*Close and free resources */

  extern int G2NetRecv (G2Net * net, G2NetAddr * from, void *buf, int len);
  extern int G2NetSend (G2Net * net, G2NetAddr * to, void *buf, int lne);
  extern char *G2NetAddrtoa (G2NetAddr * a);
  extern G2Net *G2NetAccept (G2Net * tcps, G2NetAddr * from);

#endif

  return 0;

}
