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



#ifndef __SEISCOMP_GUI_STATIONSYMBOL_H__
#define __SEISCOMP_GUI_STATIONSYMBOL_H__

#include <iostream>
#include <string>
#include <vector>

#include <QColor>

#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/gui/map/mapsymbol.h>
#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/math/coord.h>


// Forward declaration
class QPainter;


namespace Seiscomp {
namespace Gui {


class SC_GUI_API StationSymbol : public Map::Symbol {
	public:
		StationSymbol(Map::Decorator* decorator = NULL);
		StationSymbol(double latitude,
		              double longitude,
		              Map::Decorator* decorator = NULL);

		void setColor(const QColor& color);
		const QColor& color() const;

		void setRadius(int radius);
		int radius() const;

		void setFrameColor(const QColor& color);
		const QColor& frameColor() const;

		void setFrameSize(int);
		int frameSize() const;

		virtual bool isInside(int x, int y) const;
		virtual void customDraw(const Map::Canvas *canvas, QPainter &painter);

	protected:
		QPolygon generateShape(int posX, int posY, int radius);

		const QPolygon &stationPolygon() const;


	private:
		void init();


	private:
		int       _radius;
		int       _frameSize;
		QPolygon  _stationPolygon;

		QColor    _frameColor;
		QColor    _color;
};


} // namespace Gui
} // namespace Seiscomp


#endif
