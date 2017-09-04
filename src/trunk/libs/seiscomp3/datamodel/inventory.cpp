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
#include <seiscomp3/datamodel/inventory.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Inventory, PublicObject, "Inventory");


Inventory::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("stationGroup", "StationGroup", &Inventory::stationGroupCount, &Inventory::stationGroup, static_cast<bool (Inventory::*)(StationGroup*)>(&Inventory::add), &Inventory::removeStationGroup, static_cast<bool (Inventory::*)(StationGroup*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("auxDevice", "AuxDevice", &Inventory::auxDeviceCount, &Inventory::auxDevice, static_cast<bool (Inventory::*)(AuxDevice*)>(&Inventory::add), &Inventory::removeAuxDevice, static_cast<bool (Inventory::*)(AuxDevice*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("sensor", "Sensor", &Inventory::sensorCount, &Inventory::sensor, static_cast<bool (Inventory::*)(Sensor*)>(&Inventory::add), &Inventory::removeSensor, static_cast<bool (Inventory::*)(Sensor*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("datalogger", "Datalogger", &Inventory::dataloggerCount, &Inventory::datalogger, static_cast<bool (Inventory::*)(Datalogger*)>(&Inventory::add), &Inventory::removeDatalogger, static_cast<bool (Inventory::*)(Datalogger*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("responsePAZ", "ResponsePAZ", &Inventory::responsePAZCount, &Inventory::responsePAZ, static_cast<bool (Inventory::*)(ResponsePAZ*)>(&Inventory::add), &Inventory::removeResponsePAZ, static_cast<bool (Inventory::*)(ResponsePAZ*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("responseFIR", "ResponseFIR", &Inventory::responseFIRCount, &Inventory::responseFIR, static_cast<bool (Inventory::*)(ResponseFIR*)>(&Inventory::add), &Inventory::removeResponseFIR, static_cast<bool (Inventory::*)(ResponseFIR*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("responseIIR", "ResponseIIR", &Inventory::responseIIRCount, &Inventory::responseIIR, static_cast<bool (Inventory::*)(ResponseIIR*)>(&Inventory::add), &Inventory::removeResponseIIR, static_cast<bool (Inventory::*)(ResponseIIR*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("responsePolynomial", "ResponsePolynomial", &Inventory::responsePolynomialCount, &Inventory::responsePolynomial, static_cast<bool (Inventory::*)(ResponsePolynomial*)>(&Inventory::add), &Inventory::removeResponsePolynomial, static_cast<bool (Inventory::*)(ResponsePolynomial*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("responseFAP", "ResponseFAP", &Inventory::responseFAPCount, &Inventory::responseFAP, static_cast<bool (Inventory::*)(ResponseFAP*)>(&Inventory::add), &Inventory::removeResponseFAP, static_cast<bool (Inventory::*)(ResponseFAP*)>(&Inventory::remove)));
	addProperty(arrayObjectProperty("network", "Network", &Inventory::networkCount, &Inventory::network, static_cast<bool (Inventory::*)(Network*)>(&Inventory::add), &Inventory::removeNetwork, static_cast<bool (Inventory::*)(Network*)>(&Inventory::remove)));
}


IMPLEMENT_METAOBJECT(Inventory)


Inventory::Inventory() : PublicObject("Inventory") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::Inventory(const Inventory& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::~Inventory() {
	std::for_each(_stationGroups.begin(), _stationGroups.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&StationGroup::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&StationGroupPtr::get)));
	std::for_each(_auxDevices.begin(), _auxDevices.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&AuxDevice::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&AuxDevicePtr::get)));
	std::for_each(_sensors.begin(), _sensors.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Sensor::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&SensorPtr::get)));
	std::for_each(_dataloggers.begin(), _dataloggers.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Datalogger::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&DataloggerPtr::get)));
	std::for_each(_responsePAZs.begin(), _responsePAZs.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ResponsePAZ::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ResponsePAZPtr::get)));
	std::for_each(_responseFIRs.begin(), _responseFIRs.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ResponseFIR::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ResponseFIRPtr::get)));
	std::for_each(_responseIIRs.begin(), _responseIIRs.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ResponseIIR::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ResponseIIRPtr::get)));
	std::for_each(_responsePolynomials.begin(), _responsePolynomials.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ResponsePolynomial::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ResponsePolynomialPtr::get)));
	std::for_each(_responseFAPs.begin(), _responseFAPs.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ResponseFAP::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ResponseFAPPtr::get)));
	std::for_each(_networks.begin(), _networks.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Network::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&NetworkPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::operator==(const Inventory& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::operator!=(const Inventory& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::equal(const Inventory& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory& Inventory::operator=(const Inventory& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::assign(Object* other) {
	Inventory* otherInventory = Inventory::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherInventory;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Inventory::clone() const {
	Inventory* clonee = new Inventory();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::updateChild(Object* child) {
	StationGroup* stationGroupChild = StationGroup::Cast(child);
	if ( stationGroupChild != NULL ) {
		StationGroup* stationGroupElement
			= StationGroup::Cast(PublicObject::Find(stationGroupChild->publicID()));
		if ( stationGroupElement && stationGroupElement->parent() == this ) {
			*stationGroupElement = *stationGroupChild;
			return true;
		}
		return false;
	}

	AuxDevice* auxDeviceChild = AuxDevice::Cast(child);
	if ( auxDeviceChild != NULL ) {
		AuxDevice* auxDeviceElement
			= AuxDevice::Cast(PublicObject::Find(auxDeviceChild->publicID()));
		if ( auxDeviceElement && auxDeviceElement->parent() == this ) {
			*auxDeviceElement = *auxDeviceChild;
			return true;
		}
		return false;
	}

	Sensor* sensorChild = Sensor::Cast(child);
	if ( sensorChild != NULL ) {
		Sensor* sensorElement
			= Sensor::Cast(PublicObject::Find(sensorChild->publicID()));
		if ( sensorElement && sensorElement->parent() == this ) {
			*sensorElement = *sensorChild;
			return true;
		}
		return false;
	}

	Datalogger* dataloggerChild = Datalogger::Cast(child);
	if ( dataloggerChild != NULL ) {
		Datalogger* dataloggerElement
			= Datalogger::Cast(PublicObject::Find(dataloggerChild->publicID()));
		if ( dataloggerElement && dataloggerElement->parent() == this ) {
			*dataloggerElement = *dataloggerChild;
			return true;
		}
		return false;
	}

	ResponsePAZ* responsePAZChild = ResponsePAZ::Cast(child);
	if ( responsePAZChild != NULL ) {
		ResponsePAZ* responsePAZElement
			= ResponsePAZ::Cast(PublicObject::Find(responsePAZChild->publicID()));
		if ( responsePAZElement && responsePAZElement->parent() == this ) {
			*responsePAZElement = *responsePAZChild;
			return true;
		}
		return false;
	}

	ResponseFIR* responseFIRChild = ResponseFIR::Cast(child);
	if ( responseFIRChild != NULL ) {
		ResponseFIR* responseFIRElement
			= ResponseFIR::Cast(PublicObject::Find(responseFIRChild->publicID()));
		if ( responseFIRElement && responseFIRElement->parent() == this ) {
			*responseFIRElement = *responseFIRChild;
			return true;
		}
		return false;
	}

	ResponseIIR* responseIIRChild = ResponseIIR::Cast(child);
	if ( responseIIRChild != NULL ) {
		ResponseIIR* responseIIRElement
			= ResponseIIR::Cast(PublicObject::Find(responseIIRChild->publicID()));
		if ( responseIIRElement && responseIIRElement->parent() == this ) {
			*responseIIRElement = *responseIIRChild;
			return true;
		}
		return false;
	}

	ResponsePolynomial* responsePolynomialChild = ResponsePolynomial::Cast(child);
	if ( responsePolynomialChild != NULL ) {
		ResponsePolynomial* responsePolynomialElement
			= ResponsePolynomial::Cast(PublicObject::Find(responsePolynomialChild->publicID()));
		if ( responsePolynomialElement && responsePolynomialElement->parent() == this ) {
			*responsePolynomialElement = *responsePolynomialChild;
			return true;
		}
		return false;
	}

	ResponseFAP* responseFAPChild = ResponseFAP::Cast(child);
	if ( responseFAPChild != NULL ) {
		ResponseFAP* responseFAPElement
			= ResponseFAP::Cast(PublicObject::Find(responseFAPChild->publicID()));
		if ( responseFAPElement && responseFAPElement->parent() == this ) {
			*responseFAPElement = *responseFAPChild;
			return true;
		}
		return false;
	}

	Network* networkChild = Network::Cast(child);
	if ( networkChild != NULL ) {
		Network* networkElement
			= Network::Cast(PublicObject::Find(networkChild->publicID()));
		if ( networkElement && networkElement->parent() == this ) {
			*networkElement = *networkChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::accept(Visitor* visitor) {
	for ( std::vector<StationGroupPtr>::iterator it = _stationGroups.begin(); it != _stationGroups.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<AuxDevicePtr>::iterator it = _auxDevices.begin(); it != _auxDevices.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<SensorPtr>::iterator it = _sensors.begin(); it != _sensors.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<DataloggerPtr>::iterator it = _dataloggers.begin(); it != _dataloggers.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ResponsePAZPtr>::iterator it = _responsePAZs.begin(); it != _responsePAZs.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ResponseFIRPtr>::iterator it = _responseFIRs.begin(); it != _responseFIRs.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ResponseIIRPtr>::iterator it = _responseIIRs.begin(); it != _responseIIRs.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ResponsePolynomialPtr>::iterator it = _responsePolynomials.begin(); it != _responsePolynomials.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ResponseFAPPtr>::iterator it = _responseFAPs.begin(); it != _responseFAPs.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<NetworkPtr>::iterator it = _networks.begin(); it != _networks.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::stationGroupCount() const {
	return _stationGroups.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationGroup* Inventory::stationGroup(size_t i) const {
	return _stationGroups[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationGroup* Inventory::stationGroup(const StationGroupIndex& i) const {
	for ( std::vector<StationGroupPtr>::const_iterator it = _stationGroups.begin(); it != _stationGroups.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationGroup* Inventory::findStationGroup(const std::string& publicID) const {
	for ( std::vector<StationGroupPtr>::const_iterator it = _stationGroups.begin(); it != _stationGroups.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(StationGroup* stationGroup) {
	if ( stationGroup == NULL )
		return false;

	// Element has already a parent
	if ( stationGroup->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(StationGroup*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		StationGroup* stationGroupCached = StationGroup::Find(stationGroup->publicID());
		if ( stationGroupCached ) {
			if ( stationGroupCached->parent() ) {
				if ( stationGroupCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(StationGroup*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(StationGroup*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				stationGroup = stationGroupCached;
		}
	}

	// Add the element
	_stationGroups.push_back(stationGroup);
	stationGroup->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		stationGroup->accept(&nc);
	}

	// Notify registered observers
	childAdded(stationGroup);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(StationGroup* stationGroup) {
	if ( stationGroup == NULL )
		return false;

	if ( stationGroup->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(StationGroup*) -> element has another parent");
		return false;
	}

	std::vector<StationGroupPtr>::iterator it;
	it = std::find(_stationGroups.begin(), _stationGroups.end(), stationGroup);
	// Element has not been found
	if ( it == _stationGroups.end() ) {
		SEISCOMP_ERROR("Inventory::remove(StationGroup*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_stationGroups.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeStationGroup(size_t i) {
	// index out of bounds
	if ( i >= _stationGroups.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_stationGroups[i]->accept(&nc);
	}

	_stationGroups[i]->setParent(NULL);
	childRemoved(_stationGroups[i].get());
	
	_stationGroups.erase(_stationGroups.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeStationGroup(const StationGroupIndex& i) {
	StationGroup* object = stationGroup(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::auxDeviceCount() const {
	return _auxDevices.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxDevice* Inventory::auxDevice(size_t i) const {
	return _auxDevices[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxDevice* Inventory::auxDevice(const AuxDeviceIndex& i) const {
	for ( std::vector<AuxDevicePtr>::const_iterator it = _auxDevices.begin(); it != _auxDevices.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxDevice* Inventory::findAuxDevice(const std::string& publicID) const {
	for ( std::vector<AuxDevicePtr>::const_iterator it = _auxDevices.begin(); it != _auxDevices.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(AuxDevice* auxDevice) {
	if ( auxDevice == NULL )
		return false;

	// Element has already a parent
	if ( auxDevice->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(AuxDevice*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		AuxDevice* auxDeviceCached = AuxDevice::Find(auxDevice->publicID());
		if ( auxDeviceCached ) {
			if ( auxDeviceCached->parent() ) {
				if ( auxDeviceCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(AuxDevice*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(AuxDevice*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				auxDevice = auxDeviceCached;
		}
	}

	// Add the element
	_auxDevices.push_back(auxDevice);
	auxDevice->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		auxDevice->accept(&nc);
	}

	// Notify registered observers
	childAdded(auxDevice);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(AuxDevice* auxDevice) {
	if ( auxDevice == NULL )
		return false;

	if ( auxDevice->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(AuxDevice*) -> element has another parent");
		return false;
	}

	std::vector<AuxDevicePtr>::iterator it;
	it = std::find(_auxDevices.begin(), _auxDevices.end(), auxDevice);
	// Element has not been found
	if ( it == _auxDevices.end() ) {
		SEISCOMP_ERROR("Inventory::remove(AuxDevice*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_auxDevices.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeAuxDevice(size_t i) {
	// index out of bounds
	if ( i >= _auxDevices.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_auxDevices[i]->accept(&nc);
	}

	_auxDevices[i]->setParent(NULL);
	childRemoved(_auxDevices[i].get());
	
	_auxDevices.erase(_auxDevices.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeAuxDevice(const AuxDeviceIndex& i) {
	AuxDevice* object = auxDevice(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::sensorCount() const {
	return _sensors.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor* Inventory::sensor(size_t i) const {
	return _sensors[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor* Inventory::sensor(const SensorIndex& i) const {
	for ( std::vector<SensorPtr>::const_iterator it = _sensors.begin(); it != _sensors.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor* Inventory::findSensor(const std::string& publicID) const {
	for ( std::vector<SensorPtr>::const_iterator it = _sensors.begin(); it != _sensors.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(Sensor* sensor) {
	if ( sensor == NULL )
		return false;

	// Element has already a parent
	if ( sensor->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(Sensor*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Sensor* sensorCached = Sensor::Find(sensor->publicID());
		if ( sensorCached ) {
			if ( sensorCached->parent() ) {
				if ( sensorCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(Sensor*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(Sensor*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				sensor = sensorCached;
		}
	}

	// Add the element
	_sensors.push_back(sensor);
	sensor->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		sensor->accept(&nc);
	}

	// Notify registered observers
	childAdded(sensor);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(Sensor* sensor) {
	if ( sensor == NULL )
		return false;

	if ( sensor->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(Sensor*) -> element has another parent");
		return false;
	}

	std::vector<SensorPtr>::iterator it;
	it = std::find(_sensors.begin(), _sensors.end(), sensor);
	// Element has not been found
	if ( it == _sensors.end() ) {
		SEISCOMP_ERROR("Inventory::remove(Sensor*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_sensors.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeSensor(size_t i) {
	// index out of bounds
	if ( i >= _sensors.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_sensors[i]->accept(&nc);
	}

	_sensors[i]->setParent(NULL);
	childRemoved(_sensors[i].get());
	
	_sensors.erase(_sensors.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeSensor(const SensorIndex& i) {
	Sensor* object = sensor(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::dataloggerCount() const {
	return _dataloggers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Inventory::datalogger(size_t i) const {
	return _dataloggers[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Inventory::datalogger(const DataloggerIndex& i) const {
	for ( std::vector<DataloggerPtr>::const_iterator it = _dataloggers.begin(); it != _dataloggers.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Inventory::findDatalogger(const std::string& publicID) const {
	for ( std::vector<DataloggerPtr>::const_iterator it = _dataloggers.begin(); it != _dataloggers.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(Datalogger* datalogger) {
	if ( datalogger == NULL )
		return false;

	// Element has already a parent
	if ( datalogger->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(Datalogger*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Datalogger* dataloggerCached = Datalogger::Find(datalogger->publicID());
		if ( dataloggerCached ) {
			if ( dataloggerCached->parent() ) {
				if ( dataloggerCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(Datalogger*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(Datalogger*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				datalogger = dataloggerCached;
		}
	}

	// Add the element
	_dataloggers.push_back(datalogger);
	datalogger->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		datalogger->accept(&nc);
	}

	// Notify registered observers
	childAdded(datalogger);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(Datalogger* datalogger) {
	if ( datalogger == NULL )
		return false;

	if ( datalogger->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(Datalogger*) -> element has another parent");
		return false;
	}

	std::vector<DataloggerPtr>::iterator it;
	it = std::find(_dataloggers.begin(), _dataloggers.end(), datalogger);
	// Element has not been found
	if ( it == _dataloggers.end() ) {
		SEISCOMP_ERROR("Inventory::remove(Datalogger*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_dataloggers.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeDatalogger(size_t i) {
	// index out of bounds
	if ( i >= _dataloggers.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_dataloggers[i]->accept(&nc);
	}

	_dataloggers[i]->setParent(NULL);
	childRemoved(_dataloggers[i].get());
	
	_dataloggers.erase(_dataloggers.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeDatalogger(const DataloggerIndex& i) {
	Datalogger* object = datalogger(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::responsePAZCount() const {
	return _responsePAZs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ* Inventory::responsePAZ(size_t i) const {
	return _responsePAZs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ* Inventory::responsePAZ(const ResponsePAZIndex& i) const {
	for ( std::vector<ResponsePAZPtr>::const_iterator it = _responsePAZs.begin(); it != _responsePAZs.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ* Inventory::findResponsePAZ(const std::string& publicID) const {
	for ( std::vector<ResponsePAZPtr>::const_iterator it = _responsePAZs.begin(); it != _responsePAZs.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(ResponsePAZ* responsePAZ) {
	if ( responsePAZ == NULL )
		return false;

	// Element has already a parent
	if ( responsePAZ->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(ResponsePAZ*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ResponsePAZ* responsePAZCached = ResponsePAZ::Find(responsePAZ->publicID());
		if ( responsePAZCached ) {
			if ( responsePAZCached->parent() ) {
				if ( responsePAZCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(ResponsePAZ*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(ResponsePAZ*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				responsePAZ = responsePAZCached;
		}
	}

	// Add the element
	_responsePAZs.push_back(responsePAZ);
	responsePAZ->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		responsePAZ->accept(&nc);
	}

	// Notify registered observers
	childAdded(responsePAZ);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(ResponsePAZ* responsePAZ) {
	if ( responsePAZ == NULL )
		return false;

	if ( responsePAZ->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(ResponsePAZ*) -> element has another parent");
		return false;
	}

	std::vector<ResponsePAZPtr>::iterator it;
	it = std::find(_responsePAZs.begin(), _responsePAZs.end(), responsePAZ);
	// Element has not been found
	if ( it == _responsePAZs.end() ) {
		SEISCOMP_ERROR("Inventory::remove(ResponsePAZ*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_responsePAZs.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponsePAZ(size_t i) {
	// index out of bounds
	if ( i >= _responsePAZs.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_responsePAZs[i]->accept(&nc);
	}

	_responsePAZs[i]->setParent(NULL);
	childRemoved(_responsePAZs[i].get());
	
	_responsePAZs.erase(_responsePAZs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponsePAZ(const ResponsePAZIndex& i) {
	ResponsePAZ* object = responsePAZ(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::responseFIRCount() const {
	return _responseFIRs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* Inventory::responseFIR(size_t i) const {
	return _responseFIRs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* Inventory::responseFIR(const ResponseFIRIndex& i) const {
	for ( std::vector<ResponseFIRPtr>::const_iterator it = _responseFIRs.begin(); it != _responseFIRs.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR* Inventory::findResponseFIR(const std::string& publicID) const {
	for ( std::vector<ResponseFIRPtr>::const_iterator it = _responseFIRs.begin(); it != _responseFIRs.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(ResponseFIR* responseFIR) {
	if ( responseFIR == NULL )
		return false;

	// Element has already a parent
	if ( responseFIR->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(ResponseFIR*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ResponseFIR* responseFIRCached = ResponseFIR::Find(responseFIR->publicID());
		if ( responseFIRCached ) {
			if ( responseFIRCached->parent() ) {
				if ( responseFIRCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(ResponseFIR*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(ResponseFIR*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				responseFIR = responseFIRCached;
		}
	}

	// Add the element
	_responseFIRs.push_back(responseFIR);
	responseFIR->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		responseFIR->accept(&nc);
	}

	// Notify registered observers
	childAdded(responseFIR);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(ResponseFIR* responseFIR) {
	if ( responseFIR == NULL )
		return false;

	if ( responseFIR->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseFIR*) -> element has another parent");
		return false;
	}

	std::vector<ResponseFIRPtr>::iterator it;
	it = std::find(_responseFIRs.begin(), _responseFIRs.end(), responseFIR);
	// Element has not been found
	if ( it == _responseFIRs.end() ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseFIR*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_responseFIRs.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseFIR(size_t i) {
	// index out of bounds
	if ( i >= _responseFIRs.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_responseFIRs[i]->accept(&nc);
	}

	_responseFIRs[i]->setParent(NULL);
	childRemoved(_responseFIRs[i].get());
	
	_responseFIRs.erase(_responseFIRs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseFIR(const ResponseFIRIndex& i) {
	ResponseFIR* object = responseFIR(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::responseIIRCount() const {
	return _responseIIRs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* Inventory::responseIIR(size_t i) const {
	return _responseIIRs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* Inventory::responseIIR(const ResponseIIRIndex& i) const {
	for ( std::vector<ResponseIIRPtr>::const_iterator it = _responseIIRs.begin(); it != _responseIIRs.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR* Inventory::findResponseIIR(const std::string& publicID) const {
	for ( std::vector<ResponseIIRPtr>::const_iterator it = _responseIIRs.begin(); it != _responseIIRs.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(ResponseIIR* responseIIR) {
	if ( responseIIR == NULL )
		return false;

	// Element has already a parent
	if ( responseIIR->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(ResponseIIR*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ResponseIIR* responseIIRCached = ResponseIIR::Find(responseIIR->publicID());
		if ( responseIIRCached ) {
			if ( responseIIRCached->parent() ) {
				if ( responseIIRCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(ResponseIIR*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(ResponseIIR*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				responseIIR = responseIIRCached;
		}
	}

	// Add the element
	_responseIIRs.push_back(responseIIR);
	responseIIR->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		responseIIR->accept(&nc);
	}

	// Notify registered observers
	childAdded(responseIIR);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(ResponseIIR* responseIIR) {
	if ( responseIIR == NULL )
		return false;

	if ( responseIIR->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseIIR*) -> element has another parent");
		return false;
	}

	std::vector<ResponseIIRPtr>::iterator it;
	it = std::find(_responseIIRs.begin(), _responseIIRs.end(), responseIIR);
	// Element has not been found
	if ( it == _responseIIRs.end() ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseIIR*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_responseIIRs.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseIIR(size_t i) {
	// index out of bounds
	if ( i >= _responseIIRs.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_responseIIRs[i]->accept(&nc);
	}

	_responseIIRs[i]->setParent(NULL);
	childRemoved(_responseIIRs[i].get());
	
	_responseIIRs.erase(_responseIIRs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseIIR(const ResponseIIRIndex& i) {
	ResponseIIR* object = responseIIR(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::responsePolynomialCount() const {
	return _responsePolynomials.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial* Inventory::responsePolynomial(size_t i) const {
	return _responsePolynomials[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial* Inventory::responsePolynomial(const ResponsePolynomialIndex& i) const {
	for ( std::vector<ResponsePolynomialPtr>::const_iterator it = _responsePolynomials.begin(); it != _responsePolynomials.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial* Inventory::findResponsePolynomial(const std::string& publicID) const {
	for ( std::vector<ResponsePolynomialPtr>::const_iterator it = _responsePolynomials.begin(); it != _responsePolynomials.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(ResponsePolynomial* responsePolynomial) {
	if ( responsePolynomial == NULL )
		return false;

	// Element has already a parent
	if ( responsePolynomial->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(ResponsePolynomial*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ResponsePolynomial* responsePolynomialCached = ResponsePolynomial::Find(responsePolynomial->publicID());
		if ( responsePolynomialCached ) {
			if ( responsePolynomialCached->parent() ) {
				if ( responsePolynomialCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(ResponsePolynomial*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(ResponsePolynomial*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				responsePolynomial = responsePolynomialCached;
		}
	}

	// Add the element
	_responsePolynomials.push_back(responsePolynomial);
	responsePolynomial->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		responsePolynomial->accept(&nc);
	}

	// Notify registered observers
	childAdded(responsePolynomial);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(ResponsePolynomial* responsePolynomial) {
	if ( responsePolynomial == NULL )
		return false;

	if ( responsePolynomial->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(ResponsePolynomial*) -> element has another parent");
		return false;
	}

	std::vector<ResponsePolynomialPtr>::iterator it;
	it = std::find(_responsePolynomials.begin(), _responsePolynomials.end(), responsePolynomial);
	// Element has not been found
	if ( it == _responsePolynomials.end() ) {
		SEISCOMP_ERROR("Inventory::remove(ResponsePolynomial*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_responsePolynomials.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponsePolynomial(size_t i) {
	// index out of bounds
	if ( i >= _responsePolynomials.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_responsePolynomials[i]->accept(&nc);
	}

	_responsePolynomials[i]->setParent(NULL);
	childRemoved(_responsePolynomials[i].get());
	
	_responsePolynomials.erase(_responsePolynomials.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponsePolynomial(const ResponsePolynomialIndex& i) {
	ResponsePolynomial* object = responsePolynomial(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::responseFAPCount() const {
	return _responseFAPs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP* Inventory::responseFAP(size_t i) const {
	return _responseFAPs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP* Inventory::responseFAP(const ResponseFAPIndex& i) const {
	for ( std::vector<ResponseFAPPtr>::const_iterator it = _responseFAPs.begin(); it != _responseFAPs.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP* Inventory::findResponseFAP(const std::string& publicID) const {
	for ( std::vector<ResponseFAPPtr>::const_iterator it = _responseFAPs.begin(); it != _responseFAPs.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(ResponseFAP* responseFAP) {
	if ( responseFAP == NULL )
		return false;

	// Element has already a parent
	if ( responseFAP->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(ResponseFAP*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ResponseFAP* responseFAPCached = ResponseFAP::Find(responseFAP->publicID());
		if ( responseFAPCached ) {
			if ( responseFAPCached->parent() ) {
				if ( responseFAPCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(ResponseFAP*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(ResponseFAP*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				responseFAP = responseFAPCached;
		}
	}

	// Add the element
	_responseFAPs.push_back(responseFAP);
	responseFAP->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		responseFAP->accept(&nc);
	}

	// Notify registered observers
	childAdded(responseFAP);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(ResponseFAP* responseFAP) {
	if ( responseFAP == NULL )
		return false;

	if ( responseFAP->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseFAP*) -> element has another parent");
		return false;
	}

	std::vector<ResponseFAPPtr>::iterator it;
	it = std::find(_responseFAPs.begin(), _responseFAPs.end(), responseFAP);
	// Element has not been found
	if ( it == _responseFAPs.end() ) {
		SEISCOMP_ERROR("Inventory::remove(ResponseFAP*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_responseFAPs.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseFAP(size_t i) {
	// index out of bounds
	if ( i >= _responseFAPs.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_responseFAPs[i]->accept(&nc);
	}

	_responseFAPs[i]->setParent(NULL);
	childRemoved(_responseFAPs[i].get());
	
	_responseFAPs.erase(_responseFAPs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeResponseFAP(const ResponseFAPIndex& i) {
	ResponseFAP* object = responseFAP(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Inventory::networkCount() const {
	return _networks.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network* Inventory::network(size_t i) const {
	return _networks[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network* Inventory::network(const NetworkIndex& i) const {
	for ( std::vector<NetworkPtr>::const_iterator it = _networks.begin(); it != _networks.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network* Inventory::findNetwork(const std::string& publicID) const {
	for ( std::vector<NetworkPtr>::const_iterator it = _networks.begin(); it != _networks.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::add(Network* network) {
	if ( network == NULL )
		return false;

	// Element has already a parent
	if ( network->parent() != NULL ) {
		SEISCOMP_ERROR("Inventory::add(Network*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Network* networkCached = Network::Find(network->publicID());
		if ( networkCached ) {
			if ( networkCached->parent() ) {
				if ( networkCached->parent() == this )
					SEISCOMP_ERROR("Inventory::add(Network*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Inventory::add(Network*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				network = networkCached;
		}
	}

	// Add the element
	_networks.push_back(network);
	network->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		network->accept(&nc);
	}

	// Notify registered observers
	childAdded(network);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::remove(Network* network) {
	if ( network == NULL )
		return false;

	if ( network->parent() != this ) {
		SEISCOMP_ERROR("Inventory::remove(Network*) -> element has another parent");
		return false;
	}

	std::vector<NetworkPtr>::iterator it;
	it = std::find(_networks.begin(), _networks.end(), network);
	// Element has not been found
	if ( it == _networks.end() ) {
		SEISCOMP_ERROR("Inventory::remove(Network*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_networks.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeNetwork(size_t i) {
	// index out of bounds
	if ( i >= _networks.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_networks[i]->accept(&nc);
	}

	_networks[i]->setParent(NULL);
	childRemoved(_networks[i].get());
	
	_networks.erase(_networks.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Inventory::removeNetwork(const NetworkIndex& i) {
	Network* object = network(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Inventory skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("stationGroup",
	                       Seiscomp::Core::Generic::containerMember(_stationGroups,
	                       Seiscomp::Core::Generic::bindMemberFunction<StationGroup>(static_cast<bool (Inventory::*)(StationGroup*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("auxDevice",
	                       Seiscomp::Core::Generic::containerMember(_auxDevices,
	                       Seiscomp::Core::Generic::bindMemberFunction<AuxDevice>(static_cast<bool (Inventory::*)(AuxDevice*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("sensor",
	                       Seiscomp::Core::Generic::containerMember(_sensors,
	                       Seiscomp::Core::Generic::bindMemberFunction<Sensor>(static_cast<bool (Inventory::*)(Sensor*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("datalogger",
	                       Seiscomp::Core::Generic::containerMember(_dataloggers,
	                       Seiscomp::Core::Generic::bindMemberFunction<Datalogger>(static_cast<bool (Inventory::*)(Datalogger*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("responsePAZ",
	                       Seiscomp::Core::Generic::containerMember(_responsePAZs,
	                       Seiscomp::Core::Generic::bindMemberFunction<ResponsePAZ>(static_cast<bool (Inventory::*)(ResponsePAZ*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("responseFIR",
	                       Seiscomp::Core::Generic::containerMember(_responseFIRs,
	                       Seiscomp::Core::Generic::bindMemberFunction<ResponseFIR>(static_cast<bool (Inventory::*)(ResponseFIR*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("responseIIR",
		                       Seiscomp::Core::Generic::containerMember(_responseIIRs,
		                       Seiscomp::Core::Generic::bindMemberFunction<ResponseIIR>(static_cast<bool (Inventory::*)(ResponseIIR*)>(&Inventory::add), this)),
		                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("responsePolynomial",
	                       Seiscomp::Core::Generic::containerMember(_responsePolynomials,
	                       Seiscomp::Core::Generic::bindMemberFunction<ResponsePolynomial>(static_cast<bool (Inventory::*)(ResponsePolynomial*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
	if ( ar.supportsVersion<0,8>() )
		ar & NAMED_OBJECT_HINT("responseFAP",
		                       Seiscomp::Core::Generic::containerMember(_responseFAPs,
		                       Seiscomp::Core::Generic::bindMemberFunction<ResponseFAP>(static_cast<bool (Inventory::*)(ResponseFAP*)>(&Inventory::add), this)),
		                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("network",
	                       Seiscomp::Core::Generic::containerMember(_networks,
	                       Seiscomp::Core::Generic::bindMemberFunction<Network>(static_cast<bool (Inventory::*)(Network*)>(&Inventory::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
