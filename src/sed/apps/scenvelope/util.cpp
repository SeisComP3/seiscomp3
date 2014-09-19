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
