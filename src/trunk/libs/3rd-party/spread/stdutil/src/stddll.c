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
#include <stdutil/stddll.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDDLL_IS_LEGAL(l)        ((l)->end_node != NULL && (l)->vsize != 0)
#define STDDLL_IT_IS_LEGAL(l, it) ((it)->end_node == (l)->end_node && (it)->vsize == (l)->vsize)
#define STDDLL_IT_IS_LEGAL2(it)   ((it)->node != NULL && (it)->end_node != NULL && (it)->vsize != 0)
#define STDIT_DLL_IS_LEGAL(it)    ((it)->type_id == STDDLL_IT_ID && STDDLL_IT_IS_LEGAL2(&(it)->impl.dll))

#define STDDLL_NODE_SIZE(vsize) (STDARCH_PADDED_SIZE(sizeof(stddll_node)) + (vsize))
#define STDDLL_NVAL(node_ptr)   ((char*) (node_ptr) + STDARCH_PADDED_SIZE(sizeof(stddll_node)))

#define STDDLL_LBEGIN(list_ptr) ((list_ptr)->end_node->next)
#define STDDLL_LEND(list_ptr)   ((list_ptr)->end_node)

/************************************************************************************************
 * stddll_low_alloc_chain: This fcn allocates a non-zero length
 * sequence of (optionally initialized) linked nodes. It returns
 * pointers to the first and last nodes. The sequence is null
 * terminated on both ends.  If (init) the sequence will be
 * initialized by the sequence starting at 'b.'  If (!advnc) then the
 * initializing sequnce will not be advanced (i.e. - repeat insert).
 ***********************************************************************************************/

STDINLINE static stdcode stddll_low_alloc_chain(stdsize num_ins, stdsize vsize, stdbool init, const stdit *b, stdbool advnc,
						stddll_node **first, stddll_node **last)
{
  stdcode       ret    = STDESUCCESS;
  stdit         src_it = *b;
  const void *  val    = NULL;
  stddll_node * prev   = NULL;
  stddll_node * curr;
 
  if ((curr = (stddll_node*) malloc(STDDLL_NODE_SIZE(vsize))) == NULL) {  /* append values on to end of node in memory */
    ret = STDENOMEM;
    goto stddll_low_alloc_chain_end;
  }

  if (init) {
    val = stdit_val(&src_it);
    memcpy(STDDLL_NVAL(curr), val, vsize);
  }

  *first     = curr;
  curr->prev = NULL;

  while (--num_ins != 0) {
    prev = curr;

    if ((curr = (stddll_node*) malloc(STDDLL_NODE_SIZE(vsize))) == NULL) {
      ret = STDENOMEM;
      goto stddll_low_alloc_chain_fail;
    }

    if (init) {

      if (advnc) {
	stdit_next(&src_it);
	val = stdit_val(&src_it);
      }

      memcpy(STDDLL_NVAL(curr), val, vsize);      
    }

    prev->next = curr;
    curr->prev = prev;
  }

  curr->next = NULL;
  *last = curr;

  goto stddll_low_alloc_chain_end;

  /* error handling and return */

 stddll_low_alloc_chain_fail:
  while (prev != NULL) {
    curr = prev->prev;
    free(prev);
    prev = curr;
  }

  *first = NULL;
  *last  = NULL;

 stddll_low_alloc_chain_end:
  return ret;
}

/************************************************************************************************
 * stddll_low_insert: This fcn allocates and inserts a sequence of
 * initialized nodes into a list before next.
 ***********************************************************************************************/

STDINLINE static stdcode stddll_low_insert(stddll *l, stdsize num_ins, stdbool init, const stdit *b, stdbool advnc, 
					   stddll_node *next, stddll_node **first_ptr)
{
  stdcode       ret   = STDESUCCESS;
  stddll_node * first = NULL;
  stddll_node * last;
  stddll_node * prev;

  if (num_ins == 0 ||
      (ret = stddll_low_alloc_chain(num_ins, l->vsize, init, b, advnc, &first, &last)) != STDESUCCESS) {
    goto stddll_low_insert_end;
  }

  prev        = next->prev;
  prev->next  = first;
  first->prev = prev;
  last->next  = next;
  next->prev  = last;

  l->size += num_ins;

  if (first_ptr != NULL) {
    *first_ptr = first;
  }

 stddll_low_insert_end:
  return ret;
}

