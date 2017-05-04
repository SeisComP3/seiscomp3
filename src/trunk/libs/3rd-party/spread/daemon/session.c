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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef ARCH_PC_WIN95

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/ioctl.h>

#else   /* ARCH_PC_WIN95 */

#include <winsock.h>
#define	ioctl 	ioctlsocket

#endif  /* ARCH_PC_WIN95 */

#include "spread_params.h"
#include "net_types.h"
#include "spu_events.h"
#include "spu_objects.h"
#include "spu_memory.h"
#include "session.h"
#include "sess_types.h"

#define ext_sess_body 
#include "sess_body.h"
#undef  ext_sess_body

#include "groups.h"
#include "log.h"
#include "status.h"
#include "spu_alarm.h"
#if     ( SPREAD_PROTOCOL > 3 )
#include "queues.h"
#endif
#include "message.h"
#include "acm.h"

static	sp_time		Badger_timeout = { 0, 100000 };

static	message_obj	New_mess;

static  int             Accept_inet_mbox_num;
static	mailbox		Accept_inet_mbox[MAX_INTERFACES_PROC];
static	mailbox		Accept_unix_mbox;

static	int		Protocol_threshold;

#define SESSION_FD_HASH_SIZE    256
static	session		*Sessions_hash_head[SESSION_FD_HASH_SIZE];
static	session		*Sessions_head;
static	session		*Sessions_tail;
static	session		*Sessions_free;

static	void	Sess_attach_accept(void);
static	void	Sess_detach_accept(void);
static  void    Sess_recv_client_auth(mailbox mbox, int dummy, void *dummy_p);
static	void    Sess_accept( mailbox mbox, int domain, void *dummy );
static	void	Sess_accept_continue( mailbox, int, void * );
static	void    Sess_read( mailbox mbox, int domain, void *dummy );
static	void	Sess_badger( mailbox mbox );
static	void	Sess_badger_TO( mailbox mbox, void *dummy );
static  void    Sess_badger_FD( mailbox mbox, int dmy, void *dmy2 );
static	void	Sess_kill( mailbox mbox );
static	void	Sess_handle_join( message_link *mess_link );
static	void	Sess_handle_leave( message_link *mess_link );
static	void	Sess_handle_kill( message_link *mess_link );
static  void    Sess_deliver_reject( message_obj *msg );
static  void    Sess_create_reject_message ( message_obj *msg );
static  int     Sess_get_p2p_dests( int num_groups, char groups[][MAX_GROUP_NAME], char dests[][MAX_GROUP_NAME] );

static  void    Sess_activate_port_reuse(mailbox mbox)
{
    int on = 1; 
    if (setsockopt(mbox, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on)) < 0) 
        Alarm( EXIT, "Sess_activate_port_reuse: From Sess_init: Error setting SO_REUSEADDR socket option\n" ); 
}

int	Sess_get_session_index (int mbox)
{
    session *tmp;
    unsigned char *c = (unsigned char *) &mbox;
    unsigned int i;

    i = c[0] ^ c[1] ^ c[2] ^ c[3];

    Alarm( NONE, "Sess_get_session_index: mbox %d hashed to %u\n", mbox, i);
    for (tmp = Sessions_hash_head[i]; tmp; tmp = tmp->hash_next)
        if (tmp->mbox == mbox)
            return (tmp - Sessions);

    return -1;
}

static  void    Sess_hash_session (session *ses)
{
    unsigned int i;
    unsigned char *c;
    
    c = (unsigned char *) &ses->mbox;
    i = c[0] ^ c[1] ^ c[2] ^ c[3];
    ses->hash_next = Sessions_hash_head[i];
    Sessions_hash_head[i] = ses;
}

static  void    Sess_unhash_session (session *ses)
{
    unsigned int i;
    unsigned char *c;
    session *tmp;
    
    c = (unsigned char *) &ses->mbox;
    i = c[0] ^ c[1] ^ c[2] ^ c[3];
    tmp = Sessions_hash_head[i];
    if (tmp == ses)
    {
        Sessions_hash_head[i] = ses->hash_next;
        ses->hash_next = NULL;
        return;
    }

    for ( ; tmp->hash_next != ses; tmp = tmp->hash_next);
    tmp->hash_next = ses->hash_next;
    ses->hash_next = NULL;
}

static	session	*Sess_get_free_session (void)
{
    session *ses;

    if ((ses = Sessions_free) == NULL)
    {
        Alarm (EXIT, "Sess_get_free_session: BUG ! No free sessions !\n");
    }

    Sessions_free = Sessions_free->sort_next;

    return ses;
}

static	void	Sess_free_session (session *ses)
{
    ses->sort_next = Sessions_free;
    Sessions_free = ses;
}

static	int	Sess_insert_new_session (session *where, session *template)
{
    session *new_ses;

    new_ses = Sess_get_free_session();
    memmove(new_ses, template, sizeof (*template));
  
    if (!where)
    {
        /* Ok, we insert a session at the end of the list... */
        new_ses->sort_next = NULL;
    
        if (!Sessions_tail)
        {
            /* List is empty */
            new_ses->sort_prev = NULL;
            Sessions_head = Sessions_tail = new_ses;
        }
        else
        {
            new_ses->sort_prev = Sessions_tail;
            Sessions_tail->sort_next = new_ses;
            Sessions_tail = new_ses;
        }
    }
    else
    {
        /* Ok, we insert a session in the middle of the list, just
         * before where... */
        new_ses->sort_next = where;
        new_ses->sort_prev = where->sort_prev;
        where->sort_prev = new_ses;
    
        if (!new_ses->sort_prev)
        {
            /* new_ses is new head */
            Sessions_head = new_ses;
        }
        else
        {
            new_ses->sort_prev->sort_next = new_ses;
        }
    }

    return(new_ses - Sessions);
}

static	void	Sess_remove_session (session *ses)
{
    if (!ses->sort_prev && !ses->sort_next)
    {
        /* Last session */
        Sessions_head = Sessions_tail = NULL;
        Sess_free_session(ses);
    
        return;
    }

    if (!ses->sort_prev)
    {
        /* Head */
        Sessions_head = ses->sort_next;
        ses->sort_next->sort_prev = NULL;
        Sess_free_session(ses);
    
        return;
    }

    if (!ses->sort_next)
    {
        /* Tail */
        Sessions_tail = ses->sort_prev;
        ses->sort_prev->sort_next = NULL;
        Sess_free_session(ses);
    
        return;
    }

    /* All troubled cases are above ;-) */
    ses->sort_next->sort_prev = ses->sort_prev;
    ses->sort_prev->sort_next = ses->sort_next;
    Sess_free_session(ses);
}

static	void	Sess_init_sessions (void)
{
    int i;

    for (i = 0; i < SESSION_FD_HASH_SIZE; i++)
        Sessions_hash_head[i] = NULL;

    Sessions_free = Sessions_head = Sessions_tail = NULL;

    for (i = 0; i < MAX_SESSIONS; i++)
        Sess_free_session( &Sessions[i] );
}

int     count_bits_set( int32u field, int first_index, int last_index)
{       
        int i;
        int count = 0;
        int32u bitfield;
        assert(first_index >= 0 && first_index < 32);
        assert(last_index >= 0 && last_index <=32);
        assert(last_index >= first_index);

        bitfield = 0x1 << first_index;
        for ( i=0; i < (last_index - first_index); i++, bitfield <<=1)
        {
                if (field & bitfield )
                        count++;
        }
        return(count);
}

