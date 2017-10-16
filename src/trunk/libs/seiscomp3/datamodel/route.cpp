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
#include <seiscomp3/datamodel/route.h>
#include <seiscomp3/datamodel/routing.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Route, PublicObject, "Route");


Route::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("networkCode", "string", false, false, true, false, false, false, NULL, &Route::setNetworkCode, &Route::networkCode));
	addProperty(Core::simpleProperty("stationCode", "string", false, false, true, false, false, false, NULL, &Route::setStationCode, &Route::stationCode));
	addProperty(Core::simpleProperty("locationCode", "string", false, false, true, false, false, false, NULL, &Route::setLocationCode, &Route::locationCode));
	addProperty(Core::simpleProperty("streamCode", "string", false, false, true, false, false, false, NULL, &Route::setStreamCode, &Route::streamCode));
	addProperty(arrayClassProperty<RouteArclink>("arclink", "RouteArclink", &Route::routeArclinkCount, &Route::routeArclink, static_cast<bool (Route::*)(RouteArclink*)>(&Route::add), &Route::removeRouteArclink, static_cast<bool (Route::*)(RouteArclink*)>(&Route::remove)));
	addProperty(arrayClassProperty<RouteSeedlink>("seedlink", "RouteSeedlink", &Route::routeSeedlinkCount, &Route::routeSeedlink, static_cast<bool (Route::*)(RouteSeedlink*)>(&Route::add), &Route::removeRouteSeedlink, static_cast<bool (Route::*)(RouteSeedlink*)>(&Route::remove)));
}


IMPLEMENT_METAOBJECT(Route)


