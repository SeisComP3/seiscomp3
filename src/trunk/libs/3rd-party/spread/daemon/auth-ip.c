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

/* Implements the functions required by the struct acm_hooks in a basic way
 * that allows access depending on the IP address (or local unix socket)
 * of the client. 
 *
 * This works on an implicit DENY ALL, and only those ip's listed in the
 * spread.access_ip file can connect.
 */

#include "arch.h"
#include "acm.h"
#include "session.h"
#include "sess_body.h" /* for Sessions[] */
#include "spu_alarm.h"

#include <stdlib.h>
#include <string.h>

/* ACM callbacks */
void ip_auth_client_connection(struct session_auth_info *sess_auth_p);
void ip_auth_monitor_connection(mailbox mbox, int32 ip_addr);

/* internal utility functions */
static void insert_ip_rule(int32u net_address, int prefix);

static struct  auth_ops IP_ops = {
    ip_auth_client_connection,
    ip_auth_monitor_connection,
    NULL /* deliver_authinfo */
};
struct ip_rule {
    int32u      network_address;
    int         prefix_length;
    struct ip_rule *next;
};

static struct ip_rule *Allow_Rules;
static bool IP_File_Loaded = FALSE;

void ip_init(void)
{
    int32u localhost_ip, net_address;
    char        file_name[80];
    FILE        *fp;
    int         i1,i2,i3,i4, prefix;
    char        line[132];
    char        *ret;
    int         iret, i;
    bool        file_done = FALSE;

    sprintf(file_name, "spread.access_ip");

    if (!Acm_auth_add_method("IP", &IP_ops))
    {
        Alarmp( SPLOG_FATAL, ACM, "ip_init: Failed to register IP. Too many ACM methods registered. Recompile with larger limit.\n");
    }

    /* load spread.access_ip file */
    if (NULL != (fp = fopen(file_name,"r")) )
        Alarmp( SPLOG_INFO, ACM, "ip_init: using file: %s\n", file_name);
    if (fp == NULL) 
        if (NULL != (fp = fopen("./spread.access_ip", "r")) )
            Alarmp( SPLOG_INFO, ACM, "ip_init: using file: ./spread.access_ip\n");
    if (fp == NULL)
        if (NULL != (fp = fopen("/etc/spread.access_ip", "r")) )
            Alarmp( SPLOG_INFO, ACM, "ip_init: using file: /etc/spread.access_ip\n");
    if (fp == NULL)
    {
        Alarmp( SPLOG_ERROR, ACM, "ip_init: IP access control file not found.\nIf you are using IP based access controls, please make sure the file exists.\n");
        IP_File_Loaded = FALSE;
        return;
    }
    do{
        ret = fgets(line,132,fp);
        if (ret == NULL) 
            break;
        if ( line[0] == '#')
            continue;

        if ( line[0] == 'u' && line[1] == 'n' && line[2] == 'i' && line[3] == 'x')
        {
            /* Special rule for unix domain sockets */
            Alarmp( SPLOG_INFO, ACM, "ip_init: Allowing UNIX Domain Socket connections\n");
            insert_ip_rule(0, 32);
            continue;
        }
        if ( line[0] == 'l' && line[1] == 'o' && line[2] == 'c' && line[3] == 'a' && line[4] == 'l')
        {
            /* special rule for localhost connections over tcp */
            Alarmp( SPLOG_INFO, ACM, "ip_init: Allowing localhost tcp Socket connections\n");
            localhost_ip = 127 << 24;
            insert_ip_rule(localhost_ip, 8);
            continue;
        }
        for(i=0; i< 3; i++)
        {
            ret = strchr(line, '.' );
            if ( ret == NULL)
            {
                Alarmp( SPLOG_ERROR, ACM, "ip_init: incomplete line: %s\n", line);
                file_done = TRUE;
                break;
            }
            *ret = ' ';
        }
        if (file_done) break;

        iret = sscanf(line,"%d%d%d%d/%d",
                     &i1,&i2,&i3,&i4,&prefix);
        if( iret == 4 ) prefix = 32;
        else if( iret < 5 ) 
            Alarmp( SPLOG_FATAL, ACM, "ip_init: not a valid ip network address/prefix line: %s\n", line);

        Alarmp( SPLOG_INFO, ACM, "ip_init: network address %d.%d.%d.%d with prefix length: %d\n",
               i1,i2,i3,i4, prefix);

        net_address = ( (i1 << 24 ) | (i2 << 16) | (i3 << 8) | i4 );

        insert_ip_rule(net_address, prefix);
    } while(TRUE);

    fclose(fp);
    IP_File_Loaded = TRUE;
}

static void insert_ip_rule(int32u net_address, int prefix)
{
    struct ip_rule *new_rule;
    
    new_rule = malloc(sizeof(struct ip_rule));
    if (!new_rule)
        Alarmp(SPLOG_FATAL, ACM, "insert_ip_rule: Failed to allocate a struct ip_rule\n");

    new_rule->network_address = net_address;
    new_rule->prefix_length = prefix;
    new_rule->next = NULL;

    if (!Allow_Rules)
    {
        Allow_Rules = new_rule;
        return;
    }
    new_rule->next = Allow_Rules;
    Allow_Rules = new_rule;
}
void ip_auth_client_connection(struct session_auth_info *sess_auth_p)
{
    int32u client_ip, client_net;
    struct ip_rule *rule_p;
    bool allowed;
    int ses;

    if (! IP_File_Loaded )
    {
        Alarmp( SPLOG_CRITICAL, ACM,  "ip_open_connection: No spread.access_ip file loaded. NO connections will be allowed!\nYou probably don't want this!!\n");
        Sess_session_report_auth_result( sess_auth_p, FALSE );
        return;
    }
    ses = Sess_get_session_index (sess_auth_p->mbox);
    client_ip = Sessions[ses].address;
    rule_p = Allow_Rules;
    allowed = FALSE;
    /* Search allowed lists */
    while(rule_p)
    {
        Alarmp(SPLOG_INFO, ACM, "ip_open_connection: client_ip: %d.%d.%d.%d, prefix: %d premask: 0x%x mask: 0x%x\n",
              IP1(client_ip), IP2(client_ip), IP3(client_ip), IP4(client_ip), rule_p->prefix_length, ~0x0,
              ( (~0x0) << (32 - rule_p->prefix_length)) );
        client_net = (client_ip & ( (~0x0) << (32 - rule_p->prefix_length)));
        Alarmp(SPLOG_INFO, ACM, "ip_open_connection: comparing network %d.%d.%d.%d with client %d.%d.%d.%d\n",
              IP1(rule_p->network_address), IP2(rule_p->network_address), IP3(rule_p->network_address), 
              IP4(rule_p->network_address), IP1(client_net),IP2(client_net),IP3(client_net),IP4(client_net) );
        if (rule_p->network_address == client_net)
        {
            allowed = TRUE;
            break;
        }
        rule_p = rule_p->next;
    }
    if (allowed)
        Sess_session_report_auth_result( sess_auth_p, TRUE );
    else 
        Sess_session_report_auth_result( sess_auth_p, FALSE );
}

void ip_auth_monitor_connection(mailbox mbox, int32 ip_addr)
{
    /*	Mon_Connection_Allowed(); */
}
