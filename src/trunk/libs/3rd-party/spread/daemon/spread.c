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

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "arch.h"
#include "spread_params.h"
#include "session.h"
#include "configuration.h"
#include "spu_events.h"
#include "status.h"
#include "log.h"
#include "sess_body.h"
#include "spu_alarm.h"

#include "stdutil/stdutil.h"

#ifndef ARCH_PC_WIN95
#  include <grp.h>
#  include <pwd.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#ifdef	ARCH_PC_WIN95
#  include	<winsock.h>
WSADATA		WSAData;
#endif	/* ARCH_PC_WIN95 */

static	char		*My_name;
static	char		My_name_buf[80];
static	char		Config_file[512];
static	int		Log;

static  const char            Spread_build_date[] = SPREAD_BUILD_DATE;

static	void	Invalid_privilege_decrease(char *user, char *group);
static	void	Usage(int argc, char *argv[]);

/* auth-null.c: */
void null_init(void);
/* auth-ip.c: */
void ip_init(void);
/* acp-permit.c: */
void permit_init(void);

static void Bye( void ) 
{
        Alarmp( SPLOG_PRINT, PRINT, "Spread daemon exiting normally!\n" );
}

static void E_exit_events_wrapper( int signum )
{
        E_exit_events_async_safe();
}

#ifdef USE_SPREAD_MAIN
int SpreadMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef	ARCH_PC_WIN95
	int	ret;
#endif

#ifndef ARCH_PC_WIN95
	struct group  *grp;
	struct passwd *pwd;
#endif

        atexit( Bye );
        signal( SIGINT,  E_exit_events_wrapper );
        signal( SIGTERM, E_exit_events_wrapper );

	Alarm_set_types( CONF_SYS ); 
        Alarm_set_priority( SPLOG_INFO );

	Alarmp( SPLOG_PRINT, SYSTEM, "/===========================================================================\\\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| The Spread Toolkit                                                        |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Copyright (c) 1993-2014 Spread Concepts LLC                               |\n"); 
	Alarmp( SPLOG_PRINT, SYSTEM, "| All rights reserved.                                                      |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| The Spread toolkit is licensed under the Spread Open-Source License.      |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| You may only use this software in compliance with the License.            |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| A copy of the license can be found at http://www.spread.org/license       |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| This product uses software developed by Spread Concepts LLC for use       |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| in the Spread toolkit. For more information about Spread,                 |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| see http://www.spread.org                                                 |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| This software is distributed on an \"AS IS\" basis, WITHOUT WARRANTY OF     |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| ANY KIND, either express or implied.                                      |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Creators:                                                                 |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|    Yair Amir             yairamir@cs.jhu.edu                              |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|    Michal Miskin-Amir    michal@spreadconcepts.com                        |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|    Jonathan Stanton      jonathan@spreadconcepts.com                      |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    John Schultz          jschultz@spreadconcepts.com                      |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Major Contributors:                                                       |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.     |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    Ryan Caudy           rcaudy@gmail.com - contribution to process groups.|\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    Claudiu Danilov      claudiu@acm.org - scalable, wide-area support.    |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    Cristina Nita-Rotaru crisn@cs.purdue.edu - GC security.                |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.  |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.             |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Special thanks to the following for discussions and ideas:                |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|    Ken Birman, Danny Dolev, Jacob Green, Mike Goodrich, Ben Laurie,       |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|    David Shaw, Gene Tsudik, Robbert VanRenesse.                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| Partial funding provided by the Defense Advanced Research Project Agency  |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| (DARPA) and the National Security Agency (NSA) 2000-2004. The Spread      |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "| toolkit is not necessarily endorsed by DARPA or the NSA.                  |\n");
        Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| For a full list of contributors, see Readme.txt in the distribution.      |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| WWW:     www.spread.org     www.spreadconcepts.com                        |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Contact: info@spreadconcepts.com                                          |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "|                                                                           |\n");
	Alarmp( SPLOG_PRINT, SYSTEM, "| Version %d.%02d.%02d Built %-17s                                   |\n", 
		(int)SP_MAJOR_VERSION, (int)SP_MINOR_VERSION, (int)SP_PATCH_VERSION, Spread_build_date );
	Alarmp( SPLOG_PRINT, SYSTEM, "\\===========================================================================/\n");

	Usage( argc, argv );

#ifdef	ARCH_PC_WIN95

	ret = WSAStartup( MAKEWORD(1,1), &WSAData );
	if( ret != 0 )
            Alarmp( SPLOG_FATAL, NETWORK, "Spread: winsock initialization error %d\n", ret );

#endif	/* ARCH_PC_WIN95 */

        /* initialize each valid authentication protocol */
        null_init();
        ip_init();
#ifdef  ENABLE_PASSWORD
        pword_init();