void	Sess_init()
{
	struct	sockaddr_in	inet_addr;
	int16			port;
	int			ret, i;
	mailbox			mbox;

#ifndef ARCH_PC_WIN95

	struct	sockaddr_un	unix_addr;
	char			name[80];

	signal( SIGPIPE, SIG_IGN );

#endif	/* ARCH_PC_WIN95 */

        ret = Mem_init_object( MESSAGE_LINK, "message_link", sizeof(message_link), 1000, 0);
        if (ret < 0)
        {
                Alarm(EXIT, "Sess_init: Failure to Initialize MESSAGE_LINK memory objects\n");
        }

        ret = Mem_init_object( DOWN_LINK, "down_link", sizeof(down_link), 200, 0);
        if (ret < 0)
        {
                Alarm(EXIT, "Sess_Init: Failure to Initialize DOWN_LINK memory objects\n");
        }

	Sess_init_sessions ();
	
	Num_sessions = 0;
	GlobalStatus.num_sessions = Num_sessions;
	GlobalStatus.message_delivered = 0;
	My 	 = Conf_my();
	port 	 = My.port;
	Session_threshold = LOW_PRIORITY;

	/* Initializing the protocol */
	Protocol_threshold = LOW_PRIORITY;

        Prot_init_down_queues();
        Prot_set_down_queue( NORMAL_DOWNQUEUE );
        Prot_init();

	/* Initiation of the INET socket */
        memset(&inet_addr.sin_zero, 0, sizeof(inet_addr.sin_zero));

	inet_addr.sin_family	= AF_INET;
	inet_addr.sin_port	= htons(port);
        Accept_inet_mbox_num = 0;

        /* Bind to all interfaces specified in config file */
        for ( i=0; i < My.num_if; i++)
        {
                if (Is_IfType_Client(My.ifc[i].type) || Is_IfType_Any(My.ifc[i].type) )
                {
                        port_reuse type;
                        if( (mbox = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1)
                                Alarm( EXIT, "Sess_init: INET sock error\n" );
                        type = Conf_get_port_reuse_type();
                        if (type == port_reuse_on)
                                Sess_activate_port_reuse(mbox);

                        if (Is_IfType_Any(My.ifc[i].type) )
                                inet_addr.sin_addr.s_addr = INADDR_ANY;
                        else
                        {
                                if (type == port_reuse_auto)
                                        Sess_activate_port_reuse(mbox);
                                inet_addr.sin_addr.s_addr = htonl(My.ifc[i].ip);
                        }
                        if( bind( mbox,  (struct sockaddr *)&inet_addr, sizeof(inet_addr) ) == -1) 
                        {
                                Alarm( PRINT, "Sess_init: INET unable to bind to port %d, already running \n" ,port );
                                exit(0);
                        }
                        inet_addr.sin_addr.s_addr = ntohl(inet_addr.sin_addr.s_addr);
                        Alarm( SESSION, "Sess_init: INET bind for port %d interface %d.%d.%d.%d ok\n", port, 
                               IP1(inet_addr.sin_addr.s_addr), IP2(inet_addr.sin_addr.s_addr),
                               IP3(inet_addr.sin_addr.s_addr), IP4(inet_addr.sin_addr.s_addr) );

                        if( listen( mbox, 25 ) < 0 ) 
                                Alarm( EXIT, "Sess_init: INET unable to listen\n" );

                        Accept_inet_mbox[Accept_inet_mbox_num] = mbox;
                        Accept_inet_mbox_num++;
                        Alarm( SESSION, "Sess_init: INET went ok on mailbox %d\n", mbox );
                }
        }

#ifndef ARCH_PC_WIN95

	/* Initiation of the UNIX socket */

	if( (mbox = socket( AF_UNIX, SOCK_STREAM, 0 ) ) == -1)
	    Alarm( EXIT, "Sess_init: UNIX sock error\n" );

	unix_addr.sun_family	= AF_UNIX;
	snprintf( name, sizeof(name), "%s/%hu", SP_UNIX_SOCKET, My.port );
	strcpy( unix_addr.sun_path, name ); 
	unlink( name );

	if( bind( mbox, (struct sockaddr *)&unix_addr, sizeof(unix_addr) ) == -1) 
	{
		Alarm( PRINT, "Sess_init: UNIX unable to bind to name %s, already running \n" , name );
		exit(0);
	}
	Alarm( SESSION, "Sess_init: UNIX bind for name %s ok\n", name );

	chmod( name, 0666 );

	if( listen( mbox, 5 ) < 0 ) 
	    Alarm( EXIT, "Sess_init: UNIX unable to listen\n" );

	Accept_unix_mbox = mbox;
        Alarm( SESSION, "Sess_init: UNIX went ok on mailbox %d\n", mbox );

#endif	/* ARCH_PC_WIN95 */

	Sess_attach_accept();

        Message_populate_with_buffers(&New_mess);

	G_init();

        Alarm( SESSION, "Sess_init: ended ok\n" );
}

void    Sess_fini(void)
{
#ifndef ARCH_PC_WIN95
        char name[80];

        close( Accept_unix_mbox );
        snprintf( name, sizeof(name), "%s/%hu", SP_UNIX_SOCKET, My.port );
        unlink( name );
#endif
}

void    Sess_signal_conf_reload(void)
{
        My = Conf_my();

        G_signal_conf_reload();
}

void	Sess_set_active_threshold()
{
	/* This function is used only by the session (and groups) layer */
	
	if( Protocol_threshold > Session_threshold ) 
		E_set_active_threshold( Protocol_threshold );
	else 	E_set_active_threshold( Session_threshold );
}

void    Sess_block_user(int xxx)
{

        Alarm(EXIT,"Sess_block_user: NOT IMPLEMENTED!\n");
}

void    Sess_unblock_user(int xxx)
{

        Alarm(EXIT,"Sess_unblock_user: NOT IMPLEMENTED!\n");
}
void	Sess_block_users_level()
{
	/* This function is used only by lower layers (protocol) */
	if( Protocol_threshold < MEDIUM_PRIORITY )
	{
		Protocol_threshold = MEDIUM_PRIORITY;
		Sess_set_active_threshold();
	}
}

void	Sess_unblock_users_level()
{
	/* This function is used only by lower layers (protocol) */
	if( Protocol_threshold > LOW_PRIORITY )
	{
		Protocol_threshold = LOW_PRIORITY;
		Sess_set_active_threshold();
	}
}

static	void	Sess_attach_accept()
{
        int i;
        for ( i=0; i < Accept_inet_mbox_num; i++)
        {
                E_attach_fd( Accept_inet_mbox[i], READ_FD, Sess_accept, AF_INET, NULL, LOW_PRIORITY );
                E_attach_fd( Accept_inet_mbox[i], EXCEPT_FD, Sess_accept, AF_INET, NULL, LOW_PRIORITY );
        }
#ifndef ARCH_PC_WIN95

	E_attach_fd( Accept_unix_mbox, READ_FD, Sess_accept, AF_UNIX, NULL, LOW_PRIORITY );

#endif	/* ARCH_PC_WIN95 */

}

static	void	Sess_detach_accept()
{
        int i;
        for (i=0; i < Accept_inet_mbox_num; i++)
        {
                E_detach_fd( Accept_inet_mbox[i], READ_FD );
                E_detach_fd( Accept_inet_mbox[i], EXCEPT_FD );
        }
#ifndef ARCH_PC_WIN95

	E_detach_fd( Accept_unix_mbox, READ_FD );

#endif	/* ARCH_PC_WIN95 */

}

void	Sess_accept_continue2(int d1, void *d2)
{
	Sess_accept_continue(0,0,NULL);
} 
    

static	void	Sess_accept( mailbox mbox, int domain, void *dummy )
{
	struct	sockaddr_in	inet_addr;
	socklen_t		inet_len;
        sockopt_len_t           onlen;
	sp_time			accept_delay;
	char			response;
	int			ret;
	int			i;

	int32			on;

	if( domain == AF_INET ) 
	{
		inet_len = sizeof(inet_addr);
		Sessions[MAX_SESSIONS].mbox = accept( mbox, (struct sockaddr *)&inet_addr, &inet_len );

		Sessions[MAX_SESSIONS].type = AF_INET;
		/* 
		 * sender's machine ip address is: htonl(inet_addr.sin_addr.s_addr) 
		 * sender's assigned port is     : htons(inet_addr.sin_port)
		 */
		Sessions[MAX_SESSIONS].address = htonl(inet_addr.sin_addr.s_addr);
	}else if( domain == AF_UNIX ){
		/* no need for return values for AF_UNIX on the accept */
		Sessions[MAX_SESSIONS].mbox = accept( mbox, 0, 0 );

		Sessions[MAX_SESSIONS].type = AF_UNIX;
		Sessions[MAX_SESSIONS].address = 0;
	}else Alarm( EXIT, "Sess_accept: Unknown domain %d on mailbox %d\n", domain, mbox );

	if( Sessions[MAX_SESSIONS].mbox < 0 ) 
	{
		Alarm( SESSION, "Sess_accept: accept failed for domain %d\n", domain );
		return;
	}
	if( Num_sessions == MAX_SESSIONS )
	{
		response = REJECT_QUOTA;
		send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
		close( Sessions[MAX_SESSIONS].mbox );
		Alarm( SESSION, "Sess_accept: rejecting session due to quota\n" );
		return;
	}

        if ( ( (i = Sess_get_session_index(Sessions[MAX_SESSIONS].mbox)) != -1)
             && (Sessions[i].mbox == Sessions[MAX_SESSIONS].mbox) 
             && (Is_op_session( Sessions[i].status )) )
        {
            /* This is impossible as the mbox must have been closed to be returned by accept */
            Alarm(EXIT, "Sess_accept: BUG! Accepted new FD %d that is currently in use(ses %d).\n", Sessions[i].mbox, i);
        }
	for( i=10; i <= 200; i+=5 )
	{
	    on = 1024*i;
	    onlen = sizeof(on);

 	    ret = setsockopt( Sessions[MAX_SESSIONS].mbox, SOL_SOCKET, SO_SNDBUF, (void *)&on, onlen);
	    if (ret < 0 ) break;

	    ret = setsockopt( Sessions[MAX_SESSIONS].mbox, SOL_SOCKET, SO_RCVBUF, (void *)&on, onlen);
	    if (ret < 0 ) break;

	    ret= getsockopt( Sessions[MAX_SESSIONS].mbox, SOL_SOCKET, SO_SNDBUF, (void *)&on, &onlen );
	    if( on < i*1024 ) break;
	    Alarm( NONE, "Sess_accept: set sndbuf %d, ret is %d\n", on, ret );

	    onlen = sizeof(on);
	    ret= getsockopt( Sessions[MAX_SESSIONS].mbox, SOL_SOCKET, SO_RCVBUF, (void *)&on, &onlen );
	    if( on < i*1024 ) break;
	    Alarm( NONE, "Sess_accept: set rcvbuf %d, ret is %d\n", on, ret );
	}
	Alarm( SESSION, "Sess_accept: set sndbuf/rcvbuf to %d\n", 1024*(i-5) );

        if ( domain == AF_INET ) {
                on = 1;
                ret = setsockopt( Sessions[MAX_SESSIONS].mbox, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on) );
                if (ret < 0) 
                        Alarm(PRINT, "Setting TCP_NODELAY failed with error: %s\n", sock_strerror(sock_errno));
                else
                        Alarm( SESSION, "Setting TCP_NODELAY on socket %d\n", Sessions[MAX_SESSIONS].mbox );

		on = 1;
		ret = setsockopt( Sessions[MAX_SESSIONS].mbox, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on) );
                if (ret < 0) 
                        Alarm(PRINT, "Setting SO_KEEPALIVE failed with error: %s\n", sock_strerror(sock_errno));
                else
                        Alarm( SESSION, "Setting SO_KEEPALIVE on socket %d\n", Sessions[MAX_SESSIONS].mbox );
        }
	/* delaying for the private name to be written */
	Sess_detach_accept();
	E_attach_fd( Sessions[MAX_SESSIONS].mbox, READ_FD, Sess_accept_continue, 0, NULL, LOW_PRIORITY );	
	E_attach_fd( Sessions[MAX_SESSIONS].mbox, EXCEPT_FD, Sess_accept_continue, 0, NULL, LOW_PRIORITY );	
	accept_delay.sec = 1;
	accept_delay.usec= 0;
	E_queue( Sess_accept_continue2, 1, NULL, accept_delay );
}

