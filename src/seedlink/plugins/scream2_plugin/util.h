/*
 * util.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * $Id: util.h 2 2005-07-26 19:28:46Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.8  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdarg.h>

#define MSG_CONSOLE 1
#define MSG_EWLOGIT 2

/*
#define fatal(a)	do { dowhere( __FILE__, __LINE__,"Fatal Error",MSG_CONSOLE | MSG_EWLOGIT); domsg a; exit(-1); } while (0)
#define warning(a)	do { dowhere( __FILE__, __LINE__, "Warning",MSG_EWLOGIT);    domsg a; } while (0)

#define info(a)	do { dowhere( __FILE__, __LINE__, "Info",MSG_EWLOGIT);    doinfo a; } while (0)


extern void dowhere (char *_file, int _line, char *_what, int _where);
extern void domsg (char *fmt, ...);
extern void doinfo (char *fmt, ...);
*/

#define fatal(a) do { printf("%s\n", a); exit(-1); } while(0)
#define warning(a) do { printf("%s\n", a); } while(0)
extern int complete_read (int fd, char *buf, int n);

#endif /* __UTIL_H__ */
