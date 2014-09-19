/*
 * ./src/gserialp.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gserialp.c,v 1.12 2005/07/29 14:11:06 root Exp $";

/*
 * $Log: gserialp.c,v $
 * Revision 1.12  2005/07/29 14:11:06  root
 * *** empty log message ***
 *
 * Revision 1.11  2005/07/29 14:00:50  root
 * *** empty log message ***
 *
 * Revision 1.10  2005/07/29 14:00:16  root
 * *** empty log message ***
 *
 * Revision 1.9  2005/07/13 09:19:16  root
 * *** empty log message ***
 *
 * Revision 1.8  2005/06/14 09:35:01  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/02/13 15:46:04  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/02/13 15:42:03  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/06/06 09:41:41  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/29 16:40:03  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:25  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/05/14 16:31:48  root
 * #
 *
 * Revision 1.3  2003/05/13 15:10:05  root
 * #
 *
 * Revision 1.2  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.1  2003/05/13 09:16:25  root
 * #
 *
 */

#include "includes.h"

#include "gserial.h"
#include "gserialp.h"

#define RDBUF 512

void
G2SerCheckSum (G2SerBlock * b)
{
  register uint8_t *ptr = b->data;
  register int ret = 'G';
  register int n = b->size;

  ret += b->seq;
  ret += b->size & 0xff;
  ret += (b->size >> 8);

  while (n--)
    ret += *(ptr++);

  ret &= 0xffff;

  b->ck = ret;
}

#ifdef ENABLE_MONITOR

# define gserialp_timersub(a, b, result)				      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)


static const char monitor[7]="monitor";

static void inline do_monitor(G2SerialP *p,unsigned char *buf,int len)
{
if (len!=1) return 0;

if (p->monitor_state>=sizeof(monitor)) {
	p->monitor_state=0;
}

if (*buf==monitor[0]) p->monitor_state=0;

if (*buf==monitor[p->monitor_state]) p->monitor_state++;
	else p->monitor_state=0;


}

#endif

#ifdef INLINE_DISPATCH
static void inline
serial_dispatch (G2SerialP * p)
#else
void
G2SerDispatch (G2SerialP * p)
#endif
{
  int red;
  unsigned char buf[RDBUF], *ptr = buf;

/*Special case avoid a buffer copy if we can*/
  if (p->state == G2SS_DATA)
    {

      red = G2SerRead (p->s, &p->b.data[p->dred], p->dtoread);

      if (red < 0)
        return;

      if (p->all_data)
        p->all_data (p->s, &p->b.data[p->dred], red);

      p->dred += red;
      p->dtoread -= red;

      if (!(p->dtoread))
        p->state = G2SS_CK1;

      return;
    }

/*Otherwise read(,,1) is worse than a buffer copy*/

  red = G2SerRead (p->s, buf, sizeof (buf));

  if (red < 0)
    return;

  if (p->all_data)
    p->all_data (p->s, buf, red);

#ifdef ENABLE_MONITOR
	do_monitor(p,buf,red);
#endif

  while (red)
    {
      /*printf("Red=%d state=%d toread=%d\n",red,p->state,p->dtoread); */

      if (p->state == G2SS_DATA)
        {
          int u = (red < p->dtoread) ? red : p->dtoread;

          if (p->inoob)
            if (p->oob_data)
              p->oob_data (p->s, ptr, u);

          memcpy (&p->b.data[p->dred], ptr, u);

          ptr += u;
          red -= u;

          p->dred += u;
          p->dtoread -= u;

          if (!(p->dtoread))
            p->state = G2SS_CK1;

        }
      else
        {
          if (p->inoob)
            if (p->oob_data)
              p->oob_data (p->s, ptr, 1);

          switch (p->state)
            {
            case G2SS_LOST:
              if (*ptr == 'G')
                {
                  p->state = G2SS_SEQ;
                  break;
                }


              if (!p->inoob)
                {
                  if (p->oob_start)
                    p->oob_start (p->s);
                  if (p->oob_data)
                    p->oob_data (p->s, ptr, 1);
                }
              p->inoob = 1;


              if (*ptr == 0x1b)
                p->state = G2SS_1B;

              break;

            case G2SS_SEQ:
              p->b.seq = *ptr;
              p->state = G2SS_SZ1;
              break;
            case G2SS_SZ1:
              p->b.size = ((int) *ptr) << 8;
              p->state = G2SS_SZ2;
              break;
            case G2SS_SZ2:
              p->b.size += *ptr;
              p->dred = 0;
              p->dtoread = p->b.size;
              p->state = G2SS_DATA;

              if (p->b.size > 1024)
                {
                  p->inoob = 1;
                  if (p->oob_start)
                    p->oob_start (p->s);
                  p->state = G2SS_LOST;
                }
              break;
            case G2SS_CK1:
              p->cksum = ((int) *ptr) << 8;
              p->state = G2SS_CK2;
              break;
            case G2SS_CK2:
              {
                p->cksum += *ptr;
                G2SerCheckSum (&p->b);

                if (p->b.ck == p->cksum)
                  {
                    if (p->inoob)
                      {
                        if (p->oob_end)
                          p->oob_end (p->s);
                        p->inoob = 0;
                      }
                    if (p->block)
                      p->block (p->s, &p->b);
                  }
                else
                  {
                    if ((!p->inoob) && p->oob_start)
                      p->oob_start (p->s);
                    p->inoob = 1;
                  }
                p->state = G2SS_LOST;

              }
              break;
            case G2SS_1B:
              if ((*ptr) == 0x11)
                {
                  if (p->oob_end)
                    p->oob_end (p->s);
                  p->inoob = 0;
                }

              p->state = G2SS_LOST;
              break;
            }
          red--;
          ptr++;

        }
    }

}