/************************************************************************************************
 * stddll_low_erase: Erase a subsequence of the list starting at
 * erase_begin of length num_erase. It returns a pointer to the node
 * that is shifted into the position of erase_begin after the removal.
 ***********************************************************************************************/

STDINLINE static stddll_node *stddll_low_erase(stddll *l, stddll_node *erase_begin, stdsize num_erase) 
{
  stddll_node * curr = erase_begin;
  stddll_node * prev;
  stdsize       ne   = num_erase;

  erase_begin = erase_begin->prev;            /* get last node before erase region */

  while (ne-- != 0) {
    STDBOUNDS_CHECK(curr != STDDLL_LEND(l));  /* check for an illegal erasure */
    prev = curr;
    curr = curr->next;
    free(prev);
  }

  erase_begin->next = curr;
  curr->prev        = erase_begin;

  l->size -= num_erase;

  return curr;
}

/************************************************************************************************
 * stddll_low_erase_seq: Erase a subsequence of the list starting at
 * erase_begin and going to erase_end. It returns a pointer to the
 * node that is shifted into the position of erase_begin after the
 * removal.
 ***********************************************************************************************/

STDINLINE static stddll_node *stddll_low_erase_seq(stddll *l, stddll_node *erase_begin, stddll_node *erase_end)
{
  stddll_node * curr = erase_begin;
  stddll_node * prev;
  stdsize       num_erase;

  erase_begin = erase_begin->prev;            /* get last node before erase region */

  for (num_erase = 0; curr != erase_end; ++num_erase) {
    STDBOUNDS_CHECK(curr != STDDLL_LEND(l));  /* check for an illegal erasure */
    prev = curr;
    curr = curr->next;
    free(prev);
  }

  erase_begin->next = curr;
  curr->prev        = erase_begin;

  l->size -= num_erase;

  return curr;
}

/************************************************************************************************
 * stddll_low_rerase: Same as stddll_low_erase, except the erase
 * region ends with erase_end and is of length num_erase.
 ***********************************************************************************************/

STDINLINE static stddll_node *stddll_low_rerase(stddll *l, stddll_node *erase_end, stdsize num_erase) 
{
  stddll_node * curr = erase_end->prev;
  stddll_node * prev;
  stdsize       ne   = num_erase;

  while (ne-- != 0) {
    STDBOUNDS_CHECK(curr != STDDLL_LEND(l));  /* check for an illegal erasure */
    prev = curr;
    curr = curr->prev;
    free(prev);
  }

  curr->next      = erase_end;
  erase_end->prev = curr;

  l->size -= num_erase;

  return erase_end;
}

/************************************************************************************************
 * stddll_construct: Construct an initially empty list that will
 * contain elements sizeof_val bytes long.
 ***********************************************************************************************/

STDINLINE stdcode stddll_construct(stddll *l, stdsize vsize) 
{
  stdcode ret = STDESUCCESS;

  if (vsize == 0) {
    ret = STDEINVAL;
    goto stddll_construct_fail;
  }

  if ((l->end_node = (stddll_node*) malloc(STDDLL_NODE_SIZE(vsize))) == NULL) {
    ret = STDENOMEM;
    goto stddll_construct_fail;
  }

  l->end_node->prev = l->end_node;
  l->end_node->next = l->end_node;
  
  l->size  = 0;
  l->vsize = vsize;

  goto stddll_construct_end;

  /* error handling and return */

 stddll_construct_fail:
  l->end_node = NULL;
  l->vsize    = 0;  /* make STDDLL_IS_LEGAL(l) false */

 stddll_construct_end:
  return ret;
}