RouteIndex::RouteIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteIndex::RouteIndex(const std::string& networkCode_,
                       const std::string& stationCode_,
                       const std::string& locationCode_,
                       const std::string& streamCode_) {
	networkCode = networkCode_;
	stationCode = stationCode_;
	locationCode = locationCode_;
	streamCode = streamCode_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteIndex::RouteIndex(const RouteIndex& idx) {
	networkCode = idx.networkCode;
	stationCode = idx.stationCode;
	locationCode = idx.locationCode;
	streamCode = idx.streamCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RouteIndex::operator==(const RouteIndex& idx) const {
	return networkCode == idx.networkCode &&
	       stationCode == idx.stationCode &&
	       locationCode == idx.locationCode &&
	       streamCode == idx.streamCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RouteIndex::operator!=(const RouteIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route::Route() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route::Route(const Route& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route::Route(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route::~Route() {
	std::for_each(_routeArclinks.begin(), _routeArclinks.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&RouteArclink::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&RouteArclinkPtr::get)));
	std::for_each(_routeSeedlinks.begin(), _routeSeedlinks.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&RouteSeedlink::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&RouteSeedlinkPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Route::Create() {
	Route* object = new Route();
	return static_cast<Route*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Route::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Route(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route* Route::Find(const std::string& publicID) {
	return Route::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::operator==(const Route& rhs) const {
	if ( _index != rhs._index ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::operator!=(const Route& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::equal(const Route& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::setNetworkCode(const std::string& networkCode) {
	_index.networkCode = networkCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Route::networkCode() const {
	return _index.networkCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::setStationCode(const std::string& stationCode) {
	_index.stationCode = stationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Route::stationCode() const {
	return _index.stationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::setLocationCode(const std::string& locationCode) {
	_index.locationCode = locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Route::locationCode() const {
	return _index.locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::setStreamCode(const std::string& streamCode) {
	_index.streamCode = streamCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Route::streamCode() const {
	return _index.streamCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RouteIndex& Route::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::equalIndex(const Route* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing* Route::routing() const {
	return static_cast<Routing*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route& Route::operator=(const Route& other) {
	PublicObject::operator=(other);
	_index = other._index;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::assign(Object* other) {
	Route* otherRoute = Route::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherRoute;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Routing* routing = Routing::Cast(parent);
	if ( routing != NULL )
		return routing->add(this);

	SEISCOMP_ERROR("Route::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	Routing* routing = Routing::Cast(object);
	if ( routing != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return routing->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Route* child = routing->findRoute(publicID());
			if ( child != NULL )
				return routing->remove(child);
			else {
				SEISCOMP_DEBUG("Route::detachFrom(Routing): route has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Route::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Route::clone() const {
	Route* clonee = new Route();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::updateChild(Object* child) {
	RouteArclink* routeArclinkChild = RouteArclink::Cast(child);
	if ( routeArclinkChild != NULL ) {
		RouteArclink* routeArclinkElement = routeArclink(routeArclinkChild->index());
		if ( routeArclinkElement != NULL ) {
			*routeArclinkElement = *routeArclinkChild;
			return true;
		}
		return false;
	}

	RouteSeedlink* routeSeedlinkChild = RouteSeedlink::Cast(child);
	if ( routeSeedlinkChild != NULL ) {
		RouteSeedlink* routeSeedlinkElement = routeSeedlink(routeSeedlinkChild->index());
		if ( routeSeedlinkElement != NULL ) {
			*routeSeedlinkElement = *routeSeedlinkChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<RouteArclinkPtr>::iterator it = _routeArclinks.begin(); it != _routeArclinks.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<RouteSeedlinkPtr>::iterator it = _routeSeedlinks.begin(); it != _routeSeedlinks.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Route::routeArclinkCount() const {
	return _routeArclinks.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteArclink* Route::routeArclink(size_t i) const {
	return _routeArclinks[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteArclink* Route::routeArclink(const RouteArclinkIndex& i) const {
	for ( std::vector<RouteArclinkPtr>::const_iterator it = _routeArclinks.begin(); it != _routeArclinks.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::add(RouteArclink* routeArclink) {
	if ( routeArclink == NULL )
		return false;

	// Element has already a parent
	if ( routeArclink->parent() != NULL ) {
		SEISCOMP_ERROR("Route::add(RouteArclink*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<RouteArclinkPtr>::iterator it = _routeArclinks.begin(); it != _routeArclinks.end(); ++it ) {
		if ( (*it)->index() == routeArclink->index() ) {
			SEISCOMP_ERROR("Route::add(RouteArclink*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_routeArclinks.push_back(routeArclink);
	routeArclink->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		routeArclink->accept(&nc);
	}

	// Notify registered observers
	childAdded(routeArclink);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::remove(RouteArclink* routeArclink) {
	if ( routeArclink == NULL )
		return false;

	if ( routeArclink->parent() != this ) {
		SEISCOMP_ERROR("Route::remove(RouteArclink*) -> element has another parent");
		return false;
	}

	std::vector<RouteArclinkPtr>::iterator it;
	it = std::find(_routeArclinks.begin(), _routeArclinks.end(), routeArclink);
	// Element has not been found
	if ( it == _routeArclinks.end() ) {
		SEISCOMP_ERROR("Route::remove(RouteArclink*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_routeArclinks.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::removeRouteArclink(size_t i) {
	// index out of bounds
	if ( i >= _routeArclinks.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_routeArclinks[i]->accept(&nc);
	}

	_routeArclinks[i]->setParent(NULL);
	childRemoved(_routeArclinks[i].get());
	
	_routeArclinks.erase(_routeArclinks.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::removeRouteArclink(const RouteArclinkIndex& i) {
	RouteArclink* object = routeArclink(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Route::routeSeedlinkCount() const {
	return _routeSeedlinks.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteSeedlink* Route::routeSeedlink(size_t i) const {
	return _routeSeedlinks[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouteSeedlink* Route::routeSeedlink(const RouteSeedlinkIndex& i) const {
	for ( std::vector<RouteSeedlinkPtr>::const_iterator it = _routeSeedlinks.begin(); it != _routeSeedlinks.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::add(RouteSeedlink* routeSeedlink) {
	if ( routeSeedlink == NULL )
		return false;

	// Element has already a parent
	if ( routeSeedlink->parent() != NULL ) {
		SEISCOMP_ERROR("Route::add(RouteSeedlink*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<RouteSeedlinkPtr>::iterator it = _routeSeedlinks.begin(); it != _routeSeedlinks.end(); ++it ) {
		if ( (*it)->index() == routeSeedlink->index() ) {
			SEISCOMP_ERROR("Route::add(RouteSeedlink*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_routeSeedlinks.push_back(routeSeedlink);
	routeSeedlink->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		routeSeedlink->accept(&nc);
	}

	// Notify registered observers
	childAdded(routeSeedlink);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::remove(RouteSeedlink* routeSeedlink) {
	if ( routeSeedlink == NULL )
		return false;

	if ( routeSeedlink->parent() != this ) {
		SEISCOMP_ERROR("Route::remove(RouteSeedlink*) -> element has another parent");
		return false;
	}

	std::vector<RouteSeedlinkPtr>::iterator it;
	it = std::find(_routeSeedlinks.begin(), _routeSeedlinks.end(), routeSeedlink);
	// Element has not been found
	if ( it == _routeSeedlinks.end() ) {
		SEISCOMP_ERROR("Route::remove(RouteSeedlink*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_routeSeedlinks.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::removeRouteSeedlink(size_t i) {
	// index out of bounds
	if ( i >= _routeSeedlinks.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_routeSeedlinks[i]->accept(&nc);
	}

	_routeSeedlinks[i]->setParent(NULL);
	childRemoved(_routeSeedlinks[i].get());
	
	_routeSeedlinks.erase(_routeSeedlinks.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Route::removeRouteSeedlink(const RouteSeedlinkIndex& i) {
	RouteSeedlink* object = routeSeedlink(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Route::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Route skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("networkCode", _index.networkCode, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("stationCode", _index.stationCode, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("locationCode", _index.locationCode, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("streamCode", _index.streamCode, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("arclink",
	                       Seiscomp::Core::Generic::containerMember(_routeArclinks,
	                       Seiscomp::Core::Generic::bindMemberFunction<RouteArclink>(static_cast<bool (Route::*)(RouteArclink*)>(&Route::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("seedlink",
	                       Seiscomp::Core::Generic::containerMember(_routeSeedlinks,
	                       Seiscomp::Core::Generic::bindMemberFunction<RouteSeedlink>(static_cast<bool (Route::*)(RouteSeedlink*)>(&Route::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
