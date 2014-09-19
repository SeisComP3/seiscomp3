/*
 * g2cmos.c:
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

static char rcsid[] = "$Id: g2cmos.c,v 1.2 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2cmos.c,v $
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
#include <string.h>

/* CMOS parser. The only reliable way to retreive the state */
/* of the devices is to request the cmos state from it */
/* this is officially unsupported and not documented - but */
/* it's the only thing that works - this is a first cut */
/* a regularizing it. Currently it works for the DM24 MK 2 */

char *
binary (int i, int c)
{
  static char ret[10];
  char *ptr = ret;

  c = 1 << (c - 1);

  while (c)
    {
      *(ptr++) = (i & c) ? '1' : '0';
      c >>= 1;
    }
  *ptr = 0;

  return ret;
}

char *
it_to_a (int i)
{
  switch (i)
    {
    case G2_DM24_IT_40T:
      return "40T";
    case G2_DM24_IT_ESP:
      return "3ESP";
    case G2_DM24_IT_3T:
      return "3T";
    case G2_DM24_IT_3TD:
      return "3TD";
    case G2_DM24_IT_6TD:
      return "6TD";
    case G2_DM24_IT_5TD:
      return "5TD";
    default:
      return "Unknown";
    }
}

char *
tf_to_a (int i)
{
  switch (i)
    {
    case G2_DM24_FT_0__1:
      return "0.0-1.0";
    case G2_DM24_FT_0__0_9:
      return "0.0-0.9";
    case G2_DM24_FT_0_2__0_9:
      return "0.2-0.9";
    case G2_DM24_FT_0_5__0_9:
      return "0.5-0.9";
    default:
      return "Unknown";
    }
}

char *
ss_to_a (int i)
{
  switch (i)
    {
    case G2_DM24_SS_NONE:
      return "none";
    case G2_DM24_SS_TRIMBLE:
      return "Trimble";
    case G2_DM24_SS_GARMIN:
      return "Garmin";
    case G2_DM24_SS_STREAM:
      return "Stream";
    default:
      return "Unknown";
    }
}

void
dump_state_dm24 (G2CmosDM24 * dm24)
{
  struct tm tm;
  char buf[1024];
  int i;


  tm = G2UTC2tm (dm24->rtc.time);
  printf ("RTC:\t%s", asctime (&tm));

  G2base36toa (dm24->sysid, buf);
  printf ("SYSID:\t%s\n", buf);

  G2base36toa (dm24->serial, buf);
  printf ("SERIAL:\t%s\n", buf);

  printf ("pwm2:\t%d\n", dm24->pwm2);

  for (i = 0; i < 8; ++i)
    {
      printf ("channels[%d]:\t0x%02x (%s)\n", i, dm24->channels[i],
              binary (dm24->channels[i], 8));

    }



  for (i = 0; i < 8; ++i)
    {
      printf ("port %d:\t rx=%d tx=%d baud\n", i,
              dm24->rx_bauds[i], dm24->tx_bauds[i]);
    }

  printf ("aux_channels:\t0x%04x (%s)\n", dm24->aux_channels,
          binary (dm24->aux_channels, 16));

  printf ("masses:\t%d %d %d %d\n",
          G2int8 (&dm24->masses[0]),
          G2int8 (&dm24->masses[1]),
          G2int8 (&dm24->masses[2]), G2int8 (&dm24->masses[3]));

  printf ("orientation:\t%d\n", dm24->orientation);
  printf ("reboots:\t%d\n", dm24->reboots);

  tm = G2UTC2tm (dm24->lastboot.time);
  printf ("lastboot:\t%s", asctime (&tm));

  for (i = 0; i < 4; ++i)
    {
      printf ("Decimation[%d]:\t%d\n", i, dm24->decimations[i]);
    }
  for (i = 0; i < 4; ++i)
    {
      printf ("Samplerate[%d]:\t%d\n", i, dm24->samplerates[i]);
    }

  printf ("Instrument:\t%d(%s)\n", dm24->instrument_type,
          it_to_a (dm24->instrument_type));

  for (i = 0; i < 6; ++i)
    {
      printf ("Trig[%d]:\tSTC %3d LTC %3d Ratio %3d\n",
              i, dm24->stc[i], dm24->ltc[i], dm24->ratio[i]);
    }

  printf ("Trig filter:\t%d(%s)\n",
          dm24->trigger_filter, tf_to_a (dm24->trigger_filter));

  for (i = 0; i < 8; ++i)
    {
      printf ("triggered_channels[%d]:\t0x%02x (%s)\n", i, dm24->channels[i],
              binary (dm24->channels[i], 8));

    }

  for (i = 0; i < 4; ++i)
    {
      printf ("triggers[%d]:%d\n", i, dm24->triggers[i]);
    }

  printf ("pretrig:\t%d\n", dm24->pretrig);
  printf ("posttrig:\t%d\n", dm24->posttrig);

  printf ("flash_file_size:\t%d\n", dm24->flash_file_size);
  printf ("flash_mode:\t%d\n", dm24->flash_mode);
  printf ("sync_src:\t%d(%s)\n", dm24->sync_src, ss_to_a (dm24->sync_src));

  printf ("heartbeat:\t%dms\n", dm24->heartbeat * 30);
  printf ("ack_nak_wait:\t%dms\n", dm24->acknak_wait * 30);
  printf ("stopbits:\t%d\n", dm24->stopbits);

  printf ("once:\t%d\n", dm24->once);
  printf ("split:\t%d\n", dm24->split);

  printf ("gps_duty cycle:\t%d mins\n", dm24->gps_duty);
  printf ("auto_center:\t%d\n", dm24->auto_center);

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

  fd_set rfds;
  int fd;

  char *resp;

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

  G2SerCLICommand (cli, "ok-1\r");
  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to send ok-1\n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  G2SerCLICommand (cli, "$cmos\r");
  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to get cmos data\n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  resp = strdup (G2SerCLIResponse (cli));

  G2SerCLICommand (cli, "[seal]\r");
  if (G2SerCLIBlock (cli, 60))
    {
      fprintf (stderr, "Failed to send [seal]\n");
      G2DestroySerCLI (cli);
      G2SerClose (ser);
      exit (1);
    }

  G2DestroySerCLI (cli);

  /*Ok so now we have the cmos data in resp */
  {
    G2CmosRaw cmos;
    G2CmosDM24 dm24;

    /* Parse the c-string repsonse into a G2CmosRaw type */
    /* which stores the state of the cmos */

    if (G2CmosParse (resp, &cmos))
      {
        fprintf (stderr, "Failed to parse cmos string\n");
        free (resp);
        G2SerClose (ser);
        exit (1);
      }

    /* Parse the cmos state into a broken down configuration */
    /* see gcf2.h for all of these, they are mostly undocumented */

    if (G2CmosDecodeDM24 (&cmos, &dm24))
      {
        fprintf (stderr, "Failed to parse cmos data");
        free (resp);
        G2SerClose (ser);
        exit (1);
      }
    free (resp);
    G2SerClose (ser);


    dump_state_dm24 (&dm24);

  }
  return 0;
}
