/***************************************************************************** 
 * little-endian.h
 *
 * Definitions of little-endian datatypes with automatic byte swapping
 * (C code on little-endian machines, C++ on others)
 *
 * (c) 2000 Andres Heinloo
 *****************************************************************************/

#ifndef LITTLE_ENDIAN_H
#define LITTLE_ENDIAN_H

#include <sys/types.h>

#if !defined(__GNU_LIBRARY__) && !defined(__GLIBC__) && \
  !defined(U_TYPES_DEFINED)
#define U_TYPES_DEFINED
typedef unsigned char  u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int   u_int32_t;
#endif

/* Check for BSD-style BYTE_ORDER definition */

#if !defined(IS_BIG_ENDIAN) && !defined(IS_LITTLE_ENDIAN)
#if BYTE_ORDER == BIG_ENDIAN
#define IS_BIG_ENDIAN
#elif BYTE_ORDER == LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
#else
#error "unspecified byteorder"
#endif
#endif

#include "byteorder_swap.h"

#if defined(IS_BIG_ENDIAN)

typedef ByteOrderSwap::int16_swap   le_int16_t;
typedef ByteOrderSwap::u_int16_swap le_u_int16_t;
typedef ByteOrderSwap::int32_swap   le_int32_t;
typedef ByteOrderSwap::u_int32_swap le_u_int32_t;
typedef ByteOrderSwap::float32_swap le_float32_t;

#elif defined(IS_LITTLE_ENDIAN)

typedef int16_t   le_int16_t;
typedef u_int16_t le_u_int16_t;
typedef int32_t   le_int32_t;
typedef u_int32_t le_u_int32_t;
typedef float     le_float32_t;

#endif

#endif /* LITTLE_ENDIAN_H */

