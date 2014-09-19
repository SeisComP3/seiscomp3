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


#include <seiscomp3/gui/datamodel/ttdecorator.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/math/geo.h>

#include <iostream>
#include <cmath>


namespace Seiscomp {
namespace Gui {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TTDecorator::TTDecorator(Map::Canvas *canvas, Decorator *decorator)
 : Decorator(decorator),
   _canvas(canvas),
   _deltaDepth(50),
   _maxPDistance(110),
   _maxSDistance(15),
   _pDistance(_maxPDistance),
   _deltaDist(5),
   _rotDelta(3),
   _preferredMagnitudeVal(0),
   _longitude(0),
   _latitude(0),
   _depth(-1) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::setPreferredMagnitudeValue(double val)
{
	if (fabs(_preferredMagnitudeVal - val) > 0.5)
	{
		_pDistance = pow(val, 3.5) * 1.1;
		//std::cout << "Calculated pDistance: " << _pDistance << std::endl;
		if (_pDistance > _maxPDistance)
			_pDistance = _maxPDistance;
	}
	else if (val == 0)
	{
		_pDistance = _maxPDistance;
	}
	_preferredMagnitudeVal = val;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TTDecorator::longitude() const {
	return _longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::setLongitude(double longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TTDecorator::latitude() const {
	return _latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::setLatitude(double latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TTDecorator::depth() const {
	return _depth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::setDepth(double depth) {
	_depth = depth;

	computeTTT(_travelTimesS, "S", _maxSDistance);
	computeTTT(_travelTimesP, "P", _maxPDistance);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::TimeQuantity& TTDecorator::originTime() const {
	return _originTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::setOriginTime(const DataModel::TimeQuantity& time) {
	_originTime = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::customDraw(const Map::Canvas*, QPainter& painter) {
	// It is important to separate the examination of isVisible and IsWaveformPropagationVisible
	// at separate Points. This is to make sure that the waveformpropagation is still calculated
	// even when the display is globally switched off.
	if ( !isVisible() ) return;

	// Add a time contsrain here
	// if time > 1/2 hour set _d = false ...

	// std::cout << "==== P Phase ====" << std::endl;
	double distanceP = computeTTTPolygon(_travelTimesP, _polygonP);
	// std::cout << "==== S Phase ====" << std::endl;
	double distanceS = computeTTTPolygon(_travelTimesS, _polygonS);

	if ( distanceP == 0 && distanceS == 0 ) {
		setVisible(false);
		return;
	}

	painter.save();

	QPen pen;
	QColor color(Qt::yellow);
	color.setAlpha(static_cast<int>(255 * (1 - distanceP / (_pDistance + 15))));
	pen.setColor(color);
	pen.setWidth(3);
	pen.setJoinStyle(Qt::MiterJoin);
	//	pen.setStyle(Qt::DotLine);
	painter.setPen(pen);

	if ( distanceP > 0 ) {
		drawPolygon(painter, _polygonP);
		annotatePropagation(painter, distanceP, EAST_WEST);
	}

	color = Qt::blue;
	//color.setAlpha(static_cast<int>(255 * (1 - distanceS / (_maxSDistance + _deltaDist))));
	pen.setWidth(2);
	pen.setColor(color);
	painter.setPen(pen);

	if ( distanceS > 0 ) {
		drawPolygon(painter, _polygonS);
		annotatePropagation(painter, distanceS, NORTH_SOUTH);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::computeTTT(TravelTimes& travelTimes,
                             const std::string& phase,
                             double phaseDistance)
{
	travelTimes.clear();

	double lat2, lon2;

	for ( int distance = 0; distance <= phaseDistance; distance += _deltaDist ) {
		Math::Geo::delandaz2coord(distance, 90, _latitude, _longitude, &lat2, &lon2);

		try {
			travelTimes.push_back(_ttTable.compute(phase.c_str(), _latitude, _longitude, _depth, lat2, lon2, 0).time);
		}
		catch ( const std::exception &e ) {
			std::cerr << "TTT: " << e.what() << std::endl;
			//std::cout << "Could not calculate " << phase << " travel time for: " << distance << " degrees" << std::endl;
			travelTimes.push_back(-1);
		}
	}

	//std::cout << "Amount of calculated P phases: " << _travelTimesP.size() << " for depth: " << _depth << " and distance: " << phaseDistance << std::endl;
	//for (int i = 0; i < _travelTimesP.size(); ++i)
	//	std::cout << "(" << i << ") P Traveltime: " << _travelTimesP[i] << " for " << i*_deltaDist << " degrees" << std::endl;

	//std::cout << "Amount of calculated S phases: " << _travelTimesS.size() << " for depth: " << _depth << " and distance: " << phaseDistance << std::endl;
	//for (int i = 0; i < _travelTimesS.size(); ++i)
	//	std::cout << "(" << i << ") S Traveltime: " << _travelTimesS[i] << " for " << i*_deltaDist << " degrees" << std::endl;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TTDecorator::computeTTTPolygon(const std::vector<double>& travelTimes,
                                      std::vector<QPointF>& polygon) {
	polygon.clear();

	// double timeSpan = (Core::Time::GMT() - DataModel::Origin::Cast(baseObject())->time().value()).seconds();
	double timeSpan = (Core::Time::GMT() - _originTime.value()).seconds();
	//std::cout << "Calculated traveltime: " << timeSpan << std::endl;

	std::vector<double>::const_iterator it =
	    std::find_if(
	        travelTimes.begin(), travelTimes.end(),
	        std::bind1st(std::less_equal<double>(), timeSpan));

	if ( it == travelTimes.end() ) {
		return 0;
	}

	double p0 = 0;
	double t1 = *it;
	double t0 = t1;

	if ( (it - 1) != travelTimes.begin() ) {
		if ( *(it - 1) >= 0 ) {
			p0 = std::distance(travelTimes.begin(), it - 1);
			t0 = *(it - 1);
		}
		else {
			p0 = std::distance(travelTimes.begin(), it);
		}
	}

	// Interpolate
	double distance = 0;
	p0 *= _deltaDist;
	double dt = t1 - t0;
	double dtx = timeSpan - t0;

	if (dt != 0)
		distance = (dtx / dt * _deltaDist) + p0;
	else
		distance = p0;

	if ( distance > 0 ) {
		for ( int i = 0; i < 360; i += _rotDelta ) {
			double outLat = 0, outLon = 0;
			Math::Geo::delandaz2coord(distance, i, _latitude, _longitude, &outLat, &outLon);
			polygon.push_back(QPointF(outLon, outLat));
		}
	}

	return distance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::drawPolygon(QPainter& painter, const std::vector<QPointF>& polygon)
{
	for ( size_t i = 1; i < polygon.size(); ++i )
		_canvas->projection()->drawLine(painter, polygon[i-1], polygon[i]);
	if ( polygon.size() > 2 )
		_canvas->projection()->drawLine(painter, polygon.back(), polygon.front());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::annotatePropagation(QPainter& painter, double distance,
                                      Direction direction) {
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, false);

	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidth(1);
	painter.setPen(pen);

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	QColor color(Qt::white);
	color.setAlpha(100);
	brush.setColor(color);
	painter.setBrush(brush);

	QString text(QString("%1").arg(distance, 0 ,'f', 1));
	QRect bb = painter.fontMetrics().boundingRect(text);
	QRect box0 = bb, box1 = bb;
	QPoint p0, p1;
	int textHeight = bb.height();
	int textWidth = bb.width();
	int textOffset = static_cast<int>(textWidth / 2 + 0.5);

	bool p0Ok = true;
	bool p1Ok = true;

	if (direction == NORTH_SOUTH)
	{
		if ( _canvas->projection()->project(p0, QPointF(_longitude, _latitude + distance)) )
			p0.setY(p0.y() - textHeight+4);
		else
			p0Ok = false;
		if ( _canvas->projection()->project(p1, QPointF(_longitude, _latitude - distance)) )
			p1.setY(p1.y() + textHeight+4);
		else
			p1Ok = false;
	}
	else if (direction == EAST_WEST)
	{
		if ( _canvas->projection()->project(p0, QPointF(_longitude + distance, _latitude)) )
			p0.setX(p0.x() + textWidth);
		else
			p0Ok = false;
		if ( _canvas->projection()->project(p1, QPointF(_longitude - distance, _latitude)) )
			p1.setX(p1.x() - textWidth);
		else
			p1Ok = false;
	}

	int dy = 2; // improve vertical centering

	if ( p0Ok ) {
		box0.adjust(p0.x()-5-textOffset, p0.y()-2, p0.x()+1-textOffset, p0.y()+2);
		painter.drawRect(box0);
		box0.adjust(0, -dy, 0, dy);
		painter.drawText(box0, Qt::AlignCenter, text);
	}

	if ( p1Ok ) {
		box1.adjust(p1.x()-5-textOffset, p1.y()-2, p1.x()+1-textOffset, p1.y()+2);
		painter.drawRect(box1);
		box1.adjust(0, -dy, 0, dy);
		painter.drawText(box1, Qt::AlignCenter, text);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Gui
} // namespace Seiscomp
