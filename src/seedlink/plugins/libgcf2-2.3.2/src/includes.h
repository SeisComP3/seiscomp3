/*
 * ./src/includes.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: includes.h,v 1.1 2003/05/16 10:40:27 root Exp $
 */

/*
 * $Log: includes.h,v $
 * Revision 1.1  2003/05/16 10:40:27  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/04/15 11:04:46  root
 * #
 *
 * Revision 1.3  2003/04/01 18:00:18  root
 * #
 *
 * Revision 1.2  2003/04/01 17:54:54  root
 * #
 *
 * Revision 1.1  2003/04/01 17:52:13  root
 * #
 *
 */

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include "config.h"

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#elif defined(HAVE_SYS_INT_TYPES_H)
#include <sys/int_types.h>
#endif

typedef int64_t G2Offt;

#include "util.h"

#endif /* __INCLUDES_H__ */
