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


#include <seiscomp3/processing/magnitudes/ML.h>
#include <seiscomp3/seismology/magnitudes.h>


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_ML, MagnitudeProcessor, "MagnitudeProcessor_ML");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_ML, "ML");


MagnitudeProcessor_ML::MagnitudeProcessor_ML() : Processing::MagnitudeProcessor("ML") {}

std::string MagnitudeProcessor_ML::amplitudeType() const {
	return "ML";
}


MagnitudeProcessor::Status MagnitudeProcessor_ML::computeMagnitude(
			double amplitude,   // in micrometers per second
			double period,      // in seconds
			double delta,       // in degrees
			double depth,       // in kilometers
			double &value)
{
	double mag;
	bool res = Magnitudes::compute_ML(amplitude, delta, depth, &mag);
	//bool res = true;
	//ML = 1.0 * log10(amplitude) + 1.11*log10(delta) + 0.00189*delta - 2.09
	value = correctMagnitude(mag);

	return res?OK:Error;
}


}
}
