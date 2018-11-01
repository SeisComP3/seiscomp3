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


#ifndef __SEISCOMP_GEO_GEOCOORDINATE_H__
#define __SEISCOMP_GEO_GEOCOORDINATE_H__


#include <seiscomp3/math/geo.h>
#include <ostream>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API GeoCoordinate {
	public:
		typedef float ValueType;

	public:
		GeoCoordinate();
		GeoCoordinate(ValueType lat_, ValueType lon_);

	public:
		void set(ValueType lat, ValueType lon);

		ValueType latitude() const;
		ValueType longitude() const;

		bool operator==(const GeoCoordinate &other) const;
		bool operator!=(const GeoCoordinate &other) const;

		GeoCoordinate &normalize();

		static ValueType width(ValueType lon0, ValueType lon1);
		static ValueType normalizeLat(ValueType lat);
		static ValueType normalizeLon(ValueType lon);
		static void normalizeLatLon(ValueType &lat, ValueType &lon);

		static ValueType distanceLon(ValueType lon0, ValueType lon1);

	public:
		ValueType lat;
		ValueType lon;
};


// For backwards compatibility, define Vertex as GeoCoordinate
typedef GeoCoordinate Vertex;


#include <seiscomp3/geo/coordinate.ipp>


struct formatted_lat {
	formatted_lat(double lat) : v(lat) {}
	double v;
};

struct formatted_lon {
	formatted_lon(double lon) : v(lon) {}
	double v;
};

template <typename T>
struct OStreamFormat {
	OStreamFormat(const T &r) : ref(r) {}
	const T &ref;
};


template <typename T>
OStreamFormat<T> format(const T &ref) { return OStreamFormat<T>(ref); }


std::ostream &operator<<(std::ostream &os, const GeoCoordinate &);
std::ostream &operator<<(std::ostream &os, const formatted_lat &);
std::ostream &operator<<(std::ostream &os, const formatted_lon &);

template <typename T>
std::ostream &operator<<(std::ostream &os, const OStreamFormat<T> &f) {
	os << f.ref;
	return os;
}

template <>
std::ostream &operator<<(std::ostream &os, const OStreamFormat<GeoCoordinate> &);


}
}


#endif
