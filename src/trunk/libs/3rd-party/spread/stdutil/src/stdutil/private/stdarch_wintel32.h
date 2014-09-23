/* Copyright (c) 2000-2009, The Johns Hopkins University
 * All rights reserved.
 *
 * The contents of this file are subject to a license (the ``License'').
 * You may not use this file except in compliance with the License. The
 * specific language governing the rights and limitations of the License
 * can be found in the file ``STDUTIL_LICENSE'' found in this
 * distribution.
 *
 * Software distributed under the License is distributed on an AS IS
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 * The Original Software is:
 *     The Stdutil Library
 *
 * Contributors:
 *     Creator - John Lane Schultz (jschultz@cnds.jhu.edu)
 *     The Center for Networking and Distributed Systems
 *         (CNDS - http://www.cnds.jhu.edu)
 */

#ifndef stdarch_wintel32_h_2006_03_13_17_43_22_jschultz_at_cnds_jhu_edu
#define stdarch_wintel32_h_2006_03_13_17_43_22_jschultz_at_cnds_jhu_edu

#ifdef __cplusplus
extern "C" {
#endif

/* the following defines are filled in by the configure script */

#define SIZEOF_CHAR 1
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#undef  SIZEOF_LONG_LONG
#define SIZEOF_SIZE_T 4
#define SIZEOF_VOID_P 4

/* STDARCH_MAX_BYTE_ALIGNMENT must be a power of 2 (e.g. - 1, 2, 4, 8,
   etc.)  and must be the most stringent byte alignment that your
   architecture requires for any basic type; 4 should be good enough
   for any 32 bit or smaller architectures, 8 should be good enough
   for any 64 bit or smaller architectures, etc.
*/

#define STDARCH_MAX_BYTE_ALIGNMENT 4

/* check if NULL is represented as all zero in memory for "all" types of pointers */

#define STDARCH_NULL_IS_ZERO 1

/* endian byte reordering mapping */

#define STDENDIAN16_SWAP 1

#define STDENDIAN32_NET0_FROM_HOST 3
#define STDENDIAN32_NET1_FROM_HOST 2
#define STDENDIAN32_NET2_FROM_HOST 1
#define STDENDIAN32_NET3_FROM_HOST 0

#define STDENDIAN32_HOST0_FROM_NET 3
#define STDENDIAN32_HOST1_FROM_NET 2
#define STDENDIAN32_HOST2_FROM_NET 1
#define STDENDIAN32_HOST3_FROM_NET 0

#define STDENDIAN64_NET0_FROM_HOST 7
#define STDENDIAN64_NET1_FROM_HOST 6
#define STDENDIAN64_NET2_FROM_HOST 5
#define STDENDIAN64_NET3_FROM_HOST 4
#define STDENDIAN64_NET4_FROM_HOST 3
#define STDENDIAN64_NET5_FROM_HOST 2
#define STDENDIAN64_NET6_FROM_HOST 1
#define STDENDIAN64_NET7_FROM_HOST 0

#define STDENDIAN64_HOST0_FROM_NET 7
#define STDENDIAN64_HOST1_FROM_NET 6
#define STDENDIAN64_HOST2_FROM_NET 5
#define STDENDIAN64_HOST3_FROM_NET 4
#define STDENDIAN64_HOST4_FROM_NET 3
#define STDENDIAN64_HOST5_FROM_NET 2
#define STDENDIAN64_HOST6_FROM_NET 1
#define STDENDIAN64_HOST7_FROM_NET 0

/* do architecture specific integer typedefs and checks */

/* ensure char's are 1 byte long */

#if (SIZEOF_CHAR != 1)
#  error No 1 byte integer type found!
#endif

/* ensure short's are 2 bytes long */

#if (SIZEOF_SHORT != 2)
#  error No 2 byte integer type found!
#endif

/* figure out which type is 4 bytes long */

#if (SIZEOF_INT == 4)

typedef int                stdarch_int32;
typedef unsigned int       stdarch_uint32;

#elif (SIZEOF_LONG == 4)

typedef long               stdarch_int32;
typedef unsigned long      stdarch_uint32;

#else
#  error No 4 byte integer type found!
#endif

/* figure out which type is 8 bytes long */

#if (SIZEOF_LONG == 8)

typedef long               stdarch_int64;
typedef unsigned long      stdarch_uint64;

#elif defined(_MSC_VER)

typedef __int64            stdarch_int64;
typedef unsigned __int64   stdarch_uint64;

#elif (SIZEOF_LONG_LONG == 8)

typedef long long          stdarch_int64;
typedef unsigned long long stdarch_uint64;

#else
#  error No 8 byte integeral type found!
#endif

/* figure out which type is the same size as size_t */

#if (SIZEOF_SIZE_T == 2)

typedef short stdarch_ssize;

#elif (SIZEOF_SIZE_T == 4)

typedef stdarch_int32 stdarch_ssize;

#elif (SIZEOF_SIZE_T == 8)

typedef stdarch_int64 stdarch_ssize;

#else
#  error No integral type of same size as size_t!
#endif

#ifdef __cplusplus
}
#endif

#endif
