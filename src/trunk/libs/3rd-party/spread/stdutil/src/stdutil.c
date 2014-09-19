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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <stdutil/stderror.h>
#include <stdutil/stdtime.h>
#include <stdutil/stdutil.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************************
 * stdstrcpy: Like C's strcpy, but returns strlen(dst) instead.
 ***********************************************************************************************/

STDINLINE stdsize stdstrcpy(char *dst, const char *src)
{
  stdsize ret = strlen(src);

  memcpy(dst, src, ret + 1);

  return ret;
}

/************************************************************************************************
 * stdstrcpy_n: Like stdstrcpy, but only copies the first min('n',
 * strlen('src')) bytes of 'src' and then adds a NUL termination
 * (i.e. - dst must have room for n + 1 bytes).
 ***********************************************************************************************/

STDINLINE stdsize stdstrcpy_n(char *dst, const char *src, stdsize n)
{
  const char * bsrc = src;
  const char * esrc = src + n;
  stdsize      ret;

  for (; src != esrc && *src != 0; ++src);

  ret = (stdsize) (src - bsrc);
  memcpy(dst, bsrc, ret);
  dst[ret] = 0;

  return ret;
}

/************************************************************************************************
 * stdstrdup: Return a malloc'ed copy of 'dupme.'  Returns NULL on
 * allocation failure.  If len_ptr is not NULL, then strlen(dupme)
 * will always be placed there regardless of success or failure.
 ***********************************************************************************************/

STDINLINE char *stdstrdup(const char *dupme, stdsize *len_ptr)
{
  stdsize len = strlen(dupme);
  char *  ret = (char*) malloc(len + 1);
  
  if (len_ptr != NULL) {
    *len_ptr = len;
  }

  return (char*) (ret != NULL ? memcpy(ret, dupme, len + 1) : NULL);
}

/************************************************************************************************
 * stdstrdup_n: Like stdstrdup, but only allocates and copies up
 * through the first min('n', strlen('src')) bytes of 'src' and then
 * adds a NUL termination.
 ***********************************************************************************************/

STDINLINE char *stdstrdup_n(const char *src, stdsize *len_ptr, size_t n)
{
  const char * bsrc = src;
  const char * esrc = src + n;
  stdsize      len;
  char *       ret;

  for (; src != esrc && *src != 0; ++src);     /* see if strlen(src) < n */

  len = (stdsize) (src - bsrc);                /* min(n, strlen(str)) */
  ret = (char*) malloc(len + 1);

  if (len_ptr != NULL) {
    *len_ptr = len;
  }

  if (ret != NULL) {                           /* copy on successful allocation */
    memcpy(ret, src, len);
    ret[len] = 0;
  }

  return ret;
}

/************************************************************************************************
 * stdmemdup: Allocates and returns a new block of memory 'n' bytes
 * long and copies the first 'n' bytes of 'src' to the new block.  If
 * n is zero this fcn will return NULL.
 ***********************************************************************************************/

STDINLINE void *stdmemdup(const void *src, size_t n)
{
  void * ret = (n != 0 ? malloc(n) : NULL);

  if (ret != NULL) {
    memcpy(ret, src, n);
  }

  return ret;
}

/************************************************************************************************
 * stdhcode_oaat: Bob Jenkin's One-at-a-Time hash code function.
 * Computes a 32 bit integer based on all 'buf_len' bytes of 'buf.'
 * Every input bit can affect every bit of the result with about 50%
 * probability per output bit -- has no funnels.
 ***********************************************************************************************/

STDINLINE stduint32 stdhcode_oaat(const void * buf, stdsize buf_len)
{
  const char * kit  = (const char*) buf;
  const char * kend = (const char*) buf + buf_len;
  stduint32    ret  = (stduint32) buf_len;

  for (; kit != kend; ++kit) {
    ret += *kit;
    ret += (ret << 10);
    ret ^= (ret >> 6);
  }

  ret += (ret << 3);
  ret ^= (ret >> 11);
  ret += (ret << 15);

  return ret;
}

/************************************************************************************************
 * stdhcode_oaat_start: Begin computing a One-at-a-Time hash that will
 * span several buffers.  Preferably, but not required, tot_len would
 * equal the total length of the buffers to be hcoded.  If the total
 * length is unknown (or you simply don't want to compute it), then
 * consistently use the same arbitrary constant (e.g. - 0x1) when
 * computing the hcode for those objects.
 ***********************************************************************************************/

STDINLINE void stdhcode_oaat_start(stduint32 *hsh, stdsize tot_len)
{
  *hsh = ((stduint32) tot_len);
}

/************************************************************************************************
 * stdhcode_oaat_churn: Mix the 'buf_len' bytes of 'buf' into 'hsh'
 * using One-at-a-Time's mixer.
 ***********************************************************************************************/

