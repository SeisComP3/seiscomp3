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


#define SEISCOMP_COMPONENT MN

#include <seiscomp3/logging/log.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/math/geo.h>

#include <boost/thread/mutex.hpp>

#include "./regions.h"

//#define TEST_WITHOUT_REGIONS


namespace Seiscomp {
namespace Magnitudes {
namespace MN {


namespace {


#ifndef TEST_WITHOUT_REGIONS
bool validRegionInitialized = false;
Seiscomp::Geo::GeoFeatureSet validRegion;
boost::mutex regionMutex;
#endif


}


bool initialize(const Config::Config *config) {
#ifndef TEST_WITHOUT_REGIONS
	boost::mutex::scoped_lock l(regionMutex);

	if ( !validRegionInitialized ) {
		// Don't try again
		validRegionInitialized = true;

		std::string filename;

		try {
			filename = config->getString("magnitudes.MN.region");
		}
		catch ( ... ) {
			filename = "@DATADIR@/magnitudes/MN/MN.bna";
		}

		filename = Seiscomp::Environment::Instance()->absolutePath(filename);

		if ( !validRegion.readBNAFile(filename, NULL) ) {
			SEISCOMP_ERROR("Failed to read/parse MN region file: %s",
			               filename.c_str());
			return false;
		}
	}
	else if ( validRegion.features().empty() ) {
		// No region defined, nothing to do
		SEISCOMP_ERROR("No regions defined in amplitudes.MN.region file");
		return false;
	}

#endif
	return true;
}


bool isInsideRegion(double lat0, double lon0,
                    double lat1, double lon1) {
#ifndef TEST_WITHOUT_REGIONS
	double dist, az, baz;

	Math::Geo::delazi_wgs84(lat0, lon0, lat1, lon1, &dist, &az, &baz);

	// Convert to km
	dist = Math::Geo::deg2km(dist);

	// Check path each 10 km
	int steps = dist / 10;

	for ( int i = 1; i < steps; ++i ) {
		Math::Geo::delandaz2coord(Math::Geo::km2deg(dist*i/steps), az, lat0, lon0,
		                          &lat1, &lon1);
		if ( !isInsideRegion(lat1, lon1) )
			return false;
	}
#endif
	return true;
}


bool isInsideRegion(double lat, double lon) {
#ifndef TEST_WITHOUT_REGIONS
	boost::mutex::scoped_lock l(regionMutex);
	size_t numFeatures = validRegion.features().size();
	for ( size_t i = 0; i < numFeatures; ++i ) {
		Seiscomp::Geo::GeoFeature *feature = validRegion.features()[i];
		if ( feature->contains(Seiscomp::Geo::Vertex(lat, lon)) )
			return true;
	}

	return false;
#else
	return true;
#endif
}


}
}
}
