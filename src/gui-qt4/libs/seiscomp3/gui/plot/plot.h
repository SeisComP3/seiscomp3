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


#ifndef __SEISCOMP_GUI_PLOT_PLOT_H__
#define __SEISCOMP_GUI_PLOT_PLOT_H__


#include <QObject>

#include <seiscomp3/gui/qt4.h>


class QRect;
class QPainter;


namespace Seiscomp {
namespace Gui {


class Axis;
class Graph;
class AbstractLegend;


class SC_GUI_API Plot : public QObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Plot(QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Adds a graph to the plot. The graph is managed by the plot.
		 * @param keyAxis The axis to use for keys. If NULL is passed then
		 *                the default xAxis is being used.
		 * @param valueAxis The axis to use for values. If NULL is passed then
		 *                  the default yAxis is being used.
		 * @return The graph pointer or NULL in case of error.
		 */
		Graph *addGraph(Axis *keyAxis = NULL, Axis *valueAxis = NULL);

		/**
		 * @brief Adds a graph instance to the plot. This function was added
		 *        with API version 11.0.0.
		 * @param graph The graph instance. The ownership is transferred to
		 *              the plot.
		 */
		void addGraph(Graph *graph);

		/**
		 * @brief Sets a legend instance for which the draw method is called
		 *        at every plot update. This function was added with API
		 *        version 11.0.0.
		 * @param legend The legend instance. The ownership goes to the plot.
		 */
		void setLegend(AbstractLegend *legend);

		/**
		 * @brief Updates all axis ranges according to the bounds of the
		 *        attached graphs.
		 */
		void updateRanges();

		/**
		 * @brief Renders the plot into the given rect
		 * @param painter The painter used to draw the plot
		 * @param rect The target rectangle in screen coordinates
		 */
		void draw(QPainter &painter, const QRect &rect);

		/**
		 * @brief Checks whether point is inside the plot area. This function
		 *        was added with API version 11.0.0.
		 * @param p The point in pixels
		 * @return true if inside, false otherwise
		 */
		bool isInsidePlot(const QPoint &p) const;

		/**
		 * @brief Returns the rectangle of the actual data plot without
		 *        the axis rectangle. This function was added with API
		 *        version 11.0.0.
		 * @return QRect
		 */
		const QRect &plotRect() const;


	// ----------------------------------------------------------------------
	//  Public Members
	// ----------------------------------------------------------------------
	public:
		Axis *xAxis;   //!< bottom axis
		Axis *yAxis;   //!< left axis
		Axis *xAxis2;  //!< top axis
		Axis *yAxis2;  //!< right axis


	protected:
		typedef QList<Graph*> Graphs;
		Graphs          _graphs;
		AbstractLegend *_legend;
		QRect           _plotRect; //!< The plot rectangle of the last draw
};


inline bool Plot::isInsidePlot(const QPoint &p) const {
	return _plotRect.contains(p);
}

inline const QRect &Plot::plotRect() const {
	return _plotRect;
}


}
}


#endif
