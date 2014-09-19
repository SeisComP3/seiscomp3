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



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MLV_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MLV_H__

#include <vector>
#include <seiscomp3/processing/magnitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_MLv : public MagnitudeProcessor {
	DECLARE_SC_CLASS(MagnitudeProcessor_MLv);

	public:
		MagnitudeProcessor_MLv();


	public:
		bool setup(const Settings &settings);
	public:
		Status computeMagnitude(
			double amplitude, // in micrometers per second
			double period,      // in seconds
			double delta,     // in degrees
			double depth,     // in kilometers
			double &value);
	private:
		double logA0(double dist_km) const throw(Core::ValueException);
		std::vector<double> logA0_dist, logA0_val;
		double maxDistanceKm;
};


}

}


#endif
