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


#include "arch.h"
#include <string.h>
#include <assert.h>

#include "acm.h"
#include "spu_alarm.h"
#include "spu_memory.h"
#include "spu_objects.h"

struct auth_info {
    char                name[MAX_AUTH_NAME];
    bool                enabled;
    bool                required;
    struct auth_ops     ops;
};

struct acp_info {
    char                name[MAX_AUTH_NAME];
    struct acp_ops      ops;
};

static struct auth_info Auth_Methods[MAX_AUTH_METHODS];
static  int             Num_Auth_Methods;

static struct acp_info  ACP_Methods[MAX_AUTH_METHODS];
static  int             Num_ACP_Methods;

static  int             AccessControlPolicy;


static int acm_authname_to_type(char *auth_name)
{
    int i;
    for ( i=0; i < Num_Auth_Methods; i++)
    {
        if (!strncmp(Auth_Methods[i].name, auth_name, MAX_AUTH_NAME))
            return( i );
    }
    return(-1);
}

static int acm_acpname_to_type(char *acp_name)
{
    int i;
    for ( i=0; i < Num_ACP_Methods; i++)
    {
        if (!strncmp(ACP_Methods[i].name, acp_name, MAX_AUTH_NAME))
            return( i );
    }
    return(-1);
}

void Acm_init()
{
        Mem_init_object( SESSION_AUTH_INFO, "session_auth_info", sizeof( struct session_auth_info ), 0, 0 );
        /* establish default authentication -- allow all connections if nothing is configured.
         * This is overridden by whatever is configured in the spread.conf file
         */
        Acm_auth_set_enabled("NULL");

        /* establish default policy if one is not configured */
        if (!Acm_acp_set_policy("PERMIT"))
                Alarm(EXIT, "Acm_init: Unable to establish default PERMIT policy. Spread build is broken\n");
}

struct session_auth_info *Acm_auth_create_sess_info_forIP(mailbox mbox)
{
        struct session_auth_info *sess_auth_p;

        sess_auth_p = new( SESSION_AUTH_INFO );
        if (sess_auth_p == NULL) 
        {
                Alarm( EXIT, "Acm_auth_create_sess_info_forIP: Failed to allocate an struct session_auth_info\n");
                return(NULL);
        }

        sess_auth_p->mbox = mbox;
        sess_auth_p->completed_required_auths = 0;
        sess_auth_p->num_required_auths = 1;
        sess_auth_p->required_auth_methods[0] = acm_authname_to_type("IP");
        sess_auth_p->required_auth_results[0] = 0;

        return(sess_auth_p);
}

struct session_auth_info *Acm_auth_create_sess_info(mailbox mbox, char *auth_name)
{
        int num_auths, i, j, type;
        struct session_auth_info *sess_auth_p;
        bool meth_registered;

        sess_auth_p = new( SESSION_AUTH_INFO );
        if (sess_auth_p == NULL) 
        {
                Alarm( EXIT, "Acm_auth_create_sess_info: Failed to allocate an struct session_auth_info\n");
                return(NULL);
        }

        sess_auth_p->mbox = mbox;
        sess_auth_p->completed_required_auths = 0;
        
        num_auths = 0;
        i = 0;
        /* add methods client requested */
        while( auth_name[i * MAX_AUTH_NAME] != '\0')
        {
                type = acm_authname_to_type( &auth_name[i * MAX_AUTH_NAME]);
                if ( Auth_Methods[type].enabled )
                {
                        sess_auth_p->required_auth_methods[num_auths] = type;
                        num_auths++;
                }
                i++;
        }
        if (num_auths == 0 )
        {
                Alarm( ACM, "Acm_auth_create_sess_info: Client on session mbox %d failed to request any valid methods: %s\n", mbox, auth_name);
                dispose(sess_auth_p);
                return(NULL);
        }
        /* Now add required methods that were not listed by client */
        for ( i = 0; i < Num_Auth_Methods; i++)
        {
                if ( Auth_Methods[i].required )
                {
                        for (meth_registered = FALSE, j=0; j < num_auths; j++)
                        {
                                if ( sess_auth_p->required_auth_methods[j] == i )
                                        meth_registered = TRUE;
                        }
                        if ( ! meth_registered ) 
                        {
                                sess_auth_p->required_auth_methods[num_auths] = i;
                                num_auths++;
                        }
                }
        }
        if (num_auths == 0 )
        {
                Alarm( ACM, "Acm_auth_create_sess_info: Failed to find any auth methods for session mbox %d which requested %s\n", mbox, auth_name);
                dispose(sess_auth_p);
                return(NULL);
        }
        sess_auth_p->num_required_auths = num_auths;
        for (i=0; i< MAX_AUTH_METHODS; i++)
        {
                sess_auth_p->required_auth_results[i] = 0;
        }
        return(sess_auth_p);
}
bool Acm_auth_set_enabled(char *auth_name)
{
    int auth_type;
    assert(auth_name != NULL);
    auth_type = acm_authname_to_type(auth_name);
    if (auth_type == -1)
    {
        Alarm( ACM, "Acm_auth_set_enabled: unknown auth name %s\n", auth_name);
        return( FALSE );
    }
    Auth_Methods[auth_type].enabled = TRUE;
    return( TRUE );
}
bool Acm_auth_set_disabled(char *auth_name)
{
    int auth_type;
    assert(auth_name != NULL);
    auth_type = acm_authname_to_type(auth_name);
    if (auth_type == -1)
    {
        Alarm( ACM, "Acm_auth_set_disabled: unknown auth name %s\n", auth_name);
        return( FALSE );
    }
    Auth_Methods[auth_type].enabled = FALSE;
    return( TRUE );
}

