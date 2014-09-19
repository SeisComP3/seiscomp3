/*
 * g2serialcli.c:
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

static char rcsid[] =
  "$Id: application.c,v 1.4 2004/05/02 10:30:53 root Exp $";

/*
 * $Log: application.c,v $
 * Revision 1.4  2004/05/02 10:30:53  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/05/02 10:28:07  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/05/02 10:27:48  root
 * Makefile.am
 *
 * Revision 1.1  2004/05/02 10:20:56  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/05/02 10:12:20  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/02 00:43:39  root
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

/* An example application, open a serial port */
/* send a command to the instrument, get the reply */
/* and then save blocks to a file */

void
block_callback (G2Serial * ser, G2SerBlock * block)
{
  G2Block b32;
  static int nextseq = -1;
  static int nacks = 0;

  if (block->seq != nextseq)
    printf ("Got sequence %d wanted %d\n", block->seq, nextseq);

  if ((nextseq != block->seq) && (nacks < 13))
    {
      if (nextseq == -1)
        {
          nextseq = block->seq + 128;
          nextseq &= 0xff;
        }
      printf ("Sending nack %d for sequence %d\n", nacks, nextseq);
      G2SerNack (ser,  block, nextseq);
      nacks++;
    }
  else
    {
      G2SerAck (ser,  block);
      nacks = 0;
      nextseq = (block->seq) + 1;
      nextseq &= 0xff;
    }


  /*Disk files always have 32 bit blocks */
  /*Transcode 24 bit blocks to 32 bit blocks */
  /*(This copies every other sort of block) */

  G2transcode24to32 ((G2Block *) block, &b32);

  /*Write the block to the file */
  fwrite (block->data, 1024, 1, (FILE *) ser->client_data);

  {
    G2PBlockH pbh;
    G2ParseBlockHead (&b32, &pbh);
    G2DumpPBlockH (stderr, &pbh);
  }

}


static void
usage (void)
{
  fprintf (stderr, "Usage:\n"
           "g2serialcli [-p port] [-b baud]\n"
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
  G2SerialCLI *cli;
  G2SerialP *parser;
  FILE *f;

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

  ser = G2SerOpen (port, baudrate, 0);

  if (!ser)
    {
      fprintf (stderr, "Failed to open serial port %s\n", port);
      exit (1);
    }

  fd = G2SerFd (ser);
  G2ClrBlocking (fd);

  cli = G2CreateSerCLI (ser);
  if (!cli)
    {
      fprintf (stderr, "Failed to create cli\n");
      exit (1);
    }

  G2SerCLICommand (cli, ".ids\r");

  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to issue command\n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  printf ("Banner: %s\n", G2SerCLIBanner (cli));
  printf ("Response: %s\n", G2SerCLIResponse (cli));

  G2DestroySerCLI (cli);

  parser = G2CreateSerParser (ser, block_callback, NULL, NULL, NULL, NULL);

  if (!parser)
    {
      fprintf (stderr, "Failed to create parser\n");
      G2SerClose (ser);
      exit (1);
    }

  f = fopen ("out.gcf", "w");

  if (!f)
    {
      fprintf (stderr, "Failed to open out.gcf\n");
      G2SerClose (ser);
      exit (1);
    }

  ser->client_data = f;

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
          G2SerDispatch (parser);
        }

    }

  G2DestroySerParser (parser);

  G2SerClose (ser);


  return 0;
}
