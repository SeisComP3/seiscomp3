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

#include <stdlib.h>
#include <string.h>

#include <stdutil/stderror.h>
#include <stdutil/stdcarr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDCARR_IS_LEGAL(carr) (( ((carr)->cap != 0 && (carr)->base != NULL) || ((carr)->cap == 0 && (carr)->base == NULL) ) && \
				(stdsize) ((carr)->endbase - (carr)->base) == (carr)->cap * (carr)->vsize && \
				(carr)->begin >= (carr)->base && (carr)->begin <= (carr)->endbase && \
				(carr)->end >= (carr)->base && (carr)->end <= (carr)->endbase && \
				(stdsize) ((carr)->begin > (carr)->end ? \
					   ((carr)->endbase - (carr)->begin) + ((carr)->end - (carr)->base) : \
					   (carr)->end - (carr)->begin) == (carr)->size * (carr)->vsize && \
				((carr)->endbase == NULL || ((carr)->begin != (carr)->endbase && (carr)->end != (carr)->endbase)) && \
				( ((carr)->size < (carr)->cap) || ((carr)->size == 0 && (carr)->cap == 0) ) && \
				(carr)->vsize != 0 && \
				((carr)->opts & ~(STDCARR_OPTS_NO_AUTO_GROW | STDCARR_OPTS_NO_AUTO_SHRINK)) == 0)

#define STDCARR_IT_IS_LEGAL(carr, it) ((it)->base == (carr)->base && (it)->endbase == (carr)->endbase && \
				       (it)->begin == (carr)->begin && (it)->end == (carr)->end && (it)->vsize == (carr)->vsize)

#define STDCARR_IT_IS_LEGAL2(it) ((it)->base  <= (it)->endbase && \
				  (it)->begin >= (it)->base && (it)->begin <= (it)->endbase && \
				  (it)->end   >= (it)->base && (it)->end   <= (it)->endbase && \
                                  ((it)->begin <= (it)->end ? \
				   ((it)->val  >= (it)->begin && (it)->val <= (it)->end) : \
				   (((it)->val >= (it)->begin && (it)->val <  (it)->endbase) || \
				    ((it)->val >= (it)->base  && (it)->val <= (it)->end))) && \
				  ((it)->endbase == NULL || ((it)->begin != (it)->endbase && (it)->end != (it)->endbase && (it)->val != (it)->endbase)) && \
				  (it)->vsize != 0)

#define STDIT_CARR_IS_LEGAL(it) ((it)->type_id == STDCARR_IT_ID && STDCARR_IT_IS_LEGAL2(&(it)->impl.carr))

#define STDCARR_INS_SHIFT_RIGHT(carr, ptr) \
((ptr) >= (carr)->begin ? (stdsize) ((ptr) - (carr)->begin) > ((carr)->size >> 1) * (carr)->vsize \
                        : (stdsize) ((carr)->end - (ptr)) <= ((carr)->size >> 1) * (carr)->vsize)

/* NOTE: the '+' sign in '(carr)->size + (n)' is correct don't change it -- work it out on paper */

#define STDCARR_ERS_SHIFT_RIGHT(carr, ptr, n) \
((ptr) >= (carr)->begin ? (stdsize) ((ptr) - (carr)->begin) < (((carr)->size - (n)) >> 1) * (carr)->vsize \
                        : (stdsize) ((carr)->end - (ptr)) >= (((carr)->size + (n)) >> 1) * (carr)->vsize)

/************************************************************************************************
 * stdcarr_low_copy_to_buf: Copy a portion of a circular array to a
 * buffer.  Returns ptr to right after copied data in dst.  'c_begin'
 * and 'c_end' point at positions inside of carr.
 ***********************************************************************************************/

STDINLINE static char *stdcarr_low_copy_to_buf(char *dst, const stdcarr *carr, const char *c_begin, const char *c_end) 
{
  stdsize diff;

  STDSAFETY_CHECK(c_begin >= carr->base && c_begin <= carr->endbase &&
		  c_end   >= carr->base && c_end   <= carr->endbase &&
		  (carr->endbase == NULL || (c_begin != carr->endbase && c_end != carr->endbase)));

  if (c_begin <= c_end) {
    diff = (stdsize) (c_end - c_begin);

    memcpy(dst, c_begin, diff);
    dst += diff;

  } else {
    diff = (stdsize) (carr->endbase - c_begin);

    memcpy(dst, c_begin, diff);
    dst += diff;

    diff = (stdsize) (c_end - carr->base);

    memcpy(dst, carr->base, diff);
    dst += diff;
  }

  return dst;
}

/************************************************************************************************
 * stdcarr_low_forward: Return 'p' advanced by 'n' bytes forward, with wrapping.
 ***********************************************************************************************/

STDINLINE static char *stdcarr_low_forward(char *p, stdsize n, const char *base, const char *endbase) 
{
  STDSAFETY_CHECK(p >= base && p <= endbase && (endbase == NULL || p != endbase));

  return ((p += n) < endbase ? p : (char*) base + (p - (char*) endbase));
}

/************************************************************************************************
 * stdcarr_low_backward: Return 'p' advanced by 'n' bytes backward, with wrapping.
 ***********************************************************************************************/

STDINLINE static char *stdcarr_low_backward(char *p, stdsize n, const char *base, const char *endbase) 
{
  STDSAFETY_CHECK(p >= base && p <= endbase && (endbase == NULL || p != endbase));

  return ((p -= n) >= base ? p : (char*) endbase - ((char*) base - p));
}

/************************************************************************************************
 * stdcarr_low_insert_shift_right: This function shifts all the values
 * from 'it' and on, to the right by delta bytes.  It also updates end
 * and size.
 ***********************************************************************************************/

