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


#include <seiscomp3/core/array.h>

namespace Seiscomp {


IMPLEMENT_SC_ABSTRACT_CLASS(Array, "Array");


Array::Array(DataType dt): _datatype(dt) {
}

Array::~Array() {
}

Array* Array::clone() const {
	return copy(dataType());
}

std::string Array::str() const {
	return std::string(static_cast<const char*>(data()), size()*bytes());
}


}