void	Sess_accept_continue(mailbox d1, int d2, void *d3)
{
	char			response;
	int			legal_private_name;
	int			unique_private_name;
	int			name_len;
	int			ioctl_cmd;
        char                    version[3];
	char			conn[2];
        char                    priv_user_name[MAX_PRIVATE_NAME];
	int			ret, i, sess_location, rnum;
        char                    *allowed_auth_list;
        unsigned char           list_len;
	session			*tmp_ses;
 
	E_dequeue( Sess_accept_continue2, 1, NULL );
	E_detach_fd( Sessions[MAX_SESSIONS].mbox, READ_FD );
	E_detach_fd( Sessions[MAX_SESSIONS].mbox, EXCEPT_FD );
	Sess_attach_accept();

	/* set file descriptor to non blocking */
	ioctl_cmd = 1;
	ret = ioctl( Sessions[MAX_SESSIONS].mbox, FIONBIO, &ioctl_cmd);

	/* 
	 * connect message looks like:
	 *
	 * byte - version of lib
	 * byte - subversion of lib
         * (optional) byte - patchversion of lib (only if version.subversion > 3.14)
	 * byte - 1/0 with or without groups
	 * byte - len of name
	 * len bytes - name
	 *
	 */
	/* version checking  3.01 is minimal */

        version[0] = version[1] = version[2] = 0;

	ret = recv( Sessions[MAX_SESSIONS].mbox, version, 2, 0 );
	if( ret != 2 )
	{
		Alarm( SESSION, "Sess_accept_continue: reading version.subversion failed on mailbox %d\n", 
			Sessions[MAX_SESSIONS].mbox );
		close( Sessions[MAX_SESSIONS].mbox );
		return;
        }
        if ( version[0]*10000 + version[1]*100 + version[2] > 31400 )
        {
            ret = recv( Sessions[MAX_SESSIONS].mbox, &version[2], 1, 0 );
            if( ret != 1 )
            {
		Alarm( SESSION, "Sess_accept_continue: reading patch_version failed on mailbox %d\n", 
			Sessions[MAX_SESSIONS].mbox );
		close( Sessions[MAX_SESSIONS].mbox );
		return;
            }
        }
	if( version[0]*10000 + version[1]*100 + version[2] < 30100 ) 
	{
		response = REJECT_VERSION;
		send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
		Alarm( SESSION, "Sess_accept_continue: version %d.%d.%d is not supported\n",
			version[0], version[1], version[2] );
		close( Sessions[MAX_SESSIONS].mbox );
		return;
	}

        Sessions[MAX_SESSIONS].lib_version[0] = version[0];
        Sessions[MAX_SESSIONS].lib_version[1] = version[1];
        Sessions[MAX_SESSIONS].lib_version[2] = version[2];
 
	ret = recv( Sessions[MAX_SESSIONS].mbox, conn, 2, 0 );
	if( ret != 2 )
	{
		Alarm( SESSION, "Sess_accept_continue: reading private name failed on mailbox %d\n", 
			Sessions[MAX_SESSIONS].mbox );
		close( Sessions[MAX_SESSIONS].mbox );
		return;

	}

        if( ((int)conn[0] % 2) ==  1 ) Sessions[MAX_SESSIONS].status = Set_memb_session( Sessions[MAX_SESSIONS].status );
        else Sessions[MAX_SESSIONS].status = Clear_memb_session( Sessions[MAX_SESSIONS].status );
        Sessions[MAX_SESSIONS].priority = (int)conn[0] / 16 ;
          
	name_len = (int)conn[1];
	if( name_len > MAX_PRIVATE_NAME || name_len < 0 )
	{
		response = REJECT_ILLEGAL_NAME;
		send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
		Alarm( SESSION, "Sess_accept_continue: len %d of private name does not fit (ret = %d) on mailbox %d\n",
			name_len, ret,
			Sessions[MAX_SESSIONS].mbox );
		close( Sessions[MAX_SESSIONS].mbox );
		return;
	}

	/* reading private name */
	for( i=0; i < MAX_PRIVATE_NAME+1; i++ )
		Sessions[MAX_SESSIONS].name[i] = 0;

        if (name_len == 0)
        {
                /* Assign a random user name to this user. Currently this is a random 4 digit number followed
                 * by the mbox of the users connection.
                 */
                char newname[MAX_PRIVATE_NAME];
                unique_private_name = 0;
                while ( !unique_private_name ) 
                {
                        memset(newname, '\0', MAX_PRIVATE_NAME);
                        rnum = (int) (9999.0*get_rand()/(RAND_MAX+1.0));
                        snprintf(newname, MAX_PRIVATE_NAME, "r%u-%u", rnum, Sessions[MAX_SESSIONS].mbox);
                        memcpy( Sessions[MAX_SESSIONS].name, newname, MAX_PRIVATE_NAME);

                        /* checking if private name is unique */
                        for( unique_private_name=1, tmp_ses = Sessions_head; tmp_ses; tmp_ses = tmp_ses->sort_next )
                        {
                                ret = strcmp( Sessions[MAX_SESSIONS].name, tmp_ses->name );
                                if( ret <= 0 )
                                {
                                        if( ret == 0 ) unique_private_name = 0;
                                        break;
                                }
                        }
                } 
        } else {
                /* recive user name from client and validate it */
                ret = recv( Sessions[MAX_SESSIONS].mbox, priv_user_name, MAX_PRIVATE_NAME, 0 );
                if( ret < 0 )
                {
                        Alarm( SESSION, "Sess_accept_continue: reading private name failed on mailbox %d\n", 
                               Sessions[MAX_SESSIONS].mbox );
                        close( Sessions[MAX_SESSIONS].mbox );
                        return;
                }else if( ret != name_len )
                {
                        response = REJECT_ILLEGAL_NAME;
                        send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
                        Alarm( SESSION, "Sess_accept_continue: len %d of private name does not fit (ret = %d) on mailbox %d\n",
                               name_len, ret, Sessions[MAX_SESSIONS].mbox );
                        close( Sessions[MAX_SESSIONS].mbox );
                        return;
                }
                memcpy( Sessions[MAX_SESSIONS].name, priv_user_name, name_len );

                /* checking if private name is legal */
                for( legal_private_name=1, i=0; i < name_len; i++ )
                        if( Sessions[MAX_SESSIONS].name[i] <= '#' ||
                            Sessions[MAX_SESSIONS].name[i] >  '~' ) legal_private_name = 0;
                if( !legal_private_name )
                {
                        response = REJECT_ILLEGAL_NAME;
                        send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
                        Alarm( SESSION, "Sess_accept_continue: illegal private name %s on mailbox %d\n",
                               Sessions[MAX_SESSIONS].name,
                               Sessions[MAX_SESSIONS].mbox );
                        close( Sessions[MAX_SESSIONS].mbox );
                        return;
                }

                /* checking if private name is unique */
		for( unique_private_name=1, tmp_ses = Sessions_head; tmp_ses; tmp_ses = tmp_ses->sort_next )
                {
                        ret = strcmp( Sessions[MAX_SESSIONS].name, tmp_ses->name );
                        if( ret <= 0 )
                        {
                                if( ret == 0 ) unique_private_name = 0;
                                break;
                        }
                }
                if( !unique_private_name )
                {
                        response = REJECT_NOT_UNIQUE;
                        send( Sessions[MAX_SESSIONS].mbox, &response, sizeof(response), 0 );
                        Alarm( SESSION, "Sess_accept_continue: non unique private name %s on mailbox %d\n",
                               Sessions[MAX_SESSIONS].name,
                               Sessions[MAX_SESSIONS].mbox );
                        close( Sessions[MAX_SESSIONS].mbox );
                        return;
                }
        }

	/* set file descriptor back to blocking */
	ioctl_cmd = 0;
	ret = ioctl( Sessions[MAX_SESSIONS].mbox, FIONBIO, &ioctl_cmd);

	/* Insert the new session just before the point we already
	 * found while checking unique private name... */
	sess_location = Sess_insert_new_session (tmp_ses, &Sessions[MAX_SESSIONS]);
	
	Num_sessions++;
	GlobalStatus.num_sessions = Num_sessions;

        Sessions[sess_location].status = Set_preauth_session( Sessions[sess_location].status );

        Sess_hash_session (&Sessions[sess_location]);

        /* OLD client library without authentication/authorization code */
        if ( version[0]*10000 + version[1]*100 + version[2] < 31600 )
        {
                Acm_acp_fill_ops( &(Sessions[sess_location].acp_ops) );
                
                /* If IP access control is enabled, then check it.
                 */
                if ( Acm_auth_query_allowed("IP") )
                {
                        void (*auth_open)(struct session_auth_info *);
                        struct session_auth_info *sess_auth_p;

                        sess_auth_p = Acm_auth_create_sess_info_forIP(Sessions[sess_location].mbox);
                        auth_open = Acm_auth_get_auth_client_connection_byname("IP");
                        auth_open(sess_auth_p);
                        return;
                }
                /* If no IP authentication enabled, then try to use NULL */
                if ( Acm_auth_query_allowed("NULL") )
                        Sess_session_authorized(sess_location);
                else 
                        Sess_session_denied(sess_location);
                return;
        }

        allowed_auth_list = Acm_auth_get_allowed_list();
        list_len = strlen(allowed_auth_list);

        send( Sessions[sess_location].mbox, &list_len, 1, 0 );
        send( Sessions[sess_location].mbox, allowed_auth_list, list_len, 0 );

        /* If no AllowedAuthMethods are declared, reject all sessions.
         * To maintain old behaivor of allowing all, just add the NULL 
         * method to the allowed list and then all clients will be allowed.
         */
        if (list_len == 0)
        {
                Sess_session_denied(sess_location);
                return;
        }

        /* Now wait for client reply */
	E_attach_fd( Sessions[sess_location].mbox, READ_FD, Sess_recv_client_auth, 0, NULL, LOW_PRIORITY );
	E_attach_fd( Sessions[sess_location].mbox, EXCEPT_FD, Sess_recv_client_auth, 0, NULL, LOW_PRIORITY );
}

