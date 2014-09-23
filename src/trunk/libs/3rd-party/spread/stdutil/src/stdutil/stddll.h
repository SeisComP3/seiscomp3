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

#ifndef stddll_h_2000_02_14_16_22_38_jschultz_at_cnds_jhu_edu
#define stddll_h_2000_02_14_16_22_38_jschultz_at_cnds_jhu_edu

#include <stdutil/stdit.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structors */

STDINLINE stdcode stddll_construct(stddll *l, stdsize vsize);
STDINLINE stdcode stddll_copy_construct(stddll *dst, const stddll *src);
STDINLINE void    stddll_destruct(stddll *l);

/* Assigners */

STDINLINE stdcode stddll_set_eq(stddll *dst, const stddll *src);
STDINLINE void    stddll_swap(stddll *l1, stddll *l2);

/* Iterators */

STDINLINE stdit * stddll_begin(const stddll *l, stdit *it);
STDINLINE stdit * stddll_last(const stddll *l, stdit *it);
STDINLINE stdit * stddll_end(const stddll *l, stdit *it);
STDINLINE stdit * stddll_get(const stddll *l, stdit *it, stdsize elem_num);  /* O(n) */

STDINLINE stdbool stddll_is_begin(const stddll *l, const stdit *it);
STDINLINE stdbool stddll_is_end(const stddll *l, const stdit *it);  

/* Size Information */

STDINLINE stdsize stddll_size(const stddll *l);
STDINLINE stdbool stddll_empty(const stddll *l);

STDINLINE stdsize stddll_max_size(const stddll *l);
STDINLINE stdsize stddll_val_size(const stddll *l);

/* Size Operations */

STDINLINE stdcode stddll_resize(stddll *l, stdsize num_elems);
STDINLINE void    stddll_clear(stddll *l);

/* Stack Operations: O(1) operations */

STDINLINE stdcode stddll_push_front(stddll *l, const void *val);
STDINLINE stdcode stddll_push_front_n(stddll *l, const void *vals, stdsize num_push);
STDINLINE stdcode stddll_push_front_seq(stddll *l, const stdit *b, const stdit *e);
STDINLINE stdcode stddll_push_front_seq_n(stddll *l, const stdit *b, stdsize num_push);
STDINLINE stdcode stddll_push_front_rep(stddll *l, const void *val, stdsize num_times);

STDINLINE void    stddll_pop_front(stddll *l);
STDINLINE void    stddll_pop_front_n(stddll *l, stdsize num_pop);

STDINLINE stdcode stddll_push_back(stddll *l, const void *val);
STDINLINE stdcode stddll_push_back_n(stddll *l, const void *vals, stdsize num_push);
STDINLINE stdcode stddll_push_back_seq(stddll *l, const stdit *b, const stdit *e);
STDINLINE stdcode stddll_push_back_seq_n(stddll *l, const stdit *b, stdsize num_push);
STDINLINE stdcode stddll_push_back_rep(stddll *l, const void *val, stdsize num_times);

STDINLINE void    stddll_pop_back(stddll *l);
STDINLINE void    stddll_pop_back_n(stddll *l, stdsize num_pop);

/* List Operations: O(1) operations */

STDINLINE stdcode stddll_insert(stddll *l, stdit *it, const void *val);
STDINLINE stdcode stddll_insert_n(stddll *l, stdit *it, const void *vals, stdsize num_insert);
STDINLINE stdcode stddll_insert_seq(stddll *l, stdit *it, const stdit *b, const stdit *e);
STDINLINE stdcode stddll_insert_seq_n(stddll *l, stdit *it, const stdit *b, stdsize num_insert);
STDINLINE stdcode stddll_insert_rep(stddll *l, stdit *it, const void *val, stdsize num_times);

STDINLINE void    stddll_erase(stddll *l, stdit *it);
STDINLINE void    stddll_erase_n(stddll *l, stdit *it, stdsize num_erase);
STDINLINE void    stddll_erase_seq(stddll *l, stdit *b, stdit *e);

/* Iterator Fcns */

STDINLINE void *  stddll_it_val(const stdit *it);
STDINLINE stdsize stddll_it_val_size(const stdit *it);
STDINLINE stdbool stddll_it_eq(const stdit *it1, const stdit *it2);

STDINLINE stdit * stddll_it_next(stdit *it);
STDINLINE stdit * stddll_it_advance(stdit *it, stdsize num_advance);
STDINLINE stdit * stddll_it_prev(stdit *it);
STDINLINE stdit * stddll_it_retreat(stdit *it, stdsize num_retreat);

#ifdef __cplusplus
}
#endif

#endif
