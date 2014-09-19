/*
 * ./src/util.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: util.c,v 1.3 2004/04/14 15:39:52 root Exp $";

/*
 * $Log: util.c,v $
 * Revision 1.3  2004/04/14 15:39:52  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/14 11:38:30  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:28  root
 * *** empty log message ***
 *
 * Revision 1.6  2003/05/14 16:31:45  root
 * #
 *
 * Revision 1.5  2003/05/13 09:16:18  root
 * #
 *
 * Revision 1.4  2003/04/16 14:28:03  root
 * #
 *
 * Revision 1.3  2003/04/15 11:04:47  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:16  root
 * #
 *
 */

#include "includes.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include  <sys/select.h>
#endif

void
G2msg (const char *file, int line, const char *function, G2MsgAction action,
       const char *fmt, ...)
{
  va_list ap;
  char where[1024];
  char buf[1024];
  int len;

  bzero (buf, sizeof (buf));
  bzero (where, sizeof (where));

  snprintf (where, sizeof (where), "%s:%d(%s):", file, line, function);

  va_start (ap, fmt);
  len = vsnprintf (buf, sizeof (buf) - 1, fmt, ap);
  va_end (ap);


  if ((size_t) len > sizeof (buf) - 2 || len < 0)
    {
      /* Handle this better malloc? */
      len = sizeof (buf) - 2;
      buf[len] = '\0';
    }

  if (!len)
    len = strlen (buf);

  if (buf[len - 1] != '\n')
    {
      buf[len] = '\n';
      len++;
    }

  buf[len] = '\0';

  fputs (where, stderr);
  fputs (buf, stderr);
  switch (action)
    {
    case G2MsgFatal:
      abort ();
      break;
    case G2MsgNonFatal:
    case G2MsgInfo:
    case G2MsgWarning:
      break;
    }



}

void
G2SetBlocking (int fd)
{
  long arg;
  arg = fcntl (fd, F_GETFL, arg);
  arg &= ~O_NONBLOCK;
  fcntl (fd, F_SETFL, arg);
}


void
G2ClrBlocking (int fd)
{
  long arg;
  arg = fcntl (fd, F_GETFL, arg);
  arg |= O_NONBLOCK;
  fcntl (fd, F_SETFL, arg);
}

int
G2CanRead (int fd)
{
  fd_set rfd;
  struct timeval tv = { 0 };

  FD_ZERO (&rfd);
  FD_SET (fd, &rfd);

  return select (fd + 1, &rfd, NULL, NULL, &tv);
}



char *
G2strsize (uint64_t s)
{
  static char ret[128];
  double d = (double) s;

  if (s < 1024ULL)
    {
      sprintf (ret, "%d bytes", (int) s);
    }
  else if (s < 1048576ULL)
    {
      sprintf (ret, "%.1f Kb", d / 1024.0);
    }
  else if (s < 1073741824ULL)
    {
      sprintf (ret, "%.1f Mb", d / 1048576.0);
    }
  else if (s < 1099511627776ULL)
    {
      sprintf (ret, "%.1f Gb", d / 1073741824.0);
    }
  else
    {
      sprintf (ret, "%.1f Tb", d / 1099511627776.0);
    }
  return ret;
}





char *
G2strdupandnull (const char *u, int n)
{
  char *ret = malloc (n + 1), *ptr = ret;
  int i = n;
  if (!u)
    {
      *ret = 0;
      return ret;
    }

  while (i--)
    {
      if (((*u) > 31) && ((*u) < 127))
        *(ptr++) = *u;
      if (!*u)
        break;
      u++;
    }
  *ptr = 0;
  return ret;
}


void
G2bzero (void *v, int n)
{
  memset (v, 0, n);
}

void
hexdump (FILE * st, void *data, int os, int oe)
{
  uint8_t *d = (uint8_t *) data;
  int s, e;
  int i;

  oe += os;

  s = os & 15;
  e = (oe - 1) | 15;
  e++;

  for (i = s; i < e; ++i)
    {
      if (!(i & 15))
        {
          fprintf (st, "%06x:", i);
        }
      if ((i < os) || (i >= oe))
        {
          fprintf (st, "   ");
        }
      else
        {
          fprintf (st, " %02x", d[i]);
        }
      if ((i & 15) == 15)
        {
          fprintf (st, "\n");
        }
    }

}

void G2Sleep(int us)
{
struct timeval tv={0,us};
select(0,NULL,NULL,NULL,&tv);
}
