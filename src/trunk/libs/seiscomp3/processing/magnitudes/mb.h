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



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MB_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MB_H__

#include <seiscomp3/processing/magnitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mb : public MagnitudeProcessor {
	DECLARE_SC_CLASS(MagnitudeProcessor_mb);

	public:
		MagnitudeProcessor_mb();

		Status computeMagnitude(
			double amplitude, // in micrometers per second
			double period,      // in seconds
			double delta,     // in degrees
			double depth,     // in kilometers
			const DataModel::Origin *hypocenter,
			const DataModel::SensorLocation *receiver,
			double &value);
};


}

}


#endif
