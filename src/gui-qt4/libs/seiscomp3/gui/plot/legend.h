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


#ifndef __SEISCOMP_GUI_PLOT_LEGEND_H__
#define __SEISCOMP_GUI_PLOT_LEGEND_H__


#include <seiscomp3/gui/plot/abstractlegend.h>


namespace Seiscomp {
namespace Gui {


/**
 * @brief The Legend class renders a default legend for all visible graphs
 *        where the name is not empty.
 */
class SC_GUI_API Legend : public AbstractLegend {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit Legend(Qt::Alignment align = Qt::AlignTop | Qt::AlignRight,
		                QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Render interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter &p, const QRect &plotRect,
		                  const QList<Graph*> &graphs);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Qt::Alignment _alignment;
};


}
}


#endif
