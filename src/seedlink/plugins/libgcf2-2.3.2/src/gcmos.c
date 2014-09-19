/*
 * gcmos.c:
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

static char rcsid[] = "$Id: gcmos.c,v 1.12 2005/06/13 15:32:23 root Exp $";

/*
 * $Log: gcmos.c,v $
 * Revision 1.12  2005/06/13 15:32:23  root
 * *** empty log message ***
 *
 * Revision 1.11  2004/10/29 12:05:25  root
 * *** empty log message ***
 *
 * Revision 1.10  2004/10/27 12:57:37  root
 * *** empty log message ***
 *
 * Revision 1.9  2004/10/27 12:57:18  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/06/09 13:40:52  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/20 22:31:47  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/04/20 20:43:44  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/20 20:09:12  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/04/20 19:04:44  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/20 17:59:55  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/20 16:26:48  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/04/20 15:39:08  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/04/15 22:35:45  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/04/14 15:39:52  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/03/18 19:14:43  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/03/18 19:13:36  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/13 16:33:14  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/13 15:42:03  root
 * *** empty log message ***
 *
 */

#include "includes.h"
#include "gcmos.h"

static int
xtoi (char c)
{
  switch (c)
    {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'a':
    case 'A':
      return 0xa;
    case 'b':
    case 'B':
      return 0xb;
    case 'c':
    case 'C':
      return 0xc;
    case 'd':
    case 'D':
      return 0xd;
    case 'e':
    case 'E':
      return 0xe;
    case 'f':
    case 'F':
      return 0xf;
    default:
      return 0;
    }
}

static int
x2toi (char *x2)
{
  return (xtoi (*x2) << 4) + xtoi (*(x2 + 1));
}

static char *
find_cmos (char *response)
{
  char *cmos = NULL;

  while (*response)
    {
      if (!strncasecmp (response, "$cmos ", 6))
        cmos = response;

      response++;
    }

  return cmos;
}



int
G2CmosParse (char *response, G2CmosRaw * out)
{
  char *cmos = find_cmos (response);
  int len = 0;
  char *out_ptr = out->cmos;

  if (!cmos)
    return -1;

  cmos += 6;

  if ((!isxdigit (*cmos)) || (!isxdigit (*(cmos + 1))))
    {
      return -1;
    }

  out->len = x2toi (cmos) << 8;

  cmos += 2;

  if ((!isxdigit (*cmos)) || (!isxdigit (*(cmos + 1))))
    {
      return -1;
    }

  out->len += x2toi (cmos);

  cmos+=2;

  if (out->len > sizeof (out->cmos))
    return -1;

  while (isxdigit (*cmos))
    {
      if (!isxdigit (*(cmos + 1)))
        {
          return -1;
        }
      else
        {
          *(out_ptr++) = x2toi (cmos);
          len++;
        }
      if (len == out->len)
        break;
      cmos += 2;
    }

  return (out->len == len) ? 0 : -1;
}

static int bcd(uint8_t i)
{
return ((i >> 4)*10) + (i & 0xf);
}

static void
DecodeDM24Time (uint8_t * ptr, G2DM24Time * t)
{
G2GTime gt;


  t->time.sec = bcd(ptr[2]);
  t->time.min = bcd(ptr[3]);
  t->time.hour = bcd(ptr[4]);
  t->time.mday = bcd(ptr[5]);
  t->time.month = bcd(ptr[6]);
  t->time.year = (bcd(ptr[8])*100)+bcd(ptr[9]);


  if (t->time.year<1983) {
	t->time.year=0;
  	t->time.wday=-1;
  } else {
    gt=G2UTC2GTime(t->time);
    t->time=G2GTime2UTC(gt);
  }



  t->status = bcd(ptr[0]);
  t->hundredths = bcd(ptr[1]);
  t->timer = bcd(ptr[7]);

}

static int
BaudRate (int br, int divisor)
{
  switch (br & 0xf)
    {
    case 0:
      return 7200;
    case 1:
      return 800;
    case 2:
      return 1076;
    case 3:
      return 14400;
    case 4:
      return 28800;
    case 5:
      return 57600;
    case 6:
      return 115200;
    case 7:
      return 2000;
    case 8:
      return 57600;
    case 9:
      return 4800;
    case 10:
      return 14400;
    case 11:
      return 9600;
    case 12:
      return 19200;
    case 13:
      if (divisor)
        return 115200 / divisor;
      return -1;
    default:
      return -1;
    }
}

