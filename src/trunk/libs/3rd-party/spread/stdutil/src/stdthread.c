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

#include <stdlib.h>
#include <stdutil/stderror.h>
#include <stdutil/stdthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Need to implement code for STDMUTEX_NULL for threaded versions */
/* TODO: Need to implement code for STDCOND_NULL */
/* TODO: Need to break out the different implementations into their
   own sections of code (i.e. - multiple implementations of each
   function) like we did for !defined(_REENTRANT) 
*/

#if !defined(_REENTRANT)

STDINLINE stdcode stdthread_spawn(stdthread * thr_ptr, stdthread_id * id,
				  stdthread_fcn thr_fcn, void * fcn_arg)
{
  return STDENOSYS;
}

STDINLINE stdcode stdthread_detach(stdthread thr)
{
  return STDENOSYS;
}

STDINLINE stdcode stdthread_join(stdthread thr, void ** exitval_ptr)
{
  return STDENOSYS;
}

STDINLINE void stdthread_exit(void * exitval)
{
  exit(0);
}

STDINLINE stdthread_id stdthread_self(void)
{
  return STDENOSYS;
}

STDINLINE stdbool stdthread_eq(stdthread_id id1, stdthread_id id2)
{
  return STDENOSYS;
}

STDINLINE stdcode stdmutex_construct(stdmutex *mut, stdmutex_type t)
{
  return (t == STDMUTEX_NULL ? STDESUCCESS : STDENOSYS);
}

STDINLINE stdcode stdmutex_destruct(stdmutex *mut)
{
  return STDESUCCESS;
}

STDINLINE stdcode stdmutex_grab(stdmutex *mut)
{
  return STDESUCCESS;
}

STDINLINE stdcode stdmutex_trygrab(stdmutex *mut)
{
  return STDESUCCESS;
}

STDINLINE stdcode stdmutex_drop(stdmutex *mut)
{
  return STDESUCCESS;
}

STDINLINE stdcode stdmutex_is_owner(stdmutex *mut, unsigned * grab_cnt)
{
  *grab_cnt = (unsigned) -1;

  return STDESUCCESS;
}

STDINLINE stdcode stdcond_construct(stdcond *cond)
{
  return STDENOSYS;
}

STDINLINE stdcode stdcond_destruct(stdcond *cond)
{
  return STDENOSYS;
}

STDINLINE stdcode stdcond_wake_one(stdcond *cond)
{
  return STDENOSYS;
}

STDINLINE stdcode stdcond_wake_all(stdcond *cond)
{
  return STDENOSYS;
}

STDINLINE stdcode stdcond_wait(stdcond *cond, stdmutex *mut)
{
  return STDENOSYS;
}

/*STDINLINE stdcode stdcond_timedwait(stdcond *cond, stdmutex *mut, long ns);*/

#else

#  if defined(_WIN32)
#    include <process.h>
#    define STDSLEEPERS_BEGIN(cond) ((cond)->sleepers_end.next)
#    define STDSLEEPERS_LAST(cond)  ((cond)->sleepers_end.prev)
#    define STDSLEEPERS_END(cond)   (&(cond)->sleepers_end)
#  else
#    include <sys/time.h>
#  endif

/************************************************************************************************
 * stdthread_spawn: Create a new thread and start its execution.
 ***********************************************************************************************/

STDINLINE stdcode stdthread_spawn(stdthread *thr_ptr, stdthread_id *id, stdthread_fcn thr_fcn, void *fcn_arg) 
#  if defined(_WIN32)
{
  stdcode  ret = STDESUCCESS;
  HANDLE   thr;
  unsigned tid;

  if ((thr = (HANDLE) _beginthreadex(NULL, 0, (unsigned(STDTHREAD_FCN*)(void*)) thr_fcn, fcn_arg, 0, &tid)) == NULL) {
    ret = errno;
    STDSAFETY_CHECK(ret != STDESUCCESS);
    goto stdthread_spawn_end;
  }

  *thr_ptr = thr;
  *id      = tid;

 stdthread_spawn_end:
  return ret;
}
#  else
{
  stdcode   ret;
  pthread_t tid;

  if ((ret = pthread_create(&tid, NULL, thr_fcn, fcn_arg)) != STDESUCCESS) {
    goto stdthread_spawn_end;
  }

  *thr_ptr = tid;
  *id      = tid;

 stdthread_spawn_end:
  return ret;
}
#  endif

