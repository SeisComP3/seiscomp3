
/***************************************************************************
 * network.c
 *
 * Network communication routines for SeedLink
 *
 * Written by Chad Trabant, 
 *   ORFEUS/EC-Project MEREDIAN (previously)
 *   IRIS Data Management Center
 *
 * Originally based on the SeedLink interface of the modified Comserv in
 * SeisComP written by Andres Heinloo
 *
 * Version: 2007.284
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libslink.h"

/* Functions only used in this source file */
int sl_sayhello (SLCD * slconn);
int sl_batchmode (SLCD * slconn);
int sl_negotiate_uni (SLCD * slconn);
int sl_negotiate_multi (SLCD * slconn);


/***************************************************************************
 * sl_configlink:
 *
 * Configure/negotiate data stream(s) with the remote SeedLink
 * server.  Negotiation will be either uni or multi-station
 * depending on the value of 'multistation' in the SLCD
 * struct.
 *
 * Returns -1 on errors, otherwise returns the link descriptor.
 ***************************************************************************/
int
sl_configlink (SLCD * slconn)
{
  int ret = -1;

  if (slconn->multistation)
    {
      if (sl_checkversion (slconn, 2.5) >= 0)
	{
	  ret = sl_negotiate_multi (slconn);
	}
      else
	{
	  sl_log_r (slconn, 2, 0,
		    "[%s] detected  SeedLink version (%.3f) does not support multi-station protocol\n",
		    slconn->sladdr, slconn->protocol_ver);
	  ret = -1;
	}
    }
  else
    ret = sl_negotiate_uni (slconn);

  return ret;
}  /* End of sl_configlink() */


/***************************************************************************
 * sl_negotiate_uni:
 *
 * Negotiate a SeedLink connection in uni-station mode and issue
 * the DATA command.  This is compatible with SeedLink Protocol
 * version 2 or greater.
 *
 * If 'selectors' != 0 then the string is parsed on space and each
 * selector is sent.
 *
 * If 'seqnum' != -1 and the SLCD 'resume' flag is true then data is
 * requested starting at seqnum.
 *
 * Returns -1 on errors, otherwise returns the link descriptor.
 ***************************************************************************/
