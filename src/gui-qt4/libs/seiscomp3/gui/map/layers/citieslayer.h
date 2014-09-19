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

#ifndef __SEISCOMP_GUI_MAP_LAYERS_CITIESLAYER_H__
#define __SEISCOMP_GUI_MAP_LAYERS_CITIESLAYER_H__

#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/layer.h>
#include <seiscomp3/math/coord.h>

namespace Seiscomp {
namespace Gui {
namespace Map {

class Canvas;
class Projection;

class SC_GUI_API CitiesLayer : public Layer {
	public:
		CitiesLayer(QObject* = NULL);
		virtual ~CitiesLayer();

		virtual void draw(const Canvas*, QPainter&);

		void setSelectedCity(const Math::Geo::CityD*);
		const Math::Geo::CityD* selectedCity() const;

	private:
		typedef QList<QRect> Row;
		typedef QVector<Row> Grid;

	private:
		void drawCity(QPainter&, Grid&, QFont&, bool&, bool&,
                      const Projection*, const Math::Geo::CityD&,
                      const QFontMetrics&, int, int);

	private:
		const Math::Geo::CityD*         _selectedCity;
};

} // namespace Map
} // namespce Gui
} // namespace Seiscomp

#endif