STDINLINE void stdhcode_oaat_churn(stduint32 *hsh, const void * buf, stdsize buf_len)
{
  const char * kit  = (const char *) buf;
  const char * kend = (const char *) buf + buf_len;
  stduint32    ret  = *hsh;

  for (; kit != kend; ++kit) {
    ret += *kit;
    ret += (ret << 10);
    ret ^= (ret >> 6);    
  }

  *hsh = ret;
}

/************************************************************************************************
 * stdhcode_oaat_stop: Compute the final result for a multi-buffer
 * One-at-a-Time hash code.
 ***********************************************************************************************/

STDINLINE void stdhcode_oaat_stop(stduint32 * hsh)
{
  stduint32 ret = *hsh;

  ret += (ret << 3);
  ret ^= (ret >> 11);
  ret += (ret << 15);

  *hsh = ret;
}

/************************************************************************************************
 * stdhcode_sfh: Paul Hsieh's Super Fast Hash hash code function.
 * Computes a 32 bit integer based on all 'buf_len' bytes of 'buf.'
 * Every input bit can affect every bit of the result with about 50%
 * probability per output bit -- has no funnels.  Paul Hsieh claims
 * superior speed and distribution compared to One-at-a-Time.
 ***********************************************************************************************/

#if ((defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || \
     defined (__BORLANDC__) || defined (__TURBOC__))
#  define stdhcode_sfh_get16bits(d) ( *((const stduint16*)(d)) )
#else
#  define stdhcode_sfh_get16bits(d) ( ((stduint32) *((const stduint8 *)(d) + 1) << 8) | (stduint32) *(const stduint8 *)(d) )
#endif

STDINLINE stduint32 stdhcode_sfh(const void * buf, stdsize buf_len) 
{
  const stduint8 * kit  = (const stduint8*) buf;
  const stduint8 * kend = (const stduint8*) buf + (buf_len & ~0x3);  /* leave remainder modulo 4 */
  stduint32        ret  = (stduint32) buf_len;
  stduint32        tmp;

  /* main mixing loop */

  while (kit != kend) {
    ret += stdhcode_sfh_get16bits(kit);
    tmp  = (stdhcode_sfh_get16bits(kit + 2) << 11) ^ ret;
    ret  = (ret << 16) ^ tmp;
    kit += 4;
    ret += ret >> 11;
  }

  /* handle remainder modulo 4 not handled by loop */

  switch (buf_len & 0x3) {
  case 3: 
    ret += stdhcode_sfh_get16bits(kit);
    ret ^= ret << 16;
    ret ^= kit[2] << 18;
    ret += ret >> 11;
    break;
    
  case 2: 
    ret += stdhcode_sfh_get16bits(kit);
    ret ^= ret << 11;
    ret += ret >> 17;
    break;
		
  case 1: 
    ret += *kit;
    ret ^= ret << 10;
    ret += ret >> 1;
    break;
  }

  /* force avalanche of final 127 bits */

  ret ^= ret << 3;
  ret += ret >> 5;
  ret ^= ret << 4;
  ret += ret >> 17;
  ret ^= ret << 25;
  ret += ret >> 6;
  
  return ret;
}

/************************************************************************************************
 * stdhcode_sfh_start: Begin computing a SuperFastHash hash that will
 * span several buffers.  Preferably, but not required, tot_len would
 * equal the total length of the buffers to be hash coded.  If the total
 * length is unknown (or you simply don't want to compute it), then
 * consistently use the same arbitrary constant (e.g. - 0x1) when
 * computing the hcode for those objects.
 *
 * NOTE: The incremental versions of sfh depend on how you partition
 * the total data set you churn through.  If you pick different
 * partitions of the same data you will get different final results.
 * This is not true for the One-at-a-Time Hash which does not depend
 * on how you partition the data.
 ***********************************************************************************************/

STDINLINE void stdhcode_sfh_start(stduint32 *hsh, stdsize tot_len)
{
  *hsh = (stduint32) tot_len;
}

/************************************************************************************************
 * stdhcode_sfh_churn: Mix the 'buf_len' bytes of 'buf' into 'hsh'
 * using SuperFastHash's mixer.
 ***********************************************************************************************/

