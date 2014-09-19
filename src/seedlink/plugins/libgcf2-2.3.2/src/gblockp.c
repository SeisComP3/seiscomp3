/*
 * gblockp.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gblockp.c,v 1.15 2006/09/14 14:28:11 lwithers Exp $";

/*
 * $Log: gblockp.c,v $
 * Revision 1.15  2006/09/14 14:28:11  lwithers
 * Add G2GetDecLut() and the machinery for the new G2PBlock.dig_type field.
 *
 * Revision 1.14  2006/06/20 15:13:39  lwithers
 * Fix reversed arguments to memcpy() when decoding GCF status packets.
 *
 * Revision 1.13  2005/07/29 14:00:50  root
 * *** empty log message ***
 *
 * Revision 1.12  2005/06/13 15:57:04  root
 * *** empty log message ***
 *
 * Revision 1.11  2005/06/13 15:32:23  root
 * *** empty log message ***
 *
 * Revision 1.10  2004/11/18 12:43:12  root
 * *** empty log message ***
 *
 * Revision 1.9  2004/06/29 15:57:55  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/06/29 14:27:57  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/06/29 14:18:11  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/06/29 13:53:31  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/06/29 13:53:05  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/06/29 12:27:10  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/16 16:19:49  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:22  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:48  root
 * #
 *
 * Revision 1.1  2003/05/13 15:10:08  root
 * #
 *
 */

#include "includes.h"

#include "gint.h"
#include "gtime.h"
#include "gblock.h"
#include "gblockp.h"

static int
extract_8 (uint8_t * ptr, G2PBlock * b)
{
  int *optr = b->d.data;
  int n = b->samples;
  int val;

  val = G2iint32 (ptr);
  ptr += 4;

  if (*ptr)
    return -1;

  while (n--)
    {
      val += G2iint8 (ptr++);
      *(optr++) = val;
    }

  b->cric = val;
  b->tric = G2iint32 (ptr);

  if (b->cric != b->tric)
    return -1;

  return 0;
}


static int
extract_16 (uint8_t * ptr, G2PBlock * b)
{
  int *optr = b->d.data;
  int n = b->samples;
  int val;

  val = G2iint32 (ptr);
  ptr += 4;

  if (G2iint16 (ptr))
    return -1;

  while (n--)
    {
      val += G2iint16 (ptr);
      ptr += 2;
      *(optr++) = val;
    }

  b->cric = val;
  b->tric = G2iint32 (ptr);

  if (b->cric != b->tric)
    return -1;

  return 0;
}


static int
extract_24 (uint8_t * ptr, G2PBlock * b)
{
  int *optr = b->d.data;
  int n = b->samples;
  int val;

  val = G2iint32 (ptr);
  ptr += 4;

  if (G2iint24 (ptr))
    return -1;

  while (n--)
    {
      val += G2iuint24 (ptr);
      while (val >= 0x800000L)
        val -= 0x1000000L;
      ptr += 3;
      *(optr++) = val;
    }

  b->cric = val;
  b->tric = G2iint32 (ptr);

  if (b->cric != b->tric)
    return -1;

  return 0;
}


/*FIXME: write an overflowing and non overflowing version of this code*/
static int
extract_32 (uint8_t * ptr, G2PBlock * b)
{
  int *optr = b->d.data;
  int n = b->samples;
  uint32_t val;

  val = G2iint32 (ptr);
  ptr += 4;

  if (G2iint32 (ptr))
    return -1;

  while (n--)
    {
      val += G2iuint32 (ptr);
      ptr += 4;
      *(optr++) = val;
    }

  b->cric = val;
  b->tric = G2iint32 (ptr);

  if (b->cric != b->tric)
    return -1;

  return 0;
}



int
G2ParseBlockHead (G2Block * in, G2PBlockH * out)
{
  uint8_t *ptr = in->data;
  uint32_t strid,sysid;
  int i;

  sysid=G2iuint32(ptr);
  if (sysid & 0x80000000) {
    /* The mk3 digitiser and anything new will set the extended bit, in which case we steal the next
     * 5 bits of the sysid for a system identification field. This ID field will let us determine
     * which set of filter coefficients are being indexed by the reserved or `ttl' field, and we
     * call it the digitiser type.
     *
     * This was discussed with Mike Smart and Murray McGowan on 20060914.
     *
     * Currently we know about:
     *  00 = dm24mk3
     */
    switch((sysid >> 26) & 0x1F) {
    case 0x00:
      out->dig_type = G2DIGTYPEMK3;
      break;

    default:
      out->dig_type = G2DIGTYPEUNKNOWN;
      break;
    }
    sysid &= 0x3ffffff;

  } else {
    /* the mk2 digitiser doesn't set the extended bit */
    out->dig_type = G2DIGTYPEMK2;

  }

  G2base36toa (sysid, out->sysid);
  ptr += 4;
  G2base36toa ((strid=G2iuint32 (ptr)), out->strid);
  ptr += 4;

  i = G2iuint16 (ptr);
  ptr += 2;

  out->start.day = i >> 1;
  i &= 1;
  out->start.sec = (i << 16) + G2iuint16 (ptr);
  ptr += 2;

  out->ttl = *(ptr++);
  out->sample_rate = *(ptr++);
  out->format = (*(ptr++)) & 7;
  out->records = *(ptr++);

  out->samples = out->records * out->format;


  if (out->sample_rate == 0)
    {
      out->end = out->start;
	if ((out->format==4) && (out->records==4) && ((strid % (36*36))==445)) {
      		out->type=G2BLOCKTYPECDSTATUS;
	}else {
      		out->type=G2BLOCKTYPESTATUS;
 	}
      return 0;
    }

  if (out->samples % out->sample_rate) {
    out->type=G2BLOCKTYPEUNKNOWN;
    return -1;
  }
      		out->type=G2BLOCKTYPEDATA;

  out->end = G2GTimeInc (out->start, out->samples / out->sample_rate);
  return 0;
}


