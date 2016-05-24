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




#include <QPainter>
#include <iostream>

#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::Map::Projection, SC_GUI_API);

namespace Seiscomp {
namespace Gui {
namespace Map {


Projection::Projection() {
	setZoom(1.0f);
	_background = qRgb(0,0,0);
	_pixelPerDegreeFact = 90.0f;
}


Projection::~Projection() {
}


void Projection::setBackgroundColor(const QColor &c) {
	_background = c.rgba();
}


void Projection::setZoom(qreal zoom) {
	_radius = zoom;
	_visibleRadius = _radius;

	_scale = _screenRadius * _radius;
	_oneOverScale = 1.0f / _scale;

	_halfMapWidth = _scale * 2;
	_mapWidth = _scale * 4;
}


qreal Projection::zoom() const {
	return _radius;
}


qreal Projection::visibleZoom() const {
	return _visibleRadius;
}


qreal Projection::pixelPerDegree() const {
	return _scale / _pixelPerDegreeFact;
}


void Projection::setView(const QPointF &geoCoords, qreal zoom) {
	setZoom(zoom);
	centerOn(geoCoords);
}


QPointF Projection::center() const {
	return QPointF(_center.x()*180.0f, _center.y()*90.0f);
}


QPointF Projection::visibleCenter() const {
	return QPointF(_visibleCenter.x()*180.0f, _visibleCenter.y()*90.0f);
}


void Projection::displayRect(const QRectF& rect) {
	QPointF center = rect.center();

	double zoomLevelH = 360.0f / rect.width();
	double zoomLevelW = 360.0f / rect.height();

	setView(center, std::min(zoomLevelH, zoomLevelW));
}


void Projection::drawImage(QImage &, const QRectF &, const QImage &, bool) {
	static bool firstCall = true;
	if ( firstCall ) {
		std::cerr << "Drawing a georeferenced image is not yet supported" << std::endl;
		firstCall = false;
	}
}


int Projection::lineSteps(const QPointF &p0, const QPointF &p1) {
	return 20;
}


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


void Projection::moveTo(const QPointF &p) {
	_cursorVisible = project(_cursor, p);
}


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


void Projection::setSize(int width, int height) {
	_width = width;
	_height = height;

	_halfWidth = _width >> 1;
	_halfHeight = _height >> 1;

	_screenRadius = qreal(std::min(_width, _height)) * 0.5;

	_visibleCenter = _center;

	setZoom(_radius);
}


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
}


void Projection::setVisibleRadius(qreal r) {
	_visibleRadius = r;
	_scale = _screenRadius * _visibleRadius;
	_oneOverScale = 1.0f / _scale;

	_halfMapWidth = _scale * 2;
	_mapWidth = _scale * 4;
}


QPointF Projection::gridDistance() const {
	qreal dist = std::min(double((_width / 4) / pixelPerDegree()), 180.0);
	if ( dist < 0.01 )
		dist = std::max(int((dist*1000 + 0.6f)), 1)*0.001;
	else if ( dist < 0.1 )
		dist = std::max(int((dist*100 + 0.6f)), 1)*0.01;
	else if ( dist < 1 )
		dist = std::max(int((dist*10 + 0.6f)), 1)*0.1;
	else if ( dist < 5 )
		dist = std::max(int((dist + 0.6f)), 1);
	else
		dist = std::max(int((dist + 0.6f*5) / 5) * 5, 5);

	return QPointF(dist, dist);
}

/**
 * Returns true if the GUI object, represented by its bounding box in geo
 * coordinates, does not intersect the canvas. Note: This default implementation
 * is only valid for projections which protect the bounding box to a regular
 * rectangle (e.g. rectangular, mercator). Thus the clipping for the e.g.
 * spherical or kavrayskiy projections must be specialized.
 */
bool Projection::isClipped(const QPointF &bboxLR, const QPointF &bboxUL) const {
	QPoint p;
	return
		// Project the lower right corner of bounding box and check if
		// it intersects with the upper left corner of the canvas
		!project(p, bboxLR) || p.x() < 0 || p.y() < 0 ||
		// Project the upper left corner of bounding box and check if
		// it intersects with the lower right corner of the canvas
		!project(p, bboxUL) || p.x() > _width || p.y() > _height;
}


} // of ns Map
} // of ns Gui
} // of ns Seiscomp
