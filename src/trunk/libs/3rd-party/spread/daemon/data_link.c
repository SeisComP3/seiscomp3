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


#include <stdlib.h>
#include "arch.h"

#ifndef	ARCH_PC_WIN95

#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <arpa/inet.h>
#include        <sys/uio.h>
/* for select */
#include        <sys/time.h>
#include        <unistd.h>

#include        <errno.h>
#else	/* ARCH_PC_WIN95 */

#include	<winsock.h>

#endif	/* ARCH_PC_WIN95 */

#include <string.h>
#include <assert.h>
#include "data_link.h"
#include "status.h"
#include "alarm.h"
#include "sp_events.h" /* for sp_time */

channel	DL_init_channel( int32 channel_type, int16 port, int32 mcast_address, int32 interface_address )
{
	channel			chan;
	struct  sockaddr_in	soc_addr;
	int			on=1;
        int			i1,i2,i3,i4;
#ifdef	IP_MULTICAST_TTL
	unsigned char		ttl_val;
#endif

	if((chan = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		Alarm( EXIT, "DL_init_channel: socket error for port %d\n", port );

	if ( channel_type & SEND_CHANNEL )
	{
        	if (setsockopt(chan, SOL_SOCKET, SO_BROADCAST, (char *)&on, 
			       sizeof(on)) < 0) 
            		Alarm( EXIT, "DL_init_channel: setsockopt error for port %d\n",port);
		Alarm( DATA_LINK, "DL_init_channel: setsockopt for send and broadcast went ok\n");

#ifdef	IP_MULTICAST_TTL
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
        	soc_addr.sin_family    	= AF_INET;
        	soc_addr.sin_port	= htons(port);
                memset(&soc_addr.sin_zero, 0, sizeof(soc_addr.sin_zero));
                if (interface_address == 0)
                        soc_addr.sin_addr.s_addr= INADDR_ANY;
                else 
                        soc_addr.sin_addr.s_addr= htonl(interface_address);

	        if(bind( chan, (struct sockaddr *) &soc_addr, 
				sizeof(soc_addr)) == -1) 
		{
                	Alarm( EXIT, "DL_init_channel: bind error (%d): %s for port %d, with sockaddr (%d.%d.%d.%d: %d) probably already running \n", sock_errno, sock_strerror(sock_errno), port, IP1(soc_addr.sin_addr.s_addr),IP2(soc_addr.sin_addr.s_addr),IP3(soc_addr.sin_addr.s_addr),IP4(soc_addr.sin_addr.s_addr), soc_addr.sin_port );
		}
		Alarm( DATA_LINK, "DL_init_channel: bind for recv_channel for port %d with chan %d ok\n",
			port, chan);

		i1 = (mcast_address >> 24) & 0x000000ff;
		i2 = (mcast_address >> 16) & 0x000000ff;
		i3 = (mcast_address >>  8) & 0x000000ff;
		i4 =  mcast_address & 0x000000ff;
		if( i1 >=224 && i1 < 240 )
		{
#ifdef IP_MULTICAST_TTL
			struct ip_mreq	mreq;

			mreq.imr_multiaddr.s_addr = htonl( mcast_address );

			/* the interface could be changed to a specific interface if needed */
			mreq.imr_interface.s_addr = INADDR_ANY;

        		if (setsockopt(chan, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq, 
		       		sizeof(mreq)) < 0) 
			{
				Alarm( EXIT, "DL_init_channel: problem in setsockopt to multicast address %d\n", mcast_address );
			}

			Alarm( DATA_LINK, "DL_init_channel: Joining multicast address %d.%d.%d.%d went ok\n",i1,i2,i3,i4);
#else	/* no multicast support */
			Alarm( EXIT, "DL_init_channel: Old SunOS architecture does not support IP multicast: %d.%d.%d.%d\n",i1,i2,i3,i4);
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
int	DL_send( channel chan, int32 address, int16 port, sys_scatter *scat )
{

#ifndef ARCH_SCATTER_NONE
static	struct	msghdr		msg;
#else	/* ARCH_SCATTER_NONE */
static	char	pseudo_scat[MAX_PACKET_SIZE];
#endif	/* ARCH_SCATTER_NONE */
	
static	struct  sockaddr_in	soc_addr;
static	struct timeval 		select_delay = { 0, 10000 };
	int			ret;
	int			total_len;
	int			i;
	int			num_try;
        char                    *send_errormsg = NULL; /* fool compiler */

        /* Check that the scatter passed is small enough to be a valid system scatter */
        assert(scat->num_elements <= ARCH_SCATTER_SIZE);

	soc_addr.sin_family 	= AF_INET;
	soc_addr.sin_addr.s_addr= htonl(address);
	soc_addr.sin_port	= htons(port);

#ifdef ARCH_PC_HOME
	soc_addr.sin_addr.s_addr= htonl(-1073741814);
#endif /* ARCH_PC_HOME */

#ifndef ARCH_SCATTER_NONE
	msg.msg_name 	= (caddr_t) &soc_addr;
	msg.msg_namelen = sizeof(soc_addr);
	msg.msg_iov	= (struct iovec *)scat->elements;
	msg.msg_iovlen	= scat->num_elements;
#endif /* ARCH_SCATTER_NONE */

#ifdef	ARCH_SCATTER_CONTROL
	msg.msg_controllen = 0;
#endif	/* ARCH_SCATTER_CONTROL */
#ifdef	ARCH_SCATTER_ACCRIGHTS
	msg.msg_accrightslen = 0;
#endif	/* ARCH_SCATTER_ACCRIGHTS */

	for( i=0, total_len=0; i < scat->num_elements; i++)
	{
#ifdef	ARCH_SCATTER_NONE
		memcpy( &pseudo_scat[total_len], scat->elements[i].buf, 
				scat->elements[i].len );
#endif	/* ARCH_SCATTER_NONE */
		total_len+=scat->elements[i].len;
	}
#if 0
#ifndef ARCH_SCATTER_NONE
        if( msg.msg_iovlen > 16)
        {
                Alarm(EXIT, "Too Big iovec of size %d\n", msg.msg_iovlen);
        }
#endif
#endif
	for( ret=-10, num_try=0; ret < 0 && num_try < 10; num_try++ )
	{
#ifndef	ARCH_SCATTER_NONE
		ret = sendmsg(chan, &msg, 0); 
#else	/* ARCH_SCATTER_NONE */
		ret = sendto(chan, pseudo_scat, total_len, 0,
			     (struct sockaddr *)&soc_addr, sizeof(soc_addr) );
#endif	/* ARCH_SCATTER_NONE */
		if(ret < 0) {
			/* delay for a short while */
                        send_errormsg = sock_strerror(sock_errno);
			Alarm( DATA_LINK, "DL_send: delaying after failure in send to %d.%d.%d.%d, ret is %d\n", 
				IP1(address), IP2(address), IP3(address), IP4(address), ret);
			select( 0, 0, 0, 0, &select_delay );
		}
	}
	if (ret < 0)
	{		
        	for( i=0; i < scat->num_elements; i++)
		    Alarm( DATA_LINK, "DL_send: element[%d]: %d bytes\n",
			    i,scat->elements[i].len);
		Alarm( DATA_LINK, "DL_send: error: %s\n sending %d bytes on channel %d to address %d.%d.%d.%d\n",
                       send_errormsg, total_len,chan,IP1(address), IP2(address), IP3(address), IP4(address) );
	}else if(ret < total_len){
		Alarm( DATA_LINK, "DL_send: partial sending %d out of %d\n",
			ret,total_len);
	}
	Alarm( DATA_LINK, "DL_send: sent a message of %d bytes to (%d.%d.%d.%d,%d) on channel %d\n",
		ret,IP1(address), IP2(address),IP3(address), IP4(address),port,chan);

	return(ret);
}

int	DL_recv( channel chan, sys_scatter *scat )
{
#ifndef ARCH_SCATTER_NONE
static	struct	msghdr	msg;
        struct  sockaddr_in     source_address;
#else	/* ARCH_SCATTER_NONE */
static	char		pseudo_scat[MAX_PACKET_SIZE];
	int		bytes_to_copy;
	int		total_len;
	int		start;
	int		i;
#endif	/* ARCH_SCATTER_NONE */

	int		ret;

        /* check the scat is small enough to be a sys_scatter */
        assert(scat->num_elements <= ARCH_SCATTER_SIZE);

#ifndef ARCH_SCATTER_NONE
	msg.msg_name 	= (caddr_t) &source_address;
	msg.msg_namelen = sizeof(source_address);
	msg.msg_iov	= (struct iovec *)scat->elements;
	msg.msg_iovlen	= scat->num_elements;
#endif	/* ARCH_SCATTER_NONE */

#ifdef ARCH_SCATTER_CONTROL
	msg.msg_control = (caddr_t) 0;
	msg.msg_controllen = 0;
#endif /* ARCH_SCATTER_CONTROL */
#ifdef ARCH_SCATTER_ACCRIGHTS
	msg.msg_accrights = (caddr_t) 0;
	msg.msg_accrightslen = 0;
#endif /* ARCH_SCATTER_ACCRIGHTS */

#ifndef ARCH_SCATTER_NONE
	ret = recvmsg( chan, &msg, 0 ); 
#else	/* ARCH_SCATTER_NONE */
        
	total_len = 0;                             /*This is for TCP, to not receive*/
	for(i=0; i<scat->num_elements; i++)     /*more than one packet.          */
	   total_len += scat->elements[i].len;
        
        if(total_len>MAX_PACKET_SIZE)
           total_len = MAX_PACKET_SIZE;

	ret = recvfrom( chan, pseudo_scat, total_len, 0, 0, 0 );
	
	for( i=0, total_len = ret, start =0; total_len > 0; i++)
	{
		bytes_to_copy = scat->elements[i].len;
		if( bytes_to_copy > total_len ) bytes_to_copy = total_len;
		memcpy( scat->elements[i].buf, &pseudo_scat[start], 
                        bytes_to_copy );
		total_len-= scat->elements[i].len;
		start    += scat->elements[i].len;
	}
#endif	/* ARCH_SCATTER_NONE */
	if (ret < 0)
	{
		Alarm( DATA_LINK, "DL_recv: error %d receiving on channel %d\n", ret, chan );
		return( -1 );
	} 
#ifdef ARCH_SCATTER_CONTROL
        else if (ret == 0)
        {
                char    *sptr;
                sptr = (char *) inet_ntoa(source_address.sin_addr);
                Alarm( DATA_LINK, "DL_recv: received zero length packet on channel %d flags 0x%x\nfrom %s", chan, msg.msg_flags,sptr );
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
	Alarm( DATA_LINK, "DL_recv: received %d bytes on channel %d\n",
			ret, chan );

	return(ret);
}