bool Acm_auth_set_required(char *auth_name)
{
    int auth_type;
    assert(auth_name != NULL);
    auth_type = acm_authname_to_type(auth_name);
    if (auth_type == -1)
    {
        Alarm( ACM, "Acm_auth_set_required: unknown auth name %s\n", auth_name);
        return( FALSE );
    }
    Auth_Methods[auth_type].required = TRUE;
    return( TRUE );
}

bool Acm_auth_add_method(char *name, struct auth_ops *ops)
{
    assert(name != NULL);
    assert(ops != NULL);

    if (Num_Auth_Methods == (MAX_AUTH_METHODS) )
        return(FALSE);
    memcpy(Auth_Methods[Num_Auth_Methods].name, name, MAX_AUTH_NAME);
    Auth_Methods[Num_Auth_Methods].enabled = FALSE;
    Auth_Methods[Num_Auth_Methods].required = FALSE;
    Auth_Methods[Num_Auth_Methods].ops = *ops;

    Num_Auth_Methods++;
    return(TRUE);
}

bool Acm_auth_query_allowed(char *auth_name)
{
    int auth_type;

    assert(auth_name != NULL);

    auth_type = acm_authname_to_type(auth_name);
    if (auth_type == -1)
    {
        Alarm(ACM, "Acm_query_alloweds: unknown auth name %s\n", auth_name);
        return(FALSE);
    }
    return(Auth_Methods[auth_type].enabled);
}

void *Acm_auth_get_auth_client_connection_byname(char *auth_name)
{
    int auth_type;

    assert(auth_name != NULL);

    auth_type = acm_authname_to_type(auth_name);
    if (auth_type == -1)
    {
        Alarm(ACM, "Acm_auth_get_auth_client_connection_byname: unknown auth name %s\n", auth_name);
        return(FALSE);
    }
    return(Acm_auth_get_auth_client_connection(auth_type));
}

void *Acm_auth_get_auth_client_connection(int authid)
{
    assert( authid >= 0 );
    assert( authid < MAX_AUTH_METHODS );

    return(Auth_Methods[authid].ops.auth_client_connection);
}

char *Acm_auth_get_allowed_list(void)
{
    int i;
    static char list[MAX_AUTH_LIST_LEN];
    char *c_ptr;

    c_ptr = list;
    for ( i=0; i < Num_Auth_Methods; i++)
    {
        if (Auth_Methods[i].enabled)
        {
            sprintf(c_ptr, "%s ", Auth_Methods[i].name);
            c_ptr += strlen(Auth_Methods[i].name);
            c_ptr++; /* for space */
        }
    }
    *c_ptr = 0; /* null terminate */
    return(list);
}

bool Acm_acp_add_method(char *name, struct acp_ops *ops)
{
    assert(name != NULL);
    assert(ops != NULL);

    if (Num_ACP_Methods == (MAX_AUTH_METHODS) )
        return(FALSE);
    memcpy(ACP_Methods[Num_ACP_Methods].name, name, MAX_AUTH_NAME);
    ACP_Methods[Num_ACP_Methods].ops = *ops;

    Num_ACP_Methods++;
    return(TRUE);
}

bool Acm_acp_query_allowed(char *acp_name)
{
    int acp_type;

    assert(acp_name != NULL);

    acp_type = acm_acpname_to_type(acp_name);
    if (acp_type == -1)
    {
        Alarm(ACM, "Acm_acp_query_alloweds: unknown acp name %s\n", acp_name);
        return(FALSE);
    }
    return(TRUE);
}

bool Acm_acp_set_policy(char *policy_name)
{
    if (!Acm_acp_query_allowed(policy_name))
        return(FALSE);
    AccessControlPolicy = acm_acpname_to_type(policy_name);
    return(TRUE);
}

bool Acm_acp_fill_ops_byname(char *acp_name, struct acp_ops *acp_ops_h)
{
    int acp_type;

    assert(acp_name != NULL);
    assert(acp_ops_h != NULL);

    acp_type = acm_acpname_to_type(acp_name);
    if (acp_type == -1)
    {
        Alarm(ACM, "Acm_fill_acp_ops: unknown access control policy name %s\n", acp_name);
        return(FALSE);
    }
    *acp_ops_h = ACP_Methods[acp_type].ops;
    return(TRUE);
}

void Acm_acp_fill_ops(struct acp_ops *acp_ops_h)
{
    assert(acp_ops_h != NULL);
    assert( AccessControlPolicy >= 0 );
    assert( AccessControlPolicy < Num_ACP_Methods );

    *acp_ops_h = ACP_Methods[AccessControlPolicy].ops;
    return;
}
