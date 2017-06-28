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

	// Delete all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it )
		it.value()->draw(canvas, p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::calculateMapPosition(const Map::Canvas *canvas) {
	SymbolMap::iterator it;

	// Delete all symbols
	for ( it = _eventSymbols.begin(); it != _eventSymbols.end(); ++it )
		it.value()->calculateMapPosition(canvas);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::clear() {
	_eventSymbols.clear();
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::addEvent(Seiscomp::DataModel::Event *e) {
	SymbolMap::iterator it = _eventSymbols.find(e);

	DataModel::Origin *org = DataModel::Origin::Find(e->preferredOriginID());
	if ( org != NULL ) {
		OriginSymbol *symbol;

		if ( it == _eventSymbols.end() )
			symbol = new OriginSymbol();
		else
			symbol = it.value();

		updateSymbol(canvas(), symbol, e, org);
		_eventSymbols[e] = symbol;

		// Create origin symbol and register it
		updateRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::updateEvent(Seiscomp::DataModel::Event *e) {
	SymbolMap::iterator it = _eventSymbols.find(e);
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
	SymbolMap::iterator it = _eventSymbols.find(e);
	if ( it == _eventSymbols.end() ) return;
	_eventSymbols.erase(it);
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
