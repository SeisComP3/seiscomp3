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

#ifndef stdcarr_h_2000_01_30_23_42_05_jschultz_at_cnds_jhu_edu
#define stdcarr_h_2000_01_30_23_42_05_jschultz_at_cnds_jhu_edu

#include <stdutil/stdit.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDCARR_MIN_AUTO_ALLOC    /* minimum allocation size in # of elements */
#  define STDCARR_MIN_AUTO_ALLOC 16
#endif

/* Structors */

#define STDCARR_STATIC_CONSTRUCT(vsize, opts) { NULL, NULL, NULL, NULL, 0, 0, (vsize), (opts) }

STDINLINE stdcode  stdcarr_construct(stdcarr *carr, stdsize vsize, stduint8 opts);
STDINLINE stdcode  stdcarr_copy_construct(stdcarr *dst, const stdcarr *src);
STDINLINE void     stdcarr_destruct(stdcarr *carr);

/* Assigners */

STDINLINE stdcode  stdcarr_set_eq(stdcarr *dst, stdcarr *src);
STDINLINE void     stdcarr_swap(stdcarr *carr1, stdcarr *carr2);

/* Iterators */

STDINLINE stdit *  stdcarr_begin(const stdcarr *carr, stdit *it);
STDINLINE stdit *  stdcarr_last(const stdcarr *carr, stdit *it);
STDINLINE stdit *  stdcarr_end(const stdcarr *carr, stdit *it);
STDINLINE stdit *  stdcarr_get(const stdcarr *carr, stdit *it, stdsize elem_num);

STDINLINE stdbool  stdcarr_is_begin(const stdcarr *carr, const stdit *it);
STDINLINE stdbool  stdcarr_is_end(const stdcarr *carr, const stdit *it);
STDINLINE stdsize  stdcarr_rank(const stdcarr *carr, const stdit *it);

/* Size and Capacity Information */

STDINLINE stdsize  stdcarr_size(const stdcarr *carr);
STDINLINE stdbool  stdcarr_empty(const stdcarr *carr);
STDINLINE stdsize  stdcarr_high_capacity(const stdcarr *carr);
STDINLINE stdsize  stdcarr_low_capacity(const stdcarr *carr);

STDINLINE stdsize  stdcarr_max_size(const stdcarr *carr);
STDINLINE stdsize  stdcarr_val_size(const stdcarr *carr);

/* Size and Capacity Operations */

STDINLINE stdcode  stdcarr_resize(stdcarr *carr, stdsize num_elems);
STDINLINE void     stdcarr_clear(stdcarr *carr);

STDINLINE stdcode  stdcarr_set_capacity(stdcarr *carr, stdsize num_elems);
STDINLINE stdcode  stdcarr_reserve(stdcarr *carr, stdsize num_elems);
STDINLINE stdcode  stdcarr_shrink_fit(stdcarr *carr);

/* Stack Operations: amoritized O(1) operations, worst case O(n) */

STDINLINE stdcode  stdcarr_push_front(stdcarr *carr, const void *val);
STDINLINE stdcode  stdcarr_push_front_n(stdcarr *carr, const void *vals, stdsize num_push);
STDINLINE stdcode  stdcarr_push_front_seq(stdcarr *carr, const stdit *b, const stdit *e);
STDINLINE stdcode  stdcarr_push_front_seq_n(stdcarr *carr, const stdit *b, stdsize num_push);
STDINLINE stdcode  stdcarr_push_front_rep(stdcarr *carr, const void *val, stdsize num_times);

STDINLINE void     stdcarr_pop_front(stdcarr *carr);
STDINLINE void     stdcarr_pop_front_n(stdcarr *carr, stdsize num_pop);

STDINLINE stdcode  stdcarr_push_back(stdcarr *carr, const void *val);
STDINLINE stdcode  stdcarr_push_back_n(stdcarr *carr, const void *vals, stdsize num_push);
STDINLINE stdcode  stdcarr_push_back_seq(stdcarr *carr, const stdit *b, const stdit *e);
STDINLINE stdcode  stdcarr_push_back_seq_n(stdcarr *carr, const stdit *b, stdsize num_push);
STDINLINE stdcode  stdcarr_push_back_rep(stdcarr *carr, const void *val, stdsize num_times);

STDINLINE void     stdcarr_pop_back(stdcarr *carr);
STDINLINE void     stdcarr_pop_back_n(stdcarr *carr, stdsize num_pop);

/* List Operations: O(n) operations */

STDINLINE stdcode  stdcarr_insert(stdcarr *carr, stdit *it, const void *val);
STDINLINE stdcode  stdcarr_insert_n(stdcarr *carr, stdit *it, const void *vals, stdsize num_insert);
STDINLINE stdcode  stdcarr_insert_seq(stdcarr *carr, stdit *it, const stdit *b, const stdit *e);
STDINLINE stdcode  stdcarr_insert_seq_n(stdcarr *carr, stdit *it, const stdit *b, stdsize num_insert);
STDINLINE stdcode  stdcarr_insert_rep(stdcarr *carr, stdit *it, const void *val, stdsize num_times);

STDINLINE void     stdcarr_erase(stdcarr *carr, stdit *it);
STDINLINE void     stdcarr_erase_n(stdcarr *carr, stdit *it, stdsize num_erase);
STDINLINE void     stdcarr_erase_seq(stdcarr *carr, stdit *b, stdit *e);

/* Data Structure Options */

#define STDCARR_OPTS_NO_AUTO_GROW   0x1
#define STDCARR_OPTS_NO_AUTO_SHRINK 0x2

STDINLINE stduint8 stdcarr_get_opts(const stdcarr *carr);
STDINLINE stdcode  stdcarr_set_opts(stdcarr *carr, stduint8 opts);

/* Iterator Fcns */

STDINLINE void *   stdcarr_it_val(const stdit *it);
STDINLINE stdsize  stdcarr_it_val_size(const stdit *it);
STDINLINE stdbool  stdcarr_it_eq(const stdit *it1, const stdit *it2);
STDINLINE stdssize stdcarr_it_cmp(const stdit *it1, const stdit *it2);

STDINLINE stdit *  stdcarr_it_next(stdit *it);
STDINLINE stdit *  stdcarr_it_advance(stdit *it, stdsize num_advance);
STDINLINE stdit *  stdcarr_it_prev(stdit *it);
STDINLINE stdit *  stdcarr_it_retreat(stdit *it, stdsize num_retreat);
STDINLINE stdit *  stdcarr_it_offset(stdit *it, stdssize offset);

#ifdef __cplusplus
}
#endif

#endif
