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
#include <string.h>

#include <stdutil/stderror.h>
#include <stdutil/stdarr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDARR_IS_LEGAL(arr) ((arr)->begin <= (arr)->end && \
			      (arr)->begin + (arr)->size * (arr)->vsize == (arr)->end && \
                              ( ((arr)->cap != 0 && (arr)->begin != NULL) || ((arr)->cap == 0 && (arr)->begin == NULL) ) && \
                              (arr)->size <= (arr)->cap && \
                              (arr)->vsize != 0 && \
			      ((arr)->opts & ~(STDARR_OPTS_NO_AUTO_GROW | STDARR_OPTS_NO_AUTO_SHRINK)) == 0)

#define STDARR_IT_IS_LEGAL(arr, it) ((it)->val >= (arr)->begin && (it)->val <= (arr)->end && (it)->vsize == (arr)->vsize)
#define STDARR_IT_IS_LEGAL2(it)     ((it)->vsize != 0)
#define STDIT_ARR_IS_LEGAL(it)      ((it)->type_id == STDARR_IT_ID && STDARR_IT_IS_LEGAL2(&(it)->impl.arr))

/************************************************************************************************
 * stdarr_low_insert_space: This fcn efficiently inserts 'num_insert'
 * element positions before the position pointed at by 'impl.' The fcn
 * updates and possibly grows the array.
 ***********************************************************************************************/

STDINLINE static stdcode stdarr_low_insert_space(stdarr *arr, stdarr_it *it, stdsize num_insert) 
{
  stdcode ret   = STDESUCCESS;
  stdsize nsize = arr->size + num_insert;
  stdsize delta = num_insert * arr->vsize;
  stdsize after = (stdsize) (arr->end - it->val);

  /* reallocate if current capacity is not big enough */

  if (nsize > stdarr_high_capacity(arr)) {                   /* nsize > X -> nsize > 0 */
    stdsize ncap;
    stdsize prior;
    stdsize asize;
    char *  mem;    

    if ((arr->opts & STDARR_OPTS_NO_AUTO_GROW) != 0) {       /* growth explicitly disallowed */
      ret = STDEACCES;
      goto stdarr_low_insert_space_end;
    }

    ncap  = (nsize << 1);                                    /* nsize > 0 -> ncap > 0 */
    ncap  = STDMAX(ncap, STDARR_MIN_AUTO_ALLOC);             /* ensure minimum allocation */
    asize = ncap * arr->vsize;                               /* calc. alloc size in bytes */
    prior = (stdsize) (it->val - arr->begin);

    if ((mem = (char*) realloc(arr->begin, asize)) == NULL) {
      ret = STDENOMEM;
      goto stdarr_low_insert_space_end;
    }

    arr->begin = mem;                                        /* fill in new values for 'arr' */
    arr->end   = mem + prior + after;
    arr->cap   = ncap;    

    it->val    = mem + prior;                                /* relocate 'it' to insertion point */
  }

  /* shift memory to create space */

  memmove(it->val + delta, it->val, after);

  arr->end  += delta;
  arr->size  = nsize;

 stdarr_low_insert_space_end:
  return ret;
}

/************************************************************************************************
 * stdarr_low_remove_space: This fcn efficiently removes 'num_remove'
 * element positions starting at the position pointed at by 'it.' The
 * fcn updates and possibly shrinks the array.
 ***********************************************************************************************/