STDINLINE static void stdcarr_low_insert_shift_right(stdcarr *carr, char *it, stdsize delta, stdsize new_size) 
{
  stdssize diff;
  stdssize diff2;

  if (carr->begin <= carr->end) {

    if ((diff = carr->end + delta - carr->endbase) <= 0) { /* no data wraps around */
      memmove(it + delta, it, (stdsize) (carr->end - it));

    } else {

      if ((diff2 = carr->end - it) <= diff) { /* data to shift can fit between base and new end */
	memcpy(carr->base + diff - diff2, it, (stdsize) diff2);

      } else {                                /* partial fit */
	memcpy(carr->base, carr->end - diff, (stdsize) diff);
	memmove(it + delta, it, (stdsize) (diff2 - diff));
      }
    }

  } else {  /* space exists between end and begin for insertion */

    if ((diff = carr->end - it) >= 0) {                      /* it is below end */
      memmove(it + delta, it, (stdsize) diff);

    } else {
      memmove(carr->base + delta, carr->base, (stdsize) (carr->end - carr->base));  /* shift lower data */

      if ((stdsize) (diff = carr->endbase - it) <= delta) {  /* higher data fits into newly opened area */
	memcpy(carr->base + delta - diff, it, (stdsize) diff);

      } else {                                               /* partial fit */
	memcpy(carr->base, carr->endbase - delta, delta);
	memmove(it + delta, it, (stdsize) (diff - delta));
      }
    }
  }

  carr->size = new_size;
  carr->end  = stdcarr_low_forward(carr->end, delta, carr->base, carr->endbase);
}

/************************************************************************************************
 * stdcarr_low_insert_shift_left: This function shifts all values
 * before 'it' to the left by delta bytes.  It also updates begin and
 * size.
 ***********************************************************************************************/

STDINLINE static void stdcarr_low_insert_shift_left(stdcarr *carr, char *it, stdsize delta, stdsize new_size) 
{
  stdssize diff;
  stdssize diff2;

  if (carr->begin <= carr->end) {

    if ((diff = carr->base - (carr->begin - delta)) <= 0) { /* new begin doesn't wrap around */
      memmove(carr->begin - delta, carr->begin, (stdsize) (it - carr->begin));

    } else {

      if ((diff2 = it - carr->begin) <= diff) {  /* data to shift fits between new begin and endbase */
	memcpy(carr->endbase - diff, carr->begin, (stdsize) diff2);

      } else {                                   /* partial fit */
	memcpy(carr->endbase - diff, carr->begin, (stdsize) diff);
	memmove(carr->base, carr->begin + diff, (stdsize) (diff2 - diff));
      }
    }

  } else {  /* space exists between end and begin for insertion */

    if (it >= carr->begin) {                               /* it is above begin */
      memmove(carr->begin - delta, carr->begin, (stdsize) (it - carr->begin));

    } else {
      memmove(carr->begin - delta, carr->begin, (stdsize) (carr->endbase - carr->begin));

      if ((stdsize) (diff = it - carr->base) <= delta) {    /* fits into newly opened area */
	memcpy(carr->endbase - delta, carr->base, (stdsize) diff);

      } else {
	memcpy(carr->endbase - delta, carr->base, delta);
	memmove(carr->base, carr->base + delta, (stdsize) (diff - delta));
      }
    }
  }

  carr->size  = new_size;
  carr->begin = stdcarr_low_backward(carr->begin, delta, carr->base, carr->endbase);
}

/************************************************************************************************
 * stdcarr_low_erase_shift_left: This function shifts values from the
 * right to the left into erased values. erase_end points to one past
 * the last value to be erased. (e.g. - delta == 4 * carr->vsize)
 *
 * erase_shift_left(__****----1***___) => __****1***_______ 
 * 
 * Legend: _ = empty slot, * = occupied slot, 1 = erase_end, - = to be
 * deleted.
 ***********************************************************************************************/

STDINLINE static void stdcarr_low_erase_shift_left(stdcarr *carr, char *erase_end, stdsize delta, stdsize new_size) 
{
  char *   erase_begin = erase_end - delta;             /* may point outside alloc'ed mem */
  stdssize diff;
  stdssize diff2;

  if ((diff = carr->end - erase_end) >= 0) {            /* end is at or above erase_end */

    if ((diff2 = carr->base - erase_begin) <= 0) {      /* erase region doesn't wrap around */
      memmove(erase_begin, erase_end, (stdsize) diff);  

    } else {
      /* diff2: how many bytes to erase off of high portion of array */
      
      erase_begin = carr->endbase - diff2;              /* implicitly valid */

      if (diff <= diff2) {            /* data to shift fits between erase_begin and endbase */
	memcpy(erase_begin, erase_end, (stdsize) diff);

      } else {                        /* only a partial fit */
	memcpy(erase_begin, erase_end, (stdsize) diff2);
	memmove(carr->base, erase_end + diff2, (stdsize) (diff - diff2));
      }
    }

  } else {                                              /* carr->end is wrapped around */
    diff = carr->endbase - erase_end;
    memmove(erase_begin, erase_end, (stdsize) diff);    /* shift higher data first */
    erase_begin += diff;                                /* move erase_begin forward diff bytes */

    if ((stdsize) (diff = carr->end - carr->base) <= delta) { /* lower data fits in opened area */
      memcpy(erase_begin, carr->base, (stdsize) diff);        /* copy lower data to higher */

    } else {                                            /* partial fit */
      memcpy(erase_begin, carr->base, delta);           /* copy delta bytes higher */
      memmove(carr->base, carr->base + delta, (stdsize) (diff - delta));  /* shift rest lower */
    }
  }

  carr->size = new_size;
  carr->end  = stdcarr_low_backward(carr->end, delta, carr->base, carr->endbase);
}

/************************************************************************************************
 * stdcarr_low_erase_shift_right: This function shifts values from the
 * left to the right into erased values. erase_begin points to the
 * first value to be erased. (e.g. - delta == 4 * carr->vsize).
 *
 * erase_shift_right(__**1---*****___) => ______*******___
 * 
 * Legend: _ = empty slot, * = occupied slot, 1 = it, - = to be
 * deleted.
 ***********************************************************************************************/

