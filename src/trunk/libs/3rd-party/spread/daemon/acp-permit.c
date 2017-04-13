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

/* Implements the functions required by the struct acp_hooks in a basic way
 * that always allows access. This is will preserve the current behaivor
 * of Spread
 */

#include "arch.h"
#include "acm.h"
#include "session.h"
#include "spu_alarm.h"

ACM_ReturnVal permit_open_connection(char *user);
ACM_ReturnVal permit_open_monitor(char *user);
ACM_ReturnVal permit_join_group(char *member, char *group, void *dummy_token);
ACM_ReturnVal permit_leave_group(char *member, char *group, void *dummy_token);
ACM_ReturnVal permit_p2p_send(char *sender, int num_dests, char dest[][MAX_GROUP_NAME], int service_type, int16 mess_type);
ACM_ReturnVal permit_mcast_send(char *sender, int num_groups, char groups[][MAX_GROUP_NAME], int service_type, int16 mess_type);

static struct  acp_ops Permit_ops = {
    permit_open_connection,
    permit_open_monitor,
    permit_join_group,
    permit_leave_group,
    permit_p2p_send,
    permit_mcast_send,
};

void permit_init(void)
{
    /* Unknown */
    if (!Acm_acp_add_method("PERMIT", &Permit_ops) )
    {
            Alarm( EXIT, "permit_init: Failed to register NULL. Too many ACM methods registered. Recompile with larger limit.\n");
    }
}

ACM_ReturnVal  permit_open_connection(char *user)
{
        return(ACM_ACCESS_ALLOWED);
}
ACM_ReturnVal  permit_open_monitor(char *user)
{
        return(ACM_ACCESS_ALLOWED);
}
ACM_ReturnVal permit_join_group(char *member, char *group, void *dummy_token)
{
	return(ACM_ACCESS_ALLOWED);
}
ACM_ReturnVal permit_leave_group(char *member, char *group, void *dummy_token)
{
        return(ACM_ACCESS_ALLOWED);
}
ACM_ReturnVal permit_p2p_send(char *sender, int num_dests, char dests[][MAX_GROUP_NAME], int service_type, int16 mess_type)
{
	return(ACM_ACCESS_ALLOWED);
}
ACM_ReturnVal permit_mcast_send(char *sender, int num_groups, char groups[][MAX_GROUP_NAME], int service_type, int16 mess_type)
{
	return(ACM_ACCESS_ALLOWED);
}
