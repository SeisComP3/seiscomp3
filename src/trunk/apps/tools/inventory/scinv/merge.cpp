/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/

#define SEISCOMP_COMPONENT INVMGR

#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>

#include "merge.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;


namespace {


string id(const Network *obj) {
	return obj->code();
}

string id(const Station *obj) {
	return id(obj->network()) + "." + obj->code();
}

string id(const SensorLocation *obj) {
	return id(obj->station()) + "." + obj->code();
}

string id(const Stream *obj) {
	return id(obj->sensorLocation()) + "." + obj->code();
}

string id(const PublicObject *obj) {
	return obj->publicID();
}

string id(const Core::Time &t) {
	if ( t.microseconds() > 0 )
		return t.toString("%F %T.%f");
	else
		return t.toString("%F %T");
}


Network *findNetwork(Inventory *inv, const string &code,
                     const Core::Time &start, const OPT(Core::Time)&end ) {
	for ( size_t i = 0; i < inv->networkCount(); ++i ) {
		Network *net = inv->network(i);
		if ( net->code() != code ) continue;

		// Check for overlapping time windows
		if ( start < net->start() ) continue;
		OPT(Core::Time) net_end;
		try { net_end = net->end(); } catch ( ... ) {}

		// Network time window open, ok
		if ( !net_end ) return net;
		// Epoch time window open, not ok
		if ( !end ) continue;

		// Epoch time window end greater than network end, not ok
		if ( *end > *net_end ) continue;

		return net;
	}

	return NULL;
}


// Macro to backup the result of an optional value
#define BCK(name, type, query) \
	OPT(type) name;\
	try { name = query; } catch ( ... ) {}

#define COMPARE_AND_RETURN(type, inst1, inst2, query) \
	{\
		BCK(tmp1, type, inst1->query)\
		BCK(tmp2, type, inst2->query)\
		if ( tmp1 != tmp2 ) return false;\
	}


bool equal(const ResponseFIR *f1, const ResponseFIR *f2) {
	COMPARE_AND_RETURN(double, f1, f2, gain())
	COMPARE_AND_RETURN(int, f1, f2, decimationFactor())
	COMPARE_AND_RETURN(double, f1, f2, delay())
	COMPARE_AND_RETURN(double, f1, f2, correction())
	COMPARE_AND_RETURN(int, f1, f2, numberOfCoefficients())

	if ( f1->symmetry() != f2->symmetry() ) return false;

	const RealArray *coeff1 = NULL;
	const RealArray *coeff2 = NULL;

	try { coeff1 = &f1->coefficients(); } catch ( ... ) {}
	try { coeff2 = &f2->coefficients(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!coeff1 && coeff2) || (coeff1 && !coeff2) ) return false;

	// Both unset?
	if ( !coeff1 && !coeff2 ) return true;

	// Both set, compare content
	const vector<double> &c1 = coeff1->content();
	const vector<double> &c2 = coeff2->content();
	if ( c1.size() != c2.size() ) return false;
	for ( size_t i = 0; i < c1.size(); ++i )
		if ( c1[i] != c2[i] ) return false;

	return true;
}



bool equal(const ResponsePAZ *p1, const ResponsePAZ *p2) {
	if ( p1->type() != p2->type() ) return false;

	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())
	COMPARE_AND_RETURN(double, p1, p2, normalizationFactor())
	COMPARE_AND_RETURN(double, p1, p2, normalizationFrequency())
	COMPARE_AND_RETURN(int, p1, p2, numberOfPoles())
	COMPARE_AND_RETURN(int, p1, p2, numberOfZeros())

	// Compare poles
	const ComplexArray *poles1 = NULL;
	const ComplexArray *poles2 = NULL;

	try { poles1 = &p1->poles(); } catch ( ... ) {}
	try { poles2 = &p2->poles(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!poles1 && poles2) || (poles1 && !poles2) ) return false;

	// Both unset?
	if ( !poles1 && !poles2 ) return true;

	// Both set, compare content
	const vector< complex<double> > &pc1 = poles1->content();
	const vector< complex<double> > &pc2 = poles2->content();

