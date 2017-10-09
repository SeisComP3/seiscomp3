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


#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>

#include <QMenu>
#include <QMouseEvent>

#include "eventlayer.h"


namespace Seiscomp {
namespace Gui {
namespace {


void updateSymbol(Map::Canvas *canvas, OriginSymbol *symbol,
                  DataModel::Event *event, DataModel::Origin *org) {
	symbol->setLocation(org->latitude(), org->longitude());

	try {
		symbol->setDepth(org->depth());
	}
	catch ( ... ) {}

	DataModel::Magnitude *mag = DataModel::Magnitude::Find(event->preferredMagnitudeID());
	if ( mag != NULL ) {
		try {
			symbol->setPreferredMagnitudeValue(mag->magnitude());
		}
		catch ( ... ) {
			symbol->setPreferredMagnitudeValue(0);
		}
	}
	else
		symbol->setPreferredMagnitudeValue(0);

	if ( canvas )
		symbol->calculateMapPosition(canvas);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLayer::EventLayer(QObject* parent) : Map::Layer(parent) {
	setName("events");
	setVisible(false);
	_hoverChanged = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLayer::~EventLayer() {
	SymbolMap::iterator it;

	// Delete all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it ) {
		delete it.value();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::draw(const Map::Canvas *canvas, QPainter &p) {
	SymbolMap::iterator it;

	// Render all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it )
		it.value()->draw(canvas, p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::calculateMapPosition(const Map::Canvas *canvas) {
	SymbolMap::iterator it;

	// Update position of all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it )
		it.value()->calculateMapPosition(canvas);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::isInside(int x, int y) const {
	SymbolMap::const_iterator it = _eventSymbols.end();

	while ( it != _eventSymbols.begin() ) {
		--it;
		if ( it.value()->isInside(x, y) ) {
			_hoverChanged = _hoverId != it.key();
			if ( _hoverChanged )
				_hoverId = it.key();
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::handleEnterEvent() {
	//
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::handleLeaveEvent() {
	_hoverChanged = !_hoverId.empty();
	_hoverId = std::string();
	emit eventHovered(_hoverId);
	_hoverChanged = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::filterMouseMoveEvent(QMouseEvent *event, const QPointF &geoPos) {
	if ( _hoverChanged ) {
		emit eventHovered(_hoverId);
		_hoverChanged = false;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::filterMouseDoubleClickEvent(QMouseEvent *event, const QPointF &geoPos) {
	if ( _hoverId.empty() )
		return false;

	if ( event->button() == Qt::LeftButton ) {
		emit eventSelected(_hoverId);
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::clear() {
	SymbolMap::iterator it;

	// Delete all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it )
		delete it.value();

	_eventSymbols.clear();
	_hoverChanged = false;
	_hoverId = std::string();
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::addEvent(Seiscomp::DataModel::Event *e, bool) {
	SymbolMap::iterator it = _eventSymbols.find(e->publicID());

	DataModel::Origin *org = DataModel::Origin::Find(e->preferredOriginID());
	if ( org != NULL ) {
		OriginSymbol *symbol;

		if ( it == _eventSymbols.end() )
			symbol = new OriginSymbol();
		else
			symbol = it.value();

		updateSymbol(canvas(), symbol, e, org);
		_eventSymbols[e->publicID()] = symbol;

		// Create origin symbol and register it
		updateRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::updateEvent(Seiscomp::DataModel::Event *e) {
	SymbolMap::iterator it = _eventSymbols.find(e->publicID());
	if ( it == _eventSymbols.end() ) return;

	DataModel::Origin *org = DataModel::Origin::Find(e->preferredOriginID());
	if ( org != NULL ) {
		updateSymbol(canvas(), it.value(), e, org);
		updateRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::removeEvent(Seiscomp::DataModel::Event *e) {
	SymbolMap::iterator it = _eventSymbols.find(e->publicID());
	if ( it == _eventSymbols.end() ) return;
	delete it.value();
	_eventSymbols.erase(it);
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
