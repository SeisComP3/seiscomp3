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
		void addItem(StandardLegendItem *item);
		void setMaximumColumns(int columns);
		void setOrientation(Qt::Orientation orientation);


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
		int                        _maxColumns;
		bool                       _layoutDirty;

};


} // namespace Map
} // namespce Gui
} // namespace Seiscomp


#endif
