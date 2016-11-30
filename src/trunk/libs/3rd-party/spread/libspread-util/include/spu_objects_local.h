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


/* objects.h 
 *  main declarations of objects
 * Copyright 1997-2012 Jonathan Stanton <jonathan@spread.org> 
 *
 */

#ifndef OBJECTS_LOCAL_H
#define OBJECTS_LOCAL_H

/* Object types 
 *
 * Object types must start with FIRST_APPLICATION_OBJECT_TYPE and go up.
 */

/* Sample declartion of object number
 * first_val = (FIRST_APPLICATION_OBJECT_TYPE + 1)
#define MY_FIRST_OBJ  = first_val
*/

/* Note this assumes FIRST_APPLICATION_OBJECT_TYPE = 2 */
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
#define QUEUE_LINK              35

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


/* Highest valid object number is defined in objects.h as UNKNOWN_OBJ */

#endif /* OBJECTS_LOCAL_H */


