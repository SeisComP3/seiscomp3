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


#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "message.h"
#include "spu_memory.h"
#include "net_types.h"
#include "spu_objects.h"
#include "prot_objs.h"
#include "spu_alarm.h"
#include "session.h"
#include "spread_params.h"

static  char Temp_buf[100000];

int32   Obj_Inc_Refcount(void *obj)
{
        return(0);
}

void    Message_populate_with_buffers(message_obj *msg)
{
        int i;
        msg->num_elements = MAX_SCATTER_ELEMENTS;
	for( i=0; i < msg->num_elements; i++ )
	{
		msg->elements[i].len = sizeof(packet_body);
		msg->elements[i].buf = (char *) new(PACKET_BODY);
                if (msg->elements[i].buf == NULL) {
                        Alarm(EXIT, "Message_populate_with_buffers: Failed to allocate a new PACKET_BODY object\n");
                        return;
                }
        }
}

message_header *Message_get_message_header(message_obj *msg)
{
        return( (message_header *)msg->elements[0].buf);
}

int     Message_get_header_size()
{
        return(sizeof(message_header) );
}

int     Message_get_non_body_header_size()
{
        return( 0 );
}

int     Message_get_data_fragment_len()
{
        return(sizeof(packet_body) );
}
scatter *Message_get_data_scatter(message_obj *msg)
{
        return((scatter *)msg);
}

/* This is what sets which message type a message shold be sent as */
int     Message_get_packet_type(int mess_type)
{
        if( Is_unreliable_mess( mess_type ) ) return(RELIABLE_TYPE);
	if( Is_reliable_mess( mess_type ) ) return(RELIABLE_TYPE);
	if( Is_fifo_mess( mess_type ) ) return(FIFO_TYPE);
        if( Is_causal_mess( mess_type )	||
		 Is_agreed_mess( mess_type ) ) return(AGREED_TYPE);
	if( Is_safe_mess( mess_type) ) return(SAFE_TYPE);
	if( Is_join_mess( mess_type) ) return(AGREED_TYPE);
	if( Is_leave_mess( mess_type) ) return(AGREED_TYPE);
	if( Is_kill_mess( mess_type) ) return(AGREED_TYPE);
        return(-1);
}

char    *Message_get_first_data_ptr(message_obj *msg)
{
        message_header *head_ptr;
        head_ptr = Message_get_message_header(msg);
        if (Is_reject_mess(head_ptr->type)) 
        {
                return(&(msg->elements[1].buf[0]) );
        } else {
                return(&(msg->elements[0].buf[sizeof(message_header)]) );
        }
}

int     Message_get_first_data_len(message_obj *msg)
{
        message_header *head_ptr;
        head_ptr = Message_get_message_header(msg);
        if (Is_reject_mess(head_ptr->type)) 
        {
            /* the scat->element[0] has NOTHING in it for data here. 
             * just return 0 and the actual body will be sent by the
             * main loop from elemnt 1..n
             */
                return(0);
        } else {
                return(msg->elements[0].len - sizeof(message_header) );
        }
}

int     Message_get_data_header_size(void)
{
        return( sizeof(message_header) );
}

