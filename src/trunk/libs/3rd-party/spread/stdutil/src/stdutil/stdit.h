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

#ifndef stdit_h_2005_06_11_12_08_51_jschultz_at_cnds_jhu_edu
#define stdit_h_2005_06_11_12_08_51_jschultz_at_cnds_jhu_edu

#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdutil/private/stdit_p.h>

typedef enum 
{
  STDIT_FORWARD       = 0x1,
  STDIT_BIDIRECTIONAL = 0x3,
  STDIT_RANDOM_ACCESS = 0x7

} stdit_type;

/* Forward Iterators */

STDINLINE stdit_type   stdit_get_type(const stdit *it);

STDINLINE const void * stdit_key(const stdit *it);       /* returns NULL for non-dictionary iterators */
STDINLINE stdsize      stdit_key_size(const stdit *it);  /* returns 0 for non-dictionary iterators */

STDINLINE void *       stdit_val(const stdit *it);
STDINLINE stdsize      stdit_val_size(const stdit *it);

STDINLINE stdbool      stdit_eq(const stdit *it1, const stdit *it2);

STDINLINE stdit *      stdit_next(stdit *it);
STDINLINE stdit *      stdit_advance(stdit *it, stdsize num_advance);

STDINLINE stdssize     stdit_distance(const stdit *b, const stdit *e);

/* Bidirectional Iterators */

STDINLINE stdit *      stdit_prev(stdit *it);
STDINLINE stdit *      stdit_retreat(stdit *it, stdsize num_retreat);

/* Random Access Iterators */

STDINLINE stdssize     stdit_cmp(const stdit *it1, const stdit *it2);
STDINLINE stdit *      stdit_offset(stdit *it, stdssize offset);

/* Pointer/C-Array Iterator Constructor */

STDINLINE stdit *      stdit_ptr(stdit *it, const void * vals, stdsize vsize);

/* Parallel Pointer/C-Array Constructor */

STDINLINE stdit *      stdit_pptr(stdit *it, const void *keys, const void *vals, stdsize ksize, stdsize vsize);

#ifdef __cplusplus
}
#endif

#endif
