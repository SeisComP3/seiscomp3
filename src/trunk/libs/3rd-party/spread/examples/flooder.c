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
#include <stdlib.h>
#include <string.h>

/* Not positive if portable JRS 5/2004 */
#include <limits.h>

#include "sp.h"

#define	MAX_MESSLEN	100000
static	char            mess[MAX_MESSLEN] ;
static	char            recv_mess[MAX_MESSLEN] ;

#define FLOODER_MAX_GROUPS      100
static	char	User[80];
static	char	Spread_name[80];
static	char	Private_group[MAX_GROUP_NAME];
static	mailbox	Mbox;
static	int	Num_bytes;
static	int	Num_messages;
static	int	Num_members;
static	int	Read_only, Write_only;

static  char	ret_groups[FLOODER_MAX_GROUPS][MAX_GROUP_NAME];

static  int     Send_Counter;
static  int     Recv_Counters[FLOODER_MAX_GROUPS];
static  int     Lowest_Recv_Counter;
static  int     My_Counter_index;

static	void	Usage( int argc, char *argv[] );
static  void    Print_help();

int main( int argc, char *argv[] )
{
	int		ret;
	int		service_type, num_groups;
	char		sender[MAX_GROUP_NAME];
	int16		mess_type;
	int		dummy_endian_mismatch;
        int             joined_members;
        int             sender_index;
	int		i,j;

	Usage( argc, argv );

        if (Num_members == 0) {
            /* connecting to the relevant Spread daemon, no need for group info */
            printf("flooder: connecting to %s\n", Spread_name );
            ret = SP_connect( Spread_name, User, 0, 0, &Mbox, Private_group ) ;
        } else {
            /* connecting to the relevant Spread daemon, DO need group info */
            printf("flooder: connecting to %s with group membership\n", Spread_name );
            ret = SP_connect( Spread_name, User, 0, 1, &Mbox, Private_group ) ;
        }
        if(ret < 0) 
	{
		SP_error( ret );
                exit(1) ;
        }
	/* 
	 * Joining the process group.
	 *
	 * Note that this is not necessary in order to multicast the
	 * messages, but only to demonstrate end-to-end behaviour.
	 */
	if( Read_only )
	{
		printf("flooder: Only receiving messages\n");
		SP_join( Mbox, "flooder" );
	}else if( Write_only ) {
		printf("flooder: starting  multicast of %d messages, %d bytes each (self discarding).\n", Num_messages, Num_bytes);
	}else{
		SP_join( Mbox, "flooder" );
		printf("flooder: starting  multicast of %d messages, %d bytes each.\n", Num_messages, Num_bytes);
	}

        /* Wait for all members to join */
        joined_members = 0;
        while( joined_members < Num_members)
        {
            service_type = 0;
            ret = SP_receive( Mbox, &service_type, sender, FLOODER_MAX_GROUPS, &num_groups, ret_groups, 
                              &mess_type, &dummy_endian_mismatch, sizeof(recv_mess), recv_mess );
            if( ret < 0 ) 
            {
                if ( (ret == GROUPS_TOO_SHORT) || (ret == BUFFER_TOO_SHORT) ) {
                    printf("\n========Buffers or Groups too Short=======\n");
                    printf("Should NOT happen in wait for members! Program quitting\n");
                    exit(1);
                }       
            }

            if( Is_regular_mess( service_type ) )
            {
		mess[ret] = 0;
		if     ( Is_unreliable_mess( service_type ) ) printf("received UNRELIABLE ");
		else if( Is_reliable_mess(   service_type ) ) printf("received RELIABLE ");
		else if( Is_fifo_mess(       service_type ) ) printf("received FIFO ");
		else if( Is_causal_mess(     service_type ) ) printf("received CAUSAL ");
		else if( Is_agreed_mess(     service_type ) ) printf("received AGREED ");
		else if( Is_safe_mess(       service_type ) ) printf("received SAFE ");
		printf("message during wait for members, from %s, of type %d, (endian %d) to %d groups \n(%d bytes): %s\n",
			sender, mess_type, dummy_endian_mismatch, num_groups, ret, recv_mess );
            }else if( Is_membership_mess( service_type ) )
            {
		if     ( Is_reg_memb_mess( service_type ) )
		{
			printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n",
				sender, num_groups, mess_type );
			for( i=0; i < num_groups; i++ )
				printf("\t%s\n", &ret_groups[i][0] );
                        /* update count of joined members */
                        joined_members = num_groups;

		}else if( Is_transition_mess(   service_type ) ) {
			printf("received TRANSITIONAL membership for group %s\n", sender );
		}else if( Is_caused_leave_mess( service_type ) ){
			printf("received membership message that left group %s\n", sender );
		}else printf("received incorrecty membership message of type 0x%x\n", service_type );
            } else if ( Is_reject_mess( service_type ) )
            {
		printf("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
			sender, service_type, mess_type, dummy_endian_mismatch, num_groups, ret, recv_mess );
            }else printf("received message of unknown message type 0x%x with ret %d\n", service_type, ret);
           
        } /* joined_members < Num_members */

        /* Update My_Counter_index field based on location of my name in last membership message */
        if (Num_members)
        {
            My_Counter_index = mess_type;
            memcpy(&mess[0], &My_Counter_index, sizeof(int));
        }

	for( i=1; i <= Num_messages; i++ )
	{
		/* multicast a message unless Read_only */
		if( !Read_only )
		{
                    if (Num_members) 
                    {
                        ret = SP_multicast( Mbox, FIFO_MESS, "flooder", 0, Num_bytes, mess );
                        Send_Counter++;
                    } else {
                        ret = SP_multicast( Mbox, RELIABLE_MESS, "flooder", 0, Num_bytes, mess );
                    }
		    if( ret != Num_bytes ) 
		    {
			if( ret < 0 )
			{
				SP_error( ret );
				exit(1);
			}
			printf("sent a different message %d -> %d\n", Num_bytes, ret );
		    }
		}

		/* receive a message (Read_only) or one of my messages */
		if( Read_only || ( i > 200 && !Write_only ) )
		{
                    int notdone;

		    do{
                        service_type = 0;

			ret = SP_receive( Mbox, &service_type, sender, FLOODER_MAX_GROUPS, &num_groups, ret_groups, 
					  &mess_type, &dummy_endian_mismatch, sizeof(recv_mess), recv_mess );
                        if( ret < 0 ) 
                        {
                                if ( (ret == GROUPS_TOO_SHORT) || (ret == BUFFER_TOO_SHORT) ) {
                                        service_type = DROP_RECV;
                                        printf("\n========Buffers or Groups too Short=======\n");
                                        ret = SP_receive( Mbox, &service_type, sender, FLOODER_MAX_GROUPS, &num_groups, ret_groups, 
                                                          &mess_type, &dummy_endian_mismatch, sizeof(recv_mess), recv_mess );
                                }
                        }
			if( ret < 0 )
			{
				SP_error( ret );
				exit(1);
			}
                        if (Num_members) {
                            /* update counters of received messages */
                            memcpy(&sender_index, &recv_mess[0], sizeof(int));
                            Recv_Counters[sender_index]++;
                            /* 
                               printf("DEBUG: updated counter %d to value %d\n", sender_index, Recv_Counters[sender_index]);
                            */
                            if (Recv_Counters[sender_index] == (Lowest_Recv_Counter + 1)) {
                                /* Update Lowest_Recv_Counter */
                                Lowest_Recv_Counter = INT_MAX;
                                for (j=0; j < Num_members; j++) {
                                    if (Recv_Counters[j] == 0) continue;
                                    if (Recv_Counters[j] < Lowest_Recv_Counter)
                                        Lowest_Recv_Counter = Recv_Counters[j];
                                }
                                if (Lowest_Recv_Counter == INT_MAX) 
                                    Lowest_Recv_Counter = 0;
                            }
                            /* Read loop is done if we send messages and we have received all 
                             * other senders messages upto 100 less then our current send count 
                             */
                            notdone = ( Lowest_Recv_Counter < (Send_Counter - 200) && !Read_only );
                        } else {
                            notdone = (strcmp( sender, Private_group ) != 0 && !Read_only);
                        }

		    } while( notdone );
		}

		/* report some progress... */
		if( i%1000 == 0 ) printf("flooder: completed %6d messages of %d bytes\n",i, ret);
	}
	printf("flooder: completed multicast of %d messages, %d bytes each.\n", Num_messages, Num_bytes);

	return 0;
}