char    *Message_get_first_group(message_obj *msg)
{
        message_header *head_ptr;
        head_ptr = Message_get_message_header(msg);
        if (Is_reject_mess(head_ptr->type)) 
        {
                return( &(msg->elements[1].buf[4]) );
        } else {
                return( &(msg->elements[0].buf[sizeof(message_header)]) );
        }
}
char    *Message_get_groups_array(message_obj *msg)
{
	int		groups_bytes, num_bytes, first_scat_index, groups_start_location;
        int             i;
        message_header *head_ptr;
        scatter         *scat;

        head_ptr = Message_get_message_header(msg);
        scat = Message_get_data_scatter(msg);
        if (Is_reject_mess(head_ptr->type)) 
        {
                groups_bytes = 4 + (head_ptr->num_groups * MAX_GROUP_NAME);
                groups_start_location = 4;
                first_scat_index = 1;
        } else {
                groups_bytes = sizeof(message_header) + (head_ptr->num_groups * MAX_GROUP_NAME);
                groups_start_location = sizeof(message_header);
                first_scat_index = 0;
        }
        /* if groups array didn't fit in first scat element, then copy it into Temp_buf */
	if( groups_bytes > scat->elements[first_scat_index].len )
	{
		num_bytes = 0;
		for( i=first_scat_index; i < scat->num_elements && num_bytes < groups_bytes; i++ )
		{ 
                    int copy_bytes;
                    if (groups_bytes - num_bytes < scat->elements[i].len)
                        copy_bytes = groups_bytes - num_bytes;
                    else 
                        copy_bytes = scat->elements[i].len;
                    memcpy( &Temp_buf[num_bytes], scat->elements[i].buf, copy_bytes );
                    num_bytes += copy_bytes;
		}
                assert(num_bytes == groups_bytes);
		return(&Temp_buf[0]);
	} else 	
                return(&(scat->elements[first_scat_index].buf[groups_start_location]));
}

/* STORE old_type at beginning of data before groups list 
 * so Sess_write() can write it for reject messages */
/* FIXME: This is totally broken in Spread 4 */
void    Message_add_oldtype_to_reject( message_obj *msg, int32u old_type )
{
        int i;
        int             old_byte_location;
        int             new_byte_location;
        scatter         *scat;
        message_header  *head_ptr;

        head_ptr = Message_get_message_header(msg);
        scat    = Message_get_data_scatter(msg);

        for (i=scat->num_elements; i >1; i--)
        {
                scat->elements[i].len = scat->elements[i-1].len;
                scat->elements[i].buf = scat->elements[i-1].buf;
        }
        scat->elements[1].buf = (char *)new( PACKET_BODY );
        if (scat->elements[1].buf == NULL) {
                Alarm(EXIT, "Sess_create_reject_mess: Failed to allocate a new PACKET_BODY object\n");
                return;
        }
        scat->num_elements++;
        old_byte_location = sizeof(message_header);
        new_byte_location = 0;
        memcpy(&scat->elements[1].buf[new_byte_location], &old_type, 4);
        new_byte_location += 4;
        memcpy(&scat->elements[1].buf[new_byte_location], &scat->elements[0].buf[old_byte_location], head_ptr->num_groups * MAX_GROUP_NAME );
        old_byte_location += (head_ptr->num_groups * MAX_GROUP_NAME);
        if ( head_ptr->num_groups < MAX_GROUPS_PER_MESSAGE )
        {
            new_byte_location += (head_ptr->num_groups * MAX_GROUP_NAME);
            head_ptr->num_groups = head_ptr->num_groups + 1; 
        } else 
        {
            /* overwrite last group name to make sure we have the rejected sender in list */
            new_byte_location += ( (head_ptr->num_groups - 1) * MAX_GROUP_NAME);
        }
        /* Add sender to groups list */
        memcpy(&scat->elements[1].buf[new_byte_location], head_ptr->private_group_name, MAX_GROUP_NAME);
        new_byte_location += MAX_GROUP_NAME;
        /* now copy the rest of the message body from first old scatter to new scatter 1 */
        memcpy(&scat->elements[1].buf[new_byte_location], &scat->elements[0].buf[old_byte_location], scat->elements[0].len - old_byte_location);
        new_byte_location += (scat->elements[0].len - old_byte_location);

        scat->elements[1].len = new_byte_location;
        scat->elements[0].len = sizeof(message_header);
        return;
}

void    Message_Dec_Refcount(message_obj *msg)
{
        /* Not needed for Spread3 */
        return;
}

void    Message_dispose_message(message_obj *msg)
{
	packet_body	*body_ptr;
	int		i;

	for( i=0; i < msg->num_elements; i++ )
	{
		body_ptr = (packet_body *)msg->elements[i].buf;
		dispose( body_ptr );
	}
	dispose( msg );
}

