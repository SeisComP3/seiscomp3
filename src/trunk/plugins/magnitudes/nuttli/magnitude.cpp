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
#include <seiscomp3/math/geo.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <math.h>

#include "magnitude.h"
#include "regions.h"
#include "version.h"


#define AMP_TYPE "AMN"
#define MAG_TYPE "MN"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Processing;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MNMagnitude::MNMagnitude()
: MagnitudeProcessor(MAG_TYPE)
, _minSNR(2)
, _minPeriod(0.01)
, _maxPeriod(1.3)
, _minDistance(0.5)
, _maxDistance(30) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNMagnitude::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	_minSNR = 2;
	_minPeriod = 0.01;
	_maxPeriod = 1.3;
	_minDistance = 0.5;
	_maxDistance = 30;

	try { _minSNR = settings.getDouble("magnitudes." MAG_TYPE ".minSNR"); }
	catch ( ... ) {}
	try { _minPeriod = settings.getDouble("magnitudes." MAG_TYPE ".minPeriod"); }
	catch ( ... ) {}
	try { _maxPeriod = settings.getDouble("magnitudes." MAG_TYPE ".maxPeriod"); }
	catch ( ... ) {}
	try { _minDistance = settings.getDouble("magnitudes." MAG_TYPE ".minDist"); }
	catch ( ... ) {}
	try { _maxDistance = settings.getDouble("magnitudes." MAG_TYPE ".maxDist"); }
	catch ( ... ) {}

	return Seiscomp::Magnitudes::MN::initialize(settings.localConfiguration);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MNMagnitude::Status
MNMagnitude::computeMagnitude(double amplitude,
                              const string &unit,
                              double period,
                              double snr,
                              double delta, double depth,
                              const Seiscomp::DataModel::Origin *hypocenter,
                              const Seiscomp::DataModel::SensorLocation *receiver,
                              const DataModel::Amplitude *,
                              double &value) {
	Status status = OK;

	// Reset the valid flag. The flag will be selectively enabled to
	// forward the magnitude for association with weight 0.
	_validValue = false;

	// Both objects are required to make sure that they both lie inside the
	// configured region.
	if ( hypocenter == NULL || receiver == NULL )
		return MetaDataRequired;

	try {
		// All attributes must be set
		hypocenter->latitude().value();
		hypocenter->longitude().value();
		receiver->latitude();
		receiver->longitude();
	}
	catch ( ... ) {
		return MetaDataRequired;
	}

	double dist, az, baz;

	// The SensorLocation can throw exceptions if either latitude or
	// longitude isn't set. In that case the magnitude cannot be computed.
	try {
		Math::Geo::delazi_wgs84(hypocenter->latitude(), hypocenter->longitude(),
		                        receiver->latitude(), receiver->longitude(),
		                        &dist, &az, &baz);
	}
	catch ( ... ) {
		return MetaDataRequired;
	}

	if ( dist > _maxDistance )
		return DistanceOutOfRange;

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(
	         hypocenter->latitude(), hypocenter->longitude()
	      ) )
		return EpicenterOutOfRegions;

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(
	         receiver->latitude(), receiver->longitude()
	      ) )
		return ReceiverOutOfRegions;

	if ( !Seiscomp::Magnitudes::MN::isInsideRegion(
	         hypocenter->latitude(), hypocenter->longitude(),
	         receiver->latitude(), receiver->longitude()
	      ) )
		return RayPathOutOfRegions;

	if ( period < _minPeriod || period > _maxPeriod ) {
		status = PeriodOutOfRange;
		_validValue = true;
	}

	if ( snr < _minSNR ) {
		status = SNROutOfRange;
		_validValue = true;
	}

	if ( dist < _minDistance ) {
		// Forward close magnitudes but associate
		// them with zero weight
		status = DistanceOutOfRange;
		_validValue = true;
	}

	// Convert m/s to um/s
	amplitude *= 1E6;

	// The method correctMagnitude automatically scales and offsets the input
	// value with the configured coefficients. They are by default 1 and 0.
	// correctMagnitude(x) -> x * _linearCorrection + _constantCorrection
	value = correctMagnitude(3.3 + 1.66*log10(dist) + log10(amplitude / (2*M_PI)));

	return status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MNMagnitude::amplitudeType() const {
	static std::string type = AMP_TYPE;
	return type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNMagnitude::finalizeMagnitude(Seiscomp::DataModel::StationMagnitude *magnitude) const {
	if ( magnitude == NULL )
		return;

	try {
		magnitude->creationInfo().setVersion(MN_VERSION);
	}
	catch ( ... ) {
		DataModel::CreationInfo ci;
		ci.setVersion(MN_VERSION);
		magnitude->setCreationInfo(ci);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNMagnitude::treatAsValidMagnitude() const {
	return _validValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Creates and registers a factory for type "MN" and class MNMagnitude.
// This allows to create an abstract amplitude processor later with
// MagnitudeProcessorFactory::Create("MN")
REGISTER_MAGNITUDEPROCESSOR(MNMagnitude, MAG_TYPE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