#endif
        permit_init();

        /* Initialize Access Control & Authentication */
        Acm_init();

	Conf_init( Config_file, My_name );

	E_init();
	
	{
	  sp_time t = E_get_time();

	  set_rand( (int) stdhcode_oaat( &t, sizeof( t ) ) ^ (int) stdhcode_oaat( &My.id, sizeof( My.id ) ) );
	}

#ifndef	ARCH_PC_WIN95
        /* Verify that unix socket dir is safe if runing as root user */
        if (geteuid() == (uid_t) 0) {
            struct stat usock_stat;
            Alarmp( SPLOG_INFO, SECURITY, "Spread is running as root so check file locations\n");
            if (stat( SP_UNIX_SOCKET, &usock_stat)) {
                Alarmp( SPLOG_FATAL, SECURITY, "Spread unable to stat the unix domain socket dir (%s). Please verify the selected directory and restart the daemon\n", SP_UNIX_SOCKET );
                exit( 0 );
            }

            if ( (usock_stat.st_mode & S_IWOTH) || !(usock_stat.st_uid == (uid_t) 0) )
                Alarmp( SPLOG_WARNING, PRINT, "Spread: SECURITY RISK! running as root, but unix domain socket is not in a root-only writable directory. May risk denial of service or malicious deletion of unexpected file in directory: %s\n", SP_UNIX_SOCKET );
	}
#endif

	Sess_init();

	Stat_init(); 
	if( Log ) Log_init();

#ifndef	ARCH_PC_WIN95

	/* Yupp, we're paranoid */
 
	if (geteuid() != (uid_t) 0) {
            Alarmp( SPLOG_WARNING, SECURITY, "Spread: not running as root, won't chroot\n" );
	}
	else if ( (grp = getgrnam(Conf_get_group())) == NULL
                  || (pwd = getpwnam(Conf_get_user())) == NULL ) {
            Invalid_privilege_decrease(Conf_get_user(), Conf_get_group());
	}
	else if (chdir(Conf_get_runtime_dir()) < 0
                  || chroot(Conf_get_runtime_dir()) < 0 ) {
            Alarmp( SPLOG_FATAL, SECURITY, "Spread: FAILED chroot to '%s'\n",
                   Conf_get_runtime_dir() );
	}
	else if ( setgroups(1, &grp->gr_gid) < 0
                  || setgid(grp->gr_gid) < 0
                  || setuid(pwd->pw_uid) < 0) {
            Invalid_privilege_decrease(Conf_get_user(), Conf_get_group());
	} else {
            Alarmp( SPLOG_INFO, SECURITY, "Spread: setugid and chroot successeful\n" );
	}

#endif	/* ARCH_PC_WIN95 */

	E_handle_events();

        Sess_fini();

	return 0;
}

static  void    Print_help(void)
{
    Alarmp( SPLOG_FATAL, SYSTEM, "Usage: spread\n%s\n%s\n%s\n",
           "\t[-l y/n]          : print log",
           "\t[-n <proc name>]  : force computer name",
           "\t[-c <file name>]  : specify configuration file" );
}


static	void	Invalid_privilege_decrease(char *user, char *group)
{
    Alarmp( SPLOG_FATAL, SECURITY, "Spread: FAILED privilege drop to user/group "
           "'%s/%s' (defined in spread.conf or spread_params.h)\n",
           user, group );
}

static	void	Usage(int argc, char *argv[])
{
	My_name = 0; /* NULL */
	Log	= 0;
	strcpy( Config_file, "spread.conf" );

	while( --argc > 0 )
	{
		argv++;

		if( !strncmp( *argv, "-n", 2 ) )
		{
                        if (argc < 2) Print_help();
			if( strlen( argv[1] ) > MAX_PROC_NAME-1 ) /* -1 for the null */
                              Alarmp( SPLOG_FATAL, SYSTEM, "Usage: proc name %s too long\n",
					argv[1] );

			memcpy( My_name_buf, argv[1], strlen( argv[1] ) );
			My_name = My_name_buf;

			argc--; argv++;

		}else if( !strncmp( *argv, "-c", 2 ) ){
                        if (argc < 2) Print_help();
			if (strlen(argv[1]) >= sizeof(Config_file)) {
				Alarmp(SPLOG_FATAL, SYSTEM, "Usage: config file name too long\n", argv[1]);
			}
			strcpy( Config_file, argv[1] );

			argc--; argv++;

		}else if( !strncmp( *argv, "-l", 2 ) ){
                        if (argc < 2) Print_help();
			if( !strcmp( argv[1], "y" ) )
				Log = 1;
			else if( !strcmp( argv[1], "n" ) )
				Log = 0;
			else Print_help();

			argc--; argv++;

		}else{
                        Print_help();
		}
	}
}
