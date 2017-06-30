/*
 * scream.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char rcsid[] = "$Id: scream.c 1364 2008-10-24 18:42:33Z andres $";

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.7  2003/02/28 17:05:37  root
 * #
 *
 */

/* Mar 2004  - Modified by Reinoud Sleeman (ORFEUS/KNMI)  */
/*             for the SCREAM plugin in SeedLink          */


#include "project.h"

// Note: MSG_CONFIRM flag for send() does not exist on Darwin (OS X & FreeBSD)
//
#ifdef __APPLE__
#include <sys/types.h>
#define MSG_CONFIRM 1
#endif

#include <sys/socket.h>
#include <netdb.h>

static int sockfd = -1;
static int protocol;

/* _protocol is one of SCM_PROTO_... */
/* For SCM_PROTO_UDP, server is ignored, port is the */
/*   local port to listen on */
/* For SCM_PROTO_TCP, server is the remote server to connect */
/*   to, and port is the port on that server, the local port */
/*   is assigned by the OS*/

void
scm_init (int _protocol, char *server, int port)
{
  struct sockaddr_in local, remote;
  struct hostent *he;
  uint8_t cmd;

  protocol = _protocol;
  //_protocol = SCM_PROTO_TCP;
 
  switch (_protocol)
    {
    case SCM_PROTO_TCP:

      sockfd = socket (PF_INET, SOCK_STREAM, 0);

      local.sin_family = AF_INET;
      local.sin_port = 0;
      local.sin_addr.s_addr = INADDR_ANY;

      if (bind (sockfd, (struct sockaddr *) &local, sizeof (local)))
          fatal (("bind failed: %m"));

      remote.sin_family = AF_INET;
      remote.sin_port = htons (port);

      he = gethostbyname (server);

      if (!he) fatal (("gethostbyname(%s) failed: %m", server));

      if (he->h_addrtype != AF_INET)
          fatal (("gethostbyname returned a non-IP address"));

      memcpy (&remote.sin_addr.s_addr, he->h_addr, he->h_length);

      if (connect (sockfd, (struct sockaddr *) &remote, sizeof (remote))) {
          printf("connect failed\n");
      }
      else
      {
        cmd = SCREAM_CMD_START_XMIT;
        printf("send SCREAM_CMD_START_XMIT\n");

        if (send (sockfd, &cmd, 1,  MSG_CONFIRM) != 1)
           fatal (("write to socket failed: %m"));

      }


      break;

    case SCM_PROTO_UDP:

      sockfd = socket (PF_INET, SOCK_DGRAM, 0);

      /*For the UDP binding we want to be able to rebind to this listening port */

      {
        int on = 1;

        if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof (on)))

            warning (("Error setting SO_REUSEADDR: %m"));
      }

      local.sin_family = AF_INET;
      local.sin_port = htons (port);
      local.sin_addr.s_addr = INADDR_ANY;

      if (bind (sockfd, (struct sockaddr *) &local, sizeof (local)))
        fatal (("bind failed: %m"));


      break;

    default:
      fatal (("Unknown protocol"));
      break;
    }


}

void
scm_dispatch ()
{
  uint8_t buf[SCREAM_MAX_LENGTH];
  int blocknr;

  static int blocknr_prev;

  switch (protocol)
  {

        case SCM_PROTO_UDP:
	    memset(buf,0,sizeof(buf));

            if (recv (sockfd, buf, SCREAM_MAX_LENGTH, 0) < 0)
                fatal (("recv failed: %m"));
 

/*printf("version: %c %d %x\n", buf[GCF_BLOCK_LEN],buf[GCF_BLOCK_LEN],buf[GCF_BLOCK_LEN]); */

            switch (buf[GCF_BLOCK_LEN])
            {
            case 31:

                  //printf("Version 3.1\n");
                  blocknr = buf[GCF_BLOCK_LEN+34]*256 + buf[GCF_BLOCK_LEN+35];
                  break;
            case 40:
                  //printf("Version 4.0\n");
                  blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
                  break;
            case 45:
                  blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
                  break;	          
            default:
                  fprintf(stderr, "Scream version ID = %d\n", buf[GCF_BLOCK_LEN]);
                  fatal (("Unknown version of scream protocol at remote end"));
                  break;
            }

        break;   /* (case SCM_PROTO_UDP)  */


        case SCM_PROTO_TCP:
/*This horrible hack is because the protocol version isn't until later...*/

          //printf("try to read %d bytes\n", SCREAM_INITIAL_LEN) ;

          if (complete_read 
              (sockfd, (char *) buf, SCREAM_INITIAL_LEN) != SCREAM_INITIAL_LEN)
            fatal (("read failed---------"));


          switch (buf[GCF_BLOCK_LEN])
          {
            case 31:
                  if (complete_read
                      (sockfd, (char *) buf + SCREAM_INITIAL_LEN,
                       SCREAM_V31_SUBSEQUENT) != SCREAM_V31_SUBSEQUENT)
                    fatal (("read failed: %m"));
                  blocknr = buf[GCF_BLOCK_LEN+34]*256 + buf[GCF_BLOCK_LEN+35];
                  break;
            case 40:
                  if (complete_read
                      (sockfd, (char *) buf + SCREAM_INITIAL_LEN,
                       SCREAM_V40_SUBSEQUENT) != SCREAM_V40_SUBSEQUENT)
                    fatal (("read failed: %m"));
                  blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
                  break;
            case 45:
                  if (complete_read
                      (sockfd, (char *) buf + SCREAM_INITIAL_LEN,
                       SCREAM_V45_SUBSEQUENT) != SCREAM_V45_SUBSEQUENT)
                    fatal (("read failed: %m"));
                  blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
		  break;
            default:
                  fatal (("Unknown version of scream protocol at remote end"));
                  break;
          }

        //gcf_dispatch (buf, GCF_BLOCK_LEN, blocknr);

        break;   /* (case SCM_PROTO_TCP)  */

        default:
        return;
    }

    if ( blocknr_prev == 0 ) blocknr_prev = blocknr;

    //printf("UDP blocknr = %d\n", blocknr);
    //fflush(stdout);
    if ( blocknr - blocknr_prev > 1 ) {
         do {
              //printf("Block %d is missing !\n", blocknr_prev +1);
              fflush(stdout);
              blocknr_prev++;
         } while ( blocknr - blocknr_prev > 1 );
    }
  
    blocknr_prev = blocknr;

    gcf_dispatch (buf, GCF_BLOCK_LEN, blocknr);
}
