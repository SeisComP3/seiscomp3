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
#include <seiscomp3/datamodel/vs/envelope.h>
#include <seiscomp3/datamodel/vs/vs.h>
#include <seiscomp3/datamodel/vs/envelopechannel.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace VS {


IMPLEMENT_SC_CLASS_DERIVED(Envelope, PublicObject, "Envelope");


Envelope::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("network", "string", false, false, false, false, false, false, NULL, &Envelope::setNetwork, &Envelope::network));
	addProperty(Core::simpleProperty("station", "string", false, false, false, false, false, false, NULL, &Envelope::setStation, &Envelope::station));
	addProperty(Core::simpleProperty("timestamp", "datetime", false, false, false, false, false, false, NULL, &Envelope::setTimestamp, &Envelope::timestamp));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Envelope::setCreationInfo, &Envelope::creationInfo));
	addProperty(arrayObjectProperty("channel", "EnvelopeChannel", &Envelope::envelopeChannelCount, &Envelope::envelopeChannel, static_cast<bool (Envelope::*)(EnvelopeChannel*)>(&Envelope::add), &Envelope::removeEnvelopeChannel, static_cast<bool (Envelope::*)(EnvelopeChannel*)>(&Envelope::remove)));
}


IMPLEMENT_METAOBJECT(Envelope)


Envelope::Envelope() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::Envelope(const Envelope& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::Envelope(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope::~Envelope() {
	std::for_each(_envelopeChannels.begin(), _envelopeChannels.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&EnvelopeChannel::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&EnvelopeChannelPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope* Envelope::Create() {
	Envelope* object = new Envelope();
	return static_cast<Envelope*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope* Envelope::Create(const std::string& publicID) {
	if ( Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Envelope(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope* Envelope::Find(const std::string& publicID) {
	return Envelope::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::operator==(const Envelope& rhs) const {
	if ( _network != rhs._network ) return false;
	if ( _station != rhs._station ) return false;
	if ( _timestamp != rhs._timestamp ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::operator!=(const Envelope& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::equal(const Envelope& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::setNetwork(const std::string& network) {
	_network = network;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Envelope::network() const {
	return _network;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::setStation(const std::string& station) {
	_station = station;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Envelope::station() const {
	return _station;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::setTimestamp(Seiscomp::Core::Time timestamp) {
	_timestamp = timestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time Envelope::timestamp() const {
	return _timestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Envelope::creationInfo() throw(Seiscomp::Core::ValueException) {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Envelope.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Envelope::creationInfo() const throw(Seiscomp::Core::ValueException) {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Envelope.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VS* Envelope::vS() const {
	return static_cast<VS*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Envelope& Envelope::operator=(const Envelope& other) {
	PublicObject::operator=(other);
	_network = other._network;
	_station = other._station;
	_timestamp = other._timestamp;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::assign(Object* other) {
	Envelope* otherEnvelope = Envelope::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherEnvelope;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	VS* vS = VS::Cast(parent);
	if ( vS != NULL )
		return vS->add(this);

	SEISCOMP_ERROR("Envelope::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	VS* vS = VS::Cast(object);
	if ( vS != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return vS->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Envelope* child = vS->findEnvelope(publicID());
			if ( child != NULL )
				return vS->remove(child);
			else {
				SEISCOMP_DEBUG("Envelope::detachFrom(VS): envelope has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Envelope::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Envelope::clone() const {
	Envelope* clonee = new Envelope();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::updateChild(Object* child) {
	EnvelopeChannel* envelopeChannelChild = EnvelopeChannel::Cast(child);
	if ( envelopeChannelChild != NULL ) {
		EnvelopeChannel* envelopeChannelElement
			= EnvelopeChannel::Cast(PublicObject::Find(envelopeChannelChild->publicID()));
		if ( envelopeChannelElement && envelopeChannelElement->parent() == this ) {
			*envelopeChannelElement = *envelopeChannelChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<EnvelopeChannelPtr>::iterator it = _envelopeChannels.begin(); it != _envelopeChannels.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Envelope::envelopeChannelCount() const {
	return _envelopeChannels.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EnvelopeChannel* Envelope::envelopeChannel(size_t i) const {
	return _envelopeChannels[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EnvelopeChannel* Envelope::findEnvelopeChannel(const std::string& publicID) const {
	EnvelopeChannel* object = EnvelopeChannel::Cast(PublicObject::Find(publicID));
	if ( object != NULL && object->parent() == this )
		return object;
	
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::add(EnvelopeChannel* envelopeChannel) {
	if ( envelopeChannel == NULL )
		return false;

	// Element has already a parent
	if ( envelopeChannel->parent() != NULL ) {
		SEISCOMP_ERROR("Envelope::add(EnvelopeChannel*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		EnvelopeChannel* envelopeChannelCached = EnvelopeChannel::Find(envelopeChannel->publicID());
		if ( envelopeChannelCached ) {
			if ( envelopeChannelCached->parent() ) {
				if ( envelopeChannelCached->parent() == this )
					SEISCOMP_ERROR("Envelope::add(EnvelopeChannel*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Envelope::add(EnvelopeChannel*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				envelopeChannel = envelopeChannelCached;
		}
	}

	// Add the element
	_envelopeChannels.push_back(envelopeChannel);
	envelopeChannel->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		envelopeChannel->accept(&nc);
	}

	// Notify registered observers
	childAdded(envelopeChannel);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::remove(EnvelopeChannel* envelopeChannel) {
	if ( envelopeChannel == NULL )
		return false;

	if ( envelopeChannel->parent() != this ) {
		SEISCOMP_ERROR("Envelope::remove(EnvelopeChannel*) -> element has another parent");
		return false;
	}

	std::vector<EnvelopeChannelPtr>::iterator it;
	it = std::find(_envelopeChannels.begin(), _envelopeChannels.end(), envelopeChannel);
	// Element has not been found
	if ( it == _envelopeChannels.end() ) {
		SEISCOMP_ERROR("Envelope::remove(EnvelopeChannel*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_envelopeChannels.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Envelope::removeEnvelopeChannel(size_t i) {
	// index out of bounds
	if ( i >= _envelopeChannels.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_envelopeChannels[i]->accept(&nc);
	}

	_envelopeChannels[i]->setParent(NULL);
	childRemoved(_envelopeChannels[i].get());
	
	_envelopeChannels.erase(_envelopeChannels.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Envelope::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Envelope skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("network", _network, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("station", _station, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("timestamp", _timestamp, Archive::SPLIT_TIME | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("channel",
	                       Seiscomp::Core::Generic::containerMember(_envelopeChannels,
	                       Seiscomp::Core::Generic::bindMemberFunction<EnvelopeChannel>(static_cast<bool (Envelope::*)(EnvelopeChannel*)>(&Envelope::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
