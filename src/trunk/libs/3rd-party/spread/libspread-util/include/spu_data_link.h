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


#ifndef INC_DATA_LINK
#define INC_DATA_LINK

#include "spu_system.h"
#include "spu_scatter.h"

#define		MAX_PACKET_SIZE		1472    /*1472 = 1536-64 (of udp)*/

#define		SEND_CHANNEL	0x00000001
#define		RECV_CHANNEL    0x00000002
#define         NO_LOOP         0x00000004
#define         REUSE_ADDR      0x00000008
#define         DL_BIND_ALL     0x00000010

#define         IS_MCAST_ADDR(addr)     ( ( (addr) & 0xF0000000 ) == 0xE0000000 )
#define         IS_MCAST_ADDR_NET(addr) ( ( (unsigned char*) &(addr) )[0] == 0xE0 )

channel	DL_init_channel( int32 channel_type, int16 port, int32 mcast_address, int32 interface_address );
void    DL_close_channel(channel chan);
int	DL_send( channel chan, int32 address, int16 port, sys_scatter *scat );
int	DL_recv( channel chan, sys_scatter *scat );
int	DL_recvfrom( channel chan, sys_scatter *scat, int *src_address, unsigned short *src_port );

#endif  /* INC_DATA_LINK */