/************************************************************************************************
 * stddll_copy_construct: Construct a copy of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_copy_construct(stddll *dst, const stddll *src) 
{
  stdcode ret;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(src));

  if ((ret = stddll_construct(dst, src->vsize)) != STDESUCCESS) {
    goto stddll_copy_construct_end;
  }

  if ((ret = stddll_set_eq(dst, src)) != STDESUCCESS) {
    goto stddll_copy_construct_fail;
  }

  goto stddll_copy_construct_end;

  /* error handling and return */

 stddll_copy_construct_fail:
  stddll_destruct(dst);

 stddll_copy_construct_end:
  return ret;
}

/************************************************************************************************
 * stddll_destruct: Reclaim a list's resources and invalidate it.
 ***********************************************************************************************/

STDINLINE void stddll_destruct(stddll *l) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  stddll_clear(l);
  free(l->end_node);

  l->end_node = NULL;
  l->vsize    = 0;     /* make STDDLL_IS_LEGAL(l) false */
}

/************************************************************************************************
 * stddll_set_eq: Set 'dst' to have the same contents as 'src.'
 ***********************************************************************************************/

STDINLINE stdcode stddll_set_eq(stddll *dst, const stddll *src)
{
  stdcode       ret = STDESUCCESS;
  stddll_node * dst_node;
  stddll_node * src_node;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(dst) && STDDLL_IS_LEGAL(src) && dst->vsize == src->vsize);

  if (dst == src) {
    goto stddll_set_eq_end;
  }

  if ((ret = stddll_resize(dst, src->size)) != STDESUCCESS) {
    goto stddll_set_eq_end;
  }

  dst_node = STDDLL_LBEGIN(dst);
  src_node = STDDLL_LBEGIN(src);

  while (dst_node != STDDLL_LEND(dst)) {
    memcpy(STDDLL_NVAL(dst_node), STDDLL_NVAL(src_node), dst->vsize);
    dst_node = dst_node->next; 
    src_node = src_node->next;
  }

 stddll_set_eq_end:
  return ret;
}

/************************************************************************************************
 * stddll_swap: Make l1 reference l2's sequence and vice versa.
 ***********************************************************************************************/

STDINLINE void stddll_swap(stddll *l1, stddll *l2)
{
  stddll cpy;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l1) && STDDLL_IS_LEGAL(l2) && l1->vsize == l2->vsize);

  STDSWAP(*l1, *l2, cpy);
}

/************************************************************************************************
 * stddll_begin: Get an iterator the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdit *stddll_begin(const stddll *l, stdit *it) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  it->type_id           = STDDLL_IT_ID;
  it->impl.dll.node     = (stddll_node*) STDDLL_LBEGIN(l);
  it->impl.dll.end_node = (stddll_node*) STDDLL_LEND(l);
  it->impl.dll.vsize    = l->vsize;

  return it;
}

/************************************************************************************************
 * stddll_last: Get an iterator the last element of a list.
 ***********************************************************************************************/

STDINLINE stdit *stddll_last(const stddll *l, stdit *it) 
{
  STDBOUNDS_CHECK(l->size != 0);

  return stdit_prev(stddll_end(l, it));
}

/************************************************************************************************
 * stddll_end: Get an iterator to the sentinel 'end' entry of a list.
 ***********************************************************************************************/

STDINLINE stdit *stddll_end(const stddll *l, stdit *it) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  it->type_id           = STDDLL_IT_ID;
  it->impl.dll.node     = (stddll_node*) STDDLL_LEND(l);
  it->impl.dll.end_node = (stddll_node*) STDDLL_LEND(l);
  it->impl.dll.vsize    = l->vsize;

  return it;
}

/************************************************************************************************
 * stddll_get: Get an iterator to the 'elem_num'th (0 based) entry of
 * a list.  Can request/return sentinel end position.
 ***********************************************************************************************/

STDINLINE stdit *stddll_get(const stddll *l, stdit *it, stdsize elem_num) 
{
  STDBOUNDS_CHECK(elem_num <= l->size);

  if (elem_num < (l->size >> 1)) {
    stddll_it_advance(stddll_begin(l, it), elem_num);

  } else {
    stddll_it_retreat(stddll_end(l, it), l->size - elem_num);
  }

  return it;
}

