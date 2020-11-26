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


#include <seiscomp3/config/config.h>
#include <seiscomp3/gui/datamodel/eventlayer.h>

#include "mvmapwidget.h"


using namespace Seiscomp;


namespace {


class DummyEventLayer : public Gui::Map::Layer {
	public:
		DummyEventLayer(QObject *parent) : Gui::Map::Layer(parent) {
			_eventLegend = new Gui::EventLegend(this);
			_mapLegend = new Legend(this);

			addLegend(_eventLegend);
			addLegend(_mapLegend);
		}

		virtual void init(const Seiscomp::Config::Config &cfg) {
			try {
				std::string pos = cfg.getString("eventLegendPosition");
				if ( pos == "topleft" )
					_eventLegend->setArea(Qt::AlignLeft | Qt::AlignTop);
				else if ( pos == "topright" )
					_eventLegend->setArea(Qt::AlignRight | Qt::AlignTop);
				else if ( pos == "bottomright" )
					_eventLegend->setArea(Qt::AlignRight | Qt::AlignBottom);
				else if ( pos == "bottomleft" )
					_eventLegend->setArea(Qt::AlignLeft | Qt::AlignBottom);
			}
			catch ( ... ) {}

			try {
				std::string pos = cfg.getString("mapLegendPosition");
				if ( pos == "topleft" )
					_mapLegend->setArea(Qt::AlignLeft | Qt::AlignTop);
				else if ( pos == "topright" )
					_mapLegend->setArea(Qt::AlignRight | Qt::AlignTop);
				else if ( pos == "bottomright" )
					_mapLegend->setArea(Qt::AlignRight | Qt::AlignBottom);
				else if ( pos == "bottomleft" )
					_mapLegend->setArea(Qt::AlignLeft | Qt::AlignBottom);
			}
			catch ( ... ) {}
		}

		Legend *mapLegend() const {
			return _mapLegend;
		}


	private:
		Gui::EventLegend *_eventLegend;
		Legend           *_mapLegend;
};


}



MvMapWidget::MvMapWidget(const Seiscomp::Gui::MapsDesc &maps,
                         QWidget* parent, Qt::WindowFlags f)
: Gui::MapWidget(maps, parent, f) {
	setMouseTracking(true);

	DummyEventLayer *layer = new DummyEventLayer(this);
	canvas().addLayer(layer);

	_mapLegend = layer->mapLegend();

	connect(QCParameter::Instance(), SIGNAL(parameterChanged(int)),
	        _mapLegend, SLOT(setParameter(int)));
}



ApplicationStatus::Mode MvMapWidget::mode() const {
	return _mapLegend->mode();
}



void MvMapWidget::setMode(ApplicationStatus::Mode mode) {
	_mapLegend->setMode(mode);
}



void MvMapWidget::showMapLegend(bool val) {
	canvas().setDrawLegends(val);
	update();
}
