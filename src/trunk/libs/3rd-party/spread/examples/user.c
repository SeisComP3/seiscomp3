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



#include "sp.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define int32u unsigned int

#ifdef _REENTRANT

#ifndef 	ARCH_PC_WIN95
#include        <sys/types.h>
#include        <sys/socket.h>
#include 	<pthread.h>
#else		/* ARCH_PC_WIN95 */
#include	<windows.h>
#endif		/* ARCH_PC_WIN95 */

#endif /* _REENTRANT */

#ifdef ENABLE_PASSWORD
#include "auth-pword-client.h"
#endif

static	char	User[80];
static  char    Spread_name[80];

/* password variables always defined. Only have effect if ENABLE_PASSWORD is defined */
static  int     Use_Pword;
static  char    Pword_username[33];
static  char    Pword_password[9];

#ifdef ENABLE_PASSWORD
/* variables for Password authentication */
static  struct user_password Pword_user;
#endif

static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;
static	int	Num_sent;
static	unsigned int	Previous_len;

static  int     To_exit = 0;

#define MAX_MESSLEN     102400
#define MAX_VSSETS      10
#define MAX_MEMBERS     100

#ifdef	_REENTRANT

#ifndef 	ARCH_PC_WIN95
	static  pthread_t	Read_pthread;
	static  void    *Read_thread_routine();
#else		/* ARCH_PC_WIN95 */
	static	HANDLE		Read_pthread;
	static	DWORD WINAPI    Read_thread_routine( void *);
#endif		/* ARCH_PC_WIN95 */


#endif /* _REENTRANT */

static	void	Print_menu();
static	void	User_command();
static	void	Read_message();
static	void	Usage( int argc, char *argv[] );
static  void    Print_help();
static  void	Bye();

int main( int argc, char *argv[] )
{
	int	ret;
#ifdef SPREAD_VERSION
        int     mver, miver, pver;
#endif
        sp_time test_timeout;

        test_timeout.sec = 5;
        test_timeout.usec = 0;

	Usage( argc, argv );
#ifdef SPREAD_VERSION
        if (!SP_version( &mver, &miver, &pver)) 
        {
	  printf("main: Illegal variables passed to SP_version()\n");
	  Bye();
	}
	printf("Spread library version is %d.%d.%d\n", mver, miver, pver);
#else
        printf("Spread library version is %1.2f\n", SP_version() );
#endif
#ifdef  ENABLE_PASSWORD
        if (Use_Pword)
        {
                strncpy(Pword_user.username, Pword_username, 32);
                Pword_user.username[32] = '\0';
                strncpy(Pword_user.password, Pword_password, 8);
                Pword_user.password[8] = '\0';
                SP_set_auth_method("PWORD", pword_authenticate, &Pword_user);
        } 
#endif
	ret = SP_connect_timeout( Spread_name, User, 0, 1, &Mbox, Private_group, test_timeout );
	if( ret != ACCEPT_SESSION ) 
	{
		SP_error( ret );
		Bye();
	}
	printf("User: connected to %s with private group %s\n", Spread_name, Private_group );

#ifndef	_REENTRANT
	E_init();
	E_attach_fd( 0, READ_FD, User_command, 0, NULL, LOW_PRIORITY );

	E_attach_fd( Mbox, READ_FD, Read_message, 0, NULL, HIGH_PRIORITY );
#endif	/* _REENTRANT */

	Print_menu();

	printf("\nUser> ");
	fflush(stdout);

	Num_sent = 0;

#ifdef	_REENTRANT

#ifndef		ARCH_PC_WIN95
		ret = pthread_create( &Read_pthread, NULL, Read_thread_routine, 0 );
#else		/* ARCH_PC_WIN95 */
		Read_pthread = CreateThread( NULL, 0, Read_thread_routine, NULL, 0, &ret );
#endif		/* ARCH_PC_WIN95 */

	for(;;)
	{
		User_command();
#ifdef __bsdi__
		/* bug in BSDI */
		sched_yield();
#endif
	}

#else	/* _REENTRANT */

#ifndef		ARCH_PC_WIN95
		E_handle_events();
#else		/* ARCH_PC_WIN95 */
		for(;;)
			User_command();
#endif		/* ARCH_PC_WIN95 */

#endif	/* _REENTRANT */

	return( 0 );
}

#ifdef	_REENTRANT

