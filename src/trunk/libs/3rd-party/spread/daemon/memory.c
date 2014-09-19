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


/* memory.c
 * memory allocater and deallocater
 *
 */
#include "arch.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "memory.h"
#include "alarm.h"
#include "objects.h"

#define MAX_MEM_OBJECTS         200

/* Define SPREAD_STATUS when Memory is compiled with the Spread Status system.
 * If memory is being used outside of Spread then comment the below define out.
 */


#ifdef SPREAD_STATUS
#include "status.h"
#endif

/************************
 * Global Variables 
 ************************/

/* Total bytes currently allocated including overhead */
static unsigned int     Mem_Bytes_Allocated;
/* Total number of objects of all types allocated currently */
static unsigned int     Mem_Obj_Allocated;
/* Total number of objects currently used by the application */
static unsigned int     Mem_Obj_Inuse;
/* Maximum bytes allocated at any one time during execution */
static unsigned int     Mem_Max_Bytes;
/* Maximum number of objects allocated at any one time */
static unsigned int     Mem_Max_Objects;
/* Maximum number of ojects used by application at any one time */
static unsigned int     Mem_Max_Obj_Inuse;


typedef struct mem_header_d 
{
        int32u   obj_type;
        size_t   block_len;
} mem_header;

/* NOTE: Only num_obj_inpool is updated when debugging is turned off
 * (i.e. define NDEBUG) it is NECESSARY to track buffer pool size
 */
typedef struct mem_info_d
{
        bool            exist;  /* 1 = object registered, 0 = unused object number */
        size_t          size;   /* size of object in bytes (should be from sizeof so aligned ) */
        unsigned int    threshold;
#ifndef NDEBUG
        unsigned int    bytes_allocated;
        unsigned int    max_bytes;
        unsigned int    num_obj;
        unsigned int    max_obj;
        unsigned int    num_obj_inuse;
        unsigned int    max_obj_inuse;
#endif
        unsigned int    num_obj_inpool;
        void            **list_head;
} mem_info;

static mem_info Mem[MAX_MEM_OBJECTS];

#ifndef NDEBUG
 static bool Initialized;
#endif


#ifdef  SPREAD_STATUS
static bool MemStatus_initialized;
#endif

int Mem_valid_objtype(int32u objtype) 
{
        /* if any bits set higher then max object type return failure */
        if (objtype > MAX_MEM_OBJECTS) { return(0); }

        /* if table entry is valid return that */
        return(Mem[objtype].exist);
}        

/* Size of the memory object */
static size_t sizeobj(int32u objtype)
{
        return(Mem[objtype].size);
}



/************************
 * Query Functions
 ************************/

unsigned int Mem_total_bytes() 
{
        return(Mem_Bytes_Allocated);
}
unsigned int Mem_total_inuse()
{
        return( Mem_Obj_Inuse );
}
unsigned int Mem_total_obj()              
{
        return( Mem_Obj_Allocated );
}
unsigned int Mem_total_max_bytes() 
{
        return(Mem_Max_Bytes);
}
unsigned int Mem_total_max_inuse()
{
        return( Mem_Max_Obj_Inuse );
}
unsigned int Mem_total_max_obj()              
{
        return( Mem_Max_Objects );
}
unsigned int Mem_obj_in_pool(int32u objtype)  
{
        return( Mem[objtype].num_obj_inpool);
}

#ifndef NDEBUG
unsigned int Mem_obj_in_app(int32u objtype)    
{
        return( Mem[objtype].num_obj_inuse );
}
unsigned int Mem_max_in_app(int32u objtype)
{
        return( Mem[objtype].max_obj_inuse );
}

unsigned int Mem_obj_total(int32u objtype)    
{
        return( Mem[objtype].num_obj );
}
unsigned int Mem_max_obj(int32u objtype)
{
        return( Mem[objtype].max_obj );
}
unsigned int Mem_bytes(int32u objtype)
{
        return( Mem[objtype].bytes_allocated );
}
unsigned int Mem_max_bytes(int32u objtype)
{
        return( Mem[objtype].max_bytes );
}
#endif /* NDEBUG */