/************************************************************************************************
 * stdthread_detach: Set a thread to be in the detached state.  Upon
 * exiting, the thread's resources will be reclaimed.  A detached
 * thread cannot be joined.
 ***********************************************************************************************/

STDINLINE stdcode stdthread_detach(stdthread thr) 
#  if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;

  if (CloseHandle(thr) == 0) {
    ret = (stdcode) GetLastError();
    STDSAFETY_CHECK(ret != STDESUCCESS);
  }

  return ret;
}
#  else
{
  return pthread_detach(thr);
}
#  endif

/************************************************************************************************
 * stdthread_join: Wait for a thread to exit and get its return value.
 ***********************************************************************************************/

STDINLINE stdcode stdthread_join(stdthread thr, void **exitval_ptr) 
#  if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;
  DWORD   term_status;

  if (WaitForSingleObject(thr, INFINITE) != WAIT_OBJECT_0) {
    goto stdthread_join_fail;
  }

  if (exitval_ptr != NULL) {
#    if (SIZEOF_VOID_P <= 4 && SIZEOF_VOID_P <= SIZEOF_INT)

    if (GetExitCodeThread(thr, &term_status) == 0) {
      goto stdthread_join_fail;
    }

    *exitval_ptr = (void*) term_status;
#    else
    *exitval_ptr = NULL;               /* can't pass a larger pointer through a 4 byte DWORD */
#    endif
  }

  if (CloseHandle(thr) == 0) {
    goto stdthread_join_fail;
  }

  goto stdthread_join_end;

  /* error handling and return */

 stdthread_join_fail:
  ret = (stdcode) GetLastError();
  STDSAFETY_CHECK(ret != STDESUCCESS);

 stdthread_join_end:
  return ret;
}
#  else
{
  return pthread_join(thr, exitval_ptr);
}
#  endif

/************************************************************************************************
 * stdthread_exit: Terminate the execution of the current thread.
 ***********************************************************************************************/

STDINLINE void stdthread_exit(void *exitval) 
#  if defined(_WIN32)
{
#    if (SIZEOF_VOID_P <= 4 && SIZEOF_VOID_P <= SIZEOF_INT)
  _endthreadex((unsigned) exitval);
#    else
  _endthreadex(0);                     /* can't pass a larger pointer through a 4 byte DWORD */
#    endif
}
#  else
{
  pthread_exit(exitval);
}
#  endif

/************************************************************************************************
 * stdthread_self: Get the ID of the running thread.
 ***********************************************************************************************/

STDINLINE stdthread_id stdthread_self(void)
#  if defined(_WIN32)
{
  return GetCurrentThreadId();
}
#  else
{
  return pthread_self();
}
#  endif

/************************************************************************************************
 * stdthread_eq: Compare two thread IDs for equality.
 ***********************************************************************************************/

STDINLINE stdbool stdthread_eq(stdthread_id id1, stdthread_id id2)
#if defined(_WIN32)
{
  return id1 == id2;
}
#else
{
  return pthread_equal(id1, id2);
}
#endif

/************************************************************************************************
 * stdmutex_impl_init: Initialize an architecture dependent mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_impl_init(stdmutex_impl *mut)
#if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;

  __try {
    InitializeCriticalSection(mut);

  } __except (GetExceptionCode() == STATUS_NO_MEMORY ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    ret = STDENOMEM;
  }

  return ret;
}
#else
{
  return pthread_mutex_init(mut, NULL);
}
#endif

/************************************************************************************************
 * stdmutex_impl_fini: Reclaim an architecture dependent mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_impl_fini(stdmutex_impl *mut)
#if defined(_WIN32)
{
  DeleteCriticalSection(mut);

  return STDESUCCESS;
}
#else
{
  return pthread_mutex_destroy(mut);
}
#endif

/************************************************************************************************
 * stdmutex_impl_grab: Grab an architecture dependent mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_impl_grab(stdmutex_impl *mut, stdbool hardgrab)
#if defined(_WIN32)
{
  stdcode ret = STDESUCCESS;

  __try {
    if (hardgrab) {
      EnterCriticalSection(mut);

#  if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
    } else if (!TryEnterCriticalSection(mut)) {
      ret = STDEBUSY;
    }
#  else
    } else {
      ret = STDENOSYS;
    }
#  endif

  } __except ((GetExceptionCode() == STATUS_NO_MEMORY ||
	       GetExceptionCode() == STATUS_INVALID_HANDLE) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    ret = STDENOMEM;
  }

  return ret;
}
#else
{
  return (hardgrab ? pthread_mutex_lock(mut) : pthread_mutex_trylock(mut));
}
#endif

/************************************************************************************************
 * stdmutex_impl_drop: Drop an architecture dependent mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_impl_drop(stdmutex_impl *mut)
#if defined(_WIN32)
{
  LeaveCriticalSection(mut);

  return STDESUCCESS;
}
#else
{
  return pthread_mutex_unlock(mut);
}
#endif

/************************************************************************************************
 * stdcond_impl_wake: Wake either 1 or all threads waiting on a condition.
 ***********************************************************************************************/