STDINLINE void stdhcode_sfh_churn(stduint32 *hsh, const void * buf, stdsize buf_len)
{
  const stduint8 * kit  = (const stduint8*) buf;
  const stduint8 * kend = (const stduint8*) buf + (buf_len & ~0x3);  /* leave remainder modulo 4 */
  stduint32        ret  = *hsh;
  stduint32        tmp;

  /* main mixing loop */

  while (kit != kend) {
    ret += stdhcode_sfh_get16bits(kit);
    tmp  = (stdhcode_sfh_get16bits(kit + 2) << 11) ^ ret;
    ret  = (ret << 16) ^ tmp;
    kit += 4;
    ret += ret >> 11;
  }

  /* handle modulo 4 remainder not handled by loop */

  switch (buf_len & 3) {
  case 3: 
    ret += stdhcode_sfh_get16bits(kit);
    ret ^= ret << 16;
    ret ^= kit[2] << 18;
    ret += ret >> 11;
    break;
    
  case 2: 
    ret += stdhcode_sfh_get16bits(kit);
    ret ^= ret << 11;
    ret += ret >> 17;
    break;
		
  case 1: 
    ret += *kit;
    ret ^= ret << 10;
    ret += ret >> 1;
    break;
  }

  *hsh = ret;
}

/************************************************************************************************
 * stdhcode_sfh_stop: Compute the final result of a multi-buffer
 * SuperFastHash hash code.
 ***********************************************************************************************/

STDINLINE void stdhcode_sfh_stop(stduint32 *hsh)
{
  stduint32 ret = *hsh;

  /* force avalanche of final 127 bits */

  ret ^= ret << 3;
  ret += ret >> 5;
  ret ^= ret << 4;
  ret += ret >> 17;
  ret ^= ret << 25;
  ret += ret >> 6;
  
  *hsh = ret;
}

/* Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#define RAND48_SEED_0 0x330e
#define RAND48_SEED_1 0xabcd
#define RAND48_SEED_2 0x1234
#define RAND48_MULT_0 0xe66d
#define RAND48_MULT_1 0xdeec
#define RAND48_MULT_2 0x0005
#define RAND48_ADD    0x000b

/************************************************************************************************
 * stdrand32: Returns a pseudo-random 32b integer using a linear
 * congruential algorithm working on integers 48 bits in size (i.e. -
 * 'x').  The function mixes the contents of 'x' while calculating the
 * integer to return.  I believe it has a period of 2^47.
 ***********************************************************************************************/

STDINLINE stduint32 stdrand32(stduint16 x[3]) 
{
  stduint16 temp[2];
  stduint32 acc;

  acc      = (stduint32) RAND48_MULT_0 * x[0] + RAND48_ADD;
  temp[0]  = (stduint16) acc;

  acc    >>= 16;
  acc     += (stduint32) RAND48_MULT_0 * x[1] + (stduint32) RAND48_MULT_1 * x[0];
  temp[1]  = (stduint16) acc;

  acc    >>= 16;
  acc     += ((stduint32) RAND48_MULT_0 * x[2] + (stduint32) RAND48_MULT_1 * x[1] + 
	      (stduint32) RAND48_MULT_2 * x[0]);

  x[0]     = temp[0];
  x[1]     = temp[1];
  x[2]     = (stduint16) acc;

  return ((stduint32) x[2] << 16) | x[1];
}

/************************************************************************************************
 * stdrand32_seed: Seeds the contents of 'x' based on 'seed' and the
 * current system time.
 ***********************************************************************************************/

STDINLINE void stdrand32_seed(stduint16 x[3], stduint32 seed) 
{
  stdtime64 t;

  stdtime64_now(&t);  /* get time since epoch in nanoseconds */
  t >>= 16;           /* truncate to a precision which most clocks can do */

  stdrand32_dseed(x, ((stduint32) t ^ seed) * (seed | 0x1));
}

/************************************************************************************************
 * stdrand32_dseed: Seeds the contents of 'x' based solely and
 * deterministically (i.e. - repeatable) on 'seed.'
 ***********************************************************************************************/

STDINLINE void stdrand32_dseed(stduint16 x[3], stduint32 seed) 
{
  x[2] = (stduint16) ((seed >> 16) ^ RAND48_SEED_0);
  x[1] = (stduint16) ((seed & 0xFFFF) ^ RAND48_SEED_1);
  x[0] = x[1] ^ x[2] ^ RAND48_SEED_2;
}

/************************************************************************************************
 * stdrand64: Returns a pseudo-random 64b integers using a linear
 * congruential algorithm working on integers 48 bits in size (i.e. -
 * 'x').  The function mixes the contents of 'x' while calculating the
 * integer to return.  I believe it has a period of 2^47.
 ***********************************************************************************************/

STDINLINE stduint64 stdrand64(stduint32 x[3]) 
{
  return ((stduint64) stdrand32((stduint16*) x) << 32) | stdrand32((stduint16*) x + 3);
}

/************************************************************************************************
 * stdrand64_seed: Seeds the contents of 'x' based on 'seed' and the
 * current system time.
 ***********************************************************************************************/

STDINLINE void stdrand64_seed(stduint32 x[3], stduint64 seed) 
{
  stdtime64 t;
  
  stdtime64_now(&t);  /* get time in nanoseconds since epoch */

  stdrand64_dseed(x, ((stduint64) t ^ seed) * (seed | 0x1));
}

