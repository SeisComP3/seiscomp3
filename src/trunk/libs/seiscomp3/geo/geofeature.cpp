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

#include <seiscomp3/geo/geofeature.h>
#include <iostream>

using namespace Seiscomp::Geo;


GeoFeature::GeoFeature(const Category* category, unsigned int rank)
	: _category(category), _rank(rank), _closedPolygon(false) {}


GeoFeature::GeoFeature(const std::string& name, const Category* category,
	unsigned int rank) : _name(name), _category(category), _rank(rank),
	_closedPolygon(false) {}


GeoFeature::~GeoFeature() {
	_vertices.clear();
}


void GeoFeature::addVertex(const Vertex &v, bool newSubFeature) {
	// Initialize/update bounding box if necessary.
	if ( _vertices.empty() ) {
		_bbox.latMin = _bbox.latMax = v.lat;
		_bbox.lonMin = _bbox.lonMax = v.lon;
	}
	else {
		if ( v.lat < _bbox.latMin ) _bbox.latMin = v.lat;
		if ( v.lat > _bbox.latMax ) _bbox.latMax = v.lat;
		if ( v.lon < _bbox.lonMin ) _bbox.lonMin = v.lon;
		if ( v.lon > _bbox.lonMax ) _bbox.lonMax = v.lon;
	}

	// Mark this vertex as the beginning of a new sub feature. Note: The first
	// vertex is never a subfeature
	if ( newSubFeature && !_vertices.empty() )
		_subFeatures.push_back(_vertices.size());

	// Add the new vertex
	_vertices.push_back(v);
}


namespace {


template <typename T>
T sub(T a, T b) {
	T s = a - b;
	if ( s < -180 )
		s += 360;
	else if ( s > 180 )
		s -= 360;
	return s;
}


}


bool GeoFeature::contains(const Vertex &v) const {
	if ( !closedPolygon() ) return false;

	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();
	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		if ( contains(v, &_vertices[startIdx], endIdx - startIdx) )
			return true;
		startIdx = endIdx;
	}
	return false;
}


bool GeoFeature::contains(const Vertex& v, const Vertex *polygon,
                          size_t sides) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return false;

	int l = sides - 2;
	int i = sides - 1;
	bool oddCrossings = false;

//#define epsilon 1E-10
#define epsilon 0

	for ( size_t n = 0; n < sides; l = i, i = n++ ) {
		Vertex::ValueType segWidth = sub(polygon[i].lon, polygon[l].lon);
		Vertex::ValueType relLonLeft = sub(v.lon, polygon[l].lon);
		Vertex::ValueType relLonRight = sub(v.lon, polygon[i].lon);

		bool touch = relLonLeft * sub(v.lon, polygon[n].lon) > 0;

		// Skip segments where v.lon falls outside the segments horizontal
		// boundings
		if ( touch ) {
			if ( segWidth >= 0 ) {
				if ( relLonLeft < -epsilon || relLonRight > epsilon ) continue;
			}
			else {
				if ( relLonLeft > epsilon || relLonRight < -epsilon ) continue;
			}
		}
		else {
			if ( segWidth >= 0 ) {
				if ( relLonLeft < -epsilon || relLonRight >= epsilon ) continue;
			}
			else {
				if ( relLonLeft > epsilon || relLonRight <= -epsilon ) continue;
			}
		}

		Vertex::ValueType y0;

		// y0 = latitude of crossing
		y0 = relLonLeft * (polygon[i].lat - polygon[l].lat) /
		     segWidth + polygon[l].lat;

		// North crossing
		if ( y0 > v.lat )
			oddCrossings = !oddCrossings;
	}

	return oddCrossings;
}


double GeoFeature::area() const {
	if ( !closedPolygon() ) return false;

	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();
	double A = 0.0;

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		A += area(&_vertices[startIdx], endIdx - startIdx);
		startIdx = endIdx;
	}

	return A;
}


double GeoFeature::area(const Vertex *polygon, size_t sides) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return 0;

	int j = sides - 1;
	double A = 0.0;

	Vertex::ValueType ref_lon = polygon[0].lon;

	for ( size_t i = 0; i < sides; j = i++ ) {
		double l0 = sub(polygon[j].lon, ref_lon);
		double l1 = sub(polygon[i].lon, ref_lon);

		A += l1*polygon[j].lat - l0*polygon[i].lat;
	}

	return A*0.5;
}