/**********************
 * Internal functions
 **********************/

#define mem_header_ptr(obj)   ( (mem_header *) (((char *)obj) - sizeof(mem_header)) )

void    Mem_init_status()
{
#ifdef SPREAD_STATUS
        struct StatGroup *sgroup;
        int obj_type;
        char memobj_name[32];
        char memobj_desc[200];
        if (!MemStatus_initialized) 
        {
                sgroup = Stat_Group_Create("Memory_Summary_0", "Statistics for memory useage summarized over all memory objects");
                if (!sgroup)
                {
                        Alarm(PRINT, "Mem_init_object: Failed to create Memory Status group\n");
                } else {
                        Stat_Insert_Record("Memory_Summary_0", "Current Bytes Used", "bytes", STAT_INT, &Mem_Bytes_Allocated );
                        Stat_Group_Add_Member(sgroup, &Mem_Bytes_Allocated);
                        Stat_Insert_Record("Memory_Summary_0", "Maximum Bytes Used", "max_bytes", STAT_INT, &Mem_Max_Bytes );
                        Stat_Group_Add_Member(sgroup, &Mem_Max_Bytes);
                        Stat_Insert_Record("Memory_Summary_0", "Current Objects Used", "objs", STAT_INT, &Mem_Obj_Allocated );
                        Stat_Group_Add_Member(sgroup, &Mem_Obj_Allocated);
                        Stat_Insert_Record("Memory_Summary_0", "Maximum Objects Used", "max_objs", STAT_INT, &Mem_Max_Objects);
                        Stat_Group_Add_Member(sgroup, &Mem_Max_Objects);
                        Stat_Insert_Record("Memory_Summary_0", "Current Objects Inuse", "objs_inuse", STAT_INT, &Mem_Obj_Inuse);
                        Stat_Group_Add_Member(sgroup, &Mem_Obj_Inuse);
                        Stat_Insert_Record("Memory_Summary_0", "Maximum Objects Inuse", "max_obj_inuse", STAT_INT, &Mem_Max_Obj_Inuse);
                        Stat_Group_Add_Member(sgroup, &Mem_Max_Obj_Inuse);
                }
                MemStatus_initialized = TRUE;
        }

        for (obj_type = 1; obj_type < MAX_MEM_OBJECTS; obj_type++)
        {
                if (Mem[obj_type].exist) {
                        sprintf(memobj_name, "Memory_%d", obj_type);
                        sprintf(memobj_desc, "Memory statistics for object: %s", Objnum_to_String(obj_type) );
                        sgroup = Stat_Group_Create(memobj_name, memobj_desc);
                        if (!sgroup)
                                Alarm(PRINT, "Mem_init_object:Failed to create stat group for object: %s\n", Objnum_to_String(obj_type) );
                        else {
#ifndef NDEBUG
                                Stat_Insert_Record(memobj_name, "Current Bytes Used", "bytes", STAT_INT, &(Mem[obj_type].bytes_allocated) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].bytes_allocated) );
                                Stat_Insert_Record(memobj_name, "Maximum Bytes Used", "max_bytes", STAT_INT, &(Mem[obj_type].max_bytes) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_bytes) );
                                Stat_Insert_Record(memobj_name, "Current Objects Used", "objs", STAT_INT, &(Mem[obj_type].num_obj) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj) );
                                Stat_Insert_Record(memobj_name, "Maximum Objects Used", "max_objs", STAT_INT, &(Mem[obj_type].max_obj) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_obj) );
                                Stat_Insert_Record(memobj_name, "Current Objects Inuse", "obj_inuse", STAT_INT, &(Mem[obj_type].num_obj_inuse) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj_inuse) );
                                Stat_Insert_Record(memobj_name, "Maximum Objects Inuse", "max_inuse", STAT_INT, &(Mem[obj_type].max_obj_inuse) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_obj_inuse) );
#endif
                                Stat_Insert_Record(memobj_name, "Current Objects In Pool", "obj_inpool", STAT_INT, &(Mem[obj_type].num_obj_inpool) );
                                Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj_inpool) );
                        }
                } /* if exists */
        } /* for loop */
        return;
