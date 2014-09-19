/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <stationxml/response.h>
#include <stationxml/polesandzeros.h>
#include <stationxml/coefficients.h>
#include <stationxml/responselist.h>
#include <stationxml/genericresponse.h>
#include <stationxml/fir.h>
#include <stationxml/polynomial.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace StationXML {


namespace {

static Seiscomp::Core::MetaEnumImpl<StageType> metaStageType;

}


Response::MetaObject::MetaObject(const Core::RTTI* rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<Decimation>("Decimation", "StationXML::Decimation", false, true, &Response::setDecimation, &Response::decimation));
	addProperty(objectProperty<Sensitivity>("StageSensitivity", "StationXML::Sensitivity", false, false, &Response::setStageSensitivity, &Response::stageSensitivity));
	addProperty(objectProperty<Spectra>("Spectra", "StationXML::Spectra", false, true, &Response::setSpectra, &Response::spectra));
	addProperty(Core::simpleProperty("stage", "int", false, false, false, false, false, false, NULL, &Response::setStage, &Response::stage));
	addProperty(enumProperty("stageDescription", "StageType", false, true, &metaStageType, &Response::setStageDescription, &Response::stageDescription));
	addProperty(arrayClassProperty<PolesAndZeros>("PolesAndZeros", "StationXML::PolesAndZeros", &Response::polesAndZerosCount, &Response::polesAndZeros, static_cast<bool (Response::*)(PolesAndZeros*)>(&Response::addPolesAndZeros), &Response::removePolesAndZeros, static_cast<bool (Response::*)(PolesAndZeros*)>(&Response::removePolesAndZeros)));
	addProperty(arrayClassProperty<Coefficients>("Coefficients", "StationXML::Coefficients", &Response::coefficientsCount, &Response::coefficients, static_cast<bool (Response::*)(Coefficients*)>(&Response::addCoefficients), &Response::removeCoefficients, static_cast<bool (Response::*)(Coefficients*)>(&Response::removeCoefficients)));
	addProperty(arrayClassProperty<ResponseList>("ResponseList", "StationXML::ResponseList", &Response::responseListCount, &Response::responseList, static_cast<bool (Response::*)(ResponseList*)>(&Response::addResponseList), &Response::removeResponseList, static_cast<bool (Response::*)(ResponseList*)>(&Response::removeResponseList)));
	addProperty(arrayClassProperty<GenericResponse>("Generic", "StationXML::GenericResponse", &Response::genericCount, &Response::generic, static_cast<bool (Response::*)(GenericResponse*)>(&Response::addGeneric), &Response::removeGeneric, static_cast<bool (Response::*)(GenericResponse*)>(&Response::removeGeneric)));
	addProperty(arrayClassProperty<FIR>("FIR", "StationXML::FIR", &Response::fIRCount, &Response::fIR, static_cast<bool (Response::*)(FIR*)>(&Response::addFIR), &Response::removeFIR, static_cast<bool (Response::*)(FIR*)>(&Response::removeFIR)));
	addProperty(arrayClassProperty<Polynomial>("Polynomial", "StationXML::Polynomial", &Response::polynomialCount, &Response::polynomial, static_cast<bool (Response::*)(Polynomial*)>(&Response::addPolynomial), &Response::removePolynomial, static_cast<bool (Response::*)(Polynomial*)>(&Response::removePolynomial)));
}


IMPLEMENT_RTTI(Response, "StationXML::Response", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Response)
IMPLEMENT_METAOBJECT(Response)


