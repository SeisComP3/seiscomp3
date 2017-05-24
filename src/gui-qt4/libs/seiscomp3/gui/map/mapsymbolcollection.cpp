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


#include <seiscomp3/gui/map/mapsymbolcollection.h>

namespace Seiscomp {
namespace Gui {
namespace Map {


namespace {


bool byLatitude(const Symbol *s1, const Symbol *s2) {
	return s1->latitude() > s2->latitude();
}


}


DefaultSymbolCollection::DefaultSymbolCollection()
: _topSymbol(NULL)
, _dirty(false) {}


bool DefaultSymbolCollection::add(Symbol* symbol) {
	_mapSymbols.push_back(symbol);
	_dirty = true;
	return true;
}


SymbolCollection::const_iterator DefaultSymbolCollection::remove(Symbol* symbol) {
	for ( Symbols::iterator it = _mapSymbols.begin();
			it != _mapSymbols.end(); ++it ) {
		if ( *it == symbol ) {
			// Set pointer to top symbol NULL
			if ( top() == symbol )
				setTop(NULL);

			delete *it;
			_dirty = true;
			return _mapSymbols.erase(it);
		}
	}

	return _mapSymbols.end();
}


bool DefaultSymbolCollection::sendToBack(Symbol* symbol) {
	_mapSymbols.move(_mapSymbols.indexOf(symbol), 0);
	return true;
}


bool DefaultSymbolCollection::bringToTop(Symbol* symbol) {
	_mapSymbols.move(_mapSymbols.indexOf(symbol), _mapSymbols.size() - 1);
	return true;
}


void DefaultSymbolCollection::clear() {
	for ( Symbols::iterator it = _mapSymbols.begin(); it != _mapSymbols.end(); ++it )
		delete *it;
	_mapSymbols.clear();
	setTop(NULL);
}


SymbolCollection::Symbols::size_type DefaultSymbolCollection::size() const {
	return _mapSymbols.size();
}


void DefaultSymbolCollection::setTop(Symbol* topSymbol) {
	_topSymbol= topSymbol;
}


Symbol* DefaultSymbolCollection::top() const {
	return _topSymbol;
}


SymbolCollection::Symbols::iterator DefaultSymbolCollection::begin() {
	return _mapSymbols.begin();
}


SymbolCollection::Symbols::iterator DefaultSymbolCollection::end() {
	return _mapSymbols.end();
}


SymbolCollection::Symbols::const_iterator DefaultSymbolCollection::begin() const {
	return _mapSymbols.begin();
}


SymbolCollection::Symbols::const_iterator DefaultSymbolCollection::end() const {
	return _mapSymbols.end();
}


void DefaultSymbolCollection::sortByLatitude() {
	 qSort(_mapSymbols.begin(), _mapSymbols.end(), byLatitude);
}


} // namespace Map
} // namespace Gui
} // namespace Seiscomp
