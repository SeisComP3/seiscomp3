/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/polesandzeros.h>
#include <stationxml/poleandzero.h>
#include <stationxml/poleandzero.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<PzTransferFunctionType> metaPzTransferFunctionType;

}


PolesAndZeros::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<Comment>("Comment", "StationXML::Comment", false, true, &PolesAndZeros::setComment, &PolesAndZeros::comment));
	addProperty(Core::simpleProperty("InputUnits", "string", false, false, false, false, false, false, NULL, &PolesAndZeros::setInputUnits, &PolesAndZeros::inputUnits));
	addProperty(Core::simpleProperty("OutputUnits", "string", false, false, false, false, false, false, NULL, &PolesAndZeros::setOutputUnits, &PolesAndZeros::outputUnits));
	addProperty(enumProperty("PzTransferFunctionType", "PzTransferFunctionType", false, false, &metaPzTransferFunctionType, &PolesAndZeros::setPzTransferFunctionType, &PolesAndZeros::pzTransferFunctionType));
	addProperty(Core::simpleProperty("NormalizationFactor", "float", false, false, false, false, false, false, NULL, &PolesAndZeros::setNormalizationFactor, &PolesAndZeros::normalizationFactor));
	addProperty(objectProperty<FrequencyType>("NormalizationFreq", "StationXML::FrequencyType", false, false, &PolesAndZeros::setNormalizationFreq, &PolesAndZeros::normalizationFreq));
	addProperty(arrayClassProperty<PoleAndZero>("Pole", "StationXML::PoleAndZero", &PolesAndZeros::poleCount, &PolesAndZeros::pole, static_cast<bool (PolesAndZeros::*)(PoleAndZero*)>(&PolesAndZeros::addPole), &PolesAndZeros::removePole, static_cast<bool (PolesAndZeros::*)(PoleAndZero*)>(&PolesAndZeros::removePole)));
	addProperty(arrayClassProperty<PoleAndZero>("Zero", "StationXML::PoleAndZero", &PolesAndZeros::zeroCount, &PolesAndZeros::zero, static_cast<bool (PolesAndZeros::*)(PoleAndZero*)>(&PolesAndZeros::addZero), &PolesAndZeros::removeZero, static_cast<bool (PolesAndZeros::*)(PoleAndZero*)>(&PolesAndZeros::removeZero)));
}


IMPLEMENT_RTTI(PolesAndZeros, "StationXML::PolesAndZeros", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(PolesAndZeros)
IMPLEMENT_METAOBJECT(PolesAndZeros)


PolesAndZeros::PolesAndZeros() {
	_normalizationFactor = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolesAndZeros::PolesAndZeros(const PolesAndZeros& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolesAndZeros::~PolesAndZeros() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::operator==(const PolesAndZeros& rhs) const {
	if ( !(_comment == rhs._comment) )
		return false;
	if ( !(_inputUnits == rhs._inputUnits) )
		return false;
	if ( !(_outputUnits == rhs._outputUnits) )
		return false;
	if ( !(_pzTransferFunctionType == rhs._pzTransferFunctionType) )
		return false;
	if ( !(_normalizationFactor == rhs._normalizationFactor) )
		return false;
	if ( !(_normalizationFreq == rhs._normalizationFreq) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setComment(const OPT(Comment)& comment) {
	_comment = comment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment& PolesAndZeros::comment() throw(Seiscomp::Core::ValueException) {
	if ( _comment )
		return *_comment;
	throw Seiscomp::Core::ValueException("PolesAndZeros.Comment is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Comment& PolesAndZeros::comment() const throw(Seiscomp::Core::ValueException) {
	if ( _comment )
		return *_comment;
	throw Seiscomp::Core::ValueException("PolesAndZeros.Comment is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setInputUnits(const std::string& inputUnits) {
	_inputUnits = inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& PolesAndZeros::inputUnits() const {
	return _inputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setOutputUnits(const std::string& outputUnits) {
	_outputUnits = outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& PolesAndZeros::outputUnits() const {
	return _outputUnits;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setPzTransferFunctionType(PzTransferFunctionType pzTransferFunctionType) {
	_pzTransferFunctionType = pzTransferFunctionType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PzTransferFunctionType PolesAndZeros::pzTransferFunctionType() const {
	return _pzTransferFunctionType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setNormalizationFactor(double normalizationFactor) {
	_normalizationFactor = normalizationFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double PolesAndZeros::normalizationFactor() const {
	return _normalizationFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PolesAndZeros::setNormalizationFreq(const FrequencyType& normalizationFreq) {
	_normalizationFreq = normalizationFreq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FrequencyType& PolesAndZeros::normalizationFreq() {
	return _normalizationFreq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FrequencyType& PolesAndZeros::normalizationFreq() const {
	return _normalizationFreq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolesAndZeros& PolesAndZeros::operator=(const PolesAndZeros& other) {
	_comment = other._comment;
	_inputUnits = other._inputUnits;
	_outputUnits = other._outputUnits;
	_pzTransferFunctionType = other._pzTransferFunctionType;
	_normalizationFactor = other._normalizationFactor;
	_normalizationFreq = other._normalizationFreq;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t PolesAndZeros::poleCount() const {
	return _poles.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PoleAndZero* PolesAndZeros::pole(size_t i) const {
	return _poles[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::addPole(PoleAndZero* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_poles.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::removePole(PoleAndZero* obj) {
	if ( obj == NULL )
		return false;

	std::vector<PoleAndZeroPtr>::iterator it;
	it = std::find(_poles.begin(), _poles.end(), obj);
	// Element has not been found
	if ( it == _poles.end() ) {
		SEISCOMP_ERROR("PolesAndZeros::removePole(PoleAndZero*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::removePole(size_t i) {
	// index out of bounds
	if ( i >= _poles.size() )
		return false;

	_poles.erase(_poles.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t PolesAndZeros::zeroCount() const {
	return _zeros.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PoleAndZero* PolesAndZeros::zero(size_t i) const {
	return _zeros[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::addZero(PoleAndZero* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_zeros.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::removeZero(PoleAndZero* obj) {
	if ( obj == NULL )
		return false;

	std::vector<PoleAndZeroPtr>::iterator it;
	it = std::find(_zeros.begin(), _zeros.end(), obj);
	// Element has not been found
	if ( it == _zeros.end() ) {
		SEISCOMP_ERROR("PolesAndZeros::removeZero(PoleAndZero*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PolesAndZeros::removeZero(size_t i) {
	// index out of bounds
	if ( i >= _zeros.size() )
		return false;

	_zeros.erase(_zeros.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
