/***************************************************************************** 
 * big-endian.h
 *
 * Definitions of big-endian datatypes with automatic byte swapping
 * (C code on big-endian machines, C++ on others)
 *
 * (c) 2000 Andres Heinloo
 *****************************************************************************/

#ifndef BIG_ENDIAN_H
#define BIG_ENDIAN_H

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

typedef int16_t   be_int16_t;
typedef u_int16_t be_u_int16_t;
typedef int32_t   be_int32_t;
typedef u_int32_t be_u_int32_t;
typedef float     be_float32_t;

#elif defined(IS_LITTLE_ENDIAN)

typedef ByteOrderSwap::int16_swap   be_int16_t;
typedef ByteOrderSwap::u_int16_swap be_u_int16_t;
typedef ByteOrderSwap::int32_swap   be_int32_t;
typedef ByteOrderSwap::u_int32_swap be_u_int32_t;
typedef ByteOrderSwap::float32_swap be_float32_t;

#endif

#endif /* BIG_ENDIAN_H */