static void    Sess_recv_client_auth(mailbox mbox, int dummy, void *dummy_p)
{
        int         ret, i, ioctl_cmd, ses;
        char        auth_name[MAX_AUTH_NAME * MAX_AUTH_METHODS];
        void        (*auth_open)(struct session_auth_info *);
        struct session_auth_info *sess_auth_p;

        ses = Sess_get_session_index(mbox);
	if( ses < 0 || ses >= MAX_SESSIONS ) {
            Alarm( PRINT, "Sess_recv_client_auth: Illegal mbox %d for receiving client auth. Cannot deny or allow\n", mbox);
            return;
        }
        if (!Is_preauth_session(Sessions[ses].status) )
        {
                Alarm( EXIT, "Sess_recv_client_auth: BUG! Session is already authorized (status 0x%x)\n", Sessions[ses].status);
        }
    
        E_detach_fd(mbox, READ_FD);
        E_detach_fd(mbox, EXCEPT_FD);

	/* set file descriptor to non blocking */
	ioctl_cmd = 1;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
    
        /* FIXME: Support partial reads by storing the portion read so far and requeuing */
        ret = recv( mbox, auth_name, MAX_AUTH_NAME * MAX_AUTH_METHODS, 0 );
        if( ret < 0 )
        {
                Alarm( SESSION, "Sess_recv_client_auth: reading auth string failed on mailbox %d\n", mbox );
                Sess_session_denied(ses);
                return;
        }
        if( ret < (MAX_AUTH_NAME * MAX_AUTH_METHODS) )
        {
                Alarm( SESSION, "Sess_recv_client_auth: reading auth string SHORT on mailbox %d\n", mbox );
                Sess_session_denied(ses);
                return;
        }

	/* set file descriptor back to blocking */
	ioctl_cmd = 0;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

        i = 0;
        while ( auth_name[i * MAX_AUTH_NAME] != '\0')
        {
                Alarm( SESSION, "Sess_recv_client_auth: Client requested %s type authentication\n", &auth_name[i * MAX_AUTH_NAME]);
                if ( !Acm_auth_query_allowed(&auth_name[i * MAX_AUTH_NAME]) )
                {
                        Alarm( SESSION, "Sess_recv_client_auth: received non-allowed auth method %s, closing session %d on mailbox %d\n", &auth_name[i * MAX_AUTH_NAME], ses, mbox);
                        Sess_session_denied(ses);
                        return;
                }
                i++;
        }
        /* Register default permit all ops access control policy. */
        Acm_acp_fill_ops( &(Sessions[ses].acp_ops) );

        sess_auth_p = Acm_auth_create_sess_info(mbox, auth_name);
        if ( NULL == sess_auth_p )
        {
                Alarm( SESSION, "Sess_recv_client_auth: no valid auth_methods set or received: auth method %s, closing session %d on mailbox %d\n", auth_name, ses, mbox);
                Sess_session_denied(ses);
                return;
        } 
        auth_open = Acm_auth_get_auth_client_connection( sess_auth_p->required_auth_methods[0] );
        if (auth_open == NULL)
        {
                Alarm(PRINT, "Sess_recv_client_auth: Illegal auth_method_id (%d) tried\n", sess_auth_p->required_auth_methods[0]);
                dispose( sess_auth_p );
                Sess_session_denied( ses );
                return;
        }
        auth_open( sess_auth_p );
}

void    Sess_session_report_auth_result(struct session_auth_info *sess_auth_h, int authenticated_p )
{
        int ses, authid, num_auths;
        int permit_count, decision, i;
        void        (*auth_open)(struct session_auth_info *);

        ses = Sess_get_session_index(sess_auth_h->mbox);
	if( ses < 0 || ses >= MAX_SESSIONS ) {
            Alarm( PRINT, "Sess_session_report_auth_result: Illegal mbox %d for authentication. Cannot deny or allow\n", sess_auth_h->mbox);
            dispose( sess_auth_h );
            return;
        }
        num_auths = sess_auth_h->num_required_auths;
        /* finished another method. See if entire set of checks is complete */
        sess_auth_h->required_auth_results[sess_auth_h->completed_required_auths] = authenticated_p;
        sess_auth_h->completed_required_auths++;
        if (sess_auth_h->completed_required_auths < num_auths)
        {
                authid = sess_auth_h->required_auth_methods[sess_auth_h->completed_required_auths];
                auth_open = Acm_auth_get_auth_client_connection(authid);
                if (auth_open == NULL)
                {
                        Alarm(PRINT, "Sess_session_report_auth_result: Illegal auth_method_id (%d) tried\n", authid);
                        dispose( sess_auth_h );
                        Sess_session_denied( ses );
                        return;
                }
                auth_open(sess_auth_h);
                return;
        }
        permit_count = 0;
        for (i = 0; i < num_auths; i++)
                if ( sess_auth_h->required_auth_results[i] ) permit_count++;

        dispose( sess_auth_h );

        if ( permit_count < num_auths )
        {
                /* session is denied if any authentication method fails */
                Sess_session_denied( ses );
                return;
        }
        decision = Sessions[ses].acp_ops.open_connection(Sessions[ses].name);
        if (decision != ACM_ACCESS_ALLOWED)
        {
                Sess_session_denied( ses );
                return;
        }
        /* NOTE: Here and in the backwards compat support in Sess_accept_cont are the only places
         * that should authorize connections 
         */
        Sess_session_authorized( ses );
} 
void    Sess_session_denied(int ses)
{
        char response;

        if (!Is_preauth_session(Sessions[ses].status) )
        {
                Alarm( EXIT, "Sess_session_denied: BUG! Session is already authorized (status 0x%x)\n", Sessions[ses].status);
        }

        response = REJECT_AUTH;
        send( Sessions[ses].mbox, &response, sizeof(response), 0 );
        Alarm( SESSION, "Sess_session_denied: Authorization denied for %s on mailbox %d\n",
               Sessions[ses].name,
               Sessions[ses].mbox );
        close( Sessions[ses].mbox );

        Sess_unhash_session (&Sessions[ses]);
	Sess_remove_session (&Sessions[ses]);
        Num_sessions--;
        GlobalStatus.num_sessions = Num_sessions;
    
        return;
}

