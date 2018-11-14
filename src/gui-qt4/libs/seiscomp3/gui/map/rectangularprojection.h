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



#ifndef __SEISCOMP_GUI_MAP_RECTANGULAR_PROJECTION_H__
#define __SEISCOMP_GUI_MAP_RECTANGULAR_PROJECTION_H__


#include <QImage>
#include <seiscomp3/gui/map/mapwidget.h>
#include <seiscomp3/gui/map/projection.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


class SC_GUI_API RectangularProjection : public Projection {
	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	public:
		RectangularProjection();


	// ----------------------------------------------------------------------
	// Public projection interface
	// ----------------------------------------------------------------------
	public:
		void setLowZoomEnabled(bool);

		virtual bool isRectangular() const;
		virtual bool wantsGridAntialiasing() const;

		virtual bool project(QPoint &screenCoords, const QPointF &geoCoords) const;
		virtual bool unproject(QPointF &geoCoords, const QPoint &screenCoords) const;

		virtual void centerOn(const QPointF &geoCoords);

		virtual int lineSteps(const QPointF &p0, const QPointF &p1);

		virtual void drawImage(QImage &buffer, const QRectF &geoReference,
		                       const QImage &image, bool highQuality,
		                       CompositionMode cm);

		virtual bool drawLine(QPainter &p, const QPointF &from, const QPointF &to);
		virtual void moveTo(const QPointF &p);
		virtual bool lineTo(QPainter &p, const QPointF &to);

		virtual bool drawLatCircle(QPainter &p, qreal lon);
		virtual bool drawLonCircle(QPainter &p, qreal lat);

		virtual bool project(QPainterPath &screenPath, size_t n,
		                     const Geo::GeoCoordinate *poly, bool closed,
		                     uint minPixelDist, ClipHint hint = NoClip) const;


	protected:
		template <typename PROC>
		void render(QImage &img, TextureCache *cache);

		void render(QImage &img, bool highQuality, TextureCache *cache);

		void projectUnwrapped(QPoint &screenCoords, const QPointF &geoCoords) const;


	// ----------------------------------------------------------------------
	// Protected member
	// ----------------------------------------------------------------------
	protected:
		bool  _enableLowZoom;


	// ----------------------------------------------------------------------
	// Private member
	// ----------------------------------------------------------------------
	private:
		qreal _cursorLon;
};


}
}
}


#endif
