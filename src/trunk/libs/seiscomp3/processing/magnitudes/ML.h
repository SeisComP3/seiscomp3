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



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_ML_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_ML_H__

#include <seiscomp3/processing/magnitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_ML : public MagnitudeProcessor {
	DECLARE_SC_CLASS(MagnitudeProcessor_ML);

	public:
		MagnitudeProcessor_ML();


	public:
		bool setup(const Settings &settings);

		std::string amplitudeType() const;

		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        double &value);


	private:
		double logA0(double dist_km) const;


	private:
		std::vector<double> logA0_dist, logA0_val;
		double maxDistanceKm;
};


}

}


#endif
