/*
 * gnet.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: gnet.c,v 1.9 2006/09/11 13:11:07 lwithers Exp $";

/*
 * $Log: gnet.c,v $
 * Revision 1.9  2006/09/11 13:11:07  lwithers
 * Don't fail on -EINPROGRESS when opening a TCP connection in non-blocking
 * mode.
 *
 * Revision 1.8  2006/08/30 15:32:55  lwithers
 * Don't crash.
 *
 * Revision 1.7  2006/04/21 10:58:14  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/07/08 08:54:05  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/15 22:35:45  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/06/24 16:33:35  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/06/06 16:10:53  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/06/06 16:03:36  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/06/06 09:38:10  root
 * *** empty log message ***
 *
 */

#include "gnet.h"

#ifdef __APPLE__
    #include <stdlib.h>
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif

#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define DEFAULT_PORT 1567

int
G2NetLookup (char *namecolonport, G2NetAddr * a)
{
  char *cpy = strdup (namecolonport);
  char *colon = index (cpy, ':');
  struct hostent *he;

  memset (a, 0, sizeof (G2NetAddr));

  a->sin.sin_family = AF_INET;

  if (colon)
    {
      *colon = 0;
      a->sin.sin_port = htons (atoi (colon + 1));
    }
  else
    {
      a->sin.sin_port = htons (DEFAULT_PORT);
    }

  if (inet_aton (cpy, &a->sin.sin_addr))
    {
      free (cpy);
      return 0;
    }

/*FIXME: May block*/
  if ((!(he = gethostbyname (cpy))) || (he->h_addrtype != AF_INET)
      || (he->h_length != sizeof (a->sin.sin_addr)))
    {
      free (cpy);
      return 1;
    }

  memcpy (&a->sin.sin_addr, he->h_addr, sizeof (a->sin.sin_addr));

  return 0;
}

int
G2NetPort (G2Net * ret)
{
  return ntohs (ret->sin.sin_port);
}

G2Net *
G2NetOpenUDP (int port)
{
  G2Net *ret = (G2Net *) malloc (sizeof (G2Net));

  memset (&ret->sin, 0, sizeof (ret->sin));

  ret->sin.sin_family = AF_INET;
  ret->sin.sin_port = htons (port);
  ret->sin.sin_addr.s_addr = htonl (INADDR_ANY);

  ret->fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (ret->fd < 0)
    {
      free (ret);
      return NULL;
    }

  {
    int one = 1;

    setsockopt (ret->fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one));
  }


  if (bind (ret->fd, (struct sockaddr *) &ret->sin, sizeof (ret->sin)))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }


#ifdef SO_BSDCOMPAT
{
/*Linux does RFC1122 delayed error notification - it's annonying */
/*turn it off*/
int one=1;

  setsockopt(ret->fd,SOL_SOCKET,SO_BSDCOMPAT,&one,sizeof(one));
}
#endif

  return ret;
}


G2Net *
G2NetOpenUDPB (int port)
{
  G2Net *ret = G2NetOpenUDP (port);
  int i = 1;

  if (!ret)
    return ret;

  if (setsockopt (ret->fd, SOL_SOCKET, SO_BROADCAST, &i, sizeof (i)))
    {
      G2NetClose (ret);
      return NULL;
    }
  return ret;
}


G2Net *
G2NetOpenTCPC (G2NetAddr * remote)
{
  G2Net *ret = (G2Net *) malloc (sizeof (G2Net));

  memset (&ret->sin, 0, sizeof (ret->sin));

  ret->sin.sin_family = AF_INET;
  ret->sin.sin_port = htons (0);
  ret->sin.sin_addr.s_addr = htonl (INADDR_ANY);

  ret->fd = socket (PF_INET, SOCK_STREAM, 0);
  if (ret->fd < 0)
    {
      free (ret);
      return NULL;
    }

  {
    int one = 1;

    setsockopt (ret->fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one));
  }

  if (bind (ret->fd, (struct sockaddr *) &ret->sin, sizeof (ret->sin)))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

  if (connect
      (ret->fd, (struct sockaddr *) &remote->sin,
       sizeof (struct sockaddr_in)))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

  return ret;
}