#ifndef 	ARCH_PC_WIN95
static	void	*Read_thread_routine()
#else		/* ARCH_PC_WIN95 */
static	DWORD WINAPI    Read_thread_routine( void *dummy)
#endif		/* ARCH_PC_WIN95 */
{
	for(;;)
	{
		Read_message();
	}
	return( 0 );
}

#endif	/* _REENTRANT */

static	void	User_command()
{
	char	command[130];
	char	mess[MAX_MESSLEN];
	char	group[80];
	char	groups[10][MAX_GROUP_NAME];
	int	num_groups;
	unsigned int	mess_len;
	int	ret;
	int	i;

	for( i=0; i < sizeof(command); i++ ) command[i] = 0;
	if( fgets( command, 130, stdin ) == NULL ) 
            Bye();

	switch( command[0] )
	{
		case 'j':
			ret = sscanf( &command[2], "%s", group );
			if( ret < 1 ) 
			{
				printf(" invalid group \n");
				break;
			}
			ret = SP_join( Mbox, group );
			if( ret < 0 ) SP_error( ret );

			break;

		case 'l':
			ret = sscanf( &command[2], "%s", group );
			if( ret < 1 ) 
			{
				printf(" invalid group \n");
				break;
			}
			ret = SP_leave( Mbox, group );
			if( ret < 0 ) SP_error( ret );

			break;

		case 's':
			num_groups = sscanf(&command[2], "%s%s%s%s%s%s%s%s%s%s", 
						groups[0], groups[1], groups[2], groups[3], groups[4],
						groups[5], groups[6], groups[7], groups[8], groups[9] );
			if( num_groups < 1 ) 
			{
				printf(" invalid group \n");
				break;
			}
			printf("enter message: ");
			if (fgets(mess, 200, stdin) == NULL)
				Bye();
			mess_len = strlen( mess );
#ifdef _REENTRANT
#ifdef __bsdi__		/* bsdi bug - doing a close when another thread blocks on the socket causes a seg fault */
	ret = send( Mbox, mess, 0, 0 );
	if( ret < 0 )
	{
		SP_error( CONNECTION_CLOSED );
		Bye();
	}
#endif /* __bsdi__ */
#endif /* _REENTRANT */
			ret= SP_multigroup_multicast( Mbox, SAFE_MESS, num_groups, (const char (*)[MAX_GROUP_NAME]) groups, 1, mess_len, mess );
			if( ret < 0 ) 
			{
				SP_error( ret );
				Bye();
			}
			Num_sent++;

			break;

		case 'm':
			num_groups = sscanf(&command[2], "%s%s%s%s%s%s%s%s%s%s", 
						groups[0], groups[1], groups[2], groups[3], groups[4],
						groups[5], groups[6], groups[7], groups[8], groups[9] );
			if( num_groups < 1 ) 
			{
				printf(" invalid group \n");
				break;
			}
			printf("enter message: ");
                        mess_len = 0;
                        while ( mess_len < MAX_MESSLEN) {
                            if (fgets(&mess[mess_len], 200, stdin) == NULL)
				Bye();
                            if (mess[mess_len] == '\n')
                                break;
                            mess_len += strlen( &mess[mess_len] );
                        }
#ifdef _REENTRANT
#ifdef __bsdi__		/* bsdi bug - doing a close when another thread blocks on the socket causes a seg fault */
	ret = send( Mbox, mess, 0, 0 );
	if( ret < 0 )
	{
		SP_error( CONNECTION_CLOSED );
		Bye();
	}
#endif /* __bsdi__ */
#endif /* _REENTRANT */
			ret= SP_multigroup_multicast( Mbox, SAFE_MESS, num_groups, (const char (*)[MAX_GROUP_NAME]) groups, 1, mess_len, mess );
			if( ret < 0 ) 
			{
				SP_error( ret );
				Bye();
			}
			Num_sent++;

			break;

		case 'b':
			ret=sscanf( &command[2], "%s", group );
			if( ret != 1 ) strcpy( group, "dummy_group_name" );
			printf("enter size of each message: ");
			if (fgets(mess, 200, stdin) == NULL)
				Bye();
			ret=sscanf(mess, "%u", &mess_len );
			if( ret !=1 ) mess_len = Previous_len;
                        if( mess_len > MAX_MESSLEN ) mess_len = MAX_MESSLEN;
			Previous_len = mess_len;
			printf("sending 10 messages of %u bytes\n", mess_len );
			for( i=0; i<10; i++ )
			{
				Num_sent++;
				sprintf( mess, "mess num %d ", Num_sent );
#ifdef _REENTRANT
#ifdef __bsdi__		/* bsdi bug - doing a close when another thread blocks on the socket causes a seg fault */
	ret = send( Mbox, mess, 0,0 );
	if( ret < 0 )
	{
		SP_error( CONNECTION_CLOSED );
		Bye();
	}
#endif /* __bsdi__ */
#endif /* _REENTRANT */
				ret= SP_multicast( Mbox, FIFO_MESS, group, 2, mess_len, mess );

				if( ret < 0 ) 
				{
					SP_error( ret );
					Bye();
				}
				printf("sent message %d (total %d)\n", i+1, Num_sent );
			}
			break;
#ifndef _REENTRANT
		case 'r':

			Read_message();
			break;

		case 'p':

			ret = SP_poll( Mbox );
			printf("Polling sais: %d\n", ret );
			break;

		case 'e':

			E_attach_fd( Mbox, READ_FD, Read_message, 0, NULL, HIGH_PRIORITY );

			break;

		case 'd':

			E_detach_fd( Mbox, READ_FD );

			break;
#endif	/* _REENTRANT */
		case 'q':
			Bye();
			break;

		default:
			printf("\nUnknown commnad\n");
			Print_menu();

			break;
	}
	printf("\nUser> ");
	fflush(stdout);

}

