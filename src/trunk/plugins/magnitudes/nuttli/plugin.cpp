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


#include <seiscomp3/core/plugin.h>
#include "version.h"


ADD_SC_PLUGIN(
	"MN (Nuttli) magnitude implementation",
	"gempa GmbH",
	MN_VERSION_MAJOR, MN_VERSION_MINOR, MN_VERSION_PATCH
)


