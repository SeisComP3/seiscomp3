#ifndef SYSTEM_DEFS_AUTOCONF_H
#define SYSTEM_DEFS_AUTOCONF_H

#ifndef SYSTEM_DEFS_H
#error "system_defs_autoconf.h should never be directly included. Include system_defs.h."
#endif

#define         LOC_INLINE      __inline__  

#ifdef SPU_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>

#ifdef SPU_HAVE_SYS_BITYPES_H
# include <sys/bitypes.h>
#endif

#ifdef SPU_HAVE_LIMITS_H
# include <limits.h>
#endif

#ifndef SPU_HAVE_U_INT
typedef unsigned int u_int;
#endif

#ifndef SPU_HAVE_INTXX_T
# if (SPU_SIZEOF_CHAR == 1)
typedef char int8_t;
# else
#  error "8 bit int type not found."
# endif
# if (SPU_SIZEOF_SHORT_INT == 2)
typedef short int int16_t;
# else
#  error "16 bit int type not found."
# endif
# if (SPU_SIZEOF_INT == 4)
typedef int int32_t;
# else
#  error "32 bit int type not found."
# endif
#endif

/* If sys/types.h does not supply u_intXX_t, supply them ourselves */
#ifndef SPU_HAVE_U_INTXX_T
# ifdef SPU_HAVE_UINTXX_T
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
# define SPU_HAVE_U_INTXX_T 1
# else
#  if (SPU_SIZEOF_CHAR == 1)
typedef unsigned char u_int8_t;
#  else
#   error "8 bit int type not found."
#  endif
#  if (SPU_SIZEOF_SHORT_INT == 2)
typedef unsigned short int u_int16_t;
#  else
#   error "16 bit int type not found."
#  endif
#  if (SPU_SIZEOF_INT == 4)
typedef unsigned int u_int32_t;
#  else
#   error "32 bit int type not found."
#  endif
# endif
#endif

/* 64-bit types */
#ifndef SPU_HAVE_INT64_T
# if (SPU_SIZEOF_LONG_INT == 8)
typedef long int int64_t;
#   define SPU_HAVE_INT64_T 1
# else
#  if (SPU_SIZEOF_LONG_LONG_INT == 8)
typedef long long int int64_t;
#   define SPU_HAVE_INT64_T 1
#   define SPU_HAVE_LONG_LONG_INT
#  endif
# endif
#endif
#ifndef SPU_HAVE_U_INT64_T
# if (SPU_SIZEOF_LONG_INT == 8)
typedef unsigned long int u_int64_t;
#   define SPU_HAVE_U_INT64_T 1
# else
#  if (SPU_SIZEOF_LONG_LONG_INT == 8)
typedef unsigned long long int u_int64_t;
#   define SPU_HAVE_U_INT64_T 1
#  endif
# endif
#endif


#ifndef byte
#define byte u_int8_t
#endif

#ifndef int16
#define int16 int16_t
#endif

#ifndef int16u
#define int16u u_int16_t
#endif

#ifndef int32
#define int32 int32_t
#endif

#ifndef int32u
#define int32u u_int32_t
#endif

#ifdef UIO_MAXIOV
#define         SPU_ARCH_SCATTER_SIZE       UIO_MAXIOV
#else
#define         SPU_ARCH_SCATTER_SIZE       1024
#endif



#endif /* SYSTEM_DEFS_AUTOCONF_H */
