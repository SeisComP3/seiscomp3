/*
 * ./src/util.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: util.h,v 1.2 2004/04/14 11:38:30 root Exp $
 */

/*
 * $Log: util.h,v $
 * Revision 1.2  2004/04/14 11:38:30  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:28  root
 * *** empty log message ***
 *
 * Revision 1.7  2003/05/14 16:31:46  root
 * #
 *
 * Revision 1.6  2003/05/13 09:16:23  root
 * #
 *
 * Revision 1.5  2003/04/16 14:28:04  root
 * #
 *
 * Revision 1.4  2003/04/15 11:04:48  root
 * #
 *
 * Revision 1.3  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:17  root
 * #
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdarg.h>
#include <stdio.h>

typedef enum
{
  G2MsgInfo,
  G2MsgFatal,
  G2MsgWarning,
  G2MsgNonFatal
}
G2MsgAction;

extern void G2msg (const char *, int, const char *, G2MsgAction, const char *,
                   ...);


#define HAVE_PRETTY_FUNCTION

#ifdef HAVE_PRETTY_FUNCTION
#define G2info(fmt...) G2msg(__FILE__,__LINE__,__PRETTY_FUNCTION__,G2MsgInfo,fmt)
#define G2fatal(fmt...) G2msg(__FILE__,__LINE__,__PRETTY_FUNCTION__,G2MsgFatal,fmt)
#define G2warning(fmt...) G2msg(__FILE__,__LINE__,__PRETTY_FUNCTION__,G2MsgWarning,fmt)
#define G2nonfatal(fmt...) G2msg(__FILE__,__LINE__,__PRETTY_FUNCTION__,G2MsgNonFatal,fmt)
#endif

extern int G2CanRead (int);
extern void G2SetBlocking (int);
extern void G2ClrBlocking (int);
extern char *G2strdupandnull (const char *, int);
extern char *G2strsize (uint64_t);
extern void hexdump (FILE * s, void *data, int o, int n);
extern void G2Sleep(int us);


#endif /* __UTIL_H__ */
