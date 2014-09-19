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

//static char rcsid[] = "$Id: scream.c 1364 2008-10-24 18:42:33Z andres $";

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
#include <sys/socket.h>
#include <netdb.h>

extern char *dumpfile;
extern int DEBUG;

static int sockfd = -1;
//static int socktcp = -1;
//static int protocol;


/* For SCM_PROTO_UDP, server is ignored, port is the */
/*   local port to listen on */

void
scream_init_socket (char *server, int port)
{
  //struct sockaddr_in local, remote;
  struct sockaddr_in local;

 sockfd = socket (PF_INET, SOCK_DGRAM, 0);

 /*For the UDP binding we want to be able to rebind to this listening port */

  int on = 1;

  //if (setsockopt (sockfd, SOCK_STREAM, SO_REUSEADDR, (void *) &on, sizeof (on)))
  if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof (on)))
        warning (("Error setting SO_REUSEADDR: %m"));

 local.sin_family      = AF_INET;
 local.sin_port        = htons (port);
 local.sin_addr.s_addr = INADDR_ANY;

 if (bind (sockfd, (struct sockaddr *) &local, sizeof (local))) {
    printf("Bind a socket to port %d  failed in scream.c\n", port);
    exit(0);
 }
}

void scream_receive (int *thisblocknr, uint8_t *buf)
{
    int     n, blocknr;
    FILE    *fd;

    memset(buf,0,sizeof(buf));

    // try to get 1077 bytes here 

    n = read ( sockfd, buf, SCREAM_MAX_LENGTH);
    if ( n < 0 )
        fatal (("recv failed - no UDP connection ?"));


    //  useful for writing the raw GCF records to file
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

    return;

}
