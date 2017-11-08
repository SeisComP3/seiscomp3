/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_PLOT_ABSTRACTLEGEND_H__
#define __SEISCOMP_GUI_PLOT_ABSTRACTLEGEND_H__


#include <seiscomp3/gui/qt4.h>
#include <QObject>
#include <QPainter>
#include <QList>


namespace Seiscomp {
namespace Gui {


class Graph;


class SC_GUI_API AbstractLegend : public QObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit AbstractLegend(QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Setup
	// ----------------------------------------------------------------------
	public:
		void setVisible(bool visible);
		bool isVisible() const { return _visible; }


	// ----------------------------------------------------------------------
	//  Render interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter &p, const QRect &plotRect,
		                  const QList<Graph*> &graphs) = 0;


	// ----------------------------------------------------------------------
	//  Render interface
	// ----------------------------------------------------------------------
	protected:
		bool _visible;
};


}
}


#endif
