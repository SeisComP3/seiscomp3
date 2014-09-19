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


#ifndef INC_NETWORK
#define INC_NETWORK

#include "arch.h"
#include "scatter.h"
#include "configuration.h"

void	Net_init();
void	Net_set_membership( configuration memb );
void    Net_signal_conf_reload(void);

int	Net_bcast( sys_scatter *scat );
int     Net_queue_bcast(sys_scatter *scat);
int     Net_flush_bcast(void);
int	Net_scast( int16 seg_index, sys_scatter *scat );
int	Net_ucast( int32 proc_id, sys_scatter *scat );
int	Net_recv ( channel fd, sys_scatter *scat );
int	Net_send_token( sys_scatter *scat );
int	Net_recv_token( channel fd, sys_scatter *scat );
int	Net_ucast_token( int32 proc_id, sys_scatter *scat );
channel *Net_bcast_channel(void);
channel *Net_token_channel(void);
void    Net_num_channels(int *num_bcast, int *num_token);

void    Net_set_partition(int16 *new_partition);
void    Net_clear_partition(void);

#endif	/* INC_NETWORK */ 
