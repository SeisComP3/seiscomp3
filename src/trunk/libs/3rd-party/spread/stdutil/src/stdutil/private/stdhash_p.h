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

#ifndef stdhash_p_h_2000_05_17_18_02_31_jschultz_at_cnds_jhu_edu
#define stdhash_p_h_2000_05_17_18_02_31_jschultz_at_cnds_jhu_edu

/* stdhash_node: A type that contains a key-value pair and a cache of
   the key's hashcode.

   hcode - a cached copy of the hashcode of the key for this key-val pair.

   NOTE: The key and value are appended onto the end of this node in
   memory (w/ padding as necessary).
*/

typedef struct 
{
  stdhcode hcode;

} stdhash_node;

/* stdhash: An array based dictionary that maps non-unique keys to values.

   table     - pointer to the base of an alloc'ed array of node*'s, NULL if none
   table_end - pointer to one past the last of the alloc'ed array of node*'s, NULL if none
   begin     - pointer to the first active node in the table, table_end if none
   cap_min1  - number (power of 2) of node*'s that could fit in table minus 1 (bitmask for modulo)
   cap_lg    - the log base 2 of (cap_min1 + 1)
   num_nodes - the number of alloc'ed nodes contained in table (counts inactive nodes too) 
   size      - number of active key-val pairs the hash currently contains
   ksize     - size, in bytes, of the key type
   vsize     - size, in bytes, of the value type
   cmp_fcn   - user defined fcn for comparing keys
   hcode_fcn - user defined fcn for generating hashcodes for keys
   opts      - user defined options
*/

typedef struct 
{
  stdhash_node ** table;
  stdhash_node ** table_end;
  stdhash_node ** begin;

  stdsize         cap_min1;
  stdsize         cap_lg;
  stdsize         num_nodes;
  stdsize         size;

  stdsize         ksize;
  stdsize         vsize;

  stdcmp_fcn      cmp_fcn;
  stdhcode_fcn    hcode_fcn;

  stduint8        opts;

} stdhash;

/* stdhash_it: An iterator for a stdhash.

   node_pos  - address of the position in the array of node*'s this iterator is currently referencing
   table     - the hash's table
   table_end - the end of the hash's table
   ksize     - the size of the keys to which the iterator points
   vsize     - the size of the vals to which the iterator points
*/

typedef struct 
{
  stdhash_node ** node_pos;

  stdhash_node ** table;
  stdhash_node ** table_end;

  stdsize         ksize;
  stdsize         vsize;

} stdhash_it;

#endif
