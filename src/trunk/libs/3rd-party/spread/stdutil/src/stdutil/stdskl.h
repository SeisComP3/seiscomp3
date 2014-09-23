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

#ifndef stdskl_h_2004_07_24_13_13_49_jschultz_at_cnds_jhu_edu
#define stdskl_h_2004_07_24_13_13_49_jschultz_at_cnds_jhu_edu

#include <stdutil/stdit.h>

# ifdef __cplusplus
extern "C" {
# endif

/* Structors */

STDINLINE stdcode      stdskl_construct(stdskl *l, stdsize ksize, stdsize vsize, stdcmp_fcn kcmp);
STDINLINE stdcode      stdskl_copy_construct(stdskl *dst, const stdskl *src);
STDINLINE void         stdskl_destruct(stdskl *l);

/* Assigners */

STDINLINE stdcode      stdskl_set_eq(stdskl *dst, const stdskl *src);
STDINLINE void         stdskl_swap(stdskl *l1, stdskl *l2);

/* Iterators */

STDINLINE stdit *      stdskl_begin(const stdskl *l, stdit *it);
STDINLINE stdit *      stdskl_last(const stdskl *l, stdit *it);
STDINLINE stdit *      stdskl_end(const stdskl *l, stdit *it);
STDINLINE stdit *      stdskl_get(const stdskl *l, stdit *it, size_t elem_num);  /* O(n) */

STDINLINE stdbool      stdskl_is_begin(const stdskl *l, const stdit *it);
STDINLINE stdbool      stdskl_is_end(const stdskl *l, const stdit *it);

/* Size Information */

STDINLINE stdsize      stdskl_size(const stdskl *l);
STDINLINE stdbool      stdskl_empty(const stdskl *l);

/* Size Operations */

STDINLINE void         stdskl_clear(stdskl *l);

/* Dictionary Operations: O(lg n) */

STDINLINE stdit *      stdskl_find(const stdskl *l, stdit *it, const void *key);
STDINLINE stdit *      stdskl_lowerb(const stdskl *l, stdit *it, const void *key);
STDINLINE stdit *      stdskl_upperb(const stdskl *l, stdit *it, const void *key);
STDINLINE stdbool      stdskl_contains(const stdskl *l, const void *key);

STDINLINE stdcode      stdskl_put(stdskl *l, stdit *it, const void *key, const void *val, stdbool hint);
STDINLINE stdcode      stdskl_put_n(stdskl *l, stdit *it, const void *keys, const void *vals, stdsize num_put, stdbool hint);
STDINLINE stdcode      stdskl_put_seq(stdskl *l, stdit *it, const stdit *b, const stdit *e, stdbool hint);
STDINLINE stdcode      stdskl_put_seq_n(stdskl *l, stdit *it, const stdit *b, stdsize num_put, stdbool hint);

STDINLINE stdcode      stdskl_insert(stdskl *l, stdit *it, const void *key, const void *val, stdbool hint);
STDINLINE stdcode      stdskl_insert_n(stdskl *l, stdit *it, const void *keys, const void *vals, stdsize num_insert, stdbool hint);
STDINLINE stdcode      stdskl_insert_seq(stdskl *l, stdit *it, const stdit *b, const stdit *e, stdbool hint);
STDINLINE stdcode      stdskl_insert_seq_n(stdskl *l, stdit *it, const stdit *b, stdsize num_insert, stdbool hint);
STDINLINE stdcode      stdskl_insert_rep(stdskl *l, stdit *it, const void *key, const void *val, stdsize num_times, stdbool hint);

STDINLINE void         stdskl_erase(stdskl *l, stdit *it);
STDINLINE void         stdskl_erase_n(stdskl *l, stdit *it, stdsize num_erase);
STDINLINE stdsize      stdskl_erase_seq(stdskl *l, stdit *b, stdit *e);
STDINLINE stdsize      stdskl_erase_key(stdskl *l, const void *key);

/* Randomization */

STDINLINE void         stdskl_dseed(stdskl *l, const void *seed, stdsize sizeof_seed);

/* Iterator Fcns */

STDINLINE const void * stdskl_it_key(const stdit *it);
STDINLINE stdsize      stdskl_it_key_size(const stdit *it);
STDINLINE void *       stdskl_it_val(const stdit *it);
STDINLINE stdsize      stdskl_it_val_size(const stdit *it);
STDINLINE stdbool      stdskl_it_eq(const stdit *it1, const stdit *it2);

STDINLINE stdit *      stdskl_it_next(stdit *it);
STDINLINE stdit *      stdskl_it_advance(stdit *it, stdsize num_advance);
STDINLINE stdit *      stdskl_it_prev(stdit *it);
STDINLINE stdit *      stdskl_it_retreat(stdit *it, stdsize num_retreat);

# ifdef __cplusplus
}
# endif

#endif
