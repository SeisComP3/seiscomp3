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

string id(const PublicObject *obj) {
	return obj->publicID();
}

string id(const Core::Time &t) {
	if ( t.microseconds() > 0 )
		return t.toString("%F %T.%f");
	else
		return t.toString("%F %T");
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


class InventoryVisitor : public Seiscomp::DataModel::Visitor {
	public:
		InventoryVisitor(Seiscomp::DataModel::Inventory *inv,
		                 InventoryTask::SourceMap *map)
		: _source(inv), _map(map) {}

	public:
		bool visit(PublicObject *po) {
			(*_map)[po] = _source;
			return true;
		}

		virtual void visit(Object *o) {
			(*_map)[o] = _source;
		}

	private:
		Seiscomp::DataModel::Inventory *_source;
		InventoryTask::SourceMap *_map;
};


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

	InventoryVisitor bindToSource(inv, &_sources);
	inv->accept(&bindToSource);

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
	MOVE_GEN_NAME(_tmpInv, inv, ResponsePolynomial, responsePolynomial)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponseFIR, responseFIR)
	if ( _interrupted ) return false;
	MOVE_GEN_NAME(_tmpInv, inv, ResponseIIR, responseIIR)
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

	for ( size_t i = 0; i < inv->responseIIRCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponseIIR *r = inv->responseIIR(i);
		if ( _session.touchedPublics.find(r) == _session.touchedPublics.end() )
			process(r);
	}

	_sources.clear();

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
		_sources[sc_net.get()] = _sources[net];
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
		_sources[sc_sta.get()] = _sources[sta];
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
		_sources[sc_loc.get()] = _sources[loc];
		newInstance = true;
	}
	else {
		if ( !sc_loc->equal(*loc) ) {
			stringstream ss;
			ss << "Conflicting definitions for sensor location " << sta->network()->code()
			   << "." << sta->code() << "." << (loc->code().empty() ? "--" : loc->code()) << " / "
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
		_sources[sc_cha.get()] = _sources[stream];
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
		_sources[sc_aux.get()] = _sources[aux];
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
		_sources[sc_dl.get()] = _sources[dl];
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
		_sources[sc_deci.get()] = _sources[deci];
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
						const ResponseFIR *fir = findFIR(filters[i]);
						if ( fir == NULL ) {
							const ResponseIIR *iir = findIIR(filters[i]);
							if ( iir == NULL ) {
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
								ResponseIIRPtr sc_iir = process(iir);
								deciAnalogueChain += sc_iir->publicID();
							}
						}
						else {
							ResponseFIRPtr sc_fir = process(fir);
							deciAnalogueChain += sc_fir->publicID();
						}
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
					const ResponseIIR *iir = findIIR(filters[i]);
					if ( iir == NULL ) {
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
						ResponseIIRPtr sc_iir = process(iir);
						deciDigitalChain += sc_iir->publicID();
					}
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
		_sources[sc_cal.get()] = _sources[cal];
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
		_sources[sc_sensor.get()] = _sources[sensor];
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
					const ResponseIIR *iir = findIIR(sensor->response());
					if ( iir == NULL ) {
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
						ResponseIIRPtr sc_iir = process(iir);
						sc_sensor->setResponse(sc_iir->publicID());
					}
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
		_sources[sc_cal.get()] = _sources[cal];
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
		_sources[sc_device.get()] = _sources[device];
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
		_sources[sc_source.get()] = _sources[source];
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
ResponseIIR *Merge::process(const ResponseIIR *iir) {
	if ( !_stripUnreferenced )
		_session.touchedPublics.insert(iir);
	return InventoryTask::process(iir);
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