#else
        return;
#endif
}
void            Mem_init_object_abort( int32u obj_type, int32u size, unsigned int threshold, unsigned int initial )
{
        char    *obj_name;
        int     ret;

        ret = Mem_init_object( obj_type, size, threshold, initial );
        if (ret < 0 ) {
                obj_name = Objnum_to_String( obj_type );
                Alarm( EXIT, "Mem_init_object_abort: Failed to initialize a/an %s object\n", obj_name);
        }
}
/* Input: valid object type, threshold/watermark value for this object, initial objects to create
 * Output: none
 * Effects: sets watermark for type,creates initial memory buffers and updates global vars
 * Should ONLY be called once per execution of the program
 */
int            Mem_init_object(int32u obj_type, int32u size, unsigned int threshold, unsigned int initial)
{
        int mem_error = 0;
#ifdef  SPREAD_STATUS
        char memobj_name[32];
        char memobj_desc[200];
        struct StatGroup *sgroup;
#endif
        assert((obj_type > 0) && (obj_type < MAX_MEM_OBJECTS));
        assert(size > 0 );

#ifndef NDEBUG
        if (!Initialized) {
                /* do any initialization needed just once here */
                Mem_Bytes_Allocated = 0;
                Mem_Obj_Allocated = 0;
                Mem_Obj_Inuse = 0;
                Mem_Max_Bytes = 0;
                Mem_Max_Objects = 0;
                Mem_Max_Obj_Inuse = 0;
                
                Initialized = TRUE;
        }
#endif
        assert(!(Mem[obj_type].exist));

        if( obj_type == BLOCK_OBJECT )
        {
                assert(threshold == 0);
                assert(initial == 0);
        }
        
        Mem[obj_type].exist = TRUE;
        Mem[obj_type].size = size;
        Mem[obj_type].threshold = threshold;
        /* Only enabled when MEM_DISABLE_CACHE set. Disable threshold so all memory is dellocated at dispose() */
#ifdef  MEM_DISABLE_CACHE
        Mem[obj_type].threshold = 0;
#endif
#ifndef NDEBUG
        Mem[obj_type].num_obj = 0;
        Mem[obj_type].bytes_allocated = 0;
        Mem[obj_type].num_obj_inuse = 0;
        Mem[obj_type].max_bytes = 0;
        Mem[obj_type].max_obj = 0;
        Mem[obj_type].max_obj_inuse = 0;
#endif
        Mem[obj_type].num_obj_inpool = 0;
        if (initial > 0)
        {
                /* Create 'initial' objects */
                int i;
                mem_header *head_ptr;
                void  ** body_ptr;
                for(i = initial; i > 0; i--)
                {
                        head_ptr = (mem_header *) calloc(1, sizeof(mem_header) + sizeobj(obj_type) );
                        if (head_ptr == NULL) 
                        {
                                Alarm(MEMORY, "mem_init_object: Failure to calloc an initial object. Returning with existant buffers\n");
                                mem_error = 1;
                                break;
                        }


                        head_ptr->obj_type = obj_type;
                        head_ptr->block_len = sizeobj(obj_type);
                        /* We add 1 because pointer arithm. states a pointer + 1 equals a pointer
                         * to the next element in an array where each element is of a particular size.
                         * in this case that size is 8 (or 12) 
                         * (since it is a pointer to a struct of a 32bit int and a size_t)
                         * so adding one actually moves the pointer 8 (or 12) bytes forward!
                         */
                        body_ptr = (void **) (head_ptr + 1);

#ifdef TESTING
 printf("alignment objtype = %u\n", __alignof__(head_ptr->obj_type));
 printf("alignment blocklen = %u\n", __alignof__(head_ptr->block_len));
  printf("initial  head = 0x%x\n", head_ptr);
  printf("initial body = 0x%x\n", body_ptr);
 printf("alignment head = %u\n", __alignof__(head_ptr)); 
 printf("alignment body = %u\n", __alignof__(body_ptr));
 printf("sizeof body pointer = %u\n", sizeof(body_ptr));
 printf("size head = %u\t size body = %u\n", sizeof(mem_header), sizeobj(obj_type)); 
#endif

                        *body_ptr = (void *) Mem[obj_type].list_head;
                        Mem[obj_type].list_head = body_ptr;
                        Mem[obj_type].num_obj_inpool++;
#ifndef NDEBUG
                        Mem[obj_type].num_obj++;
                        Mem[obj_type].bytes_allocated += (sizeobj(obj_type) + sizeof(mem_header));
#endif
                }
#ifndef NDEBUG
                Mem[obj_type].max_bytes = Mem[obj_type].bytes_allocated;    
                Mem[obj_type].max_obj = Mem[obj_type].num_obj;

                Mem_Bytes_Allocated += Mem[obj_type].bytes_allocated;
                Mem_Obj_Allocated += Mem[obj_type].num_obj;
                if (Mem_Bytes_Allocated > Mem_Max_Bytes) 
                {
                        Mem_Max_Bytes = Mem_Bytes_Allocated;
                }
                if (Mem_Obj_Allocated > Mem_Max_Objects)
                {
                        Mem_Max_Objects = Mem_Obj_Allocated;
                } 
#endif
        }

#ifndef NDEBUG
#ifdef SPREAD_STATUS
        if (MemStatus_initialized) {
                sprintf(memobj_name, "Memory_%d", obj_type);
                sprintf(memobj_desc, "Memory statistics for object: %s", Objnum_to_String(obj_type) );
                sgroup = Stat_Group_Create(memobj_name, memobj_desc);
                if (!sgroup)
                        Alarm(PRINT, "Mem_init_object:Failed to create stat group for object: %s\n", Objnum_to_String(obj_type) );
                else {
                        Stat_Insert_Record(memobj_name, "Current Bytes Used", "bytes", STAT_INT, &(Mem[obj_type].bytes_allocated) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].bytes_allocated) );
                        Stat_Insert_Record(memobj_name, "Maximum Bytes Used", "max_bytes", STAT_INT, &(Mem[obj_type].max_bytes) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_bytes) );
                        Stat_Insert_Record(memobj_name, "Current Objects Used", "objs", STAT_INT, &(Mem[obj_type].num_obj) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj) );
                        Stat_Insert_Record(memobj_name, "Maximum Objects Used", "max_objs", STAT_INT, &(Mem[obj_type].max_obj) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_obj) );
                        Stat_Insert_Record(memobj_name, "Current Objects Inuse", "obj_inuse", STAT_INT, &(Mem[obj_type].num_obj_inuse) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj_inuse) );
                        Stat_Insert_Record(memobj_name, "Maximum Objects Inuse", "max_inuse", STAT_INT, &(Mem[obj_type].max_obj_inuse) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].max_obj_inuse) );
                        Stat_Insert_Record(memobj_name, "Current Objects In Pool", "obj_inpool", STAT_INT, &(Mem[obj_type].num_obj_inpool) );
                        Stat_Group_Add_Member(sgroup, &(Mem[obj_type].num_obj_inpool) );
                }
        } /* if memstatus_init */
