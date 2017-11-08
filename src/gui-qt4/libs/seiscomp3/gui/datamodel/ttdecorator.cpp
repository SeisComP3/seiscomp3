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
TTDecorator::TTDecorator(Decorator *decorator)
 : Decorator(decorator),
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
void TTDecorator::customDraw(const Map::Canvas *canvas, QPainter& painter) {
	// It is important to separate the examination of isVisible and IsWaveformPropagationVisible
	// at separate Points. This is to make sure that the waveformpropagation is still calculated
	// even when the display is globally switched off.
	if ( !isVisible() ) return;

	// Add a time contsrain here
	// if time > 1/2 hour set _d = false ...

	double distanceP = computeTTTPolygon(_travelTimesP, _polygonP);
	double distanceS = computeTTTPolygon(_travelTimesS, _polygonS);
	if ( distanceP == 0 && distanceS == 0 ) {
		setVisible(false);
		return;
	}

	painter.save();

	QPen pen;
	QColor color(Qt::yellow);
	int alpha = static_cast<int>(255 * (1 - distanceP / (_pDistance + 15)));
	if ( alpha < 0 ) alpha = 0;
	color.setAlpha(alpha);
	pen.setColor(color);
	pen.setWidth(3);
	pen.setJoinStyle(Qt::MiterJoin);
	//	pen.setStyle(Qt::DotLine);
	painter.setPen(pen);

	if ( (distanceP > 0) && alpha ) {
		drawPolygon(canvas, painter, _polygonP);
		annotatePropagation(canvas, painter, distanceP, EAST_WEST);
	}

	color = Qt::blue;
	//color.setAlpha(static_cast<int>(255 * (1 - distanceS / (_maxSDistance + _deltaDist))));
	pen.setWidth(2);
	pen.setColor(color);
	painter.setPen(pen);

	if ( distanceS > 0 ) {
		drawPolygon(canvas, painter, _polygonS);
		annotatePropagation(canvas, painter, distanceS, NORTH_SOUTH);
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

//	std::cout << "Amount of calculated " << phase << " phases: " << travelTimes.size() << " for depth: " << _depth << " and distance: " << phaseDistance << std::endl;
//	for ( size_t i = 0; i < travelTimes.size(); ++i )
//		std::cout << "(" << i << ") " << phase << " Traveltime: " << travelTimes[i] << " for " << i*_deltaDist << " degrees" << std::endl;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TTDecorator::computeTTTPolygon(const std::vector<double>& travelTimes,
                                      std::vector<QPointF>& polygon) {
	polygon.clear();

	// double timeSpan = (Core::Time::GMT() - DataModel::Origin::Cast(baseObject())->time().value()).seconds();
	double timeSpan = (Core::Time::GMT() - _originTime.value()).seconds();
	//std::cout << "Calculated traveltime: " << timeSpan << std::endl;

	double distance = 0;

	for ( size_t i = 0; i < travelTimes.size(); ++i ) {
		if ( timeSpan <= travelTimes[i] ) {
			if ( i > 0 ) {
				double diff = timeSpan - travelTimes[i-1];
				double length = travelTimes[i] - travelTimes[i-1];
				double t = diff/length;
				distance = (i-1+t)*_deltaDist;
			}

			break;
		}
	}

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
void TTDecorator::drawPolygon(const Map::Canvas *canvas, QPainter& painter,
                              const std::vector<QPointF>& polygon)
{
	for ( size_t i = 1; i < polygon.size(); ++i )
		canvas->projection()->drawLine(painter, polygon[i-1], polygon[i]);
	if ( polygon.size() > 2 )
		canvas->projection()->drawLine(painter, polygon.back(), polygon.front());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TTDecorator::annotatePropagation(const Map::Canvas *canvas, QPainter& painter,
                                      double distance, Direction direction) {
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
	bb.adjust(-2,-2,2,2);

	QRect box0 = bb, box1 = bb;
	QPoint p;
	bool p0Ok = true, p1Ok = true;
	double outLat = 0, outLon = 0;

	if ( direction == NORTH_SOUTH ) {
		Math::Geo::delandaz2coord(distance, 0, _latitude, _longitude, &outLat, &outLon);
		if ( canvas->projection()->project(p, QPointF(outLon, outLat)) ) {
			box0.moveCenter(p);
			box0.moveBottom(p.y()-4);
		}
		else
			p0Ok = false;

		Math::Geo::delandaz2coord(distance, 180, _latitude, _longitude, &outLat, &outLon);
		if ( canvas->projection()->project(p, QPointF(outLon, outLat)) ) {
			box1.moveCenter(p);
			box1.moveTop(p.y()+4);
		}
		else
			p1Ok = false;
	}
	else if ( direction == EAST_WEST ) {
		Math::Geo::delandaz2coord(distance, 90, _latitude, _longitude, &outLat, &outLon);
		if ( canvas->projection()->project(p, QPointF(outLon, outLat)) ) {
			box0.moveCenter(p);
			box0.moveLeft(p.x()+4);
		}
		else
			p0Ok = false;

		Math::Geo::delandaz2coord(distance, 270, _latitude, _longitude, &outLat, &outLon);
		if ( canvas->projection()->project(p, QPointF(outLon, outLat)) ) {
			box1.moveCenter(p);
			box1.moveRight(p.x()-4);
		}
		else
			p1Ok = false;
	}

	if ( p0Ok ) {
		painter.drawRect(box0);
		painter.drawText(box0, Qt::AlignCenter, text);
	}

	if ( p1Ok ) {
		painter.drawRect(box1);
		painter.drawText(box1, Qt::AlignCenter, text);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Gui
} // namespace Seiscomp
