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

#include <seiscomp3/geo/feature.h>
#include <iostream>


using namespace Seiscomp::Geo;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const Category* category, unsigned int rank)
: _category(category)
, _userData(NULL)
, _rank(rank)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const std::string& name, const Category* category,
                       unsigned int rank)
: _name(name)
, _category(category)
, _userData(NULL)
, _rank(rank)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::GeoFeature(const std::string &name, const Category* category,
                       unsigned int rank, const Attributes &attributes)
: _name(name)
, _category(category)
, _userData(NULL)
, _rank(rank)
, _attributes(attributes)
, _closedPolygon(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeature::~GeoFeature() {
	_vertices.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::addVertex(const GeoCoordinate &v, bool newSubFeature) {
	// Mark this vertex as the beginning of a new sub feature. Note: The first
	// vertex is never a subfeature
	if ( newSubFeature && !_vertices.empty() )
		_subFeatures.push_back(_vertices.size());

	// Add the new vertex
	_vertices.push_back(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::updateBoundingBox() {
	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();

	_bbox = GeoBoundingBox();

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		GeoBoundingBox subFeatureBox;
		subFeatureBox.fromPolygon(endIdx-startIdx, &_vertices[startIdx], _closedPolygon);
		_bbox += subFeatureBox;
		startIdx = endIdx;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::invertOrder() {
	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		size_t count = endIdx-startIdx;
		size_t halfCount = count/2;
		for ( size_t j = 0; j < halfCount; ++j )
			std::swap(_vertices[startIdx+j], _vertices[startIdx+count-1-j]);
		startIdx = endIdx;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

typedef std::pair<size_t, double> SubFeatureIndexWithArea;
bool cmpSubFeature(const SubFeatureIndexWithArea &sf1, const SubFeatureIndexWithArea &sf2) {
	return fabs(sf1.second) > fabs(sf2.second);
}

}

void GeoFeature::sort() {
	// Nothing to do without any subfeatures
	if ( _subFeatures.empty() || !_closedPolygon )
		return;

	std::vector<SubFeatureIndexWithArea> subFeatures;

	size_t startIdx = 0, endIdx;
	for ( size_t i = 0; i <= _subFeatures.size(); ++i, startIdx = endIdx ) {
		endIdx = (i == _subFeatures.size() ? _vertices.size() : _subFeatures[i]);
		double A = area(&_vertices[startIdx], endIdx - startIdx);
		subFeatures.push_back(SubFeatureIndexWithArea(i, A));
	}

	std::sort(subFeatures.begin(), subFeatures.end(), cmpSubFeature);

	bool needResort = false;
	size_t i = 1;
	for ( ; i < subFeatures.size(); ++i ) {
		if ( subFeatures[i].first < subFeatures[i-1].first ) {
			needResort = true;
			break;
		}
	}

	if ( !needResort )
		return;

	GeoCoordinates tmpv(_vertices);
	std::vector<size_t> tmpsf(_subFeatures);
	size_t vi = 0;
	size_t sfi = 0;

	for ( size_t i = 0; i < subFeatures.size(); ++i ) {
		size_t sf = subFeatures[i].first;
		size_t startIdx = !sf ? 0 : tmpsf[sf-1];
		size_t endIdx = sf == tmpsf.size() ? tmpv.size() : tmpsf[sf];

		if ( sfi )
			_subFeatures[sfi-1] = vi;

		for ( size_t j = startIdx; j < endIdx; ++j )
			_vertices[vi++] = tmpv[j];

		++sfi;
	}

	assert(vi == _vertices.size());
	assert(sfi == _subFeatures.size()+1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeature::setUserData(void *d) {
	_userData = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


template <typename T>
T sub(T a, T b) {
	return GeoCoordinate::normalizeLon(a-b);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeature::contains(const GeoCoordinate &v) const {
	if ( !closedPolygon() ) return false;

	size_t startIdx = 0, endIdx = 0;
	size_t nSubFeat = _subFeatures.size();
	bool isInside = false;

	for ( size_t i = 0; i <= nSubFeat; ++i ) {
		endIdx = (i == nSubFeat ? _vertices.size() : _subFeatures[i]);
		if ( contains(v, &_vertices[startIdx], endIdx - startIdx) ) {
			isInside = !isInside;
		}

		startIdx = endIdx;
	}
	return isInside;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeature::contains(const GeoCoordinate& v, const GeoCoordinate *polygon,
                          size_t sides) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return false;

	size_t i, j;
	bool oddCrossings = false;

	for ( i = 0, j = sides-1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType relLonLeft = sub(polygon[i].lon, v.lon);
		GeoCoordinate::ValueType relLonRight = sub(polygon[j].lon, v.lon);
		GeoCoordinate::ValueType relWidth = relLonLeft-relLonRight;
		if ( fabs(relWidth) > 180 ) {
			if ( relWidth < -180 )
				relWidth += 360;
			else
				relWidth -= 360;

			relLonLeft = relLonRight+relWidth;
		}

		if ( (relLonLeft > 0) == (relLonRight > 0) ) continue;
		if ( v.lat < (polygon[j].lat-polygon[i].lat) * sub(v.lon, polygon[i].lon) / sub(polygon[j].lon, polygon[i].lon) + polygon[i].lat )
			oddCrossings = !oddCrossings;
	}

	return oddCrossings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeature::contains(const GeoCoordinate& v, const GeoCoordinate *polygon,
                          size_t sides, double &area) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	area = 0;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return false;

	size_t i, j;
	bool oddCrossings = false;

	GeoCoordinate::ValueType ref_lon = polygon[0].lon;

	for ( i = 0, j = sides-1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType relLonLeft = sub(polygon[i].lon, v.lon);
		GeoCoordinate::ValueType relLonRight = sub(polygon[j].lon, v.lon);
		GeoCoordinate::ValueType relWidth = relLonLeft-relLonRight;
		if ( fabs(relWidth) > 180 ) {
			if ( relWidth < -180 )
				relWidth += 360;
			else
				relWidth -= 360;

			relLonLeft = relLonRight+relWidth;
		}

		GeoCoordinate::ValueType l0 = sub(polygon[j].lon, ref_lon);
		GeoCoordinate::ValueType l1 = sub(polygon[i].lon, ref_lon);

		area += l1*polygon[j].lat - l0*polygon[i].lat;

		if ( (relLonLeft > 0) == (relLonRight > 0) ) continue;
		if ( v.lat < (polygon[j].lat-polygon[i].lat) * sub(v.lon, polygon[i].lon) / sub(polygon[j].lon, polygon[i].lon) + polygon[i].lat )
			oddCrossings = !oddCrossings;
	}

	area *= 0.5;
	return oddCrossings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double GeoFeature::area() const {
	if ( !closedPolygon() ) return 0;

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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double GeoFeature::area(const GeoCoordinate *polygon, size_t sides) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return 0;

	size_t i, j;
	double A = 0.0;

	GeoCoordinate::ValueType ref_lon = polygon[0].lon;

	for ( i = 0, j = sides - 1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType l0 = sub(polygon[j].lon, ref_lon);
		GeoCoordinate::ValueType l1 = sub(polygon[i].lon, ref_lon);

		A += l0*polygon[i].lat- l1*polygon[j].lat;
	}

	return A*0.5;
}