/************************************************************************************************
 * stddll_is_begin: Returns whether or not an iterator refers to the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdbool stddll_is_begin(const stddll *l, const stdit *it) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  return it->impl.dll.node == STDDLL_LBEGIN(l);
}

/************************************************************************************************
 * stddll_is_end: Returns whether or not an iterator refers to the end of a list.
 ***********************************************************************************************/

STDINLINE stdbool stddll_is_end(const stddll *l, const stdit *it) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  return it->impl.dll.node == STDDLL_LEND(l);
}

/************************************************************************************************
 * stddll_size: Return the number of elements in 'l.'
 ***********************************************************************************************/

STDINLINE stdsize stddll_size(const stddll *l)
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  return l->size;
}

/************************************************************************************************
 * stddll_empty: Return whether or not 'l' is empty.
 ***********************************************************************************************/

STDINLINE stdbool stddll_empty(const stddll *l) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  return l->size == 0;
}

/************************************************************************************************
 * stddll_max_size: Return the theoretical maximum number of elements
 * 'l' could possibly hold.
 ***********************************************************************************************/

STDINLINE stdsize stddll_max_size(const stddll *l) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  return STDSIZE_MAX;
}

/************************************************************************************************
 * stddll_val_size: Return the size in bytes of the type of elements
 * 'l' holds.
 ***********************************************************************************************/

STDINLINE stdsize stddll_val_size(const stddll *l) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  return l->vsize;
}

/************************************************************************************************
 * stddll_resize: Resize a list to contain 'num_elems' elements.
 ***********************************************************************************************/

STDINLINE stdcode stddll_resize(stddll *l, stdsize num_elems) 
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));

  if (num_elems > l->size) {

    if ((ret = stddll_low_insert(l, num_elems - l->size, STDFALSE, NULL, STDFALSE, STDDLL_LEND(l), NULL)) != STDESUCCESS) {
      goto stddll_resize_end;
    }

  } else if (num_elems < l->size) {
    stddll_low_rerase(l, STDDLL_LEND(l), l->size - num_elems);
  }

 stddll_resize_end:
  return ret;
}

/************************************************************************************************
 * stddll_clear: Set a list's size to zero.
 ***********************************************************************************************/

STDINLINE void stddll_clear(stddll *l) 
{
  stddll_resize(l, 0);
}

/************************************************************************************************
 * stddll_push_front: Push an element onto the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_front(stddll *l, const void *val) 
{
  return stddll_push_front_n(l, val, 1);
}

/************************************************************************************************
 * stddll_push_front_n: Push multiple elements onto the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_front_n(stddll *l, const void *vals, stdsize num_push) 
{
  stdit it;

  return stddll_insert_n(l, stddll_begin(l, &it), vals, num_push);
}

/************************************************************************************************
 * stddll_push_front_seq: Push a sequence of elements onto the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_front_seq(stddll *l, const stdit *b, const stdit *e)
{
  stdit it;

  return stddll_insert_seq(l, stddll_begin(l, &it), b, e);
}

/************************************************************************************************
 * stddll_push_front_seq_n: Push multiple elements onto the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_front_seq_n(stddll *l, const stdit *b, stdsize num_push)
{
  stdit it;

  return stddll_insert_seq_n(l, stddll_begin(l, &it), b, num_push);
}

/************************************************************************************************
 * stddll_push_front_rep: Push multiple elements onto the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_front_rep(stddll *l, const void *val, stdsize num_times) 
{
  stdit it;

  return stddll_insert_rep(l, stddll_begin(l, &it), val, num_times);
}

/************************************************************************************************
 * stddll_pop_front: Pop an element off of the beginning of a list.
 ***********************************************************************************************/

STDINLINE void stddll_pop_front(stddll *l) 
{
  stddll_pop_front_n(l, 1);
}

/************************************************************************************************
 * stddll_pop_front: Pop multiple elements off of the front of a list.
 ***********************************************************************************************/

