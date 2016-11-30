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


#ifndef INC_MESSAGE
#define INC_MESSAGE

#include "arch.h"
#include "prot_objs.h"
#include "scatter.h"
#include "session.h"

int32           Obj_Inc_Refcount(void *obj);

void            Message_populate_with_buffers(message_obj *msg);
message_header  *Message_get_message_header(message_obj *msg);
int             Message_get_header_size();
int             Message_get_data_fragment_len();
scatter         *Message_get_data_scatter(message_obj *msg);
int             Message_get_packet_type(int mess_type);
char            *Message_get_first_data_ptr(message_obj *msg);
int             Message_get_first_data_len(message_obj *msg);
char            *Message_get_first_group(message_obj *msg);
char            *Message_get_groups_array(message_obj *msg);
int             Message_get_data_header_size(void);
int             Message_get_non_body_header_size();

void            Message_calculate_current_location(message_obj  *msg, int len_sent, struct partial_message_info *cur_msg);
void            Message_reset_current_location(struct partial_message_info *cur_msg);
void            Message_set_location_begin_body(struct partial_message_info *cur_msg);

void            Message_add_scat_element(message_obj *msg);
void            Message_remove_scat_element(message_obj *msg);

message_obj     *Message_dup_and_reset_old_message(message_obj *msg, int len);
message_obj     *Message_copy_message(message_obj *msg);
message_obj     *Message_new_message(void);
message_obj     *Message_create_message(int mess_type, char *sender_name);

void            Message_endian_correct_message_header(message_obj *msg);
int             Message_kill_mess_fixup(message_obj *msg, int orig_len, int mbox);
void            Message_element_len_fixup(message_obj *msg);
void            Message_Set_Fragment_Fields(message_obj *msg);
void            Message_Buffer_to_Message_Fragments( message_obj *msg, char buf[], int num_bytes );
void            Message_add_oldtype_to_reject( message_obj *msg, int32u old_type );

void            Message_dispose_message(message_obj *msg);
void            Message_Dec_Refcount(message_obj *msg);

#endif  /* INC_MESSAGE */
