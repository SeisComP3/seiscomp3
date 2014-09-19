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


#include "tracewidget.h"


namespace Seiscomp {
namespace Gui {


TraceWidget::TraceWidget(QWidget *parent, Qt::WFlags f)
: QFrame(parent, f) {
	_max = 0;
	_count = 0;
	_offset = 0;
	_sum = 0;
	_collectedValues = 0;
}


void TraceWidget::setValueCount(int count) {
	clear();
	_values.resize(count);
}


int TraceWidget::valueCount() const {
	return _values.size();
}


void TraceWidget::addValue(int value) {
	if ( _count == _values.size() ) {
		_values[_offset++] = value;
		if ( _offset == _values.size() )
			_offset = 0;
	}
	else
		_values[_count++] = value;

	if ( value > _max )
		_max = value;

	++_collectedValues;
	_sum += value;
}


int TraceWidget::value(int index) {
	int loc = _offset + index;
	if ( loc >= _values.size() )
		loc -= _values.size();

	return _values[loc];
}


void TraceWidget::paintEvent(QPaintEvent *event) {
	QFrame::paintEvent(event);

	if ( _values.empty() ) return;

	QPainter p(this);
	QRect widgetRect(rect());

	widgetRect.setLeft(widgetRect.left() + frameWidth());
	widgetRect.setTop(widgetRect.top() + frameWidth());
	widgetRect.setWidth(widgetRect.width() - frameWidth()*2);
	widgetRect.setHeight(widgetRect.height() - frameWidth()*2);

	const int heightOffset = 10;
	int drawHeight = widgetRect.height() - heightOffset*2;
	int drawOffsetY = widgetRect.bottom() - heightOffset - 1;

	p.fillRect(widgetRect, Qt::black);

	//if ( _collectedValues == 0 ) return;

	float scaleX = (float)widgetRect.width() / (float)(_values.size()-1);
	float scaleY = 1;
	if ( _max > 0 )
		scaleY = (float)drawHeight / (float)_max;

	p.translate(0, drawOffsetY);
	p.scale(scaleX, -1);

	p.setPen(Qt::yellow);
	p.drawLine(0, (int)(scaleY*_max), _count-1, (int)(scaleY*_max));


	float mean = 0;
	if ( _collectedValues > 0 )
		mean = (float)_sum/(float)_collectedValues;

	for ( int i = 0; i < _count; ++i ) {
		int v = value(i);

		p.setBrush(Qt::darkRed);
		p.setPen(Qt::darkRed);

		if ( v > mean ) {
			p.drawRect(i-1, 0, 1, (int)(scaleY*mean));
			p.setBrush(Qt::darkGreen);
			p.setPen(Qt::darkGreen);
			p.drawRect(i-1, (int)(scaleY*mean), 1, (int)(scaleY*(v-mean)));
		}
		else
			p.drawRect(i-1, 0, 1, (int)(scaleY*v));
	}

	QColor color;
	int last_v = value(0);

	if ( last_v <= (int)mean )
		color = Qt::red;
	else
		color = Qt::green;

	p.setPen(color);
	for ( int i = 1; i < _count; ++i ) {
		int v = value(i);

		if ( (v > mean && last_v <= mean) || (last_v > mean && v <= mean) ) {
			p.drawLine(QPoint(i-1, (int)(scaleY*last_v)), QPoint(i-1, (int)(scaleY*mean)));
			
			if ( v <= (int)mean )
				color = Qt::red;
			else
				color = Qt::green;

			p.setPen(color);

			p.drawLine(QPoint(i-1, (int)(scaleY*mean)), QPoint(i-1, (int)(scaleY*v)));
		}
		else
			p.drawLine(i-1, (int)(scaleY*last_v), i-1, (int)(scaleY*v));

		p.drawLine(i-1, (int)(scaleY*v), i, (int)(scaleY*v));

		last_v = v;
	}

	//p.setPen(Qt::darkYellow);
	//p.drawLine(QPoint(0, (int)(scaleY*mean)), QPoint(_count-1, (int)(scaleY*mean)));

	p.setMatrixEnabled(false);
	p.setPen(Qt::darkYellow);
	p.drawText(widgetRect.left(),widgetRect.bottom(),QString("Max: %1, Mean: %2").arg(_max).arg(mean));
}


void TraceWidget::clear() {
	_max = 0;
	_count = 0;
	_offset = 0;
	_sum = 0;
	_collectedValues = 0;
}


}
}
