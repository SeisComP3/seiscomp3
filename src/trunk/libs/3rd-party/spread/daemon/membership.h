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


#ifndef INC_MEMBERSHIP
#define INC_MEMBERSHIP

#include "arch.h"
#include "scatter.h"
#include "configuration.h"

#define		OP		1
#define		SEG		2
#define		REPRESENTED	3
#define		GATHER		4
#define		FORM		5
#define		EVS		6


void		Memb_init();
configuration	*Memb_active_ptr();
membership_id	Memb_id();
membership_id	Memb_trans_id();
int		Memb_is_equal( membership_id m1, membership_id m2 );
int32		Memb_state();
int		Memb_token_alive();
void		Memb_handle_message( sys_scatter *scat );
void		Memb_handle_token( sys_scatter *scat );
void		Memb_token_loss();
void 	        Memb_lookup_new_members();
void            Memb_signal_conf_reload();

void		Memb_commit();
void		Memb_transitional();
void		Memb_regular();
void	        Memb_print_form_token( sys_scatter *scat );

#endif /* INC_MEMBERSHIP */
