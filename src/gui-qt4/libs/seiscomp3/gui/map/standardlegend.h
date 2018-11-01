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


#ifndef __SEISCOMP_GUI_MAP_STANDARDLEGEND_H__
#define __SEISCOMP_GUI_MAP_STANDARDLEGEND_H__


#include <seiscomp3/gui/map/legend.h>


#include <QPen>
#include <QBrush>
#include <QRect>


namespace Seiscomp {
namespace Gui {
namespace Map {


class StandardLegend;


class SC_GUI_API StandardLegendItem {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		StandardLegendItem(StandardLegend *legend = NULL);
		StandardLegendItem(const QPen &p, const QString &l);
		StandardLegendItem(const QPen &p, const QString &l, int s);
		StandardLegendItem(const QPen &p, const QBrush &b, const QString &l);
		StandardLegendItem(const QPen &p, const QBrush &b, const QString &l, int s);
		virtual ~StandardLegendItem();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter *painter, const QRect &symbolRect,
		                  const QRect &textRect);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		void drawSymbol(QPainter *painter, const QRect &rec);
		void drawText(QPainter *painter, const QRect &rec);


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		QPen    pen;
		QBrush  brush;
		QString label;
		int     size;
};


class SC_GUI_API StandardLegend : public Legend {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		StandardLegend(QObject *parent);
		//! D'tor
		~StandardLegend();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Add an item to the legend. The legend will take the ownership of
		//! the item.
		void addItem(StandardLegendItem *item);

		//! Removes all items in the legend.
		void clear();

		//! Returns the number of items in the legend.
		int count();

		//! Returns the item at index
		StandardLegendItem *itemAt(int index) const;

		//! Removes and returns the item from the given index in the leged;
		//! otherwise returns 0. Items taken from a legend will not be managed
		//! by the legend and will need to be deleted manually.
		StandardLegendItem *takeItem(int index);

		void setMaximumColumns(int columns);
		void setOrientation(Qt::Orientation orientation);

		//! This method needs to be called if the configuration of the
		//! legend has changed to update its layout. Otherwise the update
		//! will be delayed to the second render pass and might cause
		//! one frame of distored rendering.
		void update();


	// ----------------------------------------------------------------------
	//  Legend interface
	// ----------------------------------------------------------------------
	public:
		virtual void contextResizeEvent(const QSize &size);
		virtual void draw(const QRect &r, QPainter &p);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void updateLayout(const QSize &size);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Qt:: Orientation           _orientation;
		QList<StandardLegendItem*> _items;
		int                        _columns;
		int                        _columnWidth;
		QSize                      _symbolSize;
		int                        _maxColumns;
		bool                       _layoutDirty;

};


inline int StandardLegend::count() {
	return _items.count();
}

inline StandardLegendItem *StandardLegend::itemAt(int index) const {
	return _items[index];
}


} // namespace Map
} // namespce Gui
} // namespace Seiscomp


#endif