#endif
#endif

        if (mem_error) { return(MEM_ERR); }
        return(0);
}


/* Input: a valid type of object
 * Output: a pointer to memory which will hold an object
 * Effects: will only allocate an object from system if none exist in pool
 */
void *          new(int32u obj_type)
{

        assert(Mem_valid_objtype(obj_type));

        if (Mem[obj_type].list_head == NULL) 
        {
                mem_header *    head_ptr;
                
                head_ptr = (mem_header *) calloc(1, sizeof(mem_header) + sizeobj(obj_type) );
                if (head_ptr == NULL) 
                {
                        Alarm(MEMORY, "mem_alloc_object: Failure to calloc an object. Returning NULL object\n");
                        return(NULL);
                }
                head_ptr->obj_type = obj_type;
                head_ptr->block_len = sizeobj(obj_type);

#ifndef NDEBUG
                Mem[obj_type].num_obj++;
                Mem[obj_type].num_obj_inuse++;
                Mem[obj_type].bytes_allocated += (sizeobj(obj_type) + sizeof(mem_header));
                if (Mem[obj_type].bytes_allocated > Mem[obj_type].max_bytes)
                {
                        Mem[obj_type].max_bytes = Mem[obj_type].bytes_allocated;
                }
                if (Mem[obj_type].num_obj > Mem[obj_type].max_obj)
                {       
                        Mem[obj_type].max_obj = Mem[obj_type].num_obj;
                }
                if (Mem[obj_type].num_obj_inuse > Mem[obj_type].max_obj_inuse)
                {
                        Mem[obj_type].max_obj_inuse = Mem[obj_type].num_obj_inuse;
                }

                Mem_Bytes_Allocated += (sizeobj(obj_type) + sizeof(mem_header));
                Mem_Obj_Allocated++;
                Mem_Obj_Inuse++;
                if (Mem_Bytes_Allocated > Mem_Max_Bytes) 
                {
                        Mem_Max_Bytes = Mem_Bytes_Allocated;
                }
                if (Mem_Obj_Allocated > Mem_Max_Objects)
                {
                        Mem_Max_Objects = Mem_Obj_Allocated;
                }
                if (Mem_Obj_Inuse > Mem_Max_Obj_Inuse)
                {
                        Mem_Max_Obj_Inuse = Mem_Obj_Inuse;
                }

#endif
#ifdef TESTING
        printf("alloc:object = 0x%x\n", head_ptr + 1);
        printf("alloc:mem_headerptr = 0x%x\n", head_ptr);
        printf("alloc:objtype = %u:\n", head_ptr->obj_type);
        printf("alloc:blocklen = %u:\n", head_ptr->block_len);
#endif
        Alarm(MEMORY, "new: creating pointer 0x%x to object type %d named %s\n", head_ptr + 1, obj_type, Objnum_to_String(obj_type));

                return((void *) (head_ptr + 1));
        } else
        {
                void ** body_ptr;
                assert(Mem[obj_type].num_obj_inpool > 0 );

                body_ptr = Mem[obj_type].list_head;
                Mem[obj_type].list_head = (void *) *(body_ptr);
                Mem[obj_type].num_obj_inpool--;
#ifndef NDEBUG
                Mem[obj_type].num_obj_inuse++;
                if (Mem[obj_type].num_obj_inuse > Mem[obj_type].max_obj_inuse)
                {
                        Mem[obj_type].max_obj_inuse = Mem[obj_type].num_obj_inuse;
                }
                Mem_Obj_Inuse++;
                if (Mem_Obj_Inuse > Mem_Max_Obj_Inuse)
                {
                        Mem_Max_Obj_Inuse = Mem_Obj_Inuse;
                }

#endif
#ifdef TESTING
        printf("pool:object = 0x%x\n", body_ptr);
        printf("pool:mem_headerptr = 0x%x\n", mem_header_ptr((void *) body_ptr));
        printf("pool:objtype = %u:\n", mem_header_ptr((void *) body_ptr)->obj_type);
        printf("pool:blocklen = %u:\n", mem_header_ptr((void *) body_ptr)->block_len);
#endif
                Alarm(MEMORY, "new: reusing pointer 0x%x to object type %d named %s\n", body_ptr, obj_type, Objnum_to_String(obj_type));

                return((void *) (body_ptr));
        }
}


