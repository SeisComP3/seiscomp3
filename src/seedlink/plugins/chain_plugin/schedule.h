/*****************************************************************************
 * schedule.h
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

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "bitstring.h"

#define SCHED_MINUTE_ERR    (-1)
#define SCHED_HOUR_ERR      (-2)
#define SCHED_DOM_ERR       (-3)
#define SCHED_MONTH_ERR     (-4)
#define SCHED_DOW_ERR       (-5)
#define SCHED_EOI_ERR       (-6)

#define SCHED_FIRST_MINUTE  0
#define SCHED_LAST_MINUTE   59
#define SCHED_MINUTE_COUNT  (SCHED_LAST_MINUTE - SCHED_FIRST_MINUTE + 1)

#define SCHED_FIRST_HOUR    0
#define SCHED_LAST_HOUR     23
#define SCHED_HOUR_COUNT    (SCHED_LAST_HOUR - SCHED_FIRST_HOUR + 1)

#define SCHED_FIRST_DOM     1
#define SCHED_LAST_DOM      31
#define SCHED_DOM_COUNT     (SCHED_LAST_DOM - SCHED_FIRST_DOM + 1)

#define SCHED_FIRST_MONTH   1
#define SCHED_LAST_MONTH    12
#define SCHED_MONTH_COUNT   (SCHED_LAST_MONTH - SCHED_FIRST_MONTH + 1)

/* note on DOW: 0 and 7 are both Sunday, for compatibility reasons. */
#define SCHED_FIRST_DOW     0
#define SCHED_LAST_DOW      7
#define SCHED_DOW_COUNT     (SCHED_LAST_DOW - SCHED_FIRST_DOW + 1)

typedef struct {
    bitstr_t bit_decl(minute, SCHED_MINUTE_COUNT);
    bitstr_t bit_decl(hour,   SCHED_HOUR_COUNT);
    bitstr_t bit_decl(dom,    SCHED_DOM_COUNT);
    bitstr_t bit_decl(month,  SCHED_MONTH_COUNT);
    bitstr_t bit_decl(dow,    SCHED_DOW_COUNT);
    int flags;
} schedule_t;

#ifdef __cplusplus
extern "C" {
#endif

int init_schedule(schedule_t *sch, const char *minute_hour_dom_month_dow);
int check_schedule(const schedule_t *sch, time_t sec);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULE_H */

