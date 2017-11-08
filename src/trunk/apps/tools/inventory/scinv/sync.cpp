/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/

#define SEISCOMP_COMPONENT INVMGR

#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <iostream>

#include "sync.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;


namespace {


class RemoveUntouchedObjects : public Visitor {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		RemoveUntouchedObjects(set<Object*> &registry, set<Object*> &out)
		: Visitor(TM_TOPDOWN), _registry(registry), _out(out) {}


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		bool visit(PublicObject *po) {
			if ( _registry.find(po) == _registry.end() ) {
				_out.insert(po);
				return false;
			}

			return true;
		}

		void visit(Object *o) {
			if ( _registry.find(o) == _registry.end() )
				_out.insert(o);
		}

	private:
		set<Object*> &_registry;
		set<Object*> &_out;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sync::Sync(Inventory *inv) : InventoryTask(inv) {
	// Copy public object registration
	PublicObject::Iterator it;
	for ( it = PublicObject::Begin(); it != PublicObject::End(); ++it )
		_publicObjects[it->first] = it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::push(const Seiscomp::DataModel::Inventory *inv) {
	if ( !_inv ) return false;

	prepareSession(inv);

	// Process networks. All networks are then under our control and
	// unprocessed epochs will be removed later
	for ( size_t n = 0; n < inv->networkCount(); ++n ) {
		if ( _interrupted ) return false;
		process(inv->network(n));
	}

	for ( size_t i = 0; i < inv->stationGroupCount(); ++i ) {
		if ( _interrupted ) return false;
		process(inv->stationGroup(i));
	}

	// All stations and related instruments/responses are synchronized
	// Now synchronize all remaining objects
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

	for ( size_t i = 0; i < inv->responseIIRCount(); ++i ) {
		if ( _interrupted ) return false;
		ResponseIIR *r = inv->responseIIR(i);
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
bool Sync::process(const StationGroup *group) {
	SEISCOMP_INFO("Synching station group %s", group->code().c_str());

	bool newInstance = false;
	bool needUpdate = false;

	DataModel::StationGroupPtr sc_group;
	sc_group = _inv->stationGroup(group->index());

	if ( !sc_group ) {
		sc_group = create<StationGroup>(group->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_group->equal(*group);

	// Assign values
	*sc_group = *group;

	if ( newInstance ) {
		SEISCOMP_DEBUG("Added new station group: %s", sc_group->code().c_str());
		_inv->add(sc_group.get());
	}
	else if ( needUpdate ) {
		SEISCOMP_DEBUG("Updated station group: %s", sc_group->code().c_str());
		sc_group->update();
	}

	_touchedObjects.insert(sc_group.get());

	for ( size_t i = 0; i < group->stationReferenceCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_group.get(), group->stationReference(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(StationGroup *group, const StationReference *ref) {
	StationReferencePtr sc_ref;
	StationReference tmp = *ref;

	// Map ID if necessary
	IDMap::iterator it = _stationIDMap.find(ref->stationID());
	if ( it != _stationIDMap.end() )
		tmp.setStationID(it->second);

	sc_ref = group->stationReference(tmp.index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_ref ) {
		sc_ref = new StationReference();
		newInstance = true;
		SEISCOMP_DEBUG("Create new station reference for %s", ref->stationID().c_str());
	}
	else
		needUpdate = !sc_ref->equal(tmp);

	// Assign values
	*sc_ref = tmp;

	if ( newInstance )
		group->add(sc_ref.get());
	else if ( needUpdate )
		sc_ref->update();

	_touchedObjects.insert(sc_ref.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(const Network *net) {
	SEISCOMP_INFO("Synching network %s (%s)",
	              net->code().c_str(), net->start().toString("%F %T").c_str());

	bool newInstance = false;
	bool needUpdate = false;

	DataModel::NetworkPtr sc_net;
	sc_net = _inv->network(net->index());

	if ( !sc_net ) {
		sc_net = create<Network>(net->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_net->equal(*net);

	// Assign values
	*sc_net = *net;

	if ( newInstance ) {
		SEISCOMP_DEBUG("Added new network epoch: %s (%s)",
		               sc_net->code().c_str(), sc_net->start().iso().c_str());
		_inv->add(sc_net.get());
	}
	else if ( needUpdate ) {
		SEISCOMP_DEBUG("Updated network epoch: %s (%s)",
		               sc_net->code().c_str(), sc_net->start().iso().c_str());
		sc_net->update();
	}

	_touchedObjects.insert(sc_net.get());

	for ( size_t i = 0; i < net->commentCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_net.get(), net->comment(i));
	}

	for ( size_t i = 0; i < net->stationCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_net.get(), net->station(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Seiscomp::DataModel::Network *net,
                   const Seiscomp::DataModel::Comment *comment) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::CommentPtr sc_comment;
	sc_comment = net->comment(comment->index());
	if ( !sc_comment ) {
		sc_comment = new Comment();
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_comment->equal(*comment);

	// Assign values
	*sc_comment = *comment;

	if ( newInstance ) {
		net->add(sc_comment.get());
		SEISCOMP_DEBUG("Added new network comment: %s", sc_comment->text().c_str());
	}
	else if ( needUpdate ) {
		sc_comment->update();
		SEISCOMP_DEBUG("Updated network comment: %s", sc_comment->text().c_str());
	}

	// Register station
	_touchedObjects.insert(sc_comment.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Seiscomp::DataModel::Network *net,
                   const Seiscomp::DataModel::Station *sta) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::StationPtr sc_sta;
	sc_sta = net->station(sta->index());
	if ( !sc_sta ) {
		sc_sta = create<Station>(sta->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_sta->equal(*sta);

	// Assign values
	*sc_sta = *sta;

	// Remember the new publicID of the existing station to map
	// StationReferences later correctly
	if ( sc_sta->publicID() != sta->publicID() )
		_stationIDMap[sta->publicID()] = sc_sta->publicID();

	if ( newInstance ) {
		net->add(sc_sta.get());
		SEISCOMP_DEBUG("Added new station epoch: %s (%s)",
		               sc_sta->code().c_str(), sc_sta->start().iso().c_str());
	}
	else if ( needUpdate ) {
		sc_sta->update();
		SEISCOMP_DEBUG("Updated station epoch: %s (%s)",
		               sc_sta->code().c_str(), sc_sta->start().iso().c_str());
	}

	// Register station
	_touchedObjects.insert(sc_sta.get());

	for ( size_t i = 0; i < sta->commentCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_sta.get(), sta->comment(i));
	}

	for ( size_t i = 0; i < sta->sensorLocationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_sta.get(), sta->sensorLocation(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Seiscomp::DataModel::Station *sta,
                   const Seiscomp::DataModel::Comment *comment) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::CommentPtr sc_comment;
	sc_comment = sta->comment(comment->index());
	if ( !sc_comment ) {
		sc_comment = new Comment();
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_comment->equal(*comment);

	// Assign values
	*sc_comment = *comment;

	if ( newInstance ) {
		sta->add(sc_comment.get());
		SEISCOMP_DEBUG("Added new station comment: %s", sc_comment->text().c_str());
	}
	else if ( needUpdate ) {
		sc_comment->update();
		SEISCOMP_DEBUG("Updated station comment: %s", sc_comment->text().c_str());
	}

	// Register station
	_touchedObjects.insert(sc_comment.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Station *sta, const SensorLocation *loc) {
	SensorLocationPtr sc_loc;
	sc_loc = sta->sensorLocation(loc->index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_loc ) {
		sc_loc = create<SensorLocation>(loc->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_loc->equal(*loc);

	// Assign values
	*sc_loc = *loc;

	if ( newInstance ) {
		sta->add(sc_loc.get());
		SEISCOMP_DEBUG("Added new sensor location epoch: %s (%s)",
		               sc_loc->code().c_str(), sc_loc->start().iso().c_str());
	}
	else if ( needUpdate ) {
		sc_loc->update();
		SEISCOMP_DEBUG("Updated sensor location epoch: %s (%s)",
		               sc_loc->code().c_str(), sc_loc->start().iso().c_str());
	}

	// Register sensor location
	_touchedObjects.insert(sc_loc.get());

	for ( size_t i = 0; i < loc->commentCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_loc.get(), loc->comment(i));
	}

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
bool Sync::process(Seiscomp::DataModel::SensorLocation *loc,
                   const Seiscomp::DataModel::Comment *comment) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::CommentPtr sc_comment;
	sc_comment = loc->comment(comment->index());
	if ( !sc_comment ) {
		sc_comment = new Comment();
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_comment->equal(*comment);

	// Assign values
	*sc_comment = *comment;

	if ( newInstance ) {
		loc->add(sc_comment.get());
		SEISCOMP_DEBUG("Added new sensor location comment: %s", sc_comment->text().c_str());
	}
	else if ( needUpdate ) {
		sc_comment->update();
		SEISCOMP_DEBUG("Updated sensor location comment: %s", sc_comment->text().c_str());
	}

	// Register station
	_touchedObjects.insert(sc_comment.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(SensorLocation *loc, const Stream *cha) {
	StreamPtr sc_cha;
	sc_cha = loc->stream(cha->index());

	bool newInstance = false;

	Stream tmpStream(*cha);

	if ( !sc_cha ) {
		sc_cha = create<Stream>(cha->publicID());
		newInstance = true;
	}

	// Register stream
	_touchedObjects.insert(sc_cha.get());

	if ( !tmpStream.datalogger().empty() ) {
		const Datalogger *dl = findDatalogger(tmpStream.datalogger());
		if ( dl == NULL ) {
			/*
			SEISCOMP_WARNING("%s.%s.%s.%s: datalogger not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_cha->code().c_str(),
			                 sc_cha->datalogger().c_str());
			*/
		}
		else
			process(&tmpStream, dl);
	}

	if ( !tmpStream.sensor().empty() ) {
		const Sensor *sensor = findSensor(tmpStream.sensor());
		if ( sensor == NULL ) {
			/*
			SEISCOMP_WARNING("%s.%s.%s.%s: sensor not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_cha->code().c_str(),
			                 sc_cha->sensor().c_str());
			*/
		}
		else
			process(&tmpStream, sensor);
	}

	if ( newInstance ) {
		*sc_cha = tmpStream;
		loc->add(sc_cha.get());
		SEISCOMP_DEBUG("Added new stream epoch: %s (%s)",
		               sc_cha->code().c_str(), sc_cha->start().iso().c_str());
	}
	else if ( !sc_cha->equal(tmpStream) ) {
		*sc_cha = tmpStream;
		sc_cha->update();
		SEISCOMP_DEBUG("Updated stream epoch: %s (%s)",
		               sc_cha->code().c_str(), sc_cha->start().iso().c_str());
	}

	for ( size_t i = 0; i < cha->commentCount(); ++i ) {
		if ( _interrupted ) break;
		process(sc_cha.get(), cha->comment(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(SensorLocation *loc, const AuxStream *aux) {
	AuxStreamPtr sc_aux;
	sc_aux = loc->auxStream(aux->index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_aux ) {
		sc_aux = new AuxStream();
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_aux->equal(*aux);

	// Assign values
	*sc_aux = *aux;

	if ( !sc_aux->device().empty() ) {
		const AuxDevice *device = findAuxDevice(sc_aux->device());
		if ( device == NULL ) {
			/*
			SEISCOMP_WARNING("%s.%s.%s.%s: aux device not found: %s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), sc_aux->code().c_str(),
			                 sc_aux->device().c_str());
			*/
		}
		else {
			process(sc_aux.get(), device);
			// Did the sensor reference change?
			if ( sc_aux->device() != aux->device() )
				needUpdate = true;
		}
	}

	if ( newInstance ) {
		loc->add(sc_aux.get());
		SEISCOMP_DEBUG("Added new aux stream epoch: %s (%s)",
		               sc_aux->code().c_str(), sc_aux->start().iso().c_str());
	}
	else if ( needUpdate ) {
		sc_aux->update();
		SEISCOMP_DEBUG("Updated aux stream epoch: %s (%s)",
		               sc_aux->code().c_str(), sc_aux->start().iso().c_str());
	}

	// Register stream
	_touchedObjects.insert(sc_aux.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Seiscomp::DataModel::Stream *cha,
                   const Seiscomp::DataModel::Comment *comment) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::CommentPtr sc_comment;
	sc_comment = cha->comment(comment->index());
	if ( !sc_comment ) {
		sc_comment = new Comment();
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_comment->equal(*comment);

	// Assign values
	*sc_comment = *comment;

	if ( newInstance ) {
		cha->add(sc_comment.get());
		SEISCOMP_DEBUG("Added new stream comment: %s", sc_comment->text().c_str());
	}
	else if ( needUpdate ) {
		sc_comment->update();
		SEISCOMP_DEBUG("Updated stream comment: %s", sc_comment->text().c_str());
	}

	// Register station
	_touchedObjects.insert(sc_comment.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Stream *cha, const Datalogger *dl) {
	_session.touchedPublics.insert(dl);

	DataloggerPtr sc_dl = dataloggerByName(dl->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_dl ) {
		sc_dl = create<Datalogger>(dl->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_dl->equal(*dl);

	// Assign values
	*sc_dl = *dl;

	if ( newInstance ) {
		_inv->add(sc_dl.get());
		_dataloggerNames[sc_dl->name()] = sc_dl.get();
	}
	else if ( needUpdate ) {
		sc_dl->update();
		SEISCOMP_DEBUG("Updated datalogger: %s", sc_dl->publicID().c_str());
	}

	// Update datalogger reference
	if ( cha ) cha->setDatalogger(sc_dl->publicID());

	// Register datalogger
	_touchedObjects.insert(sc_dl.get());

	// Synchronize decimations
	for ( size_t i = 0; i < dl->decimationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_dl.get(), dl->decimation(i));
	}

	// Synchronize calibrations
	for ( size_t i = 0; i < dl->dataloggerCalibrationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_dl.get(), dl->dataloggerCalibration(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Datalogger *dl, const Decimation *deci) {
	DecimationPtr sc_deci = dl->decimation(deci->index());

	bool newInstance = false;

	Decimation tmpDeci(*deci);

	if ( !sc_deci ) {
		sc_deci = new Decimation();
		newInstance = true;
	}

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
		tmpDeci.setAnalogueFilterChain(tmp);
	}
	else
		tmpDeci.setAnalogueFilterChain(Core::None);

	if ( !deciDigitalChain.empty() ) {
		Blob tmp;
		tmp.setContent(deciDigitalChain);
		tmpDeci.setDigitalFilterChain(tmp);
	}
	else
		tmpDeci.setDigitalFilterChain(Core::None);

	if ( newInstance ) {
		*sc_deci = tmpDeci;
		dl->add(sc_deci.get());
	}
	else if ( !sc_deci->equal(tmpDeci) ) {
		*sc_deci = tmpDeci;
		sc_deci->update();
	}

	// Register decimation
	_touchedObjects.insert(sc_deci.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Datalogger *dl, const DataloggerCalibration *cal) {
	DataloggerCalibrationPtr sc_cal = dl->dataloggerCalibration(cal->index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_cal ) {
		sc_cal = new DataloggerCalibration();
		newInstance = true;
	}
	else
		needUpdate = !sc_cal->equal(*cal);

	*sc_cal = *cal;

	if ( newInstance )
		dl->add(sc_cal.get());
	else if ( needUpdate )
		sc_cal->update();

	// Register calibration
	_touchedObjects.insert(sc_cal.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Stream *cha, const Sensor *sensor) {
	_session.touchedPublics.insert(sensor);

	SensorPtr sc_sensor = sensorByName(sensor->name());

	Sensor tmpSens(*sensor);

	bool newInstance = false;

	if ( !sc_sensor ) {
		sc_sensor = create<Sensor>(sensor->publicID());
		newInstance = true;
	}

	// Update datalogger reference
	if ( cha ) cha->setSensor(sc_sensor->publicID());

	if ( !tmpSens.response().empty() ) {
		const ResponsePAZ *paz = findPAZ(tmpSens.response());
		if ( paz == NULL ) {
			const ResponsePolynomial *poly = findPoly(tmpSens.response());
			if ( poly == NULL ) {
				const ResponseFAP *fap = findFAP(tmpSens.response());
				if ( fap == NULL ) {
					const ResponseIIR *iir = findIIR(sensor->response());
					if ( iir == NULL ) {
						/*
						SEISCOMP_WARNING("Sensor %s: response not found: %s",
						                 sensor->publicID().c_str(),
						                 sensor->response().c_str());
						*/
					}
					else {
						ResponseIIRPtr sc_iir = process(iir);
						tmpSens.setResponse(sc_iir->publicID());
					}
				}
				else {
					ResponseFAPPtr sc_fap = process(fap);
					tmpSens.setResponse(sc_fap->publicID());
				}
			}
			else {
				ResponsePolynomialPtr sc_poly = process(poly);
				tmpSens.setResponse(sc_poly->publicID());
			}
		}
		else {
			ResponsePAZPtr sc_paz = process(paz);
			tmpSens.setResponse(sc_paz->publicID());
		}
	}

	if ( newInstance ) {
		*sc_sensor = tmpSens;
		_inv->add(sc_sensor.get());
		_sensorNames[sc_sensor->name()] = sc_sensor.get();
	}
	else if ( !sc_sensor->equal(tmpSens) ) {
		*sc_sensor = tmpSens;
		sc_sensor->update();
		SEISCOMP_DEBUG("Updated sensor: %s", sc_sensor->publicID().c_str());
	}

	// Register sensor
	_touchedObjects.insert(sc_sensor.get());

	for ( size_t i = 0; i < sensor->sensorCalibrationCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_sensor.get(), sensor->sensorCalibration(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(Sensor *sensor, const SensorCalibration *cal) {
	SensorCalibrationPtr sc_cal = sensor->sensorCalibration(cal->index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_cal ) {
		sc_cal = new SensorCalibration();
		newInstance = true;
	}
	else
		needUpdate = !sc_cal->equal(*cal);

	*sc_cal = *cal;

	if ( newInstance )
		sensor->add(sc_cal.get());
	else if ( needUpdate )
		sc_cal->update();

	// Register calibration
	_touchedObjects.insert(sc_cal.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(AuxStream *aux, const AuxDevice *device) {
	_session.touchedPublics.insert(device);

	AuxDevicePtr sc_device = auxDeviceByName(device->name());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_device ) {
		sc_device = create<AuxDevice>(device->publicID());
		newInstance = true;
	}
	else
		// Check equality and set update flag
		needUpdate = !sc_device->equal(*device);

	// Assign values
	*sc_device = *device;

	// Update device reference
	if ( aux ) aux->setDevice(sc_device->publicID());

	if ( newInstance ) {
		_inv->add(sc_device.get());
		_auxDeviceNames[sc_device->name()] = sc_device.get();
	}
	else if ( needUpdate ) {
		sc_device->update();
		SEISCOMP_DEBUG("Updated aux device: %s", sc_device->publicID().c_str());
	}

	// Register sensor
	_touchedObjects.insert(sc_device.get());

	for ( size_t i = 0; i < device->auxSourceCount(); ++i ) {
		if ( _interrupted ) return false;
		process(sc_device.get(), device->auxSource(i));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Sync::process(AuxDevice *device, const AuxSource *source) {
	AuxSourcePtr sc_source = device->auxSource(source->index());

	bool newInstance = false;
	bool needUpdate = false;

	if ( !sc_source ) {
		sc_source = new AuxSource();
		newInstance = true;
	}
	else
		needUpdate = !sc_source->equal(*source);

	*sc_source = *source;

	if ( newInstance )
		device->add(sc_source.get());
	else if ( needUpdate )
		sc_source->update();

	// Register calibration
	_touchedObjects.insert(sc_source.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Sync::cleanUp() {
	SEISCOMP_INFO("Clean up inventory");

	// Collect all untouched/remaining objects
	ObjectSet toBeRemoved;
	RemoveUntouchedObjects cleaner(_touchedObjects, toBeRemoved);
	_inv->accept(&cleaner);

	// Detach/delete them
	ObjectSet::iterator it;
	for ( it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it )
		(*it)->detach();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFIR *Sync::process(const ResponseFIR *fir) {
	_session.touchedPublics.insert(fir);
	ResponseFIR *sc_fir = InventoryTask::process(fir);
	_touchedObjects.insert(sc_fir);
	return sc_fir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseIIR *Sync::process(const ResponseIIR *iir) {
	_session.touchedPublics.insert(iir);
	ResponseIIR *sc_iir = InventoryTask::process(iir);
	_touchedObjects.insert(sc_iir);
	return sc_iir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ *Sync::process(const ResponsePAZ *paz) {
	_session.touchedPublics.insert(paz);
	ResponsePAZ *sc_paz = InventoryTask::process(paz);
	_touchedObjects.insert(sc_paz);
	return sc_paz;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial *Sync::process(const ResponsePolynomial *poly) {
	_session.touchedPublics.insert(poly);
	ResponsePolynomial *sc_poly = InventoryTask::process(poly);
	_touchedObjects.insert(sc_poly);
	return sc_poly;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponseFAP *Sync::process(const ResponseFAP *fap) {
	_session.touchedPublics.insert(fap);
	ResponseFAP *sc_fap = InventoryTask::process(fap);
	_touchedObjects.insert(sc_fap);
	return sc_fap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
