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

#ifndef stdarr_h_2000_01_26_11_38_04_jschultz_at_cnds_jhu_edu
#define stdarr_h_2000_01_26_11_38_04_jschultz_at_cnds_jhu_edu

#include <stdutil/stdit.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDARR_MIN_AUTO_ALLOC    /* minimum allocation size in # of elements */
#  define STDARR_MIN_AUTO_ALLOC 16
#endif

/* Structors */

#define STDARR_STATIC_CONSTRUCT(vsize, opts) { NULL, NULL, 0, 0, (vsize), (opts) }

STDINLINE stdcode  stdarr_construct(stdarr *arr, stdsize vsize, stduint8 opts);
STDINLINE stdcode  stdarr_copy_construct(stdarr *dst, const stdarr *src);
STDINLINE void     stdarr_destruct(stdarr *arr);

/* Assigners */

STDINLINE stdcode  stdarr_set_eq(stdarr *dst, const stdarr *src);
STDINLINE void     stdarr_swap(stdarr *arr1, stdarr *arr2);

/* Iterators */

STDINLINE stdit *  stdarr_begin(const stdarr *arr, stdit *it);
STDINLINE stdit *  stdarr_last(const stdarr *arr, stdit *it);
STDINLINE stdit *  stdarr_end(const stdarr *arr, stdit *it);
STDINLINE stdit *  stdarr_get(const stdarr *arr, stdit *it, stdsize elem_num);

STDINLINE stdbool  stdarr_is_begin(const stdarr *arr, const stdit *it);
STDINLINE stdbool  stdarr_is_end(const stdarr *arr, const stdit *it);
STDINLINE stdsize  stdarr_rank(const stdarr *arr, const stdit *it);

/* Size and Capacity Information */

STDINLINE stdsize  stdarr_size(const stdarr *arr);
STDINLINE stdbool  stdarr_empty(const stdarr *arr);
STDINLINE stdsize  stdarr_high_capacity(const stdarr *arr);
STDINLINE stdsize  stdarr_low_capacity(const stdarr *arr);

STDINLINE stdsize  stdarr_max_size(const stdarr *arr);
STDINLINE stdsize  stdarr_val_size(const stdarr *arr);

/* Size and Capacity Operations */

STDINLINE stdcode  stdarr_resize(stdarr *arr, stdsize num_elems);
STDINLINE void     stdarr_clear(stdarr *arr);

STDINLINE stdcode  stdarr_set_capacity(stdarr *arr, stdsize num_elems);
STDINLINE stdcode  stdarr_reserve(stdarr *arr, stdsize num_elems);
STDINLINE stdcode  stdarr_shrink_fit(stdarr *arr);

/* Stack Operations: amoritized O(1) operations, worst case O(n) */

STDINLINE stdcode  stdarr_push_back(stdarr *arr, const void *val);
STDINLINE stdcode  stdarr_push_back_n(stdarr *arr, const void *vals, stdsize num_push);
STDINLINE stdcode  stdarr_push_back_seq(stdarr *arr, const stdit *b, const stdit *e);
STDINLINE stdcode  stdarr_push_back_seq_n(stdarr *arr, const stdit *b, stdsize num_push);
STDINLINE stdcode  stdarr_push_back_rep(stdarr *arr, const void *val, stdsize num_times);

STDINLINE void     stdarr_pop_back(stdarr *arr);
STDINLINE void     stdarr_pop_back_n(stdarr *arr, stdsize num_pop);

/* List Operations: O(n) operations */

STDINLINE stdcode  stdarr_insert(stdarr *arr, stdit *it, const void *val);
STDINLINE stdcode  stdarr_insert_n(stdarr *arr, stdit *it, const void *vals, stdsize num_insert);
STDINLINE stdcode  stdarr_insert_seq(stdarr *arr, stdit *it, const stdit *b, const stdit *e);
STDINLINE stdcode  stdarr_insert_seq_n(stdarr *arr, stdit *it, const stdit *b, stdsize num_push);
STDINLINE stdcode  stdarr_insert_rep(stdarr *arr, stdit *it, const void *val, stdsize num_times);

STDINLINE void     stdarr_erase(stdarr *arr, stdit *it);
STDINLINE void     stdarr_erase_n(stdarr *arr, stdit *it, stdsize num_erase);
STDINLINE void     stdarr_erase_seq(stdarr *arr, stdit *b, stdit *e);

/* Options */

#define STDARR_OPTS_NO_AUTO_GROW   0x1
#define STDARR_OPTS_NO_AUTO_SHRINK 0x2

STDINLINE stduint8 stdarr_get_opts(const stdarr *arr);
STDINLINE stdcode  stdarr_set_opts(stdarr *arr, stduint8 opts);

/* Iterator Fcns */

STDINLINE void *   stdarr_it_val(const stdit *it);
STDINLINE stdsize  stdarr_it_val_size(const stdit *it);
STDINLINE stdbool  stdarr_it_eq(const stdit *it1, const stdit *it2);
STDINLINE stdssize stdarr_it_cmp(const stdit *it1, const stdit *it2);

STDINLINE stdit *  stdarr_it_next(stdit *it);
STDINLINE stdit *  stdarr_it_advance(stdit *it, stdsize num_advance);
STDINLINE stdit *  stdarr_it_prev(stdit *it);
STDINLINE stdit *  stdarr_it_retreat(stdit *it, stdsize num_retreat);
STDINLINE stdit *  stdarr_it_offset(stdit *it, stdssize offset);

#ifdef __cplusplus
}
#endif

#endif