/************************************************************************************************
 * stdrand64_dseed: Seeds the contents of 'x' based solely and
 * deterministically (i.e. - repeatable) on 'seed.'
 ***********************************************************************************************/

STDINLINE void stdrand64_dseed(stduint32 x[3], stduint64 seed) 
{
  stdrand32_dseed((stduint16*) x, (stduint32) (seed >> 32));
  stdrand32_dseed((stduint16*) x + 3, (stduint32) seed);
}

/************************************************************************************************
 * For the following endian flipping code, I have defined in
 * stdutil/private/stdarch.h a mapping from network byte order to host
 * byte order and the reverse mapping for 16, 32 and 64 bit integers.
 * This mapping is originally discovered through the configure script
 * generated by autoconf from configure.in or is pre-defined for some
 * fixed architecture (e.g. - 32 bit little endian).
 *
 * For example, in a 64b integer on little endian architectures, the
 * most significant byte is the highest byte in memory (byte 7), while
 * on a big endian architecture it is the lowest byte in memory (byte
 * 0).  We represent this information in stdutil/stdutil_p.h as, on
 * the respective architectures,
 *
 * (#define STDENDIAN64_NET0_FROM_HOST 7, #define STDENDIAN64_HOST7_FROM_NET 0) 
 *
 * and 
 *   
 * (#define STDENDIAN64_NET0_FROM_HOST 0, #define STDENDIAN64_HOST0_FROM_NET 0)
 * 
 * If you need to correct these numbers by hand (e.g. - cross
 * compiling for a different endian architecture so configure won't
 * work correctly), then just make sure you get both the forward and
 * backward mappings correct.
 ***********************************************************************************************/

/************************************************************************************************
 * stdhton16: Rearranges the two bytes at which 'io' points from host
 * to network byte ordering.
 ***********************************************************************************************/

STDINLINE void stdhton16(void *io)
{
#if (STDENDIAN16_SWAP == 1)
  stduint8 * ptr = (stduint8*) io;
  stduint8   t;

  STDSWAP(ptr[0], ptr[1], t);
#endif
}

/************************************************************************************************
 * stdhton32: Rearranges the four bytes at which 'io' points from host
 * to network byte ordering.
 ***********************************************************************************************/

STDINLINE void stdhton32(void *io)
{
#if (STDENDIAN32_NET0_FROM_HOST != 0 || STDENDIAN32_NET1_FROM_HOST != 1 || STDENDIAN32_NET2_FROM_HOST != 2 || STDENDIAN32_NET3_FROM_HOST != 3)
  stduint8 * ptr = (stduint8*) io;
  stduint8   buf[4];


#  if (STDENDIAN32_NET0_FROM_HOST != 0)
  buf[0] = ptr[0];
  ptr[0] = ptr[STDENDIAN32_NET0_FROM_HOST];
#  endif
  

#  if (STDENDIAN32_NET1_FROM_HOST != 1)
#    if (STDENDIAN32_NET0_FROM_HOST != 1)
  buf[1] = ptr[1];                      
#    endif

#    if (STDENDIAN32_NET1_FROM_HOST > 1)
  ptr[1] = ptr[STDENDIAN32_NET1_FROM_HOST];
#    else
  ptr[1] = buf[STDENDIAN32_NET1_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN32_NET2_FROM_HOST != 2)
#    if (STDENDIAN32_NET0_FROM_HOST != 2 && STDENDIAN32_NET1_FROM_HOST != 2)
  buf[2] = ptr[2];                           /* ptr[2] was not previously consumed, so we need to save it in buf[2] */
#    endif

#    if (STDENDIAN32_NET2_FROM_HOST > 2)
  ptr[2] = ptr[STDENDIAN32_NET2_FROM_HOST];  /* STDENDIAN32_NET2_FROM_HOST > 2 -> we haven't yet consumed/saved ptr[STDENDIAN32_NET2_FROM_HOST] */
#    else
  ptr[2] = buf[STDENDIAN32_NET2_FROM_HOST];  /* STDENDIAN32_NET2_FROM_HOST < 2 -> we have consumed+saved ptr[STDENDIAN32_NET2_FROM_HOST] in buf */
#    endif
#  endif
  

#  if (STDENDIAN32_NET3_FROM_HOST != 3)
  ptr[3] = buf[STDENDIAN32_NET3_FROM_HOST];  /* we have previously saved ptr[STDENDIAN32_NET3_FROM_HOST] in buf */
#  endif
  

#endif
}

/************************************************************************************************
 * stdhton64: Rearranges the eight bytes at which 'io' points from
 * host to network byte order.
 ***********************************************************************************************/