STDINLINE static stdcode stdcond_impl_wake(stdcond *cond, stdbool wake_all)
#  if defined(_WIN32)
{
  stdcode  ret;
  stdbool  woke_one = STDFALSE;

  if ((ret = stdmutex_impl_grab(&cond->lock, STDTRUE)) != STDESUCCESS) {
    goto stdcond_wake_impl_end;
  }

  while (ret == STDESUCCESS && (wake_all || !woke_one) && STDSLEEPERS_BEGIN(cond) != STDSLEEPERS_END(cond)) {
    stdcond_link * curr     = STDSLEEPERS_BEGIN(cond);
    stdcond_link * next     = curr->next;
    HANDLE         sleeper  = curr->sleeper;
    stdbool        inf_loop = STDFALSE;
    DWORD          loop_cnt;
    DWORD          err;

    /* wake sleeper at front of sleepers list */

    woke_one = STDFALSE;

    for (loop_cnt = 10; loop_cnt != 0; --loop_cnt) {  /* don't try to wake sleeper more than a constant # of times */
      err = ResumeThread(sleeper);

      if (err == 1) {
	woke_one = STDTRUE;            /* THE SLEEPER HAS AWOKEN! */
	break;

      } else if (err == 0) {
	Sleep(0);                      /* sleeper hasn't suspended yet: yield time slice and try again */

      } else if (err == (DWORD) -1) {  /* error */
	ret = (stdcode) GetLastError();
	STDSAFETY_CHECK(ret != STDESUCCESS);
	break;

      } else if (err > MAXIMUM_SUSPEND_COUNT || (err > loop_cnt && inf_loop)) {
	ret = STDEUNKNOWN;             /* suspend count acting weird: either invalid or not decrementing! */
	break;

      } else {
	inf_loop = STDTRUE;            /* only allow setting loop_cnt one time to prevent infinite loops */
	loop_cnt = err;                /* set loop_cnt to number of ResumeThreads needed */
      }
    }

    /* if ResumeThread returns 0 on the last iteration of the loop,
       then we treat it as a (semi) success: he's already been awoken
    */

    if (loop_cnt == 0) {
      woke_one = STDTRUE;
    }

    /* unlink curr from beginning of sleepers; reclaim handle and stdcond_link */

    next->prev                  = STDSLEEPERS_END(cond);
    STDSLEEPERS_END(cond)->next = next;
    CloseHandle(sleeper);
    free(curr);
  }

  stdmutex_impl_drop(&cond->lock);

 stdcond_wake_impl_end:
  return ret;
}
#else
{
  return (wake_all ? pthread_cond_broadcast(cond) : pthread_cond_signal(cond));
}
#endif

/************************************************************************************************
 * stdmutex_fast_init: Initialize a mutex to be a fast mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_fast_init(stdmutex *mut)
{
  stdcode ret;

  if ((ret = stdmutex_impl_init(&mut->mut.fast)) != STDESUCCESS) {
    goto stdmutex_fast_init_fail;
  }

  mut->mut_type = STDMUTEX_FAST;
  goto stdmutex_fast_init_end;

  /* error handling and return */

 stdmutex_fast_init_fail:
  mut->mut_type = (stdmutex_type) 0;

 stdmutex_fast_init_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_fast_fini: Reclaim a fast mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_fast_fini(stdmutex *mut)
{
  stdcode ret;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_FAST);
  
  if ((ret = stdmutex_impl_fini(&mut->mut.fast)) == STDESUCCESS) {
    mut->mut_type = (stdmutex_type) 0;
  }

  return ret;    
}

