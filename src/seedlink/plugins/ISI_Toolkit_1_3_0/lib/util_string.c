#pragma ident "$Id: string.c,v 1.6 2007/06/28 19:32:49 dechavez Exp $"
/*======================================================================
 *
 *  various strings related utilities
 *
 *====================================================================*/
#include "util.h"

char *utilPadString(char *input, int maxlen, int padchar)
{
int i;

    if (strlen(input) == (size_t) maxlen) return input;
    for (i = strlen(input); i < maxlen-1; i++) input[i] = padchar;
    input[maxlen-1] = 0;

    return input;

}

char *utilTrimString(char *input)
{
int n;

    n = strlen(input) - 1;
    while (n >= 0 && input[n] == ' ') --n;
    input[++n] = 0;

    return input;

}

char *util_strpad(char *input, int maxlen, int padchar)
{
    return utilPadString(input, maxlen, padchar);
}

char *util_strtrm(char *input)
{
    return utilTrimString(input);
}

char *utilBoolToString(BOOL value)
{
static char *TRUEstring  = "TRUE";
static char *FALSEstring = "FALSE";

    return value ? TRUEstring : FALSEstring;
}

char *utilProcessPriorityToString(int priority)
{
static char *sIDLE_PRIORITY_CLASS     = "IDLE_PRIORITY_CLASS";
static char *sNORMAL_PRIORITY_CLASS   = "NORMAL_PRIORITY_CLASS";
static char *sHIGH_PRIORITY_CLASS     = "HIGH_PRIORITY_CLASS";
static char *sREALTIME_PRIORITY_CLASS = "REALTIME_PRIORITY_CLASS";
static char *sUnknownPriority          = "Unknown Priority!";

    switch (priority) {
#ifdef IDLE_PRIORITY_CLASS
      case IDLE_PRIORITY_CLASS: return sIDLE_PRIORITY_CLASS;
#endif /* IDLE_PRIORITY_CLASS */

#ifdef NORMAL_PRIORITY_CLASS
      case NORMAL_PRIORITY_CLASS: return sNORMAL_PRIORITY_CLASS;
#endif /* NORMAL_PRIORITY_CLASS */

#ifdef HIGH_PRIORITY_CLASS
      case HIGH_PRIORITY_CLASS: return sHIGH_PRIORITY_CLASS;
#endif /* HIGH_PRIORITY_CLASS */

#ifdef REALTIME_PRIORITY_CLASS
      case REALTIME_PRIORITY_CLASS: return sREALTIME_PRIORITY_CLASS;
#endif /* REALTIME_PRIORITY_CLASS */

      default:
        return sUnknownPriority;
    }
}

char *utilThreadPriorityToString(int priority)
{
static char *sTHREAD_PRIORITY_LOWEST       = "THREAD_PRIORITY_LOWEST";
static char *sTHREAD_PRIORITY_BELOW_NORMAL = "THREAD_PRIORITY_BELOW_NORMAL";
static char *sTHREAD_PRIORITY_NORMAL       = "THREAD_PRIORITY_NORMAL";
static char *sTHREAD_PRIORITY_ABOVE_NORMAL = "THREAD_PRIORITY_ABOVE_NORMAL";
static char *sTHREAD_PRIORITY_HIGHEST      = "THREAD_PRIORITY_HIGHEST";
static char *sUnknownPriority              = "Unknown Priority!";

    switch (priority) {
#ifdef THREAD_PRIORITY_LOWEST
      case THREAD_PRIORITY_LOWEST: return sTHREAD_PRIORITY_LOWEST;
#endif /* THREAD_PRIORITY_LOWEST */

#ifdef THREAD_PRIORITY_BELOW_NORMAL
      case THREAD_PRIORITY_BELOW_NORMAL: return sTHREAD_PRIORITY_BELOW_NORMAL;
#endif /* THREAD_PRIORITY_BELOW_NORMAL */

#ifdef THREAD_PRIORITY_NORMAL
      case THREAD_PRIORITY_NORMAL: return sTHREAD_PRIORITY_NORMAL;
#endif /* THREAD_PRIORITY_NORMAL */

#ifdef THREAD_PRIORITY_ABOVE_NORMAL
      case THREAD_PRIORITY_ABOVE_NORMAL: return sTHREAD_PRIORITY_ABOVE_NORMAL;
#endif /* THREAD_PRIORITY_ABOVE_NORMAL */

#ifdef THREAD_PRIORITY_HIGHEST
      case THREAD_PRIORITY_HIGHEST: return sTHREAD_PRIORITY_HIGHEST;
#endif /* THREAD_PRIORITY_HIGHEST */

      default:
        return sUnknownPriority;
    }
}

