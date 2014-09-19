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

#include <stdutil/stdutil.h>
#include <stdutil/stderror.h>
#include <stdutil/stdtime.h>
#include <stdutil/stdskl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: consider shrinking end_node's arrays when the top level
   becomes empty (down to some minimum height like 4 or 5); this is
   easy to detect: if the node being removed has the same height as
   the end_node and his top level prev and next are the end node then
   the level is about to become empty; alternatively after removal you
   can just check the top levels for self reference; also might not
   want to make end_node taller than the tallest node on insert --
   don't know how big an effect on find this will have as find as it
   will simply run down end_node's nexts array while next equals
   end_node not doing any real work
*/

#define STDSKL_MAX_HEIGHT 31

#define STDSKL_IS_LEGAL(l)       ((l)->end_node != NULL && (l)->ksize != 0 && (l)->bits_left >= 0 && (l)->bits_left < 32)
#define STDSKL_IT_IS_LEGAL(l, i) ((i)->ksize == (l)->ksize && (i)->vsize == (l)->vsize)
#define STDIT_SKL_IS_LEGAL(i)    (((i)->type_id == STDSKL_IT_ID || (i)->type_id == STDSKL_IT_KEY_ID) && \
				  (i)->impl.skl.node != NULL && (i)->impl.skl.ksize != 0)

#define STDSKL_NKEY(node) ((node)->key)
#define STDSKL_NVAL(node) ((node)->val)

/************************************************************************************************
 * stdskl_low_key_cmp:  Compares 2 keys, uses memcmp if no comparison fcn defined.
 ***********************************************************************************************/

STDINLINE static int stdskl_low_key_cmp(const stdskl *l, const void *k1, const void *k2)
{
  return (l->cmp_fcn == NULL ? memcmp(k1, k2, l->ksize) : l->cmp_fcn(k1, k2));
}

/************************************************************************************************
 * stdskl_low_create_node: Create a node to be inserted into 'l.'
 *
 * For random height generation, a node is promoted to a higher height
 * w/ 25% chance.  The optimum promotion chance for a skiplist is
 * somewhere between 28% and 37% depending on the analysis used.  25%
 * gives nearly optimal performance while using less memory and not
 * requiring excessive random bit generation.
 ***********************************************************************************************/

STDINLINE static stdskl_node *stdskl_low_create_node(stdskl *l, stdint8 height, const void *key, const void *val)
{
  stdskl_node * node;
  stdsize       mem_tot;
  stdsize       prevs_off;
  stdsize       nexts_off;
  stdsize       key_off;
  stdsize       val_off;

  if (height == -1) {                                             /* height generation requested */
    stdbool keep_going = STDTRUE;

    for (; keep_going && height < STDSKL_MAX_HEIGHT; ++height) {  /* loop while random height increases */

      if (l->bits_left == 0) {                                    /* out of random bits */
	l->rand_bits = stdrand32(l->seed);                        /* generate 32 new ones */
	l->bits_left = 32;
      }

      keep_going = ((l->rand_bits & 0x3) == 0x3);                 /* continue loop w/ 25% chance */
      l->bits_left  -= 2;                                         /* remove used bits */
      l->rand_bits >>= 2;
    }
  }

  /* calculate memory needed for node and pointer arrays and the offsets for the pointers in the node */

  mem_tot   = sizeof(stdskl_node);                  /* memory for node structure */
  prevs_off = mem_tot;                              /* structure is already aligned for void*'s -> good enough alignment */

  mem_tot  += (height + 1) * sizeof(stdskl_node*);  /* memory for prevs array */
  nexts_off = mem_tot;                              /* structure is already aligned for void*'s -> good enough alignment */

  mem_tot  += (height + 1) * sizeof(stdskl_node*);  /* memory for nexts array */
  mem_tot   = STDARCH_PADDED_SIZE(mem_tot);         /* pad memory for key */
  key_off   = mem_tot;

  mem_tot  += STDARCH_PADDED_SIZE(l->ksize);        /* memory for key and padding for value */
  val_off   = mem_tot;

  mem_tot  += l->vsize;                             /* memory for value */

  /* allocate the node and extra memory */
  
  if ((node = (stdskl_node*) malloc(mem_tot)) == NULL) {
    goto stdskl_low_create_node_end;
  }

  /* init node: set height, point pointers to appropriate offsets in allocated memory block */

  node->height = height;
  node->prevs  = (stdskl_node**) ((char*) node + prevs_off);
  node->nexts  = (stdskl_node**) ((char*) node + nexts_off);
  node->key    = (char*) node + key_off;
  node->val    = (char*) node + val_off;

  /* copy key + value if given */

  if (key != NULL) {
    memcpy((void*) STDSKL_NKEY(node), key, l->ksize);
    memcpy(STDSKL_NVAL(node), val, l->vsize);
  }

 stdskl_low_create_node_end:
  return node;
}