void    Sess_session_authorized(int ses)
{
        char        ip[16];
        char        response;
        unsigned int    name_len;
        char	private_group_name[MAX_GROUP_NAME];

        if (!Is_preauth_session(Sessions[ses].status) )
        {
                Alarm( EXIT, "Sess_session_authorized: BUG! Session is already authorized (status 0x%x)\n", Sessions[ses].status);
        }

        /* 
         * accept message looks like:
         *
         * byte - ACCEPT_SESSION code
         * byte - version of spread
         * byte - subversion of spread
         * (optional) byte - patch version of spread (only if library is 3.15.0 or greater)
         * byte - len of name
         * len bytes - name
         *
         */
        Sessions[ses].num_mess = 0;
        response = ACCEPT_SESSION;
        send( Sessions[ses].mbox, &response, 1, 0 );

        response = SP_MAJOR_VERSION;
        send( Sessions[ses].mbox, &response, 1, 0 );
        response = SP_MINOR_VERSION;
        send( Sessions[ses].mbox, &response, 1, 0 );

        if (Sessions[ses].lib_version[0]*10000 + Sessions[ses].lib_version[1]*100 + Sessions[ses].lib_version[2] >= 31500)
        {
                response = SP_PATCH_VERSION;
                send( Sessions[ses].mbox, &response, 1, 0 );
        }

        snprintf(private_group_name, MAX_GROUP_NAME, "#%s#%s", Sessions[ses].name, My.name );
        name_len = strlen( private_group_name );
        /* sending the len of the private group in one byte */
        response = name_len;
        send( Sessions[ses].mbox, &response, 1, 0 );
        /* sending the private group name */
        send( Sessions[ses].mbox, private_group_name, name_len, 0 );

        E_attach_fd( Sessions[ses].mbox, READ_FD, Sess_read, Sessions[ses].type, NULL, 
                     LOW_PRIORITY );
        E_attach_fd( Sessions[ses].mbox, EXCEPT_FD, Sess_read, Sessions[ses].type, NULL, 
                     LOW_PRIORITY );

        Sessions[ses].status = Set_op_session( Sessions[ses].status );
        Sessions[ses].status = Clear_preauth_session( Sessions[ses].status );

        Prot_Create_Local_Session(&Sessions[ses]);

        Message_reset_current_location(&(Sessions[ses].read) );
        Message_reset_current_location(&(Sessions[ses].write) );
        Sessions[ses].read.in_mess_head = 1;

        Log_sess_connect( Sessions[ses].mbox, Sessions[ses].address, 
                          Sessions[ses].name );

        Conf_id_to_str( Sessions[ses].address, ip );
        Alarm( SESSION, "Sess_session_authorized: Accepting from %s with private name %s on mailbox %d\n",
               ip,
               Sessions[ses].name,
               Sessions[ses].mbox );
}
static  int     Sess_validate_read_header( mailbox mbox, int ses, int head_size, message_header *head_ptr)
{
	char		private_name[MAX_PRIVATE_NAME+1];
	char		proc_name[MAX_PROC_NAME];
        int             ret, type_bits, memb_bits;

        /* Disallow more then one message type being set or more then one Join/Leave/Kill being set */
        if ( ( (type_bits = count_bits_set( head_ptr->type, 0, 6)) > 1) ||
             ( (memb_bits = count_bits_set( head_ptr->type, 16, 19)) > 1) ||
             ( (type_bits + memb_bits) != 1 ) ) 
        {
                Alarm( SESSION, "Sess_validate_read_header: Message has illegal type field 0x%x\n", head_ptr->type);
                return(-1);
        }
        head_ptr->private_group_name[MAX_GROUP_NAME -1] = '\0';

        ret = G_private_to_names( head_ptr->private_group_name, private_name, proc_name );
        if( ret < 0 )
        {
                Alarm( SESSION, "Sess_validate_read_header: Message has illegal private_group_name (priv, proc)\n");
                return(-1);
        }
        if( strncmp( proc_name, My.name, MAX_PROC_NAME ) != 0 )
        {
                Alarm( SESSION, "Sess_validate_read_header: proc name %s is not my name %s\n",
                       proc_name, My.name );
                return(-1);
        }
        if (strncmp(private_name , Sessions[ses].name, MAX_PRIVATE_NAME) )
        {
                Alarm( PRINT, "Sess_validate_read_header: Session %s trying to make session %s do something\n",
                       private_name, Sessions[ses].name );
                return(-1);
        }
                
        if ( (head_ptr->num_groups < 0) || (head_ptr->num_groups > MAX_GROUPS_PER_MESSAGE) )
        {
                Alarm( SESSION, "Sess_validate_read_header: Message has negative or too large num_groups field\n", head_ptr->num_groups);
                return(-1);
        }
        if ( head_ptr->hint & ~( 0x80000080 | 0x00ffff00 ) )
        {
                Alarm( SESSION, "Sess_validate_read_header: Message has illegal hint field 0x%x\n", head_ptr->hint);
                return(-1);
        }

        if ( (head_ptr->data_len < 0 ) || ( head_ptr->data_len > MAX_MESSAGE_BODY_LEN) )
        {
                Alarm( SESSION, "Sess_validate_read_header: Message has negative or too large data_len %d\n", head_ptr->data_len);
                return(-1);
        }
        if ( (head_ptr->data_len + MAX_GROUP_NAME * head_ptr->num_groups) > (MAX_MESSAGE_BODY_LEN - head_size ) )
        {
                Alarm( SESSION, "Sess_validate_read_header: Message + Groups is too large (%d + %d = %d). MAX size is: %d\n", 
                       head_ptr->data_len, MAX_GROUP_NAME * head_ptr->num_groups, 
                       head_ptr->data_len + MAX_GROUP_NAME * head_ptr->num_groups,
                       MAX_MESSAGE_BODY_LEN - head_size );
                return(-1);
        }
        /* Passed all checks, so valid */
        return( 0 );
}

static	void	Sess_read( mailbox mbox, int dummy, void *d2 )
{
	message_header	*head_ptr, *msg_head;
        message_obj     *msg;
        scatter         *scat;
	down_link	*down_ptr;
        message_link    *mess_link;
	int		packet_index, byte_index, to_read;
	int             len, remain, ret;
        int             head_size, data_frag_len;
        int             ses, ioctl_cmd;
        char            *head_cbuf;
#if 0
#ifndef ARCH_SCATTER_NONE
static  struct  msghdr  msgh;
#endif  /* ARCH_SCATTER_NONE */
#endif  /* 0 */
#if 0
        /* we currently don't use recvmsg */
#ifndef ARCH_SCATTER_NONE
	msgh.msg_name    = (caddr_t) 0;
	msgh.msg_namelen = 0;
	msgh.msg_iov     = (struct iovec *)scat->elements;
	msgh.msg_iovlen  = scat->num_elements;
#endif  /* ARCH_SCATTER_NONE */

#ifdef ARCH_SCATTER_CONTROL
	msgh.msg_control = (caddr_t) 0;
	msgh.msg_controllen = 0;
#endif /* ARCH_SCATTER_CONTROL */
#ifdef ARCH_SCATTER_ACCRIGHTS
	msgh.msg_accrights = (caddr_t) 0;
	msgh.msg_accrightslen = 0;
#endif /* ARCH_SCATTER_ACCRIGHTS */
#endif /* 0 */

        ses = Sess_get_session_index(mbox);
	if( ses < 0 || ses >= MAX_SESSIONS ) {
            Alarm( PRINT, "Sess_read: Illegal mbox %d for read\n", mbox);
            return;
        }
        if (Sessions[ses].read_mess == NULL)
                Sessions[ses].read_mess = Message_new_message();

        msg = Sessions[ses].read_mess;
	head_ptr = Message_get_message_header(msg);
        head_cbuf = (char *) head_ptr;
        head_size = Message_get_header_size();

        /* set file descriptor to non blocking */
	ioctl_cmd = 1;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

        if ( Sessions[ses].read.in_mess_head == 1 )
        {
                /* read up to size of message_header */
                len = Sessions[ses].read.cur_byte;
                remain = sizeof(message_header) - len;
                ret = recv( mbox, (char *) &head_cbuf[len], remain, 0 );
                if( ret  == remain )
                {
                        Sessions[ses].read.cur_byte += ret;
                        Message_set_location_begin_body(&(Sessions[ses].read) );
                } else  if (ret > 0 ) {
                        Sessions[ses].read.cur_byte += ret;
                        ioctl_cmd = 0;
                        ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
                        return;
                } else {
                        /* error reading */
                        if ( (ret == -1) && ( (sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK) ) ) {
                                ioctl_cmd = 0;
                                ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
                                return;
                        }
                        Alarm( SESSION, "Sess_read: failed receiving header on session %d: ret %d: error: %s \n", mbox, ret, sock_strerror(sock_errno) );
                        Sess_kill( mbox );
                        ioctl_cmd = 0;
                        ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
                        return;
                }
                /* When we get here we have a complete header */
                ioctl_cmd = 0;
                ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        

                /* Fliping message header to my form if needed */
                if( !Same_endian( head_ptr->type ) ) 
                {
                        Flip_mess( head_ptr );
                }
                /* Setting endian to my endian on the header */
                head_ptr->type = Set_endian( head_ptr->type );

                /* Validate all fields */
                Alarm( SESSION, "Sess_read: Message has type field 0x%x\n", head_ptr->type);
                ret = Sess_validate_read_header( mbox, ses, head_size, head_ptr);
                if (ret < 0 )
                { 
                        /* invalid header */
                        Sess_kill(mbox);
                        return;
                }
        } /* finished reading and validating  header */

	/* 
	 * to do recvmsg, but need to trick with the starting AFTER the header 
	 * on the first packet, and then to return the big_scatter to original
	 * form. read at *most* head_ptr->data_len (set scatter lengths accordingly
	 * everytime here from scratch! 
	 *
	 * ret = recvmsg( mbox, &msg, 0 ); 
	 * if( ret <=0 )
	 * {
	 * 	Alarm( SESSION, "Sess_read: failed receiving message on session %d\n", mbox );
	 * 	Sess_kill( mbox );
	 * 	return;
	 * }
	 */

	/* read the rest of the message if needed, reserving room at the beginning
         * of the first fragment(scat buf) for the message header and the lts and seq fields. */

        /* enable non-blocking io */
	ioctl_cmd = 1;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

        data_frag_len = Message_get_data_fragment_len();
        scat = Message_get_data_scatter(msg);
	remain = ( head_ptr->data_len + MAX_GROUP_NAME*head_ptr->num_groups )  - Sessions[ses].read.total_bytes;
	for(  ; remain > 0; remain -= ret )
	{
		packet_index = Sessions[ses].read.cur_element;
		byte_index   = Sessions[ses].read.cur_byte;
                if (packet_index >= (int) scat->num_elements) 
                {
                        /* We are beginning a new fragment -- so allocate it */
                        assert(byte_index == 0);
                        Message_add_scat_element(msg);
                }
		to_read = ( data_frag_len - byte_index );
		if( to_read > remain ) to_read = remain;
		ret = recv( mbox, &scat->elements[packet_index].buf[byte_index],
				to_read, 0 );
                if( ret  == to_read )
                {
                        Sessions[ses].read.cur_byte = 0;
                        Sessions[ses].read.cur_element++;
                        Sessions[ses].read.total_bytes += ret;
                } else  if (ret > 0 ) {
                        Sessions[ses].read.cur_byte += ret;
                        Sessions[ses].read.total_bytes += ret;
                        ioctl_cmd = 0;
                        ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
                        return;
                } else {
                        if ( (ret == -1) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) ) {
                                ioctl_cmd = 0;
                                ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
                                return;
                        }
			Alarm( SESSION, "Sess_read: failed receiving message on session %d, ret is %d: error: %s\n", mbox, ret, sock_strerror(sock_errno) );
			Alarm( SESSION, "Sess_read: failed recv msg more info: len read: %d, remain: %d, to_read: %d, pkt_index: %d, b_index: %d, scat_nums: %d\n",Sessions[ses].read.total_bytes, remain, to_read, packet_index, byte_index, scat->num_elements );
			Sess_kill( mbox );
                        ioctl_cmd = 0;
                        ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        
			return;
		}
	}

        /* We now have a complete message */
        ioctl_cmd = 0;
        ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);        

        /* reset active read_mess to empty */
        Message_reset_current_location(&(Sessions[ses].read));
        Sessions[ses].read.in_mess_head = 1;
        Sessions[ses].read_mess = NULL;

        Message_element_len_fixup(msg);

