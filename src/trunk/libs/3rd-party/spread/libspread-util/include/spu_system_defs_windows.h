#ifndef SYSTEM_DEFS_WINDOWS_H
#define SYSTEM_DEFS_WINDOWS_H

#ifndef SYSTEM_DEFS_H
#error "system_defs_windows.h should never be directly included. Include system_defs.h."
#endif

#define         LOC_INLINE      __inline__

#ifndef int16
#define int16 short
#endif

#ifndef int16u
#define int16u unsigned short
#endif

#ifndef int32
#define int32 int
#endif

#ifndef int32u
#define int32u unsigned int
#endif

#ifndef UINT32_MAX
#define         UINT32_MAX      UINT_MAX
#endif
#ifndef INT32_MAX
#define         INT32_MAX       INT_MAX
#endif

#ifndef int64_t
#define int64_t __int64
#endif

#ifdef MSG_MAXIOVLEN
#define         SPU_ARCH_SCATTER_SIZE       MSG_MAXIOVLEN
#else
#define         SPU_ARCH_SCATTER_SIZE       64
#endif


#endif /* SYSTEM_DEFS_WINDOWS_H */
