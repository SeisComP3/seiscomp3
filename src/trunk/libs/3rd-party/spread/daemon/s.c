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


#include "arch.h"

#include <string.h>

#include "spu_alarm.h"
#include "spu_events.h"
#include "spu_data_link.h"

#ifdef	ARCH_PC_WIN95

#include	<winsock.h>

WSADATA		WSAData;

#endif	/* ARCH_PC_WIN95 */

static  int     Num_bytes;
static  int     Num_packets;
static	char	IP[16];
static	int16u	Port;
static	sp_time	Delay;
static	int	Burst;

static	void 	Usage( int argc, char *argv[] );

int main( int argc, char *argv[] )
{

	channel chan;
	sys_scatter scat;
	char	buf[100000];
	int	ret,i,i1,i2,i3,i4;
	int	address;
	int32	*type;
	int32	*count;
	int32	*size;
	sp_time	start, end, total_time;
	int	total_problems=0;

	Usage( argc, argv );

	Alarm_set_types( NONE ); 

#ifdef	ARCH_PC_WIN95

	ret = WSAStartup( MAKEWORD(1,1), &WSAData );
	if( ret != 0 )
		Alarm( EXIT, "s: winsock initialization error %d\n", ret );

#endif	/* ARCH_PC_WIN95 */

	chan = DL_init_channel( SEND_CHANNEL, Port, 0, 0 );

	scat.num_elements = 1;
	scat.elements[0].buf = buf;
	scat.elements[0].len = Num_bytes;

	sscanf( IP ,"%d.%d.%d.%d",&i1, &i2, &i3, &i4);
	address = ( (i1 << 24 ) | (i2 << 16) | (i3 << 8) | i4 );

	printf("Checking (%d.%d.%d.%d, %hu). Each burst has %d packets, %d bytes each with %ld msec delay in between, for a total of %d packets\n",i1,i2,i3,i4, Port, Burst, Num_bytes, Delay.usec/1000+Delay.sec*1000, Num_packets );

	type  = (int32 *)buf;
	count = (int32 *)&buf[4];
	size  = (int32 *)&buf[8];

	*type = Set_endian( 0 );
	*size = Num_bytes;
	start = E_get_time();
	for(i=1; i<= Num_packets; i++ )
	{
		*count = i;
		ret = DL_send( chan, address, Port, &scat );
		if( ret != Num_bytes) 
		{
			total_problems++;
			i--;
		}
		if( i%Burst == 0 ) E_delay( Delay );
		if( i%1000  == 0) printf("sent %d packets of %d bytes\n",i, Num_bytes);
	}
	end = E_get_time();
	total_time = E_sub_time( end, start );
	Delay.usec = 10000;
	*type = Set_endian( 1 );
	E_delay( Delay );
	ret = DL_send( chan, address, Port, &scat );
	printf("total time is (%ld,%ld), with %d problems \n",total_time.sec, total_time.usec, total_problems );

        exit(0);
}

static  void    Usage(int argc, char *argv[])
{
        /* Setting defaults */
	Num_bytes = 1024;
	Num_packets = 10000;
	strcpy( IP, "127.0.0.1" );
	Port = 4444;
	Delay.sec = 0;
	Delay.usec = 10000;
	Burst	  = 100;

	while( --argc > 0 )
	{
		argv++;

                if( !strncmp( *argv, "-t", 2 ) )
		{
			sscanf(argv[1], "%ld", &Delay.usec );
			Delay.usec = Delay.usec*1000;
			Delay.sec = 0;
			if( Delay.usec > 1000000 )
			{
				Delay.sec  = Delay.usec / 1000000;
				Delay.usec = Delay.usec % 1000000;
			}
			argc--; argv++;
		}else if( !strncmp( *argv, "-p", 2 ) ){
			sscanf(argv[1], "%hu", &Port );
			argc--; argv++;
		}else if( !strncmp( *argv, "-b", 2 ) ){
			sscanf(argv[1], "%d", &Burst );
			argc--; argv++;
		}else if( !strncmp( *argv, "-n", 2 ) ){
			sscanf(argv[1], "%d", &Num_packets );
			argc--; argv++;
		}else if( !strncmp( *argv, "-s", 2 ) ){
			sscanf(argv[1], "%d", &Num_bytes );
			argc--; argv++;
		}else if( !strncmp( *argv, "-a", 2 ) ){
			sscanf(argv[1], "%s", IP );
			argc--; argv++;
                }else{
			printf( "Usage: \n%s\n%s\n%s\n%s\n%s\n%s\n",
				"\t[-p <port number>] : to send on, default is 4444",
				"\t[-b <burst>]       : number of packets in each burst, default is 100",
				"\t[-t <delay>]       : time (mili-secs) to wait between bursts, default 10",
				"\t[-n <num packets>] : total number of packets to send, default is 10000",
				"\t[-s <num bytes>]   : size of each packet, default is 1024",
				"\t[-a <IP address>]  : default is 127.0.0.1" );
			exit( 0 );
		}
	}
}