#ifdef  PROBE_LATENCY
        if (Is_latency_mess( head_ptr->type ) )
        {
                int32u          initial_offset, htime_offset;
                int32u          *p_time_offset;
                sp_time         cur_time, ncur_time;

                initial_offset = MAX_GROUP_NAME*head_ptr->num_groups + head_size;
                p_time_offset = (int32u *) &msg->body.elements[0].buf[initial_offset];
                htime_offset = ntohl(*p_time_offset);
                Alarm(SESSION, "Sess_read: Msg Data at %d with time_offset %u \n", initial_offset, htime_offset);
                cur_time = E_get_time();
                ncur_time.sec = htonl(cur_time.sec);
                ncur_time.usec = htonl(cur_time.usec);
                memcpy(&(msg->body.elements[0].buf[htime_offset + initial_offset]), &ncur_time, sizeof(sp_time));
                Alarm(SESSION, "Sess_read: timestamped time (%d, %d) in byte %d of message\n", cur_time.sec, cur_time.usec, htime_offset);
                *p_time_offset = htonl(htime_offset + sizeof(sp_time) );
        }
#endif  /* PROBE_LATENCY */

        /* Do ACM access control checks */
        /* Note, disconnects (Is_kill_mess) are not limited. Someone can always cut themselves off */
        if ( Is_leave_mess( head_ptr->type ) )
        {
                char *groups_ptr;
                int decision;
                groups_ptr = Message_get_first_group( msg );
                decision = Sessions[ses].acp_ops.leave_group( head_ptr->private_group_name, groups_ptr, NULL);
                if (decision != ACM_ACCESS_ALLOWED)
                {
                        head_ptr->type = (head_ptr->type & ~LEAVE_MESS);
                        head_ptr->type |= CAUSED_BY_LEAVE;
                        Sess_create_reject_message( msg );
                        Sess_deliver_reject( msg );
                        return;
                }
        }
        if ( Is_join_mess( head_ptr->type ) )
        {
                char *groups_ptr;
                int decision;
                groups_ptr = Message_get_first_group( msg );

                /* Make sure we don't let a join happen if the limit has been reached. */
		if( G_get_num_local( groups_ptr ) == MAX_LOCAL_GROUP_MEMBERS ) {
                        Alarm( PRINT, "Sess_read: Attempt by session %s to join group %s " 
                               "failed: too many local members.\n", head_ptr->private_group_name, groups_ptr );
                        head_ptr->type = (head_ptr->type & ~JOIN_MESS);
                        head_ptr->type |= CAUSED_BY_JOIN;
                        Sess_create_reject_message( msg );
                        Sess_deliver_reject( msg );
                        return;
                }

                decision = Sessions[ses].acp_ops.join_group( head_ptr->private_group_name, groups_ptr, NULL);
                if (decision != ACM_ACCESS_ALLOWED)
                {
                        head_ptr->type = (head_ptr->type & ~JOIN_MESS);
                        head_ptr->type |= CAUSED_BY_JOIN;
                        Sess_create_reject_message( msg );
                        Sess_deliver_reject( msg );
                        return;
                }
        }
        if ( Is_only_regular_mess( head_ptr->type ) )
        {
                char *groups_ptr;
                char target_groups[MAX_GROUPS_PER_MESSAGE][MAX_GROUP_NAME];
                int decision, num_p2p_dest;
                groups_ptr = Message_get_groups_array( msg );
                num_p2p_dest = Sess_get_p2p_dests(head_ptr->num_groups, (char (*)[MAX_GROUP_NAME])groups_ptr, target_groups);
                if (num_p2p_dest)
                {
                        decision = Sessions[ses].acp_ops.p2p_send( head_ptr->private_group_name, num_p2p_dest, target_groups, head_ptr->type,  ( (head_ptr->hint >> 8) & 0x0000ffff) );
                        if (decision != ACM_ACCESS_ALLOWED)
                        {
                                Sess_create_reject_message( msg );
                                Sess_deliver_reject( msg );
                                return;
                        }
                }
                if (head_ptr->num_groups > num_p2p_dest)
                {
                        decision = Sessions[ses].acp_ops.mcast_send( head_ptr->private_group_name, head_ptr->num_groups, (char (*)[MAX_GROUP_NAME])groups_ptr, head_ptr->type, ( (head_ptr->hint >> 8) & 0x0000ffff) );
                        if (decision != ACM_ACCESS_ALLOWED)
                        {
                                Sess_create_reject_message( msg );
                                Sess_deliver_reject( msg );
                                return;
                        }
                }
        }

	/* create new down_link and big_scatter */
        down_ptr = Prot_Create_Down_Link(msg, Message_get_packet_type(head_ptr->type), mbox, 0);
        if (down_ptr == NULL)
        {
                Alarm( SESSION, "Sess_read: Session has illegal message type 0x%x\n", head_ptr->type);
                Sess_kill( mbox );
                return;
        }
        down_ptr->mess = msg;
        msg_head = Message_get_message_header(down_ptr->mess);

        if (Is_kill_mess(msg_head->type) )
        {       
                /* We are going to overwrite the group that is sent from the library. 
                 * it is not needed for kill messages, so we shrink the data field to ignore it
                 */
                len = Message_kill_mess_fixup(down_ptr->mess, Sessions[ses].read.total_bytes - MAX_GROUP_NAME, mbox);

                /* A bug in both 3.13 and 4 I think is that if we get a DISCONNECT message
                 * from the client and we process and deliver that before discovering the
                 * closed socket ourselves and calling Sess_kill(), then the session
                 * is in the wrong state and we will crash when we try to finish delivery.
                 */
                Log_sess_disconnect( Sessions[ses].mbox, Sessions[ses].address, Sessions[ses].name,
                                     Sessions[ses].num_mess );
                /* clear his structure */
                while( Sessions[ses].num_mess > 0 )
                {
                        mess_link = Sessions[ses].first;
                        Sessions[ses].first = Sessions[ses].first->next;
                        Sess_dispose_message( mess_link );
                        Sessions[ses].num_mess--;
                }

                /* close the mailbox and mark it unoperational */
                E_dequeue( Sess_badger_TO, mbox, NULL );
                E_detach_fd( mbox, READ_FD );
                E_detach_fd( mbox, EXCEPT_FD );
		E_detach_fd( mbox, WRITE_FD );
                close( mbox );
                /* the mailbox is closed but the entry still points to it */
                Sessions[ses].status = Clear_op_session( Sessions[ses].status );
                Alarm( SESSION, "Sess_read: disconnecting session %s ( mailbox %d )\n",Sessions[ses].name, mbox );
        }

	Alarm( SESSION, "Sess_read: queueing message of type %d with len %d to the protocol\n",
		down_ptr->type, Sessions[ses].read.total_bytes );
	Prot_new_message( down_ptr, Sessions[ses].down_queue );
}

static  int     Sess_get_p2p_dests( int num_groups, char groups[][MAX_GROUP_NAME], char dests[][MAX_GROUP_NAME] )
{
        int i, num_p2p_targets = 0;
	for( i=0; i < num_groups; i++ )
	{
		if( groups[i][0] == '#' )
		{
			/* private group */
                        memcpy( dests[num_p2p_targets], groups[i], MAX_GROUP_NAME);
                        num_p2p_targets++;
		}
        }
        return( num_p2p_targets );
} 
/* Take a message received from a client and change it into the form of
 * a reject message. Destination groups, user data, mess_type and type field
 * are all preserved to give the sender information about what message was 
 * rejected.
 */
