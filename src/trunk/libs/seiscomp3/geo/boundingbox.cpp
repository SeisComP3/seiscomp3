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


#include <seiscomp3/geo/boundingbox.h>
#include <iostream>


namespace Seiscomp {
namespace Geo {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoBoundingBox GeoBoundingBox::Empty;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoBoundingBox::GeoBoundingBox()
: north(0)
, south(0)
, east(0)
, west(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoBoundingBox::GeoBoundingBox(ValueType south, ValueType west,
                               ValueType north, ValueType east)
: north(north)
, south(south)
, east(east)
, west(west) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoBoundingBox::operator==(const GeoBoundingBox &other) const {
	return north == other.north
	    && south == other.south
	    && east == other.east
	    && west == other.west;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoBoundingBox &GeoBoundingBox::normalize() {
	GeoCoordinate::normalizeLatLon(south, west);
	GeoCoordinate::normalizeLatLon(north, east);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoCoordinate GeoBoundingBox::center() const {
	GeoCoordinate result;

	if( isEmpty() )
		return result;

	result.lat = (north + south) * 0.5;
	result.lon = GeoCoordinate::normalizeLon(west + width()*0.5);

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoBoundingBox::contains(const GeoCoordinate &v) const {
	if ( (v.lat < south) || (v.lat > north) )
		return false;

	if ( west <= east )
		return v.lon >= west && v.lon <= east;
	else
		return v.lon >= west || v.lon <= east;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoBoundingBox::contains(const GeoBoundingBox &other) const {
	if ( (other.south < south) || (other.north > north) )
		return false;

	ValueType w = width();
	ValueType rwest = other.west-west;
	ValueType reast = other.east-west;

	if ( rwest < 0 ) rwest += 360;
	if ( reast < 0 ) reast += 360;

	return rwest <= w && reast <= w && rwest <= reast;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoBoundingBox::Relation GeoBoundingBox::relation(const GeoBoundingBox &other) const {
	if ( contains(other) )
		return Contains;

	if ( intersects(other) )
		return Intersects;

	return Disjunct;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoBoundingBox::merge(const GeoBoundingBox &other) {
	if ( isEmpty() ) {
		*this = other;
		return;
	}

	if ( other.isEmpty() )
		return;

	north = std::max(north, other.north);
	south = std::min(south, other.south);

	ValueType otherWest = GeoCoordinate::distanceLon(west, other.west);
	ValueType otherEast = GeoCoordinate::distanceLon(east, other.east);

	ValueType w1 = width();

	if ( fabs(otherWest) < fabs(otherEast) ) {
		if ( otherWest < 0 ) {
			west += otherWest;
			w1 -= otherWest;
			otherWest = 0;
			otherEast = other.width();
		}
		else
			otherEast = otherWest + other.width();

		if ( otherEast > w1 ) {
			east = west + otherEast;
			w1 = otherEast;
		}
	}
	else {
		if ( otherEast > 0 ) {
			east += otherEast;
			w1 += otherEast;
			otherEast = 0;
			otherWest = -other.width();
		}
		else
			otherWest = otherEast - other.width();

		if ( otherWest < -w1 ) {
			west = east + otherWest;
			w1 = -otherWest;
		}
	}

	if ( w1 >= 360 ) {
		west = -180;
		east = 180;
	}
	else {
		west = GeoCoordinate::normalizeLon(west);
		east = GeoCoordinate::normalizeLon(east);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoBoundingBox::intersects(const GeoBoundingBox &other) const {
	if ( isEmpty() || other.isEmpty() )
		return false;

	// Simple latitude check
	if ( other.north <= south ) return false;
	if ( other.south >= north ) return false;

	bool cdlThis = west > east;
	bool cdlOther = other.west > other.east;

	// One crosses the date line, the other does not
	if ( cdlThis ^ cdlOther ) {
		if ( other.east <= west && other.west >= east )
			return false;
	}
	else {
		// Either both cross or don't
		if ( !cdlThis ) {
			// Neither crosses
			if ( other.east <= west ) return false;
			if ( other.west >= east ) return false;
		}
		// else is a trivial case: either crosses so they intersect
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoBoundingBox::fromPolygon(size_t n, const GeoCoordinate *coords,
                                 bool isClosed) {
	if ( !n ) {
		*this = Empty;
		return;
	}

	south = north = coords[0].lat;
	west = east = coords[0].lon;

	ValueType w = 0; // The current width
	ValueType lastLon = GeoCoordinate::normalizeLon(coords[0].lon);
	bool fullLongitude = false;

	size_t c = n;
	if ( isClosed ) ++c;
	size_t i = 0;

	while ( --c ) {
		++i;
		if ( i >= n ) i -= n;

		// Easy case
		if ( coords[i].lat < south ) south = coords[i].lat;
		else if ( coords[i].lat > north ) north = coords[i].lat;

		if ( !fullLongitude ) {
			ValueType currentLon = GeoCoordinate::normalizeLon(coords[i].lon);
			ValueType polyDirection = GeoCoordinate::distanceLon(lastLon, currentLon);
			if ( polyDirection < 0 ) {
				// Extend west
				ValueType distanceToWest = GeoCoordinate::distanceLon(west, currentLon);
				if ( distanceToWest < 0 ) {
					west += distanceToWest;
					if ( west < -180 ) west += 360;
					w -= distanceToWest;
				}
			}
			else {
				// Extend east
				ValueType distanceToEast = GeoCoordinate::distanceLon(east, currentLon);
				if ( distanceToEast > 0 ) {
					east += distanceToEast;
					if ( east > 180 ) east -= 360;
					w += distanceToEast;
				}
			}

			lastLon = currentLon;

			if ( w >= 360 ) {
				west = -180;
				east = 180;
				fullLongitude = true;
			}
		}
	}

	if ( fullLongitude ) {
		if ( north < 0 )
			south = -90;
		else
			north = 90;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::GeoBoundingBox &box) {
	os << box.south << " / " << box.west << " ; " << box.north << " / " << box.east;
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::OStreamFormat<Seiscomp::Geo::GeoBoundingBox> &f) {
	os << Seiscomp::Geo::formatted_lat(f.ref.south)
	   << " / "
	   << Seiscomp::Geo::formatted_lon(f.ref.west)
	   << " ; "
	   << Seiscomp::Geo::formatted_lat(f.ref.north)
	   << " / "
	   << Seiscomp::Geo::formatted_lon(f.ref.east);
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
