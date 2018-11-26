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


#include <seiscomp3/gui/map/layers/gridlayer.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/geo/coordinate.h>


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridLayer::GridLayer(QObject* parent) : Layer(parent) {
	setName("grid");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridLayer::~GridLayer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridLayer::draw(const Seiscomp::Gui::Map::Canvas *canvas,
                     QPainter &painter) {
	if ( !isVisible() ) return;
	if ( canvas == NULL ) return;

	Seiscomp::Gui::Map::Projection *projection = canvas->projection();
	if ( projection == NULL ) return;

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing, isAntiAliasingEnabled());
	painter.setPen(SCScheme.colors.map.grid);

#if 0
	// Y gridlines
	projection->drawLatCircle(p,  0.0);
	projection->drawLatCircle(p, 90.0);
	projection->drawLatCircle(p, 180.0);
	projection->drawLatCircle(p, 270.0);

	// X gridlines
	projection->drawLonCircle(p,  0.0);
	projection->drawLonCircle(p,  66.55);
	projection->drawLonCircle(p,  23.4333);
	projection->drawLonCircle(p, -23.4333);
	projection->drawLonCircle(p, -66.55);
#else

	QPointF c = projection->visibleCenter();
	qreal modX = fmod(c.x(), _gridDistance.x());
	qreal modY = fmod(c.y(), _gridDistance.y());

	Geo::GeoCoordinate start0(c.y() - modY, c.x() - modX);
	Geo::GeoCoordinate start1;

	if ( c.x() < 0 ) {
		start1.lon = start0.lon;
		start0.lon = start0.lon - _gridDistance.x();
	}
	else
		start1.lon = start0.lon + _gridDistance.x();

	if ( c.y() < 0 ) {
		start1.lat = start0.lat;
		start0.lat = start0.lat - _gridDistance.y();
	}
	else
		start1.lat = start0.lat + _gridDistance.y();

	start0.normalize();
	start1.normalize();

	qreal x = start1.lon;
	qreal toX = c.x() + 180;

	while ( x < toX && projection->drawLatCircle(painter, x) )
		x += _gridDistance.x();

	x = start0.lon;
	toX = c.x() - 180;
	while ( x > toX && projection->drawLatCircle(painter, x) )
		x -= _gridDistance.x();

	qreal y = start1.lat;
	qreal toY = 90;

	while ( y < toY && projection->drawLonCircle(painter, y) )
		y += _gridDistance.y();

	y = start0.lat;
	toY = -90;

	while ( y > toY && projection->drawLonCircle(painter, y) )
		y -= _gridDistance.y();
#endif

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridLayer::setGridDistance(const QPointF &p) {
	_gridDistance = p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF &GridLayer::gridDistance() const {
	return _gridDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
