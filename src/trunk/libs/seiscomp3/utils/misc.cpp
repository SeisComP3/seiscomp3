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


#include <seiscomp3/utils/misc.h>

namespace Seiscomp {
namespace Util {


char getShortPhaseName(const std::string &phase) {
	for ( std::string::const_reverse_iterator it = phase.rbegin();
	      it != phase.rend(); ++it ) {
		if ( isupper(*it ) )
			return *it;
	}

	return '\0';
}

template <class T, class A>
T join(const A &begin, const A &end, const T &glue) {
	T result;
	bool first = true;
	for ( A it = begin; it != end; ++it ) {
		if ( first )
			first = false;
		else
			result += glue;
		result += *it;
	}
	return result;
}

}
}
