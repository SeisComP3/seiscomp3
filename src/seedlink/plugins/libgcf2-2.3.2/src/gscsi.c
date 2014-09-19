/*
 * ./src/gscsi.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gscsi.c,v 1.1 2003/05/16 10:40:24 root Exp $";

/*
 * $Log: gscsi.c,v $
 * Revision 1.1  2003/05/16 10:40:24  root
 * *** empty log message ***
 *
 * Revision 1.12  2003/05/13 15:10:02  root
 * #
 *
 * Revision 1.11  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.10  2003/04/16 14:28:03  root
 * #
 *
 * Revision 1.9  2003/04/15 11:04:47  root
 * #
 *
 * Revision 1.8  2003/04/15 08:32:23  root
 * #
 *
 * Revision 1.7  2003/04/01 18:51:10  root
 * #
 *
 * Revision 1.6  2003/04/01 18:17:47  root
 * #
 *
 * Revision 1.5  2003/04/01 18:14:24  root
 * #
 *
 * Revision 1.4  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.3  2003/04/01 17:55:15  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:17  root
 * #
 *
 */

#include "includes.h"

#include "gscsi.h"
#include "scsireg.h"


#define BUF_SIZE	32768
#define MAX_READ	32768


static int
inquiry (G2SCSI * g)
{
  uint8_t rxbuf[255];
  uint8_t snbuf[256];
  uint8_t cdb[6] = { SCSI_OP_INQUIRY, 0, 0, 0, 255, 0 };
  int tries = 0;


/*Incase we have just reset*/
  G2SCSILCmd (g->l, cdb, sizeof (cdb), NULL, 0, rxbuf, sizeof (rxbuf), NULL,
              0, NULL);


  G2bzero (rxbuf, sizeof (rxbuf));

  while (G2SCSILCmd
         (g->l, cdb, sizeof (cdb), NULL, 0, rxbuf, sizeof (rxbuf), snbuf,
          sizeof (snbuf), NULL))
    {
      G2nonfatal ("%s: SCSI INQUIRY command failed", g->dev);
      if ((tries++) > 3)
        return -1;
    }


  g->vendor = G2strdupandnull (rxbuf + 8, 8);
  g->model = G2strdupandnull (rxbuf + 16, 16);
  g->revision = G2strdupandnull (rxbuf + 32, 4);
  g->serial = G2strdupandnull (rxbuf + 36, 20);


  if (((snbuf[0] >> 5) & 7) == 1)
    {
      G2nonfatal ("%s: SCSI device OFFLINE", g->dev);
      return -1;
    }

  switch (rxbuf[0] & 0x1f)
    {
    case SCSI_TYPE_DISK:
    case SCSI_TYPE_WORM:
    case SCSI_TYPE_ROM:
    case SCSI_TYPE_MOD:
      break;
    case SCSI_TYPE_TAPE:
      G2nonfatal ("%s: Tapes not (yet) supported", g->dev);
      return -1;
    default:
      G2nonfatal ("%s: SCSI device type 0x%x not supported", g->dev,
                  rxbuf[0] & 0x1f);
      return -1;
    }

  return 0;
}

