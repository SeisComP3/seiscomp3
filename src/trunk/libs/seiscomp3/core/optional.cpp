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


#include <seiscomp3/core/optional.h>
#include <seiscomp3/core/datetime.h>

// Explicit template instantiation of some optional types
template class boost::optional<bool>;
template class boost::optional<int>;
template class boost::optional<float>;
template class boost::optional<double>;
template class boost::optional<Seiscomp::Core::Time>;


namespace Seiscomp {
namespace Core {


::boost::none_t const None = ::boost::none;

ValueError::ValueError() throw() {
}

ValueError::~ValueError() throw() {
}

const char* ValueError::what() const throw() {
	return "requested value has not been set";
}


}
}
