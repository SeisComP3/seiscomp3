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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/routing.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Routing, PublicObject, "Routing");


Routing::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("route", "Route", &Routing::routeCount, &Routing::route, static_cast<bool (Routing::*)(Route*)>(&Routing::add), &Routing::removeRoute, static_cast<bool (Routing::*)(Route*)>(&Routing::remove)));
	addProperty(arrayClassProperty<Access>("access", "Access", &Routing::accessCount, &Routing::access, static_cast<bool (Routing::*)(Access*)>(&Routing::add), &Routing::removeAccess, static_cast<bool (Routing::*)(Access*)>(&Routing::remove)));
}


IMPLEMENT_METAOBJECT(Routing)


Routing::Routing() : PublicObject("Routing") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing::Routing(const Routing& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing::~Routing() {
	std::for_each(_routes.begin(), _routes.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Route::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&RoutePtr::get)));
	std::for_each(_accesss.begin(), _accesss.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Access::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&AccessPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::operator==(const Routing& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::operator!=(const Routing& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::equal(const Routing& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing& Routing::operator=(const Routing& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::assign(Object* other) {
	Routing* otherRouting = Routing::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherRouting;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Routing::clone() const {
	Routing* clonee = new Routing();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::updateChild(Object* child) {
	Route* routeChild = Route::Cast(child);
	if ( routeChild != NULL ) {
		Route* routeElement
			= Route::Cast(PublicObject::Find(routeChild->publicID()));
		if ( routeElement && routeElement->parent() == this ) {
			*routeElement = *routeChild;
			return true;
		}
		return false;
	}

	Access* accessChild = Access::Cast(child);
	if ( accessChild != NULL ) {
		Access* accessElement = access(accessChild->index());
		if ( accessElement != NULL ) {
			*accessElement = *accessChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Routing::accept(Visitor* visitor) {
	for ( std::vector<RoutePtr>::iterator it = _routes.begin(); it != _routes.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<AccessPtr>::iterator it = _accesss.begin(); it != _accesss.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Routing::routeCount() const {
	return _routes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Routing::route(size_t i) const {
	return _routes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Routing::route(const RouteIndex& i) const {
	for ( std::vector<RoutePtr>::const_iterator it = _routes.begin(); it != _routes.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Routing::findRoute(const std::string& publicID) const {
	for ( std::vector<RoutePtr>::const_iterator it = _routes.begin(); it != _routes.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::add(Route* route) {
	if ( route == NULL )
		return false;

	// Element has already a parent
	if ( route->parent() != NULL ) {
		SEISCOMP_ERROR("Routing::add(Route*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Route* routeCached = Route::Find(route->publicID());
		if ( routeCached ) {
			if ( routeCached->parent() ) {
				if ( routeCached->parent() == this )
					SEISCOMP_ERROR("Routing::add(Route*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Routing::add(Route*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				route = routeCached;
		}
	}

	// Add the element
	_routes.push_back(route);
	route->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		route->accept(&nc);
	}

	// Notify registered observers
	childAdded(route);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::remove(Route* route) {
	if ( route == NULL )
		return false;

	if ( route->parent() != this ) {
		SEISCOMP_ERROR("Routing::remove(Route*) -> element has another parent");
		return false;
	}

	std::vector<RoutePtr>::iterator it;
	it = std::find(_routes.begin(), _routes.end(), route);
	// Element has not been found
	if ( it == _routes.end() ) {
		SEISCOMP_ERROR("Routing::remove(Route*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_routes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeRoute(size_t i) {
	// index out of bounds
	if ( i >= _routes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_routes[i]->accept(&nc);
	}

	_routes[i]->setParent(NULL);
	childRemoved(_routes[i].get());
	
	_routes.erase(_routes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeRoute(const RouteIndex& i) {
	Route* object = route(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Routing::accessCount() const {
	return _accesss.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Access* Routing::access(size_t i) const {
	return _accesss[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Access* Routing::access(const AccessIndex& i) const {
	for ( std::vector<AccessPtr>::const_iterator it = _accesss.begin(); it != _accesss.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::add(Access* access) {
	if ( access == NULL )
		return false;

	// Element has already a parent
	if ( access->parent() != NULL ) {
		SEISCOMP_ERROR("Routing::add(Access*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<AccessPtr>::iterator it = _accesss.begin(); it != _accesss.end(); ++it ) {
		if ( (*it)->index() == access->index() ) {
			SEISCOMP_ERROR("Routing::add(Access*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_accesss.push_back(access);
	access->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		access->accept(&nc);
	}

	// Notify registered observers
	childAdded(access);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::remove(Access* access) {
	if ( access == NULL )
		return false;

	if ( access->parent() != this ) {
		SEISCOMP_ERROR("Routing::remove(Access*) -> element has another parent");
		return false;
	}

	std::vector<AccessPtr>::iterator it;
	it = std::find(_accesss.begin(), _accesss.end(), access);
	// Element has not been found
	if ( it == _accesss.end() ) {
		SEISCOMP_ERROR("Routing::remove(Access*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_accesss.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeAccess(size_t i) {
	// index out of bounds
	if ( i >= _accesss.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_accesss[i]->accept(&nc);
	}

	_accesss[i]->setParent(NULL);
	childRemoved(_accesss[i].get());
	
	_accesss.erase(_accesss.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeAccess(const AccessIndex& i) {
	Access* object = access(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Routing::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Routing skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("route",
	                       Seiscomp::Core::Generic::containerMember(_routes,
	                       Seiscomp::Core::Generic::bindMemberFunction<Route>(static_cast<bool (Routing::*)(Route*)>(&Routing::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("access",
	                       Seiscomp::Core::Generic::containerMember(_accesss,
	                       Seiscomp::Core::Generic::bindMemberFunction<Access>(static_cast<bool (Routing::*)(Access*)>(&Routing::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
