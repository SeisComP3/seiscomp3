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

#ifndef __SEISCOMP_GUI_MAP_LAYERS_GEOFEATURELAYER_H__
#define __SEISCOMP_GUI_MAP_LAYERS_GEOFEATURELAYER_H__


#include <seiscomp3/gui/map/layer.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;
class Projection;


class SC_GUI_API GeoFeatureLayer : public Layer {
	public:
		GeoFeatureLayer(QObject *parent = NULL);
		virtual ~GeoFeatureLayer();

		virtual void draw(const Canvas *canvas, QPainter &painter);

	private:
		bool _initialized;
};


}
}
}


#endif
