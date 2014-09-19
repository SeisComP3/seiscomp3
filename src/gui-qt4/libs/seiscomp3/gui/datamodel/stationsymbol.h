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
	DECLARE_RTTI;

	public:
		StationSymbol(Map::Decorator* decorator = NULL);
		StationSymbol(double latitude,
		              double longitude,
		              Map::Decorator* decorator = NULL);

		virtual bool isInside(int x, int y) const;
		virtual bool isClipped(const Map::Canvas *canvas) const;
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual bool hasValidMapPosition() const;

		void setColor(const QColor& color);
		const QColor& color() const;

		void setFrameColor(const QColor& color);
		const QColor& frameColor() const;

		void setFrameSize(int);
		int frameSize() const;

		void setSize(int size);
		int size() const;

		int x() const;
		void setX(int x);

		int y() const;
		void setY(int y);

		double latitude() const;
		void setLatitude(double latitude);

		double longitude() const;
		void setLongitude(double longitude);


	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);
		const QPolygon& drawStationSymbol(QPainter& painter);
		QPolygon generateShape(int posX, int posY, int radius);

		const QPolygon& stationPolygon() const;

	private:
		void init();


	private:
		int                 _size;
		int                 _frameSize;
		QPolygon            _stationPolygon;

		int                 _xPos;
		int                 _yPos;
		double              _latitude;
		double              _longitude;

		QColor               _frameColor;
		QColor               _color;
};

} // namespace Gui
} // namespace Seiscomp

#endif
