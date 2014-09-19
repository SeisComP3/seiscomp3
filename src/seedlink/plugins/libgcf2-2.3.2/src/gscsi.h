/*
 * ./src/gscsi.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gscsi.h,v 1.1 2003/05/16 10:40:25 root Exp $
 */

/*
 * $Log: gscsi.h,v $
 * Revision 1.1  2003/05/16 10:40:25  root
 * *** empty log message ***
 *
 * Revision 1.7  2003/04/16 14:28:02  root
 * #
 *
 * Revision 1.6  2003/04/15 11:04:46  root
 * #
 *
 * Revision 1.5  2003/04/15 08:32:21  root
 * #
 *
 * Revision 1.4  2003/04/01 18:51:10  root
 * #
 *
 * Revision 1.3  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:14  root
 * #
 *
 */

#ifndef __GSCSI_H__
#define __GSCSI_H__

#include "scsilow.h"

#if 0
typedef int64_t G2SCSIPos;
#endif

typedef struct
{
  G2SCSIL *l;
  char *dev;
  uint32_t bsize;               /*Size of a block in bytes */
  uint64_t bshift;              /*number of bits to right shift to turn bytes->blocks */
  uint64_t bmask;               /*(bsize-1) */

  uint64_t blocks;              /*Number of blocks on device */
  uint64_t length;              /*Size of device in bytes */

  char *vendor;
  char *model;
  char *revision;
  char *serial;

  uint8_t *buf;
}
G2SCSI;

extern G2SCSI *G2SCSIOpen (const char *);
extern int G2SCSIRead (G2SCSI * s, G2Offt offst, uint8_t * buf, int len);
extern G2Offt G2SCSILength (G2SCSI * s);
extern void G2SCSIClose (G2SCSI *);


#endif /* __GSCSI_H__ */