/* Input: a size of memory block desired
 * Output: a pointer to memory which will hold the block
 * Effects: 
 */
void *          Mem_alloc( unsigned int length)
{
        mem_header * head_ptr;

        if (length == 0) { return(NULL); }
        if( !Mem[BLOCK_OBJECT].exist )
        { 
                Mem[BLOCK_OBJECT].exist = TRUE;
                Mem[BLOCK_OBJECT].size = 0;
                Mem[BLOCK_OBJECT].threshold = 0;
        }

        
        head_ptr = (mem_header *) calloc(1, sizeof(mem_header) + length);
        if (head_ptr == NULL) 
        {
                Alarm(MEMORY, "mem_alloc: Failure to calloc a block. Returning NULL block\n");
                return(NULL);
        }
        head_ptr->obj_type = BLOCK_OBJECT;
        head_ptr->block_len = length;

#ifndef NDEBUG

        Mem[BLOCK_OBJECT].num_obj++;
        Mem[BLOCK_OBJECT].num_obj_inuse++;
        Mem[BLOCK_OBJECT].bytes_allocated += (length + sizeof(mem_header));
        if (Mem[BLOCK_OBJECT].bytes_allocated > Mem[BLOCK_OBJECT].max_bytes)
        {
                Mem[BLOCK_OBJECT].max_bytes = Mem[BLOCK_OBJECT].bytes_allocated;
        }
        if (Mem[BLOCK_OBJECT].num_obj > Mem[BLOCK_OBJECT].max_obj)
        {       
                Mem[BLOCK_OBJECT].max_obj = Mem[BLOCK_OBJECT].num_obj;
        }
        if (Mem[BLOCK_OBJECT].num_obj_inuse > Mem[BLOCK_OBJECT].max_obj_inuse)
        {
                Mem[BLOCK_OBJECT].max_obj_inuse = Mem[BLOCK_OBJECT].num_obj_inuse;
        }
        
        Mem_Bytes_Allocated += (length + sizeof(mem_header));
        Mem_Obj_Allocated++;
        Mem_Obj_Inuse++;
        if (Mem_Bytes_Allocated > Mem_Max_Bytes) 
        {
                Mem_Max_Bytes = Mem_Bytes_Allocated;
        }
        if (Mem_Obj_Allocated > Mem_Max_Objects)
        {
                Mem_Max_Objects = Mem_Obj_Allocated;
        }
        if (Mem_Obj_Inuse > Mem_Max_Obj_Inuse)
        {
                Mem_Max_Obj_Inuse = Mem_Obj_Inuse;
        }

#endif        
        return((void *) (head_ptr + 1));
}


