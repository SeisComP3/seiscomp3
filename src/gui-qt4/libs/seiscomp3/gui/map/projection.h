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
 *                                                                         *
 *   Author: Jan Becker, gempa GmbH, jabe@gempa.de                         *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_MAP_PROJECTION_H__
#define __SEISCOMP_GUI_MAP_PROJECTION_H__


#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/gui/qt4.h>
#include <QImage>


namespace Seiscomp {
namespace Gui {
namespace Map {


/**
 * @brief The CompositionMode enum define composition modes when
 *        drawing an image.
 */
enum CompositionMode {
	CompositionMode_Default,     // Either Source or SourceOver depending on image format.
	CompositionMode_Source,      // Image pixels replace map pixels.
	CompositionMode_SourceOver,  // Image pixels are blended with map according
	                             // to the image pixels alpha value.
	CompositionMode_Xor,         // Colors are bitwise xor'd
	CompositionMode_Plus,        // Colors are added together
	CompositionMode_Multiply     // Image pixels are multiplied with map pixels, alpha
	                             // is ignored.
};


// All QPointF instances are geographical coordinates where
// x = longitude and y = latitude.
class SC_GUI_API Projection {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		Projection();
		virtual ~Projection();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void setBackgroundColor(const QColor &c);

		void setZoom(qreal zoom);
		qreal zoom() const;
		qreal visibleZoom() const;

		qreal pixelPerDegree() const;
		virtual QPointF gridDistance() const;

		void setView(const QPointF &geoCoords, qreal zoom);
		QPointF center() const;
		QPointF visibleCenter() const;

		void draw(QImage &buffer, bool filter, TextureCache *cache);

		virtual bool isRectangular() const = 0;
		virtual bool wantsGridAntialiasing() const = 0;

		virtual bool project(QPoint &screenCoords, const QPointF &geoCoords) const = 0;
		virtual bool unproject(QPointF &geoCoords, const QPoint &screenCoords) const = 0;
	
		virtual void centerOn(const QPointF &geoCoords) = 0;
		virtual void displayRect(const QRectF& rect);

		virtual void drawImage(QImage &buffer, const QRectF &geoReference, const QImage &image, bool highQuality,
		                       CompositionMode compositionMode = CompositionMode_Default);

		//! Returns the number of interpolation steps when drawing a
		//! geometric line.
		virtual int  lineSteps(const QPointF &p0, const QPointF &p1);

		//! Draws a straight line. Returns false, when the whole line has been clipped
		//! completely otherwise true.
		virtual bool drawLine(QPainter &p, const QPointF &from, const QPointF &to);

		virtual void moveTo(const QPointF &p);
		virtual bool lineTo(QPainter &p, const QPointF &to);

		virtual bool drawLatCircle(QPainter &p, qreal lon);
		virtual bool drawLonCircle(QPainter &p, qreal lat);
		
		/**
		 * Returns true if the GUI object, represented by its bounding
		 * box in geo coordinates, does not intersect the canvas.
		 */
		virtual bool isClipped(const QPointF &bboxLR, const QPointF &bboxUL) const;


	protected:
		void setSize(int width, int height);
		void setVisibleRadius(qreal r);

		virtual void render(QImage &img, bool highQuality, TextureCache *cache) = 0;


	// ----------------------------------------------------------------------
	// Protected member
	// ----------------------------------------------------------------------
	protected:
		qreal   _radius;
		qreal   _visibleRadius;
		qreal   _pixelPerDegreeFact;

		int     _width;
		int     _height;

		int     _halfWidth;
		int     _halfHeight;
		qreal   _screenRadius;
		qreal   _mapWidth;
		qreal   _halfMapWidth;

		qreal   _scale;
		qreal   _oneOverScale;

		QPointF _center;
		QPointF _visibleCenter;

		QPoint  _cursor;
		bool    _cursorVisible;

		QRgb    _background;
};


DEFINE_INTERFACE_FACTORY(Projection);

#define REGISTER_PROJECTION_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Gui::Map::Projection, Class> __##Class##InterfaceFactory__(Service)


struct NearestFilter {
	static void fetch(TextureCache *cache, QRgb &c, Coord u, Coord v, int level) {
		cache->getTexel(c,u,v,level);
	}
};


struct BilinearFilter {
	static void fetch(TextureCache *cache, QRgb &c, Coord u, Coord v, int level) {
		cache->getTexelBilinear(c,u,v,level);
	}
};


struct CompositionSourceOver {
	static void combine(QRgb &target, QRgb source) {
		int alpha = qAlpha(source);
		int iAlpha = 255 - alpha;
		target = qRgb(
			(qRed(source)*alpha + qRed(target)*iAlpha) >> 8,
			(qGreen(source)*alpha + qGreen(target)*iAlpha) >> 8,
			(qBlue(source)*alpha + qBlue(target)*iAlpha) >> 8
		);
	}
};


struct CompositionSource {
	static void combine(QRgb &target, QRgb source) {
		target = source;
	}
};


struct CompositionMultiply {
	static void combine(QRgb &target, QRgb source) {
		target = qRgb(
			(qRed(source) * qRed(target)) >> 8,
			(qGreen(source) * qGreen(target)) >> 8,
			(qBlue(source) * qBlue(target)) >> 8
		);
	}
};


struct CompositionXor {
	static void combine(QRgb &target, QRgb source) {
		target = qRgb(
			(qRed(source) xor qRed(target)),
			(qGreen(source) xor qGreen(target)),
			(qBlue(source) xor qBlue(target))
		);
	}
};


struct CompositionPlus {
	static void combine(QRgb &target, QRgb source) {
		int r = qRed(source) + qRed(target);
		int g = qGreen(source) + qGreen(target);
		int b = qBlue(source) + qBlue(target);
		target = qRgb(r > 255 ? 255 : r, g > 255 ? 255 : g, b > 255 ? 255 : b);
	}
};


}
}
}


#endif