STDINLINE static void stdcarr_low_erase_shift_right(stdcarr *carr, char *erase_begin, stdsize delta, stdsize new_size) 
{
  char *   erase_end = erase_begin + delta;   /* may point outside valid range */
  stdssize diff;
  stdssize diff2;
  stdssize diff3;

  if ((diff = erase_begin - carr->begin) >= 0) {     /* erase_begin is at or above begin */

    if ((diff2 = erase_end - carr->endbase) <= 0) {  /* erase region doesn't wrap around */
      memmove(carr->begin + delta, carr->begin, (stdsize) diff); 

    } else {
      /* diff2: how many bytes to erase off of low portion of array */

      if ((diff3 = diff2 - diff) >= 0) { /* data to shift fits between base and erase_end */
	memcpy(carr->base + diff3, carr->begin, (stdsize) diff);

      } else {                           /* only a partial fit */
	memcpy(carr->base, erase_begin - diff2, (stdsize) diff2);
	memmove(carr->begin + delta, carr->begin, (stdsize) -diff3);
      }
    }

  } else {
    diff = erase_begin - carr->base;
    erase_end -= diff;                                             /* move erase_end back diff bytes */
    memmove(erase_end, carr->base, (stdsize) diff);                /* shift lower data first */

    if ((stdsize) (diff = carr->endbase - carr->begin) <= delta) { /* fits into newly opened area */
      memcpy(erase_end - diff, carr->begin, (stdsize) diff);

    } else {                                                       /* partial fit */
      memcpy(carr->base, carr->endbase - delta, delta);            /* copy delta bytes lower */
      memmove(carr->begin + delta, carr->begin, (stdsize) (diff - delta));  /* shift rest higher */
    }
  }

  carr->size  = new_size;
  carr->begin = stdcarr_low_forward(carr->begin, delta, carr->base, carr->endbase);
}

/************************************************************************************************
 * stdcarr_low_insert_shift: This function first determines if an
 * insertion will require a reallocation. If reallocation isn't
 * needed, it calls the specified array shift function. If
 * reallocation is called for, it does it and copies the values from
 * carr to the new array while creating the open space requested. This
 * function returns a pointer to the beginning of the insertion region
 * through itp on success.
 ***********************************************************************************************/

