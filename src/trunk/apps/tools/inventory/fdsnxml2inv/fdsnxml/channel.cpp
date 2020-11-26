/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/channel.h>
#include <fdsnxml/output.h>
#include <fdsnxml/equipment.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


Channel::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<LatitudeType>("latitude", "FDSNXML::LatitudeType", false, false, &Channel::setLatitude, &Channel::latitude));
	addProperty(objectProperty<LongitudeType>("longitude", "FDSNXML::LongitudeType", false, false, &Channel::setLongitude, &Channel::longitude));
	addProperty(objectProperty<DistanceType>("elevation", "FDSNXML::DistanceType", false, false, &Channel::setElevation, &Channel::elevation));
	addProperty(objectProperty<DistanceType>("depth", "FDSNXML::DistanceType", false, false, &Channel::setDepth, &Channel::depth));
	addProperty(objectProperty<AzimuthType>("azimuth", "FDSNXML::AzimuthType", false, true, &Channel::setAzimuth, &Channel::azimuth));
	addProperty(objectProperty<DipType>("dip", "FDSNXML::DipType", false, true, &Channel::setDip, &Channel::dip));
	addProperty(objectProperty<FloatType>("waterLevel", "FDSNXML::FloatType", false, true, &Channel::setWaterLevel, &Channel::waterLevel));
	addProperty(arrayClassProperty<Output>("type", "FDSNXML::Output", &Channel::typeCount, &Channel::type, static_cast<bool (Channel::*)(Output*)>(&Channel::addType), &Channel::removeType, static_cast<bool (Channel::*)(Output*)>(&Channel::removeType)));
	addProperty(objectProperty<SampleRateType>("SampleRate", "FDSNXML::SampleRateType", false, true, &Channel::setSampleRate, &Channel::sampleRate));
	addProperty(objectProperty<SampleRateRatioType>("SampleRateRatio", "FDSNXML::SampleRateRatioType", false, true, &Channel::setSampleRateRatio, &Channel::sampleRateRatio));
	addProperty(objectProperty<ClockDriftType>("ClockDrift", "FDSNXML::ClockDriftType", false, true, &Channel::setClockDrift, &Channel::clockDrift));
	addProperty(objectProperty<UnitsType>("CalibrationUnits", "FDSNXML::UnitsType", false, true, &Channel::setCalibrationUnits, &Channel::calibrationUnits));
	addProperty(objectProperty<Equipment>("Sensor", "FDSNXML::Equipment", false, true, &Channel::setSensor, &Channel::sensor));
	addProperty(objectProperty<Equipment>("PreAmplifier", "FDSNXML::Equipment", false, true, &Channel::setPreAmplifier, &Channel::preAmplifier));
	addProperty(objectProperty<Equipment>("DataLogger", "FDSNXML::Equipment", false, true, &Channel::setDataLogger, &Channel::dataLogger));
	addProperty(objectProperty<Response>("Response", "FDSNXML::Response", false, true, &Channel::setResponse, &Channel::response));
	addProperty(Core::simpleProperty("locationCode", "string", false, false, false, false, false, false, NULL, &Channel::setLocationCode, &Channel::locationCode));
	addProperty(arrayClassProperty<Equipment>("equipment", "FDSNXML::Equipment", &Channel::equipmentCount, &Channel::equipment, static_cast<bool (Channel::*)(Equipment*)>(&Channel::addEquipment), &Channel::removeEquipment, static_cast<bool (Channel::*)(Equipment*)>(&Channel::removeEquipment)));
}


IMPLEMENT_RTTI(Channel, "FDSNXML::Channel", BaseNode)
IMPLEMENT_RTTI_METHODS(Channel)
IMPLEMENT_METAOBJECT_DERIVED(Channel, BaseNode)