int
sl_negotiate_uni (SLCD * slconn)
{
  int sellen = 0;
  int bytesread = 0;
  int acceptsel = 0;		/* Count of accepted selectors */
  char *selptr;
  char *extreply = 0;
  char *term1, *term2;
  char sendstr[100];		/* A buffer for command strings */
  char readbuf[100];		/* A buffer for responses */
  SLstream *curstream;
  
  /* Point to the stream chain */
  curstream = slconn->streams;

  selptr = curstream->selectors;
  
  /* Send the selector(s) and check the response(s) */
  if (curstream->selectors != 0)
    {
      while (1)
	{
	  selptr += sellen;
	  selptr += strspn (selptr, " ");
	  sellen = strcspn (selptr, " ");

	  if (sellen == 0)
	    break;		/* end of while loop */

	  else if (sellen > SELSIZE)
	    {
	      sl_log_r (slconn, 2, 0, "[%s] invalid selector: %.*s\n", slconn->sladdr,
			sellen, selptr);
	      selptr += sellen;
	    }
	  else
	    {
	      
	      /* Build SELECT command, send it and receive response */
	      sprintf (sendstr, "SELECT %.*s\r", sellen, selptr);
	      sl_log_r (slconn, 1, 2, "[%s] sending: SELECT %.*s\n", slconn->sladdr,
			sellen, selptr);
	      bytesread = sl_senddata (slconn, (void *) sendstr,
				       strlen (sendstr), slconn->sladdr,
				       readbuf, sizeof (readbuf));
	      if (bytesread < 0)
		{		/* Error from sl_senddata() */
		  return -1;
		}
	      
	      /* Search for 2nd "\r" indicating extended reply message present */
	      extreply = 0;
	      if ( (term1 = memchr (readbuf, '\r', bytesread)) )
		{
		  if ( (term2 = memchr (term1+1, '\r', bytesread-(readbuf-term1)-1)) )
		    {
		      *term2 = '\0';
		      extreply = term1+1;
		    }
		}
	      
	      /* Check response to SELECT */
	      if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
		{
		  sl_log_r (slconn, 1, 2, "[%s] selector %.*s is OK %s%s%s\n", slconn->sladdr,
			    sellen, selptr, (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
		  acceptsel++;
		}
	      else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
		{
		  sl_log_r (slconn, 1, 2, "[%s] selector %.*s not accepted %s%s%s\n", slconn->sladdr,
			    sellen, selptr, (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
		}
	      else
		{
		  sl_log_r (slconn, 2, 0,
			    "[%s] invalid response to SELECT command: %.*s\n",
			    slconn->sladdr, bytesread, readbuf);
		  return -1;
		}
	    }
	}
      
      /* Fail if none of the given selectors were accepted */
      if (!acceptsel)
	{
	  sl_log_r (slconn, 2, 0, "[%s] no data stream selector(s) accepted\n",
		    slconn->sladdr);
	  return -1;
	}
      else
	{
	  sl_log_r (slconn, 1, 2, "[%s] %d selector(s) accepted\n",
		    slconn->sladdr, acceptsel);
	}
    }				/* End of selector processing */

  /* Issue the DATA, FETCH or TIME action commands.  A specified start (and
     optionally, stop time) takes precedence over the resumption from any
     previous sequence number. */
  if (slconn->begin_time != NULL)
    {
      if (sl_checkversion (slconn, (float)2.92) >= 0)
	{
	  if (slconn->end_time == NULL)
	    {
	      sprintf (sendstr, "TIME %.25s\r", slconn->begin_time);
	    }
	  else
	    {
	      sprintf (sendstr, "TIME %.25s %.25s\r", slconn->begin_time,
		       slconn->end_time);
	    }
	  sl_log_r (slconn, 1, 1, "[%s] requesting specified time window\n",
		    slconn->sladdr);
	}
      else
	{
	  sl_log_r (slconn, 2, 0,
		    "[%s] detected SeedLink version (%.3f) does not support TIME windows\n",
		    slconn->sladdr, slconn->protocol_ver);
	}
    }
  else if (curstream->seqnum != -1 && slconn->resume )
    {
      char cmd[10];

      if ( slconn->dialup )
	{
	  sprintf (cmd, "FETCH");
	}
      else
	{
	  sprintf (cmd, "DATA");
	}

      /* Append the last packet time if the feature is enabled and server is >= 2.93 */
      if (slconn->lastpkttime &&
	  sl_checkversion (slconn, (float)2.93) >= 0 &&
	  strlen (curstream->timestamp))
	{
	  /* Increment sequence number by 1 */
	  sprintf (sendstr, "%s %06X %.25s\r", cmd,
		   (curstream->seqnum + 1) & 0xffffff, curstream->timestamp);

	  sl_log_r (slconn, 1, 1, "[%s] resuming data from %06X (Dec %d) at %.25s\n",
		    slconn->sladdr, (curstream->seqnum + 1) & 0xffffff,
		    (curstream->seqnum + 1), curstream->timestamp);
	}
      else
	{
	  /* Increment sequence number by 1 */
	  sprintf (sendstr, "%s %06X\r", cmd, (curstream->seqnum + 1) & 0xffffff);

	  sl_log_r (slconn, 1, 1, "[%s] resuming data from %06X (Dec %d)\n",
		    slconn->sladdr, (curstream->seqnum + 1) & 0xffffff,
		    (curstream->seqnum + 1));
	}
    }
  else
    {
      if ( slconn->dialup )
	{
	  sprintf (sendstr, "FETCH\r");
	}
      else
	{
	  sprintf (sendstr, "DATA\r");
	}

      sl_log_r (slconn, 1, 1, "[%s] requesting next available data\n", slconn->sladdr);
    }

  if (sl_senddata (slconn, (void *) sendstr, strlen (sendstr),
		   slconn->sladdr, (void *) NULL, 0) < 0)
    {
      sl_log_r (slconn, 2, 0, "[%s] error sending DATA/FETCH/TIME request\n", slconn->sladdr);
      return -1;
    }

  return slconn->link;
}  /* End of sl_negotiate_uni() */


/***************************************************************************
 * sl_negotiate_multi:
 *
 * Negotiate a SeedLink connection using multi-station mode and 
 * issue the END action command.  This is compatible with SeedLink
 * Protocol version 3, multi-station mode.
 *
 * If 'curstream->selectors' != 0 then the string is parsed on space
 * and each selector is sent.
 *
 * If 'curstream->seqnum' != -1 and the SLCD 'resume' flag is true
 * then data is requested starting at seqnum.
 *
 * Returns -1 on errors, otherwise returns the link descriptor.
 ***************************************************************************/
int
sl_negotiate_multi (SLCD * slconn)
{
  int sellen = 0;
  int bytesread = 0;
  int acceptsta = 0;		/* Count of accepted stations */
  int acceptsel = 0;		/* Count of accepted selectors */
  char *selptr;
  char *term1, *term2;
  char *extreply = 0;
  char sendstr[100];		/* A buffer for command strings */
  char readbuf[100];		/* A buffer for responses */
  char slring[12];		/* Keep track of the ring name */
  SLstream *curstream;

  /* Point to the stream chain */
  curstream = slconn->streams;

  /* Loop through the stream chain */
  while (curstream != NULL)
    {

      /* A ring identifier */
      snprintf (slring, sizeof (slring), "%s_%s",
		curstream->net, curstream->sta);

      /* Send the STATION command */
      sprintf (sendstr, "STATION %s %s\r", curstream->sta, curstream->net);
      sl_log_r (slconn, 1, 2, "[%s] sending: STATION %s %s\n",
		slring, curstream->sta, curstream->net);
      bytesread = sl_senddata (slconn, (void *) sendstr,
			       strlen (sendstr), slring, readbuf,
			       sizeof (readbuf));
      if (bytesread < 0)
	{
	  return -1;
	}
      
      /* Search for 2nd "\r" indicating extended reply message present */
      extreply = 0;
      if ( (term1 = memchr (readbuf, '\r', bytesread)) )
	{
	  if ( (term2 = memchr (term1+1, '\r', bytesread-(readbuf-term1)-1)) )
	    {
	      *term2 = '\0';
	      extreply = term1+1;
	    }
	}
      
      /* Check the response */
      if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
	{
	  sl_log_r (slconn, 1, 2, "[%s] station is OK %s%s%s\n", slring,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
	  acceptsta++;
	}
      else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
	{
	  sl_log_r (slconn, 2, 0, "[%s] station not accepted %s%s%s\n", slring,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
	  /* Increment the loop control and skip to the next stream */
	  curstream = curstream->next;
	  continue;
	}
      else
	{
	  sl_log_r (slconn, 2, 0, "[%s] invalid response to STATION command: %.*s\n",
		    slring, bytesread, readbuf);
	  return -1;
	}

      selptr = curstream->selectors;
      sellen = 0;

      /* Send the selector(s) and check the response(s) */
      if (curstream->selectors != 0)
	{
	  while (1)
	    {
	      selptr += sellen;
	      selptr += strspn (selptr, " ");
	      sellen = strcspn (selptr, " ");

	      if (sellen == 0)
		break;		/* end of while loop */

	      else if (sellen > SELSIZE)
		{
		  sl_log_r (slconn, 2, 0, "[%s] invalid selector: %.*s\n",
			    slring, sellen, selptr);
		  selptr += sellen;
		}
	      else
		{

		  /* Build SELECT command, send it and receive response */
		  sprintf (sendstr, "SELECT %.*s\r", sellen, selptr);
		  sl_log_r (slconn, 1, 2, "[%s] sending: SELECT %.*s\n", slring, sellen,
			    selptr);
		  bytesread = sl_senddata (slconn, (void *) sendstr,
					   strlen (sendstr), slring,
 					   readbuf, sizeof (readbuf));
		  if (bytesread < 0)
		    {		/* Error from sl_senddata() */
		      return -1;
		    }
		  
		  /* Search for 2nd "\r" indicating extended reply message present */
		  extreply = 0;
		  if ( (term1 = memchr (readbuf, '\r', bytesread)) )
		    {
		      if ( (term2 = memchr (term1+1, '\r', bytesread-(readbuf-term1)-1)) )
			{
			  *term2 = '\0';
			  extreply = term1+1;
			}
		    }
		  
		  /* Check response to SELECT */
		  if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
		    {
		      sl_log_r (slconn, 1, 2, "[%s] selector %.*s is OK %s%s%s\n", slring,
				sellen, selptr, (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
		      acceptsel++;
		    }
		  else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
		    {
		      sl_log_r (slconn, 2, 0, "[%s] selector %.*s not accepted %s%s%s\n", slring,
				sellen, selptr, (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
		    }
		  else
		    {
		      sl_log_r (slconn, 2, 0,
				"[%s] invalid response to SELECT command: %.*s\n",
				slring, bytesread, readbuf);
		      return -1;
		    }
		}
	    }

	  /* Fail if none of the given selectors were accepted */
	  if (!acceptsel)
	    {
	      sl_log_r (slconn, 2, 0, "[%s] no data stream selector(s) accepted",
			slring);
	      return -1;
	    }
	  else
	    {
	      sl_log_r (slconn, 1, 2, "[%s] %d selector(s) accepted\n", slring,
			acceptsel);
	    }

	  acceptsel = 0;	/* Reset the accepted selector count */

	}			/* End of selector processing */

      /* Issue the DATA, FETCH or TIME action commands.  A specified start (and
	 optionally, stop time) takes precedence over the resumption from any
	 previous sequence number. */
      if (slconn->begin_time != NULL)
	{
	  if (sl_checkversion (slconn, (float)2.92) >= 0)
	    {
	      if (slconn->end_time == NULL)
		{
		  sprintf (sendstr, "TIME %.25s\r", slconn->begin_time);
		}
	      else
		{
		  sprintf (sendstr, "TIME %.25s %.25s\r", slconn->begin_time,
			   slconn->end_time);
		}
	      sl_log_r (slconn, 1, 1, "[%s] requesting specified time window\n",
			slring);
	    }
	  else
	    {
	      sl_log_r (slconn, 2, 0,
			"[%s] detected SeedLink version (%.3f) does not support TIME windows\n",
			slring, slconn->protocol_ver);
	    }
	}
      else if (curstream->seqnum != -1 && slconn->resume )
	{
	  char cmd[10];
	  
	  if ( slconn->dialup )
	    {
	      sprintf (cmd, "FETCH");
	    }
	  else
	    {
	      sprintf (cmd, "DATA");
	    }
	  
	  /* Append the last packet time if the feature is enabled and server is >= 2.93 */
	  if (slconn->lastpkttime &&
	      sl_checkversion (slconn, (float)2.93) >= 0 &&
	      strlen (curstream->timestamp))
	    {
	      /* Increment sequence number by 1 */
	      sprintf (sendstr, "%s %06X %.25s\r", cmd,
		       (curstream->seqnum + 1) & 0xffffff, curstream->timestamp);

	      sl_log_r (slconn, 1, 1, "[%s] resuming data from %06X (Dec %d) at %.25s\n",
			slconn->sladdr, (curstream->seqnum + 1) & 0xffffff,
			(curstream->seqnum + 1), curstream->timestamp);
	    }	  
	  else
	    { /* Increment sequence number by 1 */
	      sprintf (sendstr, "%s %06X\r", cmd,
		       (curstream->seqnum + 1) & 0xffffff);

	      sl_log_r (slconn, 1, 1, "[%s] resuming data from %06X (Dec %d)\n", slring,
			(curstream->seqnum + 1) & 0xffffff,
			(curstream->seqnum + 1));
	    }
	}
      else
	{
	  if ( slconn->dialup )
	    {
	      sprintf (sendstr, "FETCH\r");
	    }
	  else
	    {
	      sprintf (sendstr, "DATA\r");
	    }
	  
	  sl_log_r (slconn, 1, 1, "[%s] requesting next available data\n", slring);
	}

      /* Send the TIME/DATA/FETCH command and receive response */
      bytesread = sl_senddata (slconn, (void *) sendstr,
			       strlen (sendstr), slring,
			       readbuf, sizeof (readbuf));
      if (bytesread < 0)
	{
	  sl_log_r (slconn, 2, 0, "[%s] error with DATA/FETCH/TIME request\n", slring);
	  return -1;
	}
      
      /* Search for 2nd "\r" indicating extended reply message present */
      extreply = 0;
      if ( (term1 = memchr (readbuf, '\r', bytesread)) )
	{
	  if ( (term2 = memchr (term1+1, '\r', bytesread-(readbuf-term1)-1)) )
	    {
	      fprintf (stderr, "term2: '%s'\n", term2);

	      *term2 = '\0';
	      extreply = term1+1;
	    }
	}
      
      /* Check response to DATA/FETCH/TIME request */
      if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
	{
	  sl_log_r (slconn, 1, 2, "[%s] DATA/FETCH/TIME command is OK %s%s%s\n", slring,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
	}
      else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
	{
	  sl_log_r (slconn, 2, 0, "[%s] DATA/FETCH/TIME command is not accepted %s%s%s\n", slring,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
	}
      else
	{
	  sl_log_r (slconn, 2, 0, "[%s] invalid response to DATA/FETCH/TIME command: %.*s\n",
		    slring, bytesread, readbuf);
	  return -1;
	}
      
      /* Point to the next stream */
      curstream = curstream->next;

    }  /* End of stream and selector config (end of stream chain). */
  
  /* Fail if no stations were accepted */
  if (!acceptsta)
    {
      sl_log_r (slconn, 2, 0, "[%s] no station(s) accepted\n", slconn->sladdr);
      return -1;
    }
  else
    {
      sl_log_r (slconn, 1, 1, "[%s] %d station(s) accepted\n",
	      slconn->sladdr, acceptsta);
    }

  /* Issue END action command */
  sprintf (sendstr, "END\r");
  sl_log_r (slconn, 1, 2, "[%s] sending: END\n", slconn->sladdr);
  if (sl_senddata (slconn, (void *) sendstr, strlen (sendstr),
		   slconn->sladdr, (void *) NULL, 0) < 0)
    {
      sl_log_r (slconn, 2, 0, "[%s] error sending END command\n", slconn->sladdr);
      return -1;
    }

  return slconn->link;
}  /* End of sl_negotiate_multi() */


/***************************************************************************
 * sl_send_info:
 *
 * Send a request for the specified INFO level.  The verbosity level
 * can be specified, allowing control of when the request should be
 * logged.
 *
 * Returns -1 on errors, otherwise the socket descriptor.
 ***************************************************************************/
int
sl_send_info (SLCD * slconn, const char * info_level, int verbose)
{
  char sendstr[40];		/* A buffer for command strings */

  if (sl_checkversion (slconn, (float)2.92) >= 0)
    {
      sprintf (sendstr, "INFO %.15s\r", info_level);

      sl_log_r (slconn, 1, verbose, "[%s] requesting INFO level %s\n",
		slconn->sladdr, info_level);

      if (sl_senddata (slconn, (void *) sendstr, strlen (sendstr),
		       slconn->sladdr, (void *) NULL, 0) < 0)
	{
	  sl_log_r (slconn, 2, 0, "[%s] error sending INFO request\n", slconn->sladdr);
	  return -1;
	}
    }
  else
    {
      sl_log_r (slconn, 2, 0,
		"[%s] detected SeedLink version (%.3f) does not support INFO requests\n",
		slconn->sladdr, slconn->protocol_ver);   
      return -1;
    }

  return slconn->link;
}  /* End of sl_send_info() */


/***************************************************************************
 * sl_connect:
 *
 * Open a network socket connection to a SeedLink server and set
 * 'slconn->link' to the new descriptor.  Expects 'slconn->sladdr' to
 * be in 'host:port' format.  Either the host, port or both are
 * optional, if the host is not specified 'localhost' is assumed, if
 * the port is not specified '18000' is assumed, if neither is
 * specified (only a colon) then 'localhost' and port '18000' are
 * assumed.
 *
 * If sayhello is true the HELLO command will be issued after
 * successfully connecting, this sets the server version in the SLCD
 * and generally verifies that the remove process is a SeedLink
 * server and is probably always desired.
 *
 * If a permanent error is detected (invalid port specified) the
 * slconn->terminate flag will be set so the sl_collect() family of
 * routines will not continue trying to connect.
 *
 * Returns -1 on errors otherwise the socket descriptor created.
 ***************************************************************************/
int
sl_connect (SLCD * slconn, int sayhello)
{
  int sock;
  int on = 1;
  int sockstat;
  long int nport;
  char nodename[300];
  char nodeport[100];
  char *ptr, *tail;
  size_t addrlen;
  struct sockaddr addr;

  if ( slp_sockstartup() ) {
    sl_log_r (slconn, 2, 0, "could not initialize network sockets\n");
    return -1;
  }
  
  /* Check server address string and use defaults if needed:
   * If only ':' is specified neither host nor port specified
   * If no ':' is included no port was specified
   * If ':' is the first character no host was specified
   */
  if ( ! strcmp (slconn->sladdr, ":") )
    {
      strcpy (nodename, "localhost");
      strcpy (nodeport, "18000");
    }
  else if ((ptr = strchr (slconn->sladdr, ':')) == NULL)
    {
      strncpy (nodename, slconn->sladdr, sizeof(nodename));
      strcpy (nodeport, "18000");
    }
  else
    {
      if ( ptr == slconn->sladdr )
	{
	  strcpy (nodename, "localhost");
	}
      else
	{
	  strncpy (nodename, slconn->sladdr, (ptr - slconn->sladdr));
	  nodename[(ptr - slconn->sladdr)] = '\0';
	}
      
      strcpy (nodeport, ptr+1);

      /* Sanity test the port number */
      nport = strtoul (nodeport, &tail, 10);
      if ( *tail || (nport <= 0 || nport > 0xffff) )
	{
	  sl_log_r (slconn, 2, 0, "server port specified incorrectly\n");
	  slconn->terminate = 1;
	  return -1;
	}
    }
  
  if ( slp_getaddrinfo (nodename, nodeport, &addr, &addrlen) )
    {
      sl_log_r (slconn, 2, 0, "cannot resolve hostname %s\n", nodename );
      return -1;
    }

  if ((sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
      sl_log_r (slconn, 2, 0, "[%s] socket(): %s\n", slconn->sladdr, slp_strerror ());
      slp_sockclose (sock);
      return -1;
    }

  /* Set non-blocking IO */
  if ( slp_socknoblock(sock) )
    {
      sl_log_r (slconn, 2, 0, "Error setting socket to non-blocking\n");
    }

  if ( (slp_sockconnect (sock, (struct sockaddr *) &addr, addrlen)) )
    {
      sl_log_r (slconn, 2, 0, "[%s] connect(): %s\n", slconn->sladdr, slp_strerror ());
      slp_sockclose (sock);
      return -1;
    }
  
  /* Wait up to 10 seconds for the socket to be connected */
  if ((sockstat = sl_checksock (sock, 10, 0)) <= 0)
    {
      if (sockstat < 0)
	{			/* select() returned error */
	  sl_log_r (slconn, 2, 1, "[%s] socket connect error\n", slconn->sladdr);
	}
      else
	{			/* socket time-out */
	  sl_log_r (slconn, 2, 1, "[%s] socket connect time-out (10s)\n",
		    slconn->sladdr);
	}

      slp_sockclose (sock);
      return -1;
    }
  else if ( ! slconn->terminate )
    {				/* socket connected */
      sl_log_r (slconn, 1, 1, "[%s] network socket opened\n", slconn->sladdr);
      
      /* Set the SO_KEEPALIVE socket option, although not really useful */
      if (setsockopt
	  (sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof (on)) < 0)
	sl_log_r (slconn, 1, 1, "[%s] cannot set SO_KEEPALIVE socket option\n",
		  slconn->sladdr);
      
      slconn->link = sock;

      if ( slconn->batchmode )
	  slconn->batchmode = 1;
      
      /* Everything should be connected, say hello if requested */
      if ( sayhello )
	{
	  if (sl_sayhello (slconn) == -1)
	    {
	      slp_sockclose (sock);
	      return -1;
	    }
	}
      
      /* Try to enter batch mode if requested */
      if ( slconn->batchmode )
        {
          if (sl_batchmode (slconn) == -1)
	    {
	      slp_sockclose (sock);
	      return -1;
	    }
	}
      
      return sock;
    }
  
  return -1;
}  /* End of sl_connect() */


/***************************************************************************
 * sl_disconnect:
 *
 * Close the network socket associated with connection
 *
 * Returns -1, historically used to set the old descriptor
 ***************************************************************************/
int
sl_disconnect (SLCD * slconn)
{
  if (slconn->link != -1)
    {
      slp_sockclose (slconn->link);
      slconn->link = -1;
      
      sl_log_r (slconn, 1, 1, "[%s] network socket closed\n", slconn->sladdr);
    }
  
  return -1;
}  /* End of sl_disconnect() */


/***************************************************************************
 * sl_sayhello:
 *
 * Send the HELLO command and attempt to parse the server version
 * number from the returned string.  The server version is set to 0.0
 * if it can not be parsed from the returned string, which indicates
 * unkown protocol functionality.
 *
 * Returns -1 on errors, 0 on success.
 ***************************************************************************/
int
sl_sayhello (SLCD * slconn)
{
  int ret = 0;
  int servcnt = 0;
  int sitecnt = 0;
  char sendstr[100];		/* A buffer for command strings */
  char servstr[200];		/* The remote server ident */
  char sitestr[100];		/* The site/data center ident */
  char servid[100];		/* Server ID string, i.e. 'SeedLink' */
  char *capptr;                 /* Pointer to capabilities flags */  
  char capflag = 0;             /* Capabilities are supported by server */

  /* Send HELLO */
  sprintf (sendstr, "HELLO\r");
  sl_log_r (slconn, 1, 2, "[%s] sending: %s\n", slconn->sladdr, sendstr);
  sl_senddata (slconn, (void *) sendstr, strlen (sendstr), slconn->sladdr,
	       NULL, 0);
  
  /* Recv the two lines of response: server ID and site installation ID */
  if ( sl_recvresp (slconn, (void *) servstr, (size_t) sizeof (servstr),
		    sendstr, slconn->sladdr) < 0 )
    {
      return -1;
    }
  
  if ( sl_recvresp (slconn, (void *) sitestr, (size_t) sizeof (sitestr),
		    sendstr, slconn->sladdr) < 0 )
    {
      return -1;
    }
  
  /* Terminate on first "\r" character or at one character before end of buffer */
  servcnt = strcspn (servstr, "\r");
  if ( servcnt > (sizeof(servstr)-2) )
    {
      servcnt = (sizeof(servstr)-2);
    }
  servstr[servcnt] = '\0';
  
  sitecnt = strcspn (sitestr, "\r");
  if ( sitecnt > (sizeof(sitestr)-2) )
    {
      sitecnt = (sizeof(sitestr)-2);
    }
  sitestr[sitecnt] = '\0';
  
  /* Search for capabilities flags in server ID by looking for "::"
   * The expected format of the complete server ID is:
   * "seedlink v#.# <optional text> <:: optional capability flags>"
   */
  capptr = strstr (servstr, "::");
  if ( capptr )
    {
      /* Truncate server ID portion of string */
      *capptr = '\0';

      /* Move pointer to beginning of flags */
      capptr += 2;
      
      /* Move capptr up to first non-space character */
      while ( *capptr == ' ' )
	capptr++;
    }
  
  /* Report received IDs */
  sl_log_r (slconn, 1, 1, "[%s] connected to: %s\n", slconn->sladdr, servstr);
  if ( capptr )
    sl_log_r (slconn, 1, 1, "[%s] capabilities: %s\n", slconn->sladdr, capptr);
  sl_log_r (slconn, 1, 1, "[%s] organization: %s\n", slconn->sladdr, sitestr);
  
  /* Parse old-school server ID and version from the returned string.
   * The expected format at this point is:
   * "seedlink v#.# <optional text>"
   * where 'seedlink' is case insensitive and '#.#' is the server/protocol version.
   */
  /* Add a space to the end to allowing parsing when the optionals are not present */
  servstr[servcnt] = ' '; servstr[servcnt+1] = '\0';
  ret = sscanf (servstr, "%s v%f ", &servid[0], &slconn->protocol_ver);
  
  if ( ret != 2 || strncasecmp (servid, "SEEDLINK", 8) )
    {
      sl_log_r (slconn, 1, 1,
                "[%s] unrecognized server version, assuming minimum functionality\n",
                slconn->sladdr);
      slconn->protocol_ver = 0.0;
    }
  
  /* Check capabilities flags */
  if ( capptr )
    {
      char *tptr;
      
      /* Parse protocol version flag: "SLPROTO:<#.#>" if present */
      if ( (tptr = strstr(capptr, "SLPROTO")) )
	{
	  /* This protocol specification overrides that from earlier in the server ID */
	  ret = sscanf (tptr, "SLPROTO:%f", &slconn->protocol_ver);
	  
	  if ( ret != 1 )
	    sl_log_r (slconn, 1, 1,
		      "[%s] could not parse protocol version from SLPROTO flag: %s\n",
		      slconn->sladdr, tptr);
	}
      
      /* Check for CAPABILITIES command support */
      if ( strstr(capptr, "CAP") )
	capflag = 1;
    }
  
  /* Send CAPABILITIES flags if supported by server */
  if ( capflag )
    {
      int bytesread = 0;
      char readbuf[100];
      
      char *term1, *term2;
      char *extreply = 0;
      
      /* Current capabilities:
       *   SLPROTO:3.0 = SeedLink protocol version 3.0
       *   CAP         = CAPABILITIES command support
       *   EXTREPLY    = Extended reply message handling
       *   NSWILDCARD  = Network and station code wildcard support
       */
      sprintf (sendstr, "CAPABILITIES SLPROTO:3.0 CAP EXTREPLY NSWILDCARD\r");
      
      /* Send CAPABILITIES and recv response */
      sl_log_r (slconn, 1, 2, "[%s] sending: %s\n", slconn->sladdr, sendstr);
      bytesread = sl_senddata (slconn, (void *) sendstr, strlen (sendstr), slconn->sladdr,
			       readbuf, sizeof (readbuf));
      
      if ( bytesread < 0 )
	{		/* Error from sl_senddata() */
	  return -1;
	}
      
      /* Search for 2nd "\r" indicating extended reply message present */
      extreply = 0;
      if ( (term1 = memchr (readbuf, '\r', bytesread)) )
	{
	  if ( (term2 = memchr (term1+1, '\r', bytesread-(readbuf-term1)-1)) )
	    {
	      *term2 = '\0';
	      extreply = term1+1;
	    }
	}
      
      /* Check response to CAPABILITIES */
      if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
	{
	  sl_log_r (slconn, 1, 2, "[%s] capabilities OK %s%s%s\n", slconn->sladdr,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");
	}
      else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
	{
	  sl_log_r (slconn, 1, 2, "[%s] CAPABILITIES not accepted %s%s%s\n", slconn->sladdr,
		    (extreply)?"{":"", (extreply)?extreply:"", (extreply)?"}":"");	  
	  return -1;
	}
      else
	{
	  sl_log_r (slconn, 2, 0,
		    "[%s] invalid response to CAPABILITIES command: %.*s\n",
		    slconn->sladdr, bytesread, readbuf);
	  return -1;
	}
    }
  
  return 0;
}  /* End of sl_sayhello() */


/***************************************************************************
 * sl_batchmode:
 *
 * Send the BATCH command
 *
 * Returns -1 on errors, 0 on success (regardless if the command was accepted).
 * Sets slconn->batchmode accordingly.
 ***************************************************************************/
int
sl_batchmode (SLCD * slconn)
{
  /* Enter batchmode if supported by server */
  char sendstr[100];		/* A buffer for command strings */
  char readbuf[100];
  int bytesread = 0;
  
  sprintf (sendstr, "BATCH\r");
  
  /* Send BATCH and recv response */
  sl_log_r (slconn, 1, 2, "[%s] sending: %s\n", slconn->sladdr, sendstr);
  bytesread = sl_senddata (slconn, (void *) sendstr, strlen (sendstr), slconn->sladdr,
    		       readbuf, sizeof (readbuf));
  
  if ( bytesread < 0 )
    {		/* Error from sl_senddata() */
      return -1;
    }
  
  /* Check response to BATCH */
  if (!strncmp (readbuf, "OK\r", 3) && bytesread >= 4)
    {
      sl_log_r (slconn, 1, 2, "[%s] BATCH accepted\n", slconn->sladdr);
      slconn->batchmode = 2;
    }
  else if (!strncmp (readbuf, "ERROR\r", 6) && bytesread >= 7)
    {
      sl_log_r (slconn, 1, 2, "[%s] BATCH not accepted\n", slconn->sladdr);
    }
  else
    {
      sl_log_r (slconn, 2, 0,
    	    "[%s] invalid response to BATCH command: %.*s\n",
    	    slconn->sladdr, bytesread, readbuf);
      return -1;
    }
  
  return 0;
}  /* End of sl_batchmode() */


/***************************************************************************
 * sl_ping:
 *
 * Connect to a server, issue the HELLO command, parse out the server
 * ID and organization resonse and disconnect.  The server ID and
 * site/organization strings are copied into serverid and site strings
 * which should have 100 characters of space each.
 *
 * Returns:
 *   0  Success
 *  -1  Connection opened but invalid response to 'HELLO'.
 *  -2  Could not open network connection
 ***************************************************************************/
int
sl_ping (SLCD * slconn, char *serverid, char *site)
{
  int servcnt = 0;
  int sitecnt = 0;
  char sendstr[100];		/* A buffer for command strings */
  char servstr[100];		/* The remote server ident */
  char sitestr[100];		/* The site/data center ident */
  
  /* Open network connection to server */
  if ( sl_connect (slconn, 0) == -1 )
    {
      sl_log_r (slconn, 2, 1, "Could not connect to server\n");
      return -2;
    }
  
  /* Send HELLO */
  sprintf (sendstr, "HELLO\r");
  sl_log_r (slconn, 1, 2, "[%s] sending: HELLO\n", slconn->sladdr);
  sl_senddata (slconn, (void *) sendstr, strlen (sendstr), slconn->sladdr,
	       NULL, 0);
  
  /* Recv the two lines of response */
  if ( sl_recvresp (slconn, (void *) servstr, (size_t) sizeof (servstr), 
		    sendstr, slconn->sladdr) < 0 )
    {
      return -1;
    }
  
  if ( sl_recvresp (slconn, (void *) sitestr, (size_t) sizeof (sitestr),
		    sendstr, slconn->sladdr) < 0 )
    {
      return -1;
    }
  
  servcnt = strcspn (servstr, "\r");
  if ( servcnt > 90 )
    {
      servcnt = 90;
    }
  servstr[servcnt] = '\0';
  
  sitecnt = strcspn (sitestr, "\r");
  if ( sitecnt > 90 )
    {
      sitecnt = 90;
    }
  sitestr[sitecnt] = '\0';
  
  /* Copy the response strings into the supplied strings */
  strncpy (serverid, servstr, sizeof(servstr));
  strncpy (site, sitestr, sizeof(sitestr));
  
  slconn->link = sl_disconnect (slconn);
  
  return 0;
}  /* End of sl_ping() */


/***************************************************************************
 * sl_checksock:
 *
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
sl_checksock (int sock, int tosec, int tousec)
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
  if ((sret = select (sock + 1, NULL, &checkset, NULL, &to)) > 0)
    ret = 1;
  else if (sret == 0)
    ret = 0;			/* time-out expired */

  /* Check read ability with recv() */
  if (ret && (recv (sock, testbuf, sizeof (char), MSG_PEEK)) <= 0)
    {
      if (! slp_noblockcheck())
	ret = 1;		/* no data for non-blocking IO */
      else
	ret = -1;
    }

  return ret;
}  /* End of sl_checksock() */


/***************************************************************************
 * sl_senddata:
 *
 * send() 'buflen' bytes from 'buffer' to 'slconn->link'.  'ident' is
 * a string to include in error messages for identification, usually
 * the address of the remote server.  If 'resp' is not NULL then read
 * up to 'resplen' bytes into 'resp' after sending 'buffer'.  This is
 * only designed for small pieces of data, specifically the server
 * responses to commands terminated by '\r\n'.
 *
 * Returns -1 on error, and size (in bytes) of the response
 * received (0 if 'resp' == NULL).
 ***************************************************************************/
int
sl_senddata (SLCD * slconn, void *buffer, size_t buflen,
	     const char *ident, void *resp, int resplen)
{
  
  int bytesread = 0;		/* bytes read into resp */
  ssize_t nwritten;
  size_t nleft = buflen;
  const char *ptr = (const char *) buffer;

  while ( nleft > 0 )
    {
      if ( (nwritten = send (slconn->link, ptr, nleft, 0)) < 0 )
        {
	  if ( errno == EAGAIN )
	    {
	      usleep(100000);
	      continue;
	    }

          sl_log_r (slconn, 2, 0, "[%s] error sending '%.*s'\n", ident,
		    strcspn ((char *) buffer, "\r\n"), (char *) buffer);
          return -1;
        }
	  
      nleft -= nwritten;
      ptr += nwritten;
    }
  
  /* If requested collect the response */
  if ( resp != NULL )
    {
      /* Clear response buffer */
      memset (resp, 0, resplen);
      
      if ( slconn->batchmode == 2 )
        {
	  /* Fake OK response */
	  strcpy(resp, "OK\r\n");
	  bytesread = 4;
	}
      else
        {
          bytesread = sl_recvresp (slconn, resp, resplen, buffer, ident);
	}
    }
  
  return bytesread;
}  /* End of sl_senddata() */


/***************************************************************************
 * sl_recvdata:
 *
 * recv() 'maxbytes' data from 'slconn->link' into a specified
 * 'buffer'.  'ident' is a string to be included in error messages for
 * identification, usually the address of the remote server.
 *
 * Returns -1 on error/EOF, 0 for no available data and the number
 * of bytes read on success.
 ***************************************************************************/
int
sl_recvdata (SLCD * slconn, void *buffer, size_t maxbytes,
	     const char *ident)
{
  int bytesread = 0;
  
  if ( buffer == NULL )
    {
      return -1;
    }
  
  bytesread = recv (slconn->link, buffer, maxbytes, 0);
  
  if ( bytesread == 0 )		/* should indicate TCP FIN or EOF */
    {
      sl_log_r (slconn, 1, 1, "[%s] recv():%d TCP FIN or EOF received\n",
		ident, bytesread);
      return -1;
    }
  else if ( bytesread < 0 )
    {
      if ( slp_noblockcheck() )
	{
	  sl_log_r (slconn, 2, 0, "[%s] recv():%d %s\n", ident, bytesread,
		    slp_strerror ());
	  return -1;
	}

      /* no data available for NONBLOCKing IO */
      return 0;
    }

  return bytesread;
}  /* End of sl_recvdata() */


/***************************************************************************
 * sl_recvresp:
 *
 * To receive a response to a command recv() one byte at a time until
 * '\r\n' or up to 'maxbytes' is read from 'slconn->link' into a
 * specified 'buffer'.  The function will wait up to 30 seconds for a
 * response to be recv'd.  'command' is a string to be included in
 * error messages indicating which command the response is
 * for. 'ident' is a string to be included in error messages for
 * identification, usually the address of the remote server.
 *
 * It should not be assumed that the populated buffer contains a
 * terminated string.
 *
 * Returns -1 on error/EOF and the number of bytes read on success.
 ***************************************************************************/
int
sl_recvresp (SLCD * slconn, void *buffer, size_t maxbytes,
	     const char *command, const char *ident)
{
  
  int bytesread = 0;		/* total bytes read */
  int recvret   = 0;            /* return from sl_recvdata */
  int ackcnt    = 0;		/* counter for the read loop */
  int ackpoll   = 50000;	/* poll at 0.05 seconds for reading */
  
  if ( buffer == NULL )
    {
      return -1;
    }
  
  /* Clear the receiving buffer */
  memset (buffer, 0, maxbytes);
  
  /* Recv a byte at a time and wait up to 30 seconds for a response */
  while ( bytesread < maxbytes )
    {
      recvret = sl_recvdata (slconn, (char *)buffer + bytesread, 1, ident);
      
      /* Trap door for termination */
      if ( slconn->terminate )
	{
	  return -1;
	}
      
      if ( recvret > 0 )
	{
	  bytesread += recvret;
	}
      else if ( recvret < 0 )
	{
	  sl_log_r (slconn, 2, 0, "[%s] bad response to '%.*s'\n",
		    ident, strcspn ((char *) command, "\r\n"),
		    (char *) command);
	  return -1;
	}
      
      /* Trap door if '\r\n' is recv'd */
      if ( bytesread >= 2 &&
	   *(char *)((char *)buffer + bytesread - 2) == '\r' &&
	   *(char *)((char *)buffer + bytesread - 1) == '\n' )
	{
	  return bytesread;
	}
      
      /* Trap door if 30 seconds has elapsed, (ackpoll x 600) */
      if ( ackcnt > 600 )
        {
	  sl_log_r (slconn, 2, 0, "[%s] timeout waiting for response to '%.*s'\n",
		    ident, strcspn ((char *) command, "\r\n"),
		    (char *) command);
	  return -1;
	}
      
      /* Delay if no data received */
      if ( recvret == 0 )
	{
	  slp_usleep (ackpoll);
	  ackcnt++;
	}
    }
  
  return bytesread;
}  /* End of sl_recvresp() */
