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


#ifndef __SEISCOMP_GUI_PLOT_ABSTRACTGRAPH_H__
#define __SEISCOMP_GUI_PLOT_ABSTRACTGRAPH_H__


#include <seiscomp3/gui/plot/axis.h>
#include <seiscomp3/gui/plot/abstractdataset.h>
#include <QObject>
#include <QColor>
#include <QPolygonF>


namespace Seiscomp {
namespace Gui {


class AbstractDataSet;

class SC_GUI_API Graph : public QObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit Graph(Axis *keyAxis, Axis *valueAxis, QObject *parent=0);
		explicit Graph(const QString &name, Axis *keyAxis, Axis *valueAxis, QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Meta data
	// ----------------------------------------------------------------------
	public:
		void setName(const QString &name);
		const QString &name() const;


	// ----------------------------------------------------------------------
	//  Setup
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the pen used to draw the graph
		 * @param p The pen
		 */
		void setPen(const QPen &p);
		const QPen &pen() const;

		/**
		 * @brief Sets the primary (line) color of the graph.
		 * @param c The color
		 */
		void setColor(QColor c);
		QColor color() const;

		/**
		 * @brief Sets the line width of the graph.
		 * @param lw The line width in pixels
		 */
		void setLineWidth(int lw);
		int lineWidth() const;

		/**
		 * @brief Sets whether anti aliasing should be enabled or not.
		 * @param e Flag
		 */
		void setAntiAliasing(bool e);
		bool antiAliasing() const;

		/**
		 * @brief Sets whether to drop a show in the plot or not for this graph
		 * @param e Flag
		 */
		void setDropShadow(bool e);
		bool dropShadow() const;


	// ----------------------------------------------------------------------
	//  Query interface
	// ----------------------------------------------------------------------
	public:
		Axis *keyAxis() const;
		Axis *valueAxis() const;

		void setData(AbstractDataSet *data);
		AbstractDataSet *data() const;

		bool isEmpty() const { return count() == 0; }

		int count() const;
		Range getXRange() const;
		Range getYRange() const;

		void getBounds(Range &x, Range &y) const;

		/**
		 * @brief Convenience function that returns a QRectF instead of two
		 *        single ranges.
		 * @return The bounding rectangle of the graph data
		 */
		QRectF getBounds() const;


	// ----------------------------------------------------------------------
	//  Data interface
	// ----------------------------------------------------------------------
	public:
		void setVisible(bool visible);
		bool isVisible() const { return _visible; }

		void clear();

		/**
		 * @brief Unprojects the data from axis space to pixel space
		 * @param poly The target polygon that holds the projected and possibly
		 *             clipped data. Note that the number of points in the
		 *             polygon does not necessarily match the number of points
		 *             in the graph due to clipping.
		 */
		virtual void unproject(QPolygonF &poly) const;


	// ----------------------------------------------------------------------
	//  Render interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Draws the graph. The default implementation projects the
		 *        associated dataset to screen coordinates and renders a
		 *        polyline with the configured pen and style.
		 *
		 * Derived classes can reimplement this method to do custom drawing.
		 * Note that screen space projection must be inverted to accomodate
		 * for top-down coordinate system. The plot sets up the correct
		 * clipping and translation before calling this method.
		 *
		 * @param p The painter
		 */
		virtual void draw(QPainter &p);

		/**
		 * @brief Draws a symbol to represent the graph. This is used by e.g.
		 *        legends. The default implementation draws a horizontal line
		 *        with the configured pen. The line is vertically centered in
		 *        the target rectangle.
		 * @param p The painter
		 * @param r The target rect
		 */
		virtual void drawSymbol(QPainter &p, const QRect &r);


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		// Meta data
		QString          _name;

		// Context
		Axis            *_keyAxis;
		Axis            *_valueAxis;

		AbstractDataSet *_data;

		// Setup
		bool             _visible;
		QPen             _pen;
		bool             _antiAliasing;
		bool             _dropShadow;
};


inline const QString &Graph::name() const {
	return _name;
}

inline Axis *Graph::keyAxis() const {
	return _keyAxis;
}

inline Axis *Graph::valueAxis() const {
	return _valueAxis;
}

inline void Graph::setData(AbstractDataSet *data) {
	_data = data;
}

inline AbstractDataSet *Graph::data() const {
	return _data;
}

inline int Graph::count() const {
	return _data != NULL ? _data->count() : 0;
}

inline Range Graph::getXRange() const {
	return _data != NULL ? _data->getXRange() : Range();
}

inline Range Graph::getYRange() const {
	return _data != NULL ? _data->getYRange() : Range();
}

inline void Graph::clear() {
	if ( _data != NULL ) _data->clear();
}


}
}


#endif