/* Input: a valid pointer to an object or block  created by new or mem_alloc
 * Output: none
 * Effects: destroys the object and frees memory associated with it if necessary 
 */
void            dispose(void *object)
{
        int32u obj_type;

        if (object == NULL) { return; }

        obj_type = mem_header_ptr(object)->obj_type;
#ifdef TESTING
        printf("disp:object = 0x%x\n", object);
        printf("disp:mem_headerptr = 0x%x\n", mem_header_ptr(object));
        printf("disp:objtype = %u:\n", mem_header_ptr(object)->obj_type);
        printf("disp:blocklen = %u:\n", mem_header_ptr(object)->block_len);
#endif
        assert(Mem_valid_objtype(obj_type));
#ifndef NDEBUG
        assert(Mem[obj_type].num_obj_inuse > 0);
        assert(Mem[obj_type].num_obj > 0);
        assert(Mem[obj_type].bytes_allocated >= mem_header_ptr(object)->block_len + sizeof(mem_header));

        Alarm(MEMORY, "dispose: disposing pointer 0x%x to object type %d named %s\n", object, obj_type, Objnum_to_String(obj_type));

        Mem[obj_type].num_obj_inuse--;
        Mem_Obj_Inuse--;
        if (obj_type == BLOCK_OBJECT) 
        {
                assert(Mem[obj_type].num_obj_inpool == 0);
                assert(Mem[obj_type].threshold == 0);
        }

#endif
        if ( Mem_obj_in_pool(obj_type) >= Mem[obj_type].threshold)
        {
#ifndef NDEBUG
                Mem[obj_type].num_obj--;
                Mem[obj_type].bytes_allocated -= (sizeobj(obj_type) + sizeof(mem_header));
                Mem_Obj_Allocated--;
                Mem_Bytes_Allocated -= (sizeobj(obj_type) + sizeof(mem_header));
#endif
                free(mem_header_ptr(object));

        } else 
        {
                void ** body_ptr;
                
                body_ptr = (void **) object;
                *body_ptr = (void *) Mem[obj_type].list_head;
                Mem[obj_type].list_head = body_ptr;
                Mem[obj_type].num_obj_inpool++;
        }
}
/* Input: A valid pointer to an object/block created with new or mem_alloc
 * Output: the obj_type of this block of memory
 */