STDINLINE static stdcode stdcarr_low_insert_shift(stdcarr *carr, char **itp, stdsize delta, 
						  stdsize new_size, stdbool shift_right) 
{
  stdcode ret = STDESUCCESS;

  if (delta == 0) {                                            /* no-op */
    goto stdcarr_low_insert_shift_end;
  }

  /* delta != 0 -> new_size >= 1 */

  if (new_size <= stdcarr_high_capacity(carr)) {               /* current table is big enough */

    if (shift_right) {
      stdcarr_low_insert_shift_right(carr, *itp, delta, new_size);

    } else {
      stdcarr_low_insert_shift_left(carr, *itp, delta, new_size);
      *itp = stdcarr_low_backward(*itp, delta, carr->base, carr->endbase);
    }

  } else if ((carr->opts & STDCARR_OPTS_NO_AUTO_GROW) == 0) {  /* auto-growth allowed */
    stdsize new_cap = (new_size << 1);                         /* new_cap > 0 */
    stdsize asize;
    char *  mem;
    char *  tmp;

    new_cap = STDMAX(new_cap, STDCARR_MIN_AUTO_ALLOC);
    asize   = new_cap * carr->vsize;

    if ((mem = (char*) malloc(asize)) == NULL) {
      ret = STDENOMEM;
      goto stdcarr_low_insert_shift_end;
    }

    if (carr->base != NULL) {
      tmp  = *itp;
      *itp = stdcarr_low_copy_to_buf(mem, carr, carr->begin, tmp);  /* copy [begin, it) */
      stdcarr_low_copy_to_buf(*itp + delta, carr, tmp, carr->end);  /* insert space and copy [it, end) */
      free(carr->base);                                             /* free old array */

    } else {
      *itp = mem;
    }

    carr->base    = mem;
    carr->endbase = mem + asize;
    carr->begin   = mem;
    carr->end     = mem + new_size * carr->vsize;
    carr->cap     = new_cap;
    carr->size    = new_size;

  } else {                                                     /* auto-growth disallowed */
    ret = STDEACCES;
    goto stdcarr_low_insert_shift_end;
  }

 stdcarr_low_insert_shift_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_low_erase_shift: This function determines if an erasure
 * will require a reallocation or not. If reallocation isn't needed,
 * it calls the specified array shift function. If reallocation is
 * called for, it does it and copies the values to the new carray
 * while deleting the proper elements. The boolean parameter
 * shift_right indicates whether 'itp' points to the beginning of the
 * erase sequence or to one past the end of the erase sequence. This
 * parameter also determines whether we erase_shift_right or
 * erase_shift_left.
 ***********************************************************************************************/

STDINLINE static void stdcarr_low_erase_shift(stdcarr *carr, char **itp, stdsize delta, 
					      stdsize new_size, stdbool shift_right) 
{
  if (delta == 0) {                                       /* no-op */
    return;
  }

  /* delta != 0 -> carr->size >= 1 && carr->cap > carr->size */

  if ((carr->opts & STDCARR_OPTS_NO_AUTO_SHRINK) != 0 ||  /* no shrinking wanted */
      new_size > stdcarr_low_capacity(carr) ||            /* no shrinking necessary */
      carr->cap == STDCARR_MIN_AUTO_ALLOC) {              /* already at min alloc size */

    /* label used for alloc failures from below */
  stdcarr_low_erase_shift_fail:

    if (shift_right) {
      stdcarr_low_erase_shift_right(carr, *itp, delta, new_size);
      *itp = stdcarr_low_forward(*itp, delta, carr->base, carr->endbase);
      
    } else {
      stdcarr_low_erase_shift_left(carr, *itp, delta, new_size);
    }

  } else {                                                /* need+allowed to shrink */
    stdsize new_cap = (new_size << 1);
    stdsize asize;
    char *  mem;
    char *  tmp;
    char *  tmp2;

    new_cap = STDMAX(new_cap, STDCARR_MIN_AUTO_ALLOC);
    asize   = new_cap * carr->vsize;

    if (asize != 0) {

      if ((mem = (char*) malloc(asize)) == NULL) {
	goto stdcarr_low_erase_shift_fail;                /* fallback to no realloc pathway */
      }

      if (shift_right) {
	tmp  = stdcarr_low_forward(*itp, delta, carr->base, carr->endbase);
	*itp = stdcarr_low_copy_to_buf(mem, carr, carr->begin, *itp);         /* copy [begin, it) */
	stdcarr_low_copy_to_buf(*itp, carr, tmp, carr->end);                  /* copy [it + delta, end) */

      } else {
	tmp  = *itp;
	tmp2 = stdcarr_low_backward(*itp, delta, carr->base, carr->endbase);
	*itp = stdcarr_low_copy_to_buf(mem, carr, carr->begin, tmp2);         /* copy [begin, it - delta) */
	stdcarr_low_copy_to_buf(*itp, carr, tmp, carr->end);                  /* copy [it, end) */
      }

    } else {
      mem  = NULL;
      *itp = NULL;
    } 

    if (carr->base != NULL) {
      free(carr->base);
    }

    carr->base    = mem;
    carr->endbase = mem + asize;
    carr->begin   = mem;
    carr->end     = mem + new_size * carr->vsize;
    carr->cap     = new_cap;
    carr->size    = new_size;
  }
}

/************************************************************************************************
 * stdcarr_construct: Construct an initially empty circular array.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_construct(stdcarr *carr, stdsize vsize, stduint8 opts) 
{
  stdcode ret = STDESUCCESS;

  if (vsize == 0 || (opts & ~(STDCARR_OPTS_NO_AUTO_GROW | STDCARR_OPTS_NO_AUTO_SHRINK)) != 0) {
    ret = STDEINVAL;
    goto stdcarr_construct_fail;
  }

  carr->base    = NULL;
  carr->endbase = NULL;
  carr->begin   = NULL;
  carr->end     = NULL;
  carr->cap     = 0;
  carr->size    = 0;
  carr->vsize   = vsize;
  carr->opts    = opts;

  goto stdcarr_construct_end;

  /* error handling and return */

 stdcarr_construct_fail:
  carr->vsize = 0;  /* make STDCARR_IS_LEGAL(carr) false */

 stdcarr_construct_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_copy_construct: Construct a copy of an array.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_copy_construct(stdcarr *dst, const stdcarr *src) 
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(src));

  *dst = *src;

  if (src->base != NULL) {
    stdsize asize = src->vsize * src->cap;

    if ((dst->base = (char*) malloc(asize)) == NULL) {
      ret = STDENOMEM;
      goto stdcarr_copy_construct_fail;
    }

    dst->endbase = dst->base + asize;
    dst->begin   = dst->base;
    dst->end     = stdcarr_low_copy_to_buf(dst->base, src, src->begin, src->end);
  }

  goto stdcarr_copy_construct_end;
  
  /* error handling and return */

 stdcarr_copy_construct_fail:
  dst->vsize = 0;  /* make STDCARR_IS_LEGAL(dst) false */
  
 stdcarr_copy_construct_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_destruct: Reclaim a circular array's resources and invalidate it.
 ***********************************************************************************************/

STDINLINE void stdcarr_destruct(stdcarr *carr) 
{
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  if (carr->base != NULL) {
    free(carr->base); 
    carr->base = NULL;
  }

  carr->vsize = 0;  /* make STDCARR_IS_LEGAL(carr) false */
}

/************************************************************************************************
 * stdcarr_set_eq: Set 'dst' to have the same contents as 'src.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_set_eq(stdcarr *dst, stdcarr *src)
{
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(dst) && STDCARR_IS_LEGAL(src) && dst->vsize == src->vsize);

  if (dst == src) {
    goto stdcarr_set_eq_end;
  }

  if ((ret = stdcarr_resize(dst, src->size)) != STDESUCCESS) {
    goto stdcarr_set_eq_end;
  }

  dst->begin = dst->base;
  dst->end   = stdcarr_low_copy_to_buf(dst->begin, src, src->begin, src->end);

 stdcarr_set_eq_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_swap: Make 'carr1' reference 'carr2's sequence and vice versa.
 ***********************************************************************************************/

STDINLINE void stdcarr_swap(stdcarr *carr1, stdcarr *carr2)
{
  stdcarr cpy;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr1) && STDCARR_IS_LEGAL(carr2));

  STDSWAP(*carr1, *carr2, cpy);
}

