/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#ifndef __SEISCOMP_CORE_OSX_H__
#define __SEISCOMP_CORE_OSX_H__

/*
 *  Signal handling
 */
#include <signal.h>
#ifdef __APPLE__
	typedef void (*sighandler_t)(int);
#else
	#include <execinfo.h>
#endif

/*
 * Header files for malloc
 * On Mac OS X: malloc is defined in stdlib.h
 */
#ifdef __APPLE__
    #include <stdlib.h>
#else
    #include <malloc.h>
#endif


#endif
