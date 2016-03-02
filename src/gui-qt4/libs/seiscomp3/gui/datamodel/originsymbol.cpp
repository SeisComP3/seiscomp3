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


IMPLEMENT_RTTI(OriginSymbol, "OriginSymbol", Map::Symbol)
IMPLEMENT_RTTI_METHODS(OriginSymbol)



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginSymbol::OriginSymbol(Map::Decorator* decorator)
 : Symbol(decorator),
   _filled(false),
   _geoPosition(0, 0),
   _depth(0) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginSymbol::OriginSymbol(double latitude,
                           double longitude,
                           double depth,
                           Map::Decorator* decorator)
 : Symbol(decorator),
   _filled(false),
   _geoPosition(latitude, longitude),
   _depth(depth) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginSymbol::~OriginSymbol() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::update() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::calculateMapPosition(const Map::Canvas *canvas) {
	double latitude = _geoPosition.x();
	double longitude = _geoPosition.y();

	setClipped(!canvas->projection()->project(_mapPosition, QPointF(longitude, latitude)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginSymbol::hasValidMapPosition() const {
	if ( _mapPosition.x() < 0 && _mapPosition.y() < 0 )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::customDraw(const Map::Canvas *canvas, QPainter& painter) {
	drawOriginSymbol(canvas, painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::drawOriginSymbol(const Map::Canvas*, QPainter& painter) {
	painter.save();

	QPen pen;
	pen.setColor(_color);
	pen.setWidth(4);
	pen.setJoinStyle(Qt::MiterJoin);
	painter.setPen(pen);

	QBrush brush;
	brush.setColor(_color);
	brush.setStyle(Qt::NoBrush);
	if ( isFilled() )
		brush.setStyle(Qt::SolidPattern);

	/*
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	if ( isFilled() )
		brush.setColor(_color);
	else
		brush.setColor(QColor(_color.red(), _color.green(), _color.blue(), 64));
	*/

	painter.setBrush(brush);

	const QSize& size = Seiscomp::Gui::Map::Symbol::size();
	int height = size.height(),
	    width = size.width(),
	    r = int(width / 2);
	QRect rect(_mapPosition.x() - r, _mapPosition.y() - r, width, height);
	_poly = rect;
	painter.drawEllipse(rect);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginSymbol::isClipped(const Map::Canvas *canvas) {
	double latitude = _geoPosition.x();
	double longitude = _geoPosition.y();

	return canvas->isInside(longitude, latitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




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
void OriginSymbol::setLatitude(double latitude) {
	_geoPosition.setX(latitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginSymbol::latitude() const {
	return _geoPosition.x();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setLongitude(double longitude) {
	_geoPosition.setY(longitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double OriginSymbol::longitude() const {
	return _geoPosition.y();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginSymbol::x() const {
	return _mapPosition.x();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setX(int xPos) {
	_mapPosition.setX(xPos);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int OriginSymbol::y() const {
	return _mapPosition.y();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginSymbol::setY(int yPos) {
	_mapPosition.setY(yPos);
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
	_mapPosition             = QPoint(-1, -1);
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