message_obj     *Message_dup_and_reset_old_message(message_obj *msg, int len)
{
        message_obj     *mess_dup;
        int             i,remain;

        mess_dup = new( SCATTER );
        if (mess_dup == NULL) {
                Alarm(EXIT, "Message_dup_message: Failed to allocate a new SCATTER object\n");
                return(NULL);
        }
	for( i=0, mess_dup->num_elements=0, remain=len ; remain > 0 ; i++, remain -= sizeof(packet_body) )
	{
		mess_dup->elements[i].buf = msg->elements[i].buf;
		if( remain > sizeof(packet_body) )
                {
			mess_dup->elements[i].len = sizeof( packet_body );
		}else{
			mess_dup->elements[i].len = remain;
                }
                mess_dup->num_elements++;

		msg->elements[i].buf = (char *)new( PACKET_BODY );
                if (msg->elements[i].buf == NULL) {
                        Alarm(EXIT, "Message_dup_message: Failed to allocate a new PACKET_BODY object\n");
                        return(NULL);
                }
                msg->elements[i].len = sizeof( packet_body );
        }
        return(mess_dup);
}

message_obj     *Message_copy_message(message_obj *msg)
{
        int i;

        message_obj *tmp_scat;
        tmp_scat = new( SCATTER );
        tmp_scat->num_elements = msg->num_elements;
	for( i=0; i < msg->num_elements; i++ )
        {
		tmp_scat->elements[i].len = msg->elements[i].len;
		tmp_scat->elements[i].buf = (char *)new( PACKET_BODY );
                if (tmp_scat == NULL) {
                        Alarm(EXIT, "Message_copy_message: Failed to allocate a new PACKET_BODY object\n");
                        return(NULL);
                }
		memcpy( tmp_scat->elements[i].buf, msg->elements[i].buf, msg->elements[i].len );
        }
        return(tmp_scat);
}

/* At the beginning of this function msg should be a scatter of one element. The beginning
 * of that buffer should contain an valid message_header structure. The buf and num_bytes fields
 * passed into this function should be everything for the message that goes AFTER the message_header.
 * Thus the first element is handled specially.
 */
void            Message_Buffer_to_Message_Fragments( message_obj *msg, char buf[], int num_bytes )
{
	scatter	        *scat;
	int		bytes_to_copy, copied_bytes;
	int		i, head_size;

	scat = Message_get_data_scatter(msg);
        head_size = Message_get_data_header_size();
	scat->num_elements = (num_bytes + head_size -1)/sizeof( packet_body ) + 1;

        if (scat->num_elements > MAX_SCATTER_ELEMENTS) {
            /* Message is too large */
            Alarmp(SPLOG_FATAL, PROTOCOL, "Message_Buffer_to_Message_Fragments: Number of scatter elements required for message (%d) is larger then the compiled in MAX_SCATTER_ELEMENTS (%d)\n", scat->num_elements, MAX_SCATTER_ELEMENTS);
            return;
        }
        copied_bytes = 0;
	for( i=0; i < scat->num_elements ; i++ )
	{
                if ( 0 == i ) {
                        bytes_to_copy = num_bytes;
                        if( bytes_to_copy > ( sizeof( packet_body ) - head_size ) )
                                bytes_to_copy = sizeof( packet_body ) - head_size ;
                        memcpy( &scat->elements[i].buf[head_size], &buf[ 0 ], bytes_to_copy );
                        scat->elements[i].len += bytes_to_copy;
                        copied_bytes += bytes_to_copy;
                } else {
                        bytes_to_copy = num_bytes - copied_bytes;
                        if( bytes_to_copy > sizeof( packet_body ) )
                                bytes_to_copy = sizeof( packet_body );
                        scat->elements[i].buf = (char *)new( PACKET_BODY );
                        memcpy( scat->elements[i].buf, &buf[ copied_bytes ], bytes_to_copy );
                        scat->elements[i].len = bytes_to_copy;
                        copied_bytes += bytes_to_copy;
                }
	}
        return;
}