static int
decimation (int i)
{
  switch (i)
    {
    case 1:
      return 2;
    case 2:
      return 3;
    case 3:
      return 4;
    case 4:
      return 5;
    case 5:
      return 8;
    case 6:
      return 10;
    case 7:
      return 16;
    default:
      return 1;
    }
}

int
G2CmosDecodeDM24 (G2CmosRaw * in, G2CmosDM24 * out)
{
  int i;

  memset(out,0,sizeof(G2CmosDM24));

  switch(in->len) {
	case 0x100:
	case 0xcc:
	break;
	default:
	return -1;
  }

  DecodeDM24Time (&in->cmos[0], &out->rtc);

  out->sysid = G2uint32 (&in->cmos[16]);
  out->serial = G2uint32 (&in->cmos[20]);
  out->pwm2 = G2uint32 (&in->cmos[24]);

  for (i=0;i<8;++i) {
	out->channels[i]=in->cmos[28+i];
  }

  for (i = 0; i < 8; ++i)
    {
      out->tx_bauds[i] = BaudRate (in->cmos[36 + i] & 0xf, G2uint16(&in->cmos[(i & 3)+124]));
      out->rx_bauds[i] = BaudRate (in->cmos[36 + i] >> 4, G2uint16(&in->cmos[(i & 3)+124]));
    }

  out->aux_channels = G2uint16 (&in->cmos[44]);

  for (i = 0; i < 4; ++i)
    {
      out->masses[i] = in->cmos[48 + i];
    }

  out->orientation = G2uint32 (&in->cmos[52]);
  out->reboots = G2uint16 (&in->cmos[56]);

  DecodeDM24Time (&in->cmos[58], &out->lastboot);

  for (i = 0; i < 4; ++i)
    {
      out->decimations[i] = decimation (in->cmos[68 + i]);
      out->samplerates[i] = in->cmos[72 + i];
    }


  /* BUG! FIXME: - DM24MKIII doesn't set decimations */

  {
  int bad=0;

  
  for (i = 1; i < 4; ++i)
    {
	if (out->decimations[i] != (out->samplerates[i-1] / out->samplerates[i])) bad++;
    }

	if (bad) {

	int sr=2000;
	for (i=0;i<4;++i) {
	   out->decimations[i]=sr/out->samplerates[i];
	   sr=out->samplerates[i];
	}

	}
   }

 

  out->instrument_type=G2uint16(&in->cmos[76]);

  for (i = 0; i < 3; ++i)
    {
      out->stc[i] = in->cmos[78 + (i*2)];
      out->ltc[i] = in->cmos[84 + (i*2)];
      out->ratio[i] = in->cmos[90 + (i*2)];

      out->stc[i+3] = in->cmos[78 + 1+(i*2)];
      out->ltc[i+3] = in->cmos[84 + 1+(i*2)];
      out->ratio[i+3] = in->cmos[90 + 1+(i*2)];

    }
/*7+8 hide here...
      out->stc[3 + i] = in->cmos[146 + i];
      out->ltc[3 + i] = in->cmos[148 + i];
      out->ratio[3 + i] = in->cmos[150 + i];
*/

  out->trigger_filter = in->cmos[96];
  out->filter_tap = in->cmos[98];

  for (i = 0; i < 4; ++i)
    {
      out->triggered_channels[i] = in->cmos[100 + i];
      out->triggers[i] = in->cmos[104 + i];
    }

  out->pretrig = in->cmos[108];
  out->posttrig = in->cmos[110];

  out->flash_file_size = in->cmos[114];
  out->flash_mode = in->cmos[116];
  out->sync_src = G2uint16 (&in->cmos[118]);
  out->heartbeat = G2uint16 (&in->cmos[120]);
  out->acknak_wait = in->cmos[122];
  out->stopbits = in->cmos[123];
  out->once = in->cmos[162];
  out->split = in->cmos[164];
  out->gps_duty = G2uint16 (&in->cmos[198]);

   if (in->len<248) {
      out->auto_center = 255;

  } else {
  i = in->cmos[248];

  if (i == (0xff & (~in->cmos[249])))
    {
      out->auto_center = i;
    }
  else
    {
      out->auto_center = 255;
    }
  }
  return 0;
}