#ifdef INLINE_DISPATCH
void
G2SerDispatch (G2SerialP * p)
{
  serial_dispatch (p);
}
#endif

G2SerialP *
G2CreateSerParser (G2Serial * s,
                   void (*block) (G2Serial *, G2SerBlock *),
                   void (*oob_start) (G2Serial *),
                   void (*oob_data) (G2Serial *, uint8_t *, int),
                   void (*oob_end) (G2Serial *),
                   void (*all_data) (G2Serial *, uint8_t *, int))
{
  G2SerialP *p = (G2SerialP *) malloc (sizeof (G2SerialP));

  p->inoob = 0;
  p->state = G2SS_LOST;

#ifdef ENABLE_MONITOR
  p->monitor_state = 0;
  p->monitor_time.tv_sec=0;
  p->monitor_time.tv_usec=0;
#endif

  p->s = s;
  p->block = block;
  p->oob_start = oob_start;
  p->oob_data = oob_data;
  p->oob_end = oob_end;
  p->all_data = all_data;

  return p;
}

G2DestroySerParser (G2SerialP * p)
{
  if (p->inoob && p->oob_end)
    p->oob_end (p->s);
  free (p);
}


void G2SerAck(G2Serial *s,G2SerBlock *b)
{
  uint8_t ack[6];

  ack[0] = 0x1;             //0x1==ACK
  ack[1] = b->data[7];      //LSB of streamid
  ack[2] = 00;              //don't breakin
  ack[3] = b->data[6];      //Rest of streamid
  ack[4] = b->data[5];
  ack[5] = b->data[4];

  G2SerWrite(s,ack,6);
}

void G2SerIntr(G2Serial *s)
{
  uint8_t intr=0x13;

  G2SerWrite(s,&intr,1);
}

void G2SerNackIntr(G2Serial *s,G2SerBlock *b, int seq)
{
  uint8_t nack[7];

  nack[0] = 0x2;              //0x1==NACK
  nack[1] = b->data[7];       //LSB of streamid
  nack[2] = seq;              //block to nack
  nack[3] = b->data[6];       //Rest of streamid
  nack[4] = b->data[5];
  nack[5] = b->data[4];
  nack[6] = 0x13;

  G2SerWrite(s,nack,7);
}


void G2SerNack(G2Serial *s,G2SerBlock *b, int seq)
{
  uint8_t ack[6];

  ack[0] = 0x2;              //0x1==ACK
  ack[1] = b->data[7];       //LSB of streamid
  ack[2] = seq;              //block to nack
  ack[3] = b->data[6];       //Rest of streamid
  ack[4] = b->data[5];
  ack[5] = b->data[4];

  G2SerWrite(s,ack,6);
}

void G2SerAckIntr(G2Serial *s,G2SerBlock *b)
{
  uint8_t ack[6];

  ack[0] = 0x1;             //0x1==ACK
  ack[1] = b->data[7];      //LSB of streamid
  ack[2] = 0x13;            //breakin
  ack[3] = b->data[6];      //Rest of streamid
  ack[4] = b->data[5];
  ack[5] = b->data[4];

  G2SerWrite(s,ack,6);
}



