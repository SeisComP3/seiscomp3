/*
 * gserial.c:
 *
 * Copyright (c) 2003 James McKenzie <james@fishsoup.dhs.org>,
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gserial.c,v 1.11 2005/06/14 09:35:01 root Exp $";

/*
 * $Log: gserial.c,v $
 * Revision 1.11  2005/06/14 09:35:01  root
 * *** empty log message ***
 *
 * Revision 1.10  2004/10/29 14:33:39  root
 * *** empty log message ***
 *
 * Revision 1.9  2004/10/29 13:54:10  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/05/06 18:28:55  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/04/14 11:38:30  root
 * *** empty log message ***
 *
 * Revision 1.5  2003/07/03 07:38:50  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/22 14:31:33  root
 * cflags ->  cflag
 *
 * Revision 1.2  2003/05/21 15:09:11  root
 * Comments to gcf2.h.in
 *
 * Revision 1.1  2003/05/16 10:40:25  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/14 16:31:43  root
 * #
 *
 * Revision 1.2  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.1  2003/05/13 09:16:15  root
 * #
 *
 */

#include "includes.h"

#include "gserial.h"

#include <fcntl.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <errno.h>

#define LOCKSIZE 128

#ifndef LOCKDIR
#define LOCKDIR  "/var/lock"
#endif


static char *
get_lockname (char *dev)
{
  static char ret[LOCKSIZE];
#ifdef SVR4_LOCKS

  if (stat (dev, &buf))
    return NULL;
  sprintf (ret, "%s/LK.%03d.%03d.%03d", LOCKDIR, major (stt.st_dev),
           major (stt.st_rdev), minor (stt.st_rdev));

#else
  char *r;

  if (!strncmp (dev, "/dev/", 5))
    {
      dev += 5;
    }
  else
    {
      r = rindex (dev, '/');
      if (r)
        dev = r + 1;
    }

  r = ret + sprintf (ret, "%s/LCK..", LOCKDIR);

  while (*dev)
    {
      if (*dev == '/')
        *(r++) = '_';
      else
        *(r++) = *dev;
      dev++;
    }
  *r = 0;
#endif

  return ret;
}

