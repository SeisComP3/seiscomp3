/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * You can redistribute and/or modify this program under the               *
 * terms of the SeisComP Public License.                                   *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * SeisComP Public License for more details.                               *
 ***************************************************************************/

// Added with API version 13.0

#include "advancedoriginsymbol.h"

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/math.h>

#include <iostream>

#ifdef MACOSX
#define sincos __sincos
#endif


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AdvancedOriginSymbol::AdvancedOriginSymbol(Map::Decorator* decorator)
 : OriginSymbol(decorator) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AdvancedOriginSymbol::AdvancedOriginSymbol(double latitude, double longitude,
                                           double depth,
                                           Map::Decorator* decorator)
 : OriginSymbol(latitude, longitude, depth, decorator) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AdvancedOriginSymbol::AdvancedOriginSymbol(DataModel::Origin *origin,
                                           Map::Decorator *decorator)
    : OriginSymbol(decorator) {
	init();
	setOrigin(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::init() {
	_confidenceEllipsePen = Qt::NoPen;
	QColor fill = Qt::magenta;
	fill.setAlpha(64);
	_confidenceEllipseBrush = fill;
	_confidenceEllipseEnabled = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::setOrigin(DataModel::Origin *origin) {
	_origin = origin;

	// reset symbol
	setLocation(0.0, 0.0);
	setDepth(0.0);
	setPreferredMagnitudeValue(0.0);
	_confidenceEllipse.clear();

	if ( ! origin ) {
		return;
	}

	// hypocenter
	setLocation(_origin->latitude(), _origin->longitude());
	try {
		double depth = _origin->depth();
		setDepth(depth);
	} catch ( Core::ValueException& ) {}

	// magnitude
	if ( _origin->magnitudeCount() > 0 ) {
		double magnitude = origin->magnitude(0)->magnitude();
		setPreferredMagnitudeValue(magnitude);
	}

	// confidence ellipse
	double azi = 0.0;
	double major = 0.0;
	double minor = 0.0;

	try {
		// try to read ellipse from uncertainty element including azimuth
		azi = _origin->uncertainty().azimuthMaxHorizontalUncertainty();
		major = _origin->uncertainty().maxHorizontalUncertainty();
		minor = _origin->uncertainty().minHorizontalUncertainty();
	}
	catch ( Core::ValueException& ) {
		// try to read ellipse from latitude and longitude uncertainties
		// azimuth remains set to 0, major is set to latitude and minor to
		// longitude uncertainties
		try {
			major = std::max(_origin->latitude().lowerUncertainty(),
			                 _origin->latitude().upperUncertainty());
		}
		catch ( Core::ValueException& ) {
			try {
				major = _origin->latitude().uncertainty();
			}
			catch ( Core::ValueException& ) {}
		}

		try {
			minor = std::max(_origin->longitude().lowerUncertainty(),
			                 _origin->longitude().upperUncertainty());
		}
		catch ( Core::ValueException& ) {
			try {
				minor = _origin->longitude().uncertainty();
			}
			catch ( Core::ValueException& ) {}
		}

		// try to read circle from horizontal uncertainty
		if ( major == 0.0 && minor == 0 ) {
			try {
				major = minor = _origin->uncertainty().horizontalUncertainty();
			}
			catch ( Core::ValueException& ) {}
		}
	}

	setConfidenceEllipse(major, minor, azi);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::setConfidenceEllipse(double major, double minor,
                                                double azimuth, int points) {
//	azimuth = 0;
	if ( major <= 0 || minor <= 0 || points <= 0 ) {
		_confidenceEllipse.clear();
		return;
	}

	_confidenceEllipse.resize(points);

	double sinAzi, cosAzi, sinLat, cosLat;
	sincos(deg2rad(90 - azimuth), &sinAzi, &cosAzi);
	sincos(deg2rad(latitude()), &sinLat, &cosLat);

	// calculate geo coordinates on confidence ellipse with theta being the
	// angle on the unrotated standard ellipse and incTheta the increment
	// of theta for each point
	double theta = 0;
	double incTheta = 2 * M_PI / _confidenceEllipse.size();
	double radius = rad2deg(KM_OF_DEGREE); // average earth radius in km
	for ( QVector<Geo::GeoCoordinate>::iterator it = _confidenceEllipse.begin();
	      it != _confidenceEllipse.end(); ++it, theta += incTheta ) {

		// calculate point on standard ellipse and distance to origin in km
		double x, y;
		sincos(theta, &y, &x);
		x *= major; y *= minor;
		double dist = hypot(x, y);

		// calculated point on rotated ellipse in km
		double xAzi = x * cosAzi - y * sinAzi;
		double yAzi = x * sinAzi + y * cosAzi;

		// convert point to geo coordinates with delta being the angular
		// distance from the ellipse origin
		double delta = dist / radius;
		double sinDelta, cosDelta;
		sincos(delta, &sinDelta, &cosDelta);

		// latitude: arcsin only defined within [-1, 1]
		y = cosDelta * sinLat + (yAzi * sinDelta * cosLat / dist);
		it->lat = fabs(y) < 1 ? rad2deg(asin(y)) : y >= 0 ? 90 : -90 ;

		// longitude
		x = xAzi;
		if ( (latitude() - 90.0) > 1.0e-8 )
			y = -yAzi; // origin close to north pole
		else if ((latitude() + 90.0) < 1.0e-8)
			y = yAzi; // origin close to south pole
		else {
			x *= sinDelta;
			y = dist * cosLat * cosDelta - yAzi * sinLat * sinDelta;
		}
		it->lon = longitude();
		if ( x != 0 || y != 0 ) { // atan2 is not defined for x=0 and y=0;
			it->lon += rad2deg(atan2(x, y));
		}

		while ( it->lon < -180.0 )
			it->lon += 360.0;
		while ( it->lon > +180.0 )
			it->lon -= 360.0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::setConfidenceEllipsePen(const QPen &pen) {
	_confidenceEllipsePen = pen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPen& AdvancedOriginSymbol::confidenceEllipsePen() const {
	return _confidenceEllipsePen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::setConfidenceEllipseBrush(const QBrush &brush) {
	_confidenceEllipseBrush = brush;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QBrush& AdvancedOriginSymbol::confidenceEllipseBrush() const {
	return _confidenceEllipseBrush;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::setConfidenceEllipseEnabled(bool enabled) {
	_confidenceEllipseEnabled = enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AdvancedOriginSymbol::confidenceEllipseEnabled() const {
	return _confidenceEllipseEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::calculateMapPosition(const Map::Canvas *canvas) {
	OriginSymbol::calculateMapPosition(canvas);
	_renderPath = QPainterPath();
	if ( !_confidenceEllipse.empty() )
		canvas->projection()->project(_renderPath, _confidenceEllipse.size(),
		                              _confidenceEllipse.constData(), true, 3);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AdvancedOriginSymbol::customDraw(const Map::Canvas *canvas,
                                      QPainter &painter) {
	if ( _confidenceEllipseEnabled
	  && !_confidenceEllipse.isEmpty()
	  && !_renderPath.isEmpty() ) {
		painter.save();

		painter.setPen(_confidenceEllipsePen);
		painter.setBrush(_confidenceEllipseBrush);
		painter.drawPath(_renderPath);

		painter.restore();
	}

	OriginSymbol::customDraw(canvas, painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Gui
} // namespace Seiscomp
