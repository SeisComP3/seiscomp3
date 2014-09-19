/*
 * gblock.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gblock.c,v 1.5 2004/05/05 17:02:49 root Exp $";

/*
 * $Log: gblock.c,v $
 * Revision 1.5  2004/05/05 17:02:49  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/10/30 10:49:43  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/10/30 10:49:32  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:22  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:49  root
 * #
 *
 * Revision 1.1  2003/05/13 15:10:06  root
 * #
 *
 */

#include "includes.h"

#include "gtime.h"
#include "gblock.h"
#include "gblockp.h"

int
G2transcode24to32 (G2Block * in, G2Block * out)
{
  uint8_t *sptr, *dptr;
  int format, samples;
  int osize;
  G2PBlock pb;
  int i=0;

  if (!(in->data[13]))
    {
      /*Sample rate is zero this is a text block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
      return 0;
    }

  format = in->data[14] & 7;



  if (format != 1)
    {
      /*Not a 24/32 bit block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
      return 0;
    }

  samples = format * (int) (in->data[15]);

  if ((in->size - 24) != (3 * samples))
    {
      /*It's not got the right size for a 24 bit block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;

      return 0;
    }

/*Ok we can't get out of doing this let's get on with it*/

  sptr = in->data;
  dptr = out->data;

  memcpy (dptr, sptr, 20);
  osize = 20;
  dptr += 20;
  sptr += 20;

  G2ParseBlock (in, &pb);

  if (samples > 250)
    return -1;

  {
  uint32_t u=pb.d.data[0];
  while (samples--)
    {
	uint32_t v=pb.d.data[i++];
	

      *(dptr++)=(v-u) >> 24;
      *(dptr++)=(v-u) >> 16;
      *(dptr++)=(v-u) >> 8;
      *(dptr++)=(v-u);
	u=v;

/*
      if ((*(sptr)) & 0x80)
        *(dptr++) = 0xff;
      else
        *(dptr++) = 0;

      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
*/

      sptr+=3;
      osize += 4;
    }
  }

  *(dptr++) = *(sptr++);
  *(dptr++) = *(sptr++);
  *(dptr++) = *(sptr++);
  *(dptr++) = *(sptr++);

  osize += 4;

  out->size = osize;

  return 0;
}

int
G2transcode32to24 (G2Block * in, G2Block * out)
{
  uint8_t *sptr, *dptr;
  int format, samples;
  int osize;

  if (!(in->data[13]))
    {
      /*Sample rate is zero this is a text block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
      return 0;
    }

  format = in->data[14] & 7;

  if (format != 1)
    {
      /*Not a 24/32 bit block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
      return 0;
    }

  samples = format * (int) (in->data[15]);

  if ((in->size - 24) == (3 * samples))
    {
      /*It's already a 24 bit block */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
      return 0;
    }

/*Ok we can't get out of doing this let's get on with it*/

  sptr = in->data;
  dptr = out->data;

  memcpy (dptr, sptr, 20);
  osize = 20;
  dptr += 20;
  sptr += 20;

  if (samples > 250)
    return -1;

  while (samples--)
    {
      if ((*sptr) == 0xff)
        {
          if (!(*(sptr + 1) & 0x80))
            {
              osize = 0;
              break;
            }
        }
      else if ((*sptr) == 0x0)
        {
          if ((*(sptr + 1) & 0x80))
            {
              osize = 0;
              break;
            }
        }
      else
        {
          osize = 0;
          break;
        }

      sptr++;
      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      osize += 3;
    }

  if (osize)
    {

      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      *(dptr++) = *(sptr++);
      osize += 4;

      out->size = osize;
    }
  else
    {                           /*Didn't fit */
      memcpy (out->data, in->data, in->size);
      out->size = in->size;
    }


  return 0;

}


int
G2FileRead1KBlock (G2File * s, G2Offt block, G2Block * out)
{
  out->size = 1024;
  return G2File1KRead (s, block, out->data, 1);
}

int
G2FileReadBlock (G2File * s, G2Block * out)
{
  out->size = 1024;
  return G2FileRead (s, out->data, out->size);
}

int
G2DumpBlock (FILE * s, G2Block * in)
{
  hexdump (s, in->data, 0, in->size);
}