/************************************************************************************************
 * stdskl_low_find_right: Search 'l' for 'key' from left to right. 
 * 
 * If (find_any) immediately return on any match. Otherwise if 'key'
 * exists in 'l' return the "leftmost" (least) match if multiple
 * matches exist.  On no match, returns the "leftmost" (least) entry
 * in 'l' with a greater key, or end if no such entry exists.
 ***********************************************************************************************/

STDINLINE static stdbool stdskl_low_find_right(const stdskl *l, const void *key, 
					       stdbool find_any, stdskl_node **pos)
{
  const stdskl_node * next = NULL;
  const stdskl_node * curr = l->end_node;
  stdint8             lvl  = l->end_node->height;
  int                 cmp  = -1;  /* get rid of compiler warning */

  /* run down to bottom level list */

  while (lvl > 0) {
    next = curr->nexts[lvl];

    /* run right at this level while key is greater than next's key */

    while (next != l->end_node && 
	   (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(next))) > 0) {
      curr = next;
      next = next->nexts[lvl];
    }

    /* if we found a match and any match will do */

    if (find_any && next != l->end_node && cmp == 0) {
      *pos = (stdskl_node*) next;
      return STDTRUE;
    }

    /* run down curr's levels while they equal next */

    for (--lvl; lvl > 0 && curr->nexts[lvl] == next; --lvl);
  }

  /* lvl is zero: run right on bottom level list trying to match key */

  curr = curr->nexts[0];

  if (curr != next) {

    while (curr != l->end_node &&
	   (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(curr))) > 0) {
      curr = curr->nexts[0];
    }
  }  /* else we already compared key to curr above */

  *pos = (stdskl_node*) curr;

  return (curr != l->end_node && cmp == 0);
}

/************************************************************************************************
 * stdskl_low_find_left: Search 'l' for 'key' from right to left.
 * 
 * If (find_any) immediately return on any match. Otherwise if 'key'
 * exists in 'l' return the "rightmost" (greatest) match if multiple
 * matches exist.  On no match, returns the "rightmost" (greatest)
 * entry in 'l' with a lesser key, or end if no such entry exists.
 ***********************************************************************************************/

STDINLINE static stdbool stdskl_low_find_left(const stdskl *l, const void *key, 
					      stdbool find_any, stdskl_node **pos)
{
  const stdskl_node * prev = NULL;
  const stdskl_node * curr = l->end_node;
  stdint8             lvl  = l->end_node->height;
  int                 cmp  = -1;  /* get rid of compiler warning */

  /* run down to bottom level list */

  while (lvl > 0) {
    prev = curr->prevs[lvl];

    /* run left at this level while key is less than prev's key */

    while (prev != l->end_node && 
	   (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(prev))) < 0) {
      curr = prev;
      prev = prev->prevs[lvl];
    }

    /* if we found a match and any match will do */

    if (find_any && prev != l->end_node && cmp == 0) {
      *pos = (stdskl_node*) prev;
      return STDTRUE;
    }

    /* run down curr's prevs levels while they equal prev */

    for (--lvl; lvl > 0 && curr->prevs[lvl] == prev; --lvl);
  }

  /* run left on bottom level list trying to match key */

  curr = curr->prevs[0];

  if (curr != prev) {

    while (curr != l->end_node &&
	   (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(curr))) < 0) {
      curr = curr->prevs[0];
    }
  }  /* else we already compared key to curr above */

  *pos = (stdskl_node*) curr;

  return (curr != l->end_node && cmp == 0);
}

/************************************************************************************************
 * stdskl_low_link_right: Link 'node' into entries greater than or
 * equal to 'ins_pos' in 'l.'
 ***********************************************************************************************/

STDINLINE static void stdskl_low_link_right(const stdskl *l, stdskl_node *ins_pos, stdskl_node *node)
{
  stdskl_node * next   = ins_pos;
  stdint8       height = node->height;
  stdint8       lvl    = 0;

  node->nexts[0] = next;
  next->prevs[0] = node;

  while (lvl != height) {

    /* if we've maxed out next's height, then find a later, taller node on this lvl */

    while (lvl == next->height) {  /* NOTE: node->height <= end_node->height -> lvl < end_node->height here */
      next = next->nexts[lvl];     /* so we can't erroneously wrap around past sentinel end node here */
    }

    ++lvl;
    node->nexts[lvl] = next;
    next->prevs[lvl] = node;
  }
}

