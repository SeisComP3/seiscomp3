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


#include <seiscomp3/core/rtti.h>


namespace Seiscomp {
namespace Core {


RTTI::RTTI(const char* classname, const RTTI* parent)
	: _classname(classname), _parent(parent) {
}


const RTTI* RTTI::parent() const {
	return _parent;
}


const char* RTTI::className() const {
	return _classname.c_str();
}


bool RTTI::operator==(const RTTI& other) const {
	return this == &other;
}


bool RTTI::operator!=(const RTTI& other) const {
	return this != &other;
}


bool RTTI::before(const RTTI& other) const {
	const RTTI* parent = other.parent();
	while ( parent != NULL && parent != this )
		parent = parent->parent();

	return parent != NULL;
}


bool RTTI::isTypeOf(const RTTI& other) const {
	return (*this == other) || other.before(*this);
}


}
}