static  void    Sess_create_reject_message ( message_obj *msg )
{
        message_header  *head_ptr;
        int32u          old_type;

        head_ptr = Message_get_message_header(msg);

        old_type = head_ptr->type;
        head_ptr->type = REJECT_MESS;
        head_ptr->type = Set_endian( head_ptr->type );
        /* If original message was SELF_DISCARD, then maintain that state */
        if (Is_self_discard( old_type) ) head_ptr->type |= SELF_DISCARD;

        Message_add_oldtype_to_reject( msg, old_type );

        Alarm( PRINT, "Sess_create_reject_mess: Created Reject for sender %s type 0x%x oldtype 0x%x for first group %s\n", 
               head_ptr->private_group_name, head_ptr->type, old_type, Message_get_first_group( msg ) );
        return;
}

static  void    Sess_deliver_reject( message_obj *msg )
{
        message_link    *mess_link;

        mess_link = new(MESSAGE_LINK);
        if (mess_link == NULL ) {
                Alarm(EXIT, "Sess_deliver_reject: Failed to allocate a new MESSAGE_LINK.\n");
                return;
        }
        mess_link->mess = msg;
        mess_link->next = NULL;
        
        Sess_deliver_message( mess_link );

        return;
}

void    Sess_write( int ses, message_link *mess_link, int *needed )
{
        message_obj     *msg;
	message_link	*tmp_link;
        scatter         *scat;
	int		ioctl_cmd;
	int 		ret;
	int		total_to_send;
	int		len_sent, first_data_len;
        char            *first_data_ptr;
	int		i;
        message_header  *head_ptr;

	if( !Is_op_session( Sessions[ses].status ) ) return;

	if( Sessions[ses].num_mess >= Conf_get_max_session_messages() )
	{
		Alarm( SESSION, 
			"Sess_write: killing mbox %d for not reading\n",
			Sessions[ses].mbox );
		Sess_kill( Sessions[ses].mbox );
		return;
	}

	if( Sessions[ses].num_mess > 0 )
	{
		Sess_badger( Sessions[ses].mbox );
	}

	msg = mess_link->mess;
        Obj_Inc_Refcount(msg);
        scat = Message_get_data_scatter(msg);

	for( total_to_send=0, i=0; i < (int) scat->num_elements; i++ )
		total_to_send += scat->elements[i].len;

        /* since also sending message_header */
        total_to_send += Message_get_non_body_header_size();

        head_ptr = Message_get_message_header(msg);
#ifdef  PROBE_LATENCY
        if (Is_latency_mess( head_ptr->type ) )
        {
                int32u          initial_offset, htime_offset;
                int32u          *p_time_offset;
                sp_time         cur_time, ncur_time;

                initial_offset = MAX_GROUP_NAME*head_ptr->num_groups;
                p_time_offset = (int32u *) &msg->body.elements[0].buf[initial_offset];
                htime_offset = ntohl(*p_time_offset);
                Alarm(SESSION, "Sess_write: Msg Data at %d with timeoffset %u\n", initial_offset, htime_offset);
                cur_time = E_get_time();
                ncur_time.sec = htonl(cur_time.sec);
                ncur_time.usec = htonl(cur_time.usec);
                memcpy(&(msg->body.elements[0].buf[htime_offset + initial_offset]), &ncur_time, sizeof(sp_time));
                Alarm(SESSION, "Sess_write: timestamped time (%d, %d) in byte %d of message\n", cur_time.sec, cur_time.usec, htime_offset);
                *p_time_offset = htonl(htime_offset + sizeof(sp_time) );
        }
#endif
	len_sent = 0;
	if( Sessions[ses].num_mess == 0 )
	{
		/* set file descriptor to non blocking */
		ioctl_cmd = 1;
		ret = ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);

                /* Send the first part of message which must be handled specially */
                ret = send(Sessions[ses].mbox, (char *) head_ptr, 
                           sizeof(message_header), 0);
                if ( ret > 0 ) len_sent += ret;
                if ( ret != sizeof(message_header) )
                {
                        goto end_write;
                }
                /* Send the first data of message which must be handled specially */
                first_data_ptr = Message_get_first_data_ptr(msg);
                first_data_len = Message_get_first_data_len(msg);
                ret = send(Sessions[ses].mbox, first_data_ptr, 
                           first_data_len, 0);
                if ( ret > 0 ) len_sent += ret;
                if ( ret != first_data_len )
                {
                        goto end_write;
                }
		/* send the message after first buffer*/
		for( i=1; i < (int) scat->num_elements; i++ )
		{
			ret = send( Sessions[ses].mbox, scat->elements[i].buf, 
							scat->elements[i].len, 0);
			if( ret > 0 ) len_sent += ret;
			if( ret != scat->elements[i].len )
			{ 
				break;
			}
		}
		/* set file descriptor back to blocking */
        end_write:
		ioctl_cmd = 0;
		ret = ioctl( Sessions[ses].mbox, FIONBIO, &ioctl_cmd);

	}

	if( len_sent < total_to_send )
	{
		/* this message has to be linked */
		if( *needed )
		{
			/* create a copy of mess_link and link it */
			tmp_link = new(MESSAGE_LINK);
                        if (tmp_link == NULL ) {
                                Alarm(EXIT, "Sess_write: Failed to allocate a new MESSAGE_LINK.\n");
                                return;
                        }
                        tmp_link->mess = Message_copy_message(msg);
			++*needed;
		}else{
			/* should link mess_link itself */
			tmp_link = mess_link;
			*needed=1;
		}
		/* link the message */
		tmp_link->next = 0;
		if( Sessions[ses].num_mess == 0 )
		{
			Sessions[ses].first = tmp_link;
			Sessions[ses].last = tmp_link;
			/* setting cur_element and cur_byte */
                        Message_calculate_current_location(tmp_link->mess, len_sent, &(Sessions[ses].write) );

			/* We will need to badger this guy */
			E_queue( Sess_badger_TO, Sessions[ses].mbox, NULL, Badger_timeout );
			E_attach_fd( Sessions[ses].mbox, WRITE_FD, Sess_badger_FD, 0, NULL, LOW_PRIORITY );
		}else{
			/* This guy was already badgered */
			Sessions[ses].last->next = tmp_link;
			Sessions[ses].last = tmp_link;
		}
		Sessions[ses].num_mess++;
	}
        Message_Dec_Refcount(msg);
}

static	void	Sess_badger( mailbox mbox )
{
	int		ses;
	message_link	*mess_link;
	int		able_to_write;
        message_obj     *msg;
        scatter         *scat;
	int		ioctl_cmd;
	int		bytes_to_send, from;
	int		i;
	int		ret;

	Alarm( SESSION, "Sess_badger: for mbox %d\n", mbox );
	ses = Sess_get_session_index( mbox );
	if( ses < 0 || ses >= MAX_SESSIONS || !Is_op_session( Sessions[ses].status ) || Sessions[ses].num_mess <= 0 ) goto NO_WORK;

	/* set file descriptor to non blocking */
	ioctl_cmd = 1;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

	for( able_to_write = 1 ; Sessions[ses].num_mess > 0 && able_to_write;  )
	{
		msg = Sessions[ses].first->mess;
                Obj_Inc_Refcount(msg);

                /* First if we havn't sent the mess_head yet, send that special
                 * and then reset the cur_byte and in_mess_head fields 
                 */
                if ( Sessions[ses].write.in_mess_head == 1 )
                {
                        char *tmp_buf;

                        tmp_buf = (char *) Message_get_message_header(msg);
                        from = Sessions[ses].write.cur_byte;
                        bytes_to_send = sizeof(message_header) - from;
                        ret = send(mbox, &tmp_buf[from], bytes_to_send, 0);
                        if (ret == bytes_to_send)
                        {
                                Message_set_location_begin_body(&(Sessions[ses].write));
                        } else 
                        {
                                if (ret > 0) Sessions[ses].write.cur_byte = from + ret;
                                able_to_write = 0;
                        }
                }
                /* Next if we still can, send some body of this messsage */
                if ( able_to_write )
                {
                        scat = Message_get_data_scatter(msg);
                        for( i=Sessions[ses].write.cur_element, from=Sessions[ses].write.cur_byte;
                             i < (int) scat->num_elements; 
                             i++, from=0 )
                        {
                                bytes_to_send = scat->elements[i].len - from;
                                ret = send( mbox, &(scat->elements[i].buf[from]), bytes_to_send, 0);
                                if( ret == bytes_to_send )
                                {        
                                        Sessions[ses].write.cur_byte = 0;
                                        Sessions[ses].write.cur_element++;
                                }else{
                                        if( ret > 0 ) Sessions[ses].write.cur_byte += ret;
                                        able_to_write = 0;
                                        break; 
                                }
                        }
                }
		if( able_to_write )
		{
			/* free that message */
			mess_link = Sessions[ses].first;
			Sessions[ses].first = Sessions[ses].first->next;
			Sessions[ses].num_mess--;
                        Message_reset_current_location(&(Sessions[ses].write) );
			Sess_dispose_message( mess_link );
		}
                Message_Dec_Refcount(msg);
	} /* for loop per message */
	/* set file descriptor back to blocking */
	ioctl_cmd = 0;
	ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

	if( Sessions[ses].num_mess > 0 ) {
	  E_queue( Sess_badger_TO, mbox, NULL, Badger_timeout );
	  E_attach_fd( mbox, WRITE_FD, Sess_badger_FD, 0, NULL, LOW_PRIORITY );

	}else{
	NO_WORK:
	  E_dequeue( Sess_badger_TO, mbox, NULL );
	  E_detach_fd( mbox, WRITE_FD );
	}
}