/************************************************************************************************
 * stdcarr_begin: Get an iterator to the beginning of a circular array.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_begin(const stdcarr *carr, stdit *it) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  it->type_id           = STDCARR_IT_ID;
  it->impl.carr.val     = (char*) carr->begin;
  it->impl.carr.base    = (char*) carr->base;
  it->impl.carr.endbase = (char*) carr->endbase;
  it->impl.carr.begin   = (char*) carr->begin;
  it->impl.carr.end     = (char*) carr->end;
  it->impl.carr.vsize   = carr->vsize;

  return it;
}

/************************************************************************************************
 * stdcarr_last: Get an iterator to the last entry of a circular array.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_last(const stdcarr *carr, stdit *it) 
{
  STDBOUNDS_CHECK(carr->size != 0);

  return stdcarr_it_prev(stdcarr_end(carr, it));
}

/************************************************************************************************
 * stdcarr_end: Get an iterator to the sentinel end entry of a circular array.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_end(const stdcarr *carr, stdit *it) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  it->type_id           = STDCARR_IT_ID;
  it->impl.carr.val     = (char*) carr->end;
  it->impl.carr.base    = (char*) carr->base;
  it->impl.carr.endbase = (char*) carr->endbase;
  it->impl.carr.begin   = (char*) carr->begin;
  it->impl.carr.end     = (char*) carr->end;
  it->impl.carr.vsize   = carr->vsize;

  return it;
}

/************************************************************************************************
 * stdcarr_get: Get an iterator to the 'elem_num'th entry in a circular array.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_get(const stdcarr *carr, stdit *it, stdsize elem_num) 
{ 
  STDBOUNDS_CHECK(elem_num <= carr->size);

  return stdcarr_it_advance(stdcarr_begin(carr, it), elem_num);
}

/************************************************************************************************
 * stdcarr_is_begin: Return whether or not an iterator references the begin of an array.
 ***********************************************************************************************/

STDINLINE stdbool stdcarr_is_begin(const stdcarr *carr, const stdit *it) 
{
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  return it->impl.carr.val == carr->begin;
}

/************************************************************************************************
 * stdcarr_is_end: Return whether or not an iterator references the end of an array.
 ***********************************************************************************************/

STDINLINE stdbool stdcarr_is_end(const stdcarr *carr, const stdit *it) 
{
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  return it->impl.carr.val == carr->end;
}

/************************************************************************************************
 * stdcarr_rank: Returns the 0-based rank of an iterator.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_rank(const stdcarr *carr, const stdit *it) 
{
  stdsize rank;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  if (it->impl.carr.val >= carr->begin) {
    rank = (stdsize) (it->impl.carr.val - carr->begin) / carr->vsize;

  } else {
    rank = (stdsize) ((carr->endbase - carr->begin) + (it->impl.carr.val - carr->base)) / carr->vsize;
  }

  return rank;
}

/************************************************************************************************
 * stdcarr_size: Return the number of elements in an array.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_size(const stdcarr *carr) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return carr->size; 
}

/************************************************************************************************
 * stdcarr_empty: Return whether or not an array's size is zero.
 ***********************************************************************************************/

STDINLINE stdbool stdcarr_empty(const stdcarr *carr) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return carr->size == 0; 
}

/************************************************************************************************
 * stdcarr_high_capacity: Return the size beyond which 'carr' will
 * (try to) grow.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_high_capacity(const stdcarr *carr) 
{
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return (carr->cap != 0 ? carr->cap - 1 : 0);  /* -1 for unusable sentinel position */
}

/************************************************************************************************
 * stdcarr_low_capacity: Return the size at (or below) which 'carr'
 * will (try to) shrink.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_low_capacity(const stdcarr *carr) 
{
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return (carr->cap >> 2);
}

/************************************************************************************************
 * stdcarr_max_size: Return the theoretical maximum number of elements 'carr' could hold.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_max_size(const stdcarr *carr) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return STDSIZE_MAX / carr->vsize; 
}

/************************************************************************************************
 * stdcarr_val_size: Return the size in bytes of the values 'carr' contains.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_val_size(const stdcarr *carr) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  
  return carr->vsize; 
}

/************************************************************************************************
 * stdcarr_resize: Resize an array to contain 'num_elems' elements.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_resize(stdcarr *carr, stdsize num_elems) 
{
  stdcode ret = STDESUCCESS;
  char *  ptr = carr->end;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  if (num_elems > carr->size) {
    ret = stdcarr_low_insert_shift(carr, &ptr, (num_elems - carr->size) * carr->vsize, num_elems, STDTRUE);

  } else if (num_elems < carr->size) {
    stdcarr_low_erase_shift(carr, &ptr, (carr->size - num_elems) * carr->vsize, num_elems, STDFALSE);
  }

  return ret;
}

/************************************************************************************************
 * stdcarr_clear: Set the size of 'carr' to zero.
 ***********************************************************************************************/

STDINLINE void stdcarr_clear(stdcarr *carr) 
{
  stdcarr_resize(carr, 0);
}

/************************************************************************************************
 * stdcarr_set_capacity: Set the capacity of 'carr' to num_elems.
 * Ignores all auto-allocation considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_set_capacity(stdcarr *carr, stdsize num_elems) 
{
  stdcode ret = STDESUCCESS;

  if (num_elems != stdcarr_high_capacity(carr)) {

    if (num_elems != 0) {
      stdsize new_cap = num_elems + 1;            /* +1 for sentinel end position */
      stdsize asize   = new_cap * carr->vsize;
      char *  mem;

      /* alloc table */

      if ((mem = (char*) malloc(asize)) == NULL) {  
	ret = STDENOMEM;
	goto stdcarr_set_capacity_end;
      }

      /* truncate if requested capacity is smaller than size */
      
      if (num_elems < carr->size) {
	carr->end  = stdcarr_low_forward(carr->begin, num_elems * carr->vsize, carr->base, carr->endbase);
	carr->size = num_elems;
      }

      /* copy to new table */

      stdcarr_low_copy_to_buf(mem, carr, carr->begin, carr->end);

      /* free old table */

      if (carr->base != NULL) {
	free(carr->base);
      }

      carr->base    = mem;
      carr->endbase = mem + asize;
      carr->begin   = mem;
      carr->end     = mem + carr->size * carr->vsize;
      carr->cap     = new_cap;

    } else {

      if (carr->base != NULL) {
	free(carr->base);
      }

      carr->base    = NULL;
      carr->endbase = NULL;
      carr->begin   = NULL;
      carr->end     = NULL;
      carr->cap     = 0;
      carr->size    = 0;
    }
  }

 stdcarr_set_capacity_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_reserve: Ensures 'carr' can contain 'num_elems' elements
 * without any additional reallocations.  Ignores all auto allocation
 * considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_reserve(stdcarr *carr, stdsize num_elems) 
{
  stdcode ret = STDESUCCESS;

  if (num_elems > stdcarr_high_capacity(carr)) {
    ret = stdcarr_set_capacity(carr, num_elems);
  }

  return ret;
}