int
G2ParseBlock (G2Block * in, G2PBlock * out)
{
  int ret;
  uint8_t *ptr = in->data + 16;

  ret = G2ParseBlockHead (in, (G2PBlockH *) out);
  if (ret)
    return ret;

  if (out->type==G2BLOCKTYPEDATA) /*Data blocks*/
    {
      switch (out->format)
        {
        case 4:
          return extract_8 (ptr, out);
          break;
        case 2:
          return extract_16 (ptr, out);
          break;
        case 1:
          if ((in->size - 24) == (4 * out->samples))
            return extract_32 (ptr, out);
          if ((in->size - 24) == (3 * out->samples))
            return extract_24 (ptr, out);
          /*Err on the side of 32 bit data */
          return extract_32 (ptr, out);
          break;
	default:
	  return -1;
        }
    }

  if (out->type==G2BLOCKTYPECDSTATUS) {

     out->d.cdstatus.gps_fix=*(ptr++);
     out->d.cdstatus.gps_mode=*(ptr++);
     out->d.cdstatus.gps_control=*(ptr++);
     out->d.cdstatus.gps_power=*(ptr++);

     out->d.cdstatus.gps_offset=G2ilint32(ptr);
     ptr+=4;
  
     out->d.cdstatus.busy_counter=G2ilint24(ptr);
     ptr+=3;

     out->d.cdstatus.locking= ((*ptr) & 1) ? 1:0;
     out->d.cdstatus.unlocking= ((*ptr) & 2) ? 1:0;
     out->d.cdstatus.cal[1]= ((*ptr) & 4) ? 1:0;
     out->d.cdstatus.cal[0]= ((*ptr) & 8) ? 1:0;
     out->d.cdstatus.cal[2]= ((*ptr) & 0x10) ? 1:0;
     out->d.cdstatus.centering= ((*ptr) & 0x20) ? 1:0;

     return 0;
  } 

  /*Else just plain text status block*/

      memcpy (out->d.status, ptr, 1008);
      memset (out->d.status + 1008, 0, 16);

      return 0;

}

void
G2DumpPBlockH (FILE * s, G2PBlockH * ph)
{
  struct tm tm;

  fprintf (s, "Block header:\n");
  fprintf (s, "  Sysid: %6s, Strid: %6s\n", ph->sysid, ph->strid);
  fprintf (s, "  rate=%3d, format=%d, ttl=%d\n", ph->sample_rate, ph->format,
           ph->ttl);
  fprintf (s, "  records=%3d, samples=%d\n", ph->records, ph->samples);

  tm = G2UTC2tm (G2GTime2UTC (ph->start));
  fprintf (s, "  start=%s", asctime (&tm));

  tm = G2UTC2tm (G2GTime2UTC (ph->end));
  fprintf (s, "  end  =%s", asctime (&tm));


}

void
G2DumpPBlock (FILE * s, G2PBlock * p)
{
  int i;

  G2DumpPBlockH (s, (G2PBlockH *) p);

  if (p->sample_rate)
    {
      fprintf (s, "  cric=%15d tric=%15d\n", p->cric, p->tric);
      for (i = 0; i < p->samples; ++i)
        {

          if (!(i & 3))
            fprintf (s, "  %4d:", i);

          fprintf (s, " %15d", p->d.data[i]);

          if ((i & 3) == 3)
            fprintf (s, "\n");
        }
      if (i & 3)
        fprintf (s, "\n");

    }
  else
    {
      fprintf (s, "  ");
      for (i = 0; i < 1024; ++i)
        {
          char c = p->d.status[i];

          if ((c >= 32) && (c < 127))
            {
              putc (c, s);
            }
          else if (c == 10)
            {
              fprintf (s, "\n  ");
            }
          else
            {
              putc ('.', s);
            }

        }
      fprintf (s, "\n");
    }

}

char *G2BlockId(G2PBlockH * ph)
{
static char ret[16];

char *ptr=ret,*sptr=ph->sysid;

while (*sptr) *(ptr++)=*(sptr++);
*(ptr++)='-';

sptr=ph->strid;
while (*sptr) *(ptr++)=*(sptr++);

ptr--;
ptr--;
*ptr=0;

return ret;
}

