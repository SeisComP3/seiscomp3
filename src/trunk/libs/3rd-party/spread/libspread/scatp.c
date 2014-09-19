/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */



#include <errno.h>
#include <assert.h>
#include <string.h>
#include <scatp.h>

long scat_capacity(const scatter *scat) 
{
  const scat_element *curr = scat->elements, *end = scat->elements + scat->num_elements;
  long ret = 0;

  if (scat->num_elements < 0 || scat->num_elements > MAX_CLIENT_SCATTER_ELEMENTS)
    return ILLEGAL_MESSAGE;

  for (; curr != end; ++curr) {
    if (curr->len < 0)
      return ILLEGAL_MESSAGE;
    ret += curr->len;
  }
  return ret;
}

int scatp_begin(scatp *pos, const scatter *scat) 
{
  int i;

  if (scat->num_elements < 0 ||  scat->num_elements > MAX_CLIENT_SCATTER_ELEMENTS)
    return ILLEGAL_MESSAGE;

  for (i = 0; i < scat->num_elements && scat->elements[i].len == 0; ++i);

  if (i != scat->num_elements && scat->elements[i].len < 0)
    return ILLEGAL_MESSAGE;

  pos->scat     = (scatter*) scat;
  pos->elem_ind = i;
  pos->buff_ind = 0;
  return 0;
}

int scatp_end(scatp *pos, const scatter *scat) 
{
  if (scat->num_elements < 0 || scat->num_elements > MAX_CLIENT_SCATTER_ELEMENTS)
    return ILLEGAL_MESSAGE;

  pos->scat     = (scatter*) scat;
  pos->elem_ind = scat->num_elements;
  pos->buff_ind = 0;
  return 0;
}

int scatp_set(scatp *pos, const scatter *scat, long offset, int whence) 
{
  int err;

  if (whence == SEEK_CUR) {
    if ((err = scatp_begin(pos, scat)))
      return err;
  } else
    pos->scat = (scatter*) scat;

  return scatp_seek(pos, offset, whence);
}

int scatp_is_legal(const scatp *pos) 
{
  const scatter *scat = pos->scat;

  return (scat->num_elements >= 0 && scat->num_elements <= MAX_CLIENT_SCATTER_ELEMENTS &&
	  (scatp_is_end(pos) ||
	   (pos->elem_ind >= 0 && pos->elem_ind < scat->num_elements &&
	    pos->buff_ind >= 0 && pos->buff_ind < scat->elements[pos->elem_ind].len)));
}

int scatp_is_not_legal(const scatp *pos) 
{
  const scatter *scat = pos->scat;

  return (scat->num_elements < 0 || scat->num_elements > MAX_CLIENT_SCATTER_ELEMENTS ||
	  (!scatp_is_end(pos) && 
	   (pos->elem_ind < 0 || pos->elem_ind >= scat->num_elements || 
	    pos->buff_ind < 0 || pos->buff_ind >= scat->elements[pos->elem_ind].len)));
}

int scatp_is_end(const scatp *pos) 
{
  if (pos->scat->num_elements < 0 || pos->scat->num_elements > MAX_CLIENT_SCATTER_ELEMENTS)
    return ILLEGAL_MESSAGE;

  return pos->elem_ind == pos->scat->num_elements && pos->buff_ind == 0;
}

int scatp_equals(const scatp *pos1, const scatp *pos2) 
{
  if (scatp_is_not_legal(pos1) || scatp_is_not_legal(pos2))
    return ILLEGAL_MESSAGE;

  if (pos1->scat != pos2->scat)
    return ILLEGAL_SERVICE;

  return pos1->elem_ind == pos2->elem_ind && pos1->buff_ind == pos2->buff_ind;
}

long scatp_comp(const scatp *pos1, const scatp *pos2) 
{
  const scat_element *curr, *end;
  const scatter *scat = pos1->scat;
  long ret;

  if (scatp_is_not_legal(pos1) || scatp_is_not_legal(pos2))
    return ILLEGAL_MESSAGE;

  if (pos1->scat != pos2->scat)
    return ILLEGAL_SERVICE;

  if (pos1->elem_ind == pos2->elem_ind)
    return pos1->buff_ind - pos2->buff_ind;

  if (pos1->elem_ind < pos2->elem_ind) {
    ret  = pos1->buff_ind - scat->elements[pos1->elem_ind].len;
    curr = scat->elements + pos1->elem_ind;
    end  = scat->elements + pos2->elem_ind;
    while (++curr != end) {
      if (curr->len < 0)
	return ILLEGAL_MESSAGE;
      ret -= curr->len;
    }
  } else {
    ret  = scat->elements[pos2->elem_ind].len - pos2->buff_ind;
    curr = scat->elements + pos2->elem_ind;
    end  = scat->elements + pos1->elem_ind;
    while (++curr != end) {
      if (curr->len < 0)
	return ILLEGAL_MESSAGE;
      ret += curr->len;
    }
  }
  return ret;
}