int32u  Mem_Obj_Type(const void *object)
{
        int32u  obj_type;

        assert(NULL != object);
        obj_type = mem_header_ptr(object)->obj_type;
        assert(Mem_valid_objtype(obj_type));

        return(obj_type);
}

/* Input: a valid pointer to an object/block created with memalloc_object or mem_alloc
 * Output: a pointer to an object/block which is an identical copy of the object input
 * Effects: same as memalloc_object or mem_alloc
 */
void *      Mem_copy(const void *object)
{
        void * new_object;
        int32u obj_type;

        if (object == NULL) { return(NULL); }

        obj_type = mem_header_ptr(object)->obj_type;
        assert(Mem_valid_objtype(obj_type));
        if (obj_type == BLOCK_OBJECT)
        {
                new_object = (void *) Mem_alloc(mem_header_ptr(object)->block_len);
        } else 
        {
                new_object =(void *) new(obj_type);
        }
        if (new_object == NULL) { return(NULL); }

        new_object =(void*)
 memcpy(new_object, object, mem_header_ptr(object)->block_len);

        mem_header_ptr(new_object)->obj_type = mem_header_ptr(object)->obj_type;
        mem_header_ptr(new_object)->block_len = mem_header_ptr(object)->block_len;

        return(new_object);

}

#if ( SPREAD_PROTOCOL == 3 )
char    *Objnum_to_String(int32u oid)
{

        switch(oid)
        {
        case BASE_OBJ:
                return("base_obj");
        case PACK_HEAD_OBJ:
                return("pack_head_obj");
        case MESSAGE_OBJ:
                return("message_obj");
        case MSG_FRAG_OBJ:
                return("msg_frag_obj");
        case RET_REQ_OBJ:
                return("ret_req_obj");
        case LINK_ACK_OBJ:
                return("link_ack_obj");
        case ARU_UPDATE_OBJ:
                return("aru_update_obj");
        case TOKEN_HEAD_OBJ:
                return("token_head_obj");
        case TOKEN_BODY_OBJ:
                return("token_body_obj");
        case JOIN_OBJ:
                return("join_obj");
        case REFER_OBJ:
                return("refer_obj");
        case ALIVE_OBJ:
                return("alive_obj");
        case SCATTER:
                return("scatter");
        case QUEUE_ELEMENT:
                return("queue_element");
        case QUEUE:
                return("queue");
        case RETRANS_ENTRY:
                return("retrans_entry");
        case RING_LINK_OBJ:
                return("ring_link_obj");
        case HOP_LINK_OBJ:
                return("hop_link_obj");
        case MESSAGE_LINK:
                return("message_link");
        case DOWN_LINK:
                return("down_link");
        case TREE_NODE:
                return("tree_node");
        case MESSAGE_FRAG_LIST:
                return("message_frag_list");
        case LBUCKET:
                return("leaky_bucket");
        case GROUP:
                return("group");
        case MEMBER:
                return("member");
        case MSG_LIST_ENTRY:
                return("msg_list_entry");
        case SESS_SEQ_ENTRY:
                return("sess_seq_entry");
        case TIME_EVENT:
                return("time_event");
	case ROUTE_WEIGHTS:
	        return("route_weights");
        case PROF_FUNCT:
                return("prof_funct");
        case QUEUE_SET:
                return("queue_set");
        case MQUEUE_ELEMENT:
                return("mqueue_element");
        case TCP_LINK_OBJ:
                return("tcp_link_object");
        case MESSAGE_META_OBJ:
                return("message_meta_object");
        case PROC_RECORD:
                return("proc_info");
        case SYS_SCATTER:
                return("sys_scatter");
        case STAT_RECORD:
                return("status_record");
        case STAT_GROUP:
                return("status_group");
        case STAT_REFRECORD:
                return("status_refrecord");
        case STATETRANS_OBJ:
                return("statetrans_obj");
        case PACKET_BODY:
                return("packet_body");
        case SESSION_AUTH_INFO:
                return("session_auth_info");
        default:
                return("Unknown_obj");
        }       
}
#endif