static  void    Usage(int argc, char *argv[])
{

	/* Setting defaults */
        sprintf( User, "flooder" );
        sprintf( Spread_name, "4803");
	Num_bytes    =  1000;
	Num_messages = 10000;
        Num_members = 0;
	Read_only    = 0;
	Write_only   = 0;

        while( --argc > 0 )
        {
                argv++;

                if( !strncmp( *argv, "-u", 2 ) )
                {
                        if (argc < 2) Print_help();
                        strcpy( User, argv[1] );
                        argc--; argv++;
                }else if( !strncmp( *argv, "-b", 2 ) ){
                        if (argc < 2) Print_help();
			sscanf(argv[1], "%d", &Num_bytes );
                        argc--; argv++;
                }else if( !strncmp( *argv, "-m", 2 ) ){
                        if (argc < 2) Print_help();
			sscanf(argv[1], "%d", &Num_messages );
                        argc--; argv++;
                }else if( !strncmp( *argv, "-n", 2 ) ){
                        if (argc < 2) Print_help();
			sscanf(argv[1], "%d", &Num_members );
                        argc--; argv++;
                }else if( !strncmp( *argv, "-s", 2 ) ){
                        if (argc < 2) Print_help();
                        strcpy( Spread_name, argv[1] ); 
                        argc--; argv++;
                }else if( !strncmp( *argv, "-ro", 3 ) ){
				Read_only  = 1;
				Write_only = 0;
                }else if( !strncmp( *argv, "-wo", 3 ) ){
				Write_only = 1;
				Read_only  = 0;
                }else{
                    Print_help();
                }
	}

        if (Num_members > FLOODER_MAX_GROUPS)
        {
            printf("Too many members. The max is %d\n", FLOODER_MAX_GROUPS);
            Print_help();
        }
}
static  void    Print_help()
{
    printf( "Usage: spflooder\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
            "\t[-u <user name>]     : unique (in this machine) user name",
            "\t[-m <num messages>]  : number of messages",
            "\t[-n <num members>]   : number of group members to wait for (also turn on multi-sender FC)",
            "\t[-b <num bytes>]     : number of bytes per message 1-100,000",
            "\t[-s <spread name>]   : either port or port@machine",
            "\t[-ro]   		: read  only (no multicast)",
            "\t[-wo]   		: write only (no receive)");
    exit(1);
}