/* This fcn moves a scat_pos forward num_bytes bytes in a scatter. On
   success, this fcn returns num_bytes and pos is modified
   appropriately. Otherwise, if the jump was so big that it would have
   jumped off the end of the scatter, the number of bytes that _could_
   have been successfully jumped is returned, and pos is unchanged. 
*/

long scatp_jforward(scatp *pos, long num_bytes) 
{ 
  long elem_ind, skip_bytes, tmp; 
  const scatter *scat = pos->scat;

  if (scatp_is_not_legal(pos))
    return ILLEGAL_MESSAGE;

  if (num_bytes < 0)
    return ILLEGAL_SERVICE;

  if (scatp_is_end(pos)) /* can't move forward from end */
    return 0;

  /* jump stays within current element buffer */
  if ((tmp = scat->elements[pos->elem_ind].len - pos->buff_ind) > num_bytes) {
    pos->buff_ind += num_bytes;
    return num_bytes;
  }
  /* incorporate what was left in current element buffer into jump */
  elem_ind   = pos->elem_ind + 1;
  skip_bytes = num_bytes - tmp;   /* how many bytes left to skip */
  
  for (; elem_ind < scat->num_elements; ++elem_ind) {
    if (scat->elements[elem_ind].len < 0)
      return ILLEGAL_MESSAGE;

    /* use < 0 because it jumps over any zero length buffers */
    if ((skip_bytes -= scat->elements[elem_ind].len) < 0) {
      skip_bytes += scat->elements[elem_ind].len; /* restore to positive */
      break;
    }
  }
  /* jump forward jumped past end of scatter */
  if (elem_ind == scat->num_elements && skip_bytes != 0) 
    return num_bytes - skip_bytes;

  pos->elem_ind = elem_ind;
  pos->buff_ind = skip_bytes;
  return num_bytes;
}

/* same as scat_jforward, except it moves the position backwards in the scatter */

long scatp_jbackward(scatp *pos, long num_bytes) 
{
  long elem_ind, e_ind, skip_bytes;
  const scatter *scat = pos->scat;

  if (scatp_is_not_legal(pos))
    return ILLEGAL_MESSAGE;

  if (num_bytes < 0)
    return ILLEGAL_SERVICE;

  /* jump stays within current element buffer */
  if (pos->buff_ind >= num_bytes) {
    pos->buff_ind -= num_bytes;
    return num_bytes;
  }
  /* incorporate what was left in current element buffer into jump */
  elem_ind   = pos->elem_ind;
  skip_bytes = num_bytes - pos->buff_ind;
  
  for (e_ind = pos->elem_ind - 1; e_ind >= 0; --e_ind) {
    if (scat->elements[e_ind].len < 0)
      return ILLEGAL_MESSAGE;

    /* again we want to ignore any zero length buffers */
    if (scat->elements[e_ind].len > 0) {
      elem_ind = e_ind; /* elem_ind must reference a non-empty element buffer */
      if ((skip_bytes -= scat->elements[e_ind].len) <= 0)
	break;
    }
  }
  if (e_ind < 0)
    return num_bytes - skip_bytes;

  pos->elem_ind = elem_ind;
  pos->buff_ind = -skip_bytes; /* restore to positive */
  return num_bytes;
}

int scatp_seek(scatp *pos, long offset, int whence) 
{
  scatp set;
  long err;

  switch (whence) {
  case SEEK_CUR:
    set = *pos;
    break;
  case SEEK_SET:
    if ((err = scatp_begin(&set, pos->scat)))
      return (int) err;
    break;
  case SEEK_END:
    if ((err = scatp_end(&set, pos->scat)))
      return (int) err;
    break;
  default:
    return EINVAL;
  }
  if (offset >= 0) {
    if ((err = scatp_jforward(&set, offset)) != offset)
      return err < 0 ? (int) err : -1;
  } else {
    offset = -offset;
    if ((err = scatp_jbackward(&set, offset)) != offset)
      return err < 0 ? (int) err : -1;
  }
  *pos = set;
  return 0;
}

long scatp_cpy0(const scatp *dst, const scatp *src, long num_bytes) 
{
  return scatp_adv_cpy0((scatp*) dst, (scatp*) src, num_bytes, 0, 0);
}

long scatp_cpy1(char *dst, const scatp *src, long num_bytes) 
{
  scatter dscat;
  scatp dscatp;
  long err;

  dscat.num_elements    = 1;
  dscat.elements[0].len = num_bytes;
  dscat.elements[0].buf = dst;

  err = scatp_begin(&dscatp, &dscat);
  assert(err == 0);

  return scatp_cpy0(&dscatp, src, num_bytes);
}

