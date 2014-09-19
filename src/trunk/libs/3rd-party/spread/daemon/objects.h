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
 *  Copyright (C) 1993-2006 Spread Concepts LLC <info@spreadconcepts.com>
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


/* objects.h 
 *  main declarations of objects
 * Copyright 1997 Jonathan Stanton <jonathan@cs.jhu.edu> 
 * Center for Networking and Distributed Systems
 *
 * $Id: objects.h 389 2006-11-30 16:22:44Z jonathan $
 *
 */

#ifndef OBJECTS_H
#define OBJECTS_H

#include "arch.h"
#include "spread_params.h"      /* For SPREAD_PROTOCOL used in memory.c */

#define MAX_OBJECTS             200
#define MAX_OBJ_USED            56

/* Object types 
 *
 * Object types must start with 1 and go up. 0 is reserved 
 */

#define BASE_OBJ                1
#define PACK_HEAD_OBJ           2       /* net_objs.h */
#define MESSAGE_OBJ             3       /* prot_objs.h */
#define MSG_FRAG_OBJ            4       /* prot_objs.h */
#define RET_REQ_OBJ             5       /* net_objs.h */
#define LINK_ACK_OBJ            6       /* net_objs.h */
#define ARU_UPDATE_OBJ          7       /* prot_objs.h */
#define TOKEN_HEAD_OBJ          8       /* net_objs.h */
#define TOKEN_BODY_OBJ          9       /* net_objs.h */
#define ALIVE_OBJ               10      /* memb_objs.h */
#define JOIN_OBJ                11      /* memb_objs.h */
#define REFER_OBJ               12      /* memb_objs.h */
#define STATETRANS_OBJ          13      /* memb_objs.h */

/* Non-Transmitted objects */
#define SCATTER                 20
#define QUEUE_ELEMENT           21
#define QUEUE                   22
#define RETRANS_ENTRY           23
#define RING_LINK_OBJ           24
#define HOP_LINK_OBJ            25               
#define MESSAGE_LINK            26
#define DOWN_LINK               27
#define TREE_NODE               28
#define MESSAGE_FRAG_LIST       29
#define LBUCKET                 30
#define GROUP                   31
#define MEMBER                  32
#define MSG_LIST_ENTRY          33
#define SESS_SEQ_ENTRY          34
#define TIME_EVENT              35

#define ROUTE_WEIGHTS           36
#define PROF_FUNCT              37
#define QUEUE_SET               38
#define MQUEUE_ELEMENT          39
#define TCP_LINK_OBJ            40
#define MESSAGE_META_OBJ        41

#define PROC_RECORD             42
#define SYS_SCATTER             43
#define STAT_RECORD             44
#define STAT_GROUP              45
#define STAT_REFRECORD          46

#define MEMB_MISSING_SEQENTRY   47
#define MEMB_MISSING_DAEMON     48
#define MEMB_DEPEND_MESSL       49
#define PACKET_BODY		50

#define SESSION_AUTH_INFO       51

#define GROUPS_BUF_LINK         52
#define GROUPS_MESSAGE_LINK     53
#define DAEMON_MEMBERS          54

/* Special objects */
#define UNKNOWN_OBJ             55      /* This represents an object of undertermined or 
                                         * variable type.  Can only be used when appropriate.
                                         * i.e. when internal structure of object is not accessed.
                                         * This is mainly used with queues
                                         */

#define Is_Pack_Head_Obj( type )        ( (type) == PACK_HEAD_OBJ )
#define Is_Message_Obj( type )          ( (type) == MESSAGE_OBJ )
#define Is_Msg_Frag_Obj( type )         ( (type) == MSG_FRAG_OBJ )
#define Is_Ret_Req_Obj( type )          ( (type) == RET_REQ_OBJ )
#define Is_Link_Ack_Obj( type )         ( (type) == LINK_ACK_OBJ )
#define Is_Aru_Update_Obj( type )              ( (type) == ARU_UPDATE_OBJ )
#define Is_Msg_Meta_Obj( type )         ( (type) == MESSAGE_META_OBJ )
#define Is_Statetrans_Obj( type )         ( (type) == STATETRANS_OBJ )


typedef struct d_obj {
        int32u          obj_type;
        int32           obj_ref_count; /* who is using it before we free it - should not be sent */
} base_obj;


/* Global Functions to manipulate objects */
int     Is_Valid_Object(int32u oid);

char    *Objnum_to_String(int32u obj_type);

int32   Obj_Inc_Refcount(void *obj);
int32   Obj_Dec_Refcount(void *obj);
int32   Obj_Net_SizeP(void *obj);
int32   Obj_Net_SizeT(int32u obj_type);
int32   Obj_SizeP(void * obj);
int32   min32(int32, int32);
int32u  min32u(int32u, int32u);

#endif /* OBJECTS_H */


