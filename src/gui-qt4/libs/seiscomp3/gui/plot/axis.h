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


#ifndef __SEISCOMP_GUI_PLOT_AXIS_H__
#define __SEISCOMP_GUI_PLOT_AXIS_H__


#include <QObject>
#include <QPainter>
#if QT_VERSION >= 0x040300
#include <QTransform>
#endif

#include <seiscomp3/gui/plot/range.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API Axis : public QObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		enum AxisPosition {
			Left,
			Right,
			Top,
			Bottom
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Axis(QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setLabel(const QString &label);
		QString label() const;

		void setTickLength(int length);
		int tickLength() const;

		void setRange(const Range &range);
		void extendRange(const Range &range);
		const Range &range() const;

		void setPosition(AxisPosition pos);
		AxisPosition position() const;

		void setLogScale(bool);
		bool logScale() const;

		void setLogBase(double);
		double logBase() const;

		void setVisible(bool visible);
		bool isVisible() const;

		void setGrid(bool grid);
		bool hasGrid() const;


	// ----------------------------------------------------------------------
	//  Render specific interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns the width in pixels of the axis. This is independent
		 *        of the direction (horizontal or vertical).
		 * @return The width or height in pixels
		 */
		int width() const;

		//! Transforms a pixel value into axis space. A call to updateLayout
		//!  must precede this call.
		double project(double pixelValue) const;

		//! Transforms an axis value into pixel. A call to updateLayout must
		//! precede this call.
		double unproject(double axisValue) const;

		/**
		 * @brief Update the layout and sizes based on the range and settings.
		 *        This calls generates also the tick marks and the extend of
		 *        the axis in the flexible direction.
		 * @param painter The painter used to render the axis.
		 * @param rect The target rect which will be changed according to the
		 *             flexible dimension, e.g. width or height.
		 */
		void updateLayout(QPainter &painter, QRect &rect);

		/**
		 * @brief Returns the size hint for the flexible dimension, for vertical
		 *        axes it is the width and for horizontal axes the height.
		 * @param painter The painter used to render the axis.
		 * @return The value in pixels or -1 if not determinable.
		 */
		int sizeHint(QPainter &painter) const;

		/**
		 * @brief Renders the axis into the given rect.
		 * @param painter The painter instance to render with.
		 * @param rect The target rectangle. Depending on the position the rect
		 *             is being updated according to the flexible direction if
		 *             the rect dimension (either width or height) is not valid.
		 *             A valid dimension forces the axis to render into the given
		 *             rect.
		 */
		void draw(QPainter &painter, const QRect &rect);

		/**
		 * @brief Draws the grid lines at the (sub)ticks within a passed plot rect.
		 * @param painter The painter instance to render with.
		 * @param rect The target rect for the plot.
		 * @param ticks Defines whether ticks should be considered or not
		 * @param subTicks Defines whether sub-ticks should be considered or not
		 */
		void drawGrid(QPainter &painter, const QRect &rect, bool ticks, bool subTicks);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		typedef QPair<double,int> Tick;

		// Setup
		bool          _visible;
		bool          _grid;
		QString       _label;
		Range         _range;
		AxisPosition  _position;
		bool          _logScale;
		double        _logBase;
		int           _tickLength;
		int           _spacing;

		// Render configuration
#if QT_VERSION >= 0x040300
		QTransform    _transform;
#endif
		int           _extent;
		int           _width;
		double        _axisStartValue;
		double        _axisEndValue;
		double        _axisPixelScale;
		double        _tickSpacing;
		double        _tickStart;
		QVector<Tick> _ticks;
		QVector<Tick> _subTicks;
};



inline int Axis::width() const {
	return _width;
}

inline void Axis::setVisible(bool visible) {
	_visible = visible;
}

inline bool Axis::isVisible() const {
	return _visible;
}

inline void Axis::setGrid(bool grid) {
	_grid = grid;
}

inline bool Axis::hasGrid() const {
	return _grid;
}

inline void Axis::setLabel(const QString &label) {
	_label = label;
}

inline QString Axis::label() const {
	return _label;
}

inline void Axis::setTickLength(int length) {
	_tickLength = length;
}

inline int Axis::tickLength() const {
	return _tickLength;
}

inline void Axis::setRange(const Range &range) {
	_range = range;
}

inline const Range &Axis::range() const {
	return _range;
}

inline void Axis::setPosition(AxisPosition pos) {
	_position = pos;
}

inline Axis::AxisPosition Axis::position() const {
	return _position;
}

inline void Axis::setLogScale(bool s) {
	_logScale = s;
}

inline bool Axis::logScale() const {
	return _logScale;
}

inline double Axis::logBase() const {
	return _logBase;
}


}
}


#endif
