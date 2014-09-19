/*****************************************************************************
 * schedule.c
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *
 * This code is derived from vixie cron sources, original copyright notice
 * follows.
 *
 *   Copyright 1988,1990,1993,1994 by Paul Vixie
 *   All rights reserved
 *
 *   Distribute freely, except: don't remove my name from the source or
 *   documentation (don't take credit for my work), mark your changes (don't
 *   get me blamed for your possible bugs), don't alter or remove this
 *   notice.  May be sold if buildable source is provided to buyer.  No
 *   warrantee of any kind, express or implied, is included with this
 *   software; use at your own risk, responsibility for damages (if any) to
 *   anyone resulting from the use of this software rests entirely with the
 *   user.
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "schedule.h"

#define TRUE   1
#define FALSE  0
#define OK     0
#define ERR    (-1)

#define MAX_TEMPSTR   100

#define DOM_STAR  0x01
#define DOW_STAR  0x02

static const char *MonthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    NULL
  };

static const char *DowNames[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
    NULL
  };

static int get_list(bitstr_t *bits, int low, int high, const char **names, const char **pp);
static int get_range(bitstr_t *bits, int low, int high, const char **names, const char **pp);
static int get_number(int *numptr, int low, const char **names, const char **pp);
static int set_element(bitstr_t *bits, int low, int high, int number);

int init_schedule(schedule_t *sch, const char *minute_hour_dom_month_dow)
  {
    const char *p;

    p = minute_hour_dom_month_dow;
    
    if(get_list(sch->minute, SCHED_FIRST_MINUTE, SCHED_LAST_MINUTE, NULL, &p) == ERR)
        return SCHED_MINUTE_ERR;

    if(get_list(sch->hour, SCHED_FIRST_HOUR, SCHED_LAST_HOUR, NULL, &p) == ERR)
        return SCHED_HOUR_ERR;

    if(*p == '*') sch->flags |= DOM_STAR;
    
    if(get_list(sch->dom, SCHED_FIRST_DOM, SCHED_LAST_DOM, NULL, &p) == ERR)
        return SCHED_DOM_ERR;

    if(get_list(sch->month, SCHED_FIRST_MONTH, SCHED_LAST_MONTH, MonthNames, &p) == ERR)
        return SCHED_MONTH_ERR;

    if(*p == '*') sch->flags |= DOW_STAR;
    
    if(get_list(sch->dow, SCHED_FIRST_DOW, SCHED_LAST_DOW, DowNames, &p) == ERR)
        return SCHED_DOW_ERR;

    if(*p != 0) return SCHED_EOI_ERR;
    
    /* make sundays equivilent */
    if(bit_test(sch->dow, 0) || bit_test(sch->dow, 7))
      {
        bit_set(sch->dow, 0);
        bit_set(sch->dow, 7);
      }

    return OK;
  }

int check_schedule(const schedule_t *sch, time_t sec)
  {
    int minute, hour, dom, month, dow;
    struct tm* tm = localtime(&sec);
    
    /* make 0-based values out of these so we can use them as indicies
     */
    minute = tm->tm_min -SCHED_FIRST_MINUTE;
    hour = tm->tm_hour -SCHED_FIRST_HOUR;
    dom = tm->tm_mday -SCHED_FIRST_DOM;
    month = tm->tm_mon +1 /* 0..11 -> 1..12 */ -SCHED_FIRST_MONTH;
    dow = tm->tm_wday -SCHED_FIRST_DOW;

    /* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
     * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
     * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
     * is why we keep 'sch->dow_star' and 'sch->dom_star'.  yes, it's bizarre.
     * like many bizarre things, it's the standard.
     */

    if (bit_test(sch->minute, minute)
      && bit_test(sch->hour, hour)
      && bit_test(sch->month, month)
      && (((sch->flags & DOM_STAR) || (sch->flags & DOW_STAR))
        ? (bit_test(sch->dow,dow) && bit_test(sch->dom,dom))
        : (bit_test(sch->dow,dow) || bit_test(sch->dom,dom))))
        return TRUE;

    return FALSE;
  }

int get_list(bitstr_t *bits, int low, int high, const char **names, const char **pp)
  {
    /* clear the bit string, since the default is 'off'.
     */
    bit_nclear(bits, 0, (high-low+1));

    /* list = range {"," range}
     */
    
    /* process all ranges
     */
    while(1)
      {
        if(get_range(bits, low, high, names, pp) == ERR)
            return ERR;

        if(**pp == ',') ++(*pp);
        else break;
      }

    /* exiting.  skip to some blanks, then skip over the blanks.
     */
    while(**pp != 0 && !isspace(**pp)) ++(*pp);
    while(isspace(**pp)) ++(*pp);

    return OK;
  }

int get_range(bitstr_t *bits, int low, int high, const char **names, const char **pp)
  {
    /* range = number | number "-" number [ "/" number ]
     */

    int i;
    int num1, num2, num3;

    if(**pp == '*')
      {
        /* '*' means "first-last" but can still be modified by /step
         */
        num1 = low;
        num2 = high;
        ++(*pp);
      }
    else
      {
        if(get_number(&num1, low, names, pp) == ERR) return ERR;

        if (**pp != '-')
          {
            /* not a range, it's a single number.
             */
            if(set_element(bits, low, high, num1) == ERR) return ERR;
            return OK;
          }
        else
          {
            /* eat the dash
             */
            ++(*pp);

            /* get the number following the dash
             */
            if(get_number(&num2, low, names, pp) == ERR) return ERR;
          }
      }

    /* check for step size
     */
    if(**pp == '/')
      {
        /* eat the slash
         */
        ++(*pp);
        
        /* get the step size
         */
        if(get_number(&num3, low, names, pp) == ERR) return ERR;
      }
    else
      {
        /* no step.  default==1.
         */
        num3 = 1;
      }

    /* range. set all elements from num1 to num2, stepping
     * by num3.  (the step is a downward-compatible extension
     * proposed conceptually by bob@acornrc, syntactically
     * designed then implmented by paul vixie).
     */
    for(i = num1;  i <= num2;  i += num3)
        if(set_element(bits, low, high, i) == ERR)
            return ERR;

    return OK;
  }

int get_number(int *numptr, int low, const char **names, const char **pp)
  {
    char temp[MAX_TEMPSTR], *pc;
    int len, i, all_digits;

    if(**pp == 0) return ERR;
    
    /* collect alphanumerics into our fixed-size temp array
     */
    pc = temp;
    len = 0;
    all_digits = TRUE;
    while(isalnum(**pp))
      {
        if(++len >= MAX_TEMPSTR)
            return ERR;

        *pc++ = **pp;

        if(!isdigit(**pp))
            all_digits = FALSE;

        ++(*pp);
      }
    
    *pc = '\0';

    /* try to find the name in the name list
     */
    if (names)
      {
        for (i = 0;  names[i] != NULL;  i++)
          {
            if (!strcasecmp(names[i], temp))
              {
                *numptr = i + low;
                return OK;
              }
          }
      }

    /* no name list specified, or there is one and our string isn't
     * in it.  either way: if it's all digits, use its magnitude.
     * otherwise, it's an error.
     */
    if (all_digits)
      {
        *numptr = atoi(temp);
        return OK;
      }

    return ERR;
  }

int set_element(bitstr_t *bits, int low, int high, int number)
  {
    if (number < low || number > high)
        return ERR;

    bit_set(bits, (number-low));
    return OK;
  }

