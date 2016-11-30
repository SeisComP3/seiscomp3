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

/* Implements the functions required by the struct auth_hooks in a basic way
 * that authenticates users based on a clear-text password that they send to 
 * the daemon.
 *
 * This works on an implicit DENY ALL, and only those users listed in the
 * spread.password file can connect.
 */

#include "arch.h"
#include "acm.h"
#include "session.h"
#include "sess_body.h" /* for Sessions[] */
#include "alarm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef ARCH_PC_WIN95

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef  ARCH_SPARC_SOLARIS
#include <sys/uio.h>
#endif

#include <sys/ioctl.h>

#else   /* ARCH_PC_WIN95 */

#include <winsock.h>
#define	ioctl 	ioctlsocket

#endif  /* ARCH_PC_WIN95 */

#define MAX_PWORD_USERNAME 32
#define MAX_PWORD_PASSWORD 8
#define MAX_PWORD_CRYPTPASSWORD 13

struct user_password {
    char username[MAX_PWORD_USERNAME + 1];
    char crypt_pass[MAX_PWORD_CRYPTPASSWORD + 1];
    struct user_password *next;
};

/* ACM callbacks */
void pword_auth_client_connection(struct session_auth_info *sess_auth_p);
void pword_auth_monitor_connection(mailbox mbox, int32 ip_addr);

/* internal utility functions */
static void insert_user(char *username, char *crypt_password);
static bool lookup_user(char *username, struct user_password **user_h);
static bool check_password(char *username, char *clear_password);
static void auth_client_conn_read(int fd, int dummy, struct session_auth_info *sess_auth_p);

static struct  auth_ops Pword_ops = {
    pword_auth_client_connection,
    pword_auth_monitor_connection,
    NULL /* deliver_authinfo */
};

static struct user_password *Users;

void pword_init(void)
{
    char        file_name[80];
    FILE        *fp;
    char        username[MAX_PWORD_USERNAME + 1];
    char        password[MAX_PWORD_CRYPTPASSWORD + 1];
    char        line[132];
    char        *ret;
    int         iret;
    bool        file_done = FALSE;

    sprintf(file_name, "spread.access_pword");

    Alarmp( SPLOG_DEBUG, ACM, "pword_init: Starting\n");
    if (!Acm_auth_add_method("PWORD", &Pword_ops))
    {
        Alarmp( SPLOG_FATAL, ACM, "pword_init: Failed to register PWORD. Too many ACM methods registered. Recompile with larger limit.\n");
    }

    /* load spread.access_ip file */
    if (NULL != (fp = fopen(file_name,"r")) )
        Alarmp( SPLOG_INFO, ACM, "pword_init: using file: %s\n", file_name);
    if (fp == NULL) 
        if (NULL != (fp = fopen("./spread.access_pword", "r")) )
            Alarmp( SPLOG_INFO,ACM, "pword_init: using file: ./spread.access_pword\n");
    if (fp == NULL)
        if (NULL != (fp = fopen("/etc/spread.access_pword", "r")) )
            Alarmp( SPLOG_INFO, ACM, "pword_init: using file: /etc/spread.access_pword\n");
    if (fp == NULL)
        Alarmp( SPLOG_FATAL, ACM, "pword_init: error opening config file %s in any of the standard locations. Please make sure the file exists\n", file_name);

    do{
        ret = fgets(line,132,fp);
        if (ret == NULL) 
            break;
        if ( line[0] == '#')
            continue;

        ret = strchr(line, ':' );

        if ( ret == NULL)
        {
            Alarmp( SPLOG_ERROR, ACM, "pword_init: incomplete line: %s\n", line);
                file_done = TRUE;
                break;
        }
        *ret = ' ';
        if (file_done) break;

        iret = sscanf(line,"%32s %13s",username,password);
        if( iret < 2 ) 
            Alarmp(SPLOG_FATAL, ACM, "pword_init: not a valid username:password entry. line: %s\n", line);

        Alarmp( SPLOG_INFO, ACM, "pword_init: loaded user %s with crypted password %s\n",username, password);

        insert_user(username, password);
    } while(TRUE);

    fclose(fp);
}

static void insert_user(char *username, char *crypt_password)
{
    struct user_password *new_user;
    
    new_user = malloc(sizeof(struct user_password));
    if (!new_user)
        Alarmp(SPLOG_FATAL, ACM, "insert_user: Failed to allocate a struct user_password\n");

    memcpy(new_user->username, username, MAX_PWORD_USERNAME);
    memcpy(new_user->crypt_pass, crypt_password, MAX_PWORD_CRYPTPASSWORD);

    if (!Users)
    {
        Users = new_user;
        return;
    }
    new_user->next = Users;
    Users = new_user;
}

