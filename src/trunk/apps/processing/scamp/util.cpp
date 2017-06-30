/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/




#include "util.h"

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
std::ostream &
operator<<(std::ostream &os, const DataModel::Amplitude &amp) {
	double val, err;

	Core::Time pickTime;
	DataModel::Pick *p = DataModel::Pick::Find(amp.pickID());
	if ( p )
		pickTime = p->time().value();

	os << "Amplitude " << amp.publicID() << endl;
	os << "    type:               " << amp.type() << endl;
	os << "    waveform ID:        " << toStreamID(amp.waveformID()) << endl;
	os << "    pick ID:            " << amp.pickID() << endl;
	os << "    pick time:          " << (pickTime?pickTime.toString("%F %T.%f"):"N.A.") << endl;

	val  = amp.amplitude().value();
	os << setprecision(9);
	try {
		err = 0.5*(amp.amplitude().lowerUncertainty() + amp.amplitude().upperUncertainty());
		os << "    amplitude:          " << val << " +/- " << err << endl;
	}
	catch(...) {
		os << "    amplitude:          " << val << endl;
	}

	try {
		val  = amp.period().value();
		os << setprecision(4);
		try {
			err = 0.5*(amp.period().lowerUncertainty() + amp.period().upperUncertainty());
			os << "    period:             " << val << " +/- " << err << endl;
		}
		catch(...) {
			os << "    period:             " << val << endl;
		}
	}
	catch(...) {
		os << "    period:             N.D." << endl;
	}

	try {
		const DataModel::TimeWindow &tw = amp.timeWindow();
		double dt0 = double(tw.reference()-pickTime), dt1 = dt0+tw.begin(), dt2 = dt0+tw.end();
		os << setprecision(2);
		os << "    measurement time:   " << dt0 << endl;
		os << "    measurement window: " << dt1 << " ... " << dt2 << "    length: " << dt2-dt1 << endl;
	}
	catch(...) {
		os << "no measurement time window for amplitude " << amp.publicID() << endl;
	}

	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
