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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_ML_IDC_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_ML_IDC_H

#include <seiscomp3/processing/magnitudeprocessor.h>


using namespace Seiscomp;
using namespace Seiscomp::Processing;


namespace {


class SC_SYSTEM_CLIENT_API Magnitude_ML_idc : public MagnitudeProcessor {
	public:
		Magnitude_ML_idc();

	public:
		virtual std::string amplitudeType() const;

		virtual bool setup(const Settings &settings);

		virtual Status computeMagnitude(double amplitude, const std::string &unit,
		                                double period, double snr,
		                                double delta, double depth,
		                                const DataModel::Origin *hypocenter,
		                                const DataModel::SensorLocation *receiver,
		                                const DataModel::Amplitude *,
		                                double &value);
};


}


#endif
