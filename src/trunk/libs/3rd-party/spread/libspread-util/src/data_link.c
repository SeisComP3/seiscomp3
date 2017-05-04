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
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
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


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "arch.h"

#ifndef ARCH_PC_WIN95
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/uio.h>
#  include <sys/time.h>
#  include <unistd.h>
#else
#  include <winsock.h>
#endif

#include "spu_data_link.h"
#include "spu_alarm.h"
#include "spu_events.h" /* for sp_time */

#define DL_MAX_NUM_SEND_RETRIES 1

channel DL_init_channel( int32 channel_type, int16 port, int32 mcast_address, int32 interface_address )
{
        channel                 chan;
        struct  sockaddr_in     soc_addr;
        int                     on=1, off=0;
#ifdef  IP_MULTICAST_TTL
        unsigned char           ttl_val;
#endif

        if((chan = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
                Alarm( EXIT, "DL_init_channel: socket error for port %d\n", port );

        if ( channel_type & SEND_CHANNEL )
        {
                if (setsockopt(chan, SOL_SOCKET, SO_BROADCAST, (char *)&on, 
                               sizeof(on)) < 0) 
                        Alarm( EXIT, "DL_init_channel: setsockopt error for port %d\n",port);
                Alarm( DATA_LINK, "DL_init_channel: setsockopt for send and broadcast went ok\n");

#ifdef  IP_MULTICAST_TTL
                /* ### Isn't this for sending??? */
                ttl_val = 1;
                if (setsockopt(chan, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl_val, 
                        sizeof(ttl_val)) < 0) 
                {
                        Alarm( DATA_LINK, "DL_init_channel: problem in setsockopt of multicast ttl %d - ignore in WinNT or Win95\n", ttl_val );
                }
                Alarm( DATA_LINK, "DL_init_channel: setting Mcast TTL to %d\n",ttl_val);
#endif
        }

        if ( channel_type & RECV_CHANNEL )
        {
                memset(&soc_addr, 0, sizeof(soc_addr));
                soc_addr.sin_family     = AF_INET;
                soc_addr.sin_port       = htons(port);
                memset(&soc_addr.sin_zero, 0, sizeof(soc_addr.sin_zero));
#ifdef HAVE_SIN_LEN_IN_STRUCT_SOCKADDR_IN
                soc_addr.sin_len        = sizeof(soc_addr);
#endif

                /* If mcast channel, the interface means the interface to
                   receive mcast packets on, and not interface to bind.
                   Must bind multicast address instead */
                if (mcast_address != 0 && IS_MCAST_ADDR(mcast_address) && !(channel_type & DL_BIND_ALL) )
                        soc_addr.sin_addr.s_addr= htonl(mcast_address);
                else if (interface_address != 0)
                        soc_addr.sin_addr.s_addr= htonl(interface_address);
                else
                        soc_addr.sin_addr.s_addr= INADDR_ANY;

                /* Older Version
                 if (interface_address == 0)
                         soc_addr.sin_addr.s_addr= INADDR_ANY;
                 else 
                         soc_addr.sin_addr.s_addr= htonl(interface_address);
                 */
 
                if ( channel_type & REUSE_ADDR ) 
                {
                        if(setsockopt(chan, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on))) 
                        {
                            Alarm( EXIT, "DL_init_channel: Failed to set socket option REUSEADDR, errno: %d\n", errno);
                        }
                }


                if(bind( chan, (struct sockaddr *) &soc_addr, 
                                sizeof(soc_addr)) == -1) 
                {
                        Alarm( EXIT, "DL_init_channel: bind error (%d): %s for port %d, with sockaddr (" IPF ": %d) probably already running\n", sock_errno, sock_strerror(sock_errno), port, IP_NET(soc_addr.sin_addr.s_addr), ntohs(soc_addr.sin_port));
                }
                Alarm( DATA_LINK, "DL_init_channel: bind for recv_channel for " IPF " port %d with chan %d ok\n",
                        IP_NET(soc_addr.sin_addr.s_addr), port, chan);

                if( IS_MCAST_ADDR(mcast_address) )
                {
#ifdef IP_MULTICAST_TTL
                        struct ip_mreq  mreq;

                        mreq.imr_multiaddr.s_addr = htonl( mcast_address );

                        /* the interface could be changed to a specific interface if needed */
                        /* WAS: mreq.imr_interface.s_addr = INADDR_ANY;
                         * If specified, then want to route through it instead of
                         * based on routing decisions at the kernel */
                        /* IP_ADD_MEMBERSHIP requires that the imr_interface be an actual physical interface
                         * or INADDR_ANY. So if this is the special case of binding to multicast or broadcast,
                         * switch the join to use INADDR_ANY. In the case when the passed in interface
                         * is a regular physical interface, then join only on that one.
                         */
                        if ( IS_MCAST_ADDR(interface_address) )
                          mreq.imr_interface.s_addr = htonl( INADDR_ANY );
                        else
                          mreq.imr_interface.s_addr = htonl( interface_address );


                        if (setsockopt(chan, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq, 
                                sizeof(mreq)) < 0) 
                        {
                          Alarm( EXIT, "DL_init_channel: problem (errno %d:%s) in setsockopt to multicast address " IPF "\n", sock_errno, sock_strerror(sock_errno), IP(mcast_address) );
                        }

                        if ( channel_type & NO_LOOP ) 
                        { 
                            if (setsockopt(chan, IPPROTO_IP, IP_MULTICAST_LOOP, 
                                        (void *)&off, 1) < 0) 
                            { 
                              Alarm( EXIT, "DL_init_channel: problem (errno %d:%s) in setsockopt loop setting " IPF "\n", sock_errno, sock_strerror(sock_errno), IP(mcast_address)); 
                            } 
                        }

                        Alarm( DATA_LINK, "DL_init_channel: Joining multicast address " IPF " went ok\n", IP(mcast_address) );
#else   /* no multicast support */
                        Alarm( EXIT, "DL_init_channel: Old SunOS architecture does not support IP multicast: " IPF "\n", IP(mcast_address));
#endif
                } else {
                    if (setsockopt(chan, SOL_SOCKET, SO_BROADCAST, (char *)&on, 
                                   sizeof(on)) < 0) 
                        Alarm( EXIT, "DL_init_channel: setsockopt SO_BROADCAST error for port %d, socket %d\n",port,chan);
                    Alarm( DATA_LINK, "DL_init_channel: setsockopt for recv and broadcast went ok\n");
                }
        }

        Alarm( DATA_LINK, "DL_init_channel: went ok on channel %d\n",chan);
        return ( chan );
}

void    DL_close_channel(channel chan)
{

        if( -1 ==  close(chan))
        {
                Alarm(EXIT, "DL_close_channel: error closing channel %d\n", chan);
        }

}

int     DL_send( channel chan, int32 address, int16 port, sys_scatter *scat )
{
#ifndef ARCH_SCATTER_NONE
        struct  msghdr      msg;
#else
        char                pseudo_scat[MAX_PACKET_SIZE];
#endif
        
        struct  sockaddr_in soc_addr;
        struct timeval      select_delay;
        int                 ret;
        int                 total_len;
        int                 i;
        int                 num_try;

        /* Check that the scatter passed is small enough to be a valid system scatter */

        if( scat->num_elements > ARCH_SCATTER_SIZE ) {
          Alarmp( SPLOG_FATAL, DATA_LINK, "DL_send: illegal scat->num_elements (%d) > ARCH_SCATTER_SIZE (%d)\n", 
                  (int) scat->num_elements, (int) ARCH_SCATTER_SIZE );
        }

        memset( &soc_addr, 0, sizeof( soc_addr ) );
        soc_addr.sin_family      = AF_INET;
        soc_addr.sin_addr.s_addr = htonl( address );
        soc_addr.sin_port        = htons( port );

#ifdef HAVE_SIN_LEN_IN_STRUCT_SOCKADDR_IN
        soc_addr.sin_len = sizeof( soc_addr );
#endif

#ifdef ARCH_PC_HOME
        soc_addr.sin_addr.s_addr = htonl( -1073741814 );  /* WTF? */
#endif

#ifndef ARCH_SCATTER_NONE
        memset( &msg, 0, sizeof( msg ) );
        msg.msg_name    = (caddr_t) &soc_addr;
        msg.msg_namelen = sizeof( soc_addr );
        msg.msg_iov     = (struct iovec *) scat->elements;
        msg.msg_iovlen  = scat->num_elements;
#endif

        for( i = 0, total_len = 0; i < (int) scat->num_elements; ++i ) {

#ifdef ARCH_SCATTER_NONE
                memcpy( &pseudo_scat[total_len], scat->elements[i].buf, scat->elements[i].len );
#endif
                total_len += scat->elements[i].len;
        }

        for( num_try = 1;; ++num_try ) {

#ifndef ARCH_SCATTER_NONE
                ret = sendmsg( chan, &msg, 0 ); 
#else
                ret = sendto( chan, pseudo_scat, total_len, 0, (struct sockaddr *) &soc_addr, sizeof( soc_addr ) );
#endif

                if( ret >= 0 || num_try > DL_MAX_NUM_SEND_RETRIES ) { break; }  /* success or give up */

                /* delay for a short while */

                select_delay.tv_sec  = 0;
                select_delay.tv_usec = 100;

                Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: delaying for %ld.%06lds after failed send to (" IPF ":%d): %d %d '%s'\n", 
                        select_delay.tv_sec, select_delay.tv_usec, IP(address), port, ret, sock_errno, sock_strerror(sock_errno) );

                select( 0, NULL, NULL, NULL, &select_delay );

                Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: woke up; about to attempt send retry #%d\n", num_try );
        }

        if( ret < 0 ) {

                Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: error: %d %d '%s'\n", ret, sock_errno, sock_strerror(sock_errno) );

                for( i = 0; i < (int) scat->num_elements; ++i ) {
                        Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: element[%d]: %d bytes\n", i, (int) scat->elements[i].len );
                }

        } else if( ret < total_len ) {

                Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: partial sending %d out of %d\n", ret, total_len );

        } else if( ret != total_len ) {

                Alarmp( SPLOG_WARNING, DATA_LINK, "DL_send: unexpected return (%d) > total_len (%d)?!\n", ret, total_len );
        }

        Alarmp( SPLOG_INFO, DATA_LINK, "DL_send: ret = %d, sending a message of %d bytes to (" IPF ":%d) on channel %d\n", 
                ret, total_len, IP(address), port, chan );

        return( ret );
}