/************************************************************************************************
 * stdcarr_shrink_fit: Sets 'carr's capacity to 'carr's size. Ignores
 * all auto allocation considerations.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_shrink_fit(stdcarr *carr) 
{ 
  return stdcarr_set_capacity(carr, carr->size);
}

/************************************************************************************************
 * stdcarr_push_front: Push a copy of 'val' onto the beginning of
 * 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_front(stdcarr *carr, const void *val) 
{
  stdit it;

  return stdcarr_insert(carr, stdcarr_begin(carr, &it), val);
}

/************************************************************************************************
 * stdcarr_push_front_n: Push copies of the 'num_push' values in
 * 'vals' onto the beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_front_n(stdcarr *carr, const void *vals, stdsize num_push) 
{
  stdit it;

  return stdcarr_insert_n(carr, stdcarr_begin(carr, &it), vals, num_push);
}

/************************************************************************************************
 * stdcarr_push_front_seq: Push a sequence of elements onto the
 * beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_front_seq(stdcarr *carr, const stdit *b, const stdit *e)
{
  stdit it;

  return stdcarr_insert_seq(carr, stdcarr_begin(carr, &it), b, e);
}

/************************************************************************************************
 * stdcarr_push_front_seq_n: Push a sequence of elements onto the
 * beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_front_seq_n(stdcarr *carr, const stdit *b, stdsize num_push)
{
  stdit it;

  return stdcarr_insert_seq_n(carr, stdcarr_begin(carr, &it), b, num_push);
}

/************************************************************************************************
 * stdcarr_push_front_rep: Push 'num_push' copies of 'val' onto
 * the beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_front_rep(stdcarr *carr, const void *val, stdsize num_times) 
{
  stdit it;

  return stdcarr_insert_rep(carr, stdcarr_begin(carr, &it), val, num_times);
}

/************************************************************************************************
 * stdcarr_pop_front: Pop an element off of the beginning of an array.
 ***********************************************************************************************/

STDINLINE void stdcarr_pop_front(stdcarr *carr) 
{
  stdcarr_pop_front_n(carr, 1);
}

/************************************************************************************************
 * stdcarr_pop_front_n: Pop multiple elements off of the beginning of
 * an array.
 ***********************************************************************************************/

STDINLINE void stdcarr_pop_front_n(stdcarr *carr, stdsize num_pop) 
{
  char * begin_ptr = carr->begin;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));
  STDBOUNDS_CHECK(num_pop <= carr->size);

  stdcarr_low_erase_shift(carr, &begin_ptr, num_pop * carr->vsize, carr->size - num_pop, STDTRUE);
}

/************************************************************************************************
 * stdcarr_push_back: Push a copy of 'val' onto the end of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_back(stdcarr *carr, const void *val) 
{
  stdit it;

  return stdcarr_insert(carr, stdcarr_end(carr, &it), val);
}

/************************************************************************************************
 * stdcarr_push_back_n: Push copies of the 'num_push' values in 'vals'
 * onto the end of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_back_n(stdcarr *carr, const void *vals, stdsize num_push) 
{
  stdit it;

  return stdcarr_insert_n(carr, stdcarr_end(carr, &it), vals, num_push);
}

/************************************************************************************************
 * stdcarr_push_back_seq: Push a sequence of elements onto the
 * beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_back_seq(stdcarr *carr, const stdit *b, const stdit *e)
{
  stdit it;

  return stdcarr_insert_seq(carr, stdcarr_end(carr, &it), b, e);
}

/************************************************************************************************
 * stdcarr_push_back_seq_n: Push a sequence of elements onto the
 * beginning of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_back_seq_n(stdcarr *carr, const stdit *b, stdsize num_push)
{
  stdit it;

  return stdcarr_insert_seq_n(carr, stdcarr_end(carr, &it), b, num_push);
}

/************************************************************************************************
 * stdcarr_push_back_rep: Push 'num_push' copies of 'val' onto the end
 * of 'carr.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_push_back_rep(stdcarr *carr, const void *val, stdsize num_times) 
{
  stdit it;

  return stdcarr_insert_rep(carr, stdcarr_end(carr, &it), val, num_times);
}

/************************************************************************************************
 * stdcarr_pop_back: Pop an element off of the end of an array.
 ***********************************************************************************************/

STDINLINE void stdcarr_pop_back(stdcarr *carr) 
{
  stdcarr_pop_back_n(carr, 1);
}

/************************************************************************************************
 * stdcarr_pop_back_n: Pop multiple elements off of the end of an
 * array.
 ***********************************************************************************************/

STDINLINE void stdcarr_pop_back_n(stdcarr *carr, stdsize num_pop) 
{
  char * end_ptr = carr->end;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));  
  STDBOUNDS_CHECK(num_pop <= carr->size);

  stdcarr_low_erase_shift(carr, &end_ptr, num_pop * carr->vsize, carr->size - num_pop, STDFALSE);
}