/************************************************************************************************
 * stdmutex_fast_grab: Grab a fast mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_fast_grab(stdmutex *mut, stdbool hardgrab) 
{
  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_FAST);

  return stdmutex_impl_grab(&mut->mut.fast, hardgrab);
}

/************************************************************************************************
 * stdmutex_fast_drop: Drop a fast mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_fast_drop(stdmutex *mut) 
{
  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_FAST);

  return stdmutex_impl_drop(&mut->mut.fast);
}

/************************************************************************************************
 * stdmutex_fast_cond_wait: Wait on a condition protected by a fast mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_fast_cond_wait(stdmutex *mut, stdcond *cond)
#  if defined(_WIN32)
{
  /* NOTE: this WIN32 code works for both fast and recursive mutexes:
     the WIN32 stdmutex_rcrsv_cond_wait is a call through to this fcn
  */

  stdcode        ret = STDESUCCESS;
  HANDLE         curr_proc;
  HANDLE         curr_thread;
  unsigned       grab_cnt;
  int            prev_priority;
  int            new_priority;
  HANDLE         dup_handle;
  stdcond_link * my_link;
  stdbool        reacquire = STDFALSE;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_FAST || mut->mut_type == STDMUTEX_RCRSV);

  /* check for recursive mutexes that owner only has one lock */

  if (mut->mut_type == STDMUTEX_RCRSV) {

    if ((ret = stdmutex_is_owner(mut, &grab_cnt)) != STDESUCCESS) {
      goto stdmutex_fast_cond_wait_end;
    }

    if (grab_cnt == 0) {
      ret = STDEPERM;
      goto stdmutex_fast_cond_wait_end;
    }

    if (grab_cnt != 1) {
      ret = STDEBUSY;
      goto stdmutex_fast_cond_wait_end;
    }
  }

  /* NOTE: we DuplicateHandle because only this thread is allowed to
     use a handle from GetCurrentThread() and another thread will need
     to resume this thread
  */ 

  curr_proc   = GetCurrentProcess();
  curr_thread = GetCurrentThread();

  if (DuplicateHandle(curr_proc, curr_thread, curr_proc, &dup_handle, 0, TRUE, DUPLICATE_SAME_ACCESS) == 0) {
    ret = (stdcode) GetLastError();
    goto stdmutex_fast_cond_wait_end;
  }

  /* allocate a node to put in the condition's linked list of sleeping threads */

  if ((my_link = (stdcond_link*) malloc(sizeof(stdcond_link))) == NULL) {
    ret = STDENOMEM;
    goto stdmutex_fast_cond_wait_malloc;
  }

  /* grab the condition's internal lock */

  if ((ret = stdmutex_impl_grab(&cond->lock, STDTRUE)) != STDESUCCESS) {
    goto stdmutex_fast_cond_wait_grab_cond;
  }

  /* drop mut */

  if ((ret = stdmutex_drop(mut)) != STDESUCCESS) {
    goto stdmutex_fast_cond_wait_drop_mut;
  }

  reacquire = STDTRUE;  /* try to reacquire mut at end of this fcn */

  /* get this thread's current priority so we can restore it after we bump it up */

  if ((prev_priority = GetThreadPriority(curr_thread)) == THREAD_PRIORITY_ERROR_RETURN) {
    ret = (stdcode) GetLastError();
    goto stdmutex_fast_cond_wait_ThreadPriority;
  }

  /* bump up priority so that waker thread can never infinitely loop in a wake call (see below) */

  if (SetThreadPriority(curr_thread, THREAD_PRIORITY_HIGHEST) == 0) {
    ret = (stdcode) GetLastError();
    goto stdmutex_fast_cond_wait_ThreadPriority;
  }

  /* link onto end of sleepers list */

  my_link->sleeper             = dup_handle;
  my_link->prev                = STDSLEEPERS_LAST(cond);
  my_link->next                = STDSLEEPERS_END(cond);
  STDSLEEPERS_LAST(cond)->next = my_link;
  STDSLEEPERS_END(cond)->prev  = my_link;

  /* NOTE: waker thread closes dup_handle, removes my_link from SLEEPERS + frees it */

  /* drop the condition lock + Suspend the thread */

  if (stdmutex_impl_drop(&cond->lock) != STDESUCCESS) {
    abort();
  }

  /* This gap between releasing cond->lock and suspending is why we
     bumped up the priority to the highest level -- waker will fail to
     wake us if his attempt occurs while this thread is in this gap
     and waker is a higher priority thread.  We need to bump to
     THREAD_PRIORITY_HIGHEST to ensure that this thread will have
     equal or better capability to run as a waker.  If a waker was of
     higher priority, then he would just loop forever waiting for this
     thread to Suspend, which would never happen due to its lower
     priority.
  */

  if (SuspendThread(curr_thread) == -1) {
    abort();
  }

  /* we've been woken: restore previous thread priority if it hasn't changed while we slept */

  if ((new_priority = GetThreadPriority(curr_thread)) == THREAD_PRIORITY_ERROR_RETURN ||
      new_priority == THREAD_PRIORITY_HIGHEST) {
    new_priority = prev_priority;
  }

  if (SetThreadPriority(curr_thread, prev_priority) == 0) {
    ret = (stdcode) GetLastError();
  }

  goto stdmutex_fast_cond_wait_end;

  /* error handling and return */

 stdmutex_fast_cond_wait_ThreadPriority:
 stdmutex_fast_cond_wait_drop_mut:
  stdmutex_impl_drop(&cond->lock);

 stdmutex_fast_cond_wait_grab_cond:
  free(my_link);

 stdmutex_fast_cond_wait_malloc:
  CloseHandle(dup_handle);
  STDSAFETY_CHECK(ret != STDESUCCESS);

 stdmutex_fast_cond_wait_end:
  if (reacquire) {
    ret |= stdmutex_grab(mut);
  }

  return ret;
}
#  else
{
  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_FAST);

  return pthread_cond_wait(cond, &mut->mut.fast);
}
#  endif

