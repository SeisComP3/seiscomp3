/*
 * ./src/gfile.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gfile.h,v 1.1 2003/05/16 10:40:24 root Exp $
 */

/*
 * $Log: gfile.h,v $
 * Revision 1.1  2003/05/16 10:40:24  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/13 09:16:23  root
 * #
 *
 * Revision 1.2  2003/04/16 14:28:04  root
 * #
 *
 * Revision 1.1  2003/04/15 11:04:48  root
 * #
 *
 */

#ifndef __GFILE_H__
#define __GFILE_H__

#include "gsfile.h"
#include "gscsi.h"
#include "gdir.h"


typedef enum
{
  G2FTSFile,
  G2FTDir,
  G2FTSCSI
}
G2FileType;

typedef struct
{
  G2SCSI *s;
  G2Dir *d;
  G2SFile *f;

  char *path;
  G2FileType t;

  G2Offt ptr, length;

}
G2File;


extern G2File *G2FileOpen (char *);
extern G2Offt G2FileBlocks (G2File * s);
extern G2Offt G2FileLength (G2File * s);
extern G2Offt G2FileTell (G2File * s);
extern G2Offt G2FileSeek (G2File *, G2Offt, int);
extern int G2FileRead (G2File *, void *, int);
extern int G2File1KRead (G2File *, G2Offt, void *, int);
extern void G2FileClose (G2File *);

#endif /*__GFILE_H__*/
