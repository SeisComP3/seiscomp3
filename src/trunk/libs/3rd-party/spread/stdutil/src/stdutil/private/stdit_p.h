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

#ifndef stdit_p_h_2005_06_11_12_37_56_jschultz_at_cnds_jhu_edu
#define stdit_p_h_2005_06_11_12_37_56_jschultz_at_cnds_jhu_edu

#define STDPTR_IT_ID      ((stduint32) 0x86958034UL)
#define STDPPTR_IT_ID     ((stduint32) 0xcc2f9985UL)
#define STDARR_IT_ID      ((stduint32) 0x85edb072UL)
#define STDCARR_IT_ID     ((stduint32) 0x6c248dc2UL)
#define STDDLL_IT_ID      ((stduint32) 0x7b868dfdUL)
#define STDHASH_IT_ID     ((stduint32) 0xdc01b2d1UL)
#define STDHASH_IT_KEY_ID ((stduint32) 0x7e78a0fdUL)
#define STDSKL_IT_ID      ((stduint32) 0x7abf271bUL)
#define STDSKL_IT_KEY_ID  ((stduint32) 0x1ac2ee79UL)

#include <stdutil/private/stdarr_p.h>
#include <stdutil/private/stdcarr_p.h>
#include <stdutil/private/stddll_p.h>
#include <stdutil/private/stdhash_p.h>
#include <stdutil/private/stdskl_p.h>

typedef struct 
{
  char *  val;
  stdsize vsize;

} stdptr_it;

typedef struct 
{
  char *  key;
  char *  val;

  stdsize ksize;
  stdsize vsize;

} stdpptr_it;

typedef struct
{
  union 
  {
    stdptr_it  ptr;
    stdpptr_it pptr;
    stdarr_it  arr;
    stdcarr_it carr;
    stddll_it  dll;
    stdhash_it hash;
    stdskl_it  skl;

  } impl;

  stduint32 type_id;

} stdit;

#endif
