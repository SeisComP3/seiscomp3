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


#include "mvstationsymbol.h"

using namespace Seiscomp;
using namespace Seiscomp::Gui;


IMPLEMENT_RTTI(MvStationSymbol, "MvStationSymbol", StationSymbol)
IMPLEMENT_RTTI_METHODS(MvStationSymbol)




MvStationSymbol::MvStationSymbol(Map::Decorator* decorator)
 : StationSymbol(decorator) {

	init();
}




MvStationSymbol::MvStationSymbol(double latitude, double longitude,
					             Map::Decorator* decorator)
 : StationSymbol(latitude, longitude, decorator) {

	init();
}




void MvStationSymbol::setIdDrawingColor(const QColor& color) {
	_idDrawingColor = color;
}




void MvStationSymbol::setIdDrawingEnabled(bool enabled) {
	_isDrawingIdEnabled = enabled;
}




bool MvStationSymbol::isIdDrawingEnabled() const {
	return _isDrawingIdEnabled;
}




void MvStationSymbol::setCharacter(const QChar& c) {
	_char = c;
}




void MvStationSymbol::setCharacterDrawingColor(const QColor& color) {
	_characterDrawingColor = color;
}




void MvStationSymbol::setCharacterDrawingEnabled(bool val) {
	_isCharacterDrawingEnabled = val;
}




bool MvStationSymbol::isCharacterDrawingEnabled() const {
	return _isCharacterDrawingEnabled;
}




const std::string& MvStationSymbol::networkCode() const {
    return _networkCode;
}




void MvStationSymbol::setNetworkCode(const std::string& networkCode) {
    _networkCode = networkCode;
}




const std::string& MvStationSymbol::stationCode() const {
    return _stationCode;
}




void MvStationSymbol::setStationCode(const std::string& stationCode) {
    _stationCode = stationCode;
}




const std::string& MvStationSymbol::locationCode() const {
    return _locationCode;
}



void MvStationSymbol::setLocationCode(const std::string& locationCode) {
    _locationCode = locationCode;
}




const std::string& MvStationSymbol::channelCode() const {
    return _channelCode;
}




void MvStationSymbol::setChannleCode(const std::string& channelCode) {
    _channelCode = channelCode;
}



void MvStationSymbol::init() {
	_isDrawingIdEnabled = false;
	_idDrawingColor     = Qt::black;

	_char                      = ' ';
	_characterDrawingColor     = Qt::black;
	_isCharacterDrawingEnabled = false;
}




void MvStationSymbol::customDraw(const Map::Canvas *canvas, QPainter& painter) {
	StationSymbol::customDraw(canvas, painter);

	if ( isCharacterDrawingEnabled() ) drawCharacter(painter);

	if ( isIdDrawingEnabled() ) drawID(painter);
}




void MvStationSymbol::drawID(QPainter& painter) {
	painter.save();

	QFont idFont = painter.font();
	idFont.setBold(true);
	painter.setFont(idFont);

	painter.setPen(_idDrawingColor);

	QPoint p0 = stationPolygon().point(0);
	QPoint p1 = stationPolygon().point(1);
	QPoint p2 = stationPolygon().point(2);
	QPoint p3(p0.x() + (int)std::ceil((p0.x() - p2.x()) * 0.4),
			  p0.y() - (int)std::ceil((p2.y() - p0.y()) * 0.4));
	QPoint p4(p1.x() + (int)std::ceil((p0.x() - p2.x()) * 0.1), p3.y());

	painter.drawLine(p0, p3);
	painter.drawLine(p3, p4);

	painter.setPen(Qt::darkGray);
	painter.drawText(p4.x(), p4.y() + 1, id().c_str());

	painter.setPen(_idDrawingColor);

    std::string stationId = networkCode () + "." + stationCode();
	painter.drawText(p4.x() + 1, p4.y(), stationId.c_str());

	painter.restore();
}




void MvStationSymbol::drawCharacter(QPainter& painter) {
	painter.save();

	painter.setPen(_characterDrawingColor);
	QRect rect = painter.fontMetrics().boundingRect(_char);
	painter.drawText(x() - rect.width() / 2, y() + int(0.5 * size()), _char);

	painter.restore();
}




