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


#include "originsymbol.h"

#include <cmath>
#include <algorithm>
#include <functional>

#include <QMatrix>
#include <QPointF>

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/projection.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginSymbol::OriginSymbol(Map::Decorator* decorator)
: Symbol(decorator)
, _filled(false)
, _preferredMagnitudeValue(0)
, _depth(0) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginSymbol::OriginSymbol(double latitude,
                           double longitude,
                           double depth,
                           Map::Decorator* decorator)
: Symbol(latitude, longitude, decorator)
, _filled(false)
, _depth(depth) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::customDraw(const Map::Canvas *, QPainter& painter) {
	painter.save();

	QPen pen;
	pen.setColor(_color);
	pen.setWidth(SCScheme.colors.originSymbol.classic ? 4 : 2);
	pen.setJoinStyle(Qt::MiterJoin);
	painter.setPen(pen);

	QBrush brush;
	if ( SCScheme.colors.originSymbol.classic ) {
		brush.setColor(_color);
		brush.setStyle(isFilled() ? Qt::SolidPattern : Qt::NoBrush);
	}
	else {
		brush.setColor(isFilled() ? _color : QColor(_color.red(), _color.green(), _color.black(), 128));
		brush.setStyle(Qt::SolidPattern);
	}

	/*
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	if ( isFilled() )
		brush.setColor(_color);
	else
		brush.setColor(QColor(_color.red(), _color.green(), _color.blue(), 64));
	*/

	painter.setBrush(brush);

	const QSize &size = Seiscomp::Gui::Map::Symbol::size();

	int height = size.height(),
	    width = size.width(),
	    r = int(width / 2);
	QRect rect(_position.x() - r, _position.y() - r, width, height);
	_poly = rect;
	painter.drawEllipse(rect);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setPreferredMagnitudeValue(double magnitudeValue) {
	_preferredMagnitudeValue = magnitudeValue;
	updateSize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginSymbol::preferredMagnitudeValue() const {
	return _preferredMagnitudeValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginSymbol::depth() const {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setDepth(double depth) {
	_depth = depth;
	depthColorCoding();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setFilled(bool val) {
	_filled = val;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginSymbol::isFilled() const {
	return _filled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginSymbol::isInside(int x, int y) const {
	return _poly.boundingRect().contains(x, y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::updateSize() {
	int width;
	if ( _preferredMagnitudeValue > 0 )
		// 2.7**1.6 = 4.9
		// 2.7**0.19 = 1.2
		width = std::max(SCScheme.map.originSymbolMinSize, int(4.9 * (_preferredMagnitudeValue - 1.2)));
	else
		width = SCScheme.map.originSymbolMinSize;

	setSize(QSize(width, width));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::init() {
	_color                   = Qt::black;
	_defaultSize             = 20;
	_preferredMagnitudeValue = 0.;

	setPriority(Symbol::HIGH);
	setSize(QSize(_defaultSize, _defaultSize));
	depthColorCoding();
	updateSize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::depthColorCoding() {
	_color = SCScheme.colors.originSymbol.depth.gradient.colorAt(
	            _depth,
	            SCScheme.colors.originSymbol.depth.discrete
	         );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Gui
} // namespace Seiscomp
