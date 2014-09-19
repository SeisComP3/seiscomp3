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


#include "mvmapwidget.h"


using namespace Seiscomp;


MvMapWidget::MvMapWidget(const Seiscomp::Gui::MapsDesc &maps,
                         QWidget* parent, Qt::WFlags f)
 : Gui::MapWidget(maps, parent, f),
   _mapLegend(new Legend) {
	setMouseTracking(true);
}



void MvMapWidget::draw(QPainter& painter) {
	Gui::MapWidget::draw(painter);
	_mapLegend->draw(&canvas(), painter);
}




ApplicationStatus::Mode MvMapWidget::mode() const {
	return _mapLegend->mode();
}




void MvMapWidget::setMode(ApplicationStatus::Mode mode) {
	_mapLegend->setMode(mode);
}




void MvMapWidget::showMapLegend(bool val) {
	_mapLegend->setVisible(val);
}
