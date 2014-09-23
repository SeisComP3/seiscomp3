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



#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/utils/replace.h>
#include <boost/thread/mutex.hpp>

namespace {

boost::mutex cacheMutex;

}


namespace Seiscomp {
namespace DataModel {

namespace _private {

struct Resolver : public Util::VariableResolver {
	Resolver(PublicObject* po) : _po(po) {}

	bool resolve(std::string& variable) const {
		if ( Util::VariableResolver::resolve(variable) )
			return true;

		if ( variable == "classname" )
			variable = _po->className();
		else if ( variable == "id" )
			variable = Core::toString(PublicObject::_publicObjectId);
		else if ( variable == "globalid" )
			variable = Core::toString(Core::BaseObject::ObjectCount());
		else if ( !variable.compare(0, 4, "time", 4) ) {
			std::string::size_type seperator;
			seperator = variable.find('/');
			if ( seperator != std::string::npos )
				variable = Core::Time::GMT().toString(variable.substr(seperator+1).c_str());
			else
				variable = Core::toString(Core::Time::GMT());
		}
		else
			return false;

		return true;
	}

	PublicObject* _po;
};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(PublicObject,
                                    Object,
                                    "PublicObject");

PublicObject::PublicObjectMap PublicObject::_publicObjects;
bool PublicObject::_generateIds = false;
std::string PublicObject::_idPattern = "@classname@#@time/%Y%m%d%H%M%S.%f@.@id@";
unsigned long PublicObject::_publicObjectId = 0;
boost::thread_specific_ptr<bool> PublicObject::_registerObjects;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject::PublicObject()
 : _registered(false) {
	++_publicObjectId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject::PublicObject(const std::string& publicID)
 : Object(), _publicID(publicID), _registered(false) {
	++_publicObjectId;
	registerMe();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject::~PublicObject() {
	unregisterMe();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject& PublicObject::operator=(const PublicObject& other) {
	if ( this == &other ) return *this;
	if ( _publicID.empty() )
		_publicID = other._publicID;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::operator==(const PublicObject& rhs) const {
	return _publicID == rhs._publicID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::operator!=(const PublicObject& rhs) const {
	return _publicID != rhs._publicID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::registerMe() {
	if ( !IsRegistrationEnabled() )
		return true;

	if ( _publicID.empty() ) return false;

	boost::mutex::scoped_lock lk(cacheMutex);

	PublicObjectMap::iterator it = _publicObjects.find(_publicID);
	if ( it == _publicObjects.end() ) {
		_publicObjects[_publicID] = this;
		_registered = true;
		return true;
	}

	/*
	SEISCOMP_ERROR("object with publicID '%s' exists already => "
                   "setting publicID to ''", _publicID.c_str());
	_publicID = "";
	*/
	SEISCOMP_DEBUG("another object with publicID '%s' exists already",
                   _publicID.c_str());

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::unregisterMe() {
	if ( _publicID.empty() || !_registered )
		return false;

	boost::mutex::scoped_lock lk(cacheMutex);

	PublicObjectMap::iterator it = _publicObjects.find(_publicID);
	if ( it != _publicObjects.end() ) {
		_publicObjects.erase(it);
		_registered = false;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::registered() const {
	return _registered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& PublicObject::publicID() const {
	return _publicID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::setPublicID(const std::string &id) {
	unregisterMe();
	_publicID = id;
	return registerMe();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::validId() const {
	return !_publicID.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject* PublicObject::Find(const std::string& publicID) {
	PublicObjectMap::iterator it = _publicObjects.find(publicID);
	if ( it == _publicObjects.end() ) return NULL;
	return (*it).second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t PublicObject::ObjectCount() {
	return _publicObjects.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject::Iterator PublicObject::Begin() {
	return _publicObjects.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject::Iterator PublicObject::End() {
	return _publicObjects.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObject::SetIdGeneration(bool e) {
	_generateIds = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObject::SetIdPattern(const std::string& pattern) {
	_idPattern = pattern;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject* PublicObject::GenerateId(PublicObject* object) {
	if ( object == NULL ) return NULL;
	object->unregisterMe();
	object->generateId(_idPattern);
	object->registerMe();
	return object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject* PublicObject::GenerateId(PublicObject* object,
                                       const std::string &pattern) {
	if ( object == NULL ) return NULL;
	object->unregisterMe();
	object->generateId(pattern);
	object->registerMe();
	return object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObject::SetRegistrationEnabled(bool enable) {
	if ( _registerObjects.get() == NULL )
		// Store a new thread specific pointer value with 'enable'
		_registerObjects.reset(new bool(enable));
	else
		*_registerObjects.get() = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PublicObject::IsRegistrationEnabled() {
	if ( _registerObjects.get() == NULL ) {
		// Store a new thread specific pointer value with default: true
		bool *value = new bool(true);
		_registerObjects.reset(value);
	}

	return *_registerObjects.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObject::generateId(const std::string &pattern) {
	_publicID = Util::replace(pattern, _private::Resolver(this));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PublicObject::serialize(Archive& ar) {
	Object::serialize(ar);

	if ( ar.isReading() )
		unregisterMe();

	//ar & NAMED_OBJECT_HINT("publicID", _publicID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("publicID", _publicID);

	if ( ar.isReading() ) {
		if ( _publicID.empty() && _generateIds )
			generateId(_idPattern);

		/* Just register the publicID. If another object with the same
		 * publicID is already registered the publicID read before will
         * be set empty.
         */
		registerMe();

		/* To strict for the first version. I keep it in mind to enable it
		 * again after working some time with the objects.
		 * If enabled, all objects being read going to be ignored when
		 * another object with the same publicID already exits.
		ar.setValidity(registerMe());
		*/
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