static	void	Print_menu()
{
	printf("\n");
	printf("==========\n");
	printf("User Menu:\n");
	printf("----------\n");
	printf("\n");
	printf("\tj <group> -- join a group\n");
	printf("\tl <group> -- leave a group\n");
	printf("\n");
	printf("\ts <group> -- send a message\n");
        printf("\tm <group> -- send a multiline message to group. Terminate with empty line\n");
	printf("\tb <group> -- send a burst of messages\n");
	printf("\n");
#ifndef	_REENTRANT
	printf("\tr -- receive a message (stuck) \n");
	printf("\tp -- poll for a message \n");
	printf("\te -- enable asynchonous read (default)\n");
	printf("\td -- disable asynchronous read \n");
	printf("\n");
#endif	/* _REENTRANT */
	printf("\tq -- quit\n");
	fflush(stdout);
}

/* FIXME: The user.c code does not use memcpy()s to avoid bus errors when
 *        dereferencing a pointer into a potentially misaligned buffer */

static	void	Read_message()
{

static	char		 mess[MAX_MESSLEN];
	char		 sender[MAX_GROUP_NAME];
	char		 target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
        membership_info  memb_info;
        vs_set_info      vssets[MAX_VSSETS];
        unsigned int     my_vsset_index;
        int              num_vs_sets;
        char             members[MAX_MEMBERS][MAX_GROUP_NAME];
	int		 num_groups;
	int		 service_type;
	int16		 mess_type;
	int		 endian_mismatch;
	int		 i,j;
	int		 ret;

        service_type = 0;

	ret = SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
		&mess_type, &endian_mismatch, sizeof(mess), mess );
	printf("\n============================\n");
	if( ret < 0 ) 
	{
                if ( (ret == GROUPS_TOO_SHORT) || (ret == BUFFER_TOO_SHORT) ) {
                        service_type = DROP_RECV;
                        printf("\n========Buffers or Groups too Short=======\n");
                        ret = SP_receive( Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups, 
                                          &mess_type, &endian_mismatch, sizeof(mess), mess );
                }
        }
        if (ret < 0 )
        {
		if( ! To_exit )
		{
			SP_error( ret );
			printf("\n============================\n");
			printf("\nBye.\n");
		}
		exit( 0 );
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
		printf("message from %s, of type %d, (endian %d) to %d groups \n(%d bytes): %s\n",
			sender, mess_type, endian_mismatch, num_groups, ret, mess );
	}else if( Is_membership_mess( service_type ) )
        {
                ret = SP_get_memb_info( mess, service_type, &memb_info );
                if (ret < 0) {
                        printf("BUG: membership message does not have valid body\n");
                        SP_error( ret );
                        exit( 1 );
                }
		if     ( Is_reg_memb_mess( service_type ) )
		{
			printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n",
				sender, num_groups, mess_type );
			for( i=0; i < num_groups; i++ )
				printf("\t%s\n", &target_groups[i][0] );
			printf("grp id is %d %d %d\n",memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2] );

			if( Is_caused_join_mess( service_type ) )
			{
				printf("Due to the JOIN of %s\n", memb_info.changed_member );
			}else if( Is_caused_leave_mess( service_type ) ){
				printf("Due to the LEAVE of %s\n", memb_info.changed_member );
			}else if( Is_caused_disconnect_mess( service_type ) ){
				printf("Due to the DISCONNECT of %s\n", memb_info.changed_member );
			}else if( Is_caused_network_mess( service_type ) ){
				printf("Due to NETWORK change with %u VS sets\n", memb_info.num_vs_sets);
                                num_vs_sets = SP_get_vs_sets_info( mess, &vssets[0], MAX_VSSETS, &my_vsset_index );
                                if (num_vs_sets < 0) {
                                        printf("BUG: membership message has more then %d vs sets. Recompile with larger MAX_VSSETS\n", MAX_VSSETS);
                                        SP_error( num_vs_sets );
                                        exit( 1 );
                                }
                                for( i = 0; i < num_vs_sets; i++ )
                                {
                                        printf("%s VS set %d has %u members:\n",
                                               (i  == my_vsset_index) ?
                                               ("LOCAL") : ("OTHER"), i, vssets[i].num_members );
                                        ret = SP_get_vs_set_members(mess, &vssets[i], members, MAX_MEMBERS);
                                        if (ret < 0) {
                                                printf("VS Set has more then %d members. Recompile with larger MAX_MEMBERS\n", MAX_MEMBERS);
                                                SP_error( ret );
                                                exit( 1 );
                                        }
                                        for( j = 0; j < (int) vssets[i].num_members; j++ )
                                                printf("\t%s\n", members[j] );
                                }
			}
		}else if( Is_transition_mess(   service_type ) ) {
			printf("received TRANSITIONAL membership for group %s\n", sender );
		}else if( Is_caused_leave_mess( service_type ) ){
			printf("received membership message that left group %s\n", sender );
		}else printf("received incorrecty membership message of type 0x%x\n", service_type );
        } else if ( Is_reject_mess( service_type ) )
        {
		printf("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
			sender, service_type, mess_type, endian_mismatch, num_groups, ret, mess );
	}else printf("received message of unknown message type 0x%x with ret %d\n", service_type, ret);


	printf("\n");
	printf("User> ");
	fflush(stdout);
}

