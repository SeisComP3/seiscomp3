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

#ifndef stdcarr_p_h_2000_05_15_18_35_08_jschultz_at_cnds_jhu_edu
#define stdcarr_p_h_2000_05_15_18_35_08_jschultz_at_cnds_jhu_edu

/* stdcarr: A circular array-based sequence: a growable circular array
            or vector.  Stores element by value, contiguously in
            memory modulo the size of the array.  Always maintains at
            least one empty element in a non-empty array to act as a
            sentinel position and to remove the amibiguous case of
            when begin == end -- Is the array full or empty?  In
            particular, this allows for the test begin <= end ? (data
            doesn't wrap around) : (data wraps around)

   base    - address of the lowest byte of the array's alloc'ed memory, NULL if none
   endbase - address of the first byte past the end of the array's alloc'ed memory, NULL if none
   begin   - address of the first byte of the first value stored in the array; [base, endbase) if none
   end     - address of the first byte past the last value stored in the array, begin if none 
   cap     - number of values that could legally fit in alloc'ed memory: is one more than the #
             of elements that will be allowed in the array before reallocating to a larger array 
   size    - number of values this array is storing   
   vsize   - size, in bytes, of the type of values this array is storing   
   opts    - user flags affecting default operation
*/

typedef struct 
{
  char *   base;
  char *   endbase;

  char *   begin;
  char *   end;

  stdsize  cap;
  stdsize  size;

  stdsize  vsize;

  stduint8 opts;

} stdcarr;

/* stdcarr_it: Iterator for stdcarrs.

   val     - address of the value in carr this iterator is referencing
   base    - base of the stdcarr at time of initialization
   endbase - endbase of the stdcarr at time of initialization
   begin   - begin of the stdcarr at time of initialization
   vsize   - size in bytes of the value this iterator references
*/

typedef struct 
{
  char *  val;

  char *  base;
  char *  endbase;

  char *  begin;
  char *  end;

  stdsize vsize;

} stdcarr_it;

#endif
