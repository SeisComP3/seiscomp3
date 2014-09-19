/*
 * gblock.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gblock.h,v 1.1 2003/05/16 10:40:22 root Exp $
 */

/*
 * $Log: gblock.h,v $
 * Revision 1.1  2003/05/16 10:40:22  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:48  root
 * #
 *
 * Revision 1.1  2003/05/13 15:10:05  root
 * #
 *
 */

#ifndef __GBLOCK_H__
#define __GBLOCK_H__

#include "gtime.h"
#include "gfile.h"

#include <stdio.h>

typedef struct
{
  uint8_t data[1024];
  int size;
}
G2Block;

extern int G2transcode24to32 (G2Block * in, G2Block * out);
extern int G2transcode32to24 (G2Block * in, G2Block * out);
extern int G2FileReadBlock (G2File * s, G2Block * out);
extern int G2FileRead1KBlock (G2File * s, G2Offt block, G2Block * out);
extern int G2DumpBlock (FILE * s, G2Block * in);

#endif /* __GBLOCK_H__ */
