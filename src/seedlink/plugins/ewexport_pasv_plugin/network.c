
/***************************************************************************
 * network.c
 *
 * General network communication routines
 *
 * Written by Chad Trabant, previously at ORFEUS, currently at IRIS DMC.
 *       
 * modified: 2009.040
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>

#include "util.h"

/* Functions only used in this source file */
static int my_getaddrinfo (char *nodename, char *nodeport,
			   struct sockaddr *addr, size_t *addrlen);
static int my_checksock (int sock, int tosec, int tousec);


/***************************************************************************
 * my_connect():
 * Open a TCP/IP network socket connection, does the creating and connecting
 * of a network socket.
 *
 * Returns 0 on errors, otherwise the socket descriptor created.
 ***************************************************************************/
extern int
my_connect (char *host, char *port)
{
  int sock;
  int on = 1;
  int noblockflag;
  int sockstat;
  char *host_name;
  size_t addrlen;
  struct sockaddr addr;
  
  if ((host_name = strdup (host)) == NULL)
    {
      gen_log (1,0, "strdup(): cannot copy host address string\n");
      return 0;
    }				/* ? needed? a hang-over from the original */
  
  /* Sanity check for the host and port specified */
  if ( strlen(host) <= 0 )
    {
      gen_log (1,0, "server address specified incorrectly: %s\n", host);
      return 0;
    }

  if ( strlen(port) <= 0 )
    {
      gen_log (1,0, "Port specified incorrectly: %s\n", port);
      return 0;
    }
  
  if ( my_getaddrinfo (host_name, port, &addr, &addrlen) )
    {
      gen_log (1,0, "Cannot resolve hostname %s\n", host_name );
      return 0;
    }

  free (host_name);
  
  if ((sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
      gen_log (1,0, "socket(): %s\n", strerror (errno));
      close (sock);
      return 0;
    }
  
  /* Set non-blocking */
  noblockflag = fcntl(sock, F_GETFL, 0);
  noblockflag |= O_NONBLOCK;
  if (fcntl(sock, F_SETFL, noblockflag) == -1)
    {
      gen_log( 1,0, "fcntl(): could not set socket non-blocking\n");
      close (sock);
      return 0;
    }
 
  if ( (connect (sock, (struct sockaddr *) &addr, addrlen)) )
    {
      if ( errno != EINPROGRESS )
	{
	  gen_log (1,0, "connect(): %s\n", strerror (errno));
	  close (sock);
	  return 0;
	}
    }
  
  /* Give a second for connecting, this is stupid but needed at the moment.
     Some firewalls that proxy/NAT TCP connections will report that the TCP
     connection is ready even when they are still opening the "other" end.
     Maybe a new test should be implemented to test for readiness.
   */
  safe_usleep (1000000);

  /* Wait up to 10 seconds for the socket to be connected */
  if ( (sockstat = my_checksock (sock, 10, 0)) <= 0 )
    {
      if (sockstat < 0)
	{			/* select() returned error */
	  gen_log (1,1, "[%s] socket connect error\n", host);
	}
      else
	{			/* socket time-out */
	  gen_log (0,1, "[%s] socket connect time-out (10s)\n", host);
	}
      close (sock);
      
      return -1;
    }
  else
    {				/* socket connected */
      gen_log (0,1, "[%s] network socket opened\n", host);
      
      /* Set the SO_KEEPALIVE socket option, not really useful in this case */
      if (setsockopt
	  (sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof (on)) < 0)
	gen_log (0,1, "[%s] cannot set SO_KEEPALIVE socket option\n", host);
      
      return sock;
    }
}				/* End of my_connect() */

/***************************************************************************
 * my_getaddrinfo():
 * Resolve IP addresses and provide parameters needed for connect().
 * On Linux (glibc2) and Solaris the reentrant gethostbyname_r() is
 * used.
 *
 * The better solution to name resolution is to use POSIX 1003.1g
 * getaddrinfo() because it is standardized, thread-safe and protocol
 * independent (i.e. IPv4, IPv6, etc.).  Unfortunately it is not
 * supported on many older platforms.
 *
 * Return 0 on success and non-zero on error.
 ***************************************************************************/
extern int
my_getaddrinfo (char *nodename, char *nodeport,
		struct sockaddr *addr, size_t *addrlen)
{
#if defined(__linux__) || defined(__linux)  || defined(__sun__) || defined(__sun)
  /* 512 bytes should be enough for the vast majority of cases.  If
     not (e.g. the node has a lot of aliases) this call will fail. */

  char buffer[512];
  struct hostent *result;
  struct hostent result_buffer;
  struct sockaddr_in inet_addr;
  int my_error;
  long int nport;
  char *tail;
  
  #if defined(__linux__) || defined(__linux)
  gethostbyname_r (nodename, &result_buffer,
		   buffer, sizeof(buffer) - 1,
		   &result, &my_error);
  
  #elif defined(__sun__) || defined(__sun)
  result = gethostbyname_r (nodename, &result_buffer,
                            buffer, sizeof(buffer) - 1,
			    &my_error);
  #endif
  
  if ( !result )
    return my_error;
  
  nport = strtoul (nodeport, &tail, 0);

  memset (&inet_addr, 0, sizeof (inet_addr));
  inet_addr.sin_family = AF_INET;
  inet_addr.sin_port = htons ((unsigned short int)nport);
  inet_addr.sin_addr = *(struct in_addr *) result->h_addr_list[0];
  
  *addr = *((struct sockaddr *) &inet_addr);
  *addrlen = sizeof(inet_addr);
  
#else
  /* This will be used by all others, it is not properly supported
     by some but this is the future of name resolution. */
  
  struct addrinfo *result;
  struct addrinfo hints;
  
  memset (&hints, 0, sizeof(hints));
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  if ( getaddrinfo (nodename, nodeport, &hints, &result) )
    {
      return -1;
    }
  
  *addr = *(result->ai_addr);
  *addrlen = result->ai_addrlen;

  freeaddrinfo (result);
  
#endif

  return 0;
}				/* End of my_getaddrinfo() */


/***************************************************************************
 * my_checksock():
 * Check a socket for write ability using select() and read ability
 * using recv(... MSG_PEEK).  Time-out values are also passed (seconds
 * and microseconds) for the select() call.
 *
 * Returns:
 *  1 = success
 *  0 = if time-out expires
 * -1 = errors
 ***************************************************************************/
int
my_checksock (int sock, int tosec, int tousec)
{
  int sret;
  int ret = -1;			/* default is failure */
  char testbuf[1];
  fd_set checkset;
  struct timeval to;

  FD_ZERO (&checkset);
  FD_SET ((unsigned int)sock, &checkset);

  to.tv_sec = tosec;
  to.tv_usec = tousec;

  /* Check write ability with select() */
  if ( (sret = select (sock + 1, NULL, &checkset, NULL, &to)) > 0 )
    ret = 1;
  else if ( sret == 0 )
    ret = 0;			/* time-out expired */

  /* Check read ability with recv() */
  if ( ret && (recv (sock, testbuf, sizeof (char), MSG_PEEK)) <= 0 )
    {
      if ( errno == EWOULDBLOCK )
	ret = 1;		/* no data for non-blocking IO */
      else
	ret = -1;
    }

  return ret;
}				/* End of my_checksock() */


/***************************************************************************
 * my_recv():
 * recv() 'bufferlen' data from 'sock' into a specified 'buffer'.
 * 'flags' is passed directly to recv.  'timeout_msecs' is a timeout
 * in milliseconds.
 *
 * Returns -1 on error/EOF, 0 for timeout and the number of bytes read
 * on success.
 ***************************************************************************/
extern int
my_recv (int sock, char *buf, size_t buflen, int flags, int timeout_msecs)
{
  int bytesread = 0;
  int ackcnt    = 0;      /* counter for the number of recv attempts */
  int ackpoll   = 5;      /* poll at 0.005 seconds for recv'ing */
  
  while ( bytesread == 0 )
    {
      bytesread = recv (sock, buf, buflen, flags);
      
      if ( bytesread == 0 )           /* should indicate TCP FIN or EOF */
	{
	  gen_log (1,0, "recv(): TCP FIN or EOF received\n");
	  bytesread = -1;
	}
      else if ( bytesread < 0 )
	{
	  if ( errno != EWOULDBLOCK )
	    {
	      gen_log (1,0, "recv(): %s\n", strerror (errno));
	      bytesread = -1;
	    }
	  else
	    {
	      bytesread = 0;
	    }
	}

      /* Check timeout when nothing received */
      if ( bytesread == 0 )
	{
	  ackcnt++;
	  
	  if ((ackpoll * ackcnt) > timeout_msecs)
	    {
	      gen_log (0,2, "my_recv(): socket recv timeout\n");
	      break;
	    }
	  
	  safe_usleep (ackpoll*1000);
	}
    }

  return bytesread;
}                               /* End of my_recv() */


/***************************************************************************
 * my_send():
 * send() 'buflen' bytes from 'buffer' to 'sendlink'.
 * 'code' is a string to include in error messages for identification.
 * If 'resp' is not NULL then read up to 'resplen' bytes into 'resp'
 * after sending 'buffer'.  This is only designed for small pieces of data,
 * specifically the server responses to commands.
 *
 * Returns -1 on error, 0 on timeout and number of bytes sent on success.
 ***************************************************************************/
extern int
my_send (int sock, char *buf, int buflen, int flags, int timeout_msecs)
{
  int bytesjustsent = 0;
  int bytessent     = 0;
  int ackcnt        = 0;      /* counter for the number of recv attempts */
  int ackpoll       = 5;      /* poll at 0.005 seconds for recv'ing */

  while ( bytessent < buflen )
    {
      bytesjustsent = send (sock, buf+bytessent, buflen-bytessent, flags);
      
      if ( bytesjustsent < 0 )
	{
	  if ( errno != EWOULDBLOCK )
	    {
	      gen_log (1,0, "recv(): %s\n", strerror (errno));
	      bytessent = -1;
	      break;
	    }
	  else
	    {
	      bytesjustsent = 0;
	    }
	}
      
      bytessent += bytesjustsent;

      /* Check timeout when nothing received */
      if ( bytesjustsent == 0 )
	{
	  ackcnt++;

	  if ((ackpoll * ackcnt) > timeout_msecs)
	    {
	      gen_log (0,2, "my_send(): socket send timeout\n");
	      bytessent = 0;
	      break;
	    }

	  safe_usleep (ackpoll*1000);
	}
    }
  
  return bytessent;
}                               /* End of my_send() */

