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


#ifndef MEMORY_H
#define MEMORY_H

#include "arch.h"

/*******************************************************************************
 * Special Object type used for 
 * NON-OBJECT Oriented allocates (traditional Malloc/free with arbitrary sizes)
 * 0 is never a valid object type for any real object (they start at 1)
 *******************************************************************************/
#define BLOCK_OBJECT            0


/************************************
 * Function Declarations
 ************************************/

/* Input: valid object type, size of object, threshold/watermark value for this object,
 *              number of initial objects to create
 * Output: error code
 * Effects: registers type, sets watermark for type,creates initial memory buffers and updates global vars
 * Should ONLY be called once per execution of the program, but must be called before any other 
 * memory management function is used on that object
 */
int            Mem_init_object(int32u obj_type, int32u size, unsigned int threshold, unsigned int initial);

/* This calls Mem_init_object and if any error results it EXIT's with a printed error */
void            Mem_init_object_abort( int32u obj_type, int32u size, unsigned int threshold, unsigned int initial );

/* This initializes the status reporting of the memory module and should be called from
 * status.c after the Group, Recod, and RefRecord objects are mem_init'ed.
 * After this is called each object created by Mem_init_object() will automatically
 * be added to the status reporting.
 * This function is needed to avoid a double-dependency loop where memory needs status working when 
 * it starts, but status requires memory working for it to work.
 */
void            Mem_init_status();
/* Input: a valid type of object
 * Output: a pointer to memory which will hold an object
 * Effects: will only allocate an object from system if none exist in pool
 */
void *          new(int32u obj_type);


/* Input: a valid pointer to an object or block  created by new or mem_alloc
 * Output: none
 * Effects: destroys the object and frees memory associated with it if necessary 
 */
void            dispose(void *object);

/***************************************************************************
 * These two functions are ONLY needed for dynamically sized allocations
 * like traditional malloc/free --NOT for object based allocations
 ***************************************************************************/

/* Input: a size of memory block desired
 * Output: a pointer to memory which will hold the block
 * Effects: 
 */
void *          Mem_alloc( unsigned int length);


/* Input: a valid pointer to an object created with memalloc_object
 * Output: a pointer to an object which is an identical copy of the object input
 * Effects: same as memalloc_object
 */
void *      Mem_copy(const void *object);

/************************
 * Query Functions
 ************************/

int     Mem_valid_objtype(int32u objtype); 

/* Input: A valid pointer to an object/block created with new or mem_alloc
 * Output: the obj_type of this block of memory
 */
int32u  Mem_Obj_Type(const void *object);

extern LOC_INLINE unsigned int Mem_total_bytes(void);
extern LOC_INLINE unsigned int Mem_total_max_bytes(void);
extern LOC_INLINE unsigned int Mem_total_inuse(void);
extern LOC_INLINE unsigned int Mem_total_max_inuse(void);
extern LOC_INLINE unsigned int Mem_total_obj(void);
extern LOC_INLINE unsigned int Mem_total_max_obj(void);
extern LOC_INLINE unsigned int Mem_bytes(int32u objtype);
extern LOC_INLINE unsigned int Mem_max_bytes(int32u objtype);
extern LOC_INLINE unsigned int Mem_obj_in_pool(int32u objtype);
extern LOC_INLINE unsigned int Mem_obj_in_app(int32u objtype);
extern LOC_INLINE unsigned int Mem_max_in_app(int32u objtype);
extern LOC_INLINE unsigned int Mem_obj_total(int32u objtype);    
extern LOC_INLINE unsigned int Mem_max_obj(int32u objtype);

#endif /* MEMORY_H */

