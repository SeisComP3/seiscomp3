
#ifndef SYSTEM_DEFS_H
#define SYSTEM_DEFS_H


#ifndef SPU_ARCH_PC_WIN95
/* For non Windows systems, use standard headers and types */
#include "spu_system_defs_autoconf.h"

#else
/* For Windows systems, use specified types and definitions */
#include "spu_system_defs_windows.h"

#endif /* SPU_ARCH_PC_WIN95 */


#endif /* SYSTEM_DEFS_H */