STDINLINE static void stdarr_low_remove_space(stdarr *arr, stdarr_it *it, stdsize num_remove) 
{
  stdsize nsize  = arr->size - num_remove;
  stdsize delta  = num_remove * arr->vsize;
  char *  it_end = it->val + delta;
  stdsize after  = (stdsize) (arr->end - it_end);

  /* shift memory to remove space */

  memmove(it->val, it_end, after);

  arr->end  -= delta;
  arr->size  = nsize;

  /* reallocate if current capacity is too big */

  if ((arr->opts & STDARR_OPTS_NO_AUTO_SHRINK) == 0 &&       /* shrinking allowed */
      nsize <= stdarr_low_capacity(arr) &&                   /* shrinking needed */
      arr->cap != STDARR_MIN_AUTO_ALLOC) {                   /* cap not at min alloc */

    stdsize ncap  = (nsize << 1);
    stdsize prior = (stdsize) (it->val - arr->begin);

    ncap = STDMAX(ncap, STDARR_MIN_AUTO_ALLOC);              /* ensure minimum allocation */

    if (ncap != 0) {                                         /* nsize/ncap can be zero here */
      stdsize asize = ncap * arr->vsize;
      char *  mem;

      if ((mem = (char*) realloc(arr->begin, asize)) == NULL) {
	return;                                              /* couldn't realloc -> that's OK */
      }

      arr->begin = mem;                                      /* fill in new values for 'arr' */
      arr->end   = mem + prior + after;
      arr->cap   = ncap;

    } else {                                                 /* ncap == 0 -> go to zero */
      
      if (arr->begin != NULL) {
	free(arr->begin);
      }

      arr->begin = NULL;
      arr->end   = NULL;
      arr->cap   = 0;
    }

    it->val = arr->begin + prior;                            /* relocate 'it' to erasure point */
  }
}

/************************************************************************************************
 * stdarr_construct: Construct an initially empty array that will
 * contain elements 'sizeof_val' bytes long.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_construct(stdarr *arr, stdsize vsize, stduint8 opts) 
{ 
  stdcode ret = STDESUCCESS;

  if (vsize == 0 || (opts & ~(STDARR_OPTS_NO_AUTO_GROW | STDARR_OPTS_NO_AUTO_SHRINK)) != 0) {
    ret = STDEINVAL;
    goto stdarr_construct_fail;
  }

  arr->begin = NULL;
  arr->end   = NULL;
  arr->cap   = 0;
  arr->size  = 0;
  arr->vsize = vsize;
  arr->opts  = opts;

  goto stdarr_construct_end;

  /* error handling and return */

 stdarr_construct_fail:
  arr->vsize = 0;  /* make STDARR_IS_LEGAL(arr) false */

 stdarr_construct_end:
  return ret;
}

/************************************************************************************************
 * stdarr_copy_construct: Construct a copy of an array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_copy_construct(stdarr *dst, const stdarr *src) 
{ 
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(src));

  *dst = *src;

  if (src->begin != NULL) {
    stdsize esize = src->vsize * src->size;
    stdsize asize = src->vsize * src->cap;

    if ((dst->begin = (char*) malloc(asize)) == NULL) {
      ret = STDENOMEM;
      goto stdarr_copy_construct_fail;
    }

    dst->end = dst->begin + esize;
    memcpy(dst->begin, src->begin, esize);
  }

  goto stdarr_copy_construct_end;

  /* error handling and return */

 stdarr_copy_construct_fail:
  dst->vsize = 0;  /* make STDARR_IS_LEGAL(dst) false */
  
 stdarr_copy_construct_end:
  return ret;
}

/************************************************************************************************
 * stdarr_destruct: Reclaim an array's resources and invalidate impl.
 ***********************************************************************************************/

STDINLINE void stdarr_destruct(stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  if (arr->begin != NULL) {
    free(arr->begin);
    arr->begin = NULL;
  }

  arr->vsize = 0;  /* make STDARR_IS_LEGAL(arr) false */
}

/************************************************************************************************
 * stdarr_set_eq: Set 'dst' to have the same contents as 'src.'
 ***********************************************************************************************/

STDINLINE stdcode stdarr_set_eq(stdarr *dst, const stdarr *src)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(dst) && STDARR_IS_LEGAL(src) && dst->vsize == src->vsize);

  if (dst == src) {
    goto stdarr_set_eq_end;
  }

  if ((ret = stdarr_resize(dst, src->size)) != STDESUCCESS) {
    goto stdarr_set_eq_end;
  }

  memcpy(dst->begin, src->begin, src->vsize * src->size);

 stdarr_set_eq_end:
  return ret;
}

