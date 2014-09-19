/*
 * gcf.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 *
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

a_fatal (char * str)
{
      printf("Fatal error:  %s\n", str);
      exit(0);
}

static char rcsid[] = "$Id: gcf.c 1364 2008-10-24 18:42:33Z andres $";

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.4  2003/02/28 17:05:37  root
 * #
 *
 */

#include "project.h"

static time_t
gcf_time_to_time_t (gcf_time * t)
{
  time_t ret = GCF_EPOCH;

  ret += (t->day) * 24 * 60 * 60;
  ret += (t->sec);

  return ret;
}

static void
extract_8 (gcf_block b)
{
  uint8_t *ptr = b->buf;
  int *optr = b->data;

  int val;
  int n = b->samples;

  ptr += 16;

  val = gp_int32 (ptr);
  ptr += 4;

  if (*ptr)
    a_fatal (("First difference is not zero"));
  while (n--)
    {
      val += gp_int8 (ptr++);
      *(optr++) = val;
    }
  b->fic = val;
  b->ric = gp_int32 (ptr);

  if (b->fic != b->ric)
    a_fatal (("Fic!=Ric"));

}

static void
extract_16 (gcf_block b)
{
  uint8_t *ptr = b->buf;
  int *optr = b->data;

  int val;
  int n = b->samples;

  ptr += 16;

  val = gp_int32 (ptr);
  ptr += 4;

  if (*ptr)
    a_fatal (("First difference is not zero"));

  while (n--)
    {
      val += gp_int16 (ptr);
      ptr += 2;
      *(optr++) = val;
    }
  b->fic = val;
  b->ric = gp_int32 (ptr);

  if (b->fic != b->ric)
    a_fatal (("Fic!=Ric"));


}

void
extract_24 (gcf_block b)
{
  uint8_t *ptr = b->buf;
  int *optr = b->data;

  int val;
  int n = b->samples;

  ptr += 16;

  if ((*ptr) && (0xff != *ptr))
    a_fatal (("claimed 24 bit data isn't"));

  val = gp_int32 (ptr);
  ptr += 4;

  if (gp_int24 (ptr))
    a_fatal (("First difference is not zero"));

  while (n--)
    {
      val += gp_uint24 (ptr);
      ptr += 3;
      while (val >= 0x800000L)
        val -= 0x1000000L;
      *(optr++) = val;
    }
  b->fic = val;
  if ((*ptr) && (0xff != *ptr))
    a_fatal (("claimed 24 bit data isn't"));

  b->ric = gp_int32 (ptr);


  if (b->fic != b->ric)
    a_fatal (("Fic!=Ric"));

}

#if 0
void
extract_32 (gcf_block b)
{
  uint8_t *ptr = b->buf;
  int *optr = b->data;

  int64_t val;
  int n = b->samples;

  ptr += 16;

  val = (int64_t) gp_int32 (ptr);
  ptr += 4;

  if (*ptr)
    a_fatal (("First difference is not zero"));

  while (n--)
    {
      val += (int64_t) gp_int32 (ptr);
      ptr += 4;
      *(optr++) = (int) val;
    }

  b->fic = (int) val;
  b->ric = gp_int32 (ptr);

  if (b->fic != b->ric)
    a_fatal (("Fic!=Ric"));
}
#else


void
extract_32 (gcf_block b)
{
  uint8_t *ptr = b->buf;
  int *optr = b->data;

  uint32_t uval;
  int32_t val;

  int n = b->samples;

  ptr += 16;

  uval = gp_uint32 (ptr);
  ptr += 4;

  if (*ptr)
    a_fatal (("First difference is not zero"));

  while (n--)
    {
      uval += gp_uint32 (ptr);
      ptr += 4;

      if (uval == 0x80000000UL)
        {
          val = -0x80000000L;
        }
      else if (uval & 0x80000000UL)
        {
          val = -1 - (int32_t) (0xffffffffUL - uval);
        }
      else
        {
          val = (int32_t) uval;
        }

      *(optr++) = (int) val;
    }

  b->fic = (int) val;
  b->ric = gp_int32 (ptr);

  if (b->fic != b->ric)
    a_fatal (("Fic!=Ric"));
}


