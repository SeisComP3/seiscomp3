/*
 * ./src/scsilow.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: scsilow.c,v 1.4 2004/03/11 17:19:14 root Exp $";

/*
 * $Log: scsilow.c,v $
 * Revision 1.4  2004/03/11 17:19:14  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/10/09 13:01:52  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/28 22:03:43  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:27  root
 * *** empty log message ***
 *
 * Revision 1.11  2003/05/14 16:31:44  root
 * #
 *
 * Revision 1.10  2003/05/13 15:10:01  root
 * #
 *
 * Revision 1.9  2003/05/13 09:16:18  root
 * #
 *
 * Revision 1.8  2003/04/16 14:28:02  root
 * #
 *
 * Revision 1.7  2003/04/15 08:32:22  root
 * #
 *
 * Revision 1.6  2003/04/01 18:51:10  root
 * #
 *
 * Revision 1.5  2003/04/01 18:16:59  root
 * #
 *
 * Revision 1.4  2003/04/01 18:14:24  root
 * #
 *
 * Revision 1.3  2003/04/01 17:55:15  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:15  root
 * #
 *
 */

#include "includes.h"
#include "scsilow.h"

#if defined(HAVE_SCSI_SG_H)

#include <fcntl.h>

#include <sys/ioctl.h>
#include <scsi/sg.h>


G2SCSIL *
G2SCSILOpen (const char *dev)
{
  G2SCSIL *ret;
  int v;
  int fd = open (dev, O_NDELAY | O_RDWR);

  if (fd < 0)
    {
      G2nonfatal ("Failed to open %s", dev);
      return NULL;
    }

  G2SetBlocking (fd);

  if ((ioctl (fd, SG_GET_VERSION_NUM, &v) < 0) || (v < 30000))
    {
      G2nonfatal ("Failed to find SCSI-Generic interface on device %s", dev);
      return NULL;
    }

  ret = malloc (sizeof (G2SCSIL));
  ret->dev = strdup (dev);
  ret->fd = fd;
  return ret;
}

void
G2SCSILClose (G2SCSIL * s)
{
  close (s->fd);
  free (s->dev);
  free (s);
}

int
G2SCSILCmd (G2SCSIL * s, void *cdb, int cdb_len, void *tx_buf, int tx_len,
            void *rx_buf, int rx_len, void *sens_buf, int sens_len, int *st)
{
#ifndef SG_IO
    return -1;

#else
  sg_io_hdr_t sg = { 0 };

  sg.interface_id = 'S';

  sg.cmd_len = cdb_len;
  sg.cmdp = cdb;

  sg.mx_sb_len = sens_len;
  sg.sbp = sens_buf;

  if (tx_buf)
    {
      sg.dxfer_direction = SG_DXFER_TO_DEV;
      sg.dxfer_len = tx_len;
      sg.dxferp = tx_buf;
    }
  else if (rx_buf)
    {
      sg.dxfer_direction = SG_DXFER_FROM_DEV;
      sg.dxfer_len = rx_len;
      sg.dxferp = rx_buf;
    }
  else
    {
      sg.dxfer_direction = SG_DXFER_NONE;
    }

  sg.timeout = 20000;
  sg.status = 0;

  if (ioctl (s->fd, SG_IO, &sg) < 0)
    {
      if (st)
        *st = sg.status;
      return -1;
    }
  if (st)
    *st = sg.status;

  if ((sg.info & SG_INFO_OK_MASK) != SG_INFO_OK)
    return -1;



  return 0;
#endif
}

#elif defined(HAVE_SYS_SCSI_IMPL_USCSI_H)

#include <fcntl.h>
#include <sys/scsi/impl/uscsi.h>


G2SCSIL *
G2SCSILOpen (const char *dev)
{
  G2SCSIL *ret;
  int v;
  int fd = open (dev, O_NDELAY | O_RDWR);

  if (fd < 0)
    {
      G2nonfatal ("Failed to open %s", dev);
      return NULL;
    }

  G2SetBlocking (fd);

  ret = malloc (sizeof (G2SCSIL));
  ret->dev = strdup (dev);
  ret->fd = fd;
  return ret;
}

void
G2SCSILClose (G2SCSIL * s)
{
  close (s->fd);
  free (s->dev);
  free (s);
}


int
G2SCSILCmd (G2SCSIL * s, void *cdb, int cdb_len, void *tx_buf, int tx_len,
            void *rx_buf, int rx_len, void *sens_buf, int sens_len, int *st)
{
  struct uscsi_cmd cmd = { 0 };



  cmd.uscsi_cdblen = cdb_len;
  cmd.uscsi_cdb = cdb;

  cmd.uscsi_rqlen = sens_len;
  cmd.uscsi_rqbuf = sens_buf;

  cmd.uscsi_flags = USCSI_SILENT | USCSI_ISOLATE;

  if (tx_buf)
    {
      cmd.uscsi_flags |= USCSI_WRITE;
      cmd.uscsi_bufaddr = tx_buf;
      cmd.uscsi_buflen = tx_len;
    }
  else if (rx_buf)
    {
      cmd.uscsi_flags |= USCSI_READ;
      cmd.uscsi_bufaddr = rx_buf;
      cmd.uscsi_buflen = rx_len;
    }

  cmd.uscsi_timeout = 20;
  cmd.uscsi_status = 0;

  if (ioctl (s->fd, USCSICMD, &cmd) < 0)
    {
      if (st)
        *st = cmd.uscsi_status;
      return -1;
    }
  if (st)
    *st = cmd.uscsi_status;

  return 0;
}

#else



G2SCSIL *
G2SCSILOpen (const char *dev)
{

  G2nonfatal ("No Lowlevel SCSI support in this binary, can't open %s", dev);
  return NULL;
}

void
G2SCSILClose (G2SCSIL * s)
{
}

int
G2SCSILCmd (G2SCSIL * s, void *cdb, int cdb_len, void *tx_buf, int tx_len,
            void *rx_buf, int rx_len, void *sens_buf, int sens_len, int *st)
{

  return -1;
}


#endif
