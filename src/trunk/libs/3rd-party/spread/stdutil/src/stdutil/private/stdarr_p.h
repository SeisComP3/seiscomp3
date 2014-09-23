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

#ifndef stdarr_p_h_2000_05_13_19_35_29_jschultz_at_cnds_jhu_edu
#define stdarr_p_h_2000_05_13_19_35_29_jschultz_at_cnds_jhu_edu

/* stdarr: An array-based sequence: a growable array.  Stores elements
           by value, contiguously in memory starting at the base
           address.
  
   begin - address of the lowest byte of the array's alloc'ed memory, NULL if none alloc'ed
   end   - address of the first byte past the last value stored in the array, same as begin if empty
   size  - number of values this array is storing [(end - begin) / vsize]
   cap   - number of values that can legally fit in alloc'ed memory
   vsize - size, in bytes, of the type of values this array is storing   
   opts  - user flags affecting default operation
*/

typedef struct 
{
  char *   begin;
  char *   end;

  stdsize  cap;
  stdsize  size;

  stdsize  vsize;

  stduint8 opts;

} stdarr;

/* stdarr_it: A stdarr iterator.

   val   - address of the value this iterator is referencing
   vsize - size of the value this iterator is referencing
*/

typedef struct 
{
  char *  val;
  stdsize vsize;

} stdarr_it;

#endif
