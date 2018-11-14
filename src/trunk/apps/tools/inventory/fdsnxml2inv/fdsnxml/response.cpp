/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#define SEISCOMP_COMPONENT SWE
#include <fdsnxml/response.h>
#include <fdsnxml/responsestage.h>
#include <algorithm>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace FDSNXML {


Response::MetaObject::MetaObject(const Core::RTTI *rtti, const Core::MetaObject *base) : Core::MetaObject(rtti, base) {
	addProperty(objectProperty<Sensitivity>("InstrumentSensitivity", "FDSNXML::Sensitivity", false, true, &Response::setInstrumentSensitivity, &Response::instrumentSensitivity));
	addProperty(objectProperty<Polynomial>("InstrumentPolynomial", "FDSNXML::Polynomial", false, true, &Response::setInstrumentPolynomial, &Response::instrumentPolynomial));
	addProperty(arrayClassProperty<ResponseStage>("Stage", "FDSNXML::ResponseStage", &Response::stageCount, &Response::stage, static_cast<bool (Response::*)(ResponseStage*)>(&Response::addStage), &Response::removeStage, static_cast<bool (Response::*)(ResponseStage*)>(&Response::removeStage)));
}


IMPLEMENT_RTTI(Response, "FDSNXML::Response", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Response)
IMPLEMENT_METAOBJECT(Response)


Response::Response() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response::Response(const Response &other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response::~Response() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::operator==(const Response &rhs) const {
	if ( !(_instrumentSensitivity == rhs._instrumentSensitivity) )
		return false;
	if ( !(_instrumentPolynomial == rhs._instrumentPolynomial) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setInstrumentSensitivity(const OPT(Sensitivity)& instrumentSensitivity) {
	_instrumentSensitivity = instrumentSensitivity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensitivity& Response::instrumentSensitivity() {
	if ( _instrumentSensitivity )
		return *_instrumentSensitivity;
	throw Seiscomp::Core::ValueException("Response.InstrumentSensitivity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Sensitivity& Response::instrumentSensitivity() const {
	if ( _instrumentSensitivity )
		return *_instrumentSensitivity;
	throw Seiscomp::Core::ValueException("Response.InstrumentSensitivity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Response::setInstrumentPolynomial(const OPT(Polynomial)& instrumentPolynomial) {
	_instrumentPolynomial = instrumentPolynomial;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Polynomial& Response::instrumentPolynomial() {
	if ( _instrumentPolynomial )
		return *_instrumentPolynomial;
	throw Seiscomp::Core::ValueException("Response.InstrumentPolynomial is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Polynomial& Response::instrumentPolynomial() const {
	if ( _instrumentPolynomial )
		return *_instrumentPolynomial;
	throw Seiscomp::Core::ValueException("Response.InstrumentPolynomial is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Response& Response::operator=(const Response &other) {
	_instrumentSensitivity = other._instrumentSensitivity;
	_instrumentPolynomial = other._instrumentPolynomial;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Response::stageCount() const {
	return _stages.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseStage* Response::stage(size_t i) const {
	return _stages[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::addStage(ResponseStage *obj) {
	if ( obj == NULL )
		return false;

	// Add the element
	_stages.push_back(obj);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeStage(ResponseStage *obj) {
	if ( obj == NULL )
		return false;

	std::vector<ResponseStagePtr>::iterator it;
	it = std::find(_stages.begin(), _stages.end(), obj);
	// Element has not been found
	if ( it == _stages.end() ) {
		SEISCOMP_ERROR("Response::removeStage(ResponseStage*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Response::removeStage(size_t i) {
	// index out of bounds
	if ( i >= _stages.size() )
		return false;

	_stages.erase(_stages.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
