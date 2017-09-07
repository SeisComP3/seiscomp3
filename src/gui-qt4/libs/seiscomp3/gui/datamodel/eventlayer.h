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
		virtual bool isInside(int x, int y) const;

		virtual void handleEnterEvent();
		virtual void handleLeaveEvent();
		virtual bool filterMouseMoveEvent(QMouseEvent *event, const QPointF &geoPos);
		virtual bool filterMouseDoubleClickEvent(QMouseEvent *event, const QPointF &geoPos);


	// ----------------------------------------------------------------------
	//  Slots
	// ----------------------------------------------------------------------
	public slots:
		void clear();
		void addEvent(Seiscomp::DataModel::Event*,bool);
		void updateEvent(Seiscomp::DataModel::Event*);
		void removeEvent(Seiscomp::DataModel::Event*);


	// ----------------------------------------------------------------------
	//  Signals
	// ----------------------------------------------------------------------
	signals:
		void eventHovered(const std::string &eventID);
		void eventSelected(const std::string &eventID);


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		typedef QMap<std::string, OriginSymbol*> SymbolMap;

		SymbolMap           _eventSymbols;
		mutable std::string _hoverId;
		mutable bool        _hoverChanged;
};



}
}


#endif