STDINLINE void stdhton64(void *io)
{
#if (STDENDIAN64_NET0_FROM_HOST != 0 || STDENDIAN64_NET1_FROM_HOST != 1 || STDENDIAN64_NET2_FROM_HOST != 2 || STDENDIAN64_NET3_FROM_HOST != 3 || \
     STDENDIAN64_NET4_FROM_HOST != 4 || STDENDIAN64_NET5_FROM_HOST != 5 || STDENDIAN64_NET6_FROM_HOST != 6 || STDENDIAN64_NET7_FROM_HOST != 7)
  stduint8 * ptr = (stduint8*) io;
  stduint8   buf[8];


#  if (STDENDIAN64_NET0_FROM_HOST != 0)
  buf[0] = ptr[0];
  ptr[0] = ptr[STDENDIAN64_NET0_FROM_HOST];
#  endif
  

#  if (STDENDIAN64_NET1_FROM_HOST != 1)
#    if (STDENDIAN64_NET0_FROM_HOST != 1)
  buf[1] = ptr[1];
#    endif

#    if (STDENDIAN64_NET1_FROM_HOST > 1)
  ptr[1] = ptr[STDENDIAN64_NET1_FROM_HOST];
#    else
  ptr[1] = buf[STDENDIAN64_NET1_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN64_NET2_FROM_HOST != 2)
#    if (STDENDIAN64_NET0_FROM_HOST != 2 && STDENDIAN64_NET1_FROM_HOST != 2)
  buf[2] = ptr[2];
#    endif

#    if (STDENDIAN64_NET2_FROM_HOST > 2)
  ptr[2] = ptr[STDENDIAN64_NET2_FROM_HOST];
#    else
  ptr[2] = buf[STDENDIAN64_NET2_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN64_NET3_FROM_HOST != 3)
#    if (STDENDIAN64_NET0_FROM_HOST != 3 && STDENDIAN64_NET1_FROM_HOST != 3 && STDENDIAN64_NET2_FROM_HOST != 3)
  buf[3] = ptr[3];
#    endif

#    if (STDENDIAN64_NET3_FROM_HOST > 3)
  ptr[3] = ptr[STDENDIAN64_NET3_FROM_HOST];
#    else
  ptr[3] = buf[STDENDIAN64_NET3_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN64_NET4_FROM_HOST != 4)
#    if (STDENDIAN64_NET0_FROM_HOST != 4 && STDENDIAN64_NET1_FROM_HOST != 4 && STDENDIAN64_NET2_FROM_HOST != 4 && STDENDIAN64_NET3_FROM_HOST != 4)
  buf[4] = ptr[4];
#    endif

#    if (STDENDIAN64_NET4_FROM_HOST > 4)
  ptr[4] = ptr[STDENDIAN64_NET4_FROM_HOST];
#    else
  ptr[4] = buf[STDENDIAN64_NET4_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN64_NET5_FROM_HOST != 5)
#    if (STDENDIAN64_NET0_FROM_HOST != 5 && STDENDIAN64_NET1_FROM_HOST != 5 && STDENDIAN64_NET2_FROM_HOST != 5 && \
         STDENDIAN64_NET3_FROM_HOST != 5 && STDENDIAN64_NET4_FROM_HOST != 5)
  buf[5] = ptr[5];
#    endif

#    if (STDENDIAN64_NET5_FROM_HOST > 5)
  ptr[5] = ptr[STDENDIAN64_NET5_FROM_HOST];
#    else
  ptr[5] = buf[STDENDIAN64_NET5_FROM_HOST];
#    endif
#  endif
  

#  if (STDENDIAN64_NET6_FROM_HOST != 6)
#    if (STDENDIAN64_NET0_FROM_HOST != 6 && STDENDIAN64_NET1_FROM_HOST != 6 && STDENDIAN64_NET2_FROM_HOST != 6 && \
         STDENDIAN64_NET3_FROM_HOST != 6 && STDENDIAN64_NET4_FROM_HOST != 6 && STDENDIAN64_NET5_FROM_HOST != 6)
  buf[6] = ptr[6];                           /* ptr[6] was not previously consumed, so we need to save it in buf[6] */
#    endif

#    if (STDENDIAN64_NET6_FROM_HOST > 6)
  ptr[6] = ptr[STDENDIAN64_NET6_FROM_HOST];  /* STDENDIAN64_NET6_FROM_HOST > 6 -> we haven't yet consumed/saved ptr[STDENDIAN64_NET6_FROM_HOST] */
#    else
  ptr[6] = buf[STDENDIAN64_NET6_FROM_HOST];  /* STDENDIAN64_NET6_FROM_HOST < 6 -> we have consumed+saved ptr[STDENDIAN64_NET6_FROM_HOST] in buf */
#    endif
#  endif
  

