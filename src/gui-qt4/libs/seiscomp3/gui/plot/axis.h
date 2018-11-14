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

		/**
		 * @brief Set the pen used to draw the axis, the ticks and the
		 *        labels.
		 * @param pen The pen
		 */
		void setPen(const QPen &pen);
		const QPen &pen() const;

		/**
		 * @brief Set the pen used to draw the primary grid at ticks. The
		 *        default is a solid pen with color (192,192,192) and line
		 *        width 1.
		 * @param pen The pen
		 */
		void setGridPen(const QPen &pen);
		const QPen &gridPen() const;

		/**
		 * @brief Set the pen used to draw the grid at subticks. The default
		 *        is a solid pen with color (224,224,224) and line width 1.
		 * @param pen The pen
		 */
		void setSubGridPen(const QPen &pen);
		const QPen &subGridPen() const;


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
		 *             flexible dimension, e.g. width or height. The width/height
		 *             of the rect is only changed if it is equal to 0.
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
		 * @param clipText Whether to check for tick label overlaps with the given
		 *                 rect or not. If enabled, then tick labels that intersect
		 *                 with the rect will be pushed into the rect if possible.
		 */
		void draw(QPainter &painter, const QRect &rect,
		          bool clipText = false);

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
		struct Tick {
			Tick() {}
			Tick(double v, int rp, int ap) : value(v), relPos(rp), absPos(ap) {}
			double value;
			int    relPos;
			int    absPos;
		};

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

		QPen          _penAxis;
		QPen          _penGridTicks;
		QPen          _penGridSubticks;
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

inline const QPen &Axis::pen() const {
	return _penAxis;
}

inline const QPen &Axis::gridPen() const {
	return _penGridTicks;
}

inline const QPen &Axis::subGridPen() const {
	return _penGridSubticks;
}


}
}


#endif
