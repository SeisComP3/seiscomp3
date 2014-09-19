/*
 * ./src/gfile.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gfile.c,v 1.2 2003/05/28 22:03:42 root Exp $";

/*
 * $Log: gfile.c,v $
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:24  root
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
 * Revision 1.1  2003/04/15 11:10:18  root
 * #
 *
 */

/*TODO: Reads need some form of caching */

#include "includes.h"

#include "gfile.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

G2File *
G2FileOpen (char *path)
{
  struct stat st;
  G2File *f;

  if (stat (path, &st) < 0)
    return NULL;

  f = malloc (sizeof (G2File));
  bzero (f, sizeof (G2File));

  f->path = strdup (path);
  f->ptr = 0;

  if (S_ISREG (st.st_mode))
    {

      /*It's a regular file */
      f->t = G2FTSFile;
      f->f = G2SFileOpen (path);


      if (f->f)
        {
          f->length = G2SFileLength (f->f);
          return f;
        }

      G2FileClose (f);
      G2nonfatal ("Can't open regular file %s\n", path);
      return NULL;

    }

  if (S_ISDIR (st.st_mode))
    {
      /*It's a directory */
      f->t = G2FTDir;
      f->d = G2DirOpen (path);

      if (f->d)
        {
          f->length = G2DirLength (f->d);
          return f;
        }

      G2FileClose (f);
      G2nonfatal ("Can't open directory %s\n", path);
      return NULL;
    }

/* Hmm we have something else - try opening it as a regular file*/
/* some OSsen/disks are ok with this*/

  f->t = G2FTSFile;
  f->f = G2SFileOpen (path);
  if (f->f)
    {
      f->length = G2SFileLength (f->f);

      return f;
    }
  G2nonfatal ("Couldn't open %s as a regular file, trying as a SCSI device\n",
              path);

  f->t = G2FTSCSI;
  f->s = G2SCSIOpen (path);


  if (f->s)
    {
      f->length = G2SCSILength (f->s);

      return f;
    }
  G2nonfatal ("Failed to find any method of opening %s\n", path);
  G2FileClose (f);
  return NULL;
}


G2Offt
G2FileTell (G2File * s)
{
  return s->ptr;
}

G2Offt
G2FileLength (G2File * s)
{
  return s->length;
}

G2Offt
G2FileBlocks (G2File * s)
{
  return s->length >> 10;
}

G2Offt
G2FileSeek (G2File * s, G2Offt where, int whence)
{
  switch (whence)
    {
    case SEEK_SET:
      s->ptr = where;
      break;
    case SEEK_END:
      s->ptr = s->length + where;
      break;
    case SEEK_CUR:
      s->ptr += where;
      break;
    default:
      return -1;
    }

  if (s->ptr < 0)
    {
      s->ptr = 0;
      return -1;
    }

  if (s->ptr >= s->length)
    {
      s->ptr = s->length;
      return -1;
    }

  return s->ptr;
}

int
G2FileRead (G2File * s, void *buf, int len)
{
  G2Offt red;

  switch (s->t)
    {
    case G2FTSFile:
      red = G2SFileRead (s->f, s->ptr, (uint8_t *) buf, len);
      break;
    case G2FTDir:
      red = G2DirRead (s->d, s->ptr, (uint8_t *) buf, len);
      break;
    case G2FTSCSI:
      red = G2SCSIRead (s->s, s->ptr, (uint8_t *) buf, len);
      break;
    }

  if (red > 0)
    s->ptr += red;

  return red;
}

int
G2File1KRead (G2File * s, G2Offt block, void *buf, int nblocks)
{
  G2Offt red;

  switch (s->t)
    {
    case G2FTSFile:
      red = G2SFileRead (s->f, block << 10, (uint8_t *) buf, nblocks << 10);
      break;
    case G2FTDir:
      red = G2DirRead (s->d, block << 10, (uint8_t *) buf, nblocks << 10);
      break;
    case G2FTSCSI:
      red = G2SCSIRead (s->s, block << 10, (uint8_t *) buf, nblocks << 10);
      break;
    }

  if (red > 0)
    red >>= 10;

  return red;

}

void
G2FileClose (G2File * f)
{
  if (f->s)
    G2SCSIClose (f->s);
  if (f->d)
    G2DirClose (f->d);
  if (f->f)
    G2SFileClose (f->f);

  if (f->path)
    free (f->path);

  free (f);
}
