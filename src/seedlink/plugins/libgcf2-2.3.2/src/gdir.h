/*
 * ./src/gdir.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gdir.h,v 1.1 2003/05/16 10:40:23 root Exp $
 */

/*
 * $Log: gdir.h,v $
 * Revision 1.1  2003/05/16 10:40:23  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/13 09:16:24  root
 * #
 *
 * Revision 1.2  2003/04/16 14:28:05  root
 * #
 *
 * Revision 1.1  2003/04/15 11:10:26  root
 * #
 *
 */

#ifndef __GDIR_H__
#define __GDIR_H__

#include "gsfile.h"

typedef struct G2DirEnt_s
{
  struct G2DirEnt_s *prev, *next;
  G2Offt start, end;
  char *path;
}
G2DirEnt;


typedef struct
{
  G2DirEnt *head, *tail;

  G2SFile *f;
  G2DirEnt *current;

  G2Offt length;
}
G2Dir;


extern G2Dir *G2DirOpen (const char *);
extern int G2DirRead (G2Dir * s, G2Offt offst, uint8_t * buf, int len);
extern G2Offt G2DirLength (G2Dir * s);
extern void G2DirClose (G2Dir *);

#endif /* __GDIR_H__ */