/************************************************************************************************
 * stdskl_low_link_left: Link in 'node' to entries less than 'ins_pos'
 * in 'l.'
 ***********************************************************************************************/

STDINLINE static void stdskl_low_link_left(const stdskl *l, stdskl_node *ins_pos, stdskl_node *node)
{
  stdskl_node * prev   = ins_pos->prevs[0];  /* NOTE: this is why we must link_left b4 link_right in insert! */
  stdint8       height = node->height;
  stdint8       lvl    = 0;

  node->prevs[0] = prev;
  prev->nexts[0] = node;

  while (lvl != height) {

    /* if we've maxed out prev's height, then find an earlier, taller node on this lvl */

    while (lvl == prev->height) {  /* NOTE: node->height <= end_node->height -> lvl < end_node->height here */
      prev = prev->prevs[lvl];     /* so we can't erroneously wrap around past sentinel end node here */
    }

    ++lvl;
    node->prevs[lvl] = prev;
    prev->nexts[lvl] = node;
  }
}

/************************************************************************************************
 * stdskl_low_insert: Insert multiple keys and values into a list
 * using an iterator sequence with various options.
 ***********************************************************************************************/

STDINLINE static stdcode stdskl_low_insert(stdskl *l, stdit *it, const stdit *b, const stdit *e, stdsize num_ins, 
					   stdbool hint, stdbool overwrite, stdbool advance)
{
  stdcode       ret       = STDESUCCESS;
  stdskl_node * node;
  stdskl_node * prev;
  stdskl_node * next      = (it != NULL ? it->impl.skl.node : l->end_node);
  stdskl_node * first_ins = NULL;
  stdit         src_it    = *b;
  stdbool       keyed     = (stdit_key_size(b) != 0);
  stdint8       lvl;
  const void *  key;
  const void *  val;
  int           cmp;

  /* loop over input sequence defined either by [b, b+num_ins) or [b, e) */

  while (num_ins-- != 0 && (e == NULL || !stdit_eq(&src_it, e))) {

    /* get pointers to the key and value we are about to insert */

    val = stdit_val(&src_it);
    key = (keyed ? stdit_key(&src_it) : val);
    cmp = -1;

    /* get insertion position for this key: next */

    if (hint) {                 /* 'next' is a potential insertion point -> verify! */
      prev = next->prevs[0];

      if ((next != l->end_node && (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(next))) > 0) ||
	  (prev != l->end_node && stdskl_low_key_cmp(l, key, STDSKL_NKEY(prev)) < 0)) {

	prev = next;
	next = next->nexts[0];  /* next was inappropriate; try ++next */
	cmp  = -1;

	if ((next != l->end_node && (cmp = stdskl_low_key_cmp(l, key, STDSKL_NKEY(next))) > 0) ||
	    (prev != l->end_node && stdskl_low_key_cmp(l, key, STDSKL_NKEY(prev)) < 0)) {
	  hint = STDFALSE;      /* both next and ++next were inappropriate: do a full search */
	}
      }
    }

    if (!hint) {
      cmp = !stdskl_low_find_right(l, key, STDTRUE, &next);
    }

    hint = STDTRUE;             /* use next as a hint for following iteration (optimize for nearly contiguous, sorted inserts) */

    /* create + insert new node; or overwrite pre-existing match if so instructed and appropriate */
    
    if (!overwrite || next == l->end_node || cmp != 0) {               /* create new node to insert */

      if ((node = stdskl_low_create_node(l, -1, key, val)) == NULL) {  /* create a node with a random height */
	ret = STDENOMEM;
	goto stdskl_low_insert_end;
      }

      /* ensure that l->end_node is at least as tall as the new node */

      if (node->height > l->end_node->height) {
	stdskl_node * new_end;

	/* ratchet up new end_node's height to be at least 1 level higher (usually 2) than before */

	lvl = node->height;  /* 0 <= node->height <= STDSKL_MAX_HEIGHT */

	if (lvl < STDSKL_MAX_HEIGHT) {
	  ++lvl;
	}

	if ((new_end = stdskl_low_create_node(l, lvl, NULL, NULL)) == NULL) {
	  ret = STDENOMEM;
	  goto stdskl_low_insert_fail;
	}

	/* set new_end's nexts and prevs and update nodes ref'ing end_node to now point to new_end */
	/* loop while we haven't updated all levels of the lists AND end_node is NOT self-referring */

	lvl = 0;
	do {

	  if (l->end_node->prevs[lvl] == l->end_node) {  /* break on end_node referring to itself -> */ 
	    break;                                       /* all prevs, nexts of this and higher lvls are self-referring */
	  }

	  new_end->prevs[lvl]                 = l->end_node->prevs[lvl];
	  l->end_node->prevs[lvl]->nexts[lvl] = new_end;

	  new_end->nexts[lvl]                 = l->end_node->nexts[lvl];
	  l->end_node->nexts[lvl]->prevs[lvl] = new_end;

	} while (lvl++ != l->end_node->height);

	/* set additional lvls in new_end to be self-referencing.  NOTE: new_end is at least 1 lvl taller than end_node */

	do {
	  new_end->prevs[lvl] = new_end;
	  new_end->nexts[lvl] = new_end;

	} while (lvl++ != new_end->height);

	/* correct next if it referenced the end_node */
    
	if (next == l->end_node) {
	  next = new_end;
	}

	free(l->end_node);
	l->end_node = new_end;
      }

      /* link 'node' into list before next */

      stdskl_low_link_left(l, next, node);                  /* NOTE: we must link_left before we link_right due to loss of information */
      stdskl_low_link_right(l, next, node);

      ++l->size;
      
    } else {                                                /* overwrite existing match */
      node = next;
      memcpy((void*) STDSKL_NKEY(node), key, l->ksize);     /* overwrite */
      memcpy(STDSKL_NVAL(node), val, l->vsize);
    }

    /* remember first insertion/overwrite */

    if (first_ins == NULL) {
      first_ins = node;
    }

    /* advance 'src_it' if so instructed */

    if (advance) {
      stdit_next(&src_it);
    }
  }

  goto stdskl_low_insert_end;

  /* error handling and return */

 stdskl_low_insert_fail:
  free(node);

 stdskl_low_insert_end:
  if (it != NULL) {
    it->type_id        = STDSKL_IT_ID;
    it->impl.skl.node  = (first_ins != NULL ? first_ins : l->end_node);  /* point to end if no insert/overwrite occurred */
    it->impl.skl.ksize = l->ksize;
    it->impl.skl.vsize = l->vsize;
  }

  return ret;
}