STDINLINE void stddll_pop_front_n(stddll *l, stdsize num_pop) 
{
  stdit it;

  stddll_erase_n(l, stddll_begin(l, &it), num_pop);
}

/************************************************************************************************
 * stddll_push_back: Push an element onto the end of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_back(stddll *l, const void *val) 
{
  return stddll_push_back_n(l, val, 1);
}

/************************************************************************************************
 * stddll_push_back_n: Push multiple elements onto the end of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_back_n(stddll *l, const void *vals, stdsize num_push) 
{
  stdit it;

  return stddll_insert_n(l, stddll_end(l, &it), vals, num_push);
}

/************************************************************************************************
 * stddll_push_back_seq: Push multiple elements onto the end of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_back_seq(stddll *l, const stdit *b, const stdit *e) 
{
  stdit it;

  return stddll_insert_seq(l, stddll_end(l, &it), b, e);
}

/************************************************************************************************
 * stddll_push_back_seq_n: Push multiple elements onto the end of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_back_seq_n(stddll *l, const stdit *b, stdsize num_push) 
{
  stdit it;

  return stddll_insert_seq_n(l, stddll_end(l, &it), b, num_push);
}

/************************************************************************************************
 * stddll_push_back_rep: Push multiple elements onto the end of a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_push_back_rep(stddll *l, const void *val, stdsize num_times) 
{
  stdit it;

  return stddll_insert_rep(l, stddll_end(l, &it), val, num_times);
}

/************************************************************************************************
 * stddll_pop_back: Pop an element off of the end of a list.
 ***********************************************************************************************/

STDINLINE void stddll_pop_back(stddll *l) 
{
  stddll_pop_back_n(l, 1);
}

/************************************************************************************************
 * stddll_pop_back_n: Pop multiple elements off of the back of a list.
 ***********************************************************************************************/

STDINLINE void stddll_pop_back_n(stddll *l, stdsize num_pop) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l));
  STDBOUNDS_CHECK(num_pop <= stddll_size(l));

  stddll_low_rerase(l, STDDLL_LEND(l), num_pop);
}

/************************************************************************************************
 * stddll_insert: Insert an element into a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_insert(stddll *l, stdit *it, const void *val) 
{
  return stddll_insert_n(l, it, val, 1);
}

/************************************************************************************************
 * stddll_insert_n: Insert multiple elements into a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_insert_n(stddll *l, stdit *it, const void *vals, stdsize num_insert) 
{
  stdit vals_it;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  return stddll_low_insert(l, num_insert, STDTRUE, stdit_ptr(&vals_it, vals, l->vsize), 
			   STDTRUE, it->impl.dll.node, &it->impl.dll.node);
}

/************************************************************************************************
 * stddll_insert_seq: Insert multiple elements into a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_insert_seq(stddll *l, stdit *it, const stdit *b, const stdit *e)
{
  stdcode  ret;
  stdssize num_ins = stdit_distance(b, e);

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  if (num_ins < 0) {
    ret = STDEINVAL;
    goto stddll_insert_seq_end;
  }

  ret = stddll_low_insert(l, num_ins, STDTRUE, b, STDTRUE, it->impl.dll.node, &it->impl.dll.node);

 stddll_insert_seq_end:
  return ret;
}

/************************************************************************************************
 * stddll_insert_seq_n: Insert multiple elements into a list.
 ***********************************************************************************************/

STDINLINE stdcode stddll_insert_seq_n(stddll *l, stdit *it, const stdit *b, stdsize num_insert)
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  return stddll_low_insert(l, num_insert, STDTRUE, b, STDTRUE, it->impl.dll.node, &it->impl.dll.node);
}

/************************************************************************************************
 * stddll_insert_rep: Insert an item into a list repeatedly.
 ***********************************************************************************************/

STDINLINE stdcode stddll_insert_rep(stddll *l, stdit *it, const void *val, stdsize num_times) 
{
  stdit val_it;

  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  return stddll_low_insert(l, num_times, STDTRUE, stdit_ptr(&val_it, val, l->vsize), 
			   STDFALSE, it->impl.dll.node, &it->impl.dll.node);
}

