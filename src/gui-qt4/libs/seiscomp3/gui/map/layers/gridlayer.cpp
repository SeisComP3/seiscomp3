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
void GridLayer::draw(const Seiscomp::Gui::Map::Canvas* canvas,
                     QPainter& painter) {
	if ( !isVisible() ) return;
	if ( canvas == NULL ) return;

	Seiscomp::Gui::Map::Projection* projection = canvas->projection();
	if ( projection == NULL ) return;

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing, isAntiAliasingEnabled());

	QPen gridPen(SCScheme.colors.map.grid);
#ifdef Q_WS_MAC
	// For unknown reasons OSX cannot display dotted lines
	gridPen.setStyle(Qt::DashLine);
#else
	gridPen.setStyle(Qt::DotLine);
#endif

	painter.setPen(gridPen);

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

	QPointF start0 = QPointF(c.x() - modX, c.y() - modY);
	QPointF start1;

	if ( c.x() < 0 ) {
		start1.setX(start0.x());
		start0.setX(start0.x() - _gridDistance.x());
	}
	else
		start1.setX(start0.x() + _gridDistance.x());

	if ( c.y() < 0 ) {
		start1.setY(start0.y());
		start0.setY(start0.y() - _gridDistance.y());
	}
	else
		start1.setY(start0.y() + _gridDistance.y());

	qreal x = start1.x();
	qreal toX = c.x() + 180;

	while ( x < toX && projection->drawLatCircle(painter, x) )
		x += _gridDistance.x();

	x = start0.x();
	toX = c.x() - 180;
	while ( x > toX && projection->drawLatCircle(painter, x) )
		x -= _gridDistance.x();

	qreal y = start1.y();
	qreal toY = 90;

	while ( y < toY && projection->drawLonCircle(painter, y) )
		y += _gridDistance.y();

	y = start0.y();
	toY = -90;

	while ( y > toY && projection->drawLonCircle(painter, y) )
		y -= _gridDistance.y();
#endif

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridLayer::setGridDistance(const QPointF& p) {
	_gridDistance = p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& GridLayer::gridDistance() const {
	return _gridDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
