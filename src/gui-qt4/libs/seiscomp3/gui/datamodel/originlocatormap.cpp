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



#define SEISCOMP_COMPONENT Gui::OriginLocatorMap
#include "originlocatormap.h"
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/math/math.h>

#include <seiscomp3/gui/datamodel/advancedoriginsymbol.h>
#include <seiscomp3/gui/datamodel/ttdecorator.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/projection.h>

#include <QMenu>

#ifdef WIN32
#undef min
#undef max
#endif

using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorMap::OriginLocatorMap(const MapsDesc &maps,
                                   QWidget *parent, Qt::WindowFlags f)
: MapWidget(maps, parent, f)
, _origin(NULL), _drawStations(false)
, _drawStationsLines(true), _interactive(true)
{
	_originSymbol = NULL;
	_lastSymbolSize = 0;
	_waveformPropagation = false;
	_enabledCreateOrigin = false;
	_hoverId = -1;
	_stationsMaxDist = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorMap::OriginLocatorMap(Map::ImageTree* mapTree,
                                   QWidget *parent, Qt::WindowFlags f)
: MapWidget(mapTree, parent, f), _origin(NULL)
, _drawStations(false), _drawStationsLines(true)
, _interactive(true)
{
	_originSymbol = NULL;
	_lastSymbolSize = 0;
	_waveformPropagation = false;
	_enabledCreateOrigin = false;
	_hoverId = -1;
	_stationsMaxDist = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::drawCustomLayer(QPainter *painter) {
	if ( _origin ) {
		QPainter &p(*painter);
		p.save();

		QPointF originLocationF(_origin->longitude(), _origin->latitude());

		if ( _drawStations ) {
			if ( _drawStationsLines ) {
				int cutOff = 0;

				QPoint originLocation;
				if ( canvas().projection()->project(originLocation, originLocationF) ) {
					if ( canvas().symbolCollection()->count() > 0 )
						cutOff = (*canvas().symbolCollection()->begin())->size().width();

					if ( cutOff ) {
						p.setClipping(true);
						p.setClipRegion(QRegion(rect()) -
						                QRegion(QRect(originLocation.x() - cutOff/2, originLocation.y() - cutOff/2,
						                              cutOff, cutOff), QRegion::Ellipse));
					}
				}

				p.setPen(SCScheme.colors.map.lines);
				for ( QVector<StationEntry>::iterator it = _stations.begin(); it != _stations.end(); ++it ) {
					if ( !(*it).validLocation || !(*it).isArrival ) continue;
					if ( !(*it).isActive ) continue;
					canvas().drawLine(p, originLocationF, (*it).location);
				}

				if ( cutOff )
					p.setClipping(false);
			}

			p.setPen(SCScheme.colors.map.outlines);
			for ( int i = _stations.count()-1; i >= 0; --i ) {
				if ( !_stations[i].validLocation ) continue;
				if ( !_interactive && !_stations[i].isActive ) continue;
				p.setBrush(
					_stations[i].isArrival?
						_stations[i].isActive?_stations[i].color:SCScheme.colors.arrivals.disabled
					:
						_stations[i].isActive?SCScheme.colors.stations.idle:Qt::gray);
				QPoint pp;
				if ( canvas().projection()->project(pp, _stations[i].location) ) {
					int size = SCScheme.map.stationSize;
					p.drawEllipse(pp.x()-size/2, pp.y()-size/2, size, size);
				}
			}
		}


		/*
		#define STAR_POINTS 4
		#define STAR_RADIUS 9
		#define STAR_INNER_RADIUS 4

		static QPoint starPoints[STAR_POINTS*2];

		for ( int i = 0; i < STAR_POINTS; ++i ) {
			starPoints[i*2] = QPoint((int)(STAR_RADIUS*sin((i*(360.0/STAR_POINTS))*M_PI/180.0))+originLocation.x(),
			                         -(int)(STAR_RADIUS*cos((i*(360.0/STAR_POINTS))*M_PI/180.0))+originLocation.y());
			starPoints[i*2+1] = QPoint((int)(STAR_INNER_RADIUS*sin((i*360.0+180.0)/STAR_POINTS*M_PI/180.0))+originLocation.x(),
			                           -(int)(STAR_INNER_RADIUS*cos((i*360.0+180.0)/STAR_POINTS*M_PI/180.0))+originLocation.y());
		}
		*/

		/* Drawing of origin symbol will be handled by the MapWidget
		p.setPen(Qt::white);
		p.setBrush(Qt::magenta);
		//p.drawPolygon(starPoints, STAR_POINTS*2);
		p.drawEllipse(originLocation.x()-5, originLocation.y()-5, 10, 10);
		*/

		/*
		#undef STAR_POINTS
		#undef STAR_RADIUS
		*/

		p.restore();
	}

	/* --- DEBUG OUTPUT ---
	p.setPen(SCScheme.colors.map.lines);
	drawLine(p, QPointF(5, 50), QPointF(-1, 50));
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::contextMenuEvent(QContextMenuEvent *e) {
	QMenu menu(this);
	QAction *actionArtificialOrigin = NULL;

	if ( _enabledCreateOrigin ) {
		actionArtificialOrigin = menu.addAction("Create artificial origin");
		menu.addSeparator();
	}

	updateContextMenu(&menu);

	QAction *action = menu.exec(e->globalPos());
	if ( action == NULL ) return;

	if ( action == actionArtificialOrigin ) {
		QPointF epicenter;
		if ( canvas().projection()->unproject(epicenter, e->pos()) )
			emit artificialOriginRequested(epicenter, e->globalPos());
	}
	else
		executeContextMenuAction(action);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::mouseMoveEvent(QMouseEvent *event) {
	int hoverId = -1;
	int stationHalfSize = SCScheme.map.stationSize/2;
	for ( int i = 0; i < _stations.count(); ++i ) {
		QPoint pp;
		if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
		if ( abs(pp.x() - event->x()) <= stationHalfSize &&
		     abs(pp.y() - event->y()) <= stationHalfSize ) {
			hoverId = i;
			break;
		}
	}

	if ( hoverId != _hoverId ) {
		_hoverId = hoverId;
		if ( _hoverId != -1 ) {
			int arrivalId = _stations[_hoverId].arrivalId;
			hoverArrival(arrivalId);
			if ( toolTip().isEmpty()
			  && !_stations[_hoverId].net.empty()
			  && !_stations[_hoverId].code.empty() )
				setToolTip((_stations[_hoverId].net + "." + _stations[_hoverId].code).c_str());
		}
		else {
			hoverArrival(-1);
			setToolTip(QString());
		}
	}

	MapWidget::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::mouseDoubleClickEvent(QMouseEvent *event) {
	if ( (event->button() == Qt::LeftButton) && _drawStations && _interactive ) {
		if ( event->modifiers() == Qt::NoModifier ) {
			int stationHalfSize = SCScheme.map.stationSize/2;
			for ( int i = 0; i < _stations.count(); ++i ) {
				QPoint pp;
				if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
				if ( abs(pp.x() - event->x()) <= stationHalfSize &&
					abs(pp.y() - event->y()) <= stationHalfSize ) {
					if ( _stations[i].isArrival ) {
						setArrivalState(_stations[i].arrivalId, !_stations[i].isActive);
						emit arrivalChanged(_stations[i].arrivalId, _stations[i].isActive);
					}
					/*
					else {
						setStationState(i, !_stations[i].isActive);
						emit stationChanged(_stations[i].code, _stations[i].isActive);
					}
					*/
					return;
				}
			}
		}
	}

	MapWidget::mouseDoubleClickEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::mousePressEvent(QMouseEvent *event) {
	if ( (event->button() == Qt::LeftButton) && _drawStations && _interactive ) {
		if ( event->modifiers() == Qt::NoModifier ) {
			int stationHalfSize = SCScheme.map.stationSize/2;
			for ( int i = 0; i < _stations.count(); ++i ) {
				QPoint pp;
				if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
				if ( abs(pp.x() - event->x()) <= stationHalfSize &&
					abs(pp.y() - event->y()) <= stationHalfSize ) {
					if ( _stations[i].isArrival )
						emit clickedArrival(_stations[i].arrivalId);
					else
						emit clickedStation(_stations[i].net, _stations[i].code);
					return;
				}
			}
		}
	}
	else if ( event->button() == Qt::MidButton && _enabledCreateOrigin ) {
		QPointF epicenter;
		if ( canvas().projection()->unproject(epicenter, event->pos()) )
			emit artificialOriginRequested(epicenter, event->globalPos());
	}

	MapWidget::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationsMaxDist(double d) {
	_stationsMaxDist = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationsInteractive(bool e) {
	_interactive = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setOrigin(DataModel::Origin* o) {
	_origin = o;
	_originSymbol = NULL;
	canvas().symbolCollection()->clear();
	_arrivals.clear();
	_stations.clear();
	_stationCodes.clear();

	if ( !_origin ) return;

	TTDecorator *ttd = new TTDecorator();
	ttd->setLatitude(o->latitude());
	ttd->setLongitude(o->longitude());
	ttd->setOriginTime(o->time());
	ttd->setVisible(_waveformPropagation);

	try {
		ttd->setDepth(o->depth());
	} catch ( Core::ValueException& ) {}

	if ( o->magnitudeCount() > 0 ) {
		ttd->setPreferredMagnitudeValue(o->magnitude(0)->magnitude());
	}

	_originSymbol = new AdvancedOriginSymbol(o, ttd);

	// TTDecorator::ShowWaveformPropagation(_waveformPropagation);
	canvas().symbolCollection()->add(_originSymbol);

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		bool foundStation = false;
		QColor itemColor;
		Arrival* arrival = _origin->arrival(i);
		try {
			itemColor = SCScheme.colors.arrivals.residuals.colorAt(arrival->timeResidual());
		}
		catch ( ... ) {
			itemColor = SCScheme.colors.arrivals.undefined;
		}

		Pick* p = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( p ) {
			/*
			try {
				switch ( p->status() ) {
					case MANUAL_PICK:
						itemColor = SCScheme.colors.arrivals.manual;
						break;
					case AUTOMATIC_PICK:
						itemColor = SCScheme.colors.arrivals.automatic;
						break;
					default:
						itemColor = SCScheme.colors.arrivals.undefined;
						break;
				}
			}
			catch ( ... ) {
				itemColor = SCScheme.colors.arrivals.undefined;
			}
			*/

			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
					p->waveformID().networkCode(),
					p->waveformID().stationCode(),
					p->time()
				);

				std::string stationCode = p->waveformID().networkCode() + "." + p->waveformID().stationCode();
				_stations.push_back(StationEntry(QPointF(loc.longitude,loc.latitude),
				                                 p->waveformID().networkCode(),
				                                 p->waveformID().stationCode(),
				                                 true));
				_stationCodes[stationCode] = _stations.size()-1;
				addArrival();
				foundStation = true;

			}
			catch ( Core::ValueException& e ) {
				SEISCOMP_DEBUG("While fetching the station location an error occured: %s -> computing position", e.what());
				foundStation = false;
			}
		}
		else {
			SEISCOMP_DEBUG("pick for arrival not found -> setting arrival color [undefined]");
			//itemColor = SCScheme.colors.arrivals.undefined;
			foundStation = false;
		}

		if ( !foundStation ) {
			try {
				double lat, lon;
				Math::Geo::delandaz2coord(arrival->distance(), arrival->azimuth(),
				                    _origin->latitude(), _origin->longitude(),
				                    &lat, &lon);

				lon = fmod(lon+180.0,360.0);
				if ( lon < 0 ) lon += 360.0;
				lon -= 180.0;

				lat = fmod(lat+90.0,180.0);
				if ( lat < 0 ) lat += 180.0;
				lat -= 90.0;

				_stations.push_back(StationEntry(QPointF(lon,lat), "", "", true));
				addArrival();
			}
			catch ( ... ) {
				_stations.push_back(StationEntry(QPointF(0,0), "", "", false));
				addArrival();
			}
		}

		try {
			_stations.back().isActive = arrival->weight() > 0.0;
		}
		catch ( ... ) {
			_stations.back().isActive = true;
		}

		_stations.back().color = itemColor;
	}

	DataModel::Inventory *inv = Client::Inventory::Instance()->inventory();
	if ( inv ) {
		for ( size_t n = 0; n < inv->networkCount(); ++n ) {
			DataModel::Network *net = inv->network(n);
			if ( net->start() > _origin->time().value() )
				continue;
			try { if ( net->end() < _origin->time().value() ) continue; }
			catch ( ... ) {}

			for ( size_t s = 0; s < net->stationCount(); ++s ) {
				DataModel::Station *sta = net->station(s);
				if ( sta->start() > _origin->time().value() )
					continue;
				try { if ( sta->end() < _origin->time().value() ) continue; }
				catch ( ... ) {}

				double dist, azi1, azi2;
				try {
					Math::Geo::delazi(
						sta->latitude(), sta->longitude(),
						_origin->latitude(), _origin->longitude(),
						&dist, &azi1, &azi2);
				}
				catch ( ... ) {
					continue;
				}

				// Limit to 20 degrees
				if ( dist > _stationsMaxDist ) continue;

				std::string stationCode = net->code() + "." + sta->code();

				// Station already registered as arrival
				if ( _stationCodes.find(stationCode) != _stationCodes.end() )
					continue;

				_stations.push_back(
					StationEntry(
						QPointF(sta->longitude(),sta->latitude()),
						net->code(), sta->code(), true
					)
				);

				_stations.back().isActive = true;
				_stationCodes[stationCode] = _stations.size()-1;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::addArrival() {
	_stations.back().arrivalId = _arrivals.size();
	_stations.back().isArrival = true;
	_arrivals.push_back(_stations.size()-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setDrawStations(bool draw) {
	_drawStations = draw;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setDrawStationLines(bool draw) {
	if ( _drawStationsLines == draw ) return;

	_drawStationsLines = draw;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorMap::drawStations() const {
	return _drawStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setWaveformPropagation(bool p) {
	if ( _waveformPropagation == p ) return;
	_waveformPropagation = p;
	if ( _originSymbol && _originSymbol->decorator() ) {
		// TTDecorator::ShowWaveformPropagation(_waveformPropagation);
		_originSymbol->decorator()->setVisible(_waveformPropagation);
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorMap::waveformPropagation() const {
	return _waveformPropagation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setArrivalState(int id, bool state) {
	if ( id < 0 || id >= _arrivals.size() ) return;
	int stationId = _arrivals[id];
	setStationState(stationId, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationState(int i, bool state) {
	if ( _stations[i].isActive == state ) return;
	_stations[i].isActive = state;
	if ( _drawStations ) {
		QPoint p;
		if ( canvas().projection()->project(p, _stations[i].location) )
			//update(p.x()-5, p.y()-5, 10, 10);
			update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setOriginCreationEnabled(bool enable) {
	_enabledCreateOrigin = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationState(const std::string& code, bool state) {
	std::map<std::string, int>::iterator it = _stationCodes.find(code);
	if ( it == _stationCodes.end() ) return;
	setStationState((*it).second, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
