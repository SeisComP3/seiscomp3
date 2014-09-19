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

#include <stdutil/stderror.h>
#include <stdutil/stdit.h>
#include <stdutil/stdarr.h>
#include <stdutil/stdcarr.h>
#include <stdutil/stddll.h>
#include <stdutil/stdhash.h>
#include <stdutil/stdskl.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************************
 * stdit_get_type: Get the functional type of an iterator.
 ***********************************************************************************************/

STDINLINE stdit_type stdit_get_type(const stdit *it)
{
  stdit_type ret;

  switch (it->type_id) {
  case STDPTR_IT_ID:
  case STDPPTR_IT_ID:
  case STDARR_IT_ID:
  case STDCARR_IT_ID:
    ret = STDIT_RANDOM_ACCESS;
    break;

  case STDDLL_IT_ID:
  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = STDIT_BIDIRECTIONAL;
    break;

  default:
    ret = (stdit_type) 0;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_key: Get the key of a key-value pair from an iterator.
 ***********************************************************************************************/

STDINLINE const void *stdit_key(const stdit *it)
{
  const void * ret;

  switch (it->type_id) {
  case STDPTR_IT_ID:
  case STDARR_IT_ID:
  case STDCARR_IT_ID:
  case STDDLL_IT_ID:
    ret = NULL;
    break;

  case STDPPTR_IT_ID:
    ret = it->impl.pptr.key;
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    ret = stdhash_it_key(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = stdskl_it_key(it);
    break;

  default:
    ret = NULL;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_key_size: Get the size in bytes of the key type 'it' references.
 ***********************************************************************************************/

STDINLINE stdsize stdit_key_size(const stdit *it)
{
  stdsize ret;

  switch (it->type_id) {
  case STDPTR_IT_ID:
  case STDARR_IT_ID:
  case STDCARR_IT_ID:
  case STDDLL_IT_ID:
    ret = 0;
    break;

  case STDPPTR_IT_ID:
    ret = it->impl.pptr.ksize;
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    ret = stdhash_it_key_size(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = stdskl_it_key_size(it);
    break;

  default:
    ret = 0;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_val: Get the value that 'it' references.
 ***********************************************************************************************/

STDINLINE void *stdit_val(const stdit *it)
{
  void * ret;

  switch (it->type_id) {
  case STDPTR_IT_ID:
    ret = it->impl.ptr.val;
    break;

  case STDPPTR_IT_ID:
    ret = it->impl.pptr.val;
    break;

  case STDARR_IT_ID:
    ret = stdarr_it_val(it);
    break;

  case STDCARR_IT_ID:
    ret = stdcarr_it_val(it);
    break;

  case STDDLL_IT_ID:
    ret = stddll_it_val(it);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    ret = stdhash_it_val(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = stdskl_it_val(it);
    break;

  default:
    ret = NULL;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_val_size: Get the size in bytes of the value 'it' references.
 ***********************************************************************************************/

STDINLINE stdsize stdit_val_size(const stdit *it)
{
  stdsize ret;

  switch (it->type_id) {
  case STDPTR_IT_ID:
    ret = it->impl.ptr.vsize;
    break;

  case STDPPTR_IT_ID:
    ret = it->impl.pptr.vsize;
    break;

  case STDARR_IT_ID:
    ret = stdarr_it_val_size(it);
    break;

  case STDCARR_IT_ID:
    ret = stdcarr_it_val_size(it);
    break;

  case STDDLL_IT_ID:
    ret = stddll_it_val_size(it);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    ret = stdhash_it_val_size(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = stdskl_it_val_size(it);
    break;

  default:
    ret = 0;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_eq: Compare two iterators for equality (reference same element).
 ***********************************************************************************************/

STDINLINE stdbool stdit_eq(const stdit *it1, const stdit *it2)
{
  stdbool ret;

  STDSAFETY_CHECK(it1->type_id == it2->type_id);

  switch (it1->type_id) {
  case STDPTR_IT_ID:
    STDSAFETY_CHECK(it1->impl.ptr.vsize == it2->impl.ptr.vsize);
    ret = (it1->impl.ptr.val == it2->impl.ptr.val);
    break;

  case STDPPTR_IT_ID:
    STDSAFETY_CHECK(it1->impl.pptr.ksize == it2->impl.pptr.ksize && it1->impl.pptr.vsize == it2->impl.pptr.vsize);
    ret = (it1->impl.pptr.key == it2->impl.pptr.key && it1->impl.pptr.val == it2->impl.pptr.val);
    break;

  case STDARR_IT_ID:
    ret = stdarr_it_eq(it1, it2);
    break;

  case STDCARR_IT_ID:
    ret = stdcarr_it_eq(it1, it2);
    break;

  case STDDLL_IT_ID:
    ret = stddll_it_eq(it1, it2);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    ret = stdhash_it_eq(it1, it2);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = stdskl_it_eq(it1, it2);
    break;

  default:
    ret = STDFALSE;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_next: Advance an iterator towards "end" by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdit_next(stdit *it)
{
  switch (it->type_id) {
  case STDPTR_IT_ID:
    it->impl.ptr.val += it->impl.ptr.vsize;
    break;

  case STDPPTR_IT_ID:
    it->impl.pptr.key += it->impl.pptr.ksize;
    it->impl.pptr.val += it->impl.pptr.vsize;
    break;

  case STDARR_IT_ID:
    stdarr_it_next(it);
    break;

  case STDCARR_IT_ID:
    stdcarr_it_next(it);
    break;

  case STDDLL_IT_ID:
    stddll_it_next(it);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    stdhash_it_next(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    stdskl_it_next(it);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return it;
}

/************************************************************************************************
 * stdit_advance: Advance an iterator towards "end" by num_advance positions.
 ***********************************************************************************************/

STDINLINE stdit *stdit_advance(stdit *it, stdsize num_advance)
{
  switch (it->type_id) {
  case STDPTR_IT_ID:
    it->impl.ptr.val += it->impl.ptr.vsize * num_advance;
    break;

  case STDPPTR_IT_ID:
    it->impl.pptr.key += it->impl.pptr.ksize * num_advance;
    it->impl.pptr.val += it->impl.pptr.vsize * num_advance;
    break;

  case STDARR_IT_ID:
    stdarr_it_advance(it, num_advance);
    break;

  case STDCARR_IT_ID:
    stdcarr_it_advance(it, num_advance);
    break;

  case STDDLL_IT_ID:
    stddll_it_advance(it, num_advance);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    stdhash_it_advance(it, num_advance);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    stdskl_it_advance(it, num_advance);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return it;
}

/************************************************************************************************
 * stdit_distance: Calculate the distance from b to e.
 ***********************************************************************************************/

STDINLINE stdssize stdit_distance(const stdit *b, const stdit *e)
{
  stdsize ret  = 0;
  stdit   curr = *b;

  switch (b->type_id) {
  case STDPTR_IT_ID:
  case STDPPTR_IT_ID:
  case STDARR_IT_ID:
  case STDCARR_IT_ID:
    ret = stdit_cmp(e, b);
    break;

  case STDDLL_IT_ID:
    for (; !stddll_it_eq(&curr, e); stddll_it_next(&curr), ++ret);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    for (; !stdhash_it_eq(&curr, e); stdhash_it_next(&curr), ++ret);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    for (; !stdskl_it_eq(&curr, e); stdskl_it_next(&curr), ++ret);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_prev: Advance an iterator towards "begin" by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdit_prev(stdit *it)
{
  switch (it->type_id) {
  case STDPTR_IT_ID:
    it->impl.ptr.val -= it->impl.ptr.vsize;
    break;

  case STDPPTR_IT_ID:
    it->impl.pptr.key -= it->impl.pptr.ksize;
    it->impl.pptr.val -= it->impl.pptr.vsize;
    break;

  case STDARR_IT_ID:
    stdarr_it_prev(it);
    break;

  case STDCARR_IT_ID:
    stdcarr_it_prev(it);
    break;

  case STDDLL_IT_ID:
    stddll_it_prev(it);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    stdhash_it_prev(it);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    stdskl_it_prev(it);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return it;
}

/************************************************************************************************
 * stdit_retreat: Advance an iterator towards "begin" by num_retreat positions.
 ***********************************************************************************************/

STDINLINE stdit *stdit_retreat(stdit *it, stdsize num_retreat)
{
  switch (it->type_id) {
  case STDPTR_IT_ID:
    it->impl.ptr.val -= it->impl.ptr.vsize * num_retreat;
    break;

  case STDPPTR_IT_ID:
    it->impl.pptr.key -= it->impl.pptr.ksize * num_retreat;
    it->impl.pptr.val -= it->impl.pptr.vsize * num_retreat;
    break;

  case STDARR_IT_ID:
    stdarr_it_retreat(it, num_retreat);
    break;

  case STDCARR_IT_ID:
    stdcarr_it_retreat(it, num_retreat);
    break;

  case STDDLL_IT_ID:
    stddll_it_retreat(it, num_retreat);
    break;

  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
    stdhash_it_retreat(it, num_retreat);
    break;

  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    stdskl_it_retreat(it, num_retreat);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return it;
}

/************************************************************************************************
 * stdit_cmp: Compare two iterators for rank difference.
 ***********************************************************************************************/

STDINLINE stdssize stdit_cmp(const stdit *it1, const stdit *it2)
{
  stdssize ret;

  STDSAFETY_CHECK(it1->type_id == it2->type_id);

  switch (it1->type_id) {
  case STDPTR_IT_ID:
    STDSAFETY_CHECK(it1->impl.ptr.vsize == it2->impl.ptr.vsize);
    ret = (it1->impl.ptr.val - it2->impl.ptr.val) / it1->impl.ptr.vsize;
    break;

  case STDPPTR_IT_ID:
    STDSAFETY_CHECK(it1->impl.pptr.ksize == it2->impl.pptr.ksize && it1->impl.pptr.vsize == it2->impl.pptr.vsize);
    ret = (it1->impl.pptr.key - it2->impl.pptr.key) / it1->impl.pptr.ksize;
    STDSAFETY_CHECK(ret == (it1->impl.pptr.val - it2->impl.pptr.val) / it1->impl.pptr.vsize);
    break;

  case STDARR_IT_ID:
    ret = stdarr_it_cmp(it1, it2);
    break;

  case STDCARR_IT_ID:
    ret = stdcarr_it_cmp(it1, it2);
    break;

  case STDDLL_IT_ID:
  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    ret = 0;
    STDEXCEPTION(iterator type does not support stdit_cmp);
    break;

  default:
    ret = 0;
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return ret;
}

/************************************************************************************************
 * stdit_offset: Advance an iterator towards "end" by 'offset' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdit_offset(stdit *it, stdssize offset)
{
  switch (it->type_id) {
  case STDPTR_IT_ID:
    it->impl.ptr.val += it->impl.ptr.vsize * offset;
    break;

  case STDPPTR_IT_ID:
    it->impl.pptr.key += it->impl.pptr.ksize * offset;
    it->impl.pptr.val += it->impl.pptr.vsize * offset;
    break;

  case STDARR_IT_ID:
    stdarr_it_offset(it, offset);
    break;

  case STDCARR_IT_ID:
    stdcarr_it_offset(it, offset);
    break;

  case STDDLL_IT_ID:
  case STDHASH_IT_ID:
  case STDHASH_IT_KEY_ID:
  case STDSKL_IT_ID:
  case STDSKL_IT_KEY_ID:
    STDEXCEPTION(iterator type does not support stdit_offset);
    break;

  default:
    STDEXCEPTION(uninitialized or corrupted iterator);
    break;
  }

  return it;
}

/************************************************************************************************
 * stdit_ptr: Initialize a stdit from a pointer.
 ***********************************************************************************************/

STDINLINE stdit *stdit_ptr(stdit *it, const void * val, stdsize vsize)
{
  it->impl.ptr.val   = (char*) val;
  it->impl.ptr.vsize = vsize;

  it->type_id = STDPTR_IT_ID;

  return it;
}

/************************************************************************************************
 * stdit_pptr: Initialize a stdit from two parallel pointers.
 ***********************************************************************************************/

STDINLINE stdit *stdit_pptr(stdit *it, const void *key, const void * val, stdsize ksize, stdsize vsize)
{
  it->impl.pptr.key   = (char*) key;
  it->impl.pptr.ksize = ksize;

  it->impl.pptr.val   = (char*) val;
  it->impl.pptr.vsize = vsize;

  it->type_id = STDPPTR_IT_ID;

  return it;
}

#ifdef __cplusplus
}
#endif
