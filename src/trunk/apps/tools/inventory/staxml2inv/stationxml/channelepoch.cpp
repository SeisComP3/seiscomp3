/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/channelepoch.h>
#include <stationxml/output.h>
#include <stationxml/response.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


ChannelEpoch::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<LatType>("latitude", "StationXML::LatType", false, false, &ChannelEpoch::setLatitude, &ChannelEpoch::latitude));
	addProperty(objectProperty<LonType>("longitude", "StationXML::LonType", false, false, &ChannelEpoch::setLongitude, &ChannelEpoch::longitude));
	addProperty(objectProperty<DistanceType>("elevation", "StationXML::DistanceType", false, false, &ChannelEpoch::setElevation, &ChannelEpoch::elevation));
	addProperty(objectProperty<DistanceType>("depth", "StationXML::DistanceType", false, false, &ChannelEpoch::setDepth, &ChannelEpoch::depth));
	addProperty(objectProperty<AzimuthType>("azimuth", "StationXML::AzimuthType", false, true, &ChannelEpoch::setAzimuth, &ChannelEpoch::azimuth));
	addProperty(objectProperty<DipType>("dip", "StationXML::DipType", false, true, &ChannelEpoch::setDip, &ChannelEpoch::dip));
	addProperty(objectProperty<Offset>("offset", "StationXML::Offset", false, true, &ChannelEpoch::setOffset, &ChannelEpoch::offset));
	addProperty(objectProperty<SampleRateType>("SampleRate", "StationXML::SampleRateType", false, true, &ChannelEpoch::setSampleRate, &ChannelEpoch::sampleRate));
	addProperty(objectProperty<SampleRateRatioType>("SampleRateRatio", "StationXML::SampleRateRatioType", false, true, &ChannelEpoch::setSampleRateRatio, &ChannelEpoch::sampleRateRatio));
	addProperty(Core::simpleProperty("StorageFormat", "string", false, false, false, false, false, false, NULL, &ChannelEpoch::setStorageFormat, &ChannelEpoch::storageFormat));
	addProperty(objectProperty<ClockDriftType>("ClockDrift", "StationXML::ClockDriftType", false, true, &ChannelEpoch::setClockDrift, &ChannelEpoch::clockDrift));
	addProperty(Core::simpleProperty("CalibrationUnit", "string", false, false, false, false, false, false, NULL, &ChannelEpoch::setCalibrationUnit, &ChannelEpoch::calibrationUnit));
	addProperty(objectProperty<Datalogger>("datalogger", "StationXML::Datalogger", false, true, &ChannelEpoch::setDatalogger, &ChannelEpoch::datalogger));
	addProperty(objectProperty<Equipment>("Sensor", "StationXML::Equipment", false, true, &ChannelEpoch::setSensor, &ChannelEpoch::sensor));
	addProperty(objectProperty<Equipment>("PreAmplifier", "StationXML::Equipment", false, true, &ChannelEpoch::setPreAmplifier, &ChannelEpoch::preAmplifier));
	addProperty(objectProperty<Sensitivity>("InstrumentSensitivity", "StationXML::Sensitivity", false, true, &ChannelEpoch::setInstrumentSensitivity, &ChannelEpoch::instrumentSensitivity));
	addProperty(Core::simpleProperty("DampingConstant", "float", false, false, false, false, true, false, NULL, &ChannelEpoch::setDampingConstant, &ChannelEpoch::dampingConstant));
	addProperty(objectProperty<FrequencyType>("NaturalFrequency", "StationXML::FrequencyType", false, true, &ChannelEpoch::setNaturalFrequency, &ChannelEpoch::naturalFrequency));
	addProperty(objectProperty<LeastSignificantBitType>("LeastSignificantBit", "StationXML::LeastSignificantBitType", false, true, &ChannelEpoch::setLeastSignificantBit, &ChannelEpoch::leastSignificantBit));
	addProperty(objectProperty<VoltageType>("FullScaleInput", "StationXML::VoltageType", false, true, &ChannelEpoch::setFullScaleInput, &ChannelEpoch::fullScaleInput));
	addProperty(objectProperty<VoltageType>("FullScaleOutput", "StationXML::VoltageType", false, true, &ChannelEpoch::setFullScaleOutput, &ChannelEpoch::fullScaleOutput));
	addProperty(objectProperty<VoltageType>("FullScaleCapability", "StationXML::VoltageType", false, true, &ChannelEpoch::setFullScaleCapability, &ChannelEpoch::fullScaleCapability));
	addProperty(objectProperty<SecondType>("PreTriggerTime", "StationXML::SecondType", false, true, &ChannelEpoch::setPreTriggerTime, &ChannelEpoch::preTriggerTime));
	addProperty(objectProperty<SecondType>("PostDetriggerTime", "StationXML::SecondType", false, true, &ChannelEpoch::setPostDetriggerTime, &ChannelEpoch::postDetriggerTime));
	addProperty(arrayClassProperty<Output>("Output", "StationXML::Output", &ChannelEpoch::outputCount, &ChannelEpoch::output, static_cast<bool (ChannelEpoch::*)(Output*)>(&ChannelEpoch::addOutput), &ChannelEpoch::removeOutput, static_cast<bool (ChannelEpoch::*)(Output*)>(&ChannelEpoch::removeOutput)));
	addProperty(arrayClassProperty<Response>("Response", "StationXML::Response", &ChannelEpoch::responseCount, &ChannelEpoch::response, static_cast<bool (ChannelEpoch::*)(Response*)>(&ChannelEpoch::addResponse), &ChannelEpoch::removeResponse, static_cast<bool (ChannelEpoch::*)(Response*)>(&ChannelEpoch::removeResponse)));
}


