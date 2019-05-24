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


#ifndef __SEISCOMP_CUSTOM_MAGNITUDE_NUTTLI_H__
#define __SEISCOMP_CUSTOM_MAGNITUDE_NUTTLI_H__


#include <seiscomp3/core/version.h>
#include <seiscomp3/processing/magnitudeprocessor.h>


namespace {


class MNMagnitude : public Seiscomp::Processing::MagnitudeProcessor {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		MNMagnitude();


	// ----------------------------------------------------------------------
	//  MagnitudeProcessor interface
	// ----------------------------------------------------------------------
	public:
		virtual std::string amplitudeType() const;
		virtual void finalizeMagnitude(Seiscomp::DataModel::StationMagnitude *magnitude) const;


	protected:
		virtual bool setup(const Seiscomp::Processing::Settings &settings);

		// The MagnitudeProcessor interface has changed with API version 11
		// and API version 12.
		// The following compile time condition accounts for that.
		virtual Status computeMagnitude(double amplitude,
		                                const std::string &unit,
		                                double period,
		                                double snr,
		                                double delta, double depth,
		                                const Seiscomp::DataModel::Origin *hypocenter,
		                                const Seiscomp::DataModel::SensorLocation *receiver,
		                                const Seiscomp::DataModel::Amplitude *,
		                                double &value);

		virtual bool treatAsValidMagnitude() const;


	private:
		bool _validValue;
		double _minSNR;
		double _minPeriod;
		double _maxPeriod;
		double _minDistance;
		double _maxDistance;
};


}


#endif
