/*
 * ./src/scsilow.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: scsilow.h,v 1.1 2003/05/16 10:40:28 root Exp $
 */

/*
 * $Log: scsilow.h,v $
 * Revision 1.1  2003/05/16 10:40:28  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/04/16 14:28:03  root
 * #
 *
 * Revision 1.3  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:15  root
 * #
 *
 */

#ifndef __SCSILOW_H__
#define __SCSILOW_H__

typedef struct
{
  char *dev;
  int fd;
}
G2SCSIL;

extern G2SCSIL *G2SCSILOpen (const char *);
extern void G2SCSILClose (G2SCSIL *);
extern int G2SCSILCmd (G2SCSIL *, void *, int, void *, int, void *, int,
                       void *, int, int *);
#endif /* __SCSILOW_H__ */
