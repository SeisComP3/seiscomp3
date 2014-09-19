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

/* Implements the functions required by the struct auth_hooks in a basic way
 * that always allows access. This is will preserve the current behaivor
 * of Spread
 */

#include "arch.h"
#include "acm.h"
#include "session.h"
#include "spu_alarm.h"

void null_auth_client_connection(struct session_auth_info *sess_auth_p);
void null_auth_monitor_connection(mailbox mbox, int32 ip_addr);

static struct  auth_ops Null_ops = {
    null_auth_client_connection,
    null_auth_monitor_connection,
    NULL /* deliver_authinfo */
};

void null_init(void)
{
    /* Unknown */
    if (!Acm_auth_add_method("NULL", &Null_ops) )
    {
        Alarmp( SPLOG_FATAL, ACM, "null_init: Failed to register NULL. Too many ACM methods registered. Recompile with larger limit.\n");
    }
}

void null_auth_client_connection(struct session_auth_info *sess_auth_p)
{
        /* report that we authenticated the session */
        Sess_session_report_auth_result( sess_auth_p, TRUE );
}
void null_auth_monitor_connection(mailbox mbox, int32 ip_addr)
{
    /*	Mon_Connection_Allowed(); */
}
