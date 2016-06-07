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


#ifndef __SEISCOMP_GUI_PLOT_DATAY_H__
#define __SEISCOMP_GUI_PLOT_DATAY_H__


#include <seiscomp3/gui/plot/abstractdataset.h>
#include <QVector>


namespace Seiscomp {
namespace Gui {


/**
 * @brief The DataY class describes a data set with an array of values and
 *        a procedural key space which is linearily interpolated for each
 *        value between index 0 and N-1. By definition the lower key corresponds
 *        with value at index 0 and the upper key with value at index N-1.
 *        That means the distance between two subsequent keys is range.length / (N-1)
 *        which requires at least two values to render a valid key space.
 */
class SC_GUI_API DataY : public AbstractDataSet {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief DataPoint represents a single value in a graph. The key values
		 *        will be generated.
		 */
		typedef double DataPoint;

		/**
		 * @brief DataPoints represents the graph data as list of data points.
		 */
		typedef QVector<DataPoint> DataPoints;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		DataY(QObject *parent = 0);


	// ----------------------------------------------------------------------
	//  AbstractGraph interface
	// ----------------------------------------------------------------------
	public:
		virtual int count() const;
		virtual Range getXRange() const;
		virtual Range getYRange() const;

		virtual void clear();

		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const;


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		//! The x range
		Range      x;
		//! The y data points
		DataPoints y;
};


}
}


#endif