/************************************************************************************************
 * stdmutex_rcrsv_init: Initialize a recursive mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_init(stdmutex *mut)
{
  stdcode          ret;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  if ((ret = stdmutex_impl_init(&rmut->outer_lock)) != STDESUCCESS) {
    goto stdmutex_rcrsv_init_fail;
  }

  if ((ret = stdmutex_impl_init(&rmut->inner_lock)) != STDESUCCESS) {
    goto stdmutex_rcrsv_init_fail2;
  }

  rmut->num_waiting = 0;
  rmut->owner_cnt   = 0;
  mut->mut_type     = STDMUTEX_RCRSV;

  goto stdmutex_rcrsv_init_end;

  /* error handling and return */

 stdmutex_rcrsv_init_fail2:
  stdmutex_impl_fini(&rmut->outer_lock);
  
 stdmutex_rcrsv_init_fail:
  mut->mut_type = (stdmutex_type) 0;

 stdmutex_rcrsv_init_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_rcrsv_fini: Invalidate a recursive mutex and reclaim its resources.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_fini(stdmutex *mut)
{
  stdcode          ret;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_RCRSV);

  if ((ret = stdmutex_impl_grab(&rmut->outer_lock, STDFALSE)) != STDESUCCESS) {
    goto stdmutex_rcrsv_fini_end;
  }

  if (mut->mut_type != STDMUTEX_RCRSV || rmut->owner_cnt < 0 || rmut->num_waiting < 0) {
    ret = STDEINVAL;
    goto stdmutex_rcrsv_fini_fail;
  }

  if (rmut->owner_cnt != 0 || rmut->num_waiting != 0) {
    ret = STDEBUSY;
    goto stdmutex_rcrsv_fini_fail;
  }

  mut->mut_type     = (stdmutex_type) 0;
  rmut->owner_cnt   = -666;              
  rmut->num_waiting = -666;

  stdmutex_impl_fini(&rmut->inner_lock);
  stdmutex_impl_drop(&rmut->outer_lock);
  stdmutex_impl_fini(&rmut->outer_lock);

  goto stdmutex_rcrsv_fini_end;

  /* error handling and return */

 stdmutex_rcrsv_fini_fail:
  stdmutex_impl_drop(&rmut->outer_lock);

 stdmutex_rcrsv_fini_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_rcrsv_is_owner: Get the current thread's grab count on mut.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_is_owner(stdmutex *mut, unsigned *grab_cnt)
{
  stdcode          ret  = STDESUCCESS;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_RCRSV);

  if ((ret = stdmutex_impl_grab(&rmut->outer_lock, STDTRUE)) != STDESUCCESS) {
    goto stdmutex_rcrsv_is_owner_end;
  }

  if (mut->mut_type != STDMUTEX_RCRSV || rmut->owner_cnt < 0 || rmut->num_waiting < 0) {
    ret = STDEINVAL;
    goto stdmutex_rcrsv_is_owner_drop;
  }

  if (rmut->owner_cnt != 0 && stdthread_eq(rmut->owner_id, stdthread_self())) {
    *grab_cnt = (unsigned) rmut->owner_cnt;

  } else {
    *grab_cnt = 0;
  }

 stdmutex_rcrsv_is_owner_drop:
  stdmutex_impl_drop(&rmut->outer_lock);

 stdmutex_rcrsv_is_owner_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_rcrsv_grab: Grab a recursive mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_grab(stdmutex *mut, stdbool hardgrab)
{
  stdcode          ret;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_RCRSV);

  if ((ret = stdmutex_impl_grab(&rmut->outer_lock, STDTRUE)) != STDESUCCESS) {
    goto stdmutex_rcrsv_grab_end;
  }

  if (mut->mut_type != STDMUTEX_RCRSV || rmut->owner_cnt < 0 || rmut->num_waiting < 0) {
    ret = STDEINVAL;
    goto stdmutex_rcrsv_grab_drop;
  }

  if (rmut->owner_cnt != 0 && stdthread_eq(rmut->owner_id, stdthread_self())) {
    ++rmut->owner_cnt;                                   /* owner just increments count */

  } else if (hardgrab) {                                 /* need to wait on inner_lock */
    ++rmut->num_waiting;

    stdmutex_impl_drop(&rmut->outer_lock);
    ret = stdmutex_impl_grab(&rmut->inner_lock, STDTRUE);
    stdmutex_impl_grab(&rmut->outer_lock, STDTRUE);

    --rmut->num_waiting;

    if (ret == STDESUCCESS) {
      rmut->owner_cnt = 1;
      rmut->owner_id  = stdthread_self();                /* it's mine! */
    }

  } else {
    ret = STDEBUSY;                                      /* caller doesn't want to potentially block */
    goto stdmutex_rcrsv_grab_drop;
  }

 stdmutex_rcrsv_grab_drop:
  stdmutex_impl_drop(&rmut->outer_lock);

 stdmutex_rcrsv_grab_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_rcrsv_drop: Drop a recursive mutex.
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_drop(stdmutex *mut) 
{
  stdcode          ret;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_RCRSV);

  if ((ret = stdmutex_impl_grab(&rmut->outer_lock, STDTRUE)) != STDESUCCESS) {
    goto stdmutex_rcrsv_drop_end;
  }

  if (mut->mut_type != STDMUTEX_RCRSV || rmut->owner_cnt < 0 || rmut->num_waiting < 0) {
    ret = STDEINVAL;
    goto stdmutex_rcrsv_drop_drop;
  }

  if (rmut->owner_cnt == 0 || !stdthread_eq(rmut->owner_id, stdthread_self())) {
    ret = STDEPERM;
    goto stdmutex_rcrsv_drop_drop;
  }

  if (--rmut->owner_cnt == 0) {
    stdmutex_impl_drop(&rmut->inner_lock);
  }

 stdmutex_rcrsv_drop_drop:
  stdmutex_impl_drop(&rmut->outer_lock);

 stdmutex_rcrsv_drop_end:
  return ret;
}

