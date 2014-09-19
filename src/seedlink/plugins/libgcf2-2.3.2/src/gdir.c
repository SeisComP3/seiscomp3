/*
 * ./src/gdir.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gdir.c,v 1.1 2003/05/16 10:40:23 root Exp $";

/*
 * $Log: gdir.c,v $
 * Revision 1.1  2003/05/16 10:40:23  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.3  2003/05/13 09:16:24  root
 * #
 *
 * Revision 1.2  2003/04/16 14:28:05  root
 * #
 *
 * Revision 1.1  2003/04/15 11:10:19  root
 * #
 *
 */


#include "includes.h"

#include "gdir.h"

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif


G2Dir *
G2DirOpen (const char *path)
{
  G2Dir *ret;
  DIR *d;
  struct dirent *ent;
  G2DirEnt *e;
  G2Offt o = 0;
  int slp = strlen (path);



  ret = (G2Dir *) malloc (sizeof (G2Dir));

  ret->head = ret->tail = NULL;
  ret->current = NULL;
  ret->f = NULL;


  d = opendir (path);
  if (!d)
    return NULL;

  while ((ent = readdir (d)))
    {
      /*Minor hacks here ignore anything which doesn't start a-z and */
      /*ignore anything that starts core.... */
      if ((ent->d_name[0] >= 'a') && (ent->d_name[0] <= 'z')
          && strncmp (ent->d_name, "core", 4))
        {
          char *cpath = (char *) malloc (2 + slp + strlen (ent->d_name));

          strcpy (cpath, path);
          cpath[slp] = '/';
          strcpy (cpath + slp + 1, ent->d_name);

          ret->f = G2SFileOpen (cpath);


          if (ret->f)
            {
              e = (G2DirEnt *) malloc (sizeof (G2DirEnt));

              e->path = cpath;
              e->start = o;
              {
                G2Offt l = G2SFileLength (ret->f);
                if (l % 1024)
                  G2info ("File %s is not a multiple of 1024 blocks long\n",
                          cpath);
                o += l;
              }

              e->end = o;

              e->prev = ret->tail;
              e->next = NULL;
              if (ret->tail)
                ret->tail->next = e;
              ret->tail = e;
              if (!ret->head)
                ret->head = ret->tail;

#if 0
              G2info ("File %p %p %p %s %20lld-%20lld\n", ret->head, e,
                      ret->tail, e->path, (long long) e->start,
                      (long long) e->end);
#endif

              G2SFileClose (ret->f);
              ret->f = NULL;
            }
          else
            {
              G2nonfatal ("G2DirOpen: can't open file %s\n", cpath);
              free (cpath);
            }

        }
      else
        {
          if ((ent->d_name[0] != '.'))
            {
              G2info ("G2DirOpen: Ignoring file %s/%s\n", path, ent->d_name);
            }
        }
    }

  closedir (d);

  ret->length = o;

  if (ret->head == NULL)
    {
      G2nonfatal ("G2DirOpen: no usable files found\n", ent->d_name);
      G2DirClose (ret);
      return NULL;
    }


  return ret;
}


int
G2DirRead (G2Dir * s, G2Offt offst, uint8_t * buf, int len)
{
  G2DirEnt c;

  if (offst >= s->length)
    return 0;

  if (s->current)
    {
      if ((offst >= s->current->start) && (offst < s->current->end))
        {
          return G2SFileRead (s->f, offst - s->current->start, buf, len);
        }
      G2SFileClose (s->f);
      s->f = NULL;
    }

  for (s->current = s->head; s->current; s->current = s->current->next)
    {
      if ((offst >= s->current->start) && (offst < s->current->end))
        {
          s->f = G2SFileOpen (s->current->path);
          if (!s->f)
            {
              G2nonfatal ("G2DirRead: Can't open %s\n", s->current->path);
            }
          return G2SFileRead (s->f, offst - s->current->start, buf, len);
        }
    }

  G2nonfatal ("G2DirRead: Impossible Read past end of directory\n");

  return 0;


}

G2Offt
G2DirLength (G2Dir * s)
{
  return s->length;
}

void
G2DirClose (G2Dir * s)
{
  while (s->head)
    {
      G2DirEnt *c = s->head;
      s->head = c->next;
      free (c->path);
      free (c);
    }

  if (s->f)
    G2SFileClose (s->f);
  free (s);
}
