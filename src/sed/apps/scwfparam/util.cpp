/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#include "util.h"

#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/waveformstreamid.h>


/*
#include <seiscomp3/core/platform/platform.h>
#ifdef __APPLE__
// On OSX HOST_NAME_MAX is not define in sys/params.h. Therefore we use
// instead on this platform.
#include <sys/param.h>
#define HOST_NAME_MAX MAXHOSTNAMELEN
#endif
*/
#include <seiscomp3/core/system.h>

#include <iomanip>


using namespace std;

namespace Seiscomp {
namespace Private {

namespace {

bool passes(const StringSet &ids, const std::string &id) {
	StringSet::iterator it;
	for ( it = ids.begin(); it != ids.end(); ++it ) {
		if ( Core::wildcmp(*it, id) )
			return true;
	}

	return false;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isAllowed(const std::string &s) const {
	// If no rules are configured, don't cache anything and just return true
	if ( allow.empty() && deny.empty() ) return true;

	StringPassMap::const_iterator it = cache.find(s);

	// Not yet cached, evaluate the string
	if ( it == cache.end() ) {
		bool check = (allow.empty()?true:passes(allow, s))
		          && (deny.empty()?true:!passes(deny, s));
		cache[s] = check;
		return check;
	}

	// Return cached result
	return it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isBlocked(const std::string &s) const {
	return !isAllowed(s);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool equivalent(const DataModel::WaveformStreamID &wfid1,
                const DataModel::WaveformStreamID &wfid2) {

	if ( wfid1.networkCode() != wfid2.networkCode() ) return false;
	if ( wfid1.stationCode() != wfid2.stationCode() ) return false;
	if ( wfid1.channelCode() != wfid2.channelCode() ) return false;
	// here we consider diffenent location codes to be irrelevant
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double
arrivalWeight(const DataModel::Arrival *arr, double defaultWeight) {
	try {
		return arr->weight();
	}
	catch ( Core::ValueException& ) {
		return defaultWeight;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double
arrivalDistance(const DataModel::Arrival *arr) {
	// TODO: If there is no distance set an exception will be thrown.
	//       The distance has to be calculated manually then.
	return arr->distance();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::EvaluationStatus
status(const DataModel::Origin *origin) {
	DataModel::EvaluationStatus status = DataModel::CONFIRMED;

	try {
		status = origin->evaluationStatus();
	}
	catch ( Core::ValueException& ) {}

	return status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char shortPhaseName(const std::string &phase) {
	for ( std::string::const_reverse_iterator it = phase.rbegin();
	      it != phase.rend(); ++it ) {
		if ( isupper(*it ) )
			return *it;
	}

	return phase[0];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toStationID(const DataModel::WaveformStreamID &wfid) {
	const std::string
		&net = wfid.networkCode(),
		&sta = wfid.stationCode();

	return net + "." + sta;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toStreamID(const DataModel::WaveformStreamID &wfid) {
	const std::string
		&net = wfid.networkCode(),
		&sta = wfid.stationCode(),
		&loc = wfid.locationCode(),
		&cha = wfid.channelCode();

	return net + "." + sta + "." + loc + "." + cha;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::WaveformStreamID
setStreamComponent(const DataModel::WaveformStreamID &id, char comp) {
	return DataModel::WaveformStreamID(
		id.networkCode(),
		id.stationCode(),
		id.locationCode(),
		id.channelCode().substr(0,id.channelCode().size()-1) + comp,
		id.resourceURI());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Stream*
findStream(DataModel::Station *station, const Core::Time &time,
           Processing::WaveformProcessor::SignalUnit requestedUnit) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		DataModel::SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			DataModel::Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			//Sensor *sensor = Sensor::Find(stream->sensor());
			//if ( !sensor ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			//if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			if ( !unit.fromString(stream->gainUnit().c_str()) ) continue;
			if ( unit != requestedUnit ) continue;

			return stream;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Stream*
findStreamMaxSR(DataModel::Station *station, const Core::Time &time,
                Processing::WaveformProcessor::SignalUnit requestedUnit,
                const StringFirewall *firewall) {
	string stationID = station->network()->code() + "." + station->code();
	DataModel::Stream *res = NULL;
	double fsampMax = 0.0;

	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		DataModel::SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			DataModel::Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			//if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			if ( !unit.fromString(stream->gainUnit().c_str()) ) continue;
			if ( unit != requestedUnit ) continue;

			if ( firewall != NULL ) {
				string streamID = stationID + "." + loc->code() + "." + stream->code();
				if ( firewall->isBlocked(streamID) ) continue;
			}

			try {
				if ( stream->sampleRateDenominator() == 0 )  {
					if ( res == NULL ) res = stream;
					continue;
				}
				else {
					double fsamp = stream->sampleRateNumerator() / stream->sampleRateDenominator();
					if ( fsamp > fsampMax ) {
						res = stream;
						fsampMax = fsamp;
						continue;
					}
				}
			}
			catch ( Core::ValueException& ) {
				if ( res == NULL ) res = stream;
				continue;
			}
		}
	}

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
