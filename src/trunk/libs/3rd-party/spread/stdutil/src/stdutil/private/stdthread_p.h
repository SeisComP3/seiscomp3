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

#ifndef stdthread_p_h_2000_05_18_12_57_48_jschultz_at_cnds_jhu_edu
#define stdthread_p_h_2000_05_18_12_57_48_jschultz_at_cnds_jhu_edu

#if !defined(_REENTRANT)

#  define STDTHREAD_FCN

typedef int stdthread;
typedef int stdthread_id;
typedef int stdmutex_impl;
typedef int stdcond_impl;

#elif defined(_WIN32)
#  include <windows.h>
#  define STDTHREAD_FCN __stdcall

typedef HANDLE stdthread;
typedef DWORD stdthread_id;
typedef CRITICAL_SECTION stdmutex_impl;

typedef struct stdcond_link 
{
  HANDLE                sleeper;
  struct stdcond_link * prev;
  struct stdcond_link * next;

} stdcond_link;

typedef struct 
{
  stdmutex_impl lock;
  stdcond_link  sleepers_end;  /* linked list of suspended threads */

} stdcond_impl;

/* NOTE: we use CRITICAL_SECTION for mutexes and SuspendThread and
   ResumeThread for conditions rather than MUTEX, EVENT and
   SignalObjectAndWait due to backwards compatability. The ones we use
   are available all the way back to Windows 95, whereas
   SignalObjectAndWait is only available in NT 4.0 and later.  MUTEXes
   are more expensive than CRITICAL_SECTIONs.  Also, with
   SignalObjectAndWait the only way to get both wake 1 and wake all
   semantics is to have an auto-reset EVENT that you PulseEvent N
   times for each sleeping thread for wake all.  If you do that you
   can have a timed sleep, whereas we can't easily do that with
   SuspendThread and ResumeThread.  (Might be able to do this with a
   dedicated 'waker' thread who manages waking up sleeping threads
   based on timeouts)
*/

#else
#  include <pthread.h>
#  define STDTHREAD_FCN

typedef pthread_t stdthread;
typedef pthread_t stdthread_id;
typedef pthread_mutex_t stdmutex_impl;
typedef pthread_cond_t stdcond_impl;

#endif

typedef stdmutex_impl stdmutex_fast;

typedef struct
{
  stdmutex_impl outer_lock;
  int           num_waiting;
  int           owner_cnt;
  stdthread_id  owner_id;
  stdmutex_impl inner_lock;

} stdmutex_rcrsv;

typedef struct stdmutex
{
  stdmutex_type mut_type;

  union {
    stdmutex_fast  fast;
    stdmutex_rcrsv rcrsv;

  } mut;

} stdmutex;

typedef stdcond_impl stdcond;

#endif
