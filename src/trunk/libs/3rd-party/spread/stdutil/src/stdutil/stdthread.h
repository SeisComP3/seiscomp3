/* Copyright (c) 2000-2009, The Johns Hopkins University
 * All rights reserved.
 *
 * The contents of this file are subject to a license (the ``License'').
 * You may not use this file except in compliance with the License. The
 * specific language governing the rights and limitations of the License
 * can be found in the file ``STDUTIL_LICENSE'' found in this 
 * distribution.
 *
 * Software distributed under the License is distributed on an AS IS 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. 
 *
 * The Original Software is:
 *     The Stdutil Library
 * 
 * Contributors:
 *     Creator - John Lane Schultz (jschultz@cnds.jhu.edu)
 *     The Center for Networking and Distributed Systems
 *         (CNDS - http://www.cnds.jhu.edu)
 */ 

#ifndef stdthread_h_2000_03_14_12_28_17_jschultz_at_cnds_jhu_edu
#define stdthread_h_2000_03_14_12_28_17_jschultz_at_cnds_jhu_edu

#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  STDMUTEX_NULL  = (int) 0xe38a690cUL,
  STDMUTEX_FAST  = (int) 0xa720c831UL,
  STDMUTEX_RCRSV = (int) 0x3f6c20deUL

} stdmutex_type;

#include <stdutil/private/stdthread_p.h>

/* Declare thread entry functions like this: void * STDTHREAD_FCN foo(void * arg); */

typedef void *(STDTHREAD_FCN *stdthread_fcn)(void *arg);  

/* Thread Management */

STDINLINE stdcode      stdthread_spawn(stdthread * thr_ptr, stdthread_id * id,
                                       stdthread_fcn thr_fcn, void * fcn_arg);

STDINLINE stdcode      stdthread_detach(stdthread thr);
STDINLINE stdcode      stdthread_join(stdthread thr, void ** exitval_ptr);
STDINLINE void         stdthread_exit(void * exitval);
STDINLINE stdthread_id stdthread_self(void);
STDINLINE stdbool      stdthread_eq(stdthread_id id1, stdthread_id id2);

/* Mutual Exclusion Locks */

STDINLINE stdcode      stdmutex_construct(stdmutex *mut, stdmutex_type t);
STDINLINE stdcode      stdmutex_destruct(stdmutex *mut);

STDINLINE stdcode      stdmutex_grab(stdmutex *mut);
STDINLINE stdcode      stdmutex_trygrab(stdmutex *mut);
STDINLINE stdcode      stdmutex_drop(stdmutex *mut);
STDINLINE stdcode      stdmutex_is_owner(stdmutex *mut, unsigned * grab_cnt);

/* Condition Variables */

STDINLINE stdcode      stdcond_construct(stdcond *cond);
STDINLINE stdcode      stdcond_destruct(stdcond *cond);

STDINLINE stdcode      stdcond_wake_one(stdcond *cond);
STDINLINE stdcode      stdcond_wake_all(stdcond *cond);

STDINLINE stdcode      stdcond_wait(stdcond *cond, stdmutex *mut);
/*STDINLINE stdcode      stdcond_timedwait(stdcond *cond, stdmutex *mut, long ns);*/

#  ifdef __cplusplus
}
#  endif

#endif
