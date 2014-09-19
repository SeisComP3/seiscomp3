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

#include <seiscomp3/core/exceptions.h>

namespace Seiscomp {
namespace Core {
namespace Generic {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>::ClassFactoryInterface(const RTTI* typeInfo) {
	_typeInfo = typeInfo;

	// while construction the interface will be added
	// to the classpool
	RegisterFactory(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>::~ClassFactoryInterface() {
	// remove the Factory from the classpool
	UnregisterFactory(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ROOT_TYPE* ClassFactoryInterface<ROOT_TYPE>::Create(const char* className) {
	ClassFactoryInterface<ROOT_TYPE>* factoryInterface = FindByClassName(className);
	if ( factoryInterface != NULL )
		return factoryInterface->create();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::IsTypeOf(const char* baseName, const char* derivedName) {
	ClassFactoryInterface<ROOT_TYPE>* derivedFactory = FindByClassName(derivedName);
	if ( derivedFactory == NULL )
		return false;

	ClassFactoryInterface<ROOT_TYPE>* baseFactory = FindByClassName(baseName);
	if ( baseFactory == NULL )
		return false;

	return derivedFactory->typeInfo()->isTypeOf(*baseFactory->typeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ROOT_TYPE* ClassFactoryInterface<ROOT_TYPE>::Create(const std::string& className) {
	return Create(className.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::ClassName(const ROOT_TYPE* object) {
	ClassNames::iterator it = Names().find(&object->typeInfo());
	return it == Names().end() ? NULL : (*it).second.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::ClassName(const RTTI* info) {
	ClassNames::iterator it = Names().find(info);
	return it == Names().end() ? NULL : (*it).second.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>* ClassFactoryInterface<ROOT_TYPE>::FindByClassName(const char* className) {
	if ( className == NULL )
		return NULL;

	typename ClassPool::iterator it = Classes().find(className);
	if ( it == Classes().end() )
		return NULL;

	return (*it).second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
unsigned int ClassFactoryInterface<ROOT_TYPE>::NumberOfRegisteredClasses() {
	return static_cast<unsigned int>(Classes().size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::className() const {
	return _typeInfo->className();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const RTTI* ClassFactoryInterface<ROOT_TYPE>::typeInfo() const {
	return _typeInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
typename ClassFactoryInterface<ROOT_TYPE>::ClassPool& ClassFactoryInterface<ROOT_TYPE>::Classes() {
    static typename ClassFactoryInterface<ROOT_TYPE>::ClassPool* classes = new typename ClassFactoryInterface<ROOT_TYPE>::ClassPool;
    return *classes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
typename ClassFactoryInterface<ROOT_TYPE>::ClassNames& ClassFactoryInterface<ROOT_TYPE>::Names() {
    static typename ClassFactoryInterface<ROOT_TYPE>::ClassNames* names = new typename ClassFactoryInterface<ROOT_TYPE>::ClassNames;
    return *names;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::RegisterFactory(ClassFactoryInterface<ROOT_TYPE>* factory) {
	if ( factory == NULL )
		return false;

	if ( Classes().find(factory->className()) != Classes().end() ) {
		throw DuplicateClassname(factory->className());
		return false;
	}

	Classes()[factory->className()] = factory;
	Names()[factory->typeInfo()] = factory->className();
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::UnregisterFactory(ClassFactoryInterface<ROOT_TYPE>* factory) {
	if ( factory == NULL )
		return false;

	typename ClassPool::iterator it = Classes().find(factory->className());
	if ( it == Classes().end() )
		// the factory has not been registered already
		return false;

	Classes().erase(it);

	typename ClassNames::iterator it_names = Names().find(factory->typeInfo());
	if ( it_names != Names().end() )
		Names().erase(it_names);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
