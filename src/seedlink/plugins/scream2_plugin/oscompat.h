/*
 * oscompat.h:
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
 * $Id: oscompat.h 2 2005-07-26 19:28:46Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.6  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __OSCOMPAT_H__
#define __OSCOMPAT_H__

/* Ghastly hack to get integer types properly defined*/

#if defined (linux)
#include <stdint.h>
#include <endian.h>
#elif defined (__sun)
#include <sys/int_types.h>
#elif defined (WIN32)
#if 0
#include <basetsd.h>
typedef __int8 int8_t;
typedef __uint8 uint8_t;
typedef __int16 int16_t;
typedef __uint16 uint16_t;
typedef __int32 int32_t;
typedef __uint32 uint32_t;
typedef __int64 int64_t;
typedef __uint64 uint64_t;
#else
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

#define snprintf	_snprintf
#define strcasecmp	_strcmpi

#endif
#endif

#ifdef _OS2
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#include <os2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

/*
#if defined (linux)
#include <endian.h>
#elif defined (WIN32)
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1
#endif
#elif defined (__sparc)
#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 1
#endif
#elif define (__i386)
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1
#endif
#endif
*/

#if defined( _BIG_ENDIAN ) || defined ( __BIG_ENDIAN )
#define SCREAM2EW_DATATYPE "s4"
#elif defined ( _LITTLE_ENDIAN ) || ( __LITTLE_ENDIAN )
#define SCREAM2EW_DATATYPE "i4"
#else
#error Unknown endianness
#endif
#endif /* __OSCOMPAT_H__ */