/************************************************************************************************
 * stdmutex_rcrsv_cond_wait: Atomically unlock a 'mut' and wait on 'cond.'
 ***********************************************************************************************/

STDINLINE static stdcode stdmutex_rcrsv_cond_wait(stdmutex *mut, stdcond *cond)                                           
#  if defined(_WIN32)
{
  /* NOTE: the WIN32 code works for both fast and recursive mutexes: 
     currently stdmutex_rcrsv_cond_wait is just a call through
  */

  return stdmutex_fast_cond_wait(mut, cond);
}
#  else
{
  stdcode          ret;
  stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

  STDSAFETY_CHECK(mut->mut_type == STDMUTEX_RCRSV);

  if ((ret = stdmutex_impl_grab(&rmut->outer_lock, STDTRUE)) != STDESUCCESS) {
    goto stdmutex_rcrsv_cond_wait_end;
  }

  if (mut->mut_type != STDMUTEX_RCRSV || rmut->owner_cnt < 0 || rmut->num_waiting < 0) {
    ret = STDEINVAL;
    goto stdmutex_rcrsv_cond_wait_drop;
  }

  if (rmut->owner_cnt == 0 || !stdthread_eq(rmut->owner_id, stdthread_self())) {
    ret = STDEPERM;
    goto stdmutex_rcrsv_cond_wait_drop;
  }

  if (rmut->owner_cnt != 1) {
    ret = STDEBUSY;
    goto stdmutex_rcrsv_cond_wait_drop;
  }

  ++rmut->num_waiting;
  rmut->owner_cnt = 0;

  stdmutex_impl_drop(&rmut->outer_lock);
  pthread_cond_wait(cond, &rmut->inner_lock);
  stdmutex_impl_grab(&rmut->outer_lock, STDTRUE);

  --rmut->num_waiting;
  rmut->owner_cnt = 1;
  rmut->owner_id  = stdthread_self();

 stdmutex_rcrsv_cond_wait_drop:
  stdmutex_impl_drop(&rmut->outer_lock);

 stdmutex_rcrsv_cond_wait_end:
  return ret;
}
#  endif

