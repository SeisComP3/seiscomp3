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


#ifndef __SEISCOMP_GUI_PLOT_ABSTRACTDATASET_H__
#define __SEISCOMP_GUI_PLOT_ABSTRACTDATASET_H__


#include <seiscomp3/gui/plot/range.h>
#include <QObject>
#include <QRectF>


class QPolygonF;


namespace Seiscomp {
namespace Gui {


class Axis;


class SC_GUI_API AbstractDataSet : public QObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit AbstractDataSet(QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Query interface
	// ----------------------------------------------------------------------
	public:
		bool isEmpty() const { return this->count() == 0; }

		virtual int count() const = 0;
		virtual Range getXRange() const = 0;
		virtual Range getYRange() const = 0;

		virtual void getBounds(Range &x, Range &y) const;

		/**
		 * @brief Convenience function that returns a QRectF instead of two
		 *        single ranges.
		 * @return The bounding rectangle of the graph data
		 */
		QRectF getBounds() const;

		virtual void clear() = 0;

		/**
		 * @brief Unprojects the data from axis space to pixel space
		 * @param poly The target polygon that holds the projected and possibly
		 *             clipped data. Note that the number of points in the
		 *             polygon does not necessarily match the number of points
		 *             in the graph due to clipping.
		 * @param keyAxis The axis to project keys to
		 * @param valueAxis The axis to project values to
		 */
		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const = 0;
};


}
}


#endif
