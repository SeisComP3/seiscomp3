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


#ifndef __SEISCOMP_GUI_MAP_LAYERS_SYMBOLLAYER_H__
#define __SEISCOMP_GUI_MAP_LAYERS_SYMBOLLAYER_H__


#include <seiscomp3/gui/map/layer.h>
#include <seiscomp3/gui/map/mapsymbol.h>
#include <QList>


namespace Seiscomp {
namespace Gui {
namespace Map {


class SC_GUI_API SymbolLayer : public Layer {
	// ----------------------------------------------------------------------
	//  Types
	// ----------------------------------------------------------------------
	public:
		typedef QList<Symbol*> Symbols;
		typedef Symbols::iterator iterator;
		typedef Symbols::const_iterator const_iterator;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SymbolLayer(QObject* parent = NULL);
		//! D'tor
		~SymbolLayer();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Adds a symbol to the layer.
		 * @param symbol The symbol instance. Ownership is  transferred to
		 *               the layer
		 * @return Success flag
		 */
		bool add(Symbol *symbol);
		Symbols::const_iterator remove(Symbol *symbol);

		void clear();

		Symbols::size_type count() const;

		bool bringToTop(Symbol *drawable);
		bool sendToBack(Symbol *drawable);

		void setTop(Symbol *topDrawable);
		Symbol *top() const;

		Symbols::iterator begin();
		Symbols::iterator end();

		Symbols::const_iterator begin() const;
		Symbols::const_iterator end() const;

		void sortByLatitude();


	// ----------------------------------------------------------------------
	//  Layer interface
	// ----------------------------------------------------------------------
	public:
		virtual void setVisible(bool);
		virtual void calculateMapPosition(const Canvas *canvas);
		virtual bool isInside(const QMouseEvent *event, const QPointF &geoPos);
		virtual void draw(const Canvas *canvas, QPainter &p);


	private:
		Symbols  _symbols;
		Symbol  *_topSymbol;
};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp


#endif
