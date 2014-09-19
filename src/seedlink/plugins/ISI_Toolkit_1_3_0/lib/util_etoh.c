#pragma ident "$Id: etoh.c,v 1.3 2007/01/07 17:40:07 dechavez Exp $"

#include "util.h"

static int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31,31};
static char *month_name[] =
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define mod(a,b)    (a) - ((int)((a)/(b))) * (b)

void util_etoh(struct date_time *dt)
{
    int diy;
    double secleft;

    dt->doy = (int) (dt->epoch / 86400.);
    secleft = mod(dt->epoch,86400.0);
    dt->hour = 0;
    dt->minute = 0;
    dt->second = 0.0;

    if(secleft) {            /* compute hours minutes seconds */
        if(secleft < 0) {    /* before 1970 */
            dt->doy--;        /* subtract a day */
            secleft += 86400;    /* add a day */
        }
        dt->hour = (int) (secleft/3600);
        secleft = mod(secleft,3600.0);
        dt->minute = (int) (secleft/60);
        dt->second = (float) (mod(secleft,60.0));
    }

    if(dt->doy >= 0){
        for( dt->year = 1970 ; ; dt->year++ ){
            diy = ISLEAP(dt->year) ? 366:365;
            if( dt->doy < diy ) break;
            dt->doy -= diy;
        }
    }
    else{
        for( dt->year = 1969 ; ; dt->year-- ){
            diy = ISLEAP(dt->year) ? 366:365;
            dt->doy += diy;
            if( dt->doy >= 0 ) break;
        }
    }
    dt->doy++;
    dt->date = dt->year * 1000 + dt->doy;
    util_month_day(dt);
    return;
}

void util_month_day(struct date_time *dt)
{
    int i,dim,leap;

    leap = ISLEAP(dt->year);
    dt->day = dt->doy;
    for( i = 0 ; i < 12 ; i ++ ){
        dim = days_in_month[i];
        if( leap && i == 1 ) dim++;
        if( dt->day <= dim ) break;
        dt->day -= dim;
    }
    dt->month = i + 1;
    strlcpy(dt->mname,month_name[i], 4);
}

/* Revision History
 *
 * $Log: etoh.c,v $
 * Revision 1.3  2007/01/07 17:40:07  dechavez
 * strlcpy() instead of strcpy()
 *
 * Revision 1.2  2005/05/25 22:41:46  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.1  2004/07/26 23:01:11  dechavez
 * imported from CSS source tree
 *
 */