void    Message_element_len_fixup(message_obj *msg)
{
        int total_size, sum_size, last;
        message_header *head_p;

        head_p = Message_get_message_header(msg);
        total_size = head_p->data_len + MAX_GROUP_NAME * head_p->num_groups + sizeof(message_header);
        sum_size = sizeof(packet_body) * (msg->num_elements - 1);
        last = total_size / sizeof(packet_body);
        if (total_size < sizeof(packet_body))
        {
                assert(last == 0);
                msg->elements[last].len = total_size;
        }
        else
        {
                msg->elements[0].len = sizeof(packet_body);
                msg->elements[last].len = total_size - sum_size;
        }
        return;
}

int     Message_kill_mess_fixup(message_obj *msg, int orig_len, int mbox)
{
        /* Don't need to do anything for spread3 */
        return(orig_len);
}

void    Message_reset_current_location(struct partial_message_info *cur_msg)
{
        cur_msg->cur_element   = 0;
        cur_msg->cur_byte      = 0;
        cur_msg->in_mess_head  = 0;
        cur_msg->total_bytes   = 0;
}

void    Message_set_location_begin_body(struct partial_message_info *cur_msg)
{
        /* do nothing really for spread3 */
        cur_msg->in_mess_head = 0;
}

void    Message_add_scat_element(message_obj *msg)
{
        int cur_num_elements;

        assert (msg->num_elements < MAX_SCATTER_ELEMENTS);

        cur_num_elements = msg->num_elements;
        msg->elements[cur_num_elements].buf = (char *) new(PACKET_BODY);
        if (msg->elements[cur_num_elements].buf == NULL) {
                Alarm(EXIT, "Message_add_scat_element: Failed to allocate a new PACKET_BODY\n");
                return;
        }
        msg->elements[cur_num_elements].len = sizeof(packet_body);
        msg->num_elements++;
}

void    Message_remove_scat_element(message_obj *msg)
{

        assert ( msg->num_elements > 0 );

        dispose( msg->elements[msg->num_elements - 1].buf );
        msg->elements[msg->num_elements - 1].len = 0;
        msg->num_elements--;
}

void    Message_calculate_current_location(message_obj  *msg, int len_sent, struct partial_message_info *cur_msg)
{
        int i, len;
	len = len_sent;
	for( i=0; i < msg->num_elements; i++ )
	{
		if( len < msg->elements[i].len ) break;
		len -= msg->elements[i].len;
	}
        cur_msg->cur_element = i;
        cur_msg->cur_byte = len;
        cur_msg->in_mess_head = 0;
}

message_obj     *Message_new_message(void)
{
        message_obj     *new_mess;

        new_mess = new( SCATTER );
        if (new_mess == NULL) {
                Alarm(EXIT, "Message_new_message: Failed to allocate a new SCATTER\n");
                return(NULL);
        }
	new_mess->num_elements = 1;
	new_mess->elements[0].len = sizeof(message_header);
	new_mess->elements[0].buf = (char *) new(PACKET_BODY);
        if (new_mess->elements[0].buf == NULL) {
                Alarm(EXIT, "Message_new_message: Failed to allocate a new PACKET_BODY\n");
                dispose(new_mess);
                return(NULL);
        }
        return(new_mess);
}

message_obj     *Message_create_message(int mess_type, char *sender_name)
{
        message_obj     *new_mess;
        message_header  *new_head;

        new_mess = Message_new_message();
	new_head = (message_header *)new_mess->elements[0].buf;

	new_head->type = mess_type;
	/* Setting endian to my endian on the header */
	new_head->type = Set_endian( new_head->type );
	new_head->hint = Set_endian( 0 );
	strncpy( new_head->private_group_name, sender_name, MAX_GROUP_NAME);
	new_head->num_groups = 0;
	new_head->data_len = 0;
        return(new_mess);
}

void    Message_endian_correct_message_header(message_obj *msg)
{
        message_header  *head_ptr;
        head_ptr = (message_header *) msg->elements[0].buf;
        /* Fliping message header to my form if needed */
	if( !Same_endian( head_ptr->type ) ) 
	{
		Flip_mess( head_ptr );
	}
}
