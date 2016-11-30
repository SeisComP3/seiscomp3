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
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */



#ifndef scatp_h_2000_05_21_19_14_47_jschultz_at_cnds_jhu_edu
#define scatp_h_2000_05_21_19_14_47_jschultz_at_cnds_jhu_edu

#include <sp.h>
#include <stdio.h>

/* don't try to initialize this structure by yourself, instead call
   scatp_begin, scatp_set, or scatp_end -- there are special cases
   where the initial values are not obvious 
*/

typedef struct scatp  /* represents a logical position within a scatter structure */
{ 
  scatter *scat;
  long elem_ind;      /* use signed because scatter uses signed on some archs */
  long buff_ind;

} scatp;

long scat_capacity(const scatter *scat);

/* initializers: need to call one to init a scatp properly */

int scatp_begin(scatp *pos, const scatter *scat);
int scatp_end(scatp *pos, const scatter *scat);
int scatp_set(scatp *pos, const scatter *scat, long offset, int whence);

/* some information about a scatp */

int  scatp_is_legal(const scatp *pos);
int  scatp_is_not_legal(const scatp *pos);
int  scatp_is_end(const scatp *pos);
int  scatp_equals(const scatp *pos1, const scatp *pos2);
long scatp_comp(const scatp *pos1, const scatp *pos2);

/* move current position of a scatp */

long scatp_jforward(scatp *pos, long num_bytes);
long scatp_jbackward(scatp *pos, long num_bytes);
int  scatp_seek(scatp *pos, long offset, int whence);

/* copy from a src to a dst like memcpy does */

long scatp_cpy0(const scatp *dst, const scatp *src, long num_bytes);
long scatp_cpy1(char *dst, const scatp *src, long num_bytes);
long scatp_cpy2(const scatp *dst, char *src, long num_bytes);

/* copy from a src to a dst like memcpy does, and advance the positions if so requested */

long scatp_adv_cpy0(scatp *dst, scatp *src, long num_bytes, int adv_dst, int adv_src);
long scatp_adv_cpy1(char **dst, scatp *src, long num_bytes, int adv_dst, int adv_src);
long scatp_adv_cpy2(scatp *dst, char **src, long num_bytes, int adv_dst, int adv_src);

#endif
