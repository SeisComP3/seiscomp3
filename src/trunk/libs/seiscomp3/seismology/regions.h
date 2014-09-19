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



#ifndef __SEISCOMP_SEISMOLOGY_REGIONS_H__
#define __SEISCOMP_SEISMOLOGY_REGIONS_H__

#include <seiscomp3/core.h>
#include <seiscomp3/seismology/regions/polygon.h>
#include <string>


namespace Seiscomp {

class SC_SYSTEM_CORE_API Regions {

	public:
		Regions();

		static void load();
		static std::string getRegionName(double lat, double lon);
		static Seiscomp::Geo::PolyRegions &polyRegions();

	private:
		static std::string getFeGeoRegionName(double lat, double lon);
		static std::string getRegionalName(double lat, double lon);

};


} // of ns Seiscomp

#endif