/************************************************************************************************
 * stddll_erase: Erase a particular element from a list.
 ***********************************************************************************************/

STDINLINE void stddll_erase(stddll *l, stdit *it) 
{
  stddll_erase_n(l, it, 1);
}

/************************************************************************************************
 * stddll_erase_n: Erase multiple elements from a list.
 ***********************************************************************************************/

STDINLINE void stddll_erase_n(stddll *l, stdit *it, stdsize num_erase) 
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(it) && STDDLL_IT_IS_LEGAL(l, &it->impl.dll));

  it->impl.dll.node = stddll_low_erase(l, it->impl.dll.node, num_erase);
}

/************************************************************************************************
 * stddll_erase_seq: Erase multiple elements from a list.
 ***********************************************************************************************/

STDINLINE void stddll_erase_seq(stddll *l, stdit *b, stdit *e)
{
  STDSAFETY_CHECK(STDDLL_IS_LEGAL(l) && STDIT_DLL_IS_LEGAL(b) && STDIT_DLL_IS_LEGAL(e) && 
		  STDDLL_IT_IS_LEGAL(l, &b->impl.dll) && STDDLL_IT_IS_LEGAL(l, &e->impl.dll));

  b->impl.dll.node = stddll_low_erase_seq(l, b->impl.dll.node, e->impl.dll.node);  
  *e = *b;
}

/************************************************************************************************
 * stddll_it_val: Return a pointer to an element from an iterator. 
 ***********************************************************************************************/

STDINLINE void *stddll_it_val(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));

  return (void*) STDDLL_NVAL(it->impl.dll.node);
}

/************************************************************************************************
 * stddll_it_val_size: Return the size of the referenced type from an iterator. 
 ***********************************************************************************************/

STDINLINE stdsize stddll_it_val_size(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));

  return it->impl.dll.vsize;
}

/************************************************************************************************
 * stddll_it_eq: Compare to iterators for equality (refer to the same element).
 ***********************************************************************************************/

STDINLINE stdbool stddll_it_eq(const stdit *it1, const stdit *it2) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it1) && STDIT_DLL_IS_LEGAL(it2) &&
		  it1->impl.dll.end_node == it2->impl.dll.end_node && 
		  it1->impl.dll.vsize == it2->impl.dll.vsize);

  return it1->impl.dll.node == it2->impl.dll.node;
}

/************************************************************************************************
 * stddll_it_next: Advance an iterator one position towards the end.
 ***********************************************************************************************/

STDINLINE stdit *stddll_it_next(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.dll.node != it->impl.dll.end_node);

  it->impl.dll.node = it->impl.dll.node->next;

  return it;
}

/************************************************************************************************
 * stddll_it_advance: Advance an iterator 'num_advance' positions towards the end.
 ***********************************************************************************************/

STDINLINE stdit *stddll_it_advance(stdit *it, stdsize num_advance) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));

  while (num_advance-- != 0) {
    STDBOUNDS_CHECK(it->impl.dll.node != it->impl.dll.end_node);
    it->impl.dll.node = it->impl.dll.node->next;
  }

  return it;
}

/************************************************************************************************
 * stddll_it_prev: Advance an iterator one postion towards the beginning.
 ***********************************************************************************************/

STDINLINE stdit *stddll_it_prev(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.dll.node->prev != it->impl.dll.end_node);

  it->impl.dll.node = it->impl.dll.node->prev;

  return it;
}

/************************************************************************************************
 * stddll_it_retreat: Advance an iterator 'num_retreat' positions towards the beginning.
 ***********************************************************************************************/

STDINLINE stdit *stddll_it_retreat(stdit *it, stdsize num_retreat) 
{
  STDSAFETY_CHECK(STDIT_DLL_IS_LEGAL(it));

  while (num_retreat-- != 0) {
    STDBOUNDS_CHECK(it->impl.dll.node->prev != it->impl.dll.end_node);
    it->impl.dll.node = it->impl.dll.node->prev;
  }

  return it;
}

#ifdef __cplusplus
}
#endif
