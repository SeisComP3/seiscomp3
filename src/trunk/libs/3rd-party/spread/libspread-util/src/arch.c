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
 *  Copyright (C) 1993-2006 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */


#include "arch.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef ARCH_PC_WIN95
#include <winsock.h>
#endif

#ifndef HAVE_STRERROR
/* return value only valid until next call to strerror */

char    *strerror(int err)
{
        char *msg;
static  char buf[32];
        
        sprintf(buf, "Error %d", err);
        msg = buf;

        return(msg);
}

#endif

#ifdef ARCH_PC_WIN95
/* return value only valid until next call to sock_strerror */

char    *sock_strerror(int err)
{
        switch( err ) {
        case WSANOTINITIALISED:
            return "WSANOTINITIALISED: A successful WSAStartup() must occur before using this function.";
        case WSAENETDOWN:
            return "WSAENETDOWN: The network subsystem has failed.";
        case WSAEACCES:
            return "WSAEACCES: The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.";
        case WSAEINTR:
            return "WSAEINTR: A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.";
        case WSAEINPROGRESS:
            return "WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEFAULT:
            return "WSAEFAULT: The buf parameter is not completely contained in a valid part of the user address space." ;
        case WSAENETRESET:
            return "WSAENETRESET: The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.";
        case WSAENOBUFS:
            return "WSAENOBUFS: No buffer space is available.";
        case WSAENOTCONN:
            return "WSAENOTCONN: The socket is not connected.";
        case WSAENOTSOCK:
            return "WSAENOTSOCK: The descriptor is not a socket.";
        case WSAEOPNOTSUPP:
            return "WSAEOPNOTSUPP: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.";
        case WSAESHUTDOWN:
            return "WSAESHUTDOWN The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.";
        case WSAEWOULDBLOCK:
            return "WSAEWOULDBLOCK: The socket is marked as nonblocking and the requested operation would block.";
        case WSAEMSGSIZE:
            return "WSAEMSGSIZE: The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.";
        case WSAEHOSTUNREACH:
            return "WSAEHOSTUNREACH: The remote host cannot be reached from this host at this time.";
        case WSAEINVAL:
            return "WSAEINVAL: The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.";
        case WSAECONNABORTED:
            return "WSAECONNABORTED: The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.";
        case WSAECONNRESET:
            return "WSAECONNRESET: The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable.";
        case WSAETIMEDOUT:
            return "WSAETIMEDOUT: The connection has been dropped, because of a network failure or because the system on the other end went down without notice.";
        default:
            return "Unknown WSA error!";
	}
}

#endif
