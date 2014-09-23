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

#ifndef stdhash_h_2000_02_14_16_22_38_jschultz_at_cnds_jhu_edu
#define stdhash_h_2000_02_14_16_22_38_jschultz_at_cnds_jhu_edu

#include <stdutil/stdit.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDHASH_MIN_AUTO_ALLOC  /* minimum allocated table capacity */
#  define STDHASH_MIN_AUTO_ALLOC 16
#endif

/* Structors */

#define STDHASH_STATIC_CONSTRUCT(ksize, vsize, kcmp, khcode, opts) \
{ NULL, NULL, NULL, (stdsize) -1, (stdsize) -1, 0, 0, (ksize), (vsize), (kcmp), (khcode), (opts) }

STDINLINE stdcode      stdhash_construct(stdhash *h, stdsize ksize, stdsize vsize, stdcmp_fcn kcmp, stdhcode_fcn khcode, stduint8 opts);
STDINLINE stdcode      stdhash_copy_construct(stdhash *dst, const stdhash *src);
STDINLINE void         stdhash_destruct(stdhash *h);

/* Assigners */

STDINLINE stdcode      stdhash_set_eq(stdhash *dst, const stdhash *src);
STDINLINE void         stdhash_swap(stdhash *h1, stdhash *h2);

/* Iterators */

STDINLINE stdit *      stdhash_begin(const stdhash *h, stdit *it);
STDINLINE stdit *      stdhash_last(const stdhash *h, stdit *it);
STDINLINE stdit *      stdhash_end(const stdhash *h, stdit *it);
STDINLINE stdit *      stdhash_get(const stdhash *h, stdit *it, stdsize elem_num);  /* O(n) */

STDINLINE stdbool      stdhash_is_begin(const stdhash *h, const stdit *it);
STDINLINE stdbool      stdhash_is_end(const stdhash *h, const stdit *it);

STDINLINE stdit *      stdhash_keyed_next(const stdhash *h, stdit *it);
STDINLINE stdit *      stdhash_keyed_prev(const stdhash *h, stdit *it);

/* Size and Table Load Information */

STDINLINE stdsize      stdhash_size(const stdhash *h);
STDINLINE stdbool      stdhash_empty(const stdhash *h);

STDINLINE stdsize      stdhash_load_lvl(const stdhash *h);
STDINLINE stdsize      stdhash_high_thresh(const stdhash *h);
STDINLINE stdsize      stdhash_low_thresh(const stdhash *h);

STDINLINE stdsize      stdhash_max_size(const stdhash *h);

/* Size and Capacity Operations */

STDINLINE void         stdhash_clear(stdhash *h);

STDINLINE stdcode      stdhash_reserve(stdhash *h, stdsize num_elems);
STDINLINE stdcode      stdhash_rehash(stdhash *h);

/* Dictionary Operations: O(1) expected, O(n) worst case */

STDINLINE stdit *      stdhash_find(const stdhash *h, stdit *it, const void *key);
STDINLINE stdbool      stdhash_contains(const stdhash *h, const void *key);

STDINLINE stdcode      stdhash_put(stdhash *h, stdit *it, const void *key, const void *val);
STDINLINE stdcode      stdhash_put_n(stdhash *h, stdit *it, const void *keys, const void *vals, stdsize num_put);
STDINLINE stdcode      stdhash_put_seq(stdhash *h, stdit *it, const stdit *b, const stdit *e);
STDINLINE stdcode      stdhash_put_seq_n(stdhash *h, stdit *it, const stdit *b, stdsize num_put);

STDINLINE stdcode      stdhash_insert(stdhash *h, stdit *it, const void *key, const void *val);
STDINLINE stdcode      stdhash_insert_n(stdhash *h, stdit *it, const void *keys, const void *vals, stdsize num_insert);
STDINLINE stdcode      stdhash_insert_seq(stdhash *h, stdit *it, const stdit *b, const stdit *e);
STDINLINE stdcode      stdhash_insert_seq_n(stdhash *h, stdit *it, const stdit *b, stdsize num_insert);
STDINLINE stdcode      stdhash_insert_rep(stdhash *h, stdit *it, const void *key, const void *val, stdsize num_times);

STDINLINE void         stdhash_erase(stdhash *h, stdit *it);
STDINLINE void         stdhash_erase_key(stdhash *h, const void *key);

/* Type Information + Options */

STDINLINE stdsize      stdhash_key_size(const stdhash *h);
STDINLINE stdsize      stdhash_val_size(const stdhash *h);

STDINLINE stdcmp_fcn   stdhash_key_cmp(const stdhash *h);  
STDINLINE stdhcode_fcn stdhash_key_hcode(const stdhash *h);

#define STDHASH_OPTS_DEFAULTS       0x0
#define STDHASH_OPTS_NO_AUTO_GROW   0x1
#define STDHASH_OPTS_NO_AUTO_SHRINK 0x2

STDINLINE stduint8     stdhash_get_opts(const stdhash *h);
STDINLINE stdcode      stdhash_set_opts(stdhash *h, stduint8 opts);

/* Iterator Fcns */

STDINLINE stdsize      stdhash_it_key_size(const stdit *it);
STDINLINE stdsize      stdhash_it_val_size(const stdit *it);

STDINLINE const void * stdhash_it_key(const stdit *it);
STDINLINE void *       stdhash_it_val(const stdit *it);
STDINLINE stdbool      stdhash_it_eq(const stdit *it1, const stdit *it2);

STDINLINE stdit *      stdhash_it_next(stdit *it);
STDINLINE stdit *      stdhash_it_advance(stdit *it, stdsize num_advance);
STDINLINE stdit *      stdhash_it_prev(stdit *it);
STDINLINE stdit *      stdhash_it_retreat(stdit *it, stdsize num_retreat);

# ifdef __cplusplus
}
# endif

#endif
