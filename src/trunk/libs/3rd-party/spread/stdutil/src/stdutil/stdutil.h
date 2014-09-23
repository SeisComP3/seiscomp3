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

#ifndef stdutil_h_2000_01_17_16_00_08_jschultz_at_cnds_jhu_edu
#define stdutil_h_2000_01_17_16_00_08_jschultz_at_cnds_jhu_edu

#include <stdutil/stddefines.h>

#ifdef __cplusplus
extern "C" {
#endif

/* string + memory fcns */

STDINLINE stdsize   stdstrcpy(char *dst, const char *src);
STDINLINE stdsize   stdstrcpy_n(char *dst, const char *src, stdsize n);

STDINLINE char *    stdstrdup(const char *dupme, stdsize *duplen);
STDINLINE char *    stdstrdup_n(const char *dupme, stdsize *duplen, stdsize n);

STDINLINE void *    stdmemdup(const void *dupme, stdsize n);

/* 32b hcode fcns */

STDINLINE stduint32 stdhcode_oaat(const void * buf, stdsize buf_len);

STDINLINE void      stdhcode_oaat_start(stduint32 *hsh, stdsize tot_len);
STDINLINE void      stdhcode_oaat_churn(stduint32 *hsh, const void * buf, stdsize buf_len);
STDINLINE void      stdhcode_oaat_stop(stduint32 *hsh);

STDINLINE stduint32 stdhcode_sfh(const void * buf, stdsize buf_len);

STDINLINE void      stdhcode_sfh_start(stduint32 *hsh, stdsize tot_len);
STDINLINE void      stdhcode_sfh_churn(stduint32 *hsh, const void * buf, stdsize buf_len);
STDINLINE void      stdhcode_sfh_stop(stduint32 *hsh);

/* uniform random number generators */

STDINLINE stduint32 stdrand32(stduint16 x[3]);
STDINLINE void      stdrand32_seed(stduint16 x[3], stduint32 seed);
STDINLINE void      stdrand32_dseed(stduint16 x[3], stduint32 seed);

STDINLINE stduint64 stdrand64(stduint32 x[3]);
STDINLINE void      stdrand64_seed(stduint32 x[3], stduint64 seed);
STDINLINE void      stdrand64_dseed(stduint32 x[3], stduint64 seed);

/* in-place host <-> network byte order (endian) flippers */

STDINLINE void      stdhton16(void *io);
STDINLINE void      stdhton32(void *io);
STDINLINE void      stdhton64(void *io);
STDINLINE stdcode   stdhton_n(void *io, size_t n);

STDINLINE void      stdntoh16(void *io);
STDINLINE void      stdntoh32(void *io);
STDINLINE void      stdntoh64(void *io);
STDINLINE stdcode   stdntoh_n(void *io, size_t n);

/* unconditional in-place big <-> little byte order (endian) flippers */

STDINLINE void      stdflip16(void *io);
STDINLINE void      stdflip32(void *io);
STDINLINE void      stdflip64(void *io);
STDINLINE void      stdflip_n(void *io, size_t n);

/* powers of 2 utilities */

STDINLINE stduint32 stdlg_down(stduint64 x);
STDINLINE stduint32 stdlg_up(stduint64 x);

STDINLINE stduint64 stdpow2_down(stduint64 x);
STDINLINE stduint64 stdpow2_up(stduint64 x);
STDINLINE stduint64 stdpow2_cap(stduint64 x);

#ifdef __cplusplus
}
#endif

#endif