/************************************************************************************************
 * stdskl_low_erase: Erase multiple key-value pairs from a list using
 * either an iterator sequence to delete or a number of elements to
 * erase.
 ***********************************************************************************************/

STDINLINE static stdsize stdskl_low_erase(stdskl *l, stdit *b, stdit *e, stdsize num_erase)
{
  stdskl_node * ers;
  stdskl_node * prev     = b->impl.skl.node->prevs[0];
  stdskl_node * next     = b->impl.skl.node;
  stdskl_node * end_node = (e != NULL ? e->impl.skl.node : NULL);
  stdint8       max_lvl  = 0;
  stdsize       erased;
  stdint8       lvl;

  /* go through [b, b+num_erase) or [b, e) free'ing nodes -- record
     max height of free'd nodes and how many nodes we erased
  */

  for (erased = 0; erased != num_erase && next != end_node; ++erased) {
    STDBOUNDS_CHECK(next != l->end_node);

    if (next->height > max_lvl) {
      max_lvl = next->height;
    }

    ers  = next;
    next = next->nexts[0];
    free(ers);
  }

  /* update list size */
  
  l->size -= erased;

  /* stitch together the left and right portions of the list */

  ers            = next;             /* remember where erasure stopped */
  prev->nexts[0] = next;             /* perform base re-linkage */
  next->prevs[0] = prev;

  for (lvl = 0; lvl != max_lvl; ) {  /* run left and run right up to max_lvl height nodes re-linking */
    
    while (lvl == prev->height) {
      prev = prev->prevs[lvl];
    }

    while (lvl == next->height) {
      next = next->nexts[lvl];
    }

    ++lvl;
    prev->nexts[lvl] = next;
    next->prevs[lvl] = prev;
  }
  
  /* advance 'b' and 'e' */
  
  b->impl.skl.node = ers;

  if (e != NULL) {
    e->impl.skl.node = ers;
  }

  return erased;
}