int DL_recv( channel chan, sys_scatter *scat )
{
    return( DL_recvfrom( chan, scat, NULL, NULL ) );
}

int     DL_recvfrom( channel chan, sys_scatter *scat, int *src_address, unsigned short *src_port )
{
#ifndef ARCH_SCATTER_NONE
static  struct  msghdr  msg;
#else
static  char            pseudo_scat[MAX_PACKET_SIZE];
        int             bytes_to_copy;
        int             total_len;
        int             start;
        int             i;
#endif

        struct  sockaddr_in     source_address;
        int             sip;
        unsigned short  sport;
        socklen_t       sa_len;
        int             ret;

        /* check the scat is small enough to be a sys_scatter */

        if( scat->num_elements > ARCH_SCATTER_SIZE ) {
          Alarmp( SPLOG_FATAL, DATA_LINK, "DL_recvfrom: illegal scat->num_elements (%d) > ARCH_SCATTER_SIZE (%d)\n", 
                  (int) scat->num_elements, (int) ARCH_SCATTER_SIZE );
        }

#ifndef ARCH_SCATTER_NONE
        memset( &msg, 0, sizeof( msg ) );
        msg.msg_name    = (caddr_t) &source_address;
        msg.msg_namelen = sizeof( source_address );
        msg.msg_iov     = (struct iovec *) scat->elements;
        msg.msg_iovlen  = scat->num_elements;
#endif

#ifndef ARCH_SCATTER_NONE
        ret = recvmsg( chan, &msg, 0 ); 
        sa_len = msg.msg_namelen;
#else
        
        total_len = 0;                             
        for( i=0; i < (int) scat->num_elements; ++i )
           total_len += scat->elements[i].len;
        
        if( total_len > MAX_PACKET_SIZE )  /* this is for TCP, to not receive more than one packet */
           total_len = MAX_PACKET_SIZE;

        sa_len = sizeof(source_address);
        ret = recvfrom( chan, pseudo_scat, total_len, 0, (struct sockaddr *) &source_address, &sa_len);
        
        for( i=0, total_len = ret, start =0; total_len > 0; i++)
        {
                bytes_to_copy = scat->elements[i].len;
                if( bytes_to_copy > total_len ) bytes_to_copy = total_len;
                memcpy( scat->elements[i].buf, &pseudo_scat[start], 
                        bytes_to_copy );
                total_len-= scat->elements[i].len;
                start    += scat->elements[i].len;
        }
#endif  /* ARCH_SCATTER_NONE */
        if (ret < 0)
        {
                Alarm( DATA_LINK, "DL_recv: error %d receiving on channel %d\n", ret, chan );
                return( -1 );
        } 
#ifdef ARCH_SCATTER_CONTROL
        else if (ret == 0)
        {
                char    *sptr;
                unsigned short port;
                Alarm( DATA_LINK, "DL_recv: received zero length packet on channel %d flags 0x%x msg_len %d\n", chan, msg.msg_flags, msg.msg_namelen);
                if (msg.msg_namelen >= sizeof(struct sockaddr_in) ) {
                    sptr = (char *) inet_ntoa(source_address.sin_addr);
                    port = Flip_int16(source_address.sin_port);
                    Alarm( DATA_LINK, "\tfrom %s with family %d port %d\n", sptr, source_address.sin_family, port );
                }
#ifdef  MSG_BCAST
                if ( msg.msg_flags & MSG_BCAST )
                {
                        Alarm( DATA_LINK, "\t(BROADCAST)");
                }
#endif
#ifdef  MSG_MCAST
                if ( msg.msg_flags & MSG_MCAST )
                {
                        Alarm( DATA_LINK, "\t(MULTICAST)");
                }
#endif
#ifdef  MSG_TRUNC
                if ( msg.msg_flags & MSG_TRUNC )
                {
                        Alarm( DATA_LINK, "\t(Data TRUNCATED)");
                }
#endif
#ifdef  MSG_CTRUNC
                if ( msg.msg_flags & MSG_CTRUNC )
                {
                        Alarm( DATA_LINK, "\t(Control TRUNCATED)");
                }
#endif
                Alarm( DATA_LINK, "\n");
        }
#endif
        /* Report the source address and port if requested by caller */
        if (sa_len >= sizeof(struct sockaddr_in) ) {
            memcpy(&sip, &source_address.sin_addr, sizeof(int32) );
            sip =  Flip_int32(sip);
            if (src_address != NULL)
                *src_address = sip;
            sport = Flip_int16(source_address.sin_port);
            if (src_port != NULL)
                *src_port = sport;
            Alarm( DATA_LINK, "\tfrom (" IPF ") with family %d port %d\n", IP(sip), source_address.sin_family, sport );
        }
        Alarm( DATA_LINK, "DL_recv: received %d bytes on channel %d\n",
                        ret, chan );

        return(ret);
}