#  if (STDENDIAN64_NET7_FROM_HOST != 7)
  ptr[7] = buf[STDENDIAN64_NET7_FROM_HOST];  /* we have previously saved ptr[STDENDIAN64_NET7_FROM_HOST] in buf */
#  endif


#endif
}

/************************************************************************************************
 * stdhton_n: Rearranges the 'n' bytes at which 'io' points from host
 * into network byte ordering.  If n is one of (0, 1, 2, 4, 8), then
 * the fcn succeeds.  Otherwise it fails with no side effects,
 * returning STDEINVAL.
 ***********************************************************************************************/

STDINLINE stdcode stdhton_n(void *io, size_t n)
{
  switch (n) {
  case 0:
  case 1:
    break;

  case 2:
    stdhton16(io);
    break;

  case 4:
    stdhton32(io);
    break;

  case 8:
    stdhton64(io);
    break;

  default:
    return STDEINVAL;
  }

  return STDESUCCESS;
}

/************************************************************************************************
 * stdntoh16: Rearranges the two bytes at which 'io' points from
 * network to host byte ordering.
 ***********************************************************************************************/

STDINLINE void stdntoh16(void *io)
{
#if (STDENDIAN16_SWAP == 1)
  stduint8 * ptr = (stduint8*) io;
  stduint8   t;

  STDSWAP(ptr[0], ptr[1], t);
#endif
}

/************************************************************************************************
 * stdntoh32: Rearranges the four bytes at which 'io' points from
 * network to host byte ordering.
 ***********************************************************************************************/

STDINLINE void stdntoh32(void *io)
{
#if (STDENDIAN32_HOST0_FROM_NET != 0 || STDENDIAN32_HOST1_FROM_NET != 1 || STDENDIAN32_HOST2_FROM_NET != 2 || STDENDIAN32_HOST3_FROM_NET != 3)
  stduint8 * ptr = (stduint8*) io;
  stduint8   buf[4];


#  if (STDENDIAN32_HOST0_FROM_NET != 0)
  buf[0] = ptr[0];
  ptr[0] = ptr[STDENDIAN32_HOST0_FROM_NET];
#  endif
  

#  if (STDENDIAN32_HOST1_FROM_NET != 1)
#    if (STDENDIAN32_HOST0_FROM_NET != 1)
  buf[1] = ptr[1];                      
#    endif

#    if (STDENDIAN32_HOST1_FROM_NET > 1)
  ptr[1] = ptr[STDENDIAN32_HOST1_FROM_NET];
#    else
  ptr[1] = buf[STDENDIAN32_HOST1_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN32_HOST2_FROM_NET != 2)
#    if (STDENDIAN32_HOST0_FROM_NET != 2 && STDENDIAN32_HOST1_FROM_NET != 2)
  buf[2] = ptr[2];                           /* ptr[2] was not previously consumed, so we need to save it in buf[2] */
#    endif

#    if (STDENDIAN32_HOST2_FROM_NET > 2)
  ptr[2] = ptr[STDENDIAN32_HOST2_FROM_NET];  /* STDENDIAN32_HOST2_FROM_NET > 2 -> we haven't yet consumed/saved ptr[STDENDIAN32_HOST2_FROM_NET] */
#    else
  ptr[2] = buf[STDENDIAN32_HOST2_FROM_NET];  /* STDENDIAN32_HOST2_FROM_NET < 2 -> we have consumed+saved ptr[STDENDIAN32_HOST2_FROM_NET] in buf */
#    endif
#  endif
  

#  if (STDENDIAN32_HOST3_FROM_NET != 3)
  ptr[3] = buf[STDENDIAN32_HOST3_FROM_NET];  /* we have previously saved ptr[STDENDIAN32_HOST3_FROM_NET] in buf */
#  endif
  

#endif
}

/************************************************************************************************
 * stdntoh64: Rearranges the eight bytes at which 'io' points from
 * network to host byte ordering.
 ***********************************************************************************************/

STDINLINE void stdntoh64(void *io)
{
#if (STDENDIAN64_HOST0_FROM_NET != 0 || STDENDIAN64_HOST1_FROM_NET != 1 || STDENDIAN64_HOST2_FROM_NET != 2 || STDENDIAN64_HOST3_FROM_NET != 3 || \
     STDENDIAN64_HOST4_FROM_NET != 4 || STDENDIAN64_HOST5_FROM_NET != 5 || STDENDIAN64_HOST6_FROM_NET != 6 || STDENDIAN64_HOST7_FROM_NET != 7)
  stduint8 * ptr = (stduint8*) io;
  stduint8   buf[8];


#  if (STDENDIAN64_HOST0_FROM_NET != 0)
  buf[0] = ptr[0];
  ptr[0] = ptr[STDENDIAN64_HOST0_FROM_NET];
