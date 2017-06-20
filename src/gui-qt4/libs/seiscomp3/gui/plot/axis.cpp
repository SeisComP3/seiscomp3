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


#include <seiscomp3/gui/plot/axis.h>
#include <seiscomp3/math/math.h>
#include <iostream>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

double getSpacing(double width, int steps, bool onlyIntegerSpacing) {
	if ( steps <= 0 ) return 0;

	double fSpacing = width / ((double)steps+1e-10);
	int pow10 = int(floor(log10(fSpacing)));
	fSpacing /= pow(10, pow10);
	int spacing = (int)Seiscomp::Math::round(fSpacing);

	// Not multiple of 5
	if ( spacing % 5 ) {
		if ( !pow10 )
			spacing = (spacing+4)/5 * 5;
		else if ( spacing > 2 )
			spacing = 5;
	}

	if ( onlyIntegerSpacing && (pow10 < 0) ) {
		pow10 = 0;
		spacing = 1;
	}

	return (double)spacing * pow(10, pow10);
}


double sLogBase(double val, double base) {
	return log(val) / log(base);
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Axis::Axis(QObject *parent) : QObject(parent) {
	_visible = true;
	_grid = false;
	_position = Left;
	_tickLength = 4;
	_spacing = 6;
	_logScale = false;
	_logBase = 10;

	_penGridTicks.setWidth(1);
	_penGridTicks.setColor(QColor(192,192,192));
	_penGridTicks.setStyle(Qt::SolidLine);

	_penGridSubticks.setWidth(1);
	_penGridSubticks.setColor(QColor(224,224,224));
	_penGridSubticks.setStyle(Qt::SolidLine);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Axis::extendRange(const Range &range) {
	if ( !range.isValid() ) return;
	if ( _range.isValid() )
		_range.extend(range);
	else
		_range = range;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Axis::setLogBase(double base) {
	if ( (base <= 0) || qFuzzyCompare(base, _logBase) )
		return;

	_logBase = base;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Axis::project(double pixelValue) const {
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Axis::unproject(double axisValue) const {
	if ( _logScale ) {
		if ( axisValue <= 0 )
			return -1;

		return (sLogBase(axisValue, _logBase) - _axisStartValue) * _axisPixelScale;
	}
	else
		return (axisValue - _axisStartValue) * _axisPixelScale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Axis::updateLayout(QPainter &painter, QRect &rect) {
#if QT_VERSION >= 0x040300
	int fontHeight = painter.fontMetrics().ascent();
	int fontDescent = painter.fontMetrics().descent();
	bool isHorizontal = _position == Top || _position == Bottom;
	int tickCount;
	double lowerRange, upperRange;

	double scale;

	_width = isHorizontal ? rect.width() : rect.height();
	_extent = -1;
	_transform = QTransform();
	_ticks.clear();
	_subTicks.clear();

	if ( isHorizontal )
		tickCount = 10;
	else {
		// Get maximum tick count based on fontHeight
		tickCount = qMax(_width / fontHeight / 3, 3);
	}

	if ( _range.isValid() ) {
		if ( !_logScale ) {
			lowerRange = _range.lower;
			upperRange = _range.upper;
		}
		else {
			if ( _range.lower > 0 && _range.upper > 0 ) {
				lowerRange = sLogBase(_range.lower, _logBase);
				upperRange = sLogBase(_range.upper, _logBase);
			}
			/*
			else if ( _range.lower < 0 && _range.upper < 0 ) {
				lowerRange = sLogBase(-_range.lower, _logBase);
				upperRange = sLogBase(-_range.upper, _logBase);
			}
			*/
			else {
				// Do not produce ticks, unsupported range
				lowerRange = upperRange = 0;
			}
		}
	}
	else
		lowerRange = upperRange = 0;

	_tickSpacing = getSpacing(upperRange-lowerRange, tickCount, _logScale);
	_tickStart = int(floor(lowerRange / _tickSpacing)) * _tickSpacing;
	if ( lowerRange == upperRange )
		scale = 1;
	else
		scale = (_width-1) / (upperRange-lowerRange);
	if ( _tickStart < lowerRange ) _tickStart += _tickSpacing;

	// Epsilon for checking of equality against tick positions
	double epsilon = _tickSpacing * 1E-2;

	if ( isHorizontal ) {
		_extent = _tickLength + _spacing + painter.fontMetrics().ascent();
		if ( !_label.isEmpty() )
			_extent += _spacing + painter.fontMetrics().ascent() + painter.fontMetrics().descent();

		rect.setHeight(_extent);

		if ( _position == Bottom ) {
			rect.moveTop(rect.top()-_extent);
			_transform.translate(rect.left(), rect.top());
		}
		else {
			_transform.translate(rect.left(), rect.bottom());
			_transform.scale(1, -1);
		}
	}
	else {
		// Get maximum text width of all labels displayed
		double s0i = _tickStart;

		_extent = 0;

		if ( _tickSpacing > 0 ) {
			while ( s0i <= upperRange ) {
				if ( s0i < lowerRange ) {
					s0i += _tickSpacing;
					continue;
				}

				// Avoid zero is being displayed as -5.54523E-17
				double value = s0i;
				if ( fabs(value) < epsilon )
					value = 0;

				int s0Width = painter.fontMetrics().width(QString::number(_logScale ? pow(_logBase, value) : value));
				if ( s0Width > _extent )
					_extent = s0Width;
				s0i += _tickSpacing;
			}
		}

		_extent += _tickLength + _spacing;

		if ( !_label.isEmpty() )
			_extent += _spacing + fontHeight + fontDescent;

		rect.setWidth(_extent);

		if ( _position == Left )
			_transform.translate(rect.right(), rect.bottom());
		else {
			rect.moveLeft(rect.left()-_extent);
			_transform.translate(rect.left(), rect.bottom());
		}

		_transform.rotate(-90);
		_transform.scale(1, _position == Left ? -1 : 1);
	}

	_axisStartValue = lowerRange;
	_axisEndValue   = upperRange;
	_axisPixelScale = scale;

	int x0, y0;

	if ( _tickSpacing > 0 ) {
		bool firstTick = true;
		double s0 = _tickStart;

		while ( s0 <= (upperRange+epsilon) ) {
			int tickPos = (int)((s0 - lowerRange) * scale);
			if ( tickPos < 0 ) {
				s0 += _tickSpacing;
				continue;
			}

			if ( _logScale ) {
				if ( firstTick ) {
					for ( int j = _logBase-1; j >= 2; --j ) {
						double subS0 = s0-_tickSpacing+sLogBase(j, _logBase);
						int subTickPos = (int)((subS0-lowerRange)*scale);
						if ( subTickPos < 0 ) break;
						_transform.map(subTickPos, 0, &x0, &y0);
						_subTicks.append(Tick(subS0, subTickPos, isHorizontal ? x0 : y0));
					}
				}

				for ( int j = 2; j < _logBase; ++j ) {
					double subS0 = s0+sLogBase(j, _logBase);
					int subTickPos = (int)((subS0-lowerRange)*scale);
					if ( subTickPos > _width ) continue;
					_transform.map(subTickPos, 0, &x0, &y0);
					_subTicks.append(Tick(subS0, subTickPos, isHorizontal ? x0 : y0));
				}
			}

			_transform.map(tickPos, 0, &x0, &y0);
			_ticks.append(Tick(s0, tickPos, isHorizontal ? x0 : y0));

			s0 += _tickSpacing;
			firstTick = false;
		}
	}
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Axis::sizeHint(QPainter &painter) const {
	if ( _position == Top || _position == Bottom ) {
		int height = _tickLength + _spacing + painter.fontMetrics().ascent();
		if ( !_label.isEmpty() )
			height += _spacing + painter.fontMetrics().ascent() + painter.fontMetrics().descent();
		return height;
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Axis::draw(QPainter &painter, const QRect &rect) {
#if QT_VERSION >= 0x040300
	if ( !_visible ) return;

	painter.setPen(_penAxis);

	int fontHeight = painter.fontMetrics().ascent();
	int fontDescent = painter.fontMetrics().descent();
	bool isHorizontal = _position == Top || _position == Bottom;
	int axisLabelOffset = 0;
	Qt::Alignment labelAlign;

	if ( isHorizontal ) {
		if ( rect.width() < 1 )
			return;

		axisLabelOffset = rect.height() - fontHeight - fontDescent;

		if ( _position == Top )
			labelAlign = Qt::AlignHCenter | Qt::AlignBottom;
		else
			labelAlign = Qt::AlignHCenter | Qt::AlignTop;
	}
	else {
		if ( rect.height() < 1 )
			return;

		axisLabelOffset = rect.width() - fontHeight - fontDescent;

		if ( _position == Left )
			labelAlign = Qt::AlignRight | Qt::AlignVCenter;
		else
			labelAlign = Qt::AlignLeft | Qt::AlignVCenter;
	}

	int x0,y0,x1,y1;
	_transform.map(0, 0, &x0, &y0);
	_transform.map(_width, 0, &x1, &y1);

	painter.drawLine(x0,y0,x1,y1);

	if ( _tickSpacing > 0 ) {
		double epsilon = _tickSpacing * 1E-2;

		for ( int i = 0; i < _subTicks.count(); ++i ) {
			int subTickPos = _subTicks[i].relPos;
			_transform.map(subTickPos, 0, &x0, &y0);
			_transform.map(subTickPos, _tickLength/2, &x1, &y1);
			painter.drawLine(x0,y0,x1,y1);
		}

		for ( int i = 0; i < _ticks.count(); ++i ) {
			int tickPos = _ticks[i].relPos;

			_transform.map(tickPos, 0, &x0, &y0);
			_transform.map(tickPos, _tickLength, &x1, &y1);
			painter.drawLine(x0,y0,x1,y1);

			// Project text base
			_transform.map(tickPos, _tickLength + _spacing, &x0, &y0);

			// Avoid zero is being displayed as -5.54523E-17
			double value = _ticks[i].value;
			if ( fabs(value) < epsilon )
				value = 0;

			QString label = QString::number(_logScale ? pow(_logBase, value) : value);
			QRect labelRect = painter.fontMetrics().boundingRect(label);
			labelRect.adjust(-1,0,1,0);
			if ( labelAlign & Qt::AlignLeft )
				labelRect.moveLeft(x0);
			else if ( labelAlign & Qt::AlignRight )
				labelRect.moveRight(x0);
			else if ( labelAlign & Qt::AlignHCenter )
				labelRect.moveLeft(x0-labelRect.width()/2);

			if ( labelAlign & Qt::AlignTop )
				labelRect.moveTop(y0);
			else if ( labelAlign & Qt::AlignBottom )
				labelRect.moveBottom(y0);
			else if ( labelAlign & Qt::AlignVCenter )
				labelRect.moveTop(y0-labelRect.height()/2);

			painter.drawText(labelRect, labelAlign, label);
		}
	}

	if ( !_label.isEmpty() ) {
		painter.save();

		QRect labelRect = painter.fontMetrics().boundingRect(_label);

		_transform.map(_width/2, axisLabelOffset, &x0, &y0);

		if ( isHorizontal ) {
			labelAlign = Qt::AlignHCenter;
			labelRect.moveLeft(x0-labelRect.width()/2);

			if ( _position == Top ) {
				labelAlign |= Qt::AlignBottom;
				labelRect.moveBottom(y0);
			}
			else {
				labelAlign |= Qt::AlignTop;
				labelRect.moveTop(y0);
			}
		}
		else {
			labelAlign = Qt::AlignHCenter | Qt::AlignBottom;
			labelRect.moveLeft(-labelRect.width()/2);
			labelRect.moveBottom(0);

			painter.translate(x0, y0);
			painter.rotate(_position == Left ? -90 : 90);
		}

		painter.drawText(labelRect, labelAlign, _label);

		painter.restore();
	}
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Axis::drawGrid(QPainter &painter, const QRect &rect, bool ticks, bool subTicks) {
	bool isHorizontal = _position == Top || _position == Bottom;

	if ( isHorizontal ) {
		if ( ticks ) {
			int c = _ticks.count();
			painter.setPen(_penGridTicks);
			for ( int i = 0; i < c; ++i ) {
				if ( _ticks[i].absPos >= rect.left() && _ticks[i].absPos < rect.right() )
					painter.drawLine(_ticks[i].absPos, rect.top(), _ticks[i].absPos, rect.bottom());
			}
		}

		if ( subTicks ) {
			int c = _subTicks.count();
			painter.setPen(_penGridSubticks);
			for ( int i = 0; i < c; ++i ) {
				if ( _subTicks[i].absPos >= rect.left() && _subTicks[i].absPos < rect.right() )
					painter.drawLine(_subTicks[i].absPos, rect.top(), _subTicks[i].absPos, rect.bottom());
			}
		}
	}
	else {
		if ( ticks ) {
			int c = _ticks.count();
			painter.setPen(_penGridTicks);
			for ( int i = 0; i < c; ++i ) {
				if ( _ticks[i].absPos >= rect.top() && _ticks[i].absPos < rect.bottom() )
					painter.drawLine(rect.left(), _ticks[i].absPos, rect.right(), _ticks[i].absPos);
			}
		}

		if ( subTicks ) {
			int c = _subTicks.count();
			painter.setPen(_penGridSubticks);
			for ( int i = 0; i < c; ++i ) {
				if ( _subTicks[i].absPos >= rect.top() && _subTicks[i].absPos < rect.bottom() )
					painter.drawLine(rect.left(), _subTicks[i].absPos, rect.right(), _subTicks[i].absPos);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
