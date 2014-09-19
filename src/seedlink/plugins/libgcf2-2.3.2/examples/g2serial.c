/*
 * g2serial.c:
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

static char rcsid[] = "$Id: g2serial.c,v 1.3 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2serial.c,v $
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

/* Serial ports */
/* There are a frightening array of serial port interfaces */
/* and commands this is a simple subset for dealing with GSL */
/* equipment and is not meant to be exhaustive */

/*This is a simple echo program on the serial port*/

uint8_t buf[1024];

static void
usage (void)
{
  fprintf (stderr, "Usage:\n"
           "g2serial [-p port] [-b baud]\n"
           "	-p port		serial port to open default is /dev/ttyS0\n"
           "	-b baudrate	baudrate default is 19200\n");
  exit (1);
}

int
main (int argc, char *argv[])
{
  char *port = "/dev/ttyS0";
  int baudrate = 19200;
  int c;
  G2Serial *ser;
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
/*is a void * for the user to use*/

  ser->client_data = (void *) 0;

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
          int red;
          /*Data is avaiable */
          red = G2SerRead (ser, buf, sizeof (buf));

          if (red > 0)          /* FIXME: a real program would check Write succeeded */
            G2SerWrite (ser, buf, red);
        }

    }


  /*Other serial functions are */
  /*G2SerWriteStrSlow(G2Serial *, char *) - which writes a C string 
   *                                        to the serial port with 
   *                                        pauses sufficient to allow
   *                                        non FIFO'd ports to cope 
   *                                        returns the number of bytes
   *                                        written 
   */


  /*G2SerDrain(G2Serial *, int timeout) - wait upto timeout seconds 
   *                                      for a serial port to drain
   *                                      returns 0 for success
   */


  G2SerClose (ser);

  return 0;
}