#endif


void
gcf_dispatch (uint8_t * buf, int sz)
{
  int i, blocknr, recno;
  uint32_t id;
  struct gcf_block_struct block;

  block.buf = buf;
  block.size = sz;
  block.csize = 0;

  id = gp_uint32(buf);
  if (id & 0x80000000) id &= 0x03FFFFFF;
  strcpy (block.sysid, gp_base36_to_a (id));
  //strcpy (block.sysid, gp_base36_to_a (gp_uint32 (buf)));
  strcpy (block.strid, gp_base36_to_a (gp_uint32 (buf + 4)));

  i = gp_uint16 (buf + 8);
  block.start.day = i >> 1;
  i &= 1;
  i <<= 16;
  block.start.sec = i | gp_uint16 (buf + 10);

  block.estart = gcf_time_to_time_t (&block.start);

  block.ttl = buf[12];               // added RS
  block.sample_rate = buf[13];
  block.format = buf[14] & 7;
  block.records = buf[15];
  block.samples = (block.format) * (block.records);

  block.text = buf + 16;

  if (block.sample_rate)
    {
      switch (block.format)
        {
        case 4:
          block.csize = 24 + block.samples;
          extract_8 (&block);
          break;
        case 2:
          block.csize = 24 + (2 * block.samples);
          extract_16 (&block);
          break;
        case 1:
          if ((sz - 24) == (4 * block.samples))
            {
              block.csize = 24 + (4 * block.samples);
              extract_32 (&block);
            }
          else if ((sz - 24) == (3 * block.samples))
            {
              block.csize = 24 + (3 * block.samples);
              extract_24 (&block);
            }
          else
            {
              /* Guess 32 */
              extract_32 (&block);
            }
          break;
        default:
          a_fatal (("unknown GCF compression format"));
        }
    }

    blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
    recno = blocknr;

    //printf("now dispatch the block with %d samples .... !!!   recno %d\n", block.samples, recno);

    dispatch (&block, recno);
}

void byte_swap_2(uint8_t* x)
{
    uint8_t y = x[0];
    x[0] = x[1];
    x[1] = y;
}

void byte_swap_4(uint8_t* x)
{
    uint8_t y = x[0];
    x[0] = x[3];
    x[3] = y;
    y = x[1];
    x[1] = x[2];
    x[2] = y;
}



void gcf_byte_swap(uint8_t* buf)
{
    int fmt, nrec;
    uint8_t *iptr;
    iptr = buf;

    // bytes 0-3 are the sysid
    byte_swap_4(buf + 0);
    // bytes 4-7 are the strid
    byte_swap_4(buf + 4);
    // bytes 8-12 are the datecode
    byte_swap_4(buf + 8);

    // compute number of records
    fmt = buf[14];
    if(!buf[13]) fmt = 0; // status
    nrec = fmt * buf[15];

    // byte swap data, if necessary
    switch(fmt) {
    case 1:
    case 2:
    case 4:
        break;

    default:
        return;
    }

    byte_swap_4(buf + 16); // fic
    buf += 20;

    switch(fmt) {
    case 1:
        if(nrec > 250) nrec = 250;
        while(nrec--) {
            byte_swap_4(buf);
            buf += 4;
        }
        break;

    case 2:
        if(nrec > 500) nrec = 500;
        while(nrec--) {
            byte_swap_2(buf);
            buf += 2;
        }
        break;

    case 4:
        if(nrec > 1000) nrec = 1000;
        buf += nrec;
        break;
    }

    byte_swap_4(buf);    // swap last sample ???

    //buf += 2;
    //byte_swap_2(buf);   // sequence number

    iptr += 1026;
    byte_swap_2(iptr);   // sequence number
}