/************************************************************************************************
 * stdcarr_insert: Insert a copy of 'val' into 'carr' before 'it.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_insert(stdcarr *carr, stdit *it, const void *val) 
{
  return stdcarr_insert_n(carr, it, val, 1);
}

/************************************************************************************************
 * stdcarr_insert_n: Insert copies of the 'num_insert' elements in
 * 'vals' into 'carr' before 'it.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_insert_n(stdcarr *carr, stdit *it, const void *vals, stdsize num_insert) 
{
  stdcode ret;
  stdsize diff;
  stdsize delta;
  stdbool shift_right;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  delta = num_insert * carr->vsize;

  /* compute whether it is roughly cheaper to shift right or left if we don't realloc */

  shift_right = STDCARR_INS_SHIFT_RIGHT(carr, it->impl.carr.val);

  /* make room for the insertion */
  
  if ((ret = stdcarr_low_insert_shift(carr, &it->impl.carr.val, delta, 
				      carr->size + num_insert, shift_right)) != STDESUCCESS) {
    goto stdcarr_insert_n_end;
  }

  /* copy the elements */

  diff = (stdsize) (carr->endbase - it->impl.carr.val);  /* open space b4 wrap around */

  if (diff >= delta) {  /* vals fits in open space b4 wrap around */
    memcpy(it->impl.carr.val, vals, delta);

  } else {              /* wrap data around */
    memcpy(it->impl.carr.val, vals, diff);
    memcpy(carr->base, (char*) vals + diff, (stdsize) (delta - diff));
  }

 stdcarr_insert_n_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_insert_seq: Insert a sequence of elements into 'carr' before
 * 'it.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_insert_seq(stdcarr *carr, stdit *it, const stdit *b, const stdit *e)
{
  stdcode  ret;
  stdssize num_insert;

  if ((num_insert = stdit_distance(b, e)) < 0) {  /* calc how many elements in [b, e) */
    ret = STDEINVAL;
    goto stdcarr_insert_seq_end;
  }

  ret = stdcarr_insert_seq_n(carr, it, b, num_insert);

 stdcarr_insert_seq_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_insert_seq_n: Insert a sequence of elements into 'carr' before
 * 'it.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_insert_seq_n(stdcarr *carr, stdit *it, const stdit *b, stdsize num_insert)
{
  stdcode ret;
  stdsize delta;
  stdbool shift_right;
  char *  dst_it;
  stdit   src_it;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  delta = num_insert * carr->vsize;

  /* compute whether it is roughly cheaper to shift right or left if we don't realloc */

  shift_right = STDCARR_INS_SHIFT_RIGHT(carr, it->impl.carr.val);

  /* make room for the insertion */
  
  if ((ret = stdcarr_low_insert_shift(carr, &it->impl.carr.val, delta, 
				      carr->size + num_insert, shift_right)) != STDESUCCESS) {
    goto stdcarr_insert_seq_n_end;
  }

  /* copy the elements */

  dst_it = it->impl.carr.val;
  src_it = *b;

  while (num_insert-- != 0) {
    memcpy(dst_it, stdit_val(&src_it), carr->vsize);

    dst_it = stdcarr_low_forward(dst_it, carr->vsize, carr->base, carr->endbase);
    stdit_next(&src_it);
  }

 stdcarr_insert_seq_n_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_insert_rep: Insert 'num_times' copies of 'val' into 'carr' before 'it.'
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_insert_rep(stdcarr *carr, stdit *it, const void *val, stdsize num_times) 
{
  stdcode ret;
  stdsize delta;
  stdbool shift_right;
  char *  dst_it;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));

  delta = num_times * carr->vsize;

  /* compute whether it is roughly cheaper to shift right or left if we don't realloc */

  shift_right = STDCARR_INS_SHIFT_RIGHT(carr, it->impl.carr.val);

  /* make room for the insertion */
  
  if ((ret = stdcarr_low_insert_shift(carr, &it->impl.carr.val, delta, 
				      carr->size + num_times, shift_right)) != STDESUCCESS) {
    goto stdcarr_insert_rep_end;
  }

  /* copy the elements */

  dst_it = it->impl.carr.val;

  while (num_times-- != 0) {
    memcpy(dst_it, val, carr->vsize);

    dst_it = stdcarr_low_forward(dst_it, carr->vsize, carr->base, carr->endbase);
  }

 stdcarr_insert_rep_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_erase: Erase the element pointed at by 'it' from 'carr.'
 ***********************************************************************************************/

STDINLINE void stdcarr_erase(stdcarr *carr, stdit *it) 
{
  stdcarr_erase_n(carr, it, 1);
}

/************************************************************************************************
 * stdcarr_erase_n: Erase 'num_erase' elements from 'carr' starting at
 * 'it.'
 ***********************************************************************************************/

STDINLINE void stdcarr_erase_n(stdcarr *carr, stdit *it, stdsize num_erase) 
{
  stdsize delta;
  stdbool shift_right;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr) && STDIT_CARR_IS_LEGAL(it) && STDCARR_IT_IS_LEGAL(carr, &it->impl.carr));
  
#ifdef STDBOUNDS_CHECKS
  {
    stdit c_end;
    
    stdcarr_end(carr, &c_end);
    STDBOUNDS_CHECK((stdsize) stdcarr_it_cmp(&c_end, it) >= num_erase);
  }
#endif

  delta = num_erase * carr->vsize;

  /* determine whether it is roughly cheaper to shift right or left if we don't realloc */

  shift_right = STDCARR_ERS_SHIFT_RIGHT(carr, it->impl.carr.val, num_erase);

  /* if we are shifting left, advance 'it' to point to end of erasure */

  if (!shift_right) {  
    it->impl.carr.val = stdcarr_low_forward(it->impl.carr.val, delta, carr->base, carr->endbase);
  }

  stdcarr_low_erase_shift(carr, &it->impl.carr.val, delta, carr->size - num_erase, shift_right);
}

/************************************************************************************************
 * stdcarr_erase_seq: Erase a sequence of elements from 'carr.'
 ***********************************************************************************************/

STDINLINE void stdcarr_erase_seq(stdcarr *carr, stdit *b, stdit *e) 
{
  stdssize diff = stdcarr_it_cmp(e, b);

  STDSAFETY_CHECK(diff >= 0);

  stdcarr_erase_n(carr, b, diff);
  *e = *b;
}

/************************************************************************************************
 * stdcarr_get_opts: Get the options of an array.
 ***********************************************************************************************/

