/*
 * ./src/gsfile.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gsfile.c,v 1.2 2003/05/28 22:03:42 root Exp $";

/*
 * $Log: gsfile.c,v $
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:26  root
 * *** empty log message ***
 *
 * Revision 1.5  2003/05/14 16:31:47  root
 * #
 *
 * Revision 1.4  2003/05/13 15:10:04  root
 * #
 *
 * Revision 1.3  2003/05/13 09:16:23  root
 * #
 *
 * Revision 1.2  2003/04/16 14:28:04  root
 * #
 *
 * Revision 1.1  2003/04/15 11:10:26  root
 * #
 *
 */


#include "includes.h"

#include "gfile.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

G2SFile *
G2SFileOpen (const char *path)
{
  G2SFile *ret;
  int fd;
  G2Offt len;

  fd = open (path, O_RDONLY);
  if (fd < 0)
    return NULL;

  len = lseek (fd, 0, SEEK_END);
  if (len <= 0)
    return 0;

  ret = (G2SFile *) malloc (sizeof (G2SFile));
  ret->fd = fd;
  ret->length = len;
  return ret;
}

int
G2SFileRead (G2SFile * s, G2Offt offst, uint8_t * buf, int len)
{
  if (lseek (s->fd, offst, SEEK_SET) != offst)
    return -1;

  return read (s->fd, buf, len);
}

G2Offt
G2SFileLength (G2SFile * s)
{
  return s->length;
}

void
G2SFileClose (G2SFile * s)
{
  close (s->fd);
  free (s);
}