	if ( pc1.size() != pc2.size() ) return false;
	for ( size_t i = 0; i < pc1.size(); ++i )
		if ( pc1[i] != pc2[i] ) return false;

	// Compare zeros
	const ComplexArray *zeros1 = NULL;
	const ComplexArray *zeros2 = NULL;

	try { zeros1 = &p1->zeros(); } catch ( ... ) {}
	try { zeros2 = &p2->zeros(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!zeros1 && zeros2) || (zeros1 && !zeros2) ) return false;

	// Both unset?
	if ( !zeros1 && !zeros2 ) return true;

	// Both set, compare content
	const vector< complex<double> > &zc1 = zeros1->content();
	const vector< complex<double> > &zc2 = zeros2->content();

	if ( zc1.size() != zc2.size() ) return false;
	for ( size_t i = 0; i < zc1.size(); ++i )
		if ( zc1[i] != zc2[i] ) return false;

	// Everything is equal
	return true;
}


bool equal(const ResponsePolynomial *p1, const ResponsePolynomial *p2) {
	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())

	if ( p1->frequencyUnit() != p2->frequencyUnit() ) return false;
	if ( p1->approximationType() != p2->approximationType() ) return false;

	COMPARE_AND_RETURN(double, p1, p2, approximationLowerBound())
	COMPARE_AND_RETURN(double, p1, p2, approximationUpperBound())
	COMPARE_AND_RETURN(double, p1, p2, approximationError())
	COMPARE_AND_RETURN(int, p1, p2, numberOfCoefficients())

	const RealArray *coeff1 = NULL;
	const RealArray *coeff2 = NULL;

	try { coeff1 = &p1->coefficients(); } catch ( ... ) {}
	try { coeff2 = &p2->coefficients(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!coeff1 && coeff2) || (coeff1 && !coeff2) ) return false;

	// Both unset?
	if ( !coeff1 && !coeff2 ) return true;

	// Both set, compare content
	const vector<double> &c1 = coeff1->content();
	const vector<double> &c2 = coeff2->content();
	if ( c1.size() != c2.size() ) return false;
	for ( size_t i = 0; i < c1.size(); ++i )
		if ( c1[i] != c2[i] ) return false;

	return true;
}


bool equal(const ResponseFAP *p1, const ResponseFAP *p2) {
	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())

	COMPARE_AND_RETURN(int, p1, p2, numberOfTuples())

	const RealArray *coeff1 = NULL;
	const RealArray *coeff2 = NULL;

	try { coeff1 = &p1->tuples(); } catch ( ... ) {}
	try { coeff2 = &p2->tuples(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!coeff1 && coeff2) || (coeff1 && !coeff2) ) return false;

	// Both unset?
	if ( !coeff1 && !coeff2 ) return true;

	// Both set, compare content
	const vector<double> &c1 = coeff1->content();
	const vector<double> &c2 = coeff2->content();
	if ( c1.size() != c2.size() ) return false;
	for ( size_t i = 0; i < c1.size(); ++i )
		if ( c1[i] != c2[i] ) return false;

	return true;
}


bool equal(const Datalogger *d1, const Datalogger *d2) {
	if ( d1->description() != d2->description() ) return false;
	if ( d1->digitizerModel() != d2->digitizerModel() ) return false;
	if ( d1->digitizerManufacturer() != d2->digitizerManufacturer() ) return false;
	if ( d1->recorderModel() != d2->recorderModel() ) return false;
	if ( d1->recorderManufacturer() != d2->recorderManufacturer() ) return false;
	if ( d1->clockModel() != d2->clockModel() ) return false;
	if ( d1->clockManufacturer() != d2->clockManufacturer() ) return false;
	if ( d1->clockType() != d2->clockType() ) return false;

	COMPARE_AND_RETURN(double, d1, d2, gain())
	COMPARE_AND_RETURN(double, d1, d2, maxClockDrift())

	return true;
}


