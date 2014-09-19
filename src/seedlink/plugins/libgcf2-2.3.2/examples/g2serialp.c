/*
 * g2serialp.c:
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

static char rcsid[] = "$Id: g2serialp.c,v 1.3 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2serialp.c,v $
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* Serial parser, the Serial parser understands GCF serial port */
/* protocol and calls user supplied callbacks when certain events  */
/* Occur*/

static void
usage (void)
{
  fprintf (stderr, "Usage:\n"
           "g2serialp [-p port] [-b baud]\n"
           "	-p port		serial port to open default is /dev/ttyS0\n"
           "	-b baudrate	baudrate default is 19200\n");
  exit (1);
}

/*called when the parser detects a complete block on the serial port */
void
block_callback (G2Serial * ser, G2SerBlock * block)
{
  G2PBlock pb;
/*Send an ack back to the digitizer, a proper implementation */
/*would check the sequence number and then send nack or ack */
/*approiately*/

/*For more details of the GCF serial protocol and ACKs/NAKs */
/*see the documentation in the docs directory */

  G2SerAck (ser, block);        /*Ack the block */

/*G2SerNack(ser,block,int seq), sends a nack for sequence number seq*/
/*G2SerAckIntr(ser,block), ack the block and send a request for the */
/*	Attached device to enter command/console mode*/
/*G2SerNackIntr(ser,block,int seq), nack the block and send a request for the */
/*	Attached device to enter command/console mode*/
/*G2SerInt(ser) send the interrupt character, which _should_ cause */
/*	the device to enter command/console*/

/*G2SerBlock is defined as follows */

/*
 * typedef struct
 * {
 *   uint8_t data[1024];     Data of the block
 *   int size;               Number of bytes of data in the block
 *   int seq;                The sequence number from the serial
 *                           handshake protocol
 *   int ck;                 The checksum from the serial
 *                           handshake protocol
 * } G2SerBlock;
 * 
 */

/*We can cast a G2SerBlock * to a G2Block * */
  if (G2ParseBlock ((G2Block *) block, &pb))
    {
      fprintf (stderr, "Parsing block failed\n");
      return;
    }

/*We can cast a G2PBlock * to a G2PBlockH * */

  printf ("block recevied={\n");
  G2DumpPBlockH (stdout, (G2PBlockH *) & pb);
  printf ("}\n");

}

/*The parser calls this when it detects the start of out */
/*of band data on the serial port */

void
oob_start_callback (G2Serial * ser)
{
  printf ("Parser detects the start of out of band data\n");
}

void
oob_data_callback (G2Serial * ser, void *_data, int len)
{
  uint8_t *data = _data;
  while (len--)
    {
      if (((*data) > 31) || ((*data) < 127))
        putchar (*data);
      else
        putchar ('.');
      data++;
    }
}

/*The parser calls this when it detects the end of out */
/*of band data on the serial port */
void
oob_end_callback (G2Serial * ser)
{
  printf ("\nParser detects the end of out of band data\n");
}

/*The parser calls this for every byte on the serial port*/
void
all_data_callback (G2Serial * ser, uint8_t * data, int len)
{
}

int
main (int argc, char *argv[])
{
  char *port = "/dev/ttyS0";
  int baudrate = 19200;
  int c;
  G2Serial *ser;
  G2SerialP *parser;

  fd_set rfds;
  int fd;

  while ((c = getopt (argc, argv, "p:b:")) > 0)
    {
      switch (c)
        {
        case 'p':
          port = optarg;
          break;
        case 'b':
          baudrate = atoi (optarg);
          break;
        default:
          usage ();
        }
    }

/*Open a serial port returns NULL on failure, the last argument */
/*is for flags and if non zero replaces tios.c_cflag in termios */
/*you might pass CREAD | CS8 | CLOCAL | CRTSCTS for example*/
/*NB the baudrate is an integer like 19200, not B19200 */

  ser = G2SerOpen (port, baudrate, 0);

  if (!ser)
    {
      fprintf (stderr, "Failed to open serial port %s\n", port);
      exit (1);
    }

/*G2Serial has a non opaque element called client_data, which */
/*is a void * for the user to use, since the parsers' callbacks*/
/*are passed ser, this can be a simple way to get data into the */
/*callback*/

  ser->client_data = (void *) 0;

/*Next create the Parser, any callback may be*/
/*specified as NULL */

  parser = G2CreateSerParser (ser, block_callback,
                              oob_start_callback,
                              oob_data_callback,
                              oob_end_callback, all_data_callback);

  if (!parser)
    {
      fprintf (stderr, "Failed to create parser\n");
      exit (1);
    }

/*G2SerFd gets the file descriptor the libgcf2 has got for the */
/*serial port - useful for selects and such, G2SerFd is a macro*/
  fd = G2SerFd (ser);

/*Serial ports are by default opened blocking, use */
/*G2ClrBlocking(G2SerFd(ser)) to switch to non-blocking */

  G2ClrBlocking (fd);

/*Rather than using select, a program could call G2SerData(ser) */
/*to test for presence of data, non-zero indicates data*/

  FD_ZERO (&rfds);

  for (;;)
    {

      FD_SET (fd, &rfds);

      if (select (fd + 1, &rfds, NULL, NULL, NULL) < 0)
        {
          fprintf (stderr, "select returns error\n");
          exit (1);
        }

      if (FD_ISSET (fd, &rfds))
        {
          /* Data is avaiable - call the parser's dispatch routine */
          G2SerDispatch (parser);
        }

    }

  G2DestroySerParser (parser);
  G2SerClose (ser);


  return 0;
}
