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



#define SEISCOMP_COMPONENT Gui::MagnitudeMap
#include "magnitudemap.h"
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/stationmagnitudecontribution.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/math.h>
#include <seiscomp3/math/geo.h>

#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/projection.h>

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
MagnitudeMap::MagnitudeMap(const MapsDesc &maps,
                           QWidget *parent, Qt::WFlags f)
 : MapWidget(maps, parent, f), _origin(NULL),
   _interactive(true), _drawStations(false) {
	_lastSymbolSize = 0;
	_stationsMaxDist = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeMap::MagnitudeMap(Map::ImageTree* mapTree,
                           QWidget *parent, Qt::WFlags f)
 : MapWidget(mapTree, parent, f), _origin(NULL),
   _interactive(true), _drawStations(false) {
	_lastSymbolSize = 0;
	_hoverId = -1;
	_stationsMaxDist = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeMap::~MagnitudeMap() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationsMaxDist(double maxDist) {
	_stationsMaxDist = maxDist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::paintEvent(QPaintEvent* e) {
	MapWidget::paintEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::drawLines(QPainter& p) {
	if ( _origin ) {
		QPointF originLocationF(_origin->longitude(), _origin->latitude());
		QPoint originLocation;
		int cutOff = 0;
		if ( canvas().projection()->project(originLocation, originLocationF) ) {
			if ( canvas().symbolCollection()->size() > 0 )
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
			if ( !(*it).validLocation ) continue;
			if ( !(*it).isActive ) continue;
			if ( !(*it).isMagnitude ) continue;
			canvas().drawGeoLine(p, originLocationF, (*it).location);
		}

		if ( cutOff )
			p.setClipping(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::drawCustomLayer(QPainter *painter) {
	QPainter &p(*painter);

	if ( _origin ) {
		int currentSymbolSize = 0;

		if ( canvas().symbolCollection()->size() > 0 )
			currentSymbolSize = (*canvas().symbolCollection()->begin())->size().width();

		if ( currentSymbolSize != _lastSymbolSize ) {
			_lastSymbolSize = currentSymbolSize;
		}

		if ( _drawStations ) drawLines(p);

		QPointF originLocationF(_origin->longitude(), _origin->latitude());
		//QPoint originLocation = project(originLocationF);

		try {
			double az = _origin->uncertainty().azimuthMaxHorizontalUncertainty();
			double lenMax = _origin->uncertainty().maxHorizontalUncertainty();
			double lenMin = _origin->uncertainty().minHorizontalUncertainty();

			p.setPen(Qt::NoPen);
			QColor fill = Qt::magenta;
			fill.setAlpha(64);
			p.setBrush(fill);

			lenMax = Math::Geo::km2deg(lenMax);
			lenMin = Math::Geo::km2deg(lenMin)/cos(deg2rad(std::max(-89.0, std::min(89.0, double(originLocationF.y())))));

			QPoint min, max;
			if ( canvas().projection()->project(min, QPointF(originLocationF.x()-lenMin, originLocationF.y()+lenMax)) &&
			     canvas().projection()->project(max, QPointF(originLocationF.x()+lenMin, originLocationF.y()-lenMax)) ) {

				QRect r(min, max);
				r = r.normalized();

				QPoint center = r.center();
				r.moveCenter(QPoint(0,0));

				p.translate(center);
				p.rotate(az);
				p.drawEllipse(r);
				p.resetMatrix();
			}
		}
		catch ( ... ) {
			double rad_x = 0.0, rad_y = 0.0;
			try {
				double err_x_l = _origin->longitude().lowerUncertainty();
				double err_x_u = _origin->longitude().upperUncertainty();

				rad_x = std::max(err_x_l, err_x_u);
			}
			catch ( ValueException& ) {
				try {
					rad_x = _origin->longitude().uncertainty();
				}
				catch ( ValueException& ) {}
			}

			try {
				double err_y_l = _origin->latitude().lowerUncertainty();
				double err_y_u = _origin->latitude().upperUncertainty();

				rad_y = std::max(err_y_l, err_y_u);
			}
			catch ( ValueException& ) {
				try {
					rad_y = _origin->latitude().uncertainty();
				}
				catch ( ValueException& ) {}
			}

			if ( rad_x == 0 && rad_y == 0 ) {
				try {
					rad_x = rad_y = _origin->uncertainty().horizontalUncertainty();
				}
				catch ( ValueException& ) {}
			}

			if ( rad_x > 0.0 && rad_y > 0.0 ) {
				rad_y = Math::Geo::km2deg(rad_y);
				rad_x = Math::Geo::km2deg(rad_x)/cos(deg2rad(std::max(-89.0, std::min(89.0, (double)_origin->latitude()))));

				p.setPen(Qt::NoPen);
				QColor fill = Qt::magenta;
				fill.setAlpha(64);
				p.setBrush(fill);

				QPoint min, max;

				if ( canvas().projection()->project(min, QPointF(originLocationF.x()-rad_x, originLocationF.y()+rad_y)) &&
				     canvas().projection()->project(max, QPointF(originLocationF.x()+rad_x, originLocationF.y()-rad_y)) )
					p.drawEllipse(min.x(), min.y(), max.x()-min.x()+1, max.y()-min.y()+1);
			}
		}

		if ( _drawStations ) {
			p.setPen(SCScheme.colors.map.outlines);
			int size = SCScheme.map.stationSize;
			for ( int i = _stations.count()-1; i >= 0; --i ) {
				if ( !_stations[i].validLocation ) continue;
				if ( !_interactive && !_stations[i].isActive ) continue;
				p.setBrush(
					_stations[i].isMagnitude?
						_stations[i].isActive?_stations[i].color:SCScheme.colors.magnitudes.disabled
					:
						_stations[i].isActive?SCScheme.colors.stations.idle:SCScheme.colors.magnitudes.unset);
				QPoint pp;
				if ( canvas().projection()->project(pp, _stations[i].location) )
					p.drawEllipse(pp.x()-size/2, pp.y()-size/2, size, size);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( (event->button() == Qt::LeftButton) && _interactive && _drawStations ) {
		if ( event->modifiers() == Qt::NoModifier ) {
			int stationHalfSize = SCScheme.map.stationSize / 2;
			for ( int i = 0; i < _stations.count(); ++i ) {
				QPoint pp;
				if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
				if ( abs(pp.x() - event->x()) <= stationHalfSize &&
					abs(pp.y() - event->y()) <= stationHalfSize ) {
					if ( _stations[i].isMagnitude ) {
						setMagnitudeState(_stations[i].magnitudeId, !_stations[i].isActive);
						emit magnitudeChanged(_stations[i].magnitudeId, _stations[i].isActive);
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
void MagnitudeMap::mousePressEvent(QMouseEvent* event) {
	if ( (event->button() == Qt::LeftButton) && _interactive && _drawStations ) {
		int stationHalfSize = SCScheme.map.stationSize / 2;
		for ( int i = 0; i < _stations.count(); ++i ) {
			QPoint pp;
			if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
			if ( abs(pp.x() - event->x()) <= stationHalfSize &&
				abs(pp.y() - event->y()) <= stationHalfSize ) {
				if ( _stations[i].isMagnitude )
					clickedMagnitude(_stations[i].magnitudeId);
				else
					clickedStation(_stations[i].net, _stations[i].code);
				return;
			}
		}
	}

	MapWidget::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::mouseMoveEvent(QMouseEvent *event) {
	int hoverId = -1;
	int stationHalfSize = SCScheme.map.stationSize / 2;
	for ( int i = 0; i < _stations.count(); ++i ) {
		QPoint pp;
		if ( !canvas().projection()->project(pp, _stations[i].location) ) continue;
		if ( abs(pp.x() - event->x()) <= stationHalfSize  &&
		     abs(pp.y() - event->y()) <= stationHalfSize ) {
			hoverId = i;
			break;
		}
	}

	if ( hoverId != _hoverId ) {
		_hoverId = hoverId;
		if ( _hoverId != -1 ) {
			int magnitudeId = _stations[_hoverId].magnitudeId;
			hoverMagnitude(magnitudeId);
			if ( toolTip().isEmpty() )
				setToolTip((_stations[_hoverId].net + "." + _stations[_hoverId].code).c_str());
		}
		else
			hoverMagnitude(-1);
	}

	MapWidget::mouseMoveEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationsInteractive(bool e) {
	_interactive = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitude(DataModel::Magnitude* nm) {
	_magnitude = nm;
	_magnitudes.clear();

	for ( int i = 0; i < _stations.size(); ++i ) {
		_stations[i].isActive = true;
		_stations[i].isMagnitude = false;
		_stations[i].magnitudeId = -1;
	}

	if ( _magnitude ) {
		for ( size_t i = 0; i < _magnitude->stationMagnitudeContributionCount(); ++i ) {
			StationMagnitude* staMag = StationMagnitude::Find(_magnitude->stationMagnitudeContribution(i)->stationMagnitudeID());
			if ( !staMag ) {
				SEISCOMP_DEBUG("StationMagnitude '%s' not found", _magnitude->stationMagnitudeContribution(i)->stationMagnitudeID().c_str());
				continue;
			}

			double residual = staMag->magnitude().value() - _magnitude->magnitude().value();
			addStationMagnitude(staMag, i);
			setMagnitudeResidual(i, residual);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setOrigin(DataModel::Origin* o) {
	_origin = o;

	_lastSymbolSize = 0;

	canvas().symbolCollection()->clear();
	_stations.clear();
	_stationCodes.clear();

	setMagnitude(NULL);

	if ( !_origin ) return;

	OriginSymbol* symbol = new OriginSymbol(o->latitude(), o->longitude());
	try { symbol->setDepth(o->depth()); } catch ( Core::ValueException& ) {}
	canvas().symbolCollection()->add(symbol);

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		bool foundStation = false;
		Arrival* arrival = _origin->arrival(i);
		Pick* p = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( p ) {
			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
					p->waveformID().networkCode(),
					p->waveformID().stationCode(),
					p->time()
				);

				std::string stationCode = p->waveformID().networkCode() + "." + p->waveformID().stationCode();
				_stations.push_back(StationEntry(QPointF(loc.longitude,loc.latitude),
				                                 p->waveformID().networkCode(),
				                                 p->waveformID().stationCode(), true));
				_stationCodes[stationCode] = _stations.size()-1;
				foundStation = true;
			}
			catch ( Core::ValueException& e ) {
				SEISCOMP_DEBUG("While fetching the station location an error occured: %s -> computing position", e.what());
				foundStation = false;
			}
		}
		else {
			SEISCOMP_DEBUG("pick for arrival not found -> setting arrival color [undefined]");
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
			}
			catch ( ... ) {
				_stations.push_back(StationEntry(QPointF(0,0), "", "", false));
			}
		}
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
						net->code(), sta->code().c_str(), true
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
void MagnitudeMap::addStationMagnitude(StationMagnitude* staMag, int index) {
	if ( index < _magnitudes.size() ) return;

	try {
		std::string stationCode = staMag->waveformID().networkCode() + "." + staMag->waveformID().stationCode();
		int stationId = findStation(stationCode);
		if ( stationId != -1 )
			addMagnitude(stationId, index);
	}
	catch ( ... ) {
		SEISCOMP_DEBUG("WaveformID in magnitude '%s' not set", staMag->publicID().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setDrawStations(bool f) {
	_drawStations = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagnitudeMap::findStation(const std::string& stationCode) const {
	std::map<std::string, int>::const_iterator it;
	it = _stationCodes.find(stationCode);
	if ( it != _stationCodes.end() )
		return it->second;
	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::addMagnitude(int stationId, int magId) {
	_stations[stationId].magnitudeId = magId;
	_stations[stationId].isMagnitude = true;

	if ( magId >= _magnitudes.size() )
		_magnitudes.resize(magId+1);

	_magnitudes[magId] = stationId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitudeState(int id, bool state) {
	if ( id < 0 || id >= _magnitudes.size() ) return;
	int stationId = _magnitudes[id];
	setStationState(stationId, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitudeResidual(int id, double residual) {
	if ( id < 0 || id >= _magnitudes.size() ) return;
	int stationId = _magnitudes[id];
	setStationResidual(stationId, residual);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationState(int i, bool state) {
	if ( _stations[i].isActive == state ) return;
	_stations[i].isActive = state;
	//QPoint p = project(_stations[i].location);
	//update(p.x()-5, p.y()-5, 10, 10);
	if ( _drawStations )
		update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationState(const std::string& code, bool state) {
	std::map<std::string, int>::iterator it = _stationCodes.find(code);
	if ( it == _stationCodes.end() ) return;
	setStationState((*it).second, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationResidual(int i, double residual) {
	_stations[i].residual = residual;
	_stations[i].color = SCScheme.colors.magnitudes.residuals.colorAt(residual);

	if ( _drawStations ) update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
