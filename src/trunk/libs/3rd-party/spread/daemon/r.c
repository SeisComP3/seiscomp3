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
#include "spu_data_link.h"

#ifdef	ARCH_PC_WIN95

#include	<winsock.h>

WSADATA		WSAData;

#endif	/* ARCH_PC_WIN95 */

static	char	IP[16];
static	int16u	Port;
static	int32	Address;
static 	int32	Interface_addr;
static	int	Detailed_report;

static  void    Usage( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
	channel chan;
	sys_scatter scat;
	char	buf[100000];
	int	ret,i;
	int	*type;
	int	*count;
	int	*size;
	int	missed;
	int	corrupt;
	int	total_missed;

	Usage( argc, argv );

	Alarm_set_types( NONE ); 

#ifdef	ARCH_PC_WIN95

	ret = WSAStartup( MAKEWORD(1,1), &WSAData );
	if( ret != 0 )
		Alarm( EXIT, "r: winsock initialization error %d\n", ret );

#endif	/* ARCH_PC_WIN95 */

	chan = DL_init_channel( RECV_CHANNEL, Port, Address, Interface_addr );

	scat.num_elements = 1;
	scat.elements[0].buf = buf;
	scat.elements[0].len = sizeof(buf);

	type  = (int32 *)buf;
	count = (int32 *)&buf[4];
	size  = (int32 *)&buf[8];

	*count = 0;
	corrupt = 0;
	total_missed = 0;

printf("Ready to receive on port %hu\n", Port );

	for(i=0; ; i++ )
	{
		ret = DL_recv( chan, &scat );
		if( !Same_endian( *type ) )
		{
			*type  = Flip_int32( *type  );
			*count = Flip_int32( *count );
			*size  = Flip_int32( *size  );
		}
		*type  = Clear_endian( *type );

		if( *size != ret )
		{
			i--;
			corrupt++;
			printf("Corruption: expected packet size is %d, received packet size is %d\n", *size, ret);
			continue;
		}
		if( *type )
		{
			i--;
			missed = *count - i;
			total_missed += missed;
			printf("-------\n");
			printf("Report: total packets %d, total missed %d, total corrupted %d\n", *count, total_missed, corrupt );
			printf("-------\n");

			i = 0;
			total_missed = 0;
			corrupt = 0;
			continue;
		}
		if (*count > i)
		{
			missed = *count -i;
			total_missed += missed;
			if( Detailed_report ) 
				printf(" --> count is %d, i is %d, missed %d total missed %d, corrupt %d\n", *count,i, missed, total_missed, corrupt );
			 i = *count;
		}else if(*count < i){
			printf("-------\n");
			printf("Report: total packets at least %d, total missed %d, total corrupted %d\n", i-1, total_missed, corrupt );
			printf( "Initiating count from %d to %d\n",i-1,*count);
			printf("-------\n");

			i = *count;
			total_missed = *count;
			corrupt = 0;
		}
	}
        exit(0);
}

static  void    Usage(int argc, char *argv[])
{
	int i1, i2, i3, i4;

        /* Setting defaults */
	Port = 4444;
	Address = 0;
	Interface_addr = 0;
	Detailed_report= 0;
	while( --argc > 0 )
	{
		argv++;

                if( !strncmp( *argv, "-d", 2 ) )
		{
			Detailed_report = 1;
			argc--;
		}else if( !strncmp( *argv, "-p", 2 ) ){
			sscanf(argv[1], "%hu", &Port );
			argc--; argv++;
                }else if( !strncmp( *argv, "-a", 2 ) ){
			sscanf(argv[1], "%s", IP );

			sscanf( IP ,"%d.%d.%d.%d",&i1, &i2, &i3, &i4);
			Address = ( (i1 << 24 ) | (i2 << 16) | (i3 << 8) | i4 );

			argc--; argv++;
                }else if( !strncmp( *argv, "-i", 2 ) ){
			sscanf(argv[1], "%s", IP );

			sscanf( IP ,"%d.%d.%d.%d",&i1, &i2, &i3, &i4);
			Interface_addr = ( (i1 << 24 ) | (i2 << 16) | (i3 << 8) | i4 );

			argc--; argv++;
                }else{
			printf( "Usage: r\n%s\n%s\n%s\n%s\n",
				"\t[-p <port number>] : to receive on, default is 4444",
				"\t[-a <multicast class D address>] : if receiving multicast is desirable, default is 0",
				"\t[-i <IP interface>] : set interface, default is 0",
				"\t[-d ]              : print a detailed report whenever messages are missed");
			exit( 0 );
		}
	}
}

