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

#ifndef OBJECTS_H
#define OBJECTS_H

#define MAX_OBJECTS             200
#define MAX_OBJ_USED            56

/* Object types 
 *
 * Object types must start with 1 and go up. 0 is reserved 
 * The implementation of the spread-util library uses the following object ids
 * so applications must start with higher numbers. 
 */

#define TIME_EVENT              1

#define FIRST_APPLICATION_OBJECT_TYPE 2

#include "spu_objects_local.h"

/* Special objects */

/* This represents an object of undertermined or 
 * variable type.  Can only be used when appropriate.
 * i.e. when internal structure of object is not accessed.
 * This is mainly used with queues
 */
#define UNKNOWN_OBJ             (MAX_OBJ_USED -1)     


#endif /* OBJECTS_H */


