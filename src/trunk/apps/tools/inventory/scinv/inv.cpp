/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/

#define SEISCOMP_COMPONENT INVMGR

#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>

#include "inv.h"

#include <iostream>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
InventoryTask::InventoryTask(Seiscomp::DataModel::Inventory *inv) : _inv(inv) {
	_logHandler = NULL;

	if ( _inv == NULL ) return;

	for ( size_t i = 0; i < _inv->dataloggerCount(); ++i ) {
		Datalogger *d = _inv->datalogger(i);
		_dataloggerNames[d->name()] = d;
	}

	for ( size_t i = 0; i < _inv->sensorCount(); ++i ) {
		Sensor *s = _inv->sensor(i);
		_sensorNames[s->name()] = s;
	}

	for ( size_t i = 0; i < _inv->auxDeviceCount(); ++i ) {
		AuxDevice *d = _inv->auxDevice(i);
		_auxDeviceNames[d->name()] = d;
	}

	for ( size_t i = 0; i < _inv->responseFIRCount(); ++i ) {
		ResponseFIR *r = _inv->responseFIR(i);
		_FIRNames[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responsePAZCount(); ++i ) {
		ResponsePAZ *r = _inv->responsePAZ(i);
		_PAZNames[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responsePolynomialCount(); ++i ) {
		ResponsePolynomial *r = _inv->responsePolynomial(i);
		_PolyNames[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responseFAPCount(); ++i ) {
		ResponseFAP *r = _inv->responseFAP(i);
		_FAPNames[r->name()] = r;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void InventoryTask::setLogHandler(LogHandler *handler) {
	_logHandler = handler;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void InventoryTask::log(LogHandler::Level level, const char *message,
                        const Seiscomp::DataModel::Object *obj1,
                        const Seiscomp::DataModel::Object *obj2) {
	if ( _logHandler == NULL ) return;
	_logHandler->publish(level, message, obj1, obj2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Datalogger *InventoryTask::dataloggerByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _dataloggerNames.find(name);
	if ( it == _dataloggerNames.end() ) return NULL;
	return (Datalogger*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *InventoryTask::sensorByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _sensorNames.find(name);
	if ( it == _sensorNames.end() ) return NULL;
	return (Sensor*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxDevice *InventoryTask::auxDeviceByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _auxDeviceNames.find(name);
	if ( it == _auxDeviceNames.end() ) return NULL;
	return (AuxDevice*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR *InventoryTask::respFIRByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _FIRNames.find(name);
	if ( it == _FIRNames.end() ) return NULL;
	return (ResponseFIR*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ *InventoryTask::respPAZByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _PAZNames.find(name);
	if ( it == _PAZNames.end() ) return NULL;
	return (ResponsePAZ*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP *InventoryTask::respFAPByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _FAPNames.find(name);
	if ( it == _FAPNames.end() ) return NULL;
	return (ResponseFAP*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial *InventoryTask::respPolynomialByName(const std::string &name) const {
	ObjectLookup::const_iterator it = _PolyNames.find(name);
	if ( it == _PolyNames.end() ) return NULL;
	return (ResponsePolynomial*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void InventoryTask::prepareSession(const Seiscomp::DataModel::Inventory *inv) {
	// Create lookup tables
	_session.dataloggerLookup.clear();
	for ( size_t i = 0; i < inv->dataloggerCount(); ++i ) {
		Datalogger *d = inv->datalogger(i);
		_session.dataloggerLookup[d->publicID()] = d;
	}

	_session.sensorLookup.clear();
	for ( size_t i = 0; i < inv->sensorCount(); ++i ) {
		Sensor *s = inv->sensor(i);
		_session.sensorLookup[s->publicID()] = s;
	}

	_session.auxDeviceLookup.clear();
	for ( size_t i = 0; i < inv->auxDeviceCount(); ++i ) {
		AuxDevice *d = inv->auxDevice(i);
		_session.auxDeviceLookup[d->publicID()] = d;
	}

	_session.firLookup.clear();
	for ( size_t i = 0; i < inv->responseFIRCount(); ++i ) {
		ResponseFIR *r = inv->responseFIR(i);
		_session.firLookup[r->publicID()] = r;
	}

	_session.pazLookup.clear();
	for ( size_t i = 0; i < inv->responsePAZCount(); ++i ) {
		ResponsePAZ *r = inv->responsePAZ(i);
		_session.pazLookup[r->publicID()] = r;
	}

	_session.polyLookup.clear();
	for ( size_t i = 0; i < inv->responsePolynomialCount(); ++i ) {
		ResponsePolynomial *r = inv->responsePolynomial(i);
		_session.polyLookup[r->publicID()] = r;
	}

	_session.fapLookup.clear();
	for ( size_t i = 0; i < inv->responseFAPCount(); ++i ) {
		ResponseFAP *r = inv->responseFAP(i);
		_session.fapLookup[r->publicID()] = r;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void InventoryTask::cleanUp() {
	_publicObjects.clear();
	_dataloggerNames.clear();
	_sensorNames.clear();
	_auxDeviceNames.clear();
	_FIRNames.clear();
	_PAZNames.clear();
	_PolyNames.clear();
	_FAPNames.clear();

	_session.dataloggerLookup.clear();
	_session.sensorLookup.clear();
	_session.auxDeviceLookup.clear();
	_session.firLookup.clear();
	_session.pazLookup.clear();
	_session.polyLookup.clear();
	_session.fapLookup.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Datalogger *InventoryTask::findDatalogger(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.dataloggerLookup.find(publicID);
	if ( it == _session.dataloggerLookup.end() ) return NULL;
	return (const Datalogger*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Sensor *InventoryTask::findSensor(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.sensorLookup.find(publicID);
	if ( it == _session.sensorLookup.end() ) return NULL;
	return (const Sensor*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AuxDevice *InventoryTask::findAuxDevice(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.auxDeviceLookup.find(publicID);
	if ( it == _session.auxDeviceLookup.end() ) return NULL;
	return (const AuxDevice*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponseFIR *InventoryTask::findFIR(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.firLookup.find(publicID);
	if ( it == _session.firLookup.end() ) return NULL;
	return (const ResponseFIR*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponsePAZ *InventoryTask::findPAZ(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.pazLookup.find(publicID);
	if ( it == _session.pazLookup.end() ) return NULL;
	return (const ResponsePAZ*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponsePolynomial *InventoryTask::findPoly(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.polyLookup.find(publicID);
	if ( it == _session.polyLookup.end() ) return NULL;
	return (const ResponsePolynomial*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ResponseFAP *InventoryTask::findFAP(const std::string &publicID) const {
	ObjectLookup::const_iterator it = _session.fapLookup.find(publicID);
	if ( it == _session.fapLookup.end() ) return NULL;
	return (const ResponseFAP*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR *InventoryTask::process(const ResponseFIR *fir) {
	ResponseFIRPtr sc_fir = respFIRByName(fir->name());
	//ResponseFIRPtr sc_fir = _inv->responseFIR(fir->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_fir ) {
		sc_fir = create<ResponseFIR>(fir->publicID());
		newInstance = true;
	}
	else
		needUpdate = !sc_fir->equal(*fir);

	*sc_fir = *fir;

	if ( newInstance ) {
		_FIRNames[sc_fir->name()] = sc_fir.get();
		_inv->add(sc_fir.get());
	}
	else if ( needUpdate )
		sc_fir->update();

	return sc_fir.get();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ *InventoryTask::process(const ResponsePAZ *paz) {
	ResponsePAZPtr sc_paz = respPAZByName(paz->name());
	//ResponsePAZPtr sc_paz = _inv->responsePAZ(paz->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_paz ) {
		sc_paz = create<ResponsePAZ>(paz->publicID());
		newInstance = true;
	}
	else
		needUpdate = !sc_paz->equal(*paz);

	*sc_paz = *paz;

	if ( newInstance ) {
		_PAZNames[sc_paz->name()] = sc_paz.get();
		_inv->add(sc_paz.get());
	}
	else if ( needUpdate )
		sc_paz->update();

	return sc_paz.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial *InventoryTask::process(const ResponsePolynomial *poly) {
	ResponsePolynomialPtr sc_poly = respPolynomialByName(poly->name());
	//ResponsePolynomialPtr sc_poly = _inv->responsePolynomial(poly->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_poly ) {
		sc_poly = create<ResponsePolynomial>(poly->publicID());
		newInstance = true;
	}
	else
		needUpdate = !sc_poly->equal(*poly);

	*sc_poly = *poly;

	if ( newInstance ) {
		_PolyNames[sc_poly->name()] = sc_poly.get();
		_inv->add(sc_poly.get());
	}
	else if ( needUpdate )
		sc_poly->update();

	return sc_poly.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP *InventoryTask::process(const ResponseFAP *fap) {
	ResponseFAPPtr sc_fap = respFAPByName(fap->name());
	//ResponseFAPPtr sc_fap = _inv->responseFAP(fap->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_fap ) {
		sc_fap = create<ResponseFAP>(fap->publicID());
		newInstance = true;
	}
	else
		needUpdate = !sc_fap->equal(*fap);

	*sc_fap = *fap;

	if ( newInstance ) {
		_FAPNames[sc_fap->name()] = sc_fap.get();
		_inv->add(sc_fap.get());
	}
	else if ( needUpdate )
		sc_fap->update();

	return sc_fap.get();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
