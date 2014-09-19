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



#ifndef __SEISCOMP_GUI_MAP_PROJECTION_H__
#define __SEISCOMP_GUI_MAP_PROJECTION_H__


#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/gui/qt4.h>
#include <QImage>


namespace Seiscomp {
namespace Gui {
namespace Map {


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

		virtual void drawImage(QImage &buffer, const QRectF &geoReference, const QImage &image, bool highQuality);

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


}
}
}


#endif
