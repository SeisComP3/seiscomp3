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


#ifndef __SEISCOMP_GUI_EVENTLAYER_H__
#define __SEISCOMP_GUI_EVENTLAYER_H__


#include <seiscomp3/gui/map/layer.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <QMap>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API EventLayer : public Map::Layer {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		EventLayer(QObject* parent = NULL);

		//! D'tor
		~EventLayer();


	// ----------------------------------------------------------------------
	//  Layer interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(const Map::Canvas *, QPainter &);
		virtual void calculateMapPosition(const Map::Canvas *canvas);


	// ----------------------------------------------------------------------
	//  Slots
	// ----------------------------------------------------------------------
	public slots:
		void clear();
		void addEvent(Seiscomp::DataModel::Event*);
		void updateEvent(Seiscomp::DataModel::Event*);
		void removeEvent(Seiscomp::DataModel::Event*);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef QMap<DataModel::Event*, OriginSymbol*> SymbolMap;
		SymbolMap _eventSymbols;
};



}
}


#endif