long scatp_cpy2(const scatp *dst, char *src, long num_bytes) 
{
  scatter sscat;
  scatp sscatp;
  long err;

  sscat.num_elements    = 1;
  sscat.elements[0].len = num_bytes;
  sscat.elements[0].buf = src;

  err = scatp_begin(&sscatp, &sscat);
  assert(err == 0);

  return scatp_cpy0(dst, &sscatp, num_bytes);
}

long scatp_adv_cpy0(scatp *dst, scatp *src, long num_bytes, int adv_dst, int adv_src) 
{
  scatter *dscat = dst->scat, *sscat = src->scat;
  long dst_elem, src_elem, bytes_left, copy_size, dst_left, src_left;
  char *dst_curr, *dst_end, *src_curr, *src_end;

  if (scatp_is_not_legal(dst) || scatp_is_not_legal(src)) {
    printf("illegal scatp! dst: %d src: %d\n", scatp_is_not_legal(dst), scatp_is_not_legal(src));
    return ILLEGAL_MESSAGE;
  }
  if (num_bytes < 0)
    return ILLEGAL_SERVICE;

  if (scatp_is_end(dst) || scatp_is_end(src))
    return 0;

  dst_elem = dst->elem_ind;
  dst_curr = dscat->elements[dst->elem_ind].buf + dst->buff_ind;
  dst_end  = dscat->elements[dst->elem_ind].buf + dscat->elements[dst->elem_ind].len;

  src_elem = src->elem_ind;
  src_curr = sscat->elements[src->elem_ind].buf + src->buff_ind;
  src_end  = sscat->elements[src->elem_ind].buf + sscat->elements[src->elem_ind].len;

  bytes_left = num_bytes;

  while (dst_elem < dscat->num_elements && src_elem < sscat->num_elements && bytes_left) {
    dst_left  = dst_end - dst_curr;
    src_left  = src_end - src_curr;
    copy_size = (dst_left < src_left) ? dst_left : src_left;
    copy_size = (copy_size < bytes_left) ? copy_size : bytes_left;

    if (copy_size < 0) {       /* ensure no empty or negative copies */
      printf("scatp_cpy: buffer size negative!\n");
      return ILLEGAL_MESSAGE;
    }

    memcpy(dst_curr, src_curr, copy_size);
    bytes_left -= copy_size;

    if (copy_size != dst_left)
      dst_curr += copy_size;
    else {
      while (++dst_elem < dscat->num_elements && dscat->elements[dst_elem].len == 0);
      if (dst_elem < dscat->num_elements) {
	dst_curr = dscat->elements[dst_elem].buf;
	dst_end  = dscat->elements[dst_elem].buf + dscat->elements[dst_elem].len;
      }
    }
    if (copy_size != src_left)
      src_curr += copy_size;
    else {
      while (++src_elem < sscat->num_elements && sscat->elements[src_elem].len == 0);
      if (src_elem < sscat->num_elements) {
	src_curr = sscat->elements[src_elem].buf;
	src_end  = sscat->elements[src_elem].buf + sscat->elements[src_elem].len;
      }
    }
  }
  if (bytes_left != 0) /* couldn't do the entire copy */
    return num_bytes - bytes_left;

  /* success! now, update the scatp's if requested to */
  if (adv_dst) {
    dst->elem_ind = dst_elem;
    if (dst_elem != dscat->num_elements)
      dst->buff_ind = dst_curr - dscat->elements[dst_elem].buf;
    else
      dst->buff_ind = 0;
  }
  if (adv_src) {
    src->elem_ind = src_elem;
    if (src_elem != sscat->num_elements)
      src->buff_ind = src_curr - sscat->elements[src_elem].buf;
    else
      src->buff_ind = 0;
  }
  return num_bytes;
}

long scatp_adv_cpy1(char **dst, scatp *src, long num_bytes, int adv_dst, int adv_src) 
{
  scatter dscat;
  scatp dscatp;
  long ret;

  dscat.num_elements    = 1;
  dscat.elements[0].len = num_bytes;
  dscat.elements[0].buf = *dst;

  ret = scatp_begin(&dscatp, &dscat);
  assert(ret == 0);

  if ((ret = scatp_adv_cpy0(&dscatp, src, num_bytes, 0, adv_src)) == num_bytes && adv_dst)
    *dst += num_bytes;

  return ret;
}

long scatp_adv_cpy2(scatp *dst, char **src, long num_bytes, int adv_dst, int adv_src) 
{
  scatter sscat;
  scatp sscatp;
  long ret;

  sscat.num_elements    = 1;
  sscat.elements[0].len = num_bytes;
  sscat.elements[0].buf = *src;

  ret = scatp_begin(&sscatp, &sscat);
  assert(ret == 0);

  if ((ret = scatp_adv_cpy0(dst, &sscatp, num_bytes, adv_dst, 0)) == num_bytes && adv_src)
    *src += num_bytes;

  return ret;
}

