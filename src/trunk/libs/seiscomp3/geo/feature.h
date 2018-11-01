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

#ifndef __SEISCOMP_GEO_FEATURE_H__
#define __SEISCOMP_GEO_FEATURE_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/geo/coordinate.h>
#include <seiscomp3/geo/boundingbox.h>


namespace Seiscomp {
namespace Geo {


DEFINE_SMARTPOINTER(Category);
DEFINE_SMARTPOINTER(GeoFeature);


struct SC_SYSTEM_CORE_API Category {
	unsigned int id;
	std::string name;
	std::string localName;
	const Category* parent;
	std::string dataDir;

	Category(unsigned int id, std::string name = "",
	         const Category* parent = NULL) :
	    id(id), name(name), parent(parent) {}
};


class SC_SYSTEM_CORE_API GeoFeature : public Core::BaseObject {
	public:
		GeoFeature(const Category* category = NULL, unsigned int rank = 1);
		GeoFeature(const std::string& name, const Category* category,
		           unsigned int rank);
		virtual ~GeoFeature();

		void setName(const std::string &name) { _name = name; }
		const std::string &name() const { return _name; }

		const Category *category() const { return _category; }
		unsigned int rank() const { return _rank; }

		/** Adds a vertex to the GeoFeature and changes the BBox if
		  * applicable. If newSubFeature is set to true */
		void addVertex(const GeoCoordinate &vertex, bool newSubFeature = false);
		void addVertex(float lat, float lon, bool newSubFeature = false) {
			addVertex(GeoCoordinate(lat, lon), newSubFeature);
		}

		bool closedPolygon() const { return _closedPolygon; }
		void setClosedPolygon(bool closed) { _closedPolygon = closed; }

		void updateBoundingBox();

		// Inverts the point order from counter-clockwise to clockwise or
		// vice versa.
		void invertOrder();

		/**
		 * @brief Sets an arbitrary pointer for user data. It is not touched
		 *        and/or utilized by the geo library.
		 */
		void setUserData(void*);
		void *userData() const;

		const std::vector<GeoCoordinate> &vertices() const { return _vertices; }
		const GeoBoundingBox &bbox() const { return _bbox; }
		const std::vector<size_t> &subFeatures() const { return _subFeatures; }

		bool contains(const GeoCoordinate &v) const;

		double area() const;

		static double area(const GeoCoordinate *polygon, size_t sides);

	private:
		static bool contains(const GeoCoordinate& v, const GeoCoordinate *polygon, size_t sides);
		static bool contains(const GeoCoordinate& v, const GeoCoordinate *polygon, size_t sides, double &area);

	private:
		typedef std::vector<GeoCoordinate> GeoCoordinates;

		std::string     _name;
		const Category *_category;
		void           *_userData;
		unsigned int    _rank;

		GeoCoordinates  _vertices;
		bool            _closedPolygon;
		GeoBoundingBox  _bbox;

		/** Index of verticies marking the start of a sub feature.
		 *  E.g. if the GeoFeature defines a main area and a group of
		 *  islands this vector would contain the indices of the start
		 *  point of each island */
		std::vector<size_t> _subFeatures;
};


inline void *GeoFeature::userData() const {
	return _userData;
}


} // of ns Geo
} // of ns Seiscomp


#endif // __SEISCOMP_GEO_GEOFEATURE_H__
