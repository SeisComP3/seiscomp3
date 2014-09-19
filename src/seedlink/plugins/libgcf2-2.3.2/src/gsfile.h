/*
 * ./src/gsfile.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gsfile.h,v 1.1 2003/05/16 10:40:26 root Exp $
 */

/*
 * $Log: gsfile.h,v $
 * Revision 1.1  2003/05/16 10:40:26  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/04/16 14:28:05  root
 * #
 *
 * Revision 1.1  2003/04/15 11:10:19  root
 * #
 *
 */

#ifndef __GSFILE_H__
#define __GSFILE_H__

typedef struct
{
  int fd;
  int length;
}
G2SFile;


extern G2SFile *G2SFileOpen (const char *);
extern int G2SFileRead (G2SFile * s, G2Offt offst, uint8_t * buf, int len);
extern G2Offt G2SFileLength (G2SFile * s);
extern void G2SFileClose (G2SFile *);

#endif /* __GSFILE_H__ */