/************************************************************************************************
 * stdskl_construct: Construct an initially empty list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_construct(stdskl *l, stdsize ksize, stdsize vsize, stdcmp_fcn kcmp)
{
  stdcode ret = STDESUCCESS;
  stdint8 lvl;
  stdtime t;

  if (ksize == 0) {
    ret = STDEINVAL;
    goto stdskl_construct_fail;
  }

  l->size    = 0;
  l->ksize   = ksize;
  l->vsize   = vsize;
  l->cmp_fcn = kcmp;

  /* initialize randomization using current system time w/ subsecond precision */

  stdtime_now(&t);  
  stdrand32_dseed(l->seed, stdhcode_sfh(&t, sizeof(t)));
  l->bits_left = 0;

  /* allocate and init end_node -- NOTE: rest of list must be initialized b4 calling this fcn */

  if ((l->end_node = stdskl_low_create_node(l, 4, NULL, NULL)) == NULL) {
    ret = STDENOMEM;
    goto stdskl_construct_fail;
  }

  /* make end_node reference itself */

  lvl = l->end_node->height;

  do {
    l->end_node->nexts[lvl] = l->end_node;
    l->end_node->prevs[lvl] = l->end_node;

  } while (lvl-- != 0);

  goto stdskl_construct_end;

  /* error handling and return */

 stdskl_construct_fail:
  l->end_node = NULL;
  l->ksize    = 0;     /* make STDSKL_IS_LEGAL(l) false */

 stdskl_construct_end:
  return ret;
}

/************************************************************************************************
 * stdskl_copy_construct: Construct a copy of a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_copy_construct(stdskl *dst, const stdskl *src)
{
  stdcode ret;
  stdit   it;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(src));

  if ((ret = stdskl_construct(dst, src->ksize, src->vsize, src->cmp_fcn)) != STDESUCCESS) {
    goto stdskl_copy_construct_fail;
  }

  if ((ret = stdskl_insert_seq_n(dst, NULL, stdskl_begin(src, &it), src->size, STDFALSE)) != STDESUCCESS) {
    goto stdskl_copy_construct_fail2;
  }

  goto stdskl_copy_construct_end;

  /* error handling and return */

 stdskl_copy_construct_fail2:
  stdskl_destruct(dst);

 stdskl_copy_construct_fail:
  dst->end_node = NULL;
  dst->ksize    = 0;     /* make STDSKL_IS_LEGAL(dst) false */

 stdskl_copy_construct_end:
  return ret;
}

/************************************************************************************************
 * stdskl_destruct: Reclaim a list's resources and invalidate it.
 ***********************************************************************************************/

STDINLINE void stdskl_destruct(stdskl *l)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  stdskl_clear(l);
  free(l->end_node);

  l->end_node = NULL;
  l->ksize    = 0;     /* make STDSKL_IS_LEGAL(l) false */
}

/************************************************************************************************
 * stdskl_set_eq: Set a list to contain the same contents as another.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_set_eq(stdskl *dst, const stdskl *src)
{
  stdcode ret = STDESUCCESS;
  stdskl  cpy;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(dst) && STDSKL_IS_LEGAL(src) &&
		  dst->ksize == src->ksize && dst->vsize == src->vsize &&
		  dst->cmp_fcn == src->cmp_fcn);

  /* TODO: an improvement would be to resize the number of nodes in
     the table as needed: just slice off or splice on an appropriate
     sequence of nodes to the begin or end of the list and overwrite
     all nodes: this would cut down on "unnecessary" memory
     allocs/frees
  */

  if (dst == src) {
    goto stdskl_set_eq_end;
  }

  if ((ret = stdskl_copy_construct(&cpy, src)) != STDESUCCESS) {
    goto stdskl_set_eq_end;
  }

  stdskl_swap(dst, &cpy);
  stdskl_destruct(&cpy);

 stdskl_set_eq_end:
  return ret;
}

/************************************************************************************************
 * stdskl_swap: Set a l1 to reference l2's sequence and vice versa.
 ***********************************************************************************************/

STDINLINE void stdskl_swap(stdskl *l1, stdskl *l2)
{
  stdskl cpy;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l1) && STDSKL_IS_LEGAL(l2));

  STDSWAP(*l1, *l2, cpy);
}

/************************************************************************************************
 * stdskl_begin: Get an iterator to the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_begin(const stdskl *l, stdit *it)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  it->type_id        = STDSKL_IT_ID;
  it->impl.skl.node  = l->end_node->nexts[0];
  it->impl.skl.ksize = l->ksize;
  it->impl.skl.vsize = l->vsize;

  return it;
}

/************************************************************************************************
 * stdskl_last: Get an iterator to the last element of a list.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_last(const stdskl *l, stdit *it)
{
  STDBOUNDS_CHECK(l->size != 0);

  return stdit_prev(stdskl_end(l, it));
}

/************************************************************************************************
 * stdskl_end: Get an iterator to the sentinel end of a list.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_end(const stdskl *l, stdit *it)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  it->type_id        = STDSKL_IT_ID;
  it->impl.skl.node  = l->end_node;
  it->impl.skl.ksize = l->ksize;
  it->impl.skl.vsize = l->vsize;

  return it;
}

/************************************************************************************************
 * stdskl_get: Get an iterator to an element of a list.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_get(const stdskl *l, stdit *it, stdsize elem_num)
{
  STDBOUNDS_CHECK(elem_num <= l->size);

  if (elem_num < (l->size >> 1)) {
    stdskl_it_advance(stdskl_begin(l, it), elem_num);

  } else {
    stdskl_it_retreat(stdskl_end(l, it), l->size - elem_num);
  }

  return it;
}

/************************************************************************************************
 * stdskl_is_begin: Return whether or not an iterator refers to the beginning of a list.
 ***********************************************************************************************/