#  endif
  

#  if (STDENDIAN64_HOST1_FROM_NET != 1)
#    if (STDENDIAN64_HOST0_FROM_NET != 1)
  buf[1] = ptr[1];
#    endif

#    if (STDENDIAN64_HOST1_FROM_NET > 1)
  ptr[1] = ptr[STDENDIAN64_HOST1_FROM_NET];
#    else
  ptr[1] = buf[STDENDIAN64_HOST1_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN64_HOST2_FROM_NET != 2)
#    if (STDENDIAN64_HOST0_FROM_NET != 2 && STDENDIAN64_HOST1_FROM_NET != 2)
  buf[2] = ptr[2];
#    endif

#    if (STDENDIAN64_HOST2_FROM_NET > 2)
  ptr[2] = ptr[STDENDIAN64_HOST2_FROM_NET];
#    else
  ptr[2] = buf[STDENDIAN64_HOST2_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN64_HOST3_FROM_NET != 3)
#    if (STDENDIAN64_HOST0_FROM_NET != 3 && STDENDIAN64_HOST1_FROM_NET != 3 && STDENDIAN64_HOST2_FROM_NET != 3)
  buf[3] = ptr[3];
#    endif

#    if (STDENDIAN64_HOST3_FROM_NET > 3)
  ptr[3] = ptr[STDENDIAN64_HOST3_FROM_NET];
#    else
  ptr[3] = buf[STDENDIAN64_HOST3_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN64_HOST4_FROM_NET != 4)
#    if (STDENDIAN64_HOST0_FROM_NET != 4 && STDENDIAN64_HOST1_FROM_NET != 4 && STDENDIAN64_HOST2_FROM_NET != 4 && STDENDIAN64_HOST3_FROM_NET != 4)
  buf[4] = ptr[4];
#    endif

#    if (STDENDIAN64_HOST4_FROM_NET > 4)
  ptr[4] = ptr[STDENDIAN64_HOST4_FROM_NET];
#    else
  ptr[4] = buf[STDENDIAN64_HOST4_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN64_HOST5_FROM_NET != 5)
#    if (STDENDIAN64_HOST0_FROM_NET != 5 && STDENDIAN64_HOST1_FROM_NET != 5 && STDENDIAN64_HOST2_FROM_NET != 5 && \
         STDENDIAN64_HOST3_FROM_NET != 5 && STDENDIAN64_HOST4_FROM_NET != 5)
  buf[5] = ptr[5];
#    endif

#    if (STDENDIAN64_HOST5_FROM_NET > 5)
  ptr[5] = ptr[STDENDIAN64_HOST5_FROM_NET];
#    else
  ptr[5] = buf[STDENDIAN64_HOST5_FROM_NET];
#    endif
#  endif
  

#  if (STDENDIAN64_HOST6_FROM_NET != 6)
#    if (STDENDIAN64_HOST0_FROM_NET != 6 && STDENDIAN64_HOST1_FROM_NET != 6 && STDENDIAN64_HOST2_FROM_NET != 6 && \
         STDENDIAN64_HOST3_FROM_NET != 6 && STDENDIAN64_HOST4_FROM_NET != 6 && STDENDIAN64_HOST5_FROM_NET != 6)
  buf[6] = ptr[6];                           /* ptr[6] was not previously consumed, so we need to save it in buf[6] */
#    endif

#    if (STDENDIAN64_HOST6_FROM_NET > 6)
  ptr[6] = ptr[STDENDIAN64_HOST6_FROM_NET];  /* STDENDIAN64_HOST6_FROM_NET > 6 -> we haven't yet consumed/saved ptr[STDENDIAN64_HOST6_FROM_NET] */
#    else
  ptr[6] = buf[STDENDIAN64_HOST6_FROM_NET];  /* STDENDIAN64_HOST6_FROM_NET < 6 -> we have consumed+saved ptr[STDENDIAN64_HOST6_FROM_NET] in buf */
#    endif
#  endif
  

#  if (STDENDIAN64_HOST7_FROM_NET != 7)
  ptr[7] = buf[STDENDIAN64_HOST7_FROM_NET];  /* we have previously saved ptr[STDENDIAN64_HOST7_FROM_NET] in buf */
#  endif


#endif
}

/************************************************************************************************
 * stdntoh_n: Rearranges the 'n' bytes at which 'io' points from
 * network to host byte ordering.  If n is one of (0, 1, 2, 4, 8),
 * then the fcn succeeds.  Otherwise it fails with no side effects,
 * returning STDEINVAL.
 ***********************************************************************************************/

STDINLINE stdcode stdntoh_n(void *io, size_t n)
{
  switch (n) {
  case 0:
  case 1:
    break;

  case 2:
    stdntoh16(io);
    break;

  case 4:
    stdntoh32(io);
    break;

  case 8:
    stdntoh64(io);
    break;

  default:
    return STDEINVAL;
  }

  return 0;
}

