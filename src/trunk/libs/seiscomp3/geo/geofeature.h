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

#ifndef __SEISCOMP_GEO_GEOFEATURE_H__
#define __SEISCOMP_GEO_GEOFEATURE_H__

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/math/geo.h>
#include <boost/concept_check.hpp>


namespace Seiscomp
{
namespace Geo
{

DEFINE_SMARTPOINTER(Category);
DEFINE_SMARTPOINTER(GeoFeature);

typedef Math::Geo::CoordF Vertex;

struct SC_SYSTEM_CORE_API BBox {
	float lonMin;
	float latMin;
	float lonMax;
	float latMax;
	bool dateLineCrossed;

	BBox() : lonMin(0), latMin(0), lonMax(0), latMax(0), dateLineCrossed(false) {}
};


struct SC_SYSTEM_CORE_API Category {
	unsigned int id;
	std::string name;
	const Category* parent;

	Category(unsigned int id, std::string name = "",
	         const Category* parent = NULL) : id(id), name(name),
	         parent(parent) {}
};


class SC_SYSTEM_CORE_API GeoFeature : public Core::BaseObject {

	public:
		GeoFeature(const Category* category = NULL, unsigned int rank = 1);
		GeoFeature(const std::string& name, const Category* category,
		           unsigned int rank);
		virtual ~GeoFeature();

		void setName(const std::string &name) { _name = name; }
		const std::string &name() const { return _name; }

		const Category* category() const { return _category; }
		unsigned int rank() const { return _rank; }

		/** Adds a vertex to the GeoFeature and changes the BBox if
		  * applicable. If newSubFeature is set to true */
		void addVertex(const Vertex &vertex, bool newSubFeature = false);
		void addVertex(float lat, float lon, bool newSubFeature = false) {
			addVertex(Vertex(lat, lon), newSubFeature);
		}

		bool closedPolygon() const { return _closedPolygon; }
		void setClosedPolygon(bool closed) { _closedPolygon = closed; }
		const std::vector<Vertex> &vertices() const { return _vertices; }
		const BBox &bbox() const { return _bbox; }
		const std::vector<size_t> &subFeatures() const { return _subFeatures; }

		bool contains(const Vertex &v) const;

		double area() const;

		static double area(const Vertex *polygon, size_t sides);

	private:
		static bool contains(const Vertex& v, const Vertex *polygon, size_t sides);

	private:
		std::string _name;
		const Category* _category;
		unsigned int _rank;

		std::vector<Vertex> _vertices;
		bool _closedPolygon;
		BBox _bbox;

		/** Index of verticies marking the start of a sub feature.
		 *  E.g. if the GeoFeature defines a main area and a group of
		 *  islands this vector would contain the indices of the start
		 *  point of each island */
		std::vector<size_t> _subFeatures;
};


} // of ns Geo
} // of ns Seiscomp

#endif // __SEISCOMP_GEO_GEOFEATURE_H__
