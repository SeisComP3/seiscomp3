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


#define SEISCOMP_COMPONENT Core

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/archive.ipp>
#include <seiscomp3/core/factory.ipp>

namespace Seiscomp {
namespace Core {

volatile unsigned int BaseObject::_objectCount = 0;

IMPLEMENT_CLASSFACTORY(BaseObject, SC_SYSTEM_CORE_API);
IMPLEMENT_ROOT_RTTI(BaseObject, "BaseObject")
IMPLEMENT_RTTI_METHODS(BaseObject)
IMPLEMENT_METAOBJECT_EMPTY_METHODS(BaseObject)
REGISTER_ABSTRACT_CLASS(BaseObject, BaseObject);

template class SC_SYSTEM_CORE_API Generic::Archive<Seiscomp::Core::BaseObject>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseObject::BaseObject() : _referenceCount(0) {	
    ++_objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	/*
	const Seiscomp::Core::MetaObject *BaseObject::Meta() {
		return NULL;
	}

	const Seiscomp::Core::MetaObject *BaseObject::meta() const {
		return NULL;
	}
	*/
BaseObject::BaseObject(const BaseObject&) : _referenceCount(0) {
    ++_objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseObject::~BaseObject() {
	//SEISCOMP_DEBUG("~BaseObject called");
	--_objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseObject* BaseObject::clone() const {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BaseObject& BaseObject::operator=(const BaseObject&) {
	// Prevent the copying of the referenceCount
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
