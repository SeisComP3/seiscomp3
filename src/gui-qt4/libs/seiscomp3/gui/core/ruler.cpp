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




#include <iostream>
#include <float.h>
#include <math.h>

#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QStyleOptionFocusRect>

#include "ruler.h"

#define CHCK255(x)                      ((x)>255?255:((x)<0?0:(x)))

namespace Seiscomp {
namespace Gui {

Ruler::Ruler(QWidget *parent, Qt::WindowFlags f, Position pos)
 : QFrame(parent, f) {
	setFrameStyle(QFrame::Panel | QFrame::Plain);
	setLineWidth(0);
	setPosition(pos);
	_scl = 1.;
	_da = _dt = -1.;
	_min = _max = 0.;
	_pos = 0;
	_ofs = 0;
	_rangemin = _rangemax = 0;

	_dragMode = 0;
	_hover = false;
	_autoScale = false;
	_enableRangeSelection = false;
	_currentSelectionHandle = -1;
	_emitRangeChangeWhileDragging = false;

	_wheelScale = _wheelTranslate = true;

	setLineCount(1);
	setSelectionHandleCount(2);
	setSelectionEnabled(false);
}

void Ruler::setLineCount(int lc) {
	_lc = lc < 1 ? 1 : lc;
	int fontHeight = fontMetrics().height();

	_tickLong  = fontHeight/2+1;
	_tickShort = fontHeight/4+1;

	int h = _lc*fontHeight + 4*(_lc-1) + (4+_tickLong);
	isHorizontal() ? setFixedHeight(h) : setFixedWidth(h);
}

void Ruler::setSelectionHandleCount(int cnt) {
	_selectionHandles.resize(cnt);
}

int Ruler::selectionHandleCount() const {
	return _selectionHandles.count();
}

double Ruler::selectionHandlePos(int i) const {
	return _selectionHandles[i];
}

void Ruler::setWheelEnabled(bool scale, bool translate) {
	_wheelScale = scale;
	_wheelTranslate = translate;
}


void Ruler::setPosition(Position pos) {
	_position = pos;
}

void Ruler::setScale(double scl) {
	_scl = scl;
	emit scaleChanged(_scl);
	updateIntervals();
	update();
}

void Ruler::setRange(double min, double max) {
	_min = min;
	_max = max;

	if ( _autoScale ) {
		if ( _max-_min > 0 )
			setScale(rulerWidth()/(_max-_min));
		else
			update();
	}
	else {
		updateIntervals();
		update();
	}
}


void Ruler::showRange(double min, double max) {
	_min = min;
	_max = max;

	if ( _max-_min > 0 )
		setScale(rulerWidth()/(_max-_min));
	else
		update();
}


void Ruler::translate(double tx) {
	_min += tx;
	_max += tx;
	updateIntervals();
	update();
}

bool Ruler::setSelected(double smin, double smax) {
	if ( _selectionHandles.count() != 2 ) return false;

	_selectionHandles[0] = smin;
	_selectionHandles[1] = smax;
	qSort(_selectionHandles.begin(), _selectionHandles.end());
	update();
	return true;
}


bool Ruler::setSelectionHandle(int handle, double pos) {
	if ( handle < 0 || handle >= _selectionHandles.count() )
		return false;

	_selectionHandles[handle] = pos;
	//qSort(selectionHandles.begin(), selectionHandles.end());

	update();
	return true;
}


void Ruler::setSelectionEnabled(bool enable) {
	_enableSelection = enable;
	setMouseTracking(enable);
	update();
}

void Ruler::setRangeSelectionEnabled(bool enable,
                                     bool emitRangeChangeWhileDragging) {
	_enableRangeSelection = enable;
	_emitRangeChangeWhileDragging = emitRangeChangeWhileDragging;
}

void Ruler::setAutoScaleEnabled(bool e) {
	_autoScale = e;
}

void Ruler::setAnnot(double da) {
	if ( _da == da) return;

	_da = _drx[0] = da;
	update();
}

void Ruler::setTicks(double dt) {
	if ( _dt == dt) return;

	_dt = _drx[1] = dt;
	update();
}

double Ruler::minimumSelection() const {
	return _enableSelection ? _selectionHandles.front() : 0;
}

double Ruler::maximumSelection() const {
	return _enableSelection ? _selectionHandles.back() : 0;
}

void Ruler::changed(int pos) {
	_pos = pos;
	update();
}


void Ruler::drawSelection(QPainter &p) {
	static QPoint marker[3] = { QPoint(0, 0), QPoint(0, 0), QPoint(0, 0) };
	if ( !_enableSelection ) return;

	p.save();
	p.setRenderHints(QPainter::Antialiasing, true);
	int selHeight = _tickLong * 1.5;
	int selHalfWidth = selHeight * 0.5;

	p.setBrush(palette().color(QPalette::WindowText));
	for ( int i = 0; i < _selectionHandles.count(); ++i ) {
		if ( ( _hover || _dragMode > 0 ) && _enableSelection &&
		     i == _currentSelectionHandle ) continue;
		int iPos = int((_selectionHandles[i]-_min)*_scl);
		marker[0] = r2wPos(iPos-selHalfWidth, selHeight);
		marker[1] = r2wPos(iPos+selHalfWidth, selHeight);
		marker[2] = r2wPos(iPos,0);
		p.drawPolygon(marker, 3);
	}
	if ( ( _hover || _dragMode > 0 ) && _enableSelection &&
	     _currentSelectionHandle >= 0 &&
	     _currentSelectionHandle < _selectionHandles.count() ) {
		p.setBrush(palette().color(QPalette::BrightText));
		int iPos = int((_selectionHandles[_currentSelectionHandle]-_min)*_scl);
		marker[0] = r2wPos(iPos-selHalfWidth, selHeight);
		marker[1] = r2wPos(iPos+selHalfWidth, selHeight);
		marker[2] = r2wPos(iPos,0);
		p.drawPolygon(marker, 3);
	}
	p.restore();
}

void Ruler::drawRangeSelection(QPainter &p) {
	if ( _rangemin != _rangemax ) {
		QRect rect;
		int offset = _tickLong + 1;
		int height = fontMetrics().height()+1;

		QLinearGradient filler(r2wPos(0, offset), r2wPos(0, offset+height));
		if ( _rangemin < _rangemax ) {
			filler.setColorAt(0, QColor(0, 128, 0, 128));
			filler.setColorAt(0.5, QColor(0, 255, 0, 128));
			filler.setColorAt(1, QColor(0, 128, 0, 128));
			rect = r2wRect(_rangemin, offset, _rangemax-_rangemin, height);
		}
		else {
			filler.setColorAt(0, QColor(128, 0, 0, 128));
			filler.setColorAt(0.5, QColor(255, 0, 0, 128));
			filler.setColorAt(1, QColor(128, 0, 0, 128));
			rect = r2wRect(_rangemax, offset, _rangemin-_rangemax, height);
		}

		if ( !_rangeValid ) {
			filler.setColorAt(0, QColor(0, 0, 0, 128));
			filler.setColorAt(0.5, QColor(255, 255, 255, 128));
			filler.setColorAt(1, QColor(0, 0, 0, 128));
		}
		p.fillRect(rect, filler);
		p.drawLine(r2wPos(_rangemin, 0), r2wPos(_rangemin, offset+height-1));
		p.drawLine(r2wPos(_rangemax, 0), r2wPos(_rangemax, offset+height-1));

		p.setBrush(Qt::NoBrush);
	}
}

QPoint Ruler::r2wPos(int rx, int ry) const {
	return QPoint(isHorizontal() ? rx : isRight() ? ry : width()-ry-1,
	              isVertical() ? height()-rx-1 : isBottom() ? ry : height()-ry-1);
}

QPoint Ruler::w2rPos(int x, int y) const {
	return QPoint(isHorizontal() ? x : height()-y-1,
		      isBottom() ? y : isTop() ? height()-y-1 : isRight() ? x : width()-x-1);
}

QRect Ruler::r2wRect(int rx, int ry, int w, int h) const {
	return QRect(isHorizontal() ? rx : isRight() ? ry : width()-ry-h-1,
	             isVertical() ? height()-rx-w-1 : isBottom() ? ry : height()-ry-h-1,
		     isHorizontal() ? w : h,
		     isHorizontal() ? h : w);
}

bool Ruler::rulerDrawText(QPainter &p, int rx, int ry, const QString &text,
                          bool allowClip, bool allowRotate) const {
	// Text width and height
	int tw = p.fontMetrics().width(text);
	int th = p.fontMetrics().height();

	// Top/left position of text box in widget coordinates
	QPoint pos;
	if ( isHorizontal() || allowRotate )
		pos = r2wPos(rx-tw/2, isTop() || isLeft() ? ry+th : ry);
	else
		pos = r2wPos(rx+th/2, isRight() ? ry : ry+tw);


	bool rotate = isVertical() && allowRotate;
	// Is text clipped?
	QRect rect(0, 0, width(), height());
	QPoint pos2(pos.x() + (rotate?th:tw), pos.y() + (rotate?-tw:th));
// 	std::cout << "rotate/tw,th/rect/pos/pos2: " << rotate << " / "
// 		<< tw << "," << th << " / "
// 		<< rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height() << " / "
// 		<< pos.x() << "," << pos.y() << " / "
// 		<< pos2.x() << "," << pos2.y() << std::endl;
	if ( !allowClip && (!rect.contains(pos) || !rect.contains(pos2)) )
		return false;

	int flags = Qt::AlignCenter | Qt::AlignBottom;
	if ( rotate ) {
		p.save();
		p.translate(pos);
		p.rotate(-90);
		p.drawText(0, 0, tw, th, flags, text);
		p.restore();
	}
	else {
		p.drawText(pos.x(), pos.y(), tw, th, flags, text);
	}
	return true;
}

bool Ruler::rulerDrawTextAtLine(QPainter &p, int rx, int line, const QString &text,
                                bool allowClip, bool allowRotate) const {
	if ( line >= _lc ) return false;

	int tickOffset = 2 + _tickLong;
	int lineHeight = fontMetrics().height();
	if ( isHorizontal() || allowRotate )
		return rulerDrawText(p, rx, tickOffset + line*lineHeight, text, allowClip, allowRotate);
	else {
		int rxOffset = (_lc-1)/2 - line;
		return rulerDrawText(p, rx + rxOffset, tickOffset, text, allowClip, allowRotate);
	}
}

void Ruler::paintEvent(QPaintEvent *e) {
	QFrame::paintEvent(e);

	QPainter painter(this);

	drawRangeSelection(painter);

	if ( _scl > 0 ) {
		double pos = _min + _ofs;

		for ( int k = 0; k < 2; ++k ) {
			if ( _drx[k] <= 0 ) continue; // no ticks/annotations

			double cpos = pos - fmod(pos, (double)_drx[k]);

			int tick = k == 0 ? _tickLong : _tickShort;

			// Draw ruler base line
			int rw = rulerWidth();
			painter.drawLine(r2wPos(0, 0), r2wPos(rw, 0));

			// Draw ticks and counts
			int rx = (int)((cpos-pos)*_scl);
			QString str;
			QVector<double> lastPos(_lc, -DBL_MAX);
			while ( rx < rw ) {
				painter.drawLine(r2wPos(rx, 0), r2wPos(rx, tick));
				if ( k == 0 ) {
					for ( int l = 0; l < _lc; ++l ) {
						if ( getTickText(cpos, lastPos[l], l, str) &&
							 rulerDrawTextAtLine(painter, rx, l, str))
							lastPos[l] = cpos;
					}
				}

				cpos += _drx[k];
				rx = (int)((cpos-pos)*_scl);
			}
		}
	}

	drawSelection(painter);

	painter.end();
}


bool Ruler::getTickText(double pos, double lastPos, int line, QString &str) const {
	if ( line != 0 )
		return false;

	str.setNum(pos);
	return true;
}

void Ruler::enterEvent(QEvent *e) {
	_hover = true;
	update();
}

void Ruler::leaveEvent(QEvent *e) {
	_hover = false;
	update();
}

void Ruler::mousePressEvent(QMouseEvent *e) {
	// Already in a drag mode?
	if ( _dragMode != 0 ) return;

	QPoint rp = w2rPos(e->x(), e->y());
	int rx = rp.x();
	_iDragStart = rx;
	_dragStart = (_pos+rx)/_scl + _min;

	if ( e->button() == Qt::LeftButton ) {
		if ( _enableSelection && _currentSelectionHandle >= 0 )
			_dragMode = _currentSelectionHandle + 1;
		else {
			_dragMode = -1;
			emit dragStarted();
			QFrame::mousePressEvent(e);
			return;
		}

		update();
	}
	else if ( e->button() == Qt::RightButton ) {
		if ( _enableRangeSelection ) {
			_dragMode = -2;
			_rangemin = rx;
			_rangemax = rx;
			_rangeValid = false;
		}
	}

	QFrame::mousePressEvent(e);
}

void Ruler::mouseReleaseEvent(QMouseEvent *e) {
	if ( e->button() == Qt::LeftButton ) {
		if ( _dragMode == -1 )
			emit dragFinished();
		else if ( _dragMode > 0 )
			emit selectionHandleMoveFinished();
		_dragMode = 0;
	}
	else if ( e->button() == Qt::RightButton ) {
		// Reset range selection
		if ( _rangeValid ) {
			double smin = (_pos+_rangemin) / _scl + _min;
			double smax = (_pos+_rangemax) / _scl + _min;

			if ( smin < smax )
				emit rangeChangeRequested(smin, smax);
			else {
				std::swap(smin, smax);
				double max = rulerWidth() / _scl + _min;
				double s = (max-_min) / (smax-smin);
				double tmin = s * (_min-smin) + _min;
				double tmax = s * (max-smax) + max;
				emit rangeChangeRequested(tmin, tmax);
			}
		}
		_rangemin = _rangemax = 0;
		update();
		_dragMode = 0;
	}
}

void Ruler::mouseMoveEvent(QMouseEvent *e) {
	if ( _dragMode == 0 ) {
		if ( _enableSelection ) {
			// check if mouse is over selection handle and if so, determine best
			// matching handle and highlight it
			QPoint rp = w2rPos(e->x(), e->y());
			int rx = rp.x();
			int ry = rp.y();
			int selHeight = _tickLong * 1.5;
			int selHalfWidth = selHeight * 0.5;
			int index = -1;
			if ( rx >= 0 && rx <= rulerWidth() && ry >= 0 && ry <= selHeight ) {
				int minDist = rulerWidth();
				for ( int i = 0; i < _selectionHandles.count(); ++i ) {
					int iPos = int((_selectionHandles[i] - _min) * _scl);
					int dist = abs(iPos - rx);
					if ( dist <= selHalfWidth && dist < minDist ) {
						minDist = dist;
						index = i;
					}
				}
			}
			if ( index != _currentSelectionHandle ) {
				_currentSelectionHandle = index;
				update();
			}
		}
		return;
	}

	// position on the ruler
	QPoint rPoint = w2rPos(e->x(), e->y());
	int rx = rPoint.x();
	double p = (_pos+rx) / _scl + _min;
	double dragOffset = _iDragStart - rx;

	if ( _dragMode == -1 ) {
		double fDragOffset = double(dragOffset) / _scl;
		_dragStart = p;
		_iDragStart = rx;
		emit dragged(fDragOffset);
		if ( _enableRangeSelection && _emitRangeChangeWhileDragging )
			emit rangeChangeRequested(_min+fDragOffset, _max+fDragOffset);
	}
	else if ( _dragMode == -2 ) {
		_rangemax = rx < 0 ? 0 : rx >= rulerWidth() ? rulerWidth()-1 : rx;
		_rangeValid = _rangemax != _rangemin &&
		              rPoint.y() >= 0 && rPoint.y() < rulerHeight();
		update();
	}
	else if ( _dragMode > 0 ) {
		if ( _enableSelection ) {
			int idx = _dragMode - 1;

			// Clip to previous handle
			if ( (idx > 0) && (p < _selectionHandles[idx-1]) )
				p = _selectionHandles[idx-1];

			// Clip to next handle
			if ( (idx < _selectionHandles.count() -1 ) &&
			     (p > _selectionHandles[idx+1]) )
				p = _selectionHandles[idx+1];

			_selectionHandles[idx] = p;

			if ( _selectionHandles.count() == 2 )
				emit changedSelection(_selectionHandles[0], _selectionHandles[1]);

			emit selectionHandleMoved(idx, _selectionHandles[idx], e->modifiers());
			update();
		}
	}
}

void Ruler::wheelEvent(QWheelEvent *event) {
	if ( !event || (!_wheelScale && !_wheelTranslate) )
		return;

	// shift modifier works as an amplifier
	int delta = event->modifiers() & Qt::ShiftModifier ?
	            event->delta() * 2 : event->delta() / 4;

	// scale (Ctrl key)
	if ( event->modifiers() & Qt::ControlModifier ) {
		QPoint rp = w2rPos(event->x(), event->y());
		double center = (double)(rp.x() + _pos) / rulerWidth();
		double ofs = delta / _scl;
		emit rangeChangeRequested(_min + ofs * center, _max - ofs * (1-center));
	}
	// translate
	else {
		double ofs = delta / _scl;
		emit rangeChangeRequested(_min + ofs, _max + ofs);
	}

	event->accept();
}

void Ruler::resizeEvent(QResizeEvent *e) {
	if ( _autoScale ) {
		if ( _max-_min > 0 ) {
			if ( rulerWidth() > 0 )
				setScale(rulerWidth() / (_max-_min));
		}
		else
			updateIntervals();
	}
	else
		updateIntervals();
}

void Ruler::updateIntervals() {
	bool changed = false;
	double max = _min + rulerWidth() / _scl;
	_max = max;

	double tmpdx[] = { _da, _dt };

	for ( int k = 0; k < 2; ++k ) {
		if ( tmpdx[k] < -0.1 ) {
			changed = true;

			// compute adequate tick/annotation interval
			// the 1st factor is for fine-tuning
			double textDim = isHorizontal() ?
			                 fontMetrics().width(" XX:XX:XX ") :
			                 fontMetrics().height()*2;
			double q = log10(2*(max-_min)*textDim/rulerWidth());
			double rx = q - floor(q);
			int d = rx < 0.3 ? 1 : rx > 0.7 ? 5 : 2;
			_drx[0] = d * pow (10., int(q-rx));

			switch (d) {
				case 1: _drx[1] = 0.20 * _drx[0];  break;
				case 2: _drx[1] = 0.25 * _drx[0];  break;
				case 5: _drx[1] = 0.20 * _drx[0];  break;
				default: break;
			}
		}
	}

	if ( changed )
		emit changedInterval(_drx[0], _drx[1], _ofs);
}

} // ns Gui
} // ns Seiscomp
