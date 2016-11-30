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



#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "stats.h"

static int dbl_cmp(const void *arg1, const void *arg2) 
{
  double a1 = *(double*) arg1, a2 = *(double*) arg2;

  if (a1 < a2) {
    return -1;

  } else if (a1 > a2) {
    return 1;
  }
  return 0;
}

int comp_stats(stats_results *ans, double *samples, size_t num_samples) 
{
  size_t ind5 = num_samples / 20, ind95 = 19 * num_samples / 20, num_samples90 = ind95 - ind5 + 1;
  size_t i;

  /* total90, mean90, stddev90, min5, max95 are all computed from samples [5%, 95%] range */

  if (num_samples == 0) { 
    return -1;
  }  
  qsort(samples, num_samples, sizeof(double), dbl_cmp);  

  ans->min    = samples[0];
  ans->min5   = samples[ind5];
  ans->max95  = samples[ind95];
  ans->max    = samples[num_samples - 1];

  ans->quart1 = samples[num_samples / 4];
  ans->median = samples[num_samples / 2];
  ans->quart3 = samples[3 * num_samples / 4];

  for (i = 0, ans->total = 0, ans->total90 = 0; i < ind5; ++i) {
    ans->total += samples[i];
  }
  for (i = ind5; i <= ind95; ++i) {
    ans->total90 += samples[i];
  }
  for (i = ind95 + 1; i < num_samples; ++i) {
    ans->total += samples[i];
  }
  ans->mean90 = ans->total90 / num_samples90;
  ans->mean   = (ans->total + ans->total90) / num_samples;

  if (num_samples != 1) {
    for (i = 0, ans->stddev = 0, ans->stddev90 = 0; i < ind5; ++i) {
      ans->stddev += (ans->mean - samples[i]) * (ans->mean - samples[i]);    
    }
    for (i = ind5; i <= ind95; ++i) {
      ans->stddev90 += (ans->mean90 - samples[i]) * (ans->mean90 - samples[i]);
      ans->stddev   += (ans->mean - samples[i]) * (ans->mean - samples[i]);    
    }
    for (i = ind95 + 1; i < num_samples; ++i) {
      ans->stddev += (ans->mean - samples[i]) * (ans->mean - samples[i]);    
    }
    ans->stddev90 = sqrt(ans->stddev90 / (num_samples90 - 1));
    ans->stddev   = sqrt(ans->stddev / (num_samples - 1));

  } else {
    ans->stddev90 = 0;
    ans->stddev   = 0;
  }
  return 0;
}

double get_time_timeofday(void) 
{
  struct timeval used;

  gettimeofday(&used, 0);

  return used.tv_sec * 1000.0 + used.tv_usec / 1000.0;
}