/************************************************************************************************
 * stdarr_swap: Make arr1 reference arr2's sequence and vice versa.
 ***********************************************************************************************/

STDINLINE void stdarr_swap(stdarr *arr1, stdarr *arr2)
{
  stdarr cpy;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr1) && STDARR_IS_LEGAL(arr2) && arr1->vsize == arr2->vsize);

  STDSWAP(*arr1, *arr2, cpy);
}

/************************************************************************************************
 * stdarr_begin: Get an iterator to the beginning of an array.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_begin(const stdarr *arr, stdit *it) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  it->type_id        = STDARR_IT_ID;
  it->impl.arr.val   = (char*) arr->begin;
  it->impl.arr.vsize = arr->vsize;

  return it;
}

/************************************************************************************************
 * stdarr_last: Get an iterator the last entry of an array.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_last(const stdarr *arr, stdit *it) 
{
  STDBOUNDS_CHECK(arr->size != 0);

  return stdarr_it_prev(stdarr_end(arr, it));
}

/************************************************************************************************
 * stdarr_end: Get an iterator to the sentinel end entry of an array.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_end(const stdarr *arr, stdit *it) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  it->type_id        = STDARR_IT_ID;
  it->impl.arr.val   = (char*) arr->end;
  it->impl.arr.vsize = arr->vsize;

  return it;
}

/************************************************************************************************
 * stdarr_get: Get an iterator the 'elem_num'th (0 based) entry of an
 * array.  Can request/return sentinel end position (i.e. get(size(arr))).
 ***********************************************************************************************/

STDINLINE stdit *stdarr_get(const stdarr *arr, stdit *it, stdsize elem_num) 
{ 
  STDBOUNDS_CHECK(elem_num <= arr->size);

  return stdarr_it_advance(stdarr_begin(arr, it), elem_num);
}

/************************************************************************************************
 * stdarr_is_begin: Returns whether or not an iterator refers to the beginning of an array.
 ***********************************************************************************************/

STDINLINE stdbool stdarr_is_begin(const stdarr *arr, const stdit *it) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  return it->impl.arr.val == arr->begin;
}

/************************************************************************************************
 * stdarr_is_end: Returns whether or not an iterator refers to the end of an array.
 ***********************************************************************************************/

STDINLINE stdbool stdarr_is_end(const stdarr *arr, const stdit *it) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  return it->impl.arr.val == arr->end;
}

/************************************************************************************************
 * stdarr_rank: Returns the 0-based rank of an iterator.
 ***********************************************************************************************/

STDINLINE stdsize stdarr_rank(const stdarr *arr, const stdit *it) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  return (stdsize) (it->impl.arr.val - arr->begin) / arr->vsize;
}

/************************************************************************************************
 * stdarr_size: Return the number of entries contained in 'arr.'
 ***********************************************************************************************/

STDINLINE stdsize stdarr_size(const stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return arr->size; 
}

/************************************************************************************************
 * stdarr_empty: Return whether or not 'arr' is empty.
 ***********************************************************************************************/

STDINLINE stdbool stdarr_empty(const stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return arr->size == 0;
}

/************************************************************************************************
 * stdarr_high_capacity: Return the maximum size 'arr' can be without
 * needing to be grown (re-allocated).
 ***********************************************************************************************/

STDINLINE stdsize stdarr_high_capacity(const stdarr *arr) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return arr->cap;
}

/************************************************************************************************
 * stdarr_low_capacity: Return the size at which 'arr' would be
 * automatically shrunk (re-allocated).
 ***********************************************************************************************/

STDINLINE stdsize stdarr_low_capacity(const stdarr *arr) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return (arr->cap >> 2);
}

/************************************************************************************************
 * stdarr_max_size: Return the theoretical maximum number of elements
 * 'arr' could possibly hold.
 ***********************************************************************************************/

STDINLINE stdsize stdarr_max_size(const stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return STDSIZE_MAX / arr->vsize;
}