static int
readcapacity (G2SCSI * g)
{
  uint8_t cdb[] = { SCSI_OP_READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  uint8_t buf[8];

  G2bzero (buf, sizeof (buf));

  if (G2SCSILCmd
      (g->l, cdb, sizeof (cdb), NULL, 0, buf, sizeof (buf), NULL, 0, NULL))
    {
      G2nonfatal ("%s: SCSI READ_CAPACITY command failed", g->dev);
      return -1;
    }

  g->blocks = 1;
  g->blocks += ((uint64_t) buf[0]) << 24ULL;
  g->blocks += ((uint64_t) buf[1]) << 16ULL;
  g->blocks += ((uint64_t) buf[2]) << 8ULL;
  g->blocks += (uint64_t) buf[3];

  g->bsize = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
  g->bmask = g->bsize;
  g->bmask--;

  {
    uint32_t bs = g->bsize;
    g->bshift = 0;
    while (bs >>= 1)
      g->bshift++;
  }

  if ((1 << g->bshift) != g->bsize)
    {
      G2nonfatal ("%s: bsize of %d is not a power of 2", g->dev);
      return -1;
    }


  g->length = g->blocks * (uint64_t) g->bsize;

  return 0;
}


static int
spinup (G2SCSI * g)
{
  uint8_t cdb[] = { SCSI_OP_TEST_UNIT_READY, 0, 0, 0, 0, 0 };
  uint8_t snbuf[256];
  int tries = 0;

  for (;;)
    {
      G2bzero (snbuf, sizeof (snbuf));
      if ((!G2SCSILCmd
           (g->l, cdb, sizeof (cdb), NULL, 0, NULL, 0, snbuf, sizeof (snbuf),
            NULL)) && (snbuf[2] != SCSI_SK_UNIT_ATTENTION))
        break;

      if ((tries++) > 3)
        {
          G2nonfatal ("%s: SCSI TEST_UNIT_READY command failed", g->dev);
          return -1;
        }
    }

  if (snbuf[2] == SCSI_SK_NOT_READY)
    {
      uint8_t ss[6] = { SCSI_OP_START_STOP, 0, 0, 0, 1, 0 };
      G2bzero (snbuf, sizeof (snbuf));
      G2info ("%s: spinning up disk", g->dev);
      if (G2SCSILCmd
          (g->l, ss, sizeof (ss), NULL, 0, NULL, 0, snbuf, sizeof (snbuf),
           NULL))
        {
          G2nonfatal ("%s: SCSI START_STOP command failed", g->dev);
          return -1;
        }
    }

  return 0;
}

/*FIXME: - this can only do reads upto 1Tb */

static int
scsiread (G2SCSI * s, void *buf, uint32_t block, uint32_t count)
{
  uint8_t cdb[10];
  int cdblen;
  int len = count * s->bsize;

  if (count > 0xffff)
    count = 0xffff;

  if ((block > 0x1fffff) || (count > 0xff))
    {
      cdb[0] = SCSI_OP_READ_10;
      cdb[1] = 0;
      cdb[2] = (block >> 24) & 0xff;
      cdb[3] = (block >> 16) & 0xff;
      cdb[4] = (block >> 8) & 0xff;
      cdb[5] = block & 0xff;
      cdb[6] = cdb[9] = 0;
      cdb[7] = (count >> 8) & 0xff;
      cdb[8] = count & 0xff;
      cdblen = 10;
    }
  else
    {
      cdb[0] = SCSI_OP_READ_6;
      cdb[1] = (block >> 16) & 0x1f;
      cdb[2] = (block >> 8) & 0xff;
      cdb[3] = block & 0xff;
      cdb[4] = count;
      cdb[5] = 0;
      cdblen = 6;
    }

  if (G2SCSILCmd (s->l, cdb, cdblen, NULL, 0, buf, len, NULL, 0, NULL))
    {
      G2nonfatal ("%s: read of sector %d failed", s->dev, block);
      return 0;
    }

  return count;
}


/************* API ***********/


void
G2SCSIClose (G2SCSI * s)
{
  if (!s)
    return;
  if (s->l)
    G2SCSILClose (s->l);
  if (s->vendor)
    free (s->vendor);
  if (s->model)
    free (s->model);
  if (s->revision)
    free (s->revision);
  if (s->serial)
    free (s->serial);
  free (s);
}


G2SCSI *
G2SCSIOpen (const char *dev)
{
  G2SCSIL *l = G2SCSILOpen (dev);
  G2SCSI *ret;

  if (!l)
    return NULL;

  ret = malloc (sizeof (G2SCSI));
  bzero (ret, sizeof (G2SCSI));

  ret->dev = strdup (dev);
  ret->l = l;

  if (inquiry (ret))
    {
      G2SCSIClose (ret);
      return NULL;
    }


  if (spinup (ret))
    {
      G2SCSIClose (ret);
      return NULL;
    }

  if (readcapacity (ret))
    {
      G2SCSIClose (ret);
      return NULL;
    }

  G2info ("%s: %s %s %s (%s) %s\n", ret->dev, ret->vendor, ret->model,
          ret->revision, ret->serial, G2strsize (ret->length));

  ret->buf = malloc (BUF_SIZE);

  return ret;
}

/*Do a Possibly unaligned read*/
int
G2SCSIRead (G2SCSI * s, G2Offt offst, uint8_t * buf, int len)
{
  uint64_t t;
  int red = 0;
  uint32_t bo, b;

  t = offst & ~s->bmask;

  bo = (offst - t);
  b = t >> s->bshift;

  if (bo)
    {                           /* We have slop at the front end */
      int tlen = s->bsize - bo;

      if (scsiread (s, s->buf, b, 1) != 1)
        {
          return 0;
        }

      memcpy (buf, s->buf + bo, tlen);

      b++;
      len -= tlen;
      red += tlen;
      buf += tlen;

    }


  while (len >= s->bsize)
    {
      int tlen = ((len > MAX_READ) ? MAX_READ : len) & ~s->bmask;
      int tc = tlen >> s->bshift;

      if (scsiread (s, buf, b, tc) != tc)
        {
          return red;
        }

      b += tc;
      len -= tlen;
      buf += tlen;
      red += tlen;
    }

  if (len)
    {                           /*Ok we have some slop at the back end */

      if (scsiread (s, s->buf, b, 1) != 1)
        {
          return red;
        }

      memcpy (buf, s->buf, len);

      red += len;
    }

  return red;
}


G2Offt
G2SCSILength (G2SCSI * s)
{
  return s->length;
}






#if 0
G2SCSIPos
G2SCSISeek (G2SCSI * s, G2SCSIPos where, int whence)
{
  G2SCSIPos p;

  switch (whence)
    {
    case SEEK_SET:
      p = where;
      break;
    case SEEK_END:
      p = s->length + where;
      break;
    case SEEK_CUR:
      p = s->bptr + where;
      break;
    default:
      return -1;
    }
  if (p & s->bmask)
    return -1;
  if (p > s->length)
    p = s->length;

  s->bptr = p;

  return p;
}

#endif

#if 0
int
G2SCSIBRead (G2SCSI * s, void *buf, int len)
{
  if (len & s->bmask)
    return -1;

  if (len > MAX_READ)
    return -1;

//FIXME: scsiread currently returns 0 or all , but it might not
  if (!scsiread (s, buf, s->bptr >> s->bshift, len >> s->bshift))
    {
      return -1;
    }

  s->bptr += s->bshift;

  return len;
}
#endif
