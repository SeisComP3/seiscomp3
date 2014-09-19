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

#ifndef __SEISCOMP_UTILS_MISC__
#define __SEISCOMP_UTILS_MISC__

#include <string>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace Util {


SC_SYSTEM_CORE_API char getShortPhaseName(const std::string &phase);

template <class T, class A>
T join(const A &begin, const A &end, const T &glue);

}
}


#endif
