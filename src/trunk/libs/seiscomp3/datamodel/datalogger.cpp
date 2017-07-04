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
#include <seiscomp3/datamodel/datalogger.h>
#include <seiscomp3/datamodel/inventory.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Datalogger, PublicObject, "Datalogger");


Datalogger::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("name", "string", false, false, true, false, false, false, NULL, &Datalogger::setName, &Datalogger::name));
	addProperty(Core::simpleProperty("description", "string", false, false, false, false, false, false, NULL, &Datalogger::setDescription, &Datalogger::description));
	addProperty(Core::simpleProperty("digitizerModel", "string", false, false, false, false, false, false, NULL, &Datalogger::setDigitizerModel, &Datalogger::digitizerModel));
	addProperty(Core::simpleProperty("digitizerManufacturer", "string", false, false, false, false, false, false, NULL, &Datalogger::setDigitizerManufacturer, &Datalogger::digitizerManufacturer));
	addProperty(Core::simpleProperty("recorderModel", "string", false, false, false, false, false, false, NULL, &Datalogger::setRecorderModel, &Datalogger::recorderModel));
	addProperty(Core::simpleProperty("recorderManufacturer", "string", false, false, false, false, false, false, NULL, &Datalogger::setRecorderManufacturer, &Datalogger::recorderManufacturer));
	addProperty(Core::simpleProperty("clockModel", "string", false, false, false, false, false, false, NULL, &Datalogger::setClockModel, &Datalogger::clockModel));
	addProperty(Core::simpleProperty("clockManufacturer", "string", false, false, false, false, false, false, NULL, &Datalogger::setClockManufacturer, &Datalogger::clockManufacturer));
	addProperty(Core::simpleProperty("clockType", "string", false, false, false, false, false, false, NULL, &Datalogger::setClockType, &Datalogger::clockType));
	addProperty(Core::simpleProperty("gain", "float", false, false, false, false, true, false, NULL, &Datalogger::setGain, &Datalogger::gain));
	addProperty(Core::simpleProperty("maxClockDrift", "float", false, false, false, false, true, false, NULL, &Datalogger::setMaxClockDrift, &Datalogger::maxClockDrift));
	addProperty(objectProperty<Blob>("remark", "Blob", false, false, true, &Datalogger::setRemark, &Datalogger::remark));
	addProperty(arrayClassProperty<DataloggerCalibration>("calibration", "DataloggerCalibration", &Datalogger::dataloggerCalibrationCount, &Datalogger::dataloggerCalibration, static_cast<bool (Datalogger::*)(DataloggerCalibration*)>(&Datalogger::add), &Datalogger::removeDataloggerCalibration, static_cast<bool (Datalogger::*)(DataloggerCalibration*)>(&Datalogger::remove)));
	addProperty(arrayClassProperty<Decimation>("decimation", "Decimation", &Datalogger::decimationCount, &Datalogger::decimation, static_cast<bool (Datalogger::*)(Decimation*)>(&Datalogger::add), &Datalogger::removeDecimation, static_cast<bool (Datalogger::*)(Decimation*)>(&Datalogger::remove)));
}


IMPLEMENT_METAOBJECT(Datalogger)


