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



#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/seismology/regions/ferdata.h>
#include <seiscomp3/seismology/regions/polygon.h>

#include <seiscomp3/system/environment.h>

using namespace Seiscomp;


namespace {


Geo::PolyRegions regions;


}


Regions::Regions() {
}


void Regions::load() {
	if ( !regions.read(Environment::Instance()->configDir() + "/fep") )
		regions.read(Environment::Instance()->shareDir() + "/fep");
}


Geo::PolyRegions &Regions::polyRegions() {
	return regions;
}


std::string Regions::getRegionName(double lat, double lon) {
	while ( lon < -180 ) lon += 360;
	while ( lon > 180 ) lon -= 360;

	std::string name = getRegionalName(lat, lon);
	return name.empty()?getFeGeoRegionName(lat, lon):name;
}


std::string Regions::getFeGeoRegionName(double lat, double lon) {
	std::string name;

	int _lat = int(lat);
	int _lon = int(lon);

	if (lat >= 0.0) _lat += 1;
	if (lon >= 0.0) _lon += 1;

	if (lat >= -90 && lat <= +90 && lon >= -180 && lon <= +180)
		name = feGeoRegionsNames[feGeoRegionsArray[_lat + 90][_lon + 180] - 1];
	else
		name = "unknown Region";

	return name;
}


std::string Regions::getRegionalName(double lat, double lon) {
	return regions.findRegionName(lat, lon);
}
