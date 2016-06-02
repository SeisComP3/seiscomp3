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

/* static char rcsid[] = "$Id: scream.c 1364 2008-10-24 18:42:33Z andres $"; */

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


// Note: MSG_CONFIRM flag for send() does not exist on Darwin (OS X & FreeBSD)
//
#ifdef __APPLE__
//#include <sys/types.h>
#define MSG_CONFIRM 1
#endif

#include "project.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

extern char *dumpfile;
extern int DEBUG;
extern int dropflag;

static int sockfd = -1;
static int protocol;


/* For SCM_PROTO_UDP, server is ignored, port is the */
/*   local port to listen on */

int scream_init_socket (int _protocol, char *server, int port)
{
    struct sockaddr_in local, remote;
    struct hostent *he;
    uint8_t cmd;

    protocol = _protocol;

    switch (_protocol)
    {
      case SCM_PROTO_TCP:
        if ( dropflag ) {
          printf("connect failed\n");
          return 1;
        }

        sockfd = socket (PF_INET, SOCK_STREAM, 0);

        remote.sin_family = AF_INET;
        remote.sin_port = htons (port);

        he = gethostbyname (server);

        if (!he)
            fatal2 ("gethostbyname(%s) failed: %m", server);

        if (he->h_addrtype != AF_INET)
            fatal ("gethostbyname returned a non-IP address");

        memcpy (&remote.sin_addr.s_addr, he->h_addr, he->h_length);

        if (connect (sockfd, (struct sockaddr *) &remote, sizeof (remote))) {
            printf("connect failed\n");
            return 1;
        }
        else
        {
          cmd = SCREAM_CMD_START_XMIT;
          printf("send SCREAM_CMD_START_XMIT\n");

          if (send (sockfd, &cmd, 1,  MSG_CONFIRM) != 1)
          {
             printf("write to socket failed: %m");
             close(sockfd);
             sockfd = -1;
             return 1;
          }
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

        if (bind (sockfd, (struct sockaddr *) &local, sizeof (local))) {
            printf("Bind a socket to port %d  failed in scream.c\n", port);
            exit(0);
        }

        break;

      default:
        fatal (("Unknown protocol"));
        break;
      }

    return 0;
}

int scream_receive (int *thisblocknr, uint8_t *buf, int buflen)
{
    int     n, blocknr;
    FILE    *fd;

    switch (protocol)
    {
        case SCM_PROTO_UDP:
            memset(buf,0,buflen);

            /* try to get 1077 bytes here */

            n = read ( sockfd, buf, buflen);
            if ( n < 0 )
                fatal (("recv failed - no UDP connection ?"));

            /*  useful for writing the raw GCF records to file */
            if ( dumpfile != NULL ) {
               fd = (FILE *) fopen (dumpfile, "a");
               if ( fd != NULL ) {
                    fwrite (buf, 1, 1077, fd);
                    fclose (fd);
               }
            }

            if (DEBUG==1) printf("version = %d   GCF_BLOCK_LEN=%d  \n",
                                 buf[GCF_BLOCK_LEN], GCF_BLOCK_LEN);

            switch (buf[GCF_BLOCK_LEN])
            {
                case 31:
                    blocknr = buf[GCF_BLOCK_LEN+34]*256 + buf[GCF_BLOCK_LEN+35];
                    break;
                case 40:
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

            if(DEBUG==1) printf("Got UDP blocknr = %d\n", blocknr);

            *thisblocknr = blocknr;
            break;

        case SCM_PROTO_TCP:
            if (dropflag) {
                 printf("read failed, closing connection\n");
                 close(sockfd); sockfd = -1;
                 return 1;
            }

            if (complete_read(sockfd, (char *) buf, SCREAM_INITIAL_LEN) != SCREAM_INITIAL_LEN) {
                printf("read failed, closing connection\n");
                close(sockfd); sockfd = -1;
                return 1;
            }

            switch (buf[GCF_BLOCK_LEN])
            {
                case 31:
                    if (complete_read(sockfd, (char *) buf + SCREAM_INITIAL_LEN, SCREAM_V31_SUBSEQUENT) != SCREAM_V31_SUBSEQUENT) {
                        printf("read failed: %m\n");
                        close(sockfd); sockfd = -1;
                        return 1;
                    }
                    blocknr = buf[GCF_BLOCK_LEN+34]*256 + buf[GCF_BLOCK_LEN+35];
                    break;
                case 40:
                    if (complete_read(sockfd, (char *) buf + SCREAM_INITIAL_LEN, SCREAM_V40_SUBSEQUENT) != SCREAM_V40_SUBSEQUENT) {
                        printf("read failed: %m\n");
                        close(sockfd); sockfd = -1;
                        return 1;
                    }
                    blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
                    break;
                case 45:
                    if (complete_read(sockfd, (char *) buf + SCREAM_INITIAL_LEN, SCREAM_V45_SUBSEQUENT) != SCREAM_V45_SUBSEQUENT) {
                        printf("read failed: %m\n");
                        close(sockfd); sockfd = -1;
                        return 1;
                    }
                    blocknr = buf[GCF_BLOCK_LEN+2]*256 + buf[GCF_BLOCK_LEN+3];
                    break;
                default:
                    fatal (("Unknown version of scream protocol at remote end"));
                    break;
            }

            if(DEBUG==1) printf("Got TCP blocknr = %d\n", blocknr);

            *thisblocknr = blocknr;
            break;
    }

    return 0;

}