/************************************************************************************************
 * stdflip16: Reverses the order of the two bytes at which 'io' points.
 ***********************************************************************************************/

STDINLINE void stdflip16(void *io) 
{
  stduint8 * ptr = (stduint8*) io;
  stduint8   t;

  STDSWAP(ptr[0], ptr[1], t);
}

/************************************************************************************************
 * stdflip32: Reverses the order of the four bytes at which 'io' points.
 ***********************************************************************************************/

STDINLINE void stdflip32(void *io) 
{
  stduint8 * ptr = (stduint8*) io;
  stduint8   t;
  
  STDSWAP(ptr[0], ptr[3], t); 
  STDSWAP(ptr[1], ptr[2], t);
}

/************************************************************************************************
 * stdflip64: Reverses the order of the eight bytes at which 'io'
 * points.
 ***********************************************************************************************/

STDINLINE void stdflip64(void *io) 
{
  stduint8 * ptr = (stduint8*) io;
  stduint8   t;

  STDSWAP(ptr[0], ptr[7], t); 
  STDSWAP(ptr[1], ptr[6], t);
  STDSWAP(ptr[2], ptr[5], t); 
  STDSWAP(ptr[3], ptr[4], t);
}

/************************************************************************************************
 * stdflip_n: Reverses the order of the 'n' bytes at which 'io'
 * points.
 ***********************************************************************************************/

STDINLINE void stdflip_n(void *io, size_t n) 
{
  stduint8 * ptr1 = (stduint8*) io;
  stduint8 * ptr2 = (stduint8*) io + n;
  stduint8 * pend = (stduint8*) io + (n >> 1);
  stduint8   t;

  for (--ptr2; ptr1 != pend; ++ptr1, --ptr2) {
    STDSWAP(*ptr1, *ptr2, t);
  }
}

/************************************************************************************************
 * stdlg_down: Returns the log base 2 of 'x' rounded down to the
 * closest integer (i.e. - floor(lg(x))).  Returns (stduint32) -1 if
 * 'x' is 0.
 ***********************************************************************************************/

STDINLINE stduint32 stdlg_down(stduint64 x) 
{
  stduint32 ret;

  if (x == 0) {
    return (stduint32) -1;
  }

  for (ret = 0; x != 0x1; x >>= 1, ++ret);

  return ret;
}

/************************************************************************************************
 * stdlg_up: Returns the log base 2 of 'x' rounded up to the closest
 * integer (i.e. - ceil(lg(x))).  Returns (stduint32) -1 if 'x' is 0.
 ***********************************************************************************************/

STDINLINE stduint32 stdlg_up(stduint64 x) 
{
  switch (x) {
  default:
    return stdlg_down(x - 1) + 1;

  case 1:
    return 0;

  case 0:
    return (stduint32) -1;
  }
}

/************************************************************************************************
 * stdpow2_down: Returns the greatest power of 2 less than or equal to
 * 'x' (i.e. - 2^floor(lg(x))).  Returns 0 if 'x' is 0.
 ***********************************************************************************************/

STDINLINE stduint64 stdpow2_down(stduint64 x) 
{
  return (x != 0 ? ((stduint64) 0x1 << stdlg_down(x)) : 0);
}

/************************************************************************************************
 * stdpow2_up: Returns the least power of 2 greater than or equal to
 * 'x' (i.e. - 2^ceil(lg(x))).  Returns 0 if 'x' is 0.  Returns 0 on
 * overflow.
 ***********************************************************************************************/

STDINLINE stduint64 stdpow2_up(stduint64 x) 
{
  return (x != 0 ? ((stduint64) 0x1 << stdlg_up(x)) : 0);
}

/************************************************************************************************
 * stdpow2_cap: This fcn returns a power of 2 that is between 3/2 and
 * 3 times of 'x' (expectation of 2.125 times 'x').  Returns 0 if 'x'
 * is zero.  Returns 0 on overflow.  This fcn is good for calculating
 * the size of a table that has to be a power of 2 for a random size
 * request.
 ***********************************************************************************************/

STDINLINE stduint64 stdpow2_cap(stduint64 x) 
{
  stduint64 up_pow2 = stdpow2_up(x);  /* up_pow2 in range [x, 2x) */

  if (up_pow2 < x + (x >> 1)) {       /* if (up_pow2 < 1.5x) */
    up_pow2 <<= 1;                    /*   double -> up_pow2 in range [2x, 3x) */
  }                                   /* else up_pow2 in range [1.5x, 2x) */

  return up_pow2;                     /* up_pow2 in range [1.5x, 3x) */
}

#ifdef __cplusplus
}
#endif
