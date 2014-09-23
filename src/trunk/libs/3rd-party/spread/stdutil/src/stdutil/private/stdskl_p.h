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

#ifndef stdskl_p_h_2005_06_07_15_09_42_jschultz_at_cnds_jhu_edu
#define stdskl_p_h_2005_06_07_15_09_42_jschultz_at_cnds_jhu_edu

/* stdskl_node: A type that contains a key-value pair and node linking information.

   height - 0 based height of this node; size of (prevs, nexts) arrays minus 1
   prevs  - an array of pointers to previous elements in the list
   nexts  - an array of pointers to later elements in the list
   key    - pointer to the node's key
   val    - pointer to the node's val
   
   NOTE: The prevs and nexts arrays are appended onto the end of the
   node in memory.

   NOTE: The key and value are then appended onto the end of this node in
   memory after the prevs and nexts arrays (w/ padding as necessary).
*/

typedef struct stdskl_node 
{
  stdint8               height;
  struct stdskl_node ** prevs;
  struct stdskl_node ** nexts;

  const void *          key;
  void *                val;

} stdskl_node;

/* stdskl: A skiplist based dictionary that maps non-unique keys to values.

   end_node  - pointer to sentinel end node of list
   size      - number of key-value pairs contained
   ksize     - size in bytes of key type contained
   vsize     - size in bytes of value type contained
   cmp_fcn   - user defined key comparison function
   seed      - state used to generate pseudo-random numbers
   rand_bits - a random number that contains random bits
   bits_left - how many unused bits remain in rand_bits
*/

typedef struct stdskl 
{
  stdskl_node * end_node;

  stdsize       size;

  stdsize       ksize;
  stdsize       vsize;

  stdcmp_fcn    cmp_fcn;

  stduint16     seed[3];
  stduint32     rand_bits;
  stdint8       bits_left;

} stdskl;

/* stdskl_it: An iterator for a stdskl.

   node  - pointer to the reference node in the list
   ksize - size in bytes of referenced key type
   vsize - size in bytes of referenced value type
*/

typedef struct 
{
  stdskl_node * node;
  stdsize       ksize;
  stdsize       vsize;

} stdskl_it;

#endif