/************************************************************************************************
 * stdarr_val_size: Return the size in bytes of the type of elements
 * 'arr' holds.
 ***********************************************************************************************/

STDINLINE stdsize stdarr_val_size(const stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return arr->vsize; 
}

/************************************************************************************************
 * stdarr_resize: Resize an array to contain 'num_elems' elements.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_resize(stdarr *arr, stdsize num_elems) 
{ 
  stdcode ret = STDESUCCESS;
  stdit   it;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  if (num_elems > arr->size) {
    ret = stdarr_low_insert_space(arr, &stdarr_end(arr, &it)->impl.arr, num_elems - arr->size);

  } else if (num_elems < arr->size) {
    stdarr_low_remove_space(arr, &stdarr_get(arr, &it, num_elems)->impl.arr, arr->size - num_elems);
  }

  return ret;
}

/************************************************************************************************
 * stdarr_clear: Remove all elements from an array (i.e. - make
 * stdarr_size() 0).
 ***********************************************************************************************/

STDINLINE void stdarr_clear(stdarr *arr) 
{ 
  stdarr_resize(arr, 0);
}

/************************************************************************************************
 * stdarr_set_capacity: Set the capacity of an array. Ignores all auto
 * allocation considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_set_capacity(stdarr *arr, stdsize num_elems) 
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  if (num_elems != arr->cap) {

    if (num_elems != 0) {
      char * mem;

      if ((mem = (char*) realloc(arr->begin, num_elems * arr->vsize)) == NULL) {
	ret = STDENOMEM;
	goto stdarr_set_capacity_end;
      }

      arr->cap   = num_elems;
      arr->size  = STDMIN(arr->size, num_elems);

      arr->begin = mem;
      arr->end   = mem + arr->size * arr->vsize;

    } else {

      if (arr->begin != NULL) {
	free(arr->begin);
      }

      arr->begin = NULL;
      arr->end   = NULL;
      arr->size  = 0;
      arr->cap   = 0;     
    }
  }

 stdarr_set_capacity_end:
  return ret;
}

/************************************************************************************************
 * stdarr_reserve: Ensures 'arr' can contain 'num_elems' elements
 * without any additional reallocations.  Ignores all auto allocation
 * considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_reserve(stdarr *arr, stdsize num_elems) 
{ 
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  if (num_elems > arr->cap) {
    ret = stdarr_set_capacity(arr, num_elems);
  }

  return ret;
}

/************************************************************************************************
 * stdarr_shrink_fit: Sets 'arr's capacity to 'arr's size.  Ignores
 * all auto allocation considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_shrink_fit(stdarr *arr) 
{
  return stdarr_set_capacity(arr, arr->size);
}

/************************************************************************************************
 * stdarr_push_back: Push an element onto the end of an array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_push_back(stdarr *arr, const void *val) 
{ 
  stdit it;

  return stdarr_insert(arr, stdarr_end(arr, &it), val);
}

/************************************************************************************************
 * stdarr_push_back_n: Push a C array of elements onto the end of an
 * array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_push_back_n(stdarr *arr, const void *vals, stdsize num_push) 
{ 
  stdit it;

  return stdarr_insert_n(arr, stdarr_end(arr, &it), vals, num_push);
}

/************************************************************************************************
 * stdarr_push_back_seq: Push a sequence of elements on the end of an
 * array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_push_back_seq(stdarr *arr, const stdit *b, const stdit *e)
{ 
  stdit it;

  return stdarr_insert_seq(arr, stdarr_end(arr, &it), b, e);
}

/************************************************************************************************
 * stdarr_push_back_seq_n: Push a sequence of elements on the end of an
 * array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_push_back_seq_n(stdarr *arr, const stdit *b, stdsize num_push)
{ 
  stdit it;

  return stdarr_insert_seq_n(arr, stdarr_end(arr, &it), b, num_push);
}

/************************************************************************************************
 * stdarr_push_back_rep: Push a C array of elements onto the end of an
 * array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_push_back_rep(stdarr *arr, const void *val, stdsize num_times) 
{ 
  stdit it;

  return stdarr_insert_rep(arr, stdarr_end(arr, &it), val, num_times);
}

/************************************************************************************************
 * stdarr_pop_back: Pop an element off of the end of an array.
 ***********************************************************************************************/

