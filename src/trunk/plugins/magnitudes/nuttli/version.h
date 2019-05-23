/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_CUSTOM_AMPLITUDE_VERSION_H__
#define __SEISCOMP_CUSTOM_AMPLITUDE_VERSION_H__


#define MN_VERSION_MAJOR 0
#define MN_VERSION_MINOR 2
#define MN_VERSION_PATCH 0

#define MN_STR_HELPER(x) #x
#define MN_STR(x) MN_STR_HELPER(x)
#define MN_VERSION (MN_STR(MN_VERSION_MAJOR) "." MN_STR(MN_VERSION_MINOR) "." MN_STR(MN_VERSION_PATCH))

#endif
