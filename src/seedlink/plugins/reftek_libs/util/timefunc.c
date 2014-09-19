#pragma ident "$Id: timefunc.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Misc. time related functions.
 *
 *----------------------------------------------------------------------
 *
 *  util_attodt:
 *  Convert a string of the form "yyyy:ddd-hh:mm:ss:msc" to double time.
 *  Can truncate fields from the right.
 *
 *----------------------------------------------------------------------
 *
 *  util_dttostr:
 *  Given a double time value and format code, make a string of one of
 *  the following formats:
 *
 *  Format code   Output string
 *       0        yyyy:ddd-hh:mm:ss.msc
 *       1        Mon dd, year hh:mm:ss:msc
 *       2        yy:ddd-hh:mm:ss.msc, where input time is an interval
 *       3        yyyydddhhmmssmsc
 *       4        yyyyddd
 *       5        Day Mon dd, year
 *       6        yyyymmddhhmmss
 *       7        yyyy mm dd hh mm ss
 *       8        ddd-hh:mm:ss.msc, where input time is an interval
 *
 *  No newline is appended.
 *
 *----------------------------------------------------------------------
 *
 *  util_lttostr:
 *  Given a long time value and format code, make a string of one of
 *  the following formats:
 *
 *  Format code   Output string
 *       0        yyyy:ddd-hh:mm:ss
 *       1        Mon dd, year hh:mm:ss
 *       2        yy:ddd-hh:mm:ss, where input time is an interval
 *       3        yydddhhmmss
 *       4        yyyyddd
 *       5        Day Mon dd, year
 *       6        yyyymmddhhmmss
 *       7        yyyy mm dd hh mm ss
 *       8        ddd-hh:mm:ss, where input time is an interval
 *
 *  No newline is appended.
 *
 *----------------------------------------------------------------------
 *
 *  util_tsplit:
 *  Split a double time to yyyy, ddd, hh, mm, ss, msc.
 *
 *----------------------------------------------------------------------
 *
 *  util_ydhmsmtod:
 *  Given year, day, hour, minutes, seconds, and milliseconds, return
 *  a double containing the seconds since 00:00:00.000 Jan 1, 1970.
 *
 *  Only works for times after Jan 1, 1970!
 *
 *----------------------------------------------------------------------
 *
 *  util_jdtomd:
 *  Given year and day of year, determine month and day of month.
 *
 *----------------------------------------------------------------------
 *
 *  util_ymdtojd:
 *  Given year, month, and day determine day of year.
 *
 *----------------------------------------------------------------------
 *
 *  util_today:
 *  Returns today's date in YYYYDDD form.
 *
 *====================================================================*/
#include "util.h"

#ifndef leap_year
#define leap_year(i) ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0)
#endif
 
#ifndef daysize
#define daysize(i) (365 + leap_year(i))
#endif

