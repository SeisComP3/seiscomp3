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




#ifndef _SEISCOMP_AUTOLOC_LOCATOR_
#define _SEISCOMP_AUTOLOC_LOCATOR_
#include <string>
#include <map>

#include <seiscomp3/seismology/locsat.h>
#include "datamodel.h"

namespace Autoloc {

class Locator : public Seiscomp::LocSAT {
	public:
		Locator();
		~Locator();

		void setStations(const Autoloc::StationDB *stations);

		void setMinimumDepth(double);

	public:
		Origin *relocate(const Origin *origin);


	protected:
		Seiscomp::DataModel::SensorLocation *getSensorLocation(Seiscomp::DataModel::Pick *pick) const;

	private:
		// this is the SC3-level relocate
		Origin *_sc3relocate(const Origin *origin, double fixedDepth=-1);

	private:
		typedef std::map<std::string, Seiscomp::DataModel::SensorLocationPtr> SensorLocationList;

		SensorLocationList _sensorsLocSAT;
		double _minDepth;

		// count the relocate() calls
		int _count;
};



bool determineAzimuthalGaps(const Origin*, double *primary, double *secondary);

}  // namespace Autoloc

#endif