Channel::Channel() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::Channel(const Channel &other)
 : BaseNode() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::~Channel() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::operator==(const Channel &rhs) const {
	if ( !(_latitude == rhs._latitude) )
		return false;
	if ( !(_longitude == rhs._longitude) )
		return false;
	if ( !(_elevation == rhs._elevation) )
		return false;
	if ( !(_depth == rhs._depth) )
		return false;
	if ( !(_azimuth == rhs._azimuth) )
		return false;
	if ( !(_dip == rhs._dip) )
		return false;
	if ( !(_waterLevel == rhs._waterLevel) )
		return false;
	if ( !(_sampleRate == rhs._sampleRate) )
		return false;
	if ( !(_sampleRateRatio == rhs._sampleRateRatio) )
		return false;
	if ( !(_clockDrift == rhs._clockDrift) )
		return false;
	if ( !(_calibrationUnits == rhs._calibrationUnits) )
		return false;
	if ( !(_sensor == rhs._sensor) )
		return false;
	if ( !(_preAmplifier == rhs._preAmplifier) )
		return false;
	if ( !(_dataLogger == rhs._dataLogger) )
		return false;
	if ( !(_response == rhs._response) )
		return false;
	if ( !(_locationCode == rhs._locationCode) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setLatitude(const LatitudeType& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LatitudeType& Channel::latitude() {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LatitudeType& Channel::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setLongitude(const LongitudeType& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LongitudeType& Channel::longitude() {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LongitudeType& Channel::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setElevation(const DistanceType& elevation) {
	_elevation = elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& Channel::elevation() {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& Channel::elevation() const {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setDepth(const DistanceType& depth) {
	_depth = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& Channel::depth() {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& Channel::depth() const {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setAzimuth(const OPT(AzimuthType)& azimuth) {
	_azimuth = azimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AzimuthType& Channel::azimuth() {
	if ( _azimuth )
		return *_azimuth;
	throw Seiscomp::Core::ValueException("Channel.azimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AzimuthType& Channel::azimuth() const {
	if ( _azimuth )
		return *_azimuth;
	throw Seiscomp::Core::ValueException("Channel.azimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setDip(const OPT(DipType)& dip) {
	_dip = dip;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DipType& Channel::dip() {
	if ( _dip )
		return *_dip;
	throw Seiscomp::Core::ValueException("Channel.dip is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DipType& Channel::dip() const {
	if ( _dip )
		return *_dip;
	throw Seiscomp::Core::ValueException("Channel.dip is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setWaterLevel(const OPT(FloatType)& waterLevel) {
	_waterLevel = waterLevel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FloatType& Channel::waterLevel() {
	if ( _waterLevel )
		return *_waterLevel;
	throw Seiscomp::Core::ValueException("Channel.waterLevel is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FloatType& Channel::waterLevel() const {
	if ( _waterLevel )
		return *_waterLevel;
	throw Seiscomp::Core::ValueException("Channel.waterLevel is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setSampleRate(const OPT(SampleRateType)& sampleRate) {
	_sampleRate = sampleRate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateType& Channel::sampleRate() {
	if ( _sampleRate )
		return *_sampleRate;
	throw Seiscomp::Core::ValueException("Channel.SampleRate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SampleRateType& Channel::sampleRate() const {
	if ( _sampleRate )
		return *_sampleRate;
	throw Seiscomp::Core::ValueException("Channel.SampleRate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setSampleRateRatio(const OPT(SampleRateRatioType)& sampleRateRatio) {
	_sampleRateRatio = sampleRateRatio;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateRatioType& Channel::sampleRateRatio() {
	if ( _sampleRateRatio )
		return *_sampleRateRatio;
	throw Seiscomp::Core::ValueException("Channel.SampleRateRatio is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SampleRateRatioType& Channel::sampleRateRatio() const {
	if ( _sampleRateRatio )
		return *_sampleRateRatio;
	throw Seiscomp::Core::ValueException("Channel.SampleRateRatio is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setClockDrift(const OPT(ClockDriftType)& clockDrift) {
	_clockDrift = clockDrift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClockDriftType& Channel::clockDrift() {
	if ( _clockDrift )
		return *_clockDrift;
	throw Seiscomp::Core::ValueException("Channel.ClockDrift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClockDriftType& Channel::clockDrift() const {
	if ( _clockDrift )
		return *_clockDrift;
	throw Seiscomp::Core::ValueException("Channel.ClockDrift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setCalibrationUnits(const OPT(UnitsType)& calibrationUnits) {
	_calibrationUnits = calibrationUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
UnitsType& Channel::calibrationUnits() {
	if ( _calibrationUnits )
		return *_calibrationUnits;
	throw Seiscomp::Core::ValueException("Channel.CalibrationUnits is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const UnitsType& Channel::calibrationUnits() const {
	if ( _calibrationUnits )
		return *_calibrationUnits;
	throw Seiscomp::Core::ValueException("Channel.CalibrationUnits is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setSensor(const OPT(Equipment)& sensor) {
	_sensor = sensor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment& Channel::sensor() {
	if ( _sensor )
		return *_sensor;
	throw Seiscomp::Core::ValueException("Channel.Sensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Equipment& Channel::sensor() const {
	if ( _sensor )
		return *_sensor;
	throw Seiscomp::Core::ValueException("Channel.Sensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setPreAmplifier(const OPT(Equipment)& preAmplifier) {
	_preAmplifier = preAmplifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment& Channel::preAmplifier() {
	if ( _preAmplifier )
		return *_preAmplifier;
	throw Seiscomp::Core::ValueException("Channel.PreAmplifier is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Equipment& Channel::preAmplifier() const {
	if ( _preAmplifier )
		return *_preAmplifier;
	throw Seiscomp::Core::ValueException("Channel.PreAmplifier is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setDataLogger(const OPT(Equipment)& dataLogger) {
	_dataLogger = dataLogger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment& Channel::dataLogger() {
	if ( _dataLogger )
		return *_dataLogger;
	throw Seiscomp::Core::ValueException("Channel.DataLogger is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Equipment& Channel::dataLogger() const {
	if ( _dataLogger )
		return *_dataLogger;
	throw Seiscomp::Core::ValueException("Channel.DataLogger is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setResponse(const OPT(Response)& response) {
	_response = response;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response& Channel::response() {
	if ( _response )
		return *_response;
	throw Seiscomp::Core::ValueException("Channel.Response is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Response& Channel::response() const {
	if ( _response )
		return *_response;
	throw Seiscomp::Core::ValueException("Channel.Response is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setLocationCode(const std::string& locationCode) {
	_locationCode = locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Channel::locationCode() const {
	return _locationCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel& Channel::operator=(const Channel &other) {
	BaseNode::operator=(other);
	_latitude = other._latitude;
	_longitude = other._longitude;
	_elevation = other._elevation;
	_depth = other._depth;
	_azimuth = other._azimuth;
	_dip = other._dip;
	_waterLevel = other._waterLevel;
	_sampleRate = other._sampleRate;
	_sampleRateRatio = other._sampleRateRatio;
	_clockDrift = other._clockDrift;
	_calibrationUnits = other._calibrationUnits;
	_sensor = other._sensor;
	_preAmplifier = other._preAmplifier;
	_dataLogger = other._dataLogger;
	_response = other._response;
	_locationCode = other._locationCode;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Channel::typeCount() const {
	return _types.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Output* Channel::type(size_t i) const {
	return _types[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::addType(Output *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_types.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeType(Output *obj) {
	if ( obj == NULL )
		return false;

	std::vector<OutputPtr>::iterator it;
	it = std::find(_types.begin(), _types.end(), obj);
	// Element has not been found
	if ( it == _types.end() ) {
		SEISCOMP_ERROR("Channel::removeType(Output*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeType(size_t i) {
	// index out of bounds
	if ( i >= _types.size() )
		return false;

	_types.erase(_types.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Channel::equipmentCount() const {
	return _equipments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment* Channel::equipment(size_t i) const {
	return _equipments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::addEquipment(Equipment *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_equipments.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeEquipment(Equipment *obj) {
	if ( obj == NULL )
		return false;

	std::vector<EquipmentPtr>::iterator it;
	it = std::find(_equipments.begin(), _equipments.end(), obj);
	// Element has not been found
	if ( it == _equipments.end() ) {
		SEISCOMP_ERROR("Channel::removeEquipment(Equipment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Channel::removeEquipment(size_t i) {
	// index out of bounds
	if ( i >= _equipments.size() )
		return false;

	_equipments.erase(_equipments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
