/***************************************************************************
 *   Copyright (C) by gempa GmbH and GFZ Potsdam                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker, gempa GmbH, jabe@gempa.de                         *
 ***************************************************************************/


#include <QPainter>
#include <iostream>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::Map::Projection, SC_GUI_API);

namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Projection::Projection() {
	setZoom(1.0f);
	_background = qRgb(0,0,0);
	_pixelPerDegreeFact = 90.0f;
	_gridLines = 4.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Projection::~Projection() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setBackgroundColor(const QColor &c) {
	_background = c.rgba();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setZoom(qreal zoom) {
	_radius = zoom;
	_visibleRadius = _radius;

	_scale = _screenRadius * _radius;
	_oneOverScale = 1.0f / _scale;

	_halfMapWidth = _scale * 2;
	_mapWidth = _scale * 4;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
qreal Projection::zoom() const {
	return _radius;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
qreal Projection::visibleZoom() const {
	return _visibleRadius;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
qreal Projection::pixelPerDegree() const {
	return _scale / _pixelPerDegreeFact;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setView(const QPointF &geoCoords, qreal zoom) {
	setZoom(zoom);
	centerOn(geoCoords);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF Projection::center() const {
	return QPointF(_center.x()*180.0f, _center.y()*90.0f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF Projection::visibleCenter() const {
	return QPointF(_visibleCenter.x()*180.0f, _visibleCenter.y()*90.0f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::updateBoundingBox() {
	_mapBoundingBox.reset();
	if ( _height <= 0 || _width <= 0 ) return;

	int top = 1, left = 1, bottom = _height-2, right = _width-2;

	if ( isRectangular() ) {
		QPointF topLeft, bottomRight;
		if ( unproject(topLeft, QPoint(left, top))
		  && unproject(bottomRight, QPoint(right, bottom)) ) {
			_mapBoundingBox.south = bottomRight.y();
			_mapBoundingBox.west = topLeft.x();
			_mapBoundingBox.north = topLeft.y();
			_mapBoundingBox.east = bottomRight.x();

			if ( _mapBoundingBox.west == _mapBoundingBox.east ) {
				_mapBoundingBox.west = -180;
				_mapBoundingBox.east = 180;
			}
		}
		else {
			std::cerr << "Failed to update map bounding box, reimplement this method!" << std::endl;
		}
	}
	else {
		bool hasVCoords = false;
		bool hasHCoords = false;

#define UPDATE_VCOORDS \
	if ( !hasVCoords )\
		_mapBoundingBox.north = _mapBoundingBox.south = gc.y();\
	else {\
		if ( gc.y() > _mapBoundingBox.north )\
			_mapBoundingBox.north = gc.y();\
		else if ( gc.y() < _mapBoundingBox.south )\
			_mapBoundingBox.south = gc.y();\
	}\
	hasVCoords = true

#define UPDATE_HCOORDS \
	if ( !hasHCoords )\
		_mapBoundingBox.west = _mapBoundingBox.east = gc.x();\
	else {\
		if ( gc.x() > _mapBoundingBox.east )\
			_mapBoundingBox.east = gc.x();\
		else if ( gc.x() < _mapBoundingBox.west )\
			_mapBoundingBox.west = gc.x();\
	}\
	hasHCoords = true

		QPointF gc;

		for ( int x = 0; x < _width; x += 10 ) {
			if ( unproject(gc, QPoint(x, top)) ) {
				UPDATE_VCOORDS;
			}

			if ( unproject(gc, QPoint(x, bottom)) ) {
				UPDATE_VCOORDS;
			}
		}

		if ( unproject(gc, QPoint(right, top)) ) {
			UPDATE_VCOORDS;
			UPDATE_HCOORDS;
		}

		if ( unproject(gc, QPoint(right, bottom)) ) {
			UPDATE_VCOORDS;
			UPDATE_HCOORDS;
		}

		for ( int y = 0; y < _height; y += 10 ) {
			if ( unproject(gc, QPoint(left, y)) ) {
				UPDATE_HCOORDS;
			}

			if ( unproject(gc, QPoint(right, y)) ) {
				UPDATE_HCOORDS;
			}
		}

		QPointF center;
		unproject(center, QPoint(_width/2, _height/2));

		if ( !hasVCoords ) {
			_mapBoundingBox.south = Geo::GeoCoordinate::normalizeLat(center.y() - 90.0);
			_mapBoundingBox.north = Geo::GeoCoordinate::normalizeLat(center.y() + 90.0);
		}

		if ( !hasHCoords ) {
			_mapBoundingBox.west = Geo::GeoCoordinate::normalizeLon(center.x() - (fabs(center.y())+90.0));
			_mapBoundingBox.east = Geo::GeoCoordinate::normalizeLon(center.x() + (fabs(center.y())+90.0));
		}

		QPoint tmp;

		if ( project(tmp, QPointF(0, 90)) ) {
			if ( tmp.y() > 0 && tmp.y() < _height ) {
				// North pole visible
				_mapBoundingBox.north = 90;
			}
		}
		else if ( project(tmp, QPointF(0, -90)) ) {
			if ( tmp.y() > 0 && tmp.y() < _height ) {
				// South pole visible
				_mapBoundingBox.south = -90;
			}
		}

		if ( _mapBoundingBox.west <= _mapBoundingBox.east ) {
			if ( center.x() < _mapBoundingBox.west || center.x() > _mapBoundingBox.east )
				std::swap(_mapBoundingBox.east, _mapBoundingBox.west);
		}
		else {
			if ( center.x() < _mapBoundingBox.west && center.x() > _mapBoundingBox.east )
				std::swap(_mapBoundingBox.east, _mapBoundingBox.west);
		}

	}

	//std::cerr << "BB " << Geo::format(_mapBoundingBox) << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::displayRect(const QRectF& rect) {
	QPointF center = rect.center();

	double zoomLevelH = 360.0f / rect.width();
	double zoomLevelW = 360.0f / rect.height();

	setView(center, std::min(zoomLevelH, zoomLevelW));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::drawImage(QImage &, const QRectF &, const QImage &, bool, CompositionMode) {
	static bool firstCall = true;
	if ( firstCall ) {
		std::cerr << "Drawing a georeferenced image is not yet supported" << std::endl;
		firstCall = false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Projection::lineSteps(const QPointF &p0, const QPointF &p1) {
	return 20;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::drawLine(QPainter &p, const QPointF &from, const QPointF &to) {
	QPoint x0, x1;
	bool x0Visible, x1Visible;

	x0Visible = project(x0, from);
	x1Visible = project(x1, to);

	if ( x0Visible && x1Visible ) {
		// Simple clipping
		if ( (x0.y() >= 0 || x1.y() >= 0) &&
		     (x0.y() < _height || x1.y() < _height) &&
		     (x0.x() >= 0 || x1.x() >= 0) &&
		     (x0.x() < _width || x1.x() < _width) ) {
			p.drawLine(x0, x1);
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::moveTo(const QPointF &p) {
	_cursorVisible = project(_cursor, p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::lineTo(QPainter &p, const QPointF &to) {
	QPoint pp;
	bool visible = project(pp, to);
	bool not_clipped = false;

	if ( _cursorVisible && visible ) {
		// Simple clipping
		if ( (_cursor.y() >= 0 || pp.y() >= 0) &&
		     (_cursor.y() < _height || pp.y() < _height) &&
		     (_cursor.x() >= 0 || pp.x() >= 0) &&
		     (_cursor.x() < _width || pp.x() < _width) ) {
			p.drawLine(_cursor, pp);
			not_clipped = true;
		}
		else
			not_clipped = false;
	}
	else
		not_clipped = false;

	_cursorVisible = visible;
	_cursor = pp;
	return not_clipped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::drawLatCircle(QPainter &p, qreal lon) {
	int steps = 45;
	bool visible = false;

	moveTo(QPointF(lon, -90.0));
	for ( int i = 0; i <= steps; ++i ) {
		bool res = lineTo(p, QPointF(lon, i * 180.0 / steps - 90.0));
		visible = visible || res;
	}

	return visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::drawLonCircle(QPainter &p, qreal lat) {
	int steps = 45;
	bool visible = false;

	moveTo(QPointF(0.0, lat));
	for ( int i = 0; i <= steps; ++i ) {
		bool res = lineTo(p, QPointF(i * 360.0 / steps, lat));
		visible = visible || res;
	}

	return visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setSize(int width, int height) {
	_width = width;
	_height = height;

	_halfWidth = _width >> 1;
	_halfHeight = _height >> 1;

	_screenRadius = qreal(std::min(_width, _height)) * 0.5;

	_visibleCenter = _center;

	setZoom(_radius);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::draw(QImage &img, bool filter, TextureCache *cache) {
	if ( cache ) cache->beginPaint();

	setSize(img.width(), img.height());

	render(img, filter, cache);

	/*
	Core::Time now = Core::Time::GMT();
	Core::TimeSpan ts = now - cache->startTime();

	std::cerr << "render time = "
	          << (double)ts
	          << ", fps = " << (1.0 / (double)ts)
	          << std::endl;
	*/

	updateBoundingBox();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setVisibleRadius(qreal r) {
	_visibleRadius = r;
	_scale = _screenRadius * _visibleRadius;
	_oneOverScale = 1.0f / _scale;

	_halfMapWidth = _scale * 2;
	_mapWidth = _scale * 4;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF Projection::gridDistance() const {
	static const qreal gridIntervals[]  = {
		0.0001, 0.0002, 0.0005,
		0.001,  0.002,  0.005,
		0.01,   0.02,   0.05,
		0.1,    0.2,    0.5,
		1, 2, 5, 10, 15, 20, 30, 45, 60, 90
	};

	const qreal *dist;

	// Grid distance is equal for latitudes and longitudes and targets
	// _gridLines visible lines for the largest screen dimension.
	dist = std::upper_bound(
	           gridIntervals,
	           gridIntervals + sizeof(gridIntervals)/sizeof(qreal) - 1,
	           qreal(std::max(_width, _height)) / (pixelPerDegree() * _gridLines));

	return QPointF(*dist, *dist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::isClipped(const Geo::GeoBoundingBox &bbox) const {
	return !_mapBoundingBox.intersects(bbox);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::isClipped(const QPointF &bboxLR, const QPointF &bboxUL) const {
	return isClipped(Geo::GeoBoundingBox(bboxLR.y(), bboxUL.x(),
	                                     bboxUL.y(), bboxLR.x()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Projection::project(QPainterPath &screenPath, size_t n,
                         const Geo::GeoCoordinate *poly, bool closed,
                         uint minPixelDist, ClipHint) const {
	if ( n == 0 || !poly ) return false;

	float minDist = ((float)minPixelDist)/pixelPerDegree();

	QPointF v;
	QPoint p;
	size_t startIdx = 0;
	while ( startIdx < n ) {
		v.setX(poly[startIdx].lon); v.setY(poly[startIdx].lat);
		++startIdx;
		if ( project(p, v) ) {
			screenPath.moveTo(p);
			break;
		}
	}

	if ( minDist == 0 ) {
		for ( size_t i = startIdx; i < n; ++i ) {
			v.setX(poly[i].lon); v.setY(poly[i].lat);
			if ( project(p, v) ) screenPath.lineTo(p);
		}
	}
	else {
		for ( size_t i = startIdx; i < n; ++i ) {
			if ( std::abs(poly[i].lon - v.x()) > minDist ||
			     std::abs(poly[i].lat - v.y()) > minDist ) {
#ifdef INTERPOLATE_FEATURE
				Math::Geo::PositionInterpolator ip(v.y(), v.x(),
				                                   poly[i].lat, poly[i].lon, 10);
				++ip;

				if ( project(p, QPointF(ip.longitude(),ip.latitude())) ) screenPath.lineTo(p);

				while ( !ip.end() ) {
					if ( project(p, QPointF(ip.longitude(),ip.latitude())) ) screenPath.lineTo(p);
					++ip;
				}
#endif
				v.setX(poly[i].lon); v.setY(poly[i].lat);
#ifndef INTERPOLATE_FEATURE
				if ( project(p, v) ) screenPath.lineTo(p);
#endif
			}
		}
	}

	if ( closed )
		screenPath.closeSubpath();

	return !screenPath.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Projection::setGridLines(qreal numLines) {
	_gridLines = numLines < 1.0 ? 1.0 : numLines;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // of ns Map
} // of ns Gui
} // of ns Seiscomp