STDINLINE stdbool stdskl_is_begin(const stdskl *l, const stdit *it)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl));

  return it->impl.skl.node == l->end_node->nexts[0];
}

/************************************************************************************************
 * stdskl_is_end: Return whether or not an iterator refers to the end of a list.
 ***********************************************************************************************/

STDINLINE stdbool stdskl_is_end(const stdskl *l, const stdit *it)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl));

  return it->impl.skl.node == l->end_node;
}

/************************************************************************************************
 * stdskl_size: Return the number of key-value pairs a list contains.
 ***********************************************************************************************/

STDINLINE stdsize stdskl_size(const stdskl *l)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  return l->size;
}

/************************************************************************************************
 * stdskl_empty: Return whether or not a list's size is zero.
 ***********************************************************************************************/

STDINLINE stdbool stdskl_empty(const stdskl *l)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  return l->size == 0;
}

/************************************************************************************************
 * stdskl_clear: Set a lists size to zero.
 ***********************************************************************************************/

STDINLINE void stdskl_clear(stdskl *l)
{
  stdskl_node * curr;
  stdskl_node * prev;
  stdint8       lvl;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  /* run through list destroying all nodes */

  for (curr = l->end_node->nexts[0]; curr != l->end_node; ) {
    prev = curr;
    curr = curr->nexts[0];
    free(prev);
  }

  /* fix list entries: end_node needs to self reference at all levels and size */

  curr = l->end_node;
  lvl  = l->end_node->height;

  do {
    curr->prevs[lvl] = curr;
    curr->nexts[lvl] = curr;

  } while (lvl-- != 0);
  
  l->size = 0;
}

/************************************************************************************************
 * stdskl_find: Find a key-value pair in a list.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_find(const stdskl *l, stdit *it, const void *key)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  if (!stdskl_low_find_right(l, key, STDTRUE, &it->impl.skl.node)) {
    it->impl.skl.node = l->end_node;
  }

  it->type_id        = STDSKL_IT_ID;
  it->impl.skl.ksize = l->ksize;
  it->impl.skl.vsize = l->vsize;

  return it;
}

/************************************************************************************************
 * stdskl_lowerb: Find the least element for which kcmp(key, elem) >= 0.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_lowerb(const stdskl *l, stdit *it, const void *key)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  stdskl_low_find_right(l, key, STDFALSE, &it->impl.skl.node);

  it->type_id        = STDSKL_IT_ID;
  it->impl.skl.ksize = l->ksize;
  it->impl.skl.vsize = l->vsize;

  return it;
}

/************************************************************************************************
 * stdskl_upperb: Find the least element for which kcmp(key, elem) > 0.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_upperb(const stdskl *l, stdit *it, const void *key)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  stdskl_low_find_left(l, key, STDFALSE, &it->impl.skl.node);
  it->impl.skl.node = it->impl.skl.node->nexts[0];  /* found either the last entry for key or the prev entry if none: so get next */

  it->type_id        = STDSKL_IT_ID;
  it->impl.skl.ksize = l->ksize;
  it->impl.skl.vsize = l->vsize;

  return it;
}

/************************************************************************************************
 * stdskl_contains: Return whether or not a list contains a certain key.
 ***********************************************************************************************/

STDINLINE stdbool stdskl_contains(const stdskl *l, const void *key)
{
  stdskl_node * n;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  return stdskl_low_find_right(l, key, STDTRUE, &n);
}

/************************************************************************************************
 * stdskl_put: If 'key' already exists in the list, overwrite such an
 * entry with 'key' and 'value;' otherwise insert 'key' and 'value'
 * into the list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_put(stdskl *l, stdit *it, const void *key, const void *val, stdbool hint)
{
  return stdskl_put_n(l, it, key, val, 1, hint);
}

/************************************************************************************************
 * stdskl_put_n: For each (key, val) in [(keys, vals), (keys+num_put, vals+num_put)) 
 * perform stdskl_put(l, it, key, val).
 ***********************************************************************************************/

