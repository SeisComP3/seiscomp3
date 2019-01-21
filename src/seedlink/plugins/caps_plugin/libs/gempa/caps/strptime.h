#ifndef __STRPTIME_H__
#define __STRPTIME_H__

/*
 * Version of "strptime()", for the benefit of OSes that don't have it.
 */
extern char *strptime(const char *, const char *, struct tm *);

#endif
