/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/gui/map/layers/citieslayer.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/gui/map/standardlegend.h>


namespace Seiscomp {
namespace Gui {
namespace Map {

#define CITY_NORMAL_SYMBOL_SIZE 4
#define CITY_BIG_SYMBOL_SIZE    6
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CitiesLayer::CitiesLayer(QObject* parent) : Layer(parent), _selectedCity(NULL) {
	setName("cities");

	StandardLegend *legend = new StandardLegend(this);
	legend->setTitle(tr("Cities"));
	legend->setArea(Qt::Alignment(Qt::AlignTop | Qt::AlignRight));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityNormal, tr("1Mio- inhabitants"),
	                                       CITY_NORMAL_SYMBOL_SIZE));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityNormal, tr("1Mio+ inhabitants"),
	                                       CITY_BIG_SYMBOL_SIZE));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityCapital, tr("Capital"),
	                                       CITY_BIG_SYMBOL_SIZE));
	addLegend(legend);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CitiesLayer::~CitiesLayer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::draw(const Seiscomp::Gui::Map::Canvas* canvas,
                       QPainter& painter) {
	if ( !isVisible() ) return;
	if ( canvas == NULL ) return;

	Seiscomp::Gui::Map::Projection* projection = canvas->projection();
	if ( projection == NULL ) return;

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing, isAntiAliasingEnabled());

	QFont font(SCScheme.fonts.cityLabels);
	font.setBold(true);
	painter.setFont(font);

	QFontMetrics fontMetrics(font);

	int height = canvas->height(),
	    width = canvas->width(),
	    fontHeight = fontMetrics.height(),
	    gridHeight = height / fontHeight;

	Grid grid(gridHeight);

	double radius = Math::Geo::deg2km(width / projection->pixelPerDegree()) *
	                SCScheme.map.cityPopulationWeight;

	bool lastUnderline = false;
	bool lastBold = true;

	if ( _selectedCity )
		drawCity(painter, grid, font, lastUnderline, lastBold, projection,
		         *_selectedCity, fontMetrics, width, fontHeight);
	foreach ( const Math::Geo::CityD& city, SCCoreApp->cities() ) {
		if ( city.population() < radius ) break;
		if ( &city == _selectedCity ) continue;

		drawCity(painter, grid, font, lastUnderline, lastBold, projection,
		         city, fontMetrics, width, fontHeight);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::drawCity(QPainter& painter, Grid &grid, QFont &font,
                           bool &lastUnderline, bool &lastBold,
                           const Projection* projection,
                           const Math::Geo::CityD& city,
                           const QFontMetrics& fontMetrics,
                           int width, int rowHeight) {
	QPoint p;
	if ( !projection->project(p, QPointF(city.lon, city.lat)) )
		return;

	int gridY, gridPrevY, gridNextY;

	gridY = p.y() / rowHeight;

	if ( gridY < 0 || gridY >= grid.count() ) return;
	if ( p.x() < 0 || p.x() >= width ) return;

	QRect labelRect(fontMetrics.boundingRect(city.name().c_str()));

	bool capital = (city.category() == "B" || city.category() == "C");
	bool bold = city.population() >= 1000000;

	int symbolSize = 4;
	if ( bold )
		symbolSize = 6;

	labelRect.moveTo(QPoint(p.x()+symbolSize/2, p.y()));
	labelRect.setWidth(labelRect.width()+2);

	QList<QRect> &gridRow = grid[gridY];

	bool foundPlace = true;
	for ( Row::iterator it = gridRow.begin();
	      it != gridRow.end(); ++it ) {
		if ( it->intersects(labelRect) ) {
			foundPlace = false;
			break;
		}
	}

	if ( !foundPlace ) {
		labelRect.moveTo(labelRect.left() - labelRect.width() - symbolSize,
		                 labelRect.top());
		foundPlace = true;
		for ( Row::iterator it = gridRow.begin();
		      it != gridRow.end(); ++it ) {
			if ( it->intersects(labelRect) ) {
				foundPlace = false;
				break;
			}
		}
	}

	if ( !foundPlace ) return;

	gridY = labelRect.top() / rowHeight;

	gridPrevY = gridY - 1;
	gridNextY = gridY + 1;

	gridRow = grid[gridY];
	gridRow.append(labelRect);
	if ( gridPrevY >= 0 ) grid[gridPrevY].append(labelRect);
	if ( gridNextY < grid.count() ) grid[gridNextY].append(labelRect);

	if ( capital ) {
		painter.setPen(SCScheme.colors.map.cityOutlines);
		painter.setBrush(SCScheme.colors.map.cityCapital);
	}
	else {
		painter.setPen(SCScheme.colors.map.cityOutlines);
		painter.setBrush(SCScheme.colors.map.cityNormal);
	}
	painter.drawRect(p.x()-symbolSize/2, p.y()-symbolSize/2, symbolSize,
	                 symbolSize);

	painter.setPen(SCScheme.colors.map.cityLabels);

	if ( capital != lastUnderline ) {
		lastUnderline = capital;
		font.setUnderline(capital);
		painter.setFont(font);
	}

	if ( bold != lastBold ) {
		lastBold = bold;
		font.setBold(bold);
		painter.setFont(font);
	}

	painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignTop |
	                 Qt::TextSingleLine, city.name().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::setSelectedCity(const Math::Geo::CityD* c) {
	_selectedCity = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Math::Geo::CityD* CitiesLayer::selectedCity() const {
	return _selectedCity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