STDINLINE stdcode stdskl_put_n(stdskl *l, stdit *it, const void *keys, const void *vals, stdsize num_put, stdbool hint)
{
  stdit b;

  return stdskl_put_seq_n(l, it, stdit_pptr(&b, keys, vals, l->ksize, l->vsize), num_put, hint);
}

/************************************************************************************************
 * stdskl_put_seq: For each (key, val) in [b, e) perform 
 * stdskl_put(l, it, key, val).
 ***********************************************************************************************/

STDINLINE stdcode stdskl_put_seq(stdskl *l, stdit *it, const stdit *b, const stdit *e, stdbool hint)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && (stdit_eq(b, e) || STDTRUE) &&
		  (!hint || (it != NULL && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl))) &&
		  (stdit_key_size(b) == l->ksize || (stdit_key_size(b) == 0 && stdit_val_size(b) == l->ksize)) &&
		  (stdit_val_size(b) == l->vsize || l->vsize == 0));

  return stdskl_low_insert(l, it, b, e, (stdsize) -1, hint, STDTRUE, STDTRUE);
}

/************************************************************************************************
 * stdskl_put_seq_n: For each (key, val) in [b, b+num_put) perform 
 * stdskl_put(l, it, key, val).
 ***********************************************************************************************/

STDINLINE stdcode stdskl_put_seq_n(stdskl *l, stdit *it, const stdit *b, stdsize num_put, stdbool hint)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && 
		  (!hint || (it != NULL && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl))) &&
		  (stdit_key_size(b) == l->ksize || (stdit_key_size(b) == 0 && stdit_val_size(b) == l->ksize)) &&
		  (stdit_val_size(b) == l->vsize || l->vsize == 0));

  return stdskl_low_insert(l, it, b, NULL, num_put, hint, STDTRUE, STDTRUE);
}

/************************************************************************************************
 * stdskl_insert: Insert a key and value into a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_insert(stdskl *l, stdit *it, const void *key, const void *val, stdbool hint)
{
  return stdskl_insert_n(l, it, key, val, 1, hint);
}

/************************************************************************************************
 * stdskl_insert_n: Insert multiple keys and values into a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_insert_n(stdskl *l, stdit *it, const void *keys, const void *vals, stdsize num_insert, stdbool hint)
{
  stdit b;

  return stdskl_insert_seq_n(l, it, stdit_pptr(&b, keys, vals, l->ksize, l->vsize), num_insert, hint);
}

/************************************************************************************************
 * stdskl_insert_seq: Insert a sequence of key-value pairs into a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_insert_seq(stdskl *l, stdit *it, const stdit *b, const stdit *e, stdbool hint)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && (stdit_eq(b, e) || STDTRUE) &&
		  (!hint || (it != NULL && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl))) &&
		  (stdit_key_size(b) == l->ksize || (stdit_key_size(b) == 0 && stdit_val_size(b) == l->ksize)) &&
		  (stdit_val_size(b) == l->vsize || l->vsize == 0));

  return stdskl_low_insert(l, it, b, e, (stdsize) -1, hint, STDFALSE, STDTRUE);
}

/************************************************************************************************
 * stdskl_insert_seq_n: Insert a sequence of key-value pairs into a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_insert_seq_n(stdskl *l, stdit *it, const stdit *b, stdsize num_insert, stdbool hint)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && 
		  (!hint || (it != NULL && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl))) &&
		  (stdit_key_size(b) == l->ksize || (stdit_key_size(b) == 0 && stdit_val_size(b) == l->ksize)) &&
		  (stdit_val_size(b) == l->vsize || l->vsize == 0));

  return stdskl_low_insert(l, it, b, NULL, num_insert, hint, STDFALSE, STDTRUE);
}

/************************************************************************************************
 * stdskl_insert_rep: Repeatedly insert a key-value pair into a list.
 ***********************************************************************************************/

STDINLINE stdcode stdskl_insert_rep(stdskl *l, stdit *it, const void *key, const void *val, stdsize num_times, stdbool hint)
{
  stdit b;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && 
		  (!hint || (it != NULL && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl))));

  return stdskl_low_insert(l, it, stdit_pptr(&b, key, val, l->ksize, l->vsize), NULL, num_times, hint, STDFALSE, STDFALSE);
}

/************************************************************************************************
 * stdskl_erase: Erase a key-value pair from a list.
 ***********************************************************************************************/

STDINLINE void stdskl_erase(stdskl *l, stdit *it)
{
  stdskl_erase_n(l, it, 1);
}

/************************************************************************************************
 * stdskl_erase_n: Erase multiple key-value pairs from a list.
 ***********************************************************************************************/