char *
G2SerLock (char *path)
{
  int fd;
  char buf[128];
  int n;
  int pid;
  mode_t mask;

  char *lock = get_lockname (path);
  if (!lock)
    return NULL;

  buf[sizeof (buf) - 1] = 0;

  if ((fd = open (lock, O_RDONLY)) >= 0)
    {
      n = read (fd, buf, sizeof (buf) - 1);
      close (fd);

      if (n < 0)
        return NULL;

      if (n == sizeof (int))
        {
          pid = *(int *) buf;
        }
      else if (n > 0)
        {
          buf[n] = 0;
          pid = atoi (buf);

          if (pid > 0)
            {
              if ((kill ((pid_t) pid, 0) < 0) && (errno == ESRCH))
                {
                  unlink (lock);
                }
              else
                {
                  return NULL;
                }
            }
          else
            {
              return NULL;
            }
        }
      else
        {
          return NULL;
        }
    }


  mask = umask (022);
  if ((fd = open (lock, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
    {
      return NULL;
    }
  umask (mask);

  n = snprintf (buf, sizeof (buf), "%10ld libgcf2\n", (long) getpid ());
  write (fd, buf, n);
  close (fd);

  return strdup (lock);
}

char *
G2SerUnlock (char *lock)
{
  unlink (lock);
}

void
G2SerClose (G2Serial * g)
{
  if (!g)
    return;
  free (g->name);

  if (!g->isfile)
    tcsetattr (g->fd, TCSAFLUSH, &g->orig_termios);


  close (g->fd);
  if (g->lock)
    G2SerUnlock (g->lock);
  free (g);
}

G2Serial *
G2SerOpen (char *name, int speed, int flags)
{
  G2Serial *ret;
  struct stat st;
  int baud;
  struct termios tios;

  ret = (G2Serial *) malloc (sizeof (G2Serial));
  bzero (ret, sizeof (G2Serial));
  ret->name = strdup (name);
  ret->lock = NULL;

  if (stat (name, &st) < 0)
    return NULL;

  if (S_ISREG (st.st_mode))
    {
      ret->fd = open (name, O_RDWR);
      ret->isfile = 1;

      if (ret->fd < 0)
        {
          G2nonfatal ("Failed to open %s %m", name);
          G2SerClose (ret);
          return NULL;
        }
    }
  else
    {
      ret->fd = open (name, O_RDWR | O_NDELAY | O_NOCTTY);
      if (ret->fd < 0)
        {
          G2nonfatal ("Failed to open %s %m", name);
          G2SerClose (ret);
          return NULL;
        }
    }

  if (ret->isfile)
    return ret;

  tcgetattr (ret->fd, &ret->orig_termios);

  tcgetattr (ret->fd, &tios);

  tios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CLOCAL | CRTSCTS);
  tios.c_cflag |= CLOCAL;
  tios.c_cflag |= CS8;

  tios.c_cflag |= CREAD;        /*| HUPCL; */
  tios.c_iflag = IGNBRK | IGNPAR;

  if (flags)
    tios.c_cflag = flags;

  tios.c_oflag = 0;
  tios.c_lflag = 0;
  tios.c_cc[VMIN] = 1;
  tios.c_cc[VTIME] = 0;

  if (speed)
    {
      switch (speed)
        {
#ifdef B50
        case 50:
          baud = B50;
          break;
#endif
#ifdef B75
        case 75:
          baud = B75;
          break;
#endif
#ifdef B110
        case 110:
          baud = B110;
          break;
#endif
#ifdef B134
        case 134:
          baud = B134;
          break;
#endif
#ifdef B150
        case 150:
          baud = B150;
          break;
#endif
#ifdef B200
        case 200:
          baud = B200;
          break;
#endif
#ifdef B300
        case 300:
          baud = B300;
          break;
#endif
#ifdef B600
        case 600:
          baud = B600;
          break;
#endif
#ifdef B1200
        case 1200:
          baud = B1200;
          break;
#endif
#ifdef B1800
        case 1800:
          baud = B1800;
          break;
#endif
#ifdef B2400
        case 2400:
          baud = B2400;
          break;
#endif
#ifdef B4800
        case 4800:
          baud = B4800;
          break;
#endif
#ifdef B9600
        case 9600:
          baud = B9600;
          break;
#endif
#ifdef B19200
        case 19200:
          baud = B19200;
          break;
#endif
#ifdef B38400
        case 38400:
          baud = B38400;
          break;
#endif
#ifdef B57600
        case 57600:
          baud = B57600;
          break;
#endif
#ifdef B115200
        case 115200:
          baud = B115200;
          break;
#endif
#ifdef B230400
        case 230400:
          baud = B230400;
          break;
#endif
#ifdef B460800
        case 460800:
          baud = B460800;
          break;
#endif
#ifdef B500000
        case 500000:
          baud = B500000;
          break;
#endif
#ifdef B576000
        case 576000:
          baud = B576000;
          break;
#endif
#ifdef B921600
        case 921600:
          baud = B921600;
          break;
#endif
#ifdef B1000000
        case 1000000:
          baud = B1000000;
          break;
#endif
#ifdef B1152000
        case 1152000:
          baud = B1152000;
          break;
#endif
#ifdef B1500000
        case 1500000:
          baud = B1500000;
          break;
#endif
#ifdef B2000000
        case 2000000:
          baud = B2000000;
          break;
#endif
#ifdef B2500000
        case 2500000:
          baud = B2500000;
          break;
#endif
#ifdef B3000000
        case 3000000:
          baud = B3000000;
          break;
#endif
#ifdef B3500000
        case 3500000:
          baud = B3500000;
          break;
#endif
#ifdef B4000000
        case 4000000:
          baud = B4000000;
          break;
#endif
        case 0:
          baud = 0;
          break;
        default:
          G2nonfatal ("%s: this platform doesn't support a speed of %d",
                      ret->name, speed);
          G2SerClose (ret);
          return NULL;
        }

      if (baud)
        {
          cfsetospeed (&tios, baud);
          cfsetispeed (&tios, baud);
        }
    }

  if (tcsetattr (ret->fd, TCSAFLUSH, &tios) < 0)
    {
      G2nonfatal ("%s: failed to set serial port parameters", ret->name);
      G2SerClose (ret);
      return NULL;
    }

  G2SetBlocking (ret->fd);

  return ret;
}


G2Serial *
G2SerOpenLock (char *name, int speed, int flags, int timeout)
{
  G2Serial *ret;
  uint8_t *lock = G2SerLock (name);

  while ((!lock) && (timeout--))
    {
      sleep (1);
      lock = G2SerLock (name);
    }

  if (!lock)
    return NULL;

  ret = G2SerOpen (name, speed, flags);

  if (!ret)
    {
      G2SerUnlock (lock);
      return NULL;
    }

  ret->lock = lock;

  return ret;
}

int
G2SerData (G2Serial * s)
{
  if (s->isfile)
    return 1;
  return G2CanRead (s->fd);
}

int
G2SerDrain (G2Serial * s, int timeout)
{
  struct timeval now, then, diff;
  unsigned char c;

  if (s->isfile)
    return -1;

  gettimeofday (&then, NULL);

  G2Sleep (500000);

  while (G2CanRead (s->fd))
    {
      read (s->fd, &c, 1);

      gettimeofday (&now, NULL);

      timersub (&now, &then, &diff);

      if (diff.tv_sec > timeout)
        return -1;
    }

  return 0;
}

#ifdef DEBUGGING
static int slow=0;
#endif

int
G2SerWrite (G2Serial * s, void *buf, int buflen)
{
  int red;

  if (s->isfile)
    return -1;


#ifdef DEBUGGING
if (!slow) {
  printf("<b>To DM --- ");
  {
  uint8_t *foo=buf;
  int i=buflen;


  while(i--) {
   if (((*foo)<32) || ((*foo)>126)) {
		printf("\\x%02x",*foo);
   } else {
	printf("%c",*foo);
   } 
   foo++;
  }
  }
  printf("</b><BR>");
}
#endif

  return write (s->fd, buf, buflen);

}

int
G2SerWriteStrSlow (G2Serial * s, char *buf)
{
  int len = strlen (buf);
  int ret = 0;
  int writ;

#ifdef DEBUGGING
  slow=1;
  printf("<b>To DMSlow --- ");
  {
  uint8_t *foo=buf;
  int i=len;


  while(i--) {
   if (((*foo)<32) || ((*foo)>126)) {
		printf("\\x%02x",*foo);
   } else {
	printf("%c",*foo);
   } 
   foo++;
  }
  }
  printf("</b><BR>");
#endif


  while (len)
    {
      G2Sleep (1000);
      writ = G2SerWrite (s, buf, len > 4 ? 4 : len);
      if (writ <= 0)
        {
#ifdef DEBUGGING
  	slow=0;
#endif
          if (ret)
            return ret;
          else
            return writ;
        }

      ret += writ;
      buf += writ;
      len -= writ;
    }

#ifdef DEBUGGING
  slow=0;
#endif
  return ret;
}


int
G2SerRead (G2Serial * s, void *buf, int buflen)
{
  int red;

  red = read (s->fd, buf, buflen);

  if (s->isfile)
    if (red != buflen)
      s->eof++;

#ifdef DEBUGGING
printf("<i>To DCM --- ");
  {
  uint8_t *foo=buf;
  int i=red;


  while(i--) {
   if (((*foo)<32) || ((*foo)>126)) {
		printf("\\x%02x",*foo);
   } else {
	printf("%c",*foo);
   } 
   foo++;
  }
  }
  printf("</i><BR>");
#endif

  return red;
}
