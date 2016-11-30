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


#include <stdio.h>
#include "arch.h"

/* undef redefined variables under windows */
#ifdef ARCH_PC_WIN95
#undef EINTR
#undef EAGAIN
#undef EWOULDBLOCK
#undef EINPROGRESS
#endif
#include <errno.h>

#include <string.h>

#include "log.h"
#include "configuration.h"
#include "membership.h"
#include "prot_body.h"
#include "spu_events.h"
#include "spu_alarm.h"

static	int	Is_inited = 0;

static	sp_time	alive_time;

static	FILE	*fd;
static	char    My_name[MAX_PROC_NAME];

static  void	Log_alive(int dummy, void *dummy_p);

void	Log_init()
{
	long	start_file_pos;
        proc    my;

	Is_inited = 1;

	my = Conf_my();
        strncpy( My_name, my.name, MAX_PROC_NAME);

	fd = fopen( My_name, "a" ); 
	if( fd == NULL )
		Alarm( EXIT, "Log_init: error (%s) could not open file %s\n",strerror(errno), My_name );
	start_file_pos = ftell(fd);
        if (start_file_pos == -1)
                Alarm( EXIT, "Log_init: failed to find end of file %s\n", My_name );
	fclose(fd);
	fd = fopen( My_name, "r+" ); 
	if( fd == NULL )
		Alarm( EXIT, "Log_init: error (%s) could not open file %s\n",strerror(errno), My_name );
	fseek( fd, start_file_pos, SEEK_SET );
	
	alive_time.sec  = 10;
	alive_time.usec =  0;

	Log_alive(0, NULL);
	fprintf( fd, "B %13ld\n",E_get_time().sec );
	fflush( fd );
}

static  void	Log_alive(int dummy, void *dummy_p)
{
	long	file_pos;

	if( !Is_inited ) return;
#if ( SPREAD_PROTOCOL == 3 )
	fprintf( fd, "A %13ld %11d\n",E_get_time().sec, Highest_seq );
#else
	fprintf( fd, "A %13ld \n",E_get_time().sec );
#endif
        if( fseek( fd, -28, SEEK_CUR ) )
                Alarm( EXIT, "Log_alive: error (%s) in fseek -28 on %s\n", strerror(errno), My_name);
	file_pos = ftell(fd);
	if( fseek( fd,  28, SEEK_CUR ) )
                Alarm( EXIT, "Log_alive: error (%s) in fseek 28 on %s\n", strerror(errno), My_name);
	fclose(fd);
	fd = fopen( My_name, "r+" ); 
	if( fd == NULL )
		Alarm( EXIT, "Log_alive: error (%s) could not open file %s\n",strerror(errno), My_name );
	if( fseek( fd, file_pos, SEEK_SET ) )
                Alarm( EXIT, "Log_alive: error (%s) in fseek file_pos (%ld) on %s\n", strerror(errno), file_pos,  My_name);
	
	E_queue( Log_alive, 0, NULL, alive_time );
}

void 	Log_membership()
{
	configuration	*Cn;
	int32		proc_id;
	proc		dummy_p;
	int		i,j;
	int		found;

	if( !Is_inited ) return;
#if ( SPREAD_PROTOCOL == 3 )
	fprintf( fd, "M %13ld %13d %11d > ",
		E_get_time().sec, Memb_id().time, Highest_seq );
#else
	fprintf( fd, "M %13ld %13d > ",
		E_get_time().sec, Memb_id_for_Network().time );
#endif
	found = -1;
	Cn = Conf_ref();
	for( i=0; i < Cn->num_segments; i++ )
	{
	    for( j=0; j < Cn->segments[i].num_procs; j++ )
	    {
		proc_id = Cn->segments[i].procs[j]->id;
		if( Conf_id_in_conf( Memb_active_ptr(), proc_id ) != -1 )
		{
		    if( found == -1 ) found = Conf_proc_by_id( proc_id, &dummy_p );
		    fprintf( fd, " %2d", found );
		}else fprintf( fd, " --" );
	    }
	}
			
	fprintf( fd, "\n");
	Log_alive(0, NULL);
}

void	Log_sess_connect( mailbox mbox, int32 address, char *private_group )
{
	char 	ip[16];

	if( !Is_inited ) return;

	Conf_id_to_str( address, ip );
#if ( SPREAD_PROTOCOL == 3 )
	fprintf( fd, "C %13ld %11d %3d %-16s %-10s\n", 
		E_get_time().sec, Highest_seq, mbox, ip, private_group );
#else
	fprintf( fd, "C %13ld %3d %-16s %-10s\n", 
		E_get_time().sec, mbox, ip, private_group );
#endif
	Log_alive(0, NULL);
}

void	Log_sess_disconnect( mailbox mbox, int32 address, char *private_group, int num_mess )
{
	char 	ip[16];

	if( !Is_inited ) return;

	Conf_id_to_str( address, ip );
#if ( SPREAD_PROTOCOL == 3 )
	fprintf( fd, "D %13ld %11d %3d %-16s %-10s %d\n", 
		E_get_time().sec, Highest_seq, mbox, ip, private_group, num_mess );
#else
	fprintf( fd, "D %13ld %3d %-16s %-10s %d\n", 
		E_get_time().sec, mbox, ip, private_group, num_mess );
#endif
	Log_alive(0, NULL);
}
