/*
 * gputil.h:
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
 * $Id: gputil.h 2 2005-07-26 19:28:46Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.5  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __GPUTIL_H__
#define __GPUTIL_H__

extern int gp_uint8 (uint8_t * buf);
extern int gp_uint16 (uint8_t * buf);
extern int gp_uint24 (uint8_t * buf);
extern int gp_uint32 (uint8_t * buf);
extern int gp_int8 (uint8_t * buf);
extern int gp_int16 (uint8_t * buf);
extern int gp_int24 (uint8_t * buf);
extern int gp_int32 (uint8_t * buf);
extern int gp_a_to_base36 (char *a);
extern char *gp_base36_to_a (int i);
#endif /* __GPUTIL_H__ */
