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



#include <seiscomp3/gui/map/rectangularprojection.h>
#include <seiscomp3/gui/map/texturecache.ipp>

#include <seiscomp3/math/geo.h>

#include <math.h>
#include <iostream>

#define deg2rad(d) (M_PI*(d)/180.0)
#define rad2deg(d) (180.0*(d)/M_PI)
#define HALF_PI (M_PI/2)

const qreal ooPi = 1.0 / M_PI;


namespace Seiscomp {
namespace Gui {
namespace Map {


REGISTER_PROJECTION_INTERFACE(RectangularProjection, "Rectangular");


namespace {

const qreal ooLat = 1.0 / 90.0;
const qreal ooLon = 1.0 / 180.0;


QString lat2String(qreal lat) {
	return QString("%1%2").arg(abs((int)lat)).arg(lat < 0?" S":lat > 0?" N":"");
}


QString lon2String(qreal lon) {
	lon = fmod(lon, 360.0);
	if ( lon < 0 ) lon += 360.0;
	if ( lon > 180.0 ) lon -= 360.0;

	return QString("%1%2").arg(abs((int)lon)).arg(lon < 0?" W":lon > 0?" E":"");
}


}


RectangularProjection::RectangularProjection()
: Projection() {
	_enableLowZoom = false;
}


void RectangularProjection::setLowZoomEnabled(bool e) {
	_enableLowZoom = e;
}


bool RectangularProjection::isRectangular() const {
	return true;
}


bool RectangularProjection::wantsGridAntialiasing() const {
	return false;
}


template <typename PROC>
void RectangularProjection::render(QImage &img, TextureCache *cache) {
	_screenRadius = std::min(_width*0.25, _height*0.5);

	QSize size(img.size());

	qreal radius = _screenRadius * _radius;
	double dt;
	qreal visibleRadius;

	if ( !_enableLowZoom ) {
		if ( radius < _halfWidth )
			radius = _width*0.25;

		if ( radius < _halfHeight )
			radius = _height*0.5;

		visibleRadius = radius / _screenRadius;
	}
	else
		visibleRadius = _radius;

	dt = 1.0 / qreal(radius-1);

	setVisibleRadius(visibleRadius);

	QPoint center = QPoint(_halfWidth, _halfHeight);

	int fromY, toY;
	fromY = 0;
	toY = size.height();

	qreal iyf = center.y() * dt;
	if ( iyf > 1.0 ) {
		if ( _enableLowZoom ) {
			fromY = (iyf - 1.0) * radius;
			toY = img.height() - fromY;
		}
		iyf = 1.0;
	}

	QRgb *data = (QRgb *)img.bits();

	int centerX = center.x();

	qreal upY = _center.y() + iyf;
	qreal downY = upY - (toY - fromY) * dt;

	if ( downY < -1.0 ) {
		downY = -1.0;
		upY = downY + (toY - fromY) * dt;
		_visibleCenter.setY((upY + downY) * 0.5);
	}

	if ( upY > 1.0 ) {
		upY = 1.0;
		downY = upY - (toY - fromY) * dt;
		_visibleCenter.setY((upY + downY) * 0.5);
	}

	data += fromY * img.width();

	qreal y = upY;

	//qreal ixf = (qreal)centerX / radius;
	qreal ixf = 2.0;
	qint64 pxf = qint64(ixf*radius);
	qint64 fx, tx;

	fx = centerX - pxf;
	tx = centerX + pxf;

	if ( fx < 0 ) {
		ixf += fx * dt;
		fx = 0;
	}

	// Clip to left border
	if ( tx < 2 ) tx = 0;

	// Clip to right border
	if ( tx >= size.width()-2 ) tx = size.width()-1;

	int fromX = (int)fx;
	int toX = (int)tx;

	if ( cache == NULL ) return;

	qreal pixelRatio = 2.0*_scale / cache->tileHeight();
	if ( cache->isMercatorProjected() )
		pixelRatio *= 2;

	if ( pixelRatio < 1 ) pixelRatio = 1;
	int level = (int)(log(pixelRatio) / log(2.0) + 0.7);
	if ( level > cache->maxLevel() )
		level = cache->maxLevel();

	qreal leftX = 2.0*_center.x() - ixf;
	qreal rightX = 2.0*_center.x() + ixf;

	int pixels = toX - fromX + 1;

	Coord leftTu;
	Coord rightTu;

	leftTu.value = (leftX*0.5+1.0) * Coord::value_type(Coord::fraction_half_max);
	rightTu.value = (rightX*0.5+1.0) * Coord::value_type(Coord::fraction_half_max);

	if ( cache->isMercatorProjected() ) {
		for ( int i = fromY; i < toY; ++i, y -= dt ) {
			if ( y <= -1.0 ) y = -1.0 + dt;

			Coord tv;
			qreal lat = y;

			if ( lat > 0.94 ) lat = 0.94;
			else if ( lat < -0.94 ) lat = -0.94;
			lat = ooPi*asinh(tan(lat*HALF_PI));

			tv.value = (1.0f-lat) * Coord::value_type(Coord::fraction_half_max);

			PROC::fetch(cache, data[fromX], leftTu, tv, level);
			PROC::fetch(cache, data[toX], rightTu, tv, level);

			// Shift only by 30 bits to keep the sign bit in the lower 32 bit
			Coord::value_type xDelta = rightTu.value - leftTu.value;
			qint64 stepU = (qint64(xDelta) << 30) / pixels;
			qint64 stepper;
			Coord lon;
			stepper = qint64(leftTu.value) << 30;
			stepper += stepU;

			for ( int k = 1; k < pixels; ++k ) {
				lon.value = stepper >> 30;
				PROC::fetch(cache, data[fromX + k], lon, tv, level);
				stepper += stepU;
			}

			data += size.width();
		}
	}
	else {
		for ( int i = fromY; i < toY; ++i, y -= dt ) {
			if ( y <= -1.0 ) y = -1.0 + dt;

			Coord tv;
			tv.value = (1.0-y) * Coord::value_type(Coord::fraction_half_max);

			PROC::fetch(cache, data[fromX], leftTu, tv, level);
			PROC::fetch(cache, data[toX], rightTu, tv, level);

			// Shift only by 30 bits to keep the sign bit in the lower 32 bit
			Coord::value_type xDelta = rightTu.value - leftTu.value;
			qint64 stepU = (qint64(xDelta) << 30) / pixels;
			qint64 stepper;
			Coord lon;
			stepper = qint64(leftTu.value) << 30;
			stepper += stepU;

			for ( int k = 1; k < pixels; ++k ) {
				lon.value = stepper >> 30;
				PROC::fetch(cache, data[fromX + k], lon, tv, level);
				stepper += stepU;
			}

			data += size.width();
		}
	}
}


void RectangularProjection::render(QImage& img, bool highQuality, TextureCache *cache) {
	if ( highQuality )
		render<BilinearFilter>(img, cache);
	else
		render<NearestFilter>(img, cache);
}


bool RectangularProjection::project(QPoint &screenCoords, const QPointF &geoCoords) const {
	qreal x = geoCoords.x() * ooLon;

	qreal lat = geoCoords.y();
	if ( lat > 90.0 ) {
		lat = 180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}
	else if ( lat < -90.0 ) {
		lat = -180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}

	qreal y = lat * ooLat;

	x = (x - _visibleCenter.x()) * _halfMapWidth;
	y = (y - _visibleCenter.y()) * _scale;

	if ( x > _halfMapWidth )
		x -= _mapWidth;

	if ( x < -_halfMapWidth )
		x += _mapWidth;

	screenCoords.setX(_halfWidth  + x);
	screenCoords.setY(_halfHeight - y);

	return true;
}


bool RectangularProjection::unproject(QPointF &geoCoords, const QPoint &screenCoords) const {
	qreal x = screenCoords.x() - _halfWidth;
	qreal y = _halfHeight - screenCoords.y();

	if ( x < -_halfMapWidth || x > _halfMapWidth ) return false;
	if ( y < -_scale || y > _scale ) return false;

	x = x / (qreal)_halfMapWidth + _visibleCenter.x();
	y = y / (qreal)_scale + _visibleCenter.y();

	x *= 180.0;
	y *= 90.0;

	if ( x < -180.0 ) x += 360.0;
	if ( x >  180.0 ) x -= 360.0;

	geoCoords.setX(x);
	geoCoords.setY(y);

	return true;
}


void RectangularProjection::centerOn(const QPointF &geoCoords) {
	qreal x = geoCoords.x() * ooLon;
	qreal y = geoCoords.y() * ooLat;

	if ( x < -1.0 ) x += 2.0;
	if ( x >  1.0 ) x -= 2.0;

	if ( y < -1.0 ) y = -1.0;
	if ( y >  1.0 ) y =  1.0;

	_center = QPointF(x,y);
	_visibleCenter = _center;
}


int RectangularProjection::lineSteps(const QPointF &p0, const QPointF &p1) {
	// Calculate the distance between p0 and p1 in pixels and
	// divide its manhattanLength by 20
	double dist, azi1, azi2;
	Math::Geo::delazi(p0.y(), p0.x(), p1.y(), p1.x(), &dist, &azi1, &azi2);

	if ( azi1 > 359.0 && azi1 < 1.0 && azi1 > 179.0 && azi1 < 180.0 )
		return 1;

	//return dist * pixelPerDegree() * (1.0 / 20.0);
	return 20;
}


void RectangularProjection::drawImage(QImage &buffer, const QRectF &geoReference,
                                      const QImage &image, bool highQuality) {
	if ( image.format() != QImage::Format_RGB32 &&
	     image.format() != QImage::Format_ARGB32 )
		return;

	bool useAlpha = image.format() == QImage::Format_ARGB32;

	QPoint p00, p11;

	qreal minLat, maxLat;
	qreal minLon, maxLon;

	minLat = geoReference.top();
	maxLat = geoReference.bottom();

	minLon = geoReference.left();
	maxLon = geoReference.right();

	if ( minLat > maxLat ) std::swap(minLat, maxLat);

	project(p00, QPointF(minLon, minLat));
	project(p11, QPointF(maxLon, maxLat));

	bool wrap = fabs(maxLon - minLon) >= 360.;

	int x0 = p00.x();
	int x1 = p11.x();

	int y0 = p00.y();
	int y1 = p11.y();


	// X can be wrapped, so we have to check more cases
	if ( geoReference.width() < 180.0f ) {
		if ( x0 >= _width ) {
			if ( x1 < 0 || x1 >= _width ) return;
		}

		if ( x1 < 0 ) {
			if ( x0 < 0 || x0 >= _width ) return;
		}
	}

	if ( y0 > y1 ) std::swap(y0,y1);

	// Y has no wrapping, so the checks are more simply
	if ( y0 >= _height ) return;
	if ( y1 < 0 ) return;

	bool drawTwoParts = false;

	// Quick hack just for testing
	// TODO: This special case has to be handled.
	if ( x0 >= x1 || wrap ) {
		drawTwoParts = true;
		if ( x0 >= x1 ) x0 -= _mapWidth;
		else if ( wrap ) x0 = x1 - _mapWidth;
	}

	int scaledWidth = x1-x0+1;
	int scaledHeight = y1-y0+1;

	Coord ratioX, ratioY;

	ratioX.parts.hi = image.width();
	ratioX.parts.lo = 0;
	ratioY.parts.hi = image.height();
	ratioY.parts.lo = 0;

	ratioX.value /= scaledWidth;
	ratioY.value /= scaledHeight;

	while ( true ) {
		int width = image.width();
		int height = image.height();

		Coord xofs, yofs;

		int x0c = x0,
		    y0c = y0,
		    x1c = x1;

		// Something has to be painted
		const QRgb *data = (const QRgb*)image.bits();
	
		QRgb *targetData = (QRgb*)buffer.bits();
		int targetWidth = buffer.width();

		if ( x0c < 0 ) {
			xofs.value = ratioX.value * -x0c;
			x0c = 0;
		}
		else
			xofs.value = 0;
	
		if ( x1c >= _width )
			x1c = _width-1;
	
		if ( y0c < 0 ) {
			yofs.value = ratioY.value * -y0c;
			height -= yofs.parts.hi;
			data += image.width() * yofs.parts.hi;
			y0c = 0;
		}
		else
			yofs.value = 0;
	
		if ( y1 >= _height )
			y1 = _height-1;
	
		targetData += targetWidth * y0c + x0c;
	
		Coord y;
		y.parts.hi = 0;
		y.parts.lo = yofs.parts.lo;

		if ( useAlpha ) {

			if ( highQuality ) {

				for ( int i = y0c; i <= y1; ++i ) {
					QRgb *targetPixel = targetData;
		
					Coord x;
					x.value = xofs.value;
			
					for ( int j = x0c; j <= x1c; ++j ) {
						QRgb c;
						getTexelBilinear(c, data, width, height, x, y);
						int alpha = qAlpha(c);
						int iAlpha = 255 - alpha;
						*targetPixel = qRgb(
							(qRed(c)*alpha + qRed(*targetPixel)*iAlpha) >> 8,
							(qGreen(c)*alpha + qGreen(*targetPixel)*iAlpha) >> 8,
							(qBlue(c)*alpha + qBlue(*targetPixel)*iAlpha) >> 8
						);
						++targetPixel;
		
						x.value += ratioX.value;
					}
		
					targetData += targetWidth;
		
					y.value += ratioY.value;
					int skipLines = y.parts.hi;
					height -= skipLines;
					while ( skipLines ) {
						data += width;
						--skipLines;
					}
					y.parts.hi = 0;
				}

			}
			else {

				for ( int i = y0c; i <= y1; ++i ) {
					QRgb *targetPixel = targetData;
		
					Coord x;
					x.value = xofs.value;
			
					for ( int j = x0c; j <= x1c; ++j ) {
						QRgb c = data[x.parts.hi];
						int alpha = qAlpha(c);
						int iAlpha = 255 - alpha;
						*targetPixel = qRgb(
							(qRed(c)*alpha + qRed(*targetPixel)*iAlpha) >> 8,
							(qGreen(c)*alpha + qGreen(*targetPixel)*iAlpha) >> 8,
							(qBlue(c)*alpha + qBlue(*targetPixel)*iAlpha) >> 8
						);
						++targetPixel;
		
						x.value += ratioX.value;
					}
		
					targetData += targetWidth;
		
					y.value += ratioY.value;
					int skipLines = y.parts.hi;
					while ( skipLines ) {
						data += width;
						--skipLines;
					}
					y.parts.hi = 0;
				}

			}

		}
		else {

			if ( highQuality ) {

				for ( int i = y0c; i <= y1; ++i ) {
					QRgb *targetPixel = targetData;
		
					Coord x;
					x.value = xofs.value;

					for ( int j = x0c; j <= x1c; ++j ) {
						getTexelBilinear(*targetPixel, data, width, height, x, y);
						++targetPixel;
		
						x.value += ratioX.value;
					}
		
					targetData += targetWidth;
		
					y.value += ratioY.value;
					int skipLines = y.parts.hi;
					height -= skipLines;
					while ( skipLines ) {
						data += width;
						--skipLines;
					}
					y.parts.hi = 0;
				}

			}
			else {

				for ( int i = y0c; i <= y1; ++i ) {
					QRgb *targetPixel = targetData;
		
					Coord x;
					x.value = xofs.value;
			
					for ( int j = x0c; j <= x1c; ++j ) {
						*targetPixel = data[x.parts.hi];
						++targetPixel;
		
						x.value += ratioX.value;
					}
		
					targetData += targetWidth;
		
					y.value += ratioY.value;
					int skipLines = y.parts.hi;
					while ( skipLines ) {
						data += width;
						--skipLines;
					}
					y.parts.hi = 0;
				}

			}

		}

		if ( drawTwoParts ) {
			x0 += _mapWidth;
			x1 += _mapWidth;
			drawTwoParts = false;
		}
		else
			break;
	}
}


bool RectangularProjection::drawLine(QPainter &p, const QPointF &from, const QPointF &to) {
	QPoint x0, x1;
	bool x0Visible, x1Visible;

	x0Visible = project(x0, from);
	x1Visible = project(x1, to);

	if ( !x0Visible || !x1Visible )
		return false;

	qreal degx0 = fmod(from.x(), 360.0);
	qreal degx1 = fmod(to.x(), 360.0);

	int dir = x1.x() - x0.x();
	qreal degdir = degx1 - degx0;
	if ( degdir > 180 )
		degdir -= 360;
	if ( degdir < -180 )
		degdir += 360;

	if ( (dir * degdir) < 0 ) {
		if ( x1.x() > x0.x() ) {
			int leftX = _halfWidth - _halfMapWidth;
			int leftY = x0.y() + ((leftX - x0.x()) * (x1.y() - x0.y())) / ((x1.x() - _mapWidth) - x0.x());

			p.drawLine(x0, QPoint(leftX, leftY));
			p.drawLine(QPoint(leftX + _mapWidth, leftY), x1);
		}
		else {
			int rightX = _halfWidth + _halfMapWidth;
			int rightY = x0.y() + ((rightX - x0.x()) * (x1.y() - x0.y())) / ((x1.x() + _mapWidth) - x0.x());

			p.drawLine(x0, QPoint(rightX, rightY));
			p.drawLine(QPoint(rightX - _mapWidth, rightY), x1);
		}
	}
	else
		p.drawLine(x0, x1);

	return true;
}


void RectangularProjection::moveTo(const QPointF &p) {
	Projection::moveTo(p);
	_cursorLon = fmod(p.x(), 360.0);
}


bool RectangularProjection::lineTo(QPainter &p, const QPointF &to) {
	QPoint x1;
	bool x1Visible;

	x1Visible = project(x1, to);

	qreal degx1 = fmod(to.x(), 360.0);

	if ( !_cursorVisible || !x1Visible ) {
		_cursorLon = degx1;
		_cursor = x1;
		_cursorVisible = x1Visible;
		return false;
	}

	int dir = x1.x() - _cursor.x();
	qreal degdir = degx1 - _cursorLon;
	if ( degdir > 180 )
		degdir -= 360;
	if ( degdir < -180 )
		degdir += 360;

	QPoint &x0 = _cursor;

	if ( (dir * degdir) < 0 ) {
		if ( x1.x() > x0.x() ) {
			int leftX = _halfWidth - _halfMapWidth;
			int leftY = x0.y() + ((leftX - x0.x()) * (x1.y() - x0.y())) / ((x1.x() - _mapWidth) - x0.x());

			p.drawLine(x0, QPoint(leftX, leftY));
			p.drawLine(QPoint(leftX + _mapWidth, leftY), x1);
		}
		else {
			int rightX = _halfWidth + _halfMapWidth;
			int rightY = x0.y() + ((rightX - x0.x()) * (x1.y() - x0.y())) / ((x1.x() + _mapWidth) - x0.x());

			p.drawLine(x0, QPoint(rightX, rightY));
			p.drawLine(QPoint(rightX - _mapWidth, rightY), x1);
		}
	}
	else
		p.drawLine(x0, x1);

	_cursorLon = degx1;
	_cursor = x1;
	_cursorVisible = x1Visible;

	return true;
}


bool RectangularProjection::drawLatCircle(QPainter &p, qreal lon) {
	QPoint pp;

	// Round to nearest integer since the text label is without decimals
	lon = round(lon);

	if ( project(pp, QPointF(lon, 90)) ) {
		if ( pp.x() >= 0 && pp.x() < _width ) {
			int top = std::max(0, pp.y());
			int bottom = std::min(_height-1, pp.y() + (int)_halfMapWidth);

			p.drawLine(pp.x(), top, pp.x(), bottom);
			p.drawText(QRect((int)pp.x() + 2, top, _width, _height), Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, lon2String(lon));

			return true;
		}
	}

	return false;
}


bool RectangularProjection::drawLonCircle(QPainter &p, qreal lat) {
	QPoint pp;

	// Round to nearest integer since the text label is without decimals
	lat = round(lat);

	if ( project(pp, QPointF(0, lat)) ) {
		if ( pp.y() >= 0 && pp.y() < _height ) {
			int left = std::max(0,_halfWidth - (int)(_scale*2));
			int right = std::min(_width-1, _halfWidth + (int)(_scale*2));

			p.drawLine(left, pp.y(), right, pp.y());
			p.drawText(QRect(left, pp.y(), _width, _height), Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, lat2String(lat));

			return true;
		}
	}

	return false;
}


}
}
}
