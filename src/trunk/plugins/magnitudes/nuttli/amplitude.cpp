/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT MN
#include <seiscomp3/logging/log.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/seismology/ttt/locsat.h>

#include "amplitude.h"
#include "regions.h"
#include "version.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Processing;


#define AMP_TYPE "AMN"
#define DEFAULT_VMIN 3.2
#define DEFAULT_VMAX 3.6


Seiscomp::TravelTimeTableInterfacePtr MNAmplitude::_travelTimeTable;


namespace {


bool computeMDAmplitude(const double *data, size_t npts,
                        double &amplitude, double &period, double &index) {
	if ( npts <= 3 ) return false;

	// This is a port of the Perl code contained in requirements document
	int ipeak_save = -1; // If > 0 indicates that at least 2 peaks or troughs
	                     // have already been found.
	                     // Stores position of 1st of pair of peak/trough
	                     // for largest amp found so far.

	// vel indicates up or down direction of signal
	// as part of search for peaks and troughs.
	int ipeak = -1;  // will be > 0 and indicate position of peak or trough.
	                 // initialize direction in most cases. (a nonzero vel is wanted.)
	double vel = data[2]-data[1];
	for ( size_t isamp = 2; isamp < npts-1; ++isamp ) {
		double vel2 = data[isamp+1]-data[isamp];
		if ( vel2*vel < 0.0 ) {
			// have found a peak or trough at $isamp.
			if ( ipeak >= 0 ) {
				// have found consecutive peak and trough.
				double amp_temp = 0.5 * fabs(data[isamp] - data[ipeak]);
				if ( ipeak_save < 0 || amp_temp > amplitude ) {
					// Save this as the largest so far.
					amplitude = amp_temp;

					// The period will be converted to seconds in
					// AmplitudeProcessor::process. Here we return the period
					// in indexes
					period = 2.0 * (isamp-ipeak);
					ipeak_save = ipeak;
				}
			}

			// store location of current peak
			ipeak = isamp;
			vel = vel2;
		}
		else {
			// re-initialize direction in case where first few samples equal.
			// This will only happen before first peak is found.
			if ( vel == 0 )
				vel = data[isamp+1]-data[isamp];
		}
	}

	if ( ipeak_save < 0 )
		// No amplitude found
		return false;

	// not really time of maximum
	index = ipeak_save;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MNAmplitude::MNAmplitude(): AmplitudeProcessor(AMP_TYPE) {
	setUsedComponent(Vertical);
	setUnit("m/s");
	setDefaults();

	setMinSNR(0);

	// Full depth range
	setMinDepth(-100);
	setMaxDepth(1000);

	// Distance range is 0 to 30 degrees
	setMinDist(0);
	setMaxDist(30);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNAmplitude::setDefaults() {
	_enableResponses = true;
	_useRMS = false;

	_Vmin = DEFAULT_VMIN;
	_Vmax = DEFAULT_VMAX;

	_signalStartPriorities[0] = PoV_Lg;
	_signalStartPriorities[1] = PoV_Sg;
	_signalStartPriorities[2] = PoV_Sn;
	_signalStartPriorities[3] = PoV_S;
	_signalStartPriorities[4] = PoV_Vmax;
	_signalStartPriorities[5] = PoV_Undefined;

	_signalEndPriorities[0] = PoV_Rg;
	_signalEndPriorities[1] = PoV_Vmin;
	_signalEndPriorities[2] = PoV_Undefined;

	setFilter(NULL);

	setNoiseStart(0);
	setNoiseEnd(0);
	setSignalStart(0);
	setSignalEnd(0);

	_snrWindowSeconds = 10;
	_noiseWindowPreSeconds = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNAmplitude::readPriorities(PhaseOrVelocity *priorities,
                                 const Seiscomp::Processing::Settings &settings,
                                 const std::string &parameter) {
	try {
		vector<string> strPriorities;
		Core::split(strPriorities, settings.getString(parameter).c_str(), ",");
		if ( strPriorities.size() > EPhaseOrVelocityQuantity ) {
			SEISCOMP_ERROR("%s: too many priorities, maximum is %d",
			               parameter.c_str(), EPhaseOrVelocityQuantity);
			return false;
		}

		for ( size_t i = 0; i < strPriorities.size(); ++i ) {
			PhaseOrVelocity pov;
			if ( !pov.fromString(strPriorities[i].c_str()) ) {
				SEISCOMP_ERROR("%s: invalid priority at index %d: %s",
				               parameter.c_str(),
				               (int)i, strPriorities[i].c_str());
				return false;
			}

			priorities[i] = pov;
		}

		if ( strPriorities.size() < EPhaseOrVelocityQuantity )
			priorities[strPriorities.size()] = PoV_Undefined;
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNAmplitude::setup(const Settings &settings) {
	setDefaults();

	if ( !Seiscomp::Magnitudes::MN::initialize(settings.localConfiguration) )
		return false;

	if ( !AmplitudeProcessor::setup(settings) )
		return false;

	_networkCode = settings.networkCode;
	_stationCode = settings.stationCode;
	_locationCode = settings.locationCode;

	if ( !_travelTimeTable ) {
		_travelTimeTable = new Seiscomp::TTT::Locsat;

		string vmodel = "iasp91";

		try {
			vmodel = settings.localConfiguration->getString("amplitudes.MN.velocityModel");
		}
		catch ( ... ) {}

		if ( vmodel.empty() )
			SEISCOMP_ERROR("Empty velocity model configured");
		else if ( !_travelTimeTable->setModel(vmodel) ) {
			SEISCOMP_ERROR("Failed to set velocity model: %s", vmodel.c_str());
			_travelTimeTable->setModel("");
			return false;
		}
	}

	if ( _travelTimeTable->model().empty() )
		return false;

	try {
		_useRMS = settings.getBool("amplitudes.MN.rms");
	}
	catch ( ... ) {}

	try {
		string strFilter;
		strFilter = settings.getString("amplitudes.MN.filter");
		Filter *filter = Filter::Create(strFilter);
		if ( filter == NULL ) {
			SEISCOMP_ERROR("Failed to create filter: %s", strFilter.c_str());
			return false;
		}

		setFilter(filter);
	}
	catch ( ... ) {}

	try {
		_Vmin = settings.getDouble("amplitudes.MN.Vmin");
	}
	catch ( ... ) {}

	try {
		_Vmax = settings.getDouble("amplitudes.MN.Vmax");
	}
	catch ( ... ) {}

	try {
		_snrWindowSeconds = settings.getDouble("amplitudes.MN.snrWindowSeconds");
	}
	catch ( ... ) {}

	try {
		_noiseWindowPreSeconds = settings.getDouble("amplitudes.MN.noiseWindowPreSeconds");
	}
	catch ( ... ) {}

	// Read priorities
	if ( !readPriorities(_signalStartPriorities, settings, "amplitudes.MN.signalStartPriorities") )
		return false;

	if ( !readPriorities(_signalEndPriorities, settings, "amplitudes.MN.signalEndPriorities") )
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) MNAmplitude::getDefinedOnset(const PhaseOrVelocity *priorities,
                                         double lat0, double lon0, double depth,
                                         double lat1, double lon1, double dist,
                                         bool left) const {
	// Compute window start
	// First take the manually picked arrivals
	// Then Vmax
	// And as last option compute theoretical arrival times
	for ( int i = 0; i < EPhaseOrVelocityQuantity; ++i ) {
		switch ( priorities[i] ) {
			case PoV_Undefined:
				// Force loop break
				i = EPhaseOrVelocityQuantity;
				break;
			case PoV_Pg:
			case PoV_Pn:
			case PoV_P:
			case PoV_Sg:
			case PoV_Sn:
			case PoV_S:
			case PoV_Lg:
			case PoV_Rg:
			{
				bool acceptAllArrivals = false;
				try {
					// If the origins evaluation mode is manual then all
					// arrivals are accepted even automatic one since they
					// have been checked in the context of the origin
					acceptAllArrivals = _environment.hypocenter->evaluationMode() == DataModel::MANUAL;
				}
				catch ( ... ) {}

				// First search for manually picked phases
				size_t arrivalCount = _environment.hypocenter->arrivalCount();
				for ( size_t j = 0; j < arrivalCount; ++j ) {
					DataModel::Arrival *arr = _environment.hypocenter->arrival(j);
					if ( arr->phase().code() != priorities[i].toString() )
						continue;

					DataModel::Pick *pick = DataModel::Pick::Find(arr->pickID());
					if ( pick == NULL )
						continue;

					if ( pick->waveformID().networkCode() != _networkCode
					  || pick->waveformID().stationCode() != _stationCode
					  || pick->waveformID().locationCode() != _locationCode )
						continue;

					if ( !acceptAllArrivals ) {
						try {
							if ( pick->evaluationMode() != DataModel::MANUAL ) {
								// We do not accept automatic picks
								SEISCOMP_DEBUG("%s.%s.%s: arrival '%s' no accepted, origin evaluation  mode != manual",
								               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
								               arr->phase().code().c_str());
								continue;
							}
						}
						catch ( ... ) {}
					}

					// Phase and location matches, use it
					double onset = pick->time().value() - _environment.hypocenter->time().value();
					double scale = left ? -1.0 : 1.0;
					try {
						onset += scale * pick->time().lowerUncertainty();
					}
					catch ( ... ) {
						try {
							onset += scale * pick->time().uncertainty();
						}
						catch ( ... ) {}
					}

					SEISCOMP_DEBUG("%s.%s.%s: arrival '%s' accepted, onset = %f",
					               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
					               arr->phase().code().c_str(), onset);

					return onset;
				}

				break;
			}
			case PoV_Vmin:
				if ( _Vmin > 0 ) {
					double scale = left ? -1.0 : 1.0;
					double hypoDist = Math::Geo::deg2km(dist);
					hypoDist = sqrt(hypoDist*hypoDist + depth*depth);
					double onset = hypoDist / _Vmin + scale * (1 + 0.05*hypoDist/((_Vmin+_Vmax)*0.5));
					SEISCOMP_DEBUG("%s.%s.%s: vmin = %f",
					               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
					               onset);
					return onset;
				}
				break;
			case PoV_Vmax:
				if ( _Vmax > 0 ) {
					double scale = left ? -1.0 : 1.0;
					double hypoDist = Math::Geo::deg2km(dist);
					hypoDist = sqrt(hypoDist*hypoDist + depth*depth);
					double onset = hypoDist / _Vmax + scale * (1 + 0.05*hypoDist/((_Vmin+_Vmax)*0.5));
					SEISCOMP_DEBUG("%s.%s.%s: vmax = %f",
					               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
					               onset);
					return onset;
				}
				break;
			default:
				break;
		}
	}

	return Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) MNAmplitude::getEarliestOnset(double lat0, double lon0, double depth,
                                          double lat1, double lon1, double dist) const {
	OPT(double) minimumOnset;
	bool acceptAllArrivals = true;

	// First search for manually picked phases
	size_t arrivalCount = _environment.hypocenter->arrivalCount();
	for ( size_t i = 0; i < arrivalCount; ++i ) {
		DataModel::Arrival *arr = _environment.hypocenter->arrival(i);
		DataModel::Pick *pick = DataModel::Pick::Find(arr->pickID());
		if ( pick == NULL )
			continue;

		if ( pick->waveformID().networkCode() != _networkCode
		  || pick->waveformID().stationCode() != _stationCode
		  || pick->waveformID().locationCode() != _locationCode )
			continue;

		if ( !acceptAllArrivals ) {
			try {
				if ( pick->evaluationMode() != DataModel::MANUAL ) {
					// We do not accept automatic picks
					SEISCOMP_DEBUG("%s.%s.%s: arrival '%s' no accepted, origin evaluation  mode != manual",
					               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
					               arr->phase().code().c_str());
					continue;
				}
			}
			catch ( ... ) {}
		}

		// Phase and location matches, use it
		double onset = pick->time().value() - _environment.hypocenter->time().value();
		if ( !minimumOnset || *minimumOnset > onset )
			minimumOnset = onset;
	}

	if ( minimumOnset )
		return *minimumOnset;

	minimumOnset = Core::None;

	PhaseOrVelocity earliest[3] = { PoV_Pg, PoV_Pn, PoV_P };
	for ( int i = 0; i < 3; ++i ) {
		switch ( earliest[i] ) {
			case PoV_Pg:
			case PoV_Pn:
			case PoV_P:
			case PoV_Sg:
			case PoV_Sn:
			case PoV_S:
			case PoV_Lg:
			case PoV_Rg:
				try {
					TravelTime tt = _travelTimeTable->compute(earliest[i].toString(), lat0, lon0, depth, lat1, lon1, 0, 1);
					if ( tt.time < 0 )
						break;

					if ( !minimumOnset || *minimumOnset > tt.time )
						minimumOnset = tt.time;
				}
				catch ( ... ) {}
				break;
			default:
				break;
		}
	}

	return minimumOnset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNAmplitude::setHint(ProcessingHint hint, double value) {
	// We don't care about simple hints like distance and depth. We need
	// the full environment with origin, receiver and pick information
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNAmplitude::setEnvironment(const Seiscomp::DataModel::Origin *hypocenter,
                                 const Seiscomp::DataModel::SensorLocation *receiver,
                                 const Seiscomp::DataModel::Pick *pick) {
	AmplitudeProcessor::setEnvironment(hypocenter, receiver, pick);

	double hypoLat, hypoLon, hypoDepth;
	double recvLat, recvLon;

	if ( _environment.hypocenter == NULL ) {
		setStatus(MissingHypocenter, 0);
		return;
	}

	try {
		// All attributes are optional and throw an exception if not set
		hypoLat = _environment.hypocenter->latitude().value();
		hypoLon = _environment.hypocenter->longitude().value();
		hypoDepth = _environment.hypocenter->depth().value();
	}
	catch ( ... ) {
		setStatus(MissingHypocenter, 1);
		return;
	}

	if ( _environment.receiver == NULL ) {
		setStatus(MissingReceiver, 0);
		return;
	}

	try {
		// Both attributes are optional and throw an exception if not set
		recvLat = _environment.receiver->latitude();
		recvLon = _environment.receiver->longitude();
	}
	catch ( ... ) {
		setStatus(MissingReceiver, 1);
		return;
	}

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(hypoLat, hypoLon) ) {
		setStatus(EpicenterOutOfRegions, 0);
		return;
	}

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(recvLat, recvLon) ) {
		setStatus(ReceiverOutOfRegions, 0);
		return;
	}

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(hypoLat, hypoLon,
	                                               recvLat, recvLon) ) {
		setStatus(RayPathOutOfRegions, 0);
		return;
	}

	double dist, az, baz;
	Math::Geo::delazi_wgs84(hypoLat, hypoLon, recvLat, recvLon,
	                        &dist, &az, &baz);

	if ( dist < _config.minimumDistance || dist > _config.maximumDistance ) {
		setStatus(DistanceOutOfRange, dist);
		return;
	}

	if ( hypoDepth < _config.minimumDepth || hypoDepth > _config.maximumDepth ) {
		setStatus(DepthOutOfRange, hypoDepth);
		return;
	}

	OPT(double) noiseWindowEnd    = getEarliestOnset(hypoLat, hypoLon, hypoDepth,
	                                                 recvLat, recvLon, dist);
	OPT(double) signalWindowStart = getDefinedOnset(_signalStartPriorities,
	                                                hypoLat, hypoLon, hypoDepth,
	                                                recvLat, recvLon, dist, true);
	OPT(double) signalWindowEnd   = getDefinedOnset(_signalEndPriorities,
	                                                hypoLat, hypoLon, hypoDepth,
	                                                recvLat, recvLon, dist, false);

	if ( !noiseWindowEnd || !signalWindowStart || !signalWindowEnd ) {
		// Use error code 1 as time window computation error
		setStatus(Error, 1);
	}

	if ( *signalWindowStart >= *signalWindowEnd ) {
		// Use error code 2 as empty or invalid time window
		setStatus(Error, 2);
	}

	*noiseWindowEnd -= _noiseWindowPreSeconds;

	double noiseWindowStart = *noiseWindowEnd - _snrWindowSeconds;

	double pickOffset = _trigger - _environment.hypocenter->time().value();
	noiseWindowStart -= pickOffset;
	*noiseWindowEnd -= pickOffset;
	*signalWindowStart-= pickOffset;
	*signalWindowEnd -= pickOffset;

	setNoiseStart(noiseWindowStart);
	setNoiseEnd(*noiseWindowEnd);
	setSignalStart(*signalWindowStart);
	setSignalEnd(*signalWindowEnd);

	SEISCOMP_DEBUG("%s.%s.%s: %f : %f : %f : %f",
	               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
	               noiseWindowStart, *noiseWindowEnd, *signalWindowStart, *signalWindowEnd);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNAmplitude::prepareData(Seiscomp::DoubleArray &data) {
	SignalUnit unit;
	if ( !unit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
		// Invalid unit string
		setStatus(IncompatibleUnit, 2);
		return;
	}

	if ( unit != MeterPerSecond ) {
		// Wrong unit
		setStatus(IncompatibleUnit, 1);
		return;
	}

	if ( _streamConfig[_usedComponent].gain == 0.0 ) {
		// Invalid gain
		setStatus(MissingGain, 1);
		return;
	}

	if ( _enableResponses ) {
		Sensor *sensor = _streamConfig[_usedComponent].sensor();
		if ( !sensor ) {
			// No meta-data associated
			setStatus(MissingResponse, 1);
			return;
		}

		if ( !sensor->response() ) {
			// No response stored
			setStatus(MissingResponse, 2);
			return;
		}
	}

	if ( !_streamConfig[_usedComponent].gainFrequency ) {
		setStatus(IncompleteMetadata, 0);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNAmplitude::computeNoise(const Seiscomp::DoubleArray &data,
                               int i1, int i2,
                               double *offset, double *amplitude) {
	if ( _useRMS ) {
		*offset = Seiscomp::Math::Statistics::mean(i2-i1, data.typedData()+i1);
		*amplitude = 0;

		for ( int i = i1; i < i2; ++i ) {
			double amp = data[i] - *offset;
			*amplitude += amp * amp;
		}

		*amplitude = sqrt(*amplitude / (i2-i1));
		SEISCOMP_DEBUG("Noise amplitude in data[%d:%d] = %f", i1, i2, *amplitude);

		return true;
	}
	else {
		size_t npts = size_t(i2-i1);
		double period, index;

		*amplitude = -1;
		*offset = 0;

		bool result = computeMDAmplitude(data.typedData() + i1, npts, *amplitude, period, index);;
		SEISCOMP_DEBUG("Noise amplitude in data[%d:%d] = %f", i1, i2, *amplitude);

		return result;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNAmplitude::computeAmplitude(const Seiscomp::DoubleArray &dataArray,
                                   size_t, size_t, // Ignore i1 and i2
                                   size_t si1, size_t si2,
                                   double offset,
                                   AmplitudeIndex *dt,
                                   AmplitudeValue *amplitude,
                                   double *period, double *snr) {
	const double *data = dataArray.typedData() + si1;
	size_t npts = si2 - si1;

	Math::Restitution::FFT::TransferFunctionPtr tf;

	if ( _enableResponses ) {
		Sensor *sensor = _streamConfig[_usedComponent].sensor();
		tf = sensor->response()->getTransferFunction();
		if ( !tf ) {
			setStatus(MissingResponse, 3);
			return false;
		}
	}

	if ( !computeMDAmplitude(data, npts, amplitude->value, *period, dt->index) )
		return false;

	dt->index += si1;
	SEISCOMP_DEBUG("Amplitude in data[%d:%d] = %f at %d",
	               int(si1), int(si1 + npts), amplitude->value, int(dt->index));

	if ( _useRMS ) {
		// The length of peak to trough in samples. The index is the smallest index
		// of the trough-peak or peak-trough pair.
		double t_len = *period * 0.5;
		// The average index of peak and trough
		double t_zero = dt->index + t_len * 0.5;

		// The start of the snr signal window in samples with respect to data window
		int snrWindowStart = int(t_zero - _snrWindowSeconds*0.5*_stream.fsamp);
		// The end of the snr signal window in samples with respect to data window
		int snrWindowEnd = int(t_zero + _snrWindowSeconds*0.5*_stream.fsamp);

		if ( snrWindowStart < int(si1) ) {
			int ofs = si1-snrWindowStart;
			snrWindowStart += ofs;
			snrWindowEnd += ofs;
		}
		else if ( snrWindowEnd > int(si2) ) {
			int ofs = si2-snrWindowEnd;
			snrWindowEnd += ofs;
			snrWindowStart += ofs;
		}

		if ( snrWindowStart < 0 ) snrWindowStart = 0;
		if ( snrWindowEnd > dataArray.size() ) snrWindowEnd = dataArray.size();

		data = dataArray.typedData() + snrWindowStart;
		npts = snrWindowEnd - snrWindowStart;

		offset = Seiscomp::Math::Statistics::mean(npts, data);
		double rms = 0;

		for ( size_t i = 0; i < npts; ++i ) {
			double amp = data[i] - offset;
			rms += amp * amp;
		}

		rms = sqrt(rms / npts);

		SEISCOMP_DEBUG("Signal snr amplitude in data[%d:%d] = %f", snrWindowStart, snrWindowEnd, rms);

		*snr = rms / *noiseAmplitude();
	}
	else
		*snr = amplitude->value / *noiseAmplitude();

	// Amplitude is now in SI
	amplitude->value /= _streamConfig[_usedComponent].gain;

	if ( (_config.snrMin > 0) && (*snr < _config.snrMin) ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	if ( tf ) {
		// Correct according to period and the transfer function
		Math::Complex amplitudeResponse, sensorResponse;
		double amplitudeFrequency = _stream.fsamp / *period;

		tf->evaluate(&amplitudeResponse, 1, &amplitudeFrequency);
		tf->evaluate(&sensorResponse, 1, &*_streamConfig[_usedComponent].gainFrequency);

		double scale = abs(sensorResponse) / abs(amplitudeResponse);
		amplitude->value *= scale;

		SEISCOMP_DEBUG("%s.%s.%s: amp = %f, snr = %f, period = %fs, "
		               "value at period = %f, value at gain frequency = %f, "
		               "correction = %f, corr(amp) = %f",
		               _networkCode.c_str(), _stationCode.c_str(), _locationCode.c_str(),
		               amplitude->value, *snr, *period, abs(amplitudeResponse),
		               abs(sensorResponse), scale, amplitude->value * scale);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNAmplitude::finalizeAmplitude(DataModel::Amplitude *amplitude) const {
	if ( amplitude == NULL )
		return;

	try {
		amplitude->creationInfo().setVersion(MN_VERSION);
	}
	catch ( ... ) {
		DataModel::CreationInfo ci;
		ci.setVersion(MN_VERSION);
		amplitude->setCreationInfo(ci);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Creates and registers a factory for type "AMN" and class MNAmplitude.
// This allows to create an abstract amplitude processor later with
// AmplitudeProcessorFactory::Create("AMN")
REGISTER_AMPLITUDEPROCESSOR(MNAmplitude, AMP_TYPE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
