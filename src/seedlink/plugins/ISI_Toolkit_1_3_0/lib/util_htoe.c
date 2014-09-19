#pragma ident "$Id: htoe.c,v 1.1 2004/07/26 23:01:11 dechavez Exp $"

#include "util.h"
static int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};

static double dtoepoch(long date)
{
    register int    cnt;
    double  days;

    cnt = (int)(date / 1000);
    days = 0;
    if (cnt > 1970)
        while (--cnt >= 1970)
            days += ISLEAP(cnt) ? 366 : 365;
    else if (cnt < 1970)
        while (cnt < 1970) {
            days -= ISLEAP(cnt) ? 366 : 365;
            cnt++;
        }
    return((days + (date - 1) % 1000) * 86400.);
}

void util_htoe(struct date_time *dt)
{
    dt->epoch = 
    dtoepoch(dt->date) + 
    dt->hour * 3600. + 
    dt->minute * 60. +
    dt->second;
}

/* Revision History
 *
 * $Log: htoe.c,v $
 * Revision 1.1  2004/07/26 23:01:11  dechavez
 * imported from CSS source tree
 *
 */
