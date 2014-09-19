/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/responsestage.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


ResponseStage::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<PolesAndZeros>("PolesZeros", "FDSNXML::PolesAndZeros", false, true, &ResponseStage::setPolesZeros, &ResponseStage::polesZeros));
	addProperty(objectProperty<Coefficients>("Coefficients", "FDSNXML::Coefficients", false, true, &ResponseStage::setCoefficients, &ResponseStage::coefficients));
	addProperty(objectProperty<ResponseList>("ResponseList", "FDSNXML::ResponseList", false, true, &ResponseStage::setResponseList, &ResponseStage::responseList));
	addProperty(objectProperty<FIR>("FIR", "FDSNXML::FIR", false, true, &ResponseStage::setFIR, &ResponseStage::fIR));
	addProperty(objectProperty<Polynomial>("Polynomial", "FDSNXML::Polynomial", false, true, &ResponseStage::setPolynomial, &ResponseStage::polynomial));
	addProperty(objectProperty<Decimation>("Decimation", "FDSNXML::Decimation", false, true, &ResponseStage::setDecimation, &ResponseStage::decimation));
	addProperty(objectProperty<Gain>("StageGain", "FDSNXML::Gain", false, false, &ResponseStage::setStageGain, &ResponseStage::stageGain));
	addProperty(Core::simpleProperty("number", "int", false, false, false, false, false, false, NULL, &ResponseStage::setNumber, &ResponseStage::number));
	addProperty(Core::simpleProperty("resourceId", "string", false, false, false, false, false, false, NULL, &ResponseStage::setResourceId, &ResponseStage::resourceId));
}


IMPLEMENT_RTTI(ResponseStage, "FDSNXML::ResponseStage", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(ResponseStage)
IMPLEMENT_METAOBJECT(ResponseStage)


ResponseStage::ResponseStage() {
	_number = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseStage::ResponseStage(const ResponseStage &other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseStage::~ResponseStage() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ResponseStage::operator==(const ResponseStage &rhs) const {
	if ( !(_polesZeros == rhs._polesZeros) )
		return false;
	if ( !(_coefficients == rhs._coefficients) )
		return false;
	if ( !(_responseList == rhs._responseList) )
		return false;
	if ( !(_fIR == rhs._fIR) )
		return false;
	if ( !(_polynomial == rhs._polynomial) )
		return false;
	if ( !(_decimation == rhs._decimation) )
		return false;
	if ( !(_stageGain == rhs._stageGain) )
		return false;
	if ( !(_number == rhs._number) )
		return false;
	if ( !(_resourceId == rhs._resourceId) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setPolesZeros(const OPT(PolesAndZeros)& polesZeros) {
	_polesZeros = polesZeros;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolesAndZeros& ResponseStage::polesZeros() throw(Seiscomp::Core::ValueException) {
	if ( _polesZeros )
		return *_polesZeros;
	throw Seiscomp::Core::ValueException("ResponseStage.PolesZeros is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const PolesAndZeros& ResponseStage::polesZeros() const throw(Seiscomp::Core::ValueException) {
	if ( _polesZeros )
		return *_polesZeros;
	throw Seiscomp::Core::ValueException("ResponseStage.PolesZeros is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setCoefficients(const OPT(Coefficients)& coefficients) {
	_coefficients = coefficients;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients& ResponseStage::coefficients() throw(Seiscomp::Core::ValueException) {
	if ( _coefficients )
		return *_coefficients;
	throw Seiscomp::Core::ValueException("ResponseStage.Coefficients is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Coefficients& ResponseStage::coefficients() const throw(Seiscomp::Core::ValueException) {
	if ( _coefficients )
		return *_coefficients;
	throw Seiscomp::Core::ValueException("ResponseStage.Coefficients is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setResponseList(const OPT(ResponseList)& responseList) {
	_responseList = responseList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseList& ResponseStage::responseList() throw(Seiscomp::Core::ValueException) {
	if ( _responseList )
		return *_responseList;
	throw Seiscomp::Core::ValueException("ResponseStage.ResponseList is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponseList& ResponseStage::responseList() const throw(Seiscomp::Core::ValueException) {
	if ( _responseList )
		return *_responseList;
	throw Seiscomp::Core::ValueException("ResponseStage.ResponseList is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setFIR(const OPT(FIR)& fIR) {
	_fIR = fIR;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FIR& ResponseStage::fIR() throw(Seiscomp::Core::ValueException) {
	if ( _fIR )
		return *_fIR;
	throw Seiscomp::Core::ValueException("ResponseStage.FIR is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const FIR& ResponseStage::fIR() const throw(Seiscomp::Core::ValueException) {
	if ( _fIR )
		return *_fIR;
	throw Seiscomp::Core::ValueException("ResponseStage.FIR is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setPolynomial(const OPT(Polynomial)& polynomial) {
	_polynomial = polynomial;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial& ResponseStage::polynomial() throw(Seiscomp::Core::ValueException) {
	if ( _polynomial )
		return *_polynomial;
	throw Seiscomp::Core::ValueException("ResponseStage.Polynomial is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Polynomial& ResponseStage::polynomial() const throw(Seiscomp::Core::ValueException) {
	if ( _polynomial )
		return *_polynomial;
	throw Seiscomp::Core::ValueException("ResponseStage.Polynomial is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setDecimation(const OPT(Decimation)& decimation) {
	_decimation = decimation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation& ResponseStage::decimation() throw(Seiscomp::Core::ValueException) {
	if ( _decimation )
		return *_decimation;
	throw Seiscomp::Core::ValueException("ResponseStage.Decimation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Decimation& ResponseStage::decimation() const throw(Seiscomp::Core::ValueException) {
	if ( _decimation )
		return *_decimation;
	throw Seiscomp::Core::ValueException("ResponseStage.Decimation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setStageGain(const Gain& stageGain) {
	_stageGain = stageGain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Gain& ResponseStage::stageGain() {
	return _stageGain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Gain& ResponseStage::stageGain() const {
	return _stageGain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setNumber(int number) {
	_number = number;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ResponseStage::number() const {
	return _number;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ResponseStage::setResourceId(const std::string& resourceId) {
	_resourceId = resourceId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ResponseStage::resourceId() const {
	return _resourceId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseStage& ResponseStage::operator=(const ResponseStage &other) {
	_polesZeros = other._polesZeros;
	_coefficients = other._coefficients;
	_responseList = other._responseList;
	_fIR = other._fIR;
	_polynomial = other._polynomial;
	_decimation = other._decimation;
	_stageGain = other._stageGain;
	_number = other._number;
	_resourceId = other._resourceId;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