STDINLINE void stdarr_pop_back(stdarr *arr) 
{
  stdarr_pop_back_n(arr, 1);
}

/************************************************************************************************
 * stdarr_pop_back_n: Pop multiple elements off of the end of an array.
 ***********************************************************************************************/

STDINLINE void stdarr_pop_back_n(stdarr *arr, stdsize num_pop) 
{
  STDBOUNDS_CHECK(num_pop <= arr->size);

  stdarr_resize(arr, arr->size - num_pop);
}

/************************************************************************************************
 * stdarr_insert: Insert an element into an array at a point.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_insert(stdarr *arr, stdit *it, const void *val)
{
  return stdarr_insert_n(arr, it, val, 1);
} 

/************************************************************************************************
 * stdarr_insert_n: Insert multiple elements into an array at a point.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_insert_n(stdarr *arr, stdit *it, const void *vals, stdsize num_insert) 
{
  stdcode ret;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  if ((ret = stdarr_low_insert_space(arr, &it->impl.arr, num_insert)) != STDESUCCESS) {
    goto stdarr_insert_n_end;
  }

  memcpy(it->impl.arr.val, vals, num_insert * arr->vsize);

 stdarr_insert_n_end:
  return ret;
}

/************************************************************************************************
 * stdarr_insert_seq: Insert a sequence of elements into an array at a
 * point.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_insert_seq(stdarr *arr, stdit *it, const stdit *b, const stdit *e)
{
  stdcode  ret;
  stdssize num_insert;

  if ((num_insert = stdit_distance(b, e)) < 0) {  /* calc how many elements in [b, e) */
    ret = STDEINVAL;
    goto stdarr_insert_seq_end;
  }

  ret = stdarr_insert_seq_n(arr, it, b, num_insert);

 stdarr_insert_seq_end:
  return ret;
}


/************************************************************************************************
 * stdarr_insert_seq_n: Insert a sequence of elements into an array at
 * a point.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_insert_seq_n(stdarr *arr, stdit *it, const stdit *b, stdsize num_insert)
{
  stdcode ret;
  char *  dst_it;
  stdit   src_it;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  if ((ret = stdarr_low_insert_space(arr, &it->impl.arr, num_insert)) != STDESUCCESS) {
    goto stdarr_insert_seq_n_end;
  }

  dst_it = it->impl.arr.val;
  src_it = *b;

  for (; num_insert-- != 0; dst_it += arr->vsize, stdit_next(&src_it)) {
    memcpy(dst_it, stdit_val(&src_it), arr->vsize);
  }

 stdarr_insert_seq_n_end:
  return ret;
}

/************************************************************************************************
 * stdarr_insert_rep: Insert the same element into an array at a point
 * multiple times.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_insert_rep(stdarr *arr, stdit *it, const void *val, stdsize num_times) 
{
  stdcode ret;
  char *  dst_it;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  if ((ret = stdarr_low_insert_space(arr, &it->impl.arr, num_times)) != STDESUCCESS) {
    goto stdarr_insert_rep_end;
  }

  dst_it = it->impl.arr.val;

  for (; num_times-- != 0; dst_it += arr->vsize) {
    memcpy(dst_it, val, arr->vsize);
  }

 stdarr_insert_rep_end:
  return ret;
}

/************************************************************************************************
 * stdarr_erase: Erase an element from an array.
 ***********************************************************************************************/

STDINLINE void stdarr_erase(stdarr *arr, stdit *it) 
{
  stdarr_erase_n(arr, it, 1);
}

/************************************************************************************************
 * stdarr_erase_n: Erase multiple elements from an array.
 ***********************************************************************************************/

STDINLINE void stdarr_erase_n(stdarr *arr, stdit *it, stdsize num_erase) 
{
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr) && STDIT_ARR_IS_LEGAL(it) && STDARR_IT_IS_LEGAL(arr, &it->impl.arr));

  stdarr_low_remove_space(arr, &it->impl.arr, num_erase);
}