G2Net *
G2NetOpenTCPCNB (G2NetAddr * remote)
{
  G2Net *ret = (G2Net *) malloc (sizeof (G2Net));

  memset (&ret->sin, 0, sizeof (ret->sin));

  ret->sin.sin_family = AF_INET;
  ret->sin.sin_port = htons (0);
  ret->sin.sin_addr.s_addr = htonl (INADDR_ANY);

  ret->fd = socket (PF_INET, SOCK_STREAM, 0);
  if (ret->fd < 0)
    {
      free (ret);
      return NULL;
    }

  {
    int one = 1;

    setsockopt (ret->fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one));
  }
  if (bind (ret->fd, (struct sockaddr *) &ret->sin, sizeof (ret->sin)))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

  G2ClrBlocking (ret->fd);

  if (connect
      (ret->fd, (struct sockaddr *) &remote->sin,
       sizeof (struct sockaddr_in)) && errno != EINPROGRESS)
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

  return ret;
}

int
G2NetOpenTCPCNBE (G2Net * ret)
{
  int err;
  int len = sizeof (err);
  if (getsockopt (ret->fd, SOL_SOCKET, SO_ERROR, &err, &len))
    {
      G2NetClose (ret);
      return -1;
    }

  if (err)
    {
      G2NetClose (ret);
      return -1;
    }
  G2SetBlocking (ret->fd);

  return 0;
}


G2Net *G2NetAccept(G2Net *tcps,G2NetAddr *from)
{
int fromlen=from ?sizeof(struct sockaddr_in):0;
G2Net *ret=(G2Net *) malloc (sizeof (G2Net));


ret->fd=accept(tcps->fd,from ? ((struct sockaddr *) &from->sin):(struct sockaddr *) 0 ,&fromlen);

if (ret->fd<0) {
	free(ret);
	return NULL;
}

G2ClrBlocking(ret->fd);

ret->eof=0;

return ret;
}

void G2NetSetBlocking(G2Net *n)
{
G2SetBlocking(n->fd);
}

void G2NetClrBlocking(G2Net *n)
{
G2ClrBlocking(n->fd);
}


G2Net *
G2NetOpenTCPS (int port)
{
  G2Net *ret = (G2Net *) malloc (sizeof (G2Net));

  memset (&ret->sin, 0, sizeof (ret->sin));

  ret->sin.sin_family = AF_INET;
  ret->sin.sin_port = htons (port);
  ret->sin.sin_addr.s_addr = htonl (INADDR_ANY);

  ret->fd = socket (PF_INET, SOCK_STREAM, 0);
  if (ret->fd < 0)
    {
      free (ret);
      return NULL;
    }
  {
    int one = 1;

    setsockopt (ret->fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one));
  }

  if (bind (ret->fd, (struct sockaddr *) &ret->sin, sizeof (ret->sin)))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

  if (listen (ret->fd, 5))
    {
      close (ret->fd);
      free (ret);
      return NULL;
    }

 G2ClrBlocking(ret->fd);

  return ret;
}


int G2NetRecv(G2Net *net,G2NetAddr *from,void *buf,int len)
{
int fromlen=from ?sizeof(struct sockaddr_in):0;

#ifdef __APPLE__
	return recvfrom(net->fd,buf,len,SO_NOSIGPIPE,(struct sockaddr *) &from->sin,&fromlen);
#else
	return recvfrom(net->fd,buf,len,MSG_NOSIGNAL,(struct sockaddr *) &from->sin,&fromlen);
#endif

}

int G2NetSend(G2Net *net,G2NetAddr *to,void *buf,int len)
{
#ifdef __APPLE__
	return  sendto(net->fd,  buf, len, SO_NOSIGPIPE, (struct sockaddr *) &to->sin,sizeof(struct sockaddr_in));
#else
	return  sendto(net->fd,  buf, len, MSG_NOSIGNAL, (struct sockaddr *) &to->sin,sizeof(struct sockaddr_in));
#endif
}

char *G2NetAddrtoa(G2NetAddr *a)
{
static char ret[32];

sprintf(ret,"%s:%d",inet_ntoa(a->sin.sin_addr),ntohs(a->sin.sin_port));
return ret;
}


int
G2NetOpenUDPTCP (int port, G2Net ** tcp, G2Net ** udp)
{
  *udp = G2NetOpenUDP (port);
  if (!*udp)
    return -1;
  *tcp = G2NetOpenUDP (G2NetPort (*udp));
  if (!*tcp)
    {
      G2NetClose (*udp);
      return -1;
    }
  return 0;
}

void
G2NetClose (G2Net * ret)
{
  close (ret->fd);
  free (ret);
}