IMPLEMENT_RTTI(ChannelEpoch, "StationXML::ChannelEpoch", Epoch)
IMPLEMENT_RTTI_METHODS(ChannelEpoch)
IMPLEMENT_METAOBJECT_DERIVED(ChannelEpoch, Epoch)


ChannelEpoch::ChannelEpoch() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ChannelEpoch::ChannelEpoch(const ChannelEpoch& other)
 : Epoch() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ChannelEpoch::~ChannelEpoch() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::operator==(const ChannelEpoch& rhs) const {
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
	if ( !(_offset == rhs._offset) )
		return false;
	if ( !(_sampleRate == rhs._sampleRate) )
		return false;
	if ( !(_sampleRateRatio == rhs._sampleRateRatio) )
		return false;
	if ( !(_storageFormat == rhs._storageFormat) )
		return false;
	if ( !(_clockDrift == rhs._clockDrift) )
		return false;
	if ( !(_calibrationUnit == rhs._calibrationUnit) )
		return false;
	if ( !(_datalogger == rhs._datalogger) )
		return false;
	if ( !(_sensor == rhs._sensor) )
		return false;
	if ( !(_preAmplifier == rhs._preAmplifier) )
		return false;
	if ( !(_instrumentSensitivity == rhs._instrumentSensitivity) )
		return false;
	if ( !(_dampingConstant == rhs._dampingConstant) )
		return false;
	if ( !(_naturalFrequency == rhs._naturalFrequency) )
		return false;
	if ( !(_leastSignificantBit == rhs._leastSignificantBit) )
		return false;
	if ( !(_fullScaleInput == rhs._fullScaleInput) )
		return false;
	if ( !(_fullScaleOutput == rhs._fullScaleOutput) )
		return false;
	if ( !(_fullScaleCapability == rhs._fullScaleCapability) )
		return false;
	if ( !(_preTriggerTime == rhs._preTriggerTime) )
		return false;
	if ( !(_postDetriggerTime == rhs._postDetriggerTime) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setLatitude(const LatType& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LatType& ChannelEpoch::latitude() {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LatType& ChannelEpoch::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setLongitude(const LonType& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LonType& ChannelEpoch::longitude() {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LonType& ChannelEpoch::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setElevation(const DistanceType& elevation) {
	_elevation = elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& ChannelEpoch::elevation() {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& ChannelEpoch::elevation() const {
	return _elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setDepth(const DistanceType& depth) {
	_depth = depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DistanceType& ChannelEpoch::depth() {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DistanceType& ChannelEpoch::depth() const {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setAzimuth(const OPT(AzimuthType)& azimuth) {
	_azimuth = azimuth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AzimuthType& ChannelEpoch::azimuth() throw(Seiscomp::Core::ValueException) {
	if ( _azimuth )
		return *_azimuth;
	throw Seiscomp::Core::ValueException("ChannelEpoch.azimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AzimuthType& ChannelEpoch::azimuth() const throw(Seiscomp::Core::ValueException) {
	if ( _azimuth )
		return *_azimuth;
	throw Seiscomp::Core::ValueException("ChannelEpoch.azimuth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setDip(const OPT(DipType)& dip) {
	_dip = dip;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DipType& ChannelEpoch::dip() throw(Seiscomp::Core::ValueException) {
	if ( _dip )
		return *_dip;
	throw Seiscomp::Core::ValueException("ChannelEpoch.dip is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DipType& ChannelEpoch::dip() const throw(Seiscomp::Core::ValueException) {
	if ( _dip )
		return *_dip;
	throw Seiscomp::Core::ValueException("ChannelEpoch.dip is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setOffset(const OPT(Offset)& offset) {
	_offset = offset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Offset& ChannelEpoch::offset() throw(Seiscomp::Core::ValueException) {
	if ( _offset )
		return *_offset;
	throw Seiscomp::Core::ValueException("ChannelEpoch.offset is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Offset& ChannelEpoch::offset() const throw(Seiscomp::Core::ValueException) {
	if ( _offset )
		return *_offset;
	throw Seiscomp::Core::ValueException("ChannelEpoch.offset is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setSampleRate(const OPT(SampleRateType)& sampleRate) {
	_sampleRate = sampleRate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateType& ChannelEpoch::sampleRate() throw(Seiscomp::Core::ValueException) {
	if ( _sampleRate )
		return *_sampleRate;
	throw Seiscomp::Core::ValueException("ChannelEpoch.SampleRate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SampleRateType& ChannelEpoch::sampleRate() const throw(Seiscomp::Core::ValueException) {
	if ( _sampleRate )
		return *_sampleRate;
	throw Seiscomp::Core::ValueException("ChannelEpoch.SampleRate is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setSampleRateRatio(const OPT(SampleRateRatioType)& sampleRateRatio) {
	_sampleRateRatio = sampleRateRatio;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SampleRateRatioType& ChannelEpoch::sampleRateRatio() throw(Seiscomp::Core::ValueException) {
	if ( _sampleRateRatio )
		return *_sampleRateRatio;
	throw Seiscomp::Core::ValueException("ChannelEpoch.SampleRateRatio is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SampleRateRatioType& ChannelEpoch::sampleRateRatio() const throw(Seiscomp::Core::ValueException) {
	if ( _sampleRateRatio )
		return *_sampleRateRatio;
	throw Seiscomp::Core::ValueException("ChannelEpoch.SampleRateRatio is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setStorageFormat(const std::string& storageFormat) {
	_storageFormat = storageFormat;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ChannelEpoch::storageFormat() const {
	return _storageFormat;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setClockDrift(const OPT(ClockDriftType)& clockDrift) {
	_clockDrift = clockDrift;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClockDriftType& ChannelEpoch::clockDrift() throw(Seiscomp::Core::ValueException) {
	if ( _clockDrift )
		return *_clockDrift;
	throw Seiscomp::Core::ValueException("ChannelEpoch.ClockDrift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClockDriftType& ChannelEpoch::clockDrift() const throw(Seiscomp::Core::ValueException) {
	if ( _clockDrift )
		return *_clockDrift;
	throw Seiscomp::Core::ValueException("ChannelEpoch.ClockDrift is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setCalibrationUnit(const std::string& calibrationUnit) {
	_calibrationUnit = calibrationUnit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ChannelEpoch::calibrationUnit() const {
	return _calibrationUnit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setDatalogger(const OPT(Datalogger)& datalogger) {
	_datalogger = datalogger;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger& ChannelEpoch::datalogger() throw(Seiscomp::Core::ValueException) {
	if ( _datalogger )
		return *_datalogger;
	throw Seiscomp::Core::ValueException("ChannelEpoch.datalogger is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Datalogger& ChannelEpoch::datalogger() const throw(Seiscomp::Core::ValueException) {
	if ( _datalogger )
		return *_datalogger;
	throw Seiscomp::Core::ValueException("ChannelEpoch.datalogger is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setSensor(const OPT(Equipment)& sensor) {
	_sensor = sensor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment& ChannelEpoch::sensor() throw(Seiscomp::Core::ValueException) {
	if ( _sensor )
		return *_sensor;
	throw Seiscomp::Core::ValueException("ChannelEpoch.Sensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Equipment& ChannelEpoch::sensor() const throw(Seiscomp::Core::ValueException) {
	if ( _sensor )
		return *_sensor;
	throw Seiscomp::Core::ValueException("ChannelEpoch.Sensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setPreAmplifier(const OPT(Equipment)& preAmplifier) {
	_preAmplifier = preAmplifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Equipment& ChannelEpoch::preAmplifier() throw(Seiscomp::Core::ValueException) {
	if ( _preAmplifier )
		return *_preAmplifier;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PreAmplifier is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Equipment& ChannelEpoch::preAmplifier() const throw(Seiscomp::Core::ValueException) {
	if ( _preAmplifier )
		return *_preAmplifier;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PreAmplifier is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setInstrumentSensitivity(const OPT(Sensitivity)& instrumentSensitivity) {
	_instrumentSensitivity = instrumentSensitivity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity& ChannelEpoch::instrumentSensitivity() throw(Seiscomp::Core::ValueException) {
	if ( _instrumentSensitivity )
		return *_instrumentSensitivity;
	throw Seiscomp::Core::ValueException("ChannelEpoch.InstrumentSensitivity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Sensitivity& ChannelEpoch::instrumentSensitivity() const throw(Seiscomp::Core::ValueException) {
	if ( _instrumentSensitivity )
		return *_instrumentSensitivity;
	throw Seiscomp::Core::ValueException("ChannelEpoch.InstrumentSensitivity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setDampingConstant(const OPT(double)& dampingConstant) {
	_dampingConstant = dampingConstant;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ChannelEpoch::dampingConstant() const throw(Seiscomp::Core::ValueException) {
	if ( _dampingConstant )
		return *_dampingConstant;
	throw Seiscomp::Core::ValueException("ChannelEpoch.DampingConstant is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setNaturalFrequency(const OPT(FrequencyType)& naturalFrequency) {
	_naturalFrequency = naturalFrequency;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& ChannelEpoch::naturalFrequency() throw(Seiscomp::Core::ValueException) {
	if ( _naturalFrequency )
		return *_naturalFrequency;
	throw Seiscomp::Core::ValueException("ChannelEpoch.NaturalFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& ChannelEpoch::naturalFrequency() const throw(Seiscomp::Core::ValueException) {
	if ( _naturalFrequency )
		return *_naturalFrequency;
	throw Seiscomp::Core::ValueException("ChannelEpoch.NaturalFrequency is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setLeastSignificantBit(const OPT(LeastSignificantBitType)& leastSignificantBit) {
	_leastSignificantBit = leastSignificantBit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeastSignificantBitType& ChannelEpoch::leastSignificantBit() throw(Seiscomp::Core::ValueException) {
	if ( _leastSignificantBit )
		return *_leastSignificantBit;
	throw Seiscomp::Core::ValueException("ChannelEpoch.LeastSignificantBit is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LeastSignificantBitType& ChannelEpoch::leastSignificantBit() const throw(Seiscomp::Core::ValueException) {
	if ( _leastSignificantBit )
		return *_leastSignificantBit;
	throw Seiscomp::Core::ValueException("ChannelEpoch.LeastSignificantBit is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setFullScaleInput(const OPT(VoltageType)& fullScaleInput) {
	_fullScaleInput = fullScaleInput;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VoltageType& ChannelEpoch::fullScaleInput() throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleInput )
		return *_fullScaleInput;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleInput is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const VoltageType& ChannelEpoch::fullScaleInput() const throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleInput )
		return *_fullScaleInput;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleInput is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setFullScaleOutput(const OPT(VoltageType)& fullScaleOutput) {
	_fullScaleOutput = fullScaleOutput;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VoltageType& ChannelEpoch::fullScaleOutput() throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleOutput )
		return *_fullScaleOutput;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleOutput is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const VoltageType& ChannelEpoch::fullScaleOutput() const throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleOutput )
		return *_fullScaleOutput;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleOutput is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setFullScaleCapability(const OPT(VoltageType)& fullScaleCapability) {
	_fullScaleCapability = fullScaleCapability;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VoltageType& ChannelEpoch::fullScaleCapability() throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleCapability )
		return *_fullScaleCapability;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleCapability is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const VoltageType& ChannelEpoch::fullScaleCapability() const throw(Seiscomp::Core::ValueException) {
	if ( _fullScaleCapability )
		return *_fullScaleCapability;
	throw Seiscomp::Core::ValueException("ChannelEpoch.FullScaleCapability is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setPreTriggerTime(const OPT(SecondType)& preTriggerTime) {
	_preTriggerTime = preTriggerTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SecondType& ChannelEpoch::preTriggerTime() throw(Seiscomp::Core::ValueException) {
	if ( _preTriggerTime )
		return *_preTriggerTime;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PreTriggerTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SecondType& ChannelEpoch::preTriggerTime() const throw(Seiscomp::Core::ValueException) {
	if ( _preTriggerTime )
		return *_preTriggerTime;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PreTriggerTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ChannelEpoch::setPostDetriggerTime(const OPT(SecondType)& postDetriggerTime) {
	_postDetriggerTime = postDetriggerTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SecondType& ChannelEpoch::postDetriggerTime() throw(Seiscomp::Core::ValueException) {
	if ( _postDetriggerTime )
		return *_postDetriggerTime;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PostDetriggerTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SecondType& ChannelEpoch::postDetriggerTime() const throw(Seiscomp::Core::ValueException) {
	if ( _postDetriggerTime )
		return *_postDetriggerTime;
	throw Seiscomp::Core::ValueException("ChannelEpoch.PostDetriggerTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ChannelEpoch& ChannelEpoch::operator=(const ChannelEpoch& other) {
	Epoch::operator=(other);
	_latitude = other._latitude;
	_longitude = other._longitude;
	_elevation = other._elevation;
	_depth = other._depth;
	_azimuth = other._azimuth;
	_dip = other._dip;
	_offset = other._offset;
	_sampleRate = other._sampleRate;
	_sampleRateRatio = other._sampleRateRatio;
	_storageFormat = other._storageFormat;
	_clockDrift = other._clockDrift;
	_calibrationUnit = other._calibrationUnit;
	_datalogger = other._datalogger;
	_sensor = other._sensor;
	_preAmplifier = other._preAmplifier;
	_instrumentSensitivity = other._instrumentSensitivity;
	_dampingConstant = other._dampingConstant;
	_naturalFrequency = other._naturalFrequency;
	_leastSignificantBit = other._leastSignificantBit;
	_fullScaleInput = other._fullScaleInput;
	_fullScaleOutput = other._fullScaleOutput;
	_fullScaleCapability = other._fullScaleCapability;
	_preTriggerTime = other._preTriggerTime;
	_postDetriggerTime = other._postDetriggerTime;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ChannelEpoch::outputCount() const {
	return _outputs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Output* ChannelEpoch::output(size_t i) const {
	return _outputs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::addOutput(Output* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_outputs.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::removeOutput(Output* obj) {
	if ( obj == NULL )
		return false;

	std::vector<OutputPtr>::iterator it;
	it = std::find(_outputs.begin(), _outputs.end(), obj);
	// Element has not been found
	if ( it == _outputs.end() ) {
		SEISCOMP_ERROR("ChannelEpoch::removeOutput(Output*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::removeOutput(size_t i) {
	// index out of bounds
	if ( i >= _outputs.size() )
		return false;

	_outputs.erase(_outputs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ChannelEpoch::responseCount() const {
	return _responses.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response* ChannelEpoch::response(size_t i) const {
	return _responses[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::addResponse(Response* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_responses.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::removeResponse(Response* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ResponsePtr>::iterator it;
	it = std::find(_responses.begin(), _responses.end(), obj);
	// Element has not been found
	if ( it == _responses.end() ) {
		SEISCOMP_ERROR("ChannelEpoch::removeResponse(Response*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ChannelEpoch::removeResponse(size_t i) {
	// index out of bounds
	if ( i >= _responses.size() )
		return false;

	_responses.erase(_responses.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
