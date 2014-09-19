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


#ifndef __SEISCOMP_GUI_MAPSYMBOLCOLLECTION_H___
#define __SEISCOMP_GUI_MAPSYMBOLCOLLECTION_H___

#include <QList>

#include <seiscomp3/gui/map/mapsymbol.h>

namespace Seiscomp {
namespace Gui {
namespace Map {


class SymbolCollection {
	public:
		typedef QList<Symbol*> Symbols;
		typedef Symbols::iterator iterator;
		typedef Symbols::const_iterator const_iterator;

	public:
		virtual ~SymbolCollection() {}

	public:
		virtual bool add(Symbol* mapSymbol) = 0;
		virtual const_iterator remove(Symbol* mapSymbol) = 0;

		virtual void clear() = 0;

		virtual Symbols::size_type size() const = 0;

		virtual bool bringToTop(Symbol* drawable) = 0;
		virtual bool sendToBack(Symbol* drawable) = 0;

		virtual void setTop(Symbol* topDrawable) = 0;
		virtual Symbol* top() const = 0;

		virtual iterator begin() = 0;
		virtual iterator end() = 0;

		virtual const_iterator begin() const = 0;
		virtual const_iterator end() const = 0;

};


class DefaultSymbolCollection : public SymbolCollection {
	public:
		DefaultSymbolCollection();

	public:
		virtual bool add(Symbol* mapSymbol);
		virtual Symbols::const_iterator remove(Symbol* mapSymbol);

		virtual void clear();

		virtual Symbols::size_type size() const;

		virtual bool bringToTop(Symbol* drawable);
		virtual bool sendToBack(Symbol* drawable);

		virtual void setTop(Symbol* topDrawable);
		virtual Symbol* top() const;

		virtual Symbols::iterator begin();
		virtual Symbols::iterator end();

		virtual Symbols::const_iterator begin() const;
		virtual Symbols::const_iterator end() const;

	private:
		Symbols _mapSymbols;
		Symbol* _topSymbol;
};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp

#endif
