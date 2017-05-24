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


#ifndef __SEISCOMP_GUI_ORIGIN_H__
#define __SEISCOMP_GUI_ORIGIN_H__


#include <vector>
#include <string>

#include <QPolygon>
#include <QColor>
#include <QPoint>
#include <QPainter>

#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/mapsymbol.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/origin.h>
#endif


namespace Seiscomp {
namespace Gui {


class Canvas;


class SC_GUI_API OriginSymbol : public Map::Symbol {
	public:
		OriginSymbol(Map::Decorator* decorator = NULL);
		OriginSymbol(double latitude,
		             double longitude,
		             double depth = 0,
		             Map::Decorator* decorator = NULL);

	public:
		void setPreferredMagnitudeValue(double magnitudeValue);
		double preferredMagnitudeValue() const;

		double depth() const;
		void setDepth(double depth);

		void setFilled(bool val);
		bool isFilled() const;

		virtual bool isInside(int x, int y) const;

	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);

		void init();
		void updateSize();
		void depthColorCoding();

	protected:
		Seiscomp::DataModel::Origin *_origin;
		QColor                       _color;
		QPolygon                     _poly;
		bool                         _filled;
		int                          _defaultSize;
		double                       _preferredMagnitudeValue;
		double                       _depth;
};


} // namespace Gui
} // namespace Seiscomp


#endif
