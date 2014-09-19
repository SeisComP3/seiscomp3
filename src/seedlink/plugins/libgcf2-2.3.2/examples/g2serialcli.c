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
  "$Id: g2serialcli.c,v 1.2 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2serialcli.c,v $
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

/* Serial command line interface, allows for interrupting and sending */
/* requests to the terminal/command mode of a digitizer */

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

/*G2SerFd gets the file descriptor the libgcf2 has got for the */
/*serial port - useful for selects and such, G2SerFd is a macro*/
  fd = G2SerFd (ser);

/*Serial ports are by default opened blocking, use */
/*G2ClrBlocking(G2SerFd(ser)) to switch to non-blocking */

  G2ClrBlocking (fd);

  /*The CLI has two interfaces - a synchronous one */
  /*and an asynchronos one */

  /*First the synchronos one */

  /*Create the interface */
  cli = G2CreateSerCLI (ser);
  if (!cli)
    {
      fprintf (stderr, "Failed to create cli\n");
      exit (1);
    }

  /*Get into command mode - actually we could skip this step */
  /*if we wanted and set the command, in which case CLIBlock */
  /*would block until the command completed the last argument */
  /*is a timeout in seconds */
  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to get into command mode \n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  /*When the device enters command mode it prints a banner */
  /*this can be retried using G2SerCLIBanner */
  printf ("Banner: %s\n", G2SerCLIBanner (cli));

  G2SerCLICommand (cli, ".ids\r");
  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to issue command\n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  /*The command's response is retried with G2SerCLIResponse */
  printf ("Response: %s\n", G2SerCLIResponse (cli));


  /*Destroying the parser with G2DestroySerCLI exits command mode on the */
  /*the device, if you don't want this (eg rebooting) call G2HushDestroySerCLI */
  /*instead */
  G2DestroySerCLI (cli);

  /*Next the async one */


  FD_ZERO (&rfds);

  /* G2SerCLIInCmdMode returns zero if the CLI engine hasn't got us */
  /* into command mode yet */

  while (!G2SerCLIInCmdMode (cli))
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
          G2SerCLIDispatch (cli);
        }

    }
  printf ("Banner: %s\n", G2SerCLIBanner (cli));

  G2SerCLICommand (cli, ".ids\r");

  /* G2SerCLICommandDone returns zero if the CLI engine hasn't */
  /* executed the command yet */
  while (!G2SerCLICommandDone (cli))
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
          G2SerCLIDispatch (cli);
        }

    }
  printf ("Response: %s\n", G2SerCLIResponse (cli));



  G2DestroySerCLI (cli);
  G2SerClose (ser);


  return 0;
}
