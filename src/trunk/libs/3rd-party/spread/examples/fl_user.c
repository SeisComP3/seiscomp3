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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fl.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

static	char	User[80];
static  char    Spread_name[80];
static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;
static	int	Num_sent;
static	int	Previous_len;
static  int     To_exit = 0;

static	void	Print_menu();
static	void	User_command();
static	void	Read_message();
static	void	Usage( int argc, char *argv[] );
static  void	Bye();

int main(int argc, char *argv[]) {
  int ret, major, minor, patch;

  Usage(argc, argv);

  FL_version(&major, &minor, &patch);
  printf("Flush library version is %d.%d.%d\n", major, minor, patch);

  FL_lib_init();

  ret = FL_connect( Spread_name, User, LOW_PRIORITY, &Mbox, Private_group );
  if( ret < 0 ) {
    FL_error( ret );
    Bye();
  }
  printf("User: connected to %s with private group %s\n", Spread_name, Private_group );
 
  Print_menu();

  printf("\nUser> ");
  fflush(stdout);

  Num_sent = 0;

  for(;;)
    User_command();
  
  return 0;
}

static	void	User_command()
{
  char	command[130];
  char	mess[102400];
  char	group[80];
  char	groups[10][MAX_GROUP_NAME];
  int	num_groups;
  int	mess_len;
  int	ret;
  int	i;

  for (i = 0; i < sizeof(command); i++) 
    command[i] = 0;

  if (fgets(command, 130, stdin) == 0) 
    Bye();

  switch(command[0]) {
  case 'j':
    ret = sscanf( &command[2], "%s", group );
    if( ret < 1 ) {
      printf(" invalid group \n");
      break;
    }
    ret = FL_join( Mbox, group );
    if( ret < 0 ) FL_error( ret );
    break;

    case 'l':
      ret = sscanf( &command[2], "%s", group );
      if( ret < 1 ) 
	{
	  printf(" invalid group \n");
	  break;
	}
      ret = FL_leave( Mbox, group );
      if( ret < 0 ) FL_error( ret );

      break;

  case 'g': {
    int i;

    if ((num_groups = sscanf(&command[2], "%s", groups[0])) < 1) {
      printf("invalid group\n");
      break;
    }
    printf("enter number of receivers: ");
    fflush(stdout);
    if (fgets(mess, 200, stdin) == 0 || 
	sscanf(mess, "%d", &num_groups) != 1 || 
	num_groups < 1 || num_groups > 9) {
      printf("invalid number of receivers, must be an integer in the range [1, 9]\n");
      break;
    }
    for (i = 1; i <= num_groups; ++i) {
      printf("enter recvr %d: ", i);
      fflush(stdout);
      if (fgets(mess, MAX_GROUP_NAME, stdin) == 0 || sscanf(mess, "%s", groups[i]) != 1) 
	Bye();
    }
    printf("enter message: ");
    fflush(stdout);
    if (fgets(mess, 200, stdin) == 0) Bye();
    mess_len = strlen(mess);

    printf("user.c: subgroupcasting message of size %d to group %s with receivers:\n", mess_len, groups[0]);
    for (i = 1; i <= num_groups; ++i)
      printf("%s\n", groups[i]);

    if ((ret = FL_subgroupcast(Mbox, SAFE_MESS, groups[0], num_groups, groups+1, 0, mess_len, mess)) != mess_len) {
      if (ret < 0) {
	FL_error(ret);
	Bye();
      } else
	printf("Wierd return from FL_subgroupcast %d, should be %d\n", ret, mess_len);
    }
    Num_sent++;
    break;
  }

    case 's':
      num_groups = sscanf(&command[2], "%s", groups[0]);
      if( num_groups < 1 ) 
	{
	  printf(" invalid group \n");
	  break;
	}
      printf("enter message: ");
	  if (fgets(mess, 200, stdin) == NULL)
		  Bye();
      mess_len = strlen( mess );

      printf("user.c : multicasting message of size %d:\n'%s'\n", mess_len, mess);

      ret = FL_multicast( Mbox, SAFE_MESS, groups[0], 1, mess_len, mess );
      if( ret < 0 ) 
	{
	  FL_error( ret );
	  Bye();
	}
      Num_sent++;
      break;

    case 'b':
      ret = sscanf( &command[2], "%s", group );
      if( ret != 1 ) strcpy( group, "dummy_group_name" );
      printf("enter size of each message: ");
	  if (fgets(mess, 200, stdin) == NULL)
		  Bye();
      ret = sscanf(mess, "%d", &mess_len );
      if( ret !=1 ) mess_len = Previous_len;
      if( mess_len < 0 ) mess_len = 0;
      Previous_len = mess_len;
      printf("sending 10 messages of %d bytes\n", mess_len );
      for( i=0; i<10; i++ )
	{
	  Num_sent++;
	  sprintf( mess, "mess num %d", Num_sent );
	  ret= FL_multicast( Mbox, FIFO_MESS, group, 2, mess_len, mess );

	  if( ret < 0 ) 
	    {
	      FL_error( ret );
	      Bye();
	    }
	  printf("sent message %d (total %d)\n", i+1, Num_sent );
	}
      break;
    case 'r':

      Read_message();
      break;

    case 'p':

      ret = FL_poll( Mbox );
      printf("Polling says: %d\n", ret );
      break;

      /*
	case 'e':

	E_attach_fd( Mbox, Read_message, 0, HIGH_PRIORITY );
	break;

	case 'd':

	E_detach_fd( Mbox );
	break;

      */
    case 'f':
      ret = sscanf( &command[2], "%s", group);
      if( ret < 1 ) 
	{
	  printf(" invalid group \n");
	  break;
	}
      ret = FL_flush(Mbox, group);

      printf("Sent a FLUSH_OK message to group '%s'\n", group);

      if (ret < 0) {
	FL_error(ret);
	Bye();
      }
      break;

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
  printf("\tg <group> -- send a subgroup message\n");
  printf("\tb <group> -- send a burst of messages\n");
  printf("\n");
  printf("\tr -- receive a message (stuck) \n");
  printf("\tp -- poll for a message \n");
  /*
    printf("\te -- enable asynchonous read (default)\n");
    printf("\td -- disable asynchronous read \n");
  */
  printf("\tf <group> -- send a FLUSH_OK message\n");
  printf("\n");
  printf("\tq -- quit\n");
  fflush(stdout);
}

static	void	Read_message()
{

  static	char		mess[102400];
  char		sender[MAX_GROUP_NAME];
  char		target_groups[100][MAX_GROUP_NAME];
  group_id	*grp_id;
  int32		*num_vs;
  char		*vs_members;
  int		num_groups;
  int		num_bytes;
  int		service_type;
  int16		mess_type;
  int		endian_mismatch;
  int		i;
  int		ret;
  int             more_messes;

  ret = FL_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
		    &mess_type, &endian_mismatch, sizeof(mess), mess, &more_messes);
  printf("\n============================\n");
  if( ret < 0 ) 
    {
      if( ! To_exit )
	{
	  FL_error( ret );
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
      printf("Message is:\n'%s'\n", mess);
      if (Is_subgroup_mess(service_type)) {
	int i;

	printf("Received a subgroupcast message to group %s: receivers:\n", target_groups[num_groups - 1]);
	for (i = 0; i < num_groups - 1; ++i)
	  printf("\t%s\n", target_groups[i]);
      }
    } else if( Is_membership_mess( service_type ) ){
      if     ( Is_reg_memb_mess( service_type ) )
	{
	  num_bytes = 0;
	  grp_id = (group_id *)&mess[num_bytes];
	  num_bytes += sizeof( group_id );
	  num_vs = (int32 *)&mess[num_bytes];
	  num_bytes += sizeof( int32 );
	  vs_members = &mess[num_bytes];

	  printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n",
		 sender, num_groups, mess_type );

	  for( i=0; i < num_groups; i++ )
	    printf("\t%s\n", &target_groups[i][0]);

	  printf("grp id is %d %d %d\n", grp_id->id[0], grp_id->id[1], grp_id->id[2]);

	  if( Is_caused_join_mess( service_type ) )
	    {
	      printf("Due to the JOIN of %s\n", vs_members );
	    }else if( Is_caused_leave_mess( service_type ) ){
	      printf("Due to the LEAVE of %s\n", vs_members);
	    }else if( Is_caused_disconnect_mess( service_type ) ){
	      printf("Due to the DISCONNECT of %s\n", vs_members );
	    }else if( Is_caused_network_mess( service_type ) ){
	      printf("Due to NETWORK change. ");
	      printf("VS set has %d members:\n", *num_vs );
	      for( i=0; i < *num_vs; i++, vs_members+= MAX_GROUP_NAME )
		printf("\t%s\n", vs_members );
	    }
	}else if( Is_transition_mess(   service_type ) ) {
	  printf("received TRANSITIONAL membership for group %s\n", sender );
	}else if( Is_caused_leave_mess( service_type ) ){
	  printf("received membership message that left group %s\n", sender );
	}else printf("received incorrecty membership message of type %d\n", service_type );
    }else if (Is_flush_req_mess(service_type)) {
      printf("received a FLUSH_REQ message for group %s\n", sender);
    }else printf("received message of unknown message type %d with ret %d\n", service_type, ret);

  printf("There are %d buffered messages waiting to be delivered\n", more_messes);

  printf("\n");
  printf("User> ");
  fflush(stdout);
}

static	void	Usage(int argc, char *argv[])
{

  sprintf( User, "user" );
  sprintf( Spread_name, "4803");
  while( --argc > 0 )
    {
      argv++;

      if( !strncmp( *argv, "-u", 2 ) )
	{
	  strcpy( User, argv[1] );
	  argc--; argv++;
	}else if( !strncmp( *argv, "-s", 2 ) ){
	  strcpy( Spread_name, argv[1] ); 
	  argc--; argv++;
	}else{
	  printf( "Usage: user\n%s\n%s\n",
		  "\t[-u <user name>]  : unique (in this machine) user name",
		  "\t[-s <address>]    : either port or port@machine");
	  exit( 0 );
	}
    }
}

static  void	Bye()
{
  To_exit = 1;

  printf("\nBye.\n");

  printf("Calling FL_disconnect(mbox = %d)\n", Mbox);

  FL_disconnect( Mbox );
  exit( 0 );
}