/************************************************************************************************
 * stdmutex_construct: Initialize a mutex.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_construct(stdmutex *mut, stdmutex_type t)
{
  stdcode ret = STDEINVAL;

  switch (t) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_init(mut);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_init(mut);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdmutex_destruct: Invalidate a mutex and reclaim its resources.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_destruct(stdmutex *mut) 
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_fini(mut);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_fini(mut);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdmutex_grab: Grab a mutex.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_grab(stdmutex *mut) 
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_grab(mut, STDTRUE);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_grab(mut, STDTRUE);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdmutex_trygrab: Try to grab a mutex without blocking.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_trygrab(stdmutex *mut) 
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_grab(mut, STDFALSE);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_grab(mut, STDFALSE);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdmutex_drop: Drop a mutex.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_drop(stdmutex *mut) 
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_drop(mut);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_drop(mut);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdmutex_is_owner: Get the grab count on a recursive mutex.
 ***********************************************************************************************/

STDINLINE stdcode stdmutex_is_owner(stdmutex *mut, unsigned *grab_cnt)
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = STDENOSYS;
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_is_owner(mut, grab_cnt);
    break;

  case STDMUTEX_NULL:
    *grab_cnt = (unsigned) -1;
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdcond_construct: Initialize a condition variable.
 ***********************************************************************************************/

STDINLINE stdcode stdcond_construct(stdcond *cond)
#  if defined(_WIN32)
{
  stdcode ret;

  if ((ret = stdmutex_impl_init(&cond->lock)) != STDESUCCESS) {
    goto stdcond_init_end;
  }

  cond->sleepers_end.sleeper = NULL;
  cond->sleepers_end.next    = STDSLEEPERS_END(cond);
  cond->sleepers_end.prev    = STDSLEEPERS_END(cond);

 stdcond_init_end:
  return ret;
}
#  else
{
  return pthread_cond_init(cond, NULL);
}
#  endif

/************************************************************************************************
 * stdcond_destruct: Invalidate a condition variable and reclaim its resources.
 ***********************************************************************************************/

STDINLINE stdcode stdcond_destruct(stdcond *cond)
#  if defined(_WIN32)
{
  stdcode ret;

  if ((ret = stdmutex_impl_grab(&cond->lock, STDFALSE)) != STDESUCCESS) {
    goto stdcond_fini_end;
  }

  if (STDSLEEPERS_BEGIN(cond) != STDSLEEPERS_END(cond)) { /* sleepers list not empty */
    ret = STDEBUSY;
    goto stdcond_fini_fail;
  }

  stdmutex_impl_drop(&cond->lock);
  stdmutex_impl_fini(&cond->lock);

  goto stdcond_fini_end;

  /* error handling and return */

 stdcond_fini_fail:
  stdmutex_impl_drop(&cond->lock);

 stdcond_fini_end:
  return ret;
}
#  else
{
  return pthread_cond_destroy(cond);
}
#  endif

