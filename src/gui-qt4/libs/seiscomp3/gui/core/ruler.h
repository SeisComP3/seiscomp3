/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/



#ifndef _RULER_H_
#define _RULER_H_

#include <seiscomp3/gui/qt4.h>

#include <QtGui/QFrame>

namespace Seiscomp {
namespace Gui {

class SC_GUI_API Ruler : public QFrame
{
	Q_OBJECT

	public:
		enum Position { Bottom, Top, Left, Right };

		Ruler(QWidget* = 0, Qt::WindowFlags f = 0, Position pos = Bottom);
		~Ruler() {}

		void setPosition(Position);
		void setRange(double, double);
		void setLimits(double leftValue, double rightValue, double minRange, double maxRange);
		void setScale(double);
		bool setSelected(double, double);
		bool setSelectionHandle(int handle, double pos);
		void setAnnot(double = -1); // -1 is "auto", 0 is none
		void setTicks(double = -1); // -1 is "auto", 0 is none

		void setSelectionHandleCount(int);

		bool isSelectionEnabled() const { return _enableSelection; }
		int selectionHandleCount() const;
		double selectionHandlePos(int) const;

		void setWheelEnabled(bool scale, bool translate);

		double minimumSelection() const;
		double maximumSelection() const;

		double dA() const { return _drx[0]; }
		double dT() const { return _drx[1]; }
		double dOfs() const { return _ofs; }

		virtual QSize sizeHint() const;


	public slots:
		void showRange(double, double);
		void translate(double);
		void setAutoScaleEnabled(bool);
		void setSelectionEnabled(bool);

		void setRangeSelectionEnabled(
		       bool enabled,
		       bool emitRangeChangeWhileDragging = false
		);

		void changed(int pos = -1);


	signals:
		void scaleChanged(double);
		void changedSelection(double, double);
		void selectionHandleMoved(int handle, double pos, Qt::KeyboardModifiers);
		void selectionHandleMoveFinished();
		void changedInterval(double dA, double dT, double ofs);
		void dragged(double diffTime);
		void dragStarted();
		void dragFinished();

		void rangeChangeRequested(double tmin, double tmax);


	protected:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);

		//! Should be reimplemented in derived classes to
		//! customize the displayed string. str holds the string
		//! used for displaying and value describes the position
		//! on the ruler.
		virtual bool getTickText(double pos, double lastPos,
		                         int line, QString &str) const;
		virtual void updateIntervals();

		void enterEvent(QEvent *e);
		void leaveEvent(QEvent *e);

		void setLineCount(int);
		virtual void drawSelection(QPainter &p);
		void drawRangeSelection(QPainter &p);

		//! Functions for position independent drawing
		bool isBottom() const { return _position == Bottom; }
		bool isTop() const { return _position == Top; }
		bool isLeft() const { return _position == Left; }
		bool isRight() const { return _position == Right; }
		bool isHorizontal() const { return isBottom() || isTop(); }
		bool isVertical() const { return !isHorizontal(); }
		int rulerWidth() const { return isHorizontal() ? width() : height(); }
		int rulerHeight() const { return isHorizontal() ? height() : width(); }
		//! Converts ruler position to point in widget coordinates, rx is the
		//! position on the ruler, ry the distance from the rulers baseline
		QPoint r2wPos(int rx, int ry) const;
		//! Converts widget coordinates to ruler position
		QPoint w2rPos(int x, int y) const;
		//! Converts ruler rectangle to rectangle in widget coordinates.
		//! rx is the position on the ruler, ry the distance from the rulers
		//! baseline
		QRect r2wRect(int rx, int ry, int rw, int rh) const;
		//! Draws text at the specified ruler position (rx) with
		//! the specified distance (ry) from the rulers baseline.
		//! If allowRotate is set to 'true' the text is rotated
		//! by 90 degrees if the ruler is in a vertical mode.
		bool rulerDrawText(QPainter &p, int rx, int ry,
		                   const QString &text, bool allowClip = false,
		                   bool allowRotate = false) const;
		bool rulerDrawTextAtLine(QPainter &p, int rx, int line,
		                         const QString &text, bool allowClip = false,
		                         bool allowRotate = false) const;

		void checkLimit(double &tmin, double &tmax);
		void changeRange(double tmin, double tmax);

	protected:
		Position _position;

		double  _ofs;
		double  _scl,
		        _min, _max,       // ruler range
		        _da,              // annotation interval
		        _dt,              // tick mark interval
		        _limitLeft, _limitRight,
		        _limitMinRange, _limitMaxRange;
		int     _pos, _tickLong, _tickShort, _lc;
		QVector<double> _selectionHandles;
		int     _currentSelectionHandle;

		double  _drx[2];   // current intervals

		int     _dragMode;
		double  _dragStart;
		int     _iDragStart;

		int     _rangemin, _rangemax;
		bool    _rangeValid;
		bool    _enableSelection;
		bool    _enableRangeSelection;
		bool    _emitRangeChangeWhileDragging;
		bool    _hover;

		bool    _wheelScale;
		bool    _wheelTranslate;

		bool    _autoScale;
};


} // ns Gui
} // ns Seiscomp

# endif // _RULER_H_