/************************************************************************************************
 * stdarr_erase_seq: Erase a sequence of elements from an array.
 ***********************************************************************************************/

STDINLINE void stdarr_erase_seq(stdarr *arr, stdit *b, stdit *e) 
{ 
  stdssize diff = stdarr_it_cmp(e, b);

  STDSAFETY_CHECK(diff >= 0);

  stdarr_erase_n(arr, b, diff);
  *e = *b;
}

/************************************************************************************************
 * stdarr_get_opts: Get the options of an array.
 ***********************************************************************************************/

STDINLINE stduint8 stdarr_get_opts(const stdarr *arr) 
{ 
  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  return arr->opts;
}

/************************************************************************************************
 * stdarr_set_opts: Set the options of an array.
 ***********************************************************************************************/

STDINLINE stdcode stdarr_set_opts(stdarr *arr, stduint8 opts)
{ 
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDARR_IS_LEGAL(arr));

  if ((opts & ~(STDARR_OPTS_NO_AUTO_GROW | STDARR_OPTS_NO_AUTO_SHRINK)) != 0) {
    ret = STDEINVAL;
    goto stdarr_set_opts_end;
  }
  
  arr->opts = opts;

 stdarr_set_opts_end:
  return ret;
}

/************************************************************************************************
 * stdarr_it_val: Get a pointer to the element that 'it' references.
 ***********************************************************************************************/

STDINLINE void *stdarr_it_val(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  return it->impl.arr.val;
}

/************************************************************************************************
 * stdarr_it_val_size: Returns the size of the element in bytes that 'it' references.
 ***********************************************************************************************/

STDINLINE stdsize stdarr_it_val_size(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  return it->impl.arr.vsize;
}

/************************************************************************************************
 * stdarr_it_eq: Compare two iterators for equality (refer to same element).
 ***********************************************************************************************/

STDINLINE stdbool stdarr_it_eq(const stdit *it1, const stdit *it2) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it1) && STDIT_ARR_IS_LEGAL(it2) && it1->impl.arr.vsize == it2->impl.arr.vsize);

  return it1->impl.arr.val == it2->impl.arr.val;
}

/************************************************************************************************
 * stdarr_it_cmp: Compare two iterators for rank difference.
 ***********************************************************************************************/

STDINLINE stdssize stdarr_it_cmp(const stdit *it1, const stdit *it2) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it1) && STDIT_ARR_IS_LEGAL(it2) && it1->impl.arr.vsize == it2->impl.arr.vsize);

  return (stdssize) (it1->impl.arr.val - it2->impl.arr.val) / (stdssize) it1->impl.arr.vsize;
}

/************************************************************************************************
 * stdarr_it_next: Advance 'it' by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_it_next(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  it->impl.arr.val += it->impl.arr.vsize;

  return it;
}

/************************************************************************************************
 * stdarr_it_advance: Advance 'it' by 'num_advance' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_it_advance(stdit *it, stdsize num_advance) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  it->impl.arr.val += num_advance * it->impl.arr.vsize;

  return it;
}

/************************************************************************************************
 * stdarr_it_prev: Retreat 'it' by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_it_prev(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  it->impl.arr.val -= it->impl.arr.vsize;

  return it;
}

/************************************************************************************************
 * stdarr_it_retreat: Retreat 'it' by 'num_retreat' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_it_retreat(stdit *it, stdsize num_retreat) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  it->impl.arr.val -= num_retreat * it->impl.arr.vsize;

  return it;
}

/************************************************************************************************
 * stdarr_it_offset: Offset 'it' by 'offset' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdarr_it_offset(stdit *it, stdssize offset) 
{
  STDSAFETY_CHECK(STDIT_ARR_IS_LEGAL(it));

  it->impl.arr.val += offset * it->impl.arr.vsize;

  return it;
}

#ifdef __cplusplus
}
#endif