STDINLINE stduint8 stdcarr_get_opts(const stdcarr *carr) 
{ 
  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  return carr->opts;
}

/************************************************************************************************
 * stdcarr_set_opts: Set the options of an array.
 ***********************************************************************************************/

STDINLINE stdcode stdcarr_set_opts(stdcarr *carr, stduint8 opts)
{ 
  stdcode ret = STDESUCCESS;

  STDSAFETY_CHECK(STDCARR_IS_LEGAL(carr));

  if ((opts & ~(STDCARR_OPTS_NO_AUTO_GROW | STDCARR_OPTS_NO_AUTO_SHRINK)) != 0) {
    ret = STDEINVAL;
    goto stdcarr_set_opts_end;
  }
  
  carr->opts = opts;

 stdcarr_set_opts_end:
  return ret;
}

/************************************************************************************************
 * stdcarr_it_val: Get a pointer to the element to which 'it' refers.
 ***********************************************************************************************/

STDINLINE void *stdcarr_it_val(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));

  return it->impl.carr.val;
}

/************************************************************************************************
 * stdcarr_it_val_size: Return the size in bytes of the value type
 * 'it' references.
 ***********************************************************************************************/

STDINLINE stdsize stdcarr_it_val_size(const stdit *it) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));

  return it->impl.carr.vsize;
}

/************************************************************************************************
 * stdcarr_it_eq: Compare two iterators for equality (refer to same element).
 ***********************************************************************************************/

STDINLINE stdbool stdcarr_it_eq(const stdit *it1, const stdit *it2) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it1) && STDIT_CARR_IS_LEGAL(it2) && 
		  it1->impl.carr.base    == it2->impl.carr.base && 
		  it1->impl.carr.endbase == it2->impl.carr.endbase && 
		  it1->impl.carr.begin   == it2->impl.carr.begin &&
		  it1->impl.carr.end     == it2->impl.carr.end &&
		  it1->impl.carr.vsize   == it2->impl.carr.vsize);

  return it1->impl.carr.val == it2->impl.carr.val;
}

/************************************************************************************************
 * stdcarr_it_cmp: Compare two iterators for rank difference.
 ***********************************************************************************************/

STDINLINE stdssize stdcarr_it_cmp(const stdit *it_gen1, const stdit *it_gen2) 
{  
  stdssize           ret;
  const stdcarr_it * it1 = &it_gen1->impl.carr;
  const stdcarr_it * it2 = &it_gen2->impl.carr;

  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it_gen1) && STDIT_CARR_IS_LEGAL(it_gen2) && 
		  it1->base    == it2->base && 
		  it1->endbase == it2->endbase && 
		  it1->begin   == it2->begin && 
		  it1->end     == it2->end && 
		  it1->vsize   == it2->vsize);

  if (it1->val >= it1->begin) {
    ret = (it2->val >= it1->begin ? it1->val - it2->val : (it1->val - it1->endbase) + (it1->base - it2->val));

  } else {
    ret = (it2->val < it1->begin ? it1->val - it2->val : (it1->endbase - it2->val) + (it1->val - it1->base));
  }

  return ret / it1->vsize;
}

/************************************************************************************************
 * stdcarr_it_next: Advance 'it' by one position towards end.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_it_next(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.carr.val != it->impl.carr.end);

  it->impl.carr.val = stdcarr_low_forward(it->impl.carr.val, it->impl.carr.vsize, 
					  it->impl.carr.base, it->impl.carr.endbase);

  return it;
}

/************************************************************************************************
 * stdcarr_it_advance: Advance 'it' by 'num_advance' positions towards end.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_it_advance(stdit *it, stdsize num_advance) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.carr.val <= it->impl.carr.end ?
		  (stdsize) (it->impl.carr.end - it->impl.carr.val) <= num_advance * it->impl.carr.vsize :
		  (stdsize) (it->impl.carr.endbase - it->impl.carr.val) + (it->impl.carr.end - it->impl.carr.base) <= num_advance * it->impl.carr.vsize);

  it->impl.carr.val = stdcarr_low_forward(it->impl.carr.val, it->impl.carr.vsize * num_advance, 
					  it->impl.carr.base, it->impl.carr.endbase);

  return it;
}


/************************************************************************************************
 * stdcarr_it_prev: Advance 'it' by one position towards begin.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_it_prev(stdit *it) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.carr.val != it->impl.carr.begin);

  it->impl.carr.val = stdcarr_low_backward(it->impl.carr.val, it->impl.carr.vsize, 
					   it->impl.carr.base, it->impl.carr.endbase);

  return it;
}

/************************************************************************************************
 * stdcarr_it_retreat: Advance 'it' by 'num_retreat' positions towards begin.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_it_retreat(stdit *it, stdsize num_retreat) 
{
  STDSAFETY_CHECK(STDIT_CARR_IS_LEGAL(it));
  STDBOUNDS_CHECK(it->impl.carr.val >= it->impl.carr.begin ?
		  (stdsize) (it->impl.carr.val - it->impl.carr.begin) <= num_retreat * it->impl.carr.vsize :
		  (stdsize) (it->impl.carr.val - it->impl.carr.base) + (it->impl.carr.endbase - it->impl.carr.begin) <= num_retreat * it->impl.carr.vsize);

  it->impl.carr.val = stdcarr_low_backward(it->impl.carr.val, it->impl.carr.vsize * num_retreat, 
					   it->impl.carr.base, it->impl.carr.endbase);

  return it;
}

/************************************************************************************************
 * stdcarr_it_offset: Advance 'it' by 'offset' positions towards end.
 ***********************************************************************************************/

STDINLINE stdit *stdcarr_it_offset(stdit *it, stdssize offset) 
{
  return (offset >= 0 ? stdcarr_it_advance(it, (stdsize) offset) : stdcarr_it_retreat(it, (stdsize) -offset));
}

#ifdef __cplusplus
}
#endif