static CHAR daytab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static CHAR *month_name[] = {
    "   ", /* cause we increment it after gmtime() */
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static CHAR *day_name[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static CHAR *bad_call = "Bad util_dttostr input!";

static UINT16 minimum_code = 0;
static UINT16 maximum_code = 8;
static CHAR   *bad_string[9] = {
    "                     ",
    "                         ",
    "                   ",
    "                ",
    "       ",
    "                ",
    "              ",
    "                   ",
    "                "
};

#define SPM (       60L)
#define SPH (SPM *  60L)
#define SPD (SPH *  24L)
#define SPY (SPD * 365L)

/***********************************************************************/

#define COPYBUFLEN 32

REAL64 util_attodt(CHAR *string)
{
CHAR copy[COPYBUFLEN];
CHAR *token[6];
INT16 errors, ntoken, yr, da, hr, mn, sc, ms;

/*  Check for special case ("present")  */

    if (strcasecmp(string, "present") == 0) return (double) 2147483647;

/*  Parse (copy of) string  */

    memcpy((void *) copy, (void *) string, (size_t) COPYBUFLEN-1);
    copy[COPYBUFLEN] = 0;

    ntoken = util_parse(copy, token, "-/.:(),;", 6, 0);

    yr = hr = mn = sc = ms = 0; da = 1;

/* Decode the various pieces */

    switch (ntoken) {

      case 6:
        ms = (INT16) atoi(token[5]);
      case 5:
        sc = (INT16) atoi(token[4]);
      case 4:
        mn = (INT16) atoi(token[3]);
      case 3:
        hr = (INT16) atoi(token[2]);
      case 2:
        da = (INT16) atoi(token[1]);
      case 1:
        yr = (INT16) atoi(token[0]) + ((strlen(token[0]) == 2) ? 1900 : 0);
        break;
      default:
        errno = EINVAL;
        return -1.0;
    }

    errors = 0;
    if (yr < 1970)          ++errors;
    if (da < 1 || da > 366) ++errors;
    if (hr < 0 || hr >  23) ++errors;
    if (mn < 0 || mn >  59) ++errors;
    if (sc < 0 || sc >  59) ++errors;
    if (ms < 0 || ms > 999) ++errors;
    
    if (errors) {
        errno = EINVAL;
        return -2.0;
    }

    return util_ydhmsmtod(yr, da, hr, mn, sc, ms);

}

/***********************************************************************/

CHAR *util_dttostr(REAL64 dtime, UINT16 code, CHAR *buf)
{
struct tm *tm;
long  ltime;
float ffrac;
int   ifrac, yr, da, hr, mn, sc, ms;

    if (code < minimum_code || code > maximum_code) return bad_call;

    ltime = (long) dtime;
    ffrac = (float) ((dtime - (double) ltime) * 1000.0);
    ifrac = (int) ffrac;
    if (ffrac - (float) ifrac >= 0.5) ifrac++;

/* Deal with the intervals */

    if (code == 2 || code == 8) {
        yr = ltime / SPY; ltime -= yr * SPY;
        da = ltime / SPD; ltime -= da * SPD;
        hr = ltime / SPH; ltime -= hr * SPH;
        mn = ltime / SPM; ltime -= mn * SPM;
        sc = ltime;
        ms = ifrac;
        if (code == 2) {
            sprintf((char *) buf,"%2.2d:%3.3d-%2.2d:%2.2d:%2.2d.%3.3d",
                (int) yr, (int) da, (int) hr, (int) mn, (int) sc, (int) ms
            );
        } else {
            sprintf((char *) buf,"%3.3d-%2.2d:%2.2d:%2.2d.%3.3d",
                (int) da, (int) hr, (int) mn, (int) sc, (int) ms
            );
        }
        return buf;
    }

    if ((tm = gmtime(&ltime)) == NULL) return bad_string[code];
    tm->tm_year += 1900;
    tm->tm_yday += 1;
    tm->tm_mon  += 1;

    switch (code) {
        case 0:
            sprintf((char *) buf,"%4.4d:%3.3d-%2.2d:%2.2d:%2.2d.%3.3d",
                   tm->tm_year, tm->tm_yday, tm->tm_hour,
                   tm->tm_min, tm->tm_sec, ifrac);
            break;
        case 1:
            sprintf((char *) buf, "%s %2.2d, %4.4d %2.2d:%2.2d:%2.2d.%3.3d",
              month_name[tm->tm_mon], tm->tm_mday, tm->tm_year,
              tm->tm_hour, tm->tm_min, tm->tm_sec, ifrac);
            break;
        case 3:
            sprintf((char *) buf,"%4.4d%3.3d%2.2d%2.2d%2.2d%3.3d",
                   tm->tm_year, tm->tm_yday, tm->tm_hour,
                   tm->tm_min, tm->tm_sec, ifrac);
            break;
        case 4:
            sprintf((char *) buf,"%4.4d%3.3d",
                   tm->tm_year, tm->tm_yday);
            break;
        case 5:
            sprintf((char *) buf, "%s %2.2d/%2.2d/%4.4d",
              day_name[tm->tm_wday], tm->tm_mon, tm->tm_mday, tm->tm_year);
            break;
        case 6:
            sprintf((char *) buf,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",
                   tm->tm_year, tm->tm_mon, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec);
            break;
        case 7:
            sprintf((char *) buf,"%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d",
                   tm->tm_year, tm->tm_mon, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec);
            break;
        default: 
            strcpy(buf, bad_call);
            break;
    }

    return buf;
}

/***********************************************************************/

CHAR *util_lttostr(INT32 ltime, UINT16 code, CHAR *buf)
{
CHAR *string;

    string = util_dttostr((double) ltime, code, buf);
    if (code >= 4 && code <= 7) return string;

    if (code != 3) {
        string[strlen(string)-strlen(".msc")] = 0;
    } else {
        string[strlen(string)-strlen("msc")] = 0;
    }

    return string;
}

/***********************************************************************/

VOID util_tsplit(
    REAL64 dtime, UINT16 *yr, UINT16 *da, UINT16 *hr, UINT16 *mn, UINT16 *sc, UINT16 *ms
){
INT32  ltime;
UINT16  imsc;
REAL64 dmsc;
struct tm *tiempo;

    ltime = (INT32) dtime;
    dmsc = ((dtime - (REAL64) ltime)) * (REAL64) 1000.0;
    imsc = (UINT16) dmsc;
    if (dmsc - (REAL64) imsc >= (double) 0.5) imsc++;
    if (imsc == 1000) {
        ++ltime;
        imsc = 0;
    }

    tiempo = gmtime(&ltime);
    *yr = 1900 + tiempo->tm_year;
    *da = ++tiempo->tm_yday;
    *hr = tiempo->tm_hour;
    *mn = tiempo->tm_min;
    *sc = tiempo->tm_sec;
    *ms = imsc;
}

/***********************************************************************/

REAL64 util_ydhmsmtod(UINT16 yr, UINT16 da, UINT16 hr, UINT16 mn, UINT16 sc, UINT16 ms)
{
UINT16 i, days_in_year_part;
INT32   secs;

    days_in_year_part = 0;
    for (i = 1970; i < yr; i++) days_in_year_part += daysize(i);
    secs = (INT32) days_in_year_part * SPD;

    secs += (INT32)(da-1)*SPD + (INT32)hr*SPH + (INT32)mn*SPM + (INT32)sc;

    return (REAL64) secs + ((REAL64) (ms)/1000.0);

}

/***********************************************************************/

VOID util_jdtomd(UINT16 year, UINT16 day, UINT16 *m_no, UINT16 *d_no)
{
UINT16 i, leap;

    leap = leap_year(year);
    
    for (i = 1; day > daytab[leap][i]; i++) day -= daytab[leap][i];

    *m_no = i;
    *d_no = day;
}

/***********************************************************************/

INT32 util_ymdtojd(UINT16 year, UINT16 mo, UINT16 da)
{
INT32 jd, m, leap;

    leap = leap_year(year);
    for (jd = 0, m = 1; m < mo; m++) jd += daytab[leap][m];
    jd += da;

    return jd;
}

/***********************************************************************/

INT32 util_today()
{
time_t now;
struct tm *current;

    now = time(NULL);
    current = localtime(&now);
    current->tm_year += 1900;
    ++current->tm_yday;

    return (1000 * current->tm_year) + current->tm_yday;

}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:51:45  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