DataloggerIndex::DataloggerIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataloggerIndex::DataloggerIndex(const std::string& name_) {
	name = name_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataloggerIndex::DataloggerIndex(const DataloggerIndex& idx) {
	name = idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataloggerIndex::operator==(const DataloggerIndex& idx) const {
	return name == idx.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DataloggerIndex::operator!=(const DataloggerIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::Datalogger() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::Datalogger(const Datalogger& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::Datalogger(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger::~Datalogger() {
	std::for_each(_dataloggerCalibrations.begin(), _dataloggerCalibrations.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&DataloggerCalibration::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&DataloggerCalibrationPtr::get)));
	std::for_each(_decimations.begin(), _decimations.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Decimation::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&DecimationPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Datalogger::Create() {
	Datalogger* object = new Datalogger();
	return static_cast<Datalogger*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Datalogger::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Datalogger(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger* Datalogger::Find(const std::string& publicID) {
	return Datalogger::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::operator==(const Datalogger& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _description != rhs._description ) return false;
	if ( _digitizerModel != rhs._digitizerModel ) return false;
	if ( _digitizerManufacturer != rhs._digitizerManufacturer ) return false;
	if ( _recorderModel != rhs._recorderModel ) return false;
	if ( _recorderManufacturer != rhs._recorderManufacturer ) return false;
	if ( _clockModel != rhs._clockModel ) return false;
	if ( _clockManufacturer != rhs._clockManufacturer ) return false;
	if ( _clockType != rhs._clockType ) return false;
	if ( _gain != rhs._gain ) return false;
	if ( _maxClockDrift != rhs._maxClockDrift ) return false;
	if ( _remark != rhs._remark ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::operator!=(const Datalogger& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::equal(const Datalogger& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setName(const std::string& name) {
	_index.name = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::name() const {
	return _index.name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setDescription(const std::string& description) {
	_description = description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setDigitizerModel(const std::string& digitizerModel) {
	_digitizerModel = digitizerModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::digitizerModel() const {
	return _digitizerModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setDigitizerManufacturer(const std::string& digitizerManufacturer) {
	_digitizerManufacturer = digitizerManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::digitizerManufacturer() const {
	return _digitizerManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setRecorderModel(const std::string& recorderModel) {
	_recorderModel = recorderModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::recorderModel() const {
	return _recorderModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setRecorderManufacturer(const std::string& recorderManufacturer) {
	_recorderManufacturer = recorderManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::recorderManufacturer() const {
	return _recorderManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setClockModel(const std::string& clockModel) {
	_clockModel = clockModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::clockModel() const {
	return _clockModel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setClockManufacturer(const std::string& clockManufacturer) {
	_clockManufacturer = clockManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::clockManufacturer() const {
	return _clockManufacturer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setClockType(const std::string& clockType) {
	_clockType = clockType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Datalogger::clockType() const {
	return _clockType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setGain(const OPT(double)& gain) {
	_gain = gain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Datalogger::gain() const {
	if ( _gain )
		return *_gain;
	throw Seiscomp::Core::ValueException("Datalogger.gain is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setMaxClockDrift(const OPT(double)& maxClockDrift) {
	_maxClockDrift = maxClockDrift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Datalogger::maxClockDrift() const {
	if ( _maxClockDrift )
		return *_maxClockDrift;
	throw Seiscomp::Core::ValueException("Datalogger.maxClockDrift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::setRemark(const OPT(Blob)& remark) {
	_remark = remark;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Blob& Datalogger::remark() {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Datalogger.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Blob& Datalogger::remark() const {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Datalogger.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataloggerIndex& Datalogger::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::equalIndex(const Datalogger* lhs) const {
	if ( lhs == NULL ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory* Datalogger::inventory() const {
	return static_cast<Inventory*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger& Datalogger::operator=(const Datalogger& other) {
	PublicObject::operator=(other);
	_index = other._index;
	_description = other._description;
	_digitizerModel = other._digitizerModel;
	_digitizerManufacturer = other._digitizerManufacturer;
	_recorderModel = other._recorderModel;
	_recorderManufacturer = other._recorderManufacturer;
	_clockModel = other._clockModel;
	_clockManufacturer = other._clockManufacturer;
	_clockType = other._clockType;
	_gain = other._gain;
	_maxClockDrift = other._maxClockDrift;
	_remark = other._remark;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::assign(Object* other) {
	Datalogger* otherDatalogger = Datalogger::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherDatalogger;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	Inventory* inventory = Inventory::Cast(parent);
	if ( inventory != NULL )
		return inventory->add(this);

	SEISCOMP_ERROR("Datalogger::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	Inventory* inventory = Inventory::Cast(object);
	if ( inventory != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return inventory->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Datalogger* child = inventory->findDatalogger(publicID());
			if ( child != NULL )
				return inventory->remove(child);
			else {
				SEISCOMP_DEBUG("Datalogger::detachFrom(Inventory): datalogger has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Datalogger::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Datalogger::clone() const {
	Datalogger* clonee = new Datalogger();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::updateChild(Object* child) {
	DataloggerCalibration* dataloggerCalibrationChild = DataloggerCalibration::Cast(child);
	if ( dataloggerCalibrationChild != NULL ) {
		DataloggerCalibration* dataloggerCalibrationElement = dataloggerCalibration(dataloggerCalibrationChild->index());
		if ( dataloggerCalibrationElement != NULL ) {
			*dataloggerCalibrationElement = *dataloggerCalibrationChild;
			return true;
		}
		return false;
	}

	Decimation* decimationChild = Decimation::Cast(child);
	if ( decimationChild != NULL ) {
		Decimation* decimationElement = decimation(decimationChild->index());
		if ( decimationElement != NULL ) {
			*decimationElement = *decimationChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<DataloggerCalibrationPtr>::iterator it = _dataloggerCalibrations.begin(); it != _dataloggerCalibrations.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<DecimationPtr>::iterator it = _decimations.begin(); it != _decimations.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Datalogger::dataloggerCalibrationCount() const {
	return _dataloggerCalibrations.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataloggerCalibration* Datalogger::dataloggerCalibration(size_t i) const {
	return _dataloggerCalibrations[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataloggerCalibration* Datalogger::dataloggerCalibration(const DataloggerCalibrationIndex& i) const {
	for ( std::vector<DataloggerCalibrationPtr>::const_iterator it = _dataloggerCalibrations.begin(); it != _dataloggerCalibrations.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::add(DataloggerCalibration* dataloggerCalibration) {
	if ( dataloggerCalibration == NULL )
		return false;

	// Element has already a parent
	if ( dataloggerCalibration->parent() != NULL ) {
		SEISCOMP_ERROR("Datalogger::add(DataloggerCalibration*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<DataloggerCalibrationPtr>::iterator it = _dataloggerCalibrations.begin(); it != _dataloggerCalibrations.end(); ++it ) {
		if ( (*it)->index() == dataloggerCalibration->index() ) {
			SEISCOMP_ERROR("Datalogger::add(DataloggerCalibration*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_dataloggerCalibrations.push_back(dataloggerCalibration);
	dataloggerCalibration->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		dataloggerCalibration->accept(&nc);
	}

	// Notify registered observers
	childAdded(dataloggerCalibration);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::remove(DataloggerCalibration* dataloggerCalibration) {
	if ( dataloggerCalibration == NULL )
		return false;

	if ( dataloggerCalibration->parent() != this ) {
		SEISCOMP_ERROR("Datalogger::remove(DataloggerCalibration*) -> element has another parent");
		return false;
	}

	std::vector<DataloggerCalibrationPtr>::iterator it;
	it = std::find(_dataloggerCalibrations.begin(), _dataloggerCalibrations.end(), dataloggerCalibration);
	// Element has not been found
	if ( it == _dataloggerCalibrations.end() ) {
		SEISCOMP_ERROR("Datalogger::remove(DataloggerCalibration*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_dataloggerCalibrations.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::removeDataloggerCalibration(size_t i) {
	// index out of bounds
	if ( i >= _dataloggerCalibrations.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_dataloggerCalibrations[i]->accept(&nc);
	}

	_dataloggerCalibrations[i]->setParent(NULL);
	childRemoved(_dataloggerCalibrations[i].get());
	
	_dataloggerCalibrations.erase(_dataloggerCalibrations.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::removeDataloggerCalibration(const DataloggerCalibrationIndex& i) {
	DataloggerCalibration* object = dataloggerCalibration(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Datalogger::decimationCount() const {
	return _decimations.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation* Datalogger::decimation(size_t i) const {
	return _decimations[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation* Datalogger::decimation(const DecimationIndex& i) const {
	for ( std::vector<DecimationPtr>::const_iterator it = _decimations.begin(); it != _decimations.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::add(Decimation* decimation) {
	if ( decimation == NULL )
		return false;

	// Element has already a parent
	if ( decimation->parent() != NULL ) {
		SEISCOMP_ERROR("Datalogger::add(Decimation*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<DecimationPtr>::iterator it = _decimations.begin(); it != _decimations.end(); ++it ) {
		if ( (*it)->index() == decimation->index() ) {
			SEISCOMP_ERROR("Datalogger::add(Decimation*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_decimations.push_back(decimation);
	decimation->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		decimation->accept(&nc);
	}

	// Notify registered observers
	childAdded(decimation);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::remove(Decimation* decimation) {
	if ( decimation == NULL )
		return false;

	if ( decimation->parent() != this ) {
		SEISCOMP_ERROR("Datalogger::remove(Decimation*) -> element has another parent");
		return false;
	}

	std::vector<DecimationPtr>::iterator it;
	it = std::find(_decimations.begin(), _decimations.end(), decimation);
	// Element has not been found
	if ( it == _decimations.end() ) {
		SEISCOMP_ERROR("Datalogger::remove(Decimation*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_decimations.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::removeDecimation(size_t i) {
	// index out of bounds
	if ( i >= _decimations.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_decimations[i]->accept(&nc);
	}

	_decimations[i]->setParent(NULL);
	childRemoved(_decimations[i].get());
	
	_decimations.erase(_decimations.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Datalogger::removeDecimation(const DecimationIndex& i) {
	Decimation* object = decimation(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Datalogger::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Datalogger skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("name", _index.name, Archive::INDEX_ATTRIBUTE);
	ar & NAMED_OBJECT_HINT("description", _description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("digitizerModel", _digitizerModel, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("digitizerManufacturer", _digitizerManufacturer, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("recorderModel", _recorderModel, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("recorderManufacturer", _recorderManufacturer, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clockModel", _clockModel, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clockManufacturer", _clockManufacturer, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clockType", _clockType, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("gain", _gain, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("maxClockDrift", _maxClockDrift, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("remark", _remark, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("calibration",
	                       Seiscomp::Core::Generic::containerMember(_dataloggerCalibrations,
	                       Seiscomp::Core::Generic::bindMemberFunction<DataloggerCalibration>(static_cast<bool (Datalogger::*)(DataloggerCalibration*)>(&Datalogger::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("decimation",
	                       Seiscomp::Core::Generic::containerMember(_decimations,
	                       Seiscomp::Core::Generic::bindMemberFunction<Decimation>(static_cast<bool (Datalogger::*)(Decimation*)>(&Datalogger::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
