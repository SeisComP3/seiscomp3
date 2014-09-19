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


#ifndef __SEISCOMP_MATH_GEO_POLYGON_H__
#define __SEISCOMP_MATH_GEO_POLYGON_H__

#include <string>
#include <vector>
#include <utility>
#include <seiscomp3/math/coord.h>


namespace Seiscomp
{

namespace Math
{

namespace Geo
{

template <typename T>
class Polygon : public std::vector< Coord<T> > {
	public:
		Polygon();
		~Polygon();

		/**
		 * Returns whether a location lies inside the polygon
		 * or not.
		 * @param p The location
		 * @return True, if the location lies inside, else false
		 */
		bool operator&(const Coord<T>& c);

		void addVertex(double lat, double lon);
		void addCoord(const Coord<T>& c);

		bool pointInPolygon(const T& lat, const T& lon) const;
		bool pointInPolygon(const Coord<T>& c) const;

		size_t vertexCount() const;

		const Coord<T>& vertex(int i) const;
		Coord<T>& vertex(int i);

		void print() const;

};


typedef Polygon<float> PolygonF;
typedef Polygon<double> PolygonD;


} // of ns  Geo
} // of ns  Math
} // of ns Seiscomp


template <typename T>
std::ostream& operator<<(std::ostream& os, const Seiscomp::Math::Geo::Polygon<T>& poly);


#endif
