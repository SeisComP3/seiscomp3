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


#ifndef __SEISCOMP_GUI_PLOT_DATAXY_H__
#define __SEISCOMP_GUI_PLOT_DATAXY_H__


#include <seiscomp3/gui/plot/abstractdataset.h>
#include <QVector>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API DataXY : public AbstractDataSet {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief DataPoint represents a single point in a graph with a key
		 *        and a value.
		 */
		typedef QPointF DataPoint;

		/**
		 * @brief DataPoints represents the graph data as list of data points.
		 */
		typedef QVector<DataPoint> DataPoints;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		DataXY(QObject *parent = 0);


	// ----------------------------------------------------------------------
	//  AbstractGraph interface
	// ----------------------------------------------------------------------
	public:
		virtual int count() const;
		virtual Range getXRange() const;
		virtual Range getYRange() const;

		virtual void getBounds(Range &x, Range &y) const;

		virtual void clear();

		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const;


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		DataPoints data;
};


}
}


#endif
