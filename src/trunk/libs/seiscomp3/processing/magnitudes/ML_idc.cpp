/***************************************************************************
 *   Copyright (C) Preparatory Commission for the Comprehensive            *
 *   Nuclear-Test-Ban Treaty Organization (CTBTO).                         *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT Magnitudes/mb_idc

#include "ML_idc_private.h"

#include <seiscomp3/logging/log.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/utils/tabvalues.h>

#include <cmath> // log10
#include <vector> // log10
#include <cfloat>
#include <fstream>


#define AMP_TYPE "SBSNR"
#define MAG_TYPE "ML_idc"

#define MINIMUM_DISTANCE 1.0   // in degrees
#define MAXIMUM_DISTANCE 20.0  // in degrees

#define MAXIMUM_DEPTH    40.0  // in km


using namespace std;


namespace {


std::string ExpectedAmplitudeUnit = "nm";

static Util::TabValues tableA;
static bool validTableA = false;
static bool readTableA = false;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude_ML_idc::Magnitude_ML_idc()
: MagnitudeProcessor(MAG_TYPE) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Magnitude_ML_idc::amplitudeType() const {
	return AMP_TYPE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Magnitude_ML_idc::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	if ( !readTableA ) {
		string tablePath = Environment::Instance()->absolutePath("@DATADIR@/magnitudes/IDC/def1.ml");
		validTableA = tableA.read(tablePath);
		if ( !validTableA ) {
			SEISCOMP_ERROR("Failed to read A values from: %s", tablePath.c_str());
		}

		readTableA = true;
	}
	else if ( !validTableA ) {
		SEISCOMP_ERROR("Invalid A value table");
	}

	return validTableA;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status
Magnitude_ML_idc::computeMagnitude(double amplitude,
                                   const std::string &unit,
                                   double period, double,
                                   double delta, double depth,
                                   const DataModel::Origin *,
                                   const DataModel::SensorLocation *,
                                   const DataModel::Amplitude *,
                                   double &value) {
	if ( !validTableA )
		return IncompleteConfiguration;

	if ( (delta < MINIMUM_DISTANCE) || (delta > MAXIMUM_DISTANCE) )
		return DistanceOutOfRange;

	if ( depth > MAXIMUM_DEPTH )
		return DepthOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	double x_1st_deriv, z_1st_deriv, x_2nd_deriv, z_2nd_deriv;
	double interpolated_value;
	int interp_err;

	if ( !tableA.interpolate(interpolated_value, false, true,
	                         delta, 0, &x_1st_deriv, &z_1st_deriv,
	                         &x_2nd_deriv, &z_2nd_deriv, &interp_err) ) {
		return Error;
	}

	if ( interp_err ) {
		return Error;
	}

	value = correctMagnitude(log10(amplitude) + double(interpolated_value));

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_MAGNITUDEPROCESSOR(Magnitude_ML_idc, MAG_TYPE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
}
