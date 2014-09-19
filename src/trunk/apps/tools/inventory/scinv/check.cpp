/***************************************************************************
 * Copyright (C) 2013 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/

#define SEISCOMP_COMPONENT INVMGR

#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>

#include "check.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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


string toString(const Core::TimeWindow &tw ) {
	string res;
	if ( tw.startTime().microseconds() > 0 )
		res = tw.startTime().toString("%F %T.%f");
	else
		res = tw.startTime().toString("%F %T");

	res += " - ";

	if ( tw.endTime().valid() ) {
		if ( tw.endTime().microseconds() > 0 )
			res += tw.endTime().toString("%F %T.%f");
		else
			res += tw.endTime().toString("%F %T");
	}
	else
		res += "...";

	return res;
}


bool overlaps(const Core::TimeWindow &tw1, const Core::TimeWindow &tw2) {
	// Both end times are open -> overlap
	if ( !tw1.endTime().valid() && !tw2.endTime().valid() ) return true;

	// Check overlap on both closed epochs
	if ( tw1.endTime().valid() && tw2.endTime().valid() ) {
		if ( tw1.startTime() >= tw2.endTime() ) return false;
		if ( tw1.endTime() <= tw2.startTime() ) return false;
	}
	// tw1 is an open epoch
	else if ( !tw1.endTime().valid() ) {
		if ( tw2.endTime() <= tw1.startTime() ) return false;
	}
	// tw2 is an open epoch
	else {
		if ( tw1.endTime() <= tw2.startTime() ) return false;
	}

	return true;
}


const Core::TimeWindow *
overlaps(const Check::TimeWindows &epochs, const Core::TimeWindow &tw) {
	Check::TimeWindows::const_iterator it;
	for ( it = epochs.begin(); it != epochs.end(); ++it ) {
		const Core::TimeWindow &epoch = *it;
		if ( overlaps(epoch, tw) )
			return &epoch;
	}

	return NULL;
}


bool outside(const Core::TimeWindow &fence, const Core::TimeWindow &child) {
	if ( child.startTime() < fence.startTime() ) return true;
	if ( !child.endTime().valid() && fence.endTime().valid() ) return true;

	// Check overlap on both closed epochs
	if ( fence.endTime().valid() && child.endTime().valid() ) {
		if ( child.startTime() > fence.endTime() ) return true;
		if ( child.endTime() > fence.endTime() ) return true;
	}

	return false;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Check::Check(Inventory *inv) : InventoryTask(inv) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Check::check() {
	if ( _inv == NULL ) return false;

	EpochMap networkEpochs;

	for ( size_t n = 0; n < _inv->networkCount(); ++n ) {
		Network *net = _inv->network(n);
		checkEpoch(net);
		checkOverlap(networkEpochs[net->code()], net);

		EpochMap stationEpochs;

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			Station *sta = net->station(s);
			checkEpoch(sta);
			checkOverlap(stationEpochs[sta->code()], sta);
			checkOutside(net, sta);

			EpochMap locationEpochs;
			double lat = sta->latitude();
			double lon = sta->longitude();

			if ( lat == 0.0 && lon == 0.0 ) {
				log(LogHandler::Warning,
				    (string(sta->className()) + " " + id(sta) + "\n  "
				     "coordinates are 0.0/0.0").c_str(),
				     NULL, NULL);
			}

			for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
				SensorLocation *loc = sta->sensorLocation(l);
				checkEpoch(loc);
				checkOverlap(locationEpochs[loc->code()], loc);
				checkOutside(sta, loc);

				double llat = loc->latitude();
				double llon = loc->longitude();

				if ( llat == 0.0 && llon == 0.0 ) {
					log(LogHandler::Warning,
					    (string(loc->className()) + " " + id(loc) + "\n  "
					     "coordinates are 0.0/0.0").c_str(),
					     NULL, NULL);
				}

				double dist,a1,a2;

				Math::Geo::delazi(lat,lon,llat,llon, &dist, &a1, &a2);
				dist = Math::Geo::deg2km(dist);

				if ( dist > 10 ) {
					log(LogHandler::Warning,
					    (string(loc->className()) + " " + id(loc) + "\n  "
					     "location is " + Core::toString(dist) + "km away from parent Station").c_str(),
					     NULL, NULL);
				}

				EpochMap channelEpochs;

				for ( size_t c = 0; c < loc->streamCount(); ++c ) {
					Stream *cha = loc->stream(c);
					checkEpoch(cha);
					checkOverlap(channelEpochs[cha->code()], cha);
					checkOutside(loc, cha);
					try {
						if ( cha->gain() == 0.0 ) {
							log(LogHandler::Warning,
							    (string(cha->className()) + " " + id(cha) + "\n  "
							     "invalid gain of 0").c_str(), NULL, NULL);
						}
					}
					catch ( ... ) {
						log(LogHandler::Warning,
						    (string(cha->className()) + " " + id(cha) + "\n  "
						     "no gain set").c_str(), NULL, NULL);
					}

					if ( cha->gainUnit().empty() ) {
						log(LogHandler::Warning,
						    (string(cha->className()) + " " + id(cha) + "\n  "
						     "no gain unit set").c_str(), NULL, NULL);
					}

					if ( !cha->sensor().empty() ) {
						// Done already in merge
						/*
						Sensor *sensor = findSensor(cha->sensor());
						if ( sensor == NULL ) {
							log(LogHandler::Unresolved,
							    (string(cha->className()) + " " + id(cha) + "\n  "
							     "referenced sensor is not available").c_str(), NULL, NULL);
						}
						*/
					}
					else
						log(LogHandler::Information,
						    (string(cha->className()) + " " + id(cha) + "\n  "
						     "no sensor and thus no response information available").c_str(), NULL, NULL);
				}
			}
		}
	}

	for ( size_t i = 0; i < _inv->sensorCount(); ++i ) {
		Sensor *sensor = _inv->sensor(i);
		Object *o = findPAZ(sensor->response());
		if ( o == NULL ) o = findPoly(sensor->response());
		if ( o == NULL ) {
			// Done in merge
			/*
			log(LogHandler::Unresolved,
			    (string(sensor->className()) + " " + id(sensor) + "\n  "
			     "referenced response is not available").c_str(), NULL, NULL);
			*/
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void Check::checkEpoch(const T *obj) {
	try {
		if ( obj->start() >= obj->end() ) {
			log(LogHandler::Error,
			    (string(obj->className()) + " " + id(obj) + "\n  "
			     "invalid epoch: start >= end: " +
			     toString(Core::TimeWindow(obj->start(), obj->end()))).c_str(),
			     NULL, NULL);
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void Check::checkOverlap(TimeWindows &epochs, const T *obj) {
	Core::Time end;
	try { end = obj->end(); }
	catch ( ... ) {}

	Core::TimeWindow epoch(obj->start(), end);
	const Core::TimeWindow *tw = overlaps(epochs, epoch);
	if ( tw != NULL ) {
		log(LogHandler::Conflict,
		    (string(obj->className()) + " " + id(obj) + "\n  "
		     "overlapping epochs " +
		     toString(epoch) + " and " + toString(*tw)).c_str(), NULL, NULL);
	}

	epochs.push_back(epoch);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T1, typename T2>
void Check::checkOutside(const T1 *parent, const T2 *obj) {
	Core::Time pend, end;

	try { pend = parent->end(); }
	catch ( ... ) {}
	try { end = obj->end(); }
	catch ( ... ) {}

	Core::TimeWindow pepoch(parent->start(), pend);
	Core::TimeWindow epoch(obj->start(), end);

	if ( outside(pepoch, epoch) ) {
		log(LogHandler::Conflict,
		    (string(obj->className()) + " " + id(obj) + "\n  "
		     "epoch " + toString(epoch) + " outside parent " +
		     parent->className() + " epoch " + toString(pepoch)).c_str(),
		     NULL, NULL);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *Check::findSensor(const string &id) const {
	for ( size_t i = 0; i < _inv->sensorCount(); ++i ) {
		Sensor *sensor = _inv->sensor(i);
		if ( sensor->publicID() == id ) return sensor;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePAZ *Check::findPAZ(const string &id) const {
	for ( size_t i = 0; i < _inv->responsePAZCount(); ++i ) {
		ResponsePAZ *paz = _inv->responsePAZ(i);
		if ( paz->publicID() == id ) return paz;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ResponsePolynomial *Check::findPoly(const string &id) const {
	for ( size_t i = 0; i < _inv->responsePolynomialCount(); ++i ) {
		ResponsePolynomial *poly = _inv->responsePolynomial(i);
		if ( poly->publicID() == id ) return poly;
	}

	return NULL;
}
