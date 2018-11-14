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



#ifndef __SEISCOMP_SEISMOLOGY_REGIONS_POLY_H__
#define __SEISCOMP_SEISMOLOGY_REGIONS_POLY_H__

#include <seiscomp3/core.h>
#include <seiscomp3/geo/feature.h>
#include <vector>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API PolyRegions {
	public:
		PolyRegions();
		PolyRegions(const std::string& location);
		~PolyRegions();

		void print();
		void info();

		GeoFeature *findRegion(double lat, double lon) const;
		std::string findRegionName(double lat, double lon) const;

		size_t regionCount() const;
		void addRegion(GeoFeature* r);
		GeoFeature *region(int i) const;

		size_t read(const std::string& location);

		const std::string& dataDir() const { return _dataDir; }

	private:
		bool readFepBoundaries(const std::string& filename);

	private:
		std::vector<GeoFeature*> _regions;
		std::string _dataDir;
};


} // of ns Regions
} // of ns Seiscomp


#endif
