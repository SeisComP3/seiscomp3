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

#ifndef stddll_p_h_2000_05_16_18_07_15_jschultz_at_cnds_jhu_edu
#define stddll_p_h_2000_05_16_18_07_15_jschultz_at_cnds_jhu_edu

/* stddll_node: A node within a doubly linked list.

   prev - pointer to previous stddll_node on list
   next - pointer to next stddll_node on list

   NOTE: the element is appended (with padding as necessary) onto the
   end of the node in memory.
*/

typedef struct stddll_node 
{
  struct stddll_node * prev;
  struct stddll_node * next;

} stddll_node;

/* stddll: A doubly linked list of values contained by value.

   end_node - pointer to the sentinel end node of a list
   size     - number of elements this list is storing
   vsize    - size, in bytes, of the type of elements this list stores
*/

typedef struct 
{
  stddll_node * end_node;
  stdsize       size;
  stdsize       vsize;

} stddll;

/* stddll_it: An iterator for a stddll.

   node     - address of a node in a stddll sequence
   end_node - the sentinel end node of a stddll
   vsize    - size, in bytes, of the values that this iterator references
*/

typedef struct 
{
  stddll_node * node;
  stddll_node * end_node;  
  stdsize       vsize;

} stddll_it;

#endif