Response::Response() {
	_stage = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response::Response(const Response& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response::~Response() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::operator==(const Response& rhs) const {
	if ( !(_decimation == rhs._decimation) )
		return false;
	if ( !(_stageSensitivity == rhs._stageSensitivity) )
		return false;
	if ( !(_spectra == rhs._spectra) )
		return false;
	if ( !(_stage == rhs._stage) )
		return false;
	if ( !(_stageDescription == rhs._stageDescription) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setDecimation(const OPT(Decimation)& decimation) {
	_decimation = decimation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation& Response::decimation() throw(Seiscomp::Core::ValueException) {
	if ( _decimation )
		return *_decimation;
	throw Seiscomp::Core::ValueException("Response.Decimation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Decimation& Response::decimation() const throw(Seiscomp::Core::ValueException) {
	if ( _decimation )
		return *_decimation;
	throw Seiscomp::Core::ValueException("Response.Decimation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setStageSensitivity(const Sensitivity& stageSensitivity) {
	_stageSensitivity = stageSensitivity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity& Response::stageSensitivity() {
	return _stageSensitivity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Sensitivity& Response::stageSensitivity() const {
	return _stageSensitivity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setSpectra(const OPT(Spectra)& spectra) {
	_spectra = spectra;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Spectra& Response::spectra() throw(Seiscomp::Core::ValueException) {
	if ( _spectra )
		return *_spectra;
	throw Seiscomp::Core::ValueException("Response.Spectra is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Spectra& Response::spectra() const throw(Seiscomp::Core::ValueException) {
	if ( _spectra )
		return *_spectra;
	throw Seiscomp::Core::ValueException("Response.Spectra is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setStage(int stage) {
	_stage = stage;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Response::stage() const {
	return _stage;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setStageDescription(const OPT(StageType)& stageDescription) {
	_stageDescription = stageDescription;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StageType Response::stageDescription() const throw(Seiscomp::Core::ValueException) {
	if ( _stageDescription )
		return *_stageDescription;
	throw Seiscomp::Core::ValueException("Response.stageDescription is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response& Response::operator=(const Response& other) {
	_decimation = other._decimation;
	_stageSensitivity = other._stageSensitivity;
	_spectra = other._spectra;
	_stage = other._stage;
	_stageDescription = other._stageDescription;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::polesAndZerosCount() const {
	return _polesAndZeross.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PolesAndZeros* Response::polesAndZeros(size_t i) const {
	return _polesAndZeross[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addPolesAndZeros(PolesAndZeros* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_polesAndZeross.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removePolesAndZeros(PolesAndZeros* obj) {
	if ( obj == NULL )
		return false;

	std::vector<PolesAndZerosPtr>::iterator it;
	it = std::find(_polesAndZeross.begin(), _polesAndZeross.end(), obj);
	// Element has not been found
	if ( it == _polesAndZeross.end() ) {
		SEISCOMP_ERROR("Response::removePolesAndZeros(PolesAndZeros*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removePolesAndZeros(size_t i) {
	// index out of bounds
	if ( i >= _polesAndZeross.size() )
		return false;

	_polesAndZeross.erase(_polesAndZeross.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::coefficientsCount() const {
	return _coefficientss.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Coefficients* Response::coefficients(size_t i) const {
	return _coefficientss[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addCoefficients(Coefficients* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_coefficientss.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeCoefficients(Coefficients* obj) {
	if ( obj == NULL )
		return false;

	std::vector<CoefficientsPtr>::iterator it;
	it = std::find(_coefficientss.begin(), _coefficientss.end(), obj);
	// Element has not been found
	if ( it == _coefficientss.end() ) {
		SEISCOMP_ERROR("Response::removeCoefficients(Coefficients*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeCoefficients(size_t i) {
	// index out of bounds
	if ( i >= _coefficientss.size() )
		return false;

	_coefficientss.erase(_coefficientss.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::responseListCount() const {
	return _responseLists.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseList* Response::responseList(size_t i) const {
	return _responseLists[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addResponseList(ResponseList* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_responseLists.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeResponseList(ResponseList* obj) {
	if ( obj == NULL )
		return false;

	std::vector<ResponseListPtr>::iterator it;
	it = std::find(_responseLists.begin(), _responseLists.end(), obj);
	// Element has not been found
	if ( it == _responseLists.end() ) {
		SEISCOMP_ERROR("Response::removeResponseList(ResponseList*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeResponseList(size_t i) {
	// index out of bounds
	if ( i >= _responseLists.size() )
		return false;

	_responseLists.erase(_responseLists.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::genericCount() const {
	return _generics.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GenericResponse* Response::generic(size_t i) const {
	return _generics[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addGeneric(GenericResponse* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_generics.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeGeneric(GenericResponse* obj) {
	if ( obj == NULL )
		return false;

	std::vector<GenericResponsePtr>::iterator it;
	it = std::find(_generics.begin(), _generics.end(), obj);
	// Element has not been found
	if ( it == _generics.end() ) {
		SEISCOMP_ERROR("Response::removeGeneric(GenericResponse*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeGeneric(size_t i) {
	// index out of bounds
	if ( i >= _generics.size() )
		return false;

	_generics.erase(_generics.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::fIRCount() const {
	return _fIRs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FIR* Response::fIR(size_t i) const {
	return _fIRs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addFIR(FIR* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_fIRs.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeFIR(FIR* obj) {
	if ( obj == NULL )
		return false;

	std::vector<FIRPtr>::iterator it;
	it = std::find(_fIRs.begin(), _fIRs.end(), obj);
	// Element has not been found
	if ( it == _fIRs.end() ) {
		SEISCOMP_ERROR("Response::removeFIR(FIR*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeFIR(size_t i) {
	// index out of bounds
	if ( i >= _fIRs.size() )
		return false;

	_fIRs.erase(_fIRs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::polynomialCount() const {
	return _polynomials.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial* Response::polynomial(size_t i) const {
	return _polynomials[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addPolynomial(Polynomial* obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_polynomials.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removePolynomial(Polynomial* obj) {
	if ( obj == NULL )
		return false;

	std::vector<PolynomialPtr>::iterator it;
	it = std::find(_polynomials.begin(), _polynomials.end(), obj);
	// Element has not been found
	if ( it == _polynomials.end() ) {
		SEISCOMP_ERROR("Response::removePolynomial(Polynomial*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removePolynomial(size_t i) {
	// index out of bounds
	if ( i >= _polynomials.size() )
		return false;

	_polynomials.erase(_polynomials.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