bool equal(const Sensor *s1, const Sensor *s2) {
	if ( s1->description() != s2->description() ) return false;
	if ( s1->model() != s2->model() ) return false;
	if ( s1->manufacturer() != s2->manufacturer() ) return false;
	if ( s1->type() != s2->type() ) return false;
	if ( s1->unit() != s2->unit() ) return false;
	if ( s1->response() != s2->response() ) return false;

	COMPARE_AND_RETURN(double, s1, s2, lowFrequency())
	COMPARE_AND_RETURN(double, s1, s2, highFrequency())

	return true;
}



}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Merge::Merge(Inventory *inv) : InventoryTask(inv) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define MOVE(target, source, type, role) \
	while ( source->role##Count() > 0 ) {\
		type##Ptr o = source->role(0);\
		o->detach();\
		target->add(o.get());\
	}

#define MOVE_GEN_NAME(target, source, type, role) \
	while ( source->role##Count() > 0 ) {\
		type##Ptr o = source->role(0);\
		o->detach();\
		if ( o->name().empty() ) \
			o->setName(o->publicID()); \
		target->add(o.get());\
	}

bool Merge::push(Inventory *inv) {
	if ( _inv == NULL ) return false;

	bool bckReg = PublicObject::IsRegistrationEnabled();
	bool bckNot = Notifier::IsEnabled();

	PublicObject::SetRegistrationEnabled(false);
	Notifier::SetEnabled(false);

	if ( !_tmpInv )
		_tmpInv = new Inventory;

	MOVE(_tmpInv, inv, StationGroup, stationGroup)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, AuxDevice, auxDevice)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, Sensor, sensor)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, Datalogger, datalogger)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponsePAZ, responsePAZ)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponseFAP, responseFAP)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponseFIR, responseFIR)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponsePolynomial, responsePolynomial)
	if ( _interrupted ) return false;
	MOVE(_tmpInv, inv, Network, network)

	PublicObject::SetRegistrationEnabled(bckReg);
	Notifier::SetEnabled(bckNot);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::merge(bool stripUnreferenced) {
	if ( _inv == NULL || _tmpInv == NULL ) return false;

	_stripUnreferenced = stripUnreferenced;
	_stationIDMap.clear();

	const Inventory *inv = _tmpInv.get();

	prepareSession(inv);

	// Merge inventory
	for ( size_t i = 0; i < inv->networkCount(); ++i ) {
		if ( _interrupted ) return false;
		process(inv->network(i));
	}

	for ( size_t i = 0; i < inv->stationGroupCount(); ++i ) {
		if ( _interrupted ) return false;
		process(inv->stationGroup(i));
	}

	if ( _stripUnreferenced ) return true;

	// Collect all unreferenced objects
	for ( size_t i = 0; i < inv->dataloggerCount(); ++i ) {
		if ( _interrupted ) return false;
		Datalogger *d = inv->datalogger(i);
		if ( _session.touchedPublics.find(d) == _session.touchedPublics.end() )
			process(NULL, d);
	}

	for ( size_t i = 0; i < inv->sensorCount(); ++i ) {
		if ( _interrupted ) return false;
		Sensor *s = inv->sensor(i);
		if ( _session.touchedPublics.find(s) == _session.touchedPublics.end() )
			process(NULL, s);
	}

	for ( size_t i = 0; i < inv->auxDeviceCount(); ++i ) {
		if ( _interrupted ) return false;
		AuxDevice *d = inv->auxDevice(i);
		if ( _session.touchedPublics.find(d) == _session.touchedPublics.end() )
			process(NULL, d);
	}

	for ( size_t i = 0; i < inv->responseFIRCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponseFIR *r = inv->responseFIR(i);
		if ( _session.touchedPublics.find(r) == _session.touchedPublics.end() )
			process(r);

	}

	for ( size_t i = 0; i < inv->responsePAZCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponsePAZ *r = inv->responsePAZ(i);
		if ( _session.touchedPublics.find(r) == _session.touchedPublics.end() )
			process(r);
	}

	for ( size_t i = 0; i < inv->responsePolynomialCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponsePolynomial *r = inv->responsePolynomial(i);
		if ( _session.touchedPublics.find(r) == _session.touchedPublics.end() )
			process(r);
	}

	for ( size_t i = 0; i < inv->responseFAPCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponseFAP *r = inv->responseFAP(i);
		if ( _session.touchedPublics.find(r) == _session.touchedPublics.end() )
			process(r);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(const StationGroup *group) {
	SEISCOMP_INFO("Processing station group %s", group->code().c_str());

	StationGroupPtr sc_group;
	sc_group = _inv->stationGroup(group->index());

	bool newInstance = false;
	if ( !sc_group ) {
		sc_group = create<StationGroup>(group->publicID());
		newInstance = true;
	}

	// Assign values
	*sc_group = *group;

	if ( newInstance ) _inv->add(sc_group.get());

	for ( size_t i = 0; i < group->stationReferenceCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_group.get(), group->stationReference(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(StationGroup *group, const StationReference *ref) {
	StationReferencePtr sc_ref;
	StationReference tmp = *ref;

	// Map ID if necessary
	IDMap::iterator it = _stationIDMap.find(ref->stationID());
	if ( it != _stationIDMap.end() )
		tmp.setStationID(it->second);

	sc_ref = group->stationReference(tmp.index());

	bool newInstance = false;
	if ( !sc_ref ) {
		sc_ref = new StationReference();
		newInstance = true;
	}

	// Assign values
	*sc_ref = tmp;

	if ( newInstance ) group->add(sc_ref.get());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(const Network *net) {
	SEISCOMP_INFO("Processing network %s (%s)",
	              net->code().c_str(), net->start().toString("%F %T").c_str());

	NetworkPtr sc_net;
	sc_net = _inv->network(net->index());

	bool newInstance = false;
	if ( !sc_net ) {
		sc_net = create<Network>(net->publicID());
		newInstance = true;
	}
	else {
		if ( !sc_net->equal(*net) ) {
			stringstream ss;
			ss << "Conflicting definitions for network " << net->code()
			   << " / " << net->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_net.get(), net);
		}
	}

	// Assign values
	*sc_net = *net;

	if ( newInstance ) _inv->add(sc_net.get());

	for ( size_t i = 0; i < net->stationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_net.get(), net->station(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Network *net, const Station *sta) {
	StationPtr sc_sta;
	sc_sta = net->station(sta->index());

	bool newInstance = false;
	if ( !sc_sta ) {
		sc_sta = create<Station>(sta->publicID());
		newInstance = true;
	}
	else {
		if ( !sc_sta->equal(*sta) ) {
			stringstream ss;
			ss << "Conflicting definitions for station " << net->code() << "." << sta->code()
			   << " / " << sta->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_sta.get(), sta);
		}
	}

	// Assign values
	*sc_sta = *sta;

	// Remember the new publicID of the existing station to map
	// StationReferences later correctly
	if ( sc_sta->publicID() != sta->publicID() )
		_stationIDMap[sta->publicID()] = sc_sta->publicID();

	if ( newInstance ) net->add(sc_sta.get());

	for ( size_t i = 0; i < sta->sensorLocationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_sta.get(), sta->sensorLocation(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Station *sta, const SensorLocation *loc) {
	SensorLocationPtr sc_loc;
	sc_loc = sta->sensorLocation(loc->index());

	bool newInstance = false;
	if ( !sc_loc ) {
		sc_loc = create<SensorLocation>(loc->publicID());
		newInstance = true;
	}
	else {
		if ( !sc_loc->equal(*loc) ) {
			stringstream ss;
			ss << "Conflicting definitions for sensor location " << sta->network()->code()
			   << "." << sta->code() << "." << loc->code() << " / "
			   << loc->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_loc.get(), loc);
		}
	}

	// Assign values
	*sc_loc = *loc;

	if ( newInstance ) sta->add(sc_loc.get());

	for ( size_t i = 0; i < loc->streamCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_loc.get(), loc->stream(i));
	}

	for ( size_t i = 0; i < loc->auxStreamCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_loc.get(), loc->auxStream(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(SensorLocation *loc, const Stream *stream) {
	StreamPtr sc_cha;
	sc_cha = loc->stream(stream->index());

	bool newInstance = false;
	if ( !sc_cha ) {
		sc_cha = new Stream();
		newInstance = true;

		// Assign values
		*sc_cha = *stream;
	}
	else {
		string dataloggerID = stream->datalogger();
		string sensorID = stream->sensor();

		// Compare references datalogger and sensor
		if ( !compareDatalogger(dataloggerID, sc_cha->datalogger(), dataloggerID) ) {
			stringstream ss;
			ss << "Conflicting datalogger references for channel " << loc->station()->network()->code()
			   << "." << loc->station()->code() << "." << loc->code() << "."
			   << stream->code() << " / " << loc->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_cha.get(), stream);
		}

		if ( !compareSensor(sensorID, sc_cha->sensor(), sensorID) ) {
			stringstream ss;
			ss << "Conflicting sensor references for channel " << loc->station()->network()->code()
			   << "." << loc->station()->code() << "." << loc->code() << "."
			   << stream->code() << " / " << loc->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_cha.get(), stream);
		}

		sc_cha->setDatalogger(stream->datalogger());
		sc_cha->setSensor(stream->sensor());

		if ( !sc_cha->equal(*stream) ) {
			stringstream ss;
			ss << "Conflicting definitions for channel " << loc->station()->network()->code()
			   << "." << loc->station()->code() << "." << loc->code() << "."
			   << stream->code() << " / " << loc->start().toString("%FT%T");
			log(LogHandler::Conflict, ss.str().c_str(), sc_cha.get(), stream);
		}

		// Assign values
		*sc_cha = *stream;

		// Copy merged datalogger and sensor ID
		sc_cha->setDatalogger(dataloggerID);
		sc_cha->setSensor(sensorID);
	}


	if ( !sc_cha->datalogger().empty() ) {
		const Datalogger *dl = findDatalogger(sc_cha->datalogger());
		if ( dl == NULL ) {
			log(LogHandler::Unresolved,
			    (string(sc_cha->className()) + " " + id(loc) + "." + sc_cha->code() + "/" + id(sc_cha->start()) + "\n  "
			     "referenced datalogger is not available").c_str(), NULL, NULL);
			/*
			SEISCOMP_WARNING("%s.%s.%s.%s: datalogger not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_cha->code().c_str(),
			                 sc_cha->datalogger().c_str());
			*/
		}
		else
			process(sc_cha.get(), dl);
	}

	if ( !sc_cha->sensor().empty() ) {
		const Sensor *sensor = findSensor(sc_cha->sensor());
		if ( sensor == NULL ) {
			log(LogHandler::Unresolved,
			    (string(sc_cha->className()) + " " + id(loc) + "." + sc_cha->code() + "/" + id(sc_cha->start()) + "\n  "
			    "referenced sensor is not available").c_str(), NULL, NULL);
			/*
			SEISCOMP_WARNING("%s.%s.%s.%s: sensor not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_cha->code().c_str(),
			                 sc_cha->sensor().c_str());
			*/
		}
		else
			process(sc_cha.get(), sensor);
	}

	if ( newInstance ) loc->add(sc_cha.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(SensorLocation *loc, const AuxStream *aux) {
	AuxStreamPtr sc_aux;
	sc_aux = loc->auxStream(aux->index());

	bool newInstance = false;
	if ( !sc_aux ) {
		sc_aux = new AuxStream();
		newInstance = true;
	}

	// Assign values
	*sc_aux = *aux;

	if ( !sc_aux->device().empty() ) {
		const AuxDevice *d = findAuxDevice(sc_aux->device());
		if ( d == NULL ) {
			SEISCOMP_INFO("%s.%s.%s.%s: aux device not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_aux->code().c_str(),
			                 sc_aux->device().c_str());
		}
		else
			process(sc_aux.get(), d);
	}

	if ( newInstance ) loc->add(sc_aux.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Stream *cha, const Datalogger *dl) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(dl);

	DataloggerPtr sc_dl = _inv->datalogger(dl->index());

	bool newInstance = false;
	if ( !sc_dl ) {
		sc_dl = create<Datalogger>(dl->publicID());
		newInstance = true;
	}

	*sc_dl = *dl;

	if ( newInstance ) {
		_inv->add(sc_dl.get());
		_dataloggerNames[sc_dl->name()] = sc_dl.get();
	}

	if ( cha )
		cha->setDatalogger(sc_dl->publicID());

	if ( !_stripUnreferenced ) {
		// Handle all decimations
		for ( size_t i = 0; i < dl->decimationCount(); ++i ) {
			if ( _interrupted ) return false;
			process(sc_dl.get(), dl->decimation(i));
		}
	}
	else {
		// Filter decimations according to the stream
		Decimation *deci = NULL;
		try {
			deci = dl->decimation(DecimationIndex(cha->sampleRateNumerator(), cha->sampleRateDenominator()));
		}
		catch ( ... ) {}

		if ( deci )
			process(sc_dl.get(), deci);
	}

	for ( size_t i = 0; i < dl->dataloggerCalibrationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_dl.get(), dl->dataloggerCalibration(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Datalogger *dl, const Decimation *deci) {
	DecimationPtr sc_deci = dl->decimation(deci->index());

	bool newInstance = false;
	if ( !sc_deci ) {
		sc_deci = new Decimation();
		newInstance = true;
	}

	*sc_deci = *deci;

	if ( newInstance ) dl->add(sc_deci.get());

	string deciAnalogueChain;
	string deciDigitalChain;

	// Copy analogue filter chain
	try {
		const string &analogueFilterChain = deci->analogueFilterChain().content();
		vector<string> filters;
		Core::split(filters, analogueFilterChain.c_str(), " ");

		for ( size_t i = 0; i < filters.size(); ++i ) {
			if ( filters[i].empty() ) continue;

			if ( !deciAnalogueChain.empty() )
				deciAnalogueChain += " ";

			const ResponsePAZ *paz = findPAZ(filters[i]);
			if ( paz == NULL ) {
				const ResponsePolynomial *poly = findPoly(filters[i]);
				if ( poly == NULL ) {
					const ResponseFAP *fap = findFAP(filters[i]);
					if ( fap == NULL ) {
						log(LogHandler::Unresolved,
						    (string(dl->className()) + " " + id(dl) + "/decimation " + Core::toString(sc_deci->sampleRateNumerator()) + "/" + Core::toString(sc_deci->sampleRateDenominator()) + "\n  "
						    "analogue filter chain: response not found: " + filters[i]).c_str(), NULL, NULL);
						/*
						SEISCOMP_WARNING("Datalogger %s/decimation %d/%d analogue filter chain: response not found: %s",
						                 dl->publicID().c_str(),
						                 sc_deci->sampleRateNumerator(),
						                 sc_deci->sampleRateDenominator(),
						                 filters[i].c_str());
						*/
						deciAnalogueChain += filters[i];
					}
					else {
						ResponseFAPPtr sc_fap = process(fap);
						deciAnalogueChain += sc_fap->publicID();
					}
				}
				else {
					ResponsePolynomialPtr sc_poly = process(poly);
					deciAnalogueChain += sc_poly->publicID();
				}
			}
			else {
				ResponsePAZPtr sc_paz = process(paz);
				deciAnalogueChain += sc_paz->publicID();
			}
		}
	}
	catch ( ... ) {}


	// Copy digital filter chain
	try {
		const string &digitalFilterChain = deci->digitalFilterChain().content();
		vector<string> filters;
		Core::split(filters, digitalFilterChain.c_str(), " ");

		for ( size_t i = 0; i < filters.size(); ++i ) {
			if ( filters[i].empty() ) continue;

			if ( !deciDigitalChain.empty() )
				deciDigitalChain += " ";

			const ResponsePAZ *paz = findPAZ(filters[i]);
			if ( paz == NULL ) {
				const ResponseFIR *fir = findFIR(filters[i]);
				if ( fir == NULL ) {
					log(LogHandler::Unresolved,
					    (string(dl->className()) + " " + id(dl) + "/decimation " + Core::toString(sc_deci->sampleRateNumerator()) + "/" + Core::toString(sc_deci->sampleRateDenominator()) + "\n  "
					    "digital filter chain: response not found: " + filters[i]).c_str(), NULL, NULL);
					/*
					SEISCOMP_WARNING("Datalogger %s/decimation %d/%d digital filter chain: response not found: %s",
					                 dl->publicID().c_str(),
					                 sc_deci->sampleRateNumerator(),
					                 sc_deci->sampleRateDenominator(),
					                 filters[i].c_str());
					*/
					deciDigitalChain += filters[i];
				}
				else {
					ResponseFIRPtr sc_fir = process(fir);
					deciDigitalChain += sc_fir->publicID();
				}
			}
			else {
				ResponsePAZPtr sc_paz = process(paz);
				deciDigitalChain += sc_paz->publicID();
			}
		}
	}
	catch ( ... ) {}

	if ( !deciAnalogueChain.empty() ) {
		Blob tmp;
		tmp.setContent(deciAnalogueChain);
		sc_deci->setAnalogueFilterChain(tmp);
	}
	else
		sc_deci->setAnalogueFilterChain(Core::None);

	if ( !deciDigitalChain.empty() ) {
		Blob tmp;
		tmp.setContent(deciDigitalChain);
		sc_deci->setDigitalFilterChain(tmp);
	}
	else
		sc_deci->setDigitalFilterChain(Core::None);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Datalogger *dl, const DataloggerCalibration *cal) {
	DataloggerCalibrationPtr sc_cal = dl->dataloggerCalibration(cal->index());

	bool newInstance = false;
	if ( !sc_cal ) {
		sc_cal = new DataloggerCalibration();
		newInstance = true;
	}

	*sc_cal = *cal;

	if ( newInstance ) dl->add(sc_cal.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Stream *cha, const Sensor *sensor) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(sensor);

	SensorPtr sc_sensor = _inv->sensor(sensor->index());

	bool newInstance = false;
	if ( !sc_sensor ) {
		sc_sensor = create<Sensor>(sensor->publicID());
		newInstance = true;
	}

	*sc_sensor = *sensor;

	if ( newInstance ) {
		_sensorNames[sc_sensor->name()] = sc_sensor.get();
		_inv->add(sc_sensor.get());
	}

	if ( cha )
		cha->setSensor(sc_sensor->publicID());

	if ( !sensor->response().empty() ) {
		const ResponsePAZ *paz = findPAZ(sensor->response());
		if ( paz == NULL ) {
			const ResponsePolynomial *poly = findPoly(sensor->response());
			if ( poly == NULL ) {
				const ResponseFAP *fap = findFAP(sensor->response());
				if ( fap == NULL ) {
					log(LogHandler::Unresolved,
					    (string(sensor->className()) + " " + id(sensor) + "\n  "
					     "referenced response is not available").c_str(), NULL, NULL);
					/*
					SEISCOMP_WARNING("Sensor %s: response not found: %s",
					                 sensor->publicID().c_str(),
					                 sensor->response().c_str());
					*/
				}
				else {
					ResponseFAPPtr sc_fap = process(fap);
					sc_sensor->setResponse(sc_fap->publicID());
				}
			}
			else {
				ResponsePolynomialPtr sc_poly = process(poly);
				sc_sensor->setResponse(sc_poly->publicID());
			}
		}
		else {
			ResponsePAZPtr sc_paz = process(paz);
			sc_sensor->setResponse(sc_paz->publicID());
		}
	}

	for ( size_t i = 0; i < sensor->sensorCalibrationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_sensor.get(), sensor->sensorCalibration(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(Sensor *sensor, const SensorCalibration *cal) {
	SensorCalibrationPtr sc_cal = sensor->sensorCalibration(cal->index());

	bool newInstance = false;
	if ( !sc_cal ) {
		sc_cal = new SensorCalibration();
		newInstance = true;
	}

	*sc_cal = *cal;

	if ( newInstance ) sensor->add(sc_cal.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(AuxStream *aux, const AuxDevice *device) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(device);

	AuxDevicePtr sc_device = _inv->auxDevice(device->index());

	bool newInstance = false;
	if ( !sc_device ) {
		sc_device = create<AuxDevice>(device->publicID());
		newInstance = true;
	}

	*sc_device = *device;

	if ( newInstance ) {
		_auxDeviceNames[sc_device->name()] = sc_device.get();
		_inv->add(sc_device.get());
	}

	if ( aux )
		aux->setDevice(sc_device->publicID());

	for ( size_t i = 0; i < device->auxSourceCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_device.get(), device->auxSource(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::process(AuxDevice *device, const AuxSource *source) {
	AuxSourcePtr sc_source = device->auxSource(source->index());

	bool newInstance = false;
	if ( !sc_source ) {
		sc_source = new AuxSource();
		newInstance = true;
	}

	*sc_source = *source;

	if ( newInstance ) device->add(sc_source.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR *Merge::process(const ResponseFIR *fir) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(fir);
	return InventoryTask::process(fir);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ *Merge::process(const ResponsePAZ *paz) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(paz);
	return InventoryTask::process(paz);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial *Merge::process(const ResponsePolynomial *poly) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(poly);
	return InventoryTask::process(poly);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP *Merge::process(const ResponseFAP *fap) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(fap);
	return InventoryTask::process(fap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::compareDatalogger(string &finalID, const string &id1,
                              const string &id2) {
	if ( id1 == id2 ) {
		finalID = id1;
		return true;
	}

	if ( id1.empty() != id2.empty() ) {
		if ( id1.empty() )
			finalID = id2;
		else
			finalID = id1;
		return true;
	}

	const DataModel::Datalogger *dl1 = findDatalogger(id1);
	const DataModel::Datalogger *dl2 = findDatalogger(id2);

	if ( dl1 == NULL ) {
		{
			stringstream ss;
			ss << "Datalogger " << id1 << " not defined";
			log(LogHandler::Information, ss.str().c_str(), NULL, NULL);
		}

		if ( dl2 == NULL ) {
			stringstream ss;
			ss << "Datalogger " << id2 << " not defined, clear reference";
			log(LogHandler::Information, ss.str().c_str(), NULL, NULL);
			finalID = "";
		}
		else
			finalID = id2;

		return true;
	}

	if ( dl2 == NULL ) {
		{
			stringstream ss;
			ss << "Datalogger " << id2 << " not defined";
			log(LogHandler::Information, ss.str().c_str(), NULL, NULL);
		}

		if ( dl1 == NULL ) {
			stringstream ss;
			ss << "Datalogger " << id1 << " not defined, clear reference";
			log(LogHandler::Information, ss.str().c_str(), NULL, NULL);
			finalID = "";
		}
		else
			finalID = id1;

		return true;
	}

	// Both are set, compare them
	finalID = id1;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Merge::compareSensor(string &finalID, const string &id1,
                          const string &id2) {
	if ( id1 == id2 ) {
		finalID = id1;
		return true;
	}

	if ( id1.empty() != id2.empty() ) {
		if ( id1.empty() )
			finalID = id2;
		else
			finalID = id1;
		return true;
	}

	const DataModel::Sensor *s1 = findSensor(id1);
	const DataModel::Sensor *s2 = findSensor(id2);

	if ( s1 == NULL ) {
		// Give warning about missing datalogger
		if ( s2 == NULL ) {
			// Give warning about missing datalogger
			finalID = "";
		}
		else
			finalID = id2;

		return true;
	}

	if ( s2 == NULL ) {
		// Give warning about missing datalogger
		if ( s1 == NULL ) {
			// Give warning about missing datalogger
			finalID = "";
		}
		else
			finalID = id1;

		return true;
	}

	// Both are set, compare them
	finalID = id1;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