static bool lookup_user(char *username, struct user_password **user_h)
{
    struct user_password *user_p;
    bool allowed;

    user_p = Users;
    allowed = FALSE;
    /* Search allowed lists */
    while(user_p)
    {
        Alarmp( SPLOG_INFO, ACM, "lookup_user: Checking user: %s with crypted password %s\n", user_p->username, user_p->crypt_pass);
        if (!strncmp(username, user_p->username, MAX_PWORD_USERNAME) )
        {
            Alarmp( SPLOG_INFO, ACM, "lookup_user: Found user %s = %s with crypted password %s\n", username, user_p->username, user_p->crypt_pass);
            *user_h = user_p;
            return(TRUE);
        }
        user_p = user_p->next;
    }
    *user_h = NULL;
    return(FALSE);
}

static bool check_password(char *username, char *clear_password)
{
    struct user_password *user_p;
    char *crypt_presented_pass;
    char salt[2];

    /* Search allowed lists */
    if (lookup_user(username, &user_p))
    {
        memcpy(salt, user_p->crypt_pass, 2);
        crypt_presented_pass = crypt(clear_password, salt);
        if (!strncmp(crypt_presented_pass, user_p->crypt_pass, 13)) {
            return(TRUE);
        } else {
            Alarmp( SPLOG_WARNING, ACM, "pword_auth_client_connection: Password (%s) did NOT match (%s) for user %s\n", crypt_presented_pass, user_p->crypt_pass, username);
            return(FALSE);
        }
    } else {
        /* user not found */
        return(FALSE);
    }
}
void pword_auth_client_connection(struct session_auth_info *sess_auth_p)
{
    /* Get username and password from client library */
    E_attach_fd(sess_auth_p->mbox, READ_FD, (void (*)(int, int, void *)) auth_client_conn_read, 0, sess_auth_p, LOW_PRIORITY);
    E_attach_fd(sess_auth_p->mbox, EXCEPT_FD, (void (*)(int, int, void *)) auth_client_conn_read, 0, sess_auth_p, LOW_PRIORITY);
    return;
}

static void auth_client_conn_read(mailbox mbox, int dummy, struct session_auth_info *sess_auth_p)
{
    char username[MAX_PWORD_USERNAME + 1];
    char clear_password[MAX_PWORD_PASSWORD + 1];
    int ioctl_cmd, ret;
    unsigned char response;

    E_detach_fd(mbox, READ_FD);
    E_detach_fd(mbox, EXCEPT_FD);

#if 0
    /* temporarily disabled to test race bug */
    /* set file descriptor to non-blocking */
    ioctl_cmd = 1;
    ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
#endif
    ret = recv( mbox, username, MAX_PWORD_USERNAME, 0 );
    if( ret < 0 )
    {
        Alarmp( SPLOG_WARNING, ACM, "auth_client_conn_read: reading username string failed on mailbox %d\n", mbox );
        ioctl_cmd = 0;
        ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
        Sess_session_report_auth_result( sess_auth_p, FALSE);
        return;
    }
    if( ret < MAX_PWORD_USERNAME )
    {
        Alarmp( SPLOG_WARNING, ACM, "auth_client_conn_read: reading username string SHORT on mailbox %d\n", mbox );
        ioctl_cmd = 0;
        ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
        Sess_session_report_auth_result( sess_auth_p, FALSE);
        return;
    }
    username[MAX_PWORD_USERNAME] = '\0';

    ret = recv( mbox, clear_password, MAX_PWORD_PASSWORD, 0 );
    if( ret < 0 )
    {
        Alarmp( SPLOG_WARNING, ACM, "auth_client_conn_read: reading password string failed on mailbox %d\n", mbox );
        /* set blocking and return failure to auth */
        ioctl_cmd = 0;
        ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
        Sess_session_report_auth_result( sess_auth_p, FALSE);
        return;
    }
    if( ret < MAX_PWORD_PASSWORD )
    {
        Alarmp( SPLOG_WARNING, ACM, "auth_client_conn_read: reading password string SHORT on mailbox %d\n", mbox );
        ioctl_cmd = 0;
        ret = ioctl( mbox, FIONBIO, &ioctl_cmd);
        Sess_session_report_auth_result( sess_auth_p, FALSE);
        return;
    }
    clear_password[MAX_PWORD_PASSWORD] = '\0';

    /* set file descriptor back to blocking */
    ioctl_cmd = 0;
    ret = ioctl( mbox, FIONBIO, &ioctl_cmd);

    if ( check_password(username, clear_password) )
    {
            response = 1;
            send( mbox, &response, 1, 0); 
            Sess_session_report_auth_result( sess_auth_p, TRUE );
    } else {
            response = 0;
            send( mbox, &response, 1, 0); 
            Sess_session_report_auth_result( sess_auth_p, FALSE );
    }
    return;
}
void pword_auth_monitor_connection(mailbox mbox, int32 ip_addr)
{
    /*	Mon_Connection_Allowed(); */
}