static void Sess_badger_TO( mailbox mbox, void *dmy )
{
        Sess_badger( mbox );
}

static void Sess_badger_FD( mailbox mbox, int dmy, void *dmy2 )
{
        Sess_badger( mbox );
}

static	void	Sess_kill( mailbox mbox )
{
	int		ses;
	message_link	*mess_link;
	message_obj	*kill_mess;
        message_header  *kill_head;
	down_link	*down_ptr;
        int             head_size;
        char            private_send_group[MAX_GROUP_NAME];

        head_size = Message_get_header_size();
	ses = Sess_get_session_index(mbox);
	if( ses < 0 || ses >= MAX_SESSIONS ) {
            Alarm( PRINT, "Sess_kill: Illegal mbox %d for killing. Cannot cleanup\n", mbox);
            return;
        }
	if( !Is_op_session( Sessions[ses].status ) )
        {
                if ( !Is_preauth_session( Sessions[ses].status ) )
                        Alarm( PRINT, 
                               "Sess_kill: session %s with mailbox %d is already killed (status %d)\n",
                               Sessions[ses].name, mbox, Sessions[ses].status );
                else 
                        Alarm( EXIT, 
                               "Sess_kill: BUG! session %s with mailbox %d killed before authentication and authorization completed (status 0x%x)\n",
                               Sessions[ses].name, mbox, Sessions[ses].status);
        }
	
	/* send a message to kill this session */
	snprintf( private_send_group, MAX_GROUP_NAME, "#%s#%s", Sessions[ses].name, My.name );
        kill_mess = Message_create_message(KILL_MESS | SAFE_MESS, private_send_group);
        kill_head = Message_get_message_header(kill_mess);
        Message_kill_mess_fixup(kill_mess, head_size, mbox );

        down_ptr = Prot_Create_Down_Link(kill_mess, Message_get_packet_type(kill_head->type), mbox, 0);
	down_ptr->mess = kill_mess;

        Obj_Inc_Refcount(down_ptr->mess);
	Prot_new_message( down_ptr, Sessions[ses].down_queue );
        Message_Dec_Refcount(kill_mess);

	Log_sess_disconnect( Sessions[ses].mbox, Sessions[ses].address, Sessions[ses].name,
			     Sessions[ses].num_mess );
	/* clear his structure */
	while( Sessions[ses].num_mess > 0 )
	{
		mess_link = Sessions[ses].first;
		Sessions[ses].first = Sessions[ses].first->next;
		Sess_dispose_message( mess_link );
		Sessions[ses].num_mess--;
	}
        /* reset active read_mess to empty */
        Message_reset_current_location(&(Sessions[ses].read));
        Sessions[ses].read.in_mess_head = 1;
        if (Sessions[ses].read_mess != NULL)
            Message_dispose_message( Sessions[ses].read_mess );
        Sessions[ses].read_mess = NULL;

	/* close the mailbox and mark it unoperational */
	E_dequeue( Sess_badger_TO, mbox, NULL );
	E_detach_fd( mbox, READ_FD );
	E_detach_fd( mbox, EXCEPT_FD );
	E_detach_fd( mbox, WRITE_FD );
        close(mbox);
	/* the mailbox is closed but the entry still points to it */
	Sessions[ses].status = Clear_op_session( Sessions[ses].status );
	Alarm( SESSION, "Sess_kill: killing session %s ( mailbox %d )\n",Sessions[ses].name, mbox );
}

void	Sess_deliver_message( message_link *mess_link )
{
static	int		target_sessions[MAX_SESSIONS];
	int		num_target_sessions;
	int		source_ses;
	message_header	*head_ptr;
        message_obj     *msg;
        scatter         *scat;
	char		*target_groups;
	char		private_name[MAX_PRIVATE_NAME+1];
	char		proc_name[MAX_PROC_NAME];
	int		needed;
	int		i;

	msg = mess_link->mess;
        Obj_Inc_Refcount(msg);
	head_ptr = Message_get_message_header(msg);

        Message_endian_correct_message_header(msg);

	if( Is_join_mess( head_ptr->type ) )
	{
		Sess_handle_join( mess_link );
                Message_Dec_Refcount(msg);
		return;
	}
	
	if( Is_leave_mess( head_ptr->type ) )
	{
		Sess_handle_leave( mess_link );
                Message_Dec_Refcount(msg);
		return;
	}

	if( Is_kill_mess( head_ptr->type ) )
	{
		Sess_handle_kill( mess_link );
                Message_Dec_Refcount(msg);
		return;
	}
	
	if( Is_groups_mess( head_ptr->type ) )
	{
		G_handle_groups( mess_link );
                Message_Dec_Refcount(msg);
		return;
	}

	/* regular message */

	GlobalStatus.message_delivered++;

	/* Setting endian to my endian on the header */
	head_ptr->type = Set_endian( head_ptr->type );
        scat = Message_get_data_scatter(msg);
	/* analyze message  groups to sessions  */
        if ( Is_reject_mess(head_ptr->type) ) {
            num_target_sessions = 1;
            G_private_to_names( head_ptr->private_group_name, private_name, proc_name );
            target_sessions[0] = Sess_get_session( private_name );
        } else {
            target_groups = Message_get_groups_array(msg);
            num_target_sessions = G_analize_groups( head_ptr->num_groups, 
                                                    (char (*)[MAX_GROUP_NAME])target_groups, 
                                                    target_sessions ) ;
        }
	/* if self_discard, sender is local and a target then eliminate sender from targets */
	source_ses = -1;
	if( num_target_sessions > 0 && Is_self_discard( head_ptr->type ) )
	{
		G_private_to_names( head_ptr->private_group_name, private_name, proc_name );
		if( strcmp( My.name, proc_name ) == 0 )
		{
			source_ses = Sess_get_session( private_name );
		}
	}
	needed = 0;
	for( i = 0; i < num_target_sessions ; i++ )
	{
		if( source_ses == target_sessions[i] ) continue; /* self_discard */
		Sess_write( target_sessions[i], mess_link, &needed );
	}

        Message_Dec_Refcount(msg);
	if( !needed ) Sess_dispose_message( mess_link );
}

void	Sess_deliver_reg_memb( configuration reg_memb, membership_id reg_memb_id )
{
	G_handle_reg_memb( reg_memb, reg_memb_id );
}

void	Sess_deliver_trans_memb( configuration trans_memb, membership_id trans_memb_id )
{
	G_handle_trans_memb( trans_memb, trans_memb_id );
}

static	void	Sess_handle_join( message_link *mess_link )
{
	message_header	*join_head;
	char		*group_name;

	join_head = Message_get_message_header(mess_link->mess);
	group_name = Message_get_first_group(mess_link->mess);

	G_handle_join( join_head->private_group_name, group_name );

	Sess_dispose_message( mess_link );

}

static	void	Sess_handle_leave( message_link *mess_link )
{
	message_header	*leave_head;
	char		*group_name;

	leave_head = Message_get_message_header(mess_link->mess);
	group_name = Message_get_first_group(mess_link->mess);

	G_handle_leave( leave_head->private_group_name, group_name );

	Sess_dispose_message( mess_link );
}

static	void	Sess_handle_kill( message_link *mess_link )
{
	char		private_name[MAX_PRIVATE_NAME+1];
	char		proc_name[MAX_PROC_NAME];
	message_header	*kill_head;
	int		ses;
	int		ret;

	kill_head = Message_get_message_header(mess_link->mess);

	ret = G_private_to_names( kill_head->private_group_name, private_name, proc_name );
	if( ret < 0 ) Alarm( EXIT, "Sess_handle_kill: Illegal private name to kill %s\n", 
				kill_head->private_group_name );

	G_handle_kill( kill_head->private_group_name );

	if( strcmp( My.name, proc_name ) == 0 )
	{
		/* my machine, we need to find the session */
		ses = Sess_get_session( private_name );
		if( ses >= 0 )
		{
			/* delete session ses */
			if( Is_op_session( Sessions[ses].status ) )
				Alarm( EXIT, "Sess_handle_kill: killing unkilled session bug!\n");
			Sess_unhash_session (&Sessions[ses]);
                        Prot_Destroy_Local_Session(&(Sessions[ses]) );
			Sess_remove_session (&Sessions[ses]);
			Num_sessions--;
			GlobalStatus.num_sessions = Num_sessions;
		}
	}
        Prot_kill_session(mess_link->mess);

	Sess_dispose_message( mess_link );
}

void	Sess_dispose_message( message_link *mess_link )
{
        Message_dispose_message(mess_link->mess);
	dispose( mess_link );
}

int	Sess_get_session( char *name )
{
	int	ret;
	session	*ses;

	for( ses = Sessions_head; ses; ses = ses->sort_next )
	{
		ret = strcmp( ses->name, name );
		if( ret <  0 ) continue;
		if( ret == 0 ) return( ses - Sessions );
		if( ret >  0 ) return( -1 );
	}
	return( -1 );
}

void    Flip_mess( message_header *head_ptr )
{
	head_ptr->type		= Flip_int32( head_ptr->type );
	head_ptr->num_groups	= Flip_int32( head_ptr->num_groups );
	head_ptr->data_len	= Flip_int32( head_ptr->data_len );
}