/************************************************************************************************
 * stdcond_wake_one: Wake at most a single thread waiting on a condition variable.
 ***********************************************************************************************/

STDINLINE stdcode stdcond_wake_one(stdcond * cond)
{
  return stdcond_impl_wake(cond, STDFALSE);
}

/************************************************************************************************
 * stdcond_wake_all: Wake all threads currently waiting on a condition variable.
 ***********************************************************************************************/

STDINLINE stdcode stdcond_wake_all(stdcond * cond)
{
  return stdcond_impl_wake(cond, STDTRUE);
}

/************************************************************************************************
 * stdcond_wait: Wait on a condition variable until woken.
 ***********************************************************************************************/

STDINLINE stdcode stdcond_wait(stdcond *cond, stdmutex *mut)
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_cond_wait(mut, cond);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_cond_wait(mut, cond);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}

#if 0  /* comment out stdcond_wait_timed */							   

/************************************************************************************************
 * stdcond_wait_timed:
 ***********************************************************************************************/

STDINLINE stdcode stdcond_wait_timed(stdcond *  cond, 
                                     stdmutex * mut)
{
  stdcode ret = STDEINVAL;

  switch (mut->mut_type) {

  case STDMUTEX_FAST:
    ret = stdmutex_fast_cond_wait_timed(mut, cond);
    break;

  case STDMUTEX_RCRSV:
    ret = stdmutex_rcrsv_cond_wait_timed(mut, cond);
    break;

  case STDMUTEX_NULL:
    ret = STDESUCCESS;
    break;
  }

  return ret;
}							   

/************************************************************************************************
 * stdcond_wait_timed:
 ***********************************************************************************************/

STDINLINE stdcode stdcond_wait_timed(stdcond *cond, stdmutex *mut, const stdtime *abs_time)
#  if defined(_WIN32)
{
  /* TODO: figure out how + implement me! */
  return ENOSYS;
}
#  else
{
  /* TODO: needs work; make it look like stdmutex_*_cond_wait */
  struct timespec timeout;
  stdmutex_impl * mut_impl = NULL;
  long        grab_cnt = 0;
  stdcode         ret;

  timeout.tv_sec  = abs_time->m_Sec;
  timeout.tv_nsec = abs_time->m_Nsec;

  if ((ret = stdmutex_is_owner(mut, &grab_cnt)) != STDESUCCESS) {
    assert(STDFALSE /* stdcond_wait_timed: user race condition on mut, protect calls to fini! */);
    goto stdcond_wait_timed_fail;
  }

  if (grab_cnt == 0) {
    assert(STDFALSE /* stdcond_wait_timed: user doesn't own mut! */);
    ret = EPERM;
    goto stdcond_wait_timed_EPERM;
  }

  if (grab_cnt != 1) {
    assert(STDFALSE /* stdcond_wait: recursive mutex must have a grab_cnt of 1! */);
    ret = EINVAL;
    goto stdcond_wait_timed_EINVAL;
  }

  switch (mut->mut_type) {
  case STDMUTEX_FAST:
    {
      stdmutex_fast * fmut = &mut->m_Mutex.m_Fast;

      fmut->owner_cnt = 0;
      mut_impl         = &fmut->m_Lock;
    }
    break;

  case STDMUTEX_RCRSV:
    {
      stdmutex_rcrsv * rmut = &mut->mut.rcrsv;

      rmut->owner_cnt = 0;
      mut_impl         = &rmut->inner_lock;
    }
    break;

  default:
    assert(STDFALSE);
    ret = EINVAL;
    goto stdcond_wait_timed_EINVAL;
  }

  if ((ret = pthread_cond_timedwait(cond, mut_impl, &timeout)) != STDESUCCESS) {
    goto stdcond_wait_timed_pcond_timedwait;
  }

  assert(ret == STDESUCCESS);
  goto stdcond_wait_timed_end;

  /* error handling and return */

 stdcond_wait_timed_pcond_timedwait:
 stdcond_wait_timed_EINVAL:
 stdcond_wait_timed_fail:
  assert(ret != STDESUCCESS);

 stdcond_wait_timed_end:
  return ret;
}
#  endif
#endif

#endif

#ifdef __cplusplus
}
#endif
