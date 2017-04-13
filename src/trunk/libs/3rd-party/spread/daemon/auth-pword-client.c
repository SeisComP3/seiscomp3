/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

/* Implements the functions required by the client side SP library 
 * to authenticates users based on a clear-text password that they send to 
 * the daemon.
 *
 */

#include "arch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifndef ARCH_PC_WIN95

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#else   /* ARCH_PC_WIN95 */

#include <winsock.h>

#endif  /* ARCH_PC_WIN95 */

#include "auth-pword-client.h"

int pword_authenticate(int fd, void *data_p)
{
    struct user_password *user_p;
    int ret;
    char response;

    user_p = data_p;

    /* Send username and password */
    while(((ret = send( fd, user_p->username, MAX_PWORD_USERNAME, 0 )) == -1) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)))
        ;
    if( ret != (MAX_PWORD_USERNAME) )
    {
        printf("pword_authenticate: unable to send username %d %d: %s\n", ret, MAX_PWORD_USERNAME, sock_strerror(sock_errno));
        return( 0 );
    }
    while(((ret = send( fd, user_p->password, MAX_PWORD_PASSWORD, 0 )) == -1) && ((errno == EINTR) || (errno == EAGAIN) || (sock_errno == EWOULDBLOCK)))
        ;
    if( ret != (MAX_PWORD_PASSWORD) )
    {
        printf("pword_authenticate: unable to send password %d %d: %s\n", ret, MAX_PWORD_PASSWORD, sock_strerror(sock_errno));
        return( 0 );
    }

    /* Receive response code */
    ret = recv( fd, &response, 1, 0 );
    if( ret < 0 )
    {
            printf("pword_authenticate: reading response failed on mailbox %d\n", fd );
            return( 0 );
    }
    if( ret < 1 )
    {
            printf("pword_authenticate: reading response string SHORT on mailbox %d\n", fd );
            return( 0 );
    }
    if ( response == 1 )
        return( 1 );
    else 
        return( 0 );
}