static	void	Usage(int argc, char *argv[])
{
        Use_Pword = 0;
	sprintf( User, "user" );
	sprintf( Spread_name, "4803");
	while( --argc > 0 )
	{
		argv++;

		if( !strncmp( *argv, "-u", 2 ) )
		{
                        if (argc < 2) Print_help();
                        strcpy( User, argv[1] );
                        argc--; argv++;
		}else if( !strncmp( *argv, "-r", 2 ) )
		{
			strcpy( User, "" );
		}else if( !strncmp( *argv, "-s", 2 ) ){
                        if (argc < 2) Print_help();
			strcpy( Spread_name, argv[1] ); 
			argc--; argv++;
		}else if( !strncmp( *argv, "-n", 2 ) ){
                        if (argc < 2) Print_help();
			strcpy( Pword_username, argv[1] ); 
                        Use_Pword = 1;
			argc--; argv++;
		}else if( !strncmp( *argv, "-p", 2 ) ){
                        if (argc < 2) Print_help();
			strcpy( Pword_password, argv[1] ); 
                        Use_Pword = 1;
			argc--; argv++;
		}else{
                    Print_help();
                }
	 }
}
static  void    Print_help()
{
    printf( "Usage: spuser\n%s\n%s\n%s\n%s\n%s\n",
            "\t[-u <user name>]  : unique (in this machine) user name",
            "\t[-s <address>]    : either port or port@machine",
            "\t[-n <username>]    : username for authentication",
            "\t[-p <password>]    : users password",
            "\t[-r ]    : use random user name");
    exit( 0 );
}
static  void	Bye()
{
	To_exit = 1;

	printf("\nBye.\n");

#ifdef	_REENTRANT
#ifdef 		__bsdi__
	/* bug in threaded bsdi that cores in SP_disconnect when closing mbox while another thread is reading*/
	exit( 0 );
#endif		/* __bsdi__ */
#endif	/* _REENTRANT */

	SP_disconnect( Mbox );
#ifdef	_REENTRANT
#ifndef		ARCH_PC_WIN95
		pthread_join( Read_pthread, NULL );
#else		/* ARCH_PC_WIN95 */

#endif		/* ARCH_PC_WIN95 */
#endif	/* _REENTRANT */
	exit( 0 );
}