STDINLINE void stdskl_erase_n(stdskl *l, stdit *it, stdsize num_erase)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && STDIT_SKL_IS_LEGAL(it) && STDSKL_IT_IS_LEGAL(l, &it->impl.skl));

  stdskl_low_erase(l, it, NULL, num_erase);
}

/************************************************************************************************
 * stdskl_erase_seq: Erase a sequence from a list.
 ***********************************************************************************************/

STDINLINE stdsize stdskl_erase_seq(stdskl *l, stdit *b, stdit *e)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l) && STDIT_SKL_IS_LEGAL(b) && STDSKL_IT_IS_LEGAL(l, &b->impl.skl) && (stdit_eq(b, e) || STDTRUE));

  return stdskl_low_erase(l, b, e, (stdsize) -1);
}

/************************************************************************************************
 * stdskl_erase_key: Erase all entries of a key from a list.
 ***********************************************************************************************/

STDINLINE stdsize stdskl_erase_key(stdskl *l, const void *key)
{
  stdsize ret = 0;
  stdit   it;

  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  stdskl_lowerb(l, &it, key);

  for (; it.impl.skl.node != l->end_node && stdskl_low_key_cmp(l, key, STDSKL_NKEY(it.impl.skl.node)) == 0; ++ret) {
    stdskl_erase(l, &it);  /* advances 'it' */
  }

  return ret;
}

/************************************************************************************************
 * stdskl_dseed: Set a skiplist's random number generator with a deterministic value.
 ***********************************************************************************************/

STDINLINE void stdskl_dseed(stdskl *l, const void *seed, stdsize sizeof_seed)
{
  STDSAFETY_CHECK(STDSKL_IS_LEGAL(l));

  stdrand32_dseed(l->seed, stdhcode_sfh(seed, sizeof_seed));
  l->bits_left = 0;
}

/************************************************************************************************
 * stdskl_it_key: Get a key from an iterator.
 ***********************************************************************************************/

STDINLINE const void *stdskl_it_key(const stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  return STDSKL_NKEY(it->impl.skl.node);
}

/************************************************************************************************
 * stdskl_it_key_size: Get the size in bytes of the keys to which 'it' refers.
 ***********************************************************************************************/

STDINLINE stdsize stdskl_it_key_size(const stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  return it->impl.skl.ksize;
}

/************************************************************************************************
 * stdskl_it_val: Get a value from an iterator.
 ***********************************************************************************************/

STDINLINE void *stdskl_it_val(const stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  return (void*) (it->type_id == STDSKL_IT_ID ? STDSKL_NVAL(it->impl.skl.node) : STDSKL_NKEY(it->impl.skl.node));
}

/************************************************************************************************
 * stdskl_it_val_size: Get the size in bytes of the values to which 'it' refers.
 ***********************************************************************************************/

STDINLINE stdsize stdskl_it_val_size(const stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  return (it->type_id == STDSKL_IT_ID ? it->impl.skl.vsize : it->impl.skl.ksize);
}

/************************************************************************************************
 * stdskl_it_eq: Compare two iterators for equality (refer to the same element).
 ***********************************************************************************************/

STDINLINE stdbool stdskl_it_eq(const stdit *it1, const stdit *it2)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it1) && STDIT_SKL_IS_LEGAL(it2) &&
		  it1->impl.skl.ksize == it2->impl.skl.ksize &&
		  it1->impl.skl.vsize == it2->impl.skl.vsize);

  return it1->impl.skl.node == it2->impl.skl.node;
}

/************************************************************************************************
 * stdskl_it_next: Advance 'it' towards end by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_it_next(stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  it->impl.skl.node = it->impl.skl.node->nexts[0];

  return it;
}

/************************************************************************************************
 * stdskl_it_advance: Advance 'it' towards end by 'num_advance' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_it_advance(stdit *it, stdsize num_advance)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  while (num_advance-- != 0) {
    it->impl.skl.node = it->impl.skl.node->nexts[0];
  }

  return it;
}

/************************************************************************************************
 * stdskl_it_prev: Advance 'it' towards begin by one position.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_it_prev(stdit *it)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  it->impl.skl.node = it->impl.skl.node->prevs[0];

  return it;
}

/************************************************************************************************
 * stdskl_it_retreat: Advance 'it' towards end by 'num_retreat' positions.
 ***********************************************************************************************/

STDINLINE stdit *stdskl_it_retreat(stdit *it, stdsize num_retreat)
{
  STDSAFETY_CHECK(STDIT_SKL_IS_LEGAL(it));

  while (num_retreat-- != 0) {
    it->impl.skl.node = it->impl.skl.node->prevs[0];
  }

  return it;
}

#ifdef __cplusplus
}
#endif
