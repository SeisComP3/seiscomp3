/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#ifndef __SEISCOMP_GUI_MAP_LAYERS_GRIDLAYER_H__
#define __SEISCOMP_GUI_MAP_LAYERS_GRIDLAYER_H__


#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/layer.h>
#include <seiscomp3/math/coord.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;


class SC_GUI_API GridLayer : public Layer {
	public:
		GridLayer(QObject* = NULL);
		virtual ~GridLayer();

		virtual void draw(const Canvas*, QPainter&);

		void setGridDistance(const QPointF&);
		const QPointF& gridDistance() const;
	private:
		QPointF                _gridDistance;
};


} // namespace Map
} // namespce Gui
} // namespace Seiscomp

#endif