int utilStringToProcessPriority(char *string, BOOL *status)
{

    if (status != NULL) *status = TRUE;

#ifdef IDLE_PRIORITY_CLASS
    if (
        strcasecmp(string, "IDLE_PRIORITY_CLASS") == 0 ||
        strcasecmp(string, "idle") == 0
    ) return IDLE_PRIORITY_CLASS;
#endif /* IDLE_PRIORITY_CLASS */

#ifdef NORMAL_PRIORITY_CLASS
    if (
        strcasecmp(string, "NORMAL_PRIORITY_CLASS") == 0 ||
        strcasecmp(string, "normal") == 0
    ) return NORMAL_PRIORITY_CLASS;
#endif /* NORMAL_PRIORITY_CLASS */

#ifdef HIGH_PRIORITY_CLASS
    if (
        strcasecmp(string, "HIGH_PRIORITY_CLASS") == 0 ||
        strcasecmp(string, "high") == 0
    ) return HIGH_PRIORITY_CLASS;
#endif /* HIGH_PRIORITY_CLASS */

#ifdef REALTIME_PRIORITY_CLASS
    if (
        strcasecmp(string, "REALTIME_PRIORITY_CLASS") == 0 ||
        strcasecmp(string, "realtime") == 0
    ) return REALTIME_PRIORITY_CLASS;
#endif /* REALTIME_PRIORITY_CLASS */

    if (status != NULL) *status = FALSE;
    return 0;
}

int utilStringToThreadPriority(char *string, BOOL *status)
{

    if (status != NULL) *status = TRUE;

#ifdef THREAD_PRIORITY_LOWEST
    if (
        strcasecmp(string, "THREAD_PRIORITY_LOWEST") == 0 ||
        strcasecmp(string, "lowest") == 0
    ) return THREAD_PRIORITY_LOWEST;
#endif /* THREAD_PRIORITY_LOWEST */

#ifdef THREAD_PRIORITY_BELOW_NORMAL
    if (
        strcasecmp(string, "THREAD_PRIORITY_BELOW_NORMAL") == 0 ||
        strcasecmp(string, "below_normal") == 0
    ) return THREAD_PRIORITY_BELOW_NORMAL;
#endif /* THREAD_PRIORITY_BELOW_NORMAL */

#ifdef THREAD_PRIORITY_NORMAL
    if (
        strcasecmp(string, "THREAD_PRIORITY_NORMAL") == 0 ||
        strcasecmp(string, "normal") == 0
    ) return THREAD_PRIORITY_NORMAL;
#endif /* THREAD_PRIORITY_NORMAL */

#ifdef THREAD_PRIORITY_ABOVE_NORMAL
    if (
        strcasecmp(string, "THREAD_PRIORITY_ABOVE_NORMAL") == 0 ||
        strcasecmp(string, "above_normal") == 0
    ) return THREAD_PRIORITY_ABOVE_NORMAL;
#endif /* THREAD_PRIORITY_ABOVE_NORMAL */

#ifdef THREAD_PRIORITY_HIGHEST
    if (
        strcasecmp(string, "THREAD_PRIORITY_HIGHEST") == 0 ||
        strcasecmp(string, "highest") == 0
    ) return THREAD_PRIORITY_HIGHEST;
#endif /* THREAD_PRIORITY_HIGHEST */

    if (status != NULL) *status = FALSE;
    return 0;
}

#ifndef HAVE_STRLCPY

char *strlcpy (char *dst, const char *src, size_t len)
{
    if(src == NULL) return NULL;
    if(dst == NULL) return NULL;
    if(len <= 0)    return NULL;

    if (strlen(src) < len) return strcpy (dst, src);
 
    memcpy (dst, src, len);
    dst [len-1] = '\0';
    return (dst);
}

#endif /* !HAVE_STRLCPY */

/* Revision History
 *
 * $Log: string.c,v $
 * Revision 1.6  2007/06/28 19:32:49  dechavez
 * added strlcpy for systems without HAVE_STRLCPY defined
 *
 * Revision 1.5  2005/09/30 18:04:41  dechavez
 * utilPadString(), utilTrimString()
 *
 * Revision 1.4  2005/05/25 22:41:46  dechavez
 * mods to calm Visual C++ warnings
 *
 * Revision 1.3  2003/06/09 23:57:22  dechavez
 * added utilBoolToString() and utilProcessPriorityToString()
 *
 * Revision 1.2  2001/05/07 22:40:13  dec
 * ANSI function declarations
 *
 * Revision 1.1.1.1  2000/02/08 20:20:41  dec
 * import existing IDA/NRTS sources
 *
 */
