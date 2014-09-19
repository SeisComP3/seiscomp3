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
	// X'struction
	// ----------------------------------------------------------------------
	public:
		RectangularProjection();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void setLowZoomEnabled(bool);

		bool isRectangular() const;
		bool wantsGridAntialiasing() const;

		bool project(QPoint &screenCoords, const QPointF &geoCoords) const;
		bool unproject(QPointF &geoCoords, const QPoint &screenCoords) const;

		void centerOn(const QPointF &geoCoords);

		int  lineSteps(const QPointF &p0, const QPointF &p1);

		void drawImage(QImage &buffer, const QRectF &geoReference, const QImage &image, bool highQuality);

		bool drawLine(QPainter &p, const QPointF &from, const QPointF &to);
		void moveTo(const QPointF &p);
		bool lineTo(QPainter &p, const QPointF &to);

		bool drawLatCircle(QPainter &p, qreal lon);
		bool drawLonCircle(QPainter &p, qreal lat);


	protected:
		template <typename PROC>
		void render(QImage &img, TextureCache *cache);

		void render(QImage &img, bool highQuality, TextureCache *cache);


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
