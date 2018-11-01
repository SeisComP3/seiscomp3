/***************************************************************************
 *   Copyright (C) by gempa GmbH and GFZ Potsdam                           *
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
#include <seiscomp3/geo/boundingbox.h>
#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/gui/qt4.h>

#include <QPainterPath>
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


/**
 * @brief The FilterMode enum define filter modes when drawing images and
 *        map tiles.
 */
enum FilterMode {
	FilterMode_Auto,     // Let the canvas decide how to filter the image
	FilterMode_Nearest,  // The nearest image pixel for a subpixel is used
	FilterMode_Bilinear  // The four neighbor pixels are interpolated for a subpixel
};


enum ClipHint {
	NoClip, // Clipping not necessary, bounding box is completely contained
	DoClip  // Clipping would help to reduce render time
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

		/**
		 * @brief Updates the visible map bounding box. This function will be
		 *        called *after* rendering, so after calling
		 *        @render(QImage &, bool, TextureCache *).
		 */
		virtual void updateBoundingBox();

		/**
		 * @brief Returns the current visible map bounding box. Note that this
		 *        bounding box will only be valid after the projection has
		 *        been rendered otherwise it will be in an undefined state.
		 * @return The bounding box currently visible.
		 */
		const Geo::GeoBoundingBox &boundingBox() const;

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
		 * Returns true if the GUI object, represented by its bounding box in geo
		 * coordinates, does not intersect the canvas. Note: The default
		 * implementation checks clipping against the map bounding box which is
		 * perfectly OK in all situations given the bounding box is computed
		 * correctly. If more complex checks needs to be implemented, then
		 * override this method. The default implementation just calls
		 * @isClipped(const Geo::GeoBoundingBox &).
		 *
		 * Warning: This method is deprecated. In future this virtual method
		 * will be removed and the only clipping check will be against the map
		 * bounding box. There is no need to delegate clipping to custom
		 * projections as long as a correct bounding box is maintained.
		 */
		virtual bool isClipped(const QPointF &bboxLR, const QPointF &bboxUL) const;

		bool isClipped(const Geo::GeoBoundingBox &bbox) const;

		/**
		 * @brief Projects a closed input geo polygon to a QPainterPath
		 * @param screenPath The painter path in screen coordinates. That
		 *                   path must represent the input polygon on screen.
		 * @param n The number of input coordinates
		 * @param poly The coordinates of the polygon. The polygon is assumed
		 *             to be closed.
		 * @param minPixelDist The minimum pixel distance two coordinates must
		 *                     have to become two separate projections on screen.
		 *                     If their distance is less then the coordinates
		 *                     will collapse into a single pixel.
		 * @return A flag that defines wether the returned path is valid.
		 */
		virtual bool project(QPainterPath &screenPath, size_t n,
		                     const Geo::GeoCoordinate *poly, bool closed,
		                     uint minPixelDist, ClipHint hint = NoClip) const;


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

		Geo::GeoBoundingBox _mapBoundingBox;
};


DEFINE_INTERFACE_FACTORY(Projection);


inline const Geo::GeoBoundingBox &Projection::boundingBox() const {
	return _mapBoundingBox;
}


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
