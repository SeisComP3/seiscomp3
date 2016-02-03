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


#include "diagramwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QHBoxLayout>
#include <cmath>
#include <iostream>
#include <cstdio>

#include <seiscomp3/math/math.h>

#define SEISCOMP_COMPONENT Gui::DiagramWidget
#include <seiscomp3/logging/log.h>
#include <seiscomp3/gui/core/scheme.h>


#define LOW_BORDER  60.0
#define HIGH_BORDER 105.0


using namespace std;


namespace {

double getSpacing(double width, int steps) {
	double fSpacing = width / steps;
	double pow10 = int(floor(log10(fSpacing)));
	fSpacing /= pow(10, pow10);
	int spacing = (int)Seiscomp::Math::round(fSpacing);

	// Round to even number if not equal than 5
	/*if ( spacing > 5 ) spacing = 10;
	else */if ( spacing != 5 && (spacing % 2) )
		++spacing;

	return (double)spacing * pow(10, pow10);
}

}


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DiagramWidget::DiagramWidget(QWidget * parent, Type type, Qt::WFlags f)
 : QWidget(parent, f) {
	setType(type);

	_hideDisabledValues = false;
	_drawGridLines = false;
	_mode = SelectActiveState;

	_columns = 2;
	_xIndex = 0; _yIndex = 1;

	_horMargin = 0;
	_verMargin = 0;
	_tickLength  = 1;
	_tickLengthA = 2;
	_annotOffset = 5;
	_textHeight  = 0;
	_dragging = false;
	_dragStart = false;
	_indicesChanged = false;
	_background = Qt::white;
	_hoverId = -1;
	_markerDistance = QPointF(0,0);
	setMouseTracking(true);
	_disabledColor = Qt::red;
	_rubberBandOperation = Select;

	_zoomAction = new QAction(this);
	_zoomAction->setText("Zoom into selected values");
	addAction(_zoomAction);

	_resetAction = new QAction(this);
	_resetAction->setText("Reset zoom");
	addAction(_resetAction);

	/*
	_selectAbscissaInterval = new QAction(this);
	_selectAbscissaInterval->setText("Select abscissa interval");
	_selectAbscissaInterval->setEnabled(false);
	addAction(_selectAbscissaInterval);

	_selectOrdinateInterval = new QAction(this);
	_selectOrdinateInterval->setText("Select ordinate interval");
	_selectOrdinateInterval->setEnabled(false);
	addAction(_selectOrdinateInterval);
	*/

	_actionActivate = new QAction(this);
	_actionActivate->setText("Select 'active' state");

	_actionEnable = new QAction(this);
	_actionEnable->setText("Select 'enable' state");

	setFocusPolicy(Qt::ClickFocus);

	connect(_zoomAction, SIGNAL(triggered(bool)),
	        this, SLOT(zoomIntoSelectedValues()));
	connect(_resetAction, SIGNAL(triggered(bool)),
	        this, SLOT(resetZoom()));

	/*
	connect(_selectAbscissaInterval, SIGNAL(triggered(bool)),
	        this, SLOT(selectAbscissaInterval()));
	connect(_selectOrdinateInterval, SIGNAL(triggered(bool)),
	        this, SLOT(selectOrdinateInterval()));
	*/

	connect(_actionActivate, SIGNAL(triggered(bool)),
	        this, SLOT(selectActiveState()));

	connect(_actionEnable, SIGNAL(triggered(bool)),
	        this, SLOT(selectEnableState()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DiagramWidget::~DiagramWidget() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setType(Type t) {
	_type = t;
	switch ( _type ) {
		case Rectangular:
			project = &DiagramWidget::projectRectangular;
			unProject = &DiagramWidget::unProjectRectangular;
			contains = &DiagramWidget::containsRectangular;
			adjustZoom = &DiagramWidget::adjustZoomRectangular;
			break;
		case Spherical:
			project = &DiagramWidget::projectSpherical;
			unProject = &DiagramWidget::unProjectSpherical;
			contains = &DiagramWidget::containsSpherical;
			adjustZoom = &DiagramWidget::adjustZoomSpherical;
			break;
		default:
			project = NULL;
			unProject = NULL;
			contains = NULL;
			adjustZoom = NULL;
			break;
	}

	updateDiagramArea();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setIndicies(int x, int y) {
	_xIndex = x; _yIndex = y;
	updateDiagramArea();
	updateBoundingRect();
	update();
	resetZoom();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setAbscissaName(const QString &name) {
	_nameX = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setOrdinateName(const QString &name) {
	_nameY = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::updateDiagramArea() {
	if ( _type == Rectangular ) {
		_diagramArea.setCoords(_clientArea.left() + _horMargin + _textHeight,
		                       _clientArea.top() + _verMargin + _textHeight/2,
		                       _clientArea.right() - _horMargin,
		                       _clientArea.bottom() - _verMargin - 2*_textHeight);
		_indicesChanged = true;
	}
	else
		_diagramArea.setCoords(_clientArea.left() + 3,
		                       _clientArea.top() + _verMargin + _textHeight,
		                       _clientArea.right() - 3,
		                       _clientArea.bottom() - _verMargin - _textHeight);

	diagramAreaUpdated(_diagramArea);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::resizeEvent(QResizeEvent *) {
	_clientArea = rect();
	updateDiagramArea();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::paintEvent(QPaintEvent *e) {
	QPainter painter(this);

	if ( !_textHeight ) {
		QRect rect = painter.fontMetrics().boundingRect("-000.0");
		_textHeight = rect.height() + _annotOffset;
		//_horMargin = rect.width();
		//_verMargin = _textHeight;
		updateDiagramArea();
	}

	switch ( _type ) {
		case Rectangular:
			paintRectangular(painter);
			break;
		case Spherical:
			paintSpherical(painter);
			break;
		default:
			break;
	}

	if ( _hoverId != -1 ) {
		ValueItem v = _values[_hoverId];
		if ( !v.isVisible ) _hoverId = -1;
		else if ( _displayRect.contains(v.pt(_xIndex,_yIndex)) ) {
			QColor c;
			if ( v.isActive )
				c = v.cols[_yIndex].color;
			else
				c = _disabledColor;

			painter.setPen(QPen(c, 3));
			painter.setBrush(c);

			drawValue(_hoverId, painter, (this->*project)(v.pt(_xIndex,_yIndex)),
			          v.type, v.valid(_xIndex,_yIndex));
			//painter.setBrush(Qt::red);
			//painter.drawText(_diagramArea.bottomLeft(), QString("%1,%2").arg(_values[_hoverId].first.x()).arg(_values[_hoverId].first.y()));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawValues(QPainter &painter) {
	for ( int id = 0; id < _values.size(); ++id ) {
		const ValueItem &v = _values[id];
		if ( !_displayRect.contains(v.pt(_xIndex,_yIndex)) )
			continue;

		if ( !v.isVisible ) continue;

		QColor c;
		if ( v.isActive )
			c = v.cols[_yIndex].color;
		else
			c = _disabledColor;

		painter.setPen(QColor(c.red()/2, c.green()/2, c.blue()/2));
		if ( !v.isEnabled ) {
			if ( _hideDisabledValues ) continue;
			painter.setBrush(Qt::NoBrush);
		}
		else
			painter.setBrush(c);

		drawValue(id, painter, (this->*project)(v.pt(_xIndex,_yIndex)),
		          v.type, v.valid(_xIndex,_yIndex));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::paintRectangular(QPainter &painter) {
	//if ( isEmpty() ) return;

	if ( _indicesChanged ) {
		// Compute horizontal offset depending on the width of the values
		double vGridSpacing = getSpacing(_displayRect.height(), 6);
		int textOffset = 0;

		qreal s0 = int(floor(_displayRect.top() / vGridSpacing)) * vGridSpacing;

		if ( s0 < _displayRect.top() ) s0 += vGridSpacing;

		for ( ; s0 < _displayRect.bottom(); s0 += vGridSpacing ) {
			if ( fabs(s0) < 1E-10 ) s0 = 0;
			textOffset = max(textOffset, fontMetrics().width(QString("%1").arg(s0)));
		}

		_diagramArea.setLeft(_diagramArea.left() + textOffset + _annotOffset);
		_indicesChanged = false;
	}

	QRect bak(_diagramArea);
	//_diagramArea.translate(_horMargin, -_horMargin);
	//_diagramArea.setHeight(_diagramArea.height() + _horMargin);

	painter.fillRect(_diagramArea, _background);

	qreal abscissaValue = 0;
	QPoint abscissa = (this->*project)(QPointF(0,abscissaValue));
	int abscissaHeight = abscissa.y();
	if ( (abscissaHeight < _diagramArea.top()) ||
	     (abscissaHeight > _diagramArea.bottom()) ) {
		abscissaValue = _boundingRect.top();
		abscissaHeight = _diagramArea.bottom();
	}

	QColor mainColor, subColor;

	if ( _displayRect != _requestRect ) {
		mainColor = blend(Qt::black, Qt::red, 60);
		subColor = blend(Qt::gray, Qt::red, 60);
	}
	else {
		mainColor = Qt::black;
		subColor = Qt::gray;
	}

	/*
	QColor c = blend(Qt::lightGray, _background, 50);
	QPoint p0 = (this->*project)(QPointF(std::min(LOW_BORDER, _displayRect.right()), _displayRect.bottom()));
	QPoint p1 = (this->*project)(QPointF(std::min(HIGH_BORDER, _displayRect.right()), _displayRect.top()));
	painter.fillRect(p0.x(), p0.y(), p1.x()-p0.x(), p1.y() - p0.y(), c);

	c = blend(Qt::gray, _background, 50);
	p0 = (this->*project)(QPointF(std::min(HIGH_BORDER, _displayRect.right()), _displayRect.bottom()));
	p1 = (this->*project)(QPointF(_displayRect.right(), _displayRect.top()));
	painter.fillRect(p0.x(), p0.y(), p1.x()-p0.x(), p1.y() - p0.y(), c);
	*/

	if ( _drawGridLines ) {
#ifdef Q_WS_MAC
		// For unknown reasons OSX cannot display dotted lines
		painter.setPen(QPen(QColor(192,192,192), 1, Qt::DashLine));
#else
		painter.setPen(QPen(QColor(192,192,192), 1, Qt::DotLine));
#endif

		double spacing = getSpacing(_displayRect.width(), 6);

		// Draw horizontal grid
		if ( spacing > 0 ) {
			qreal s0 = int(floor(_displayRect.left() / spacing)) * spacing;
			qreal scale = _diagramArea.width() / _displayRect.width();

			if ( s0 < _displayRect.left() ) s0 += spacing;

			while ( s0 < _displayRect.right() ) {
				int posX = _diagramArea.left() + (int)((s0 - _displayRect.left()) * scale);
				painter.drawLine(posX, _diagramArea.top(), posX, _diagramArea.bottom());
				s0 += spacing;
			}
		}

		spacing = getSpacing(_displayRect.height(), 6);

		// Draw vertical grid
		if ( spacing > 0 ) {
			qreal s0 = int(floor(_displayRect.top() / spacing)) * spacing;
			qreal scale = _diagramArea.height() / _displayRect.height();

			if ( s0 < _displayRect.top() ) s0 += spacing;

			while ( s0 < _displayRect.bottom() ) {
				int posY = _diagramArea.bottom() - (int)((s0 - _displayRect.top()) * scale);
				painter.drawLine(_diagramArea.left(), posY, _diagramArea.right(), posY);
				s0 += spacing;
			}
		}
	}

	// Draw abscissa
	painter.setPen(mainColor);
	drawAbscissa(painter, abscissaHeight, false, false);
	drawAbscissa(painter, _diagramArea.bottom(), true, true);
	painter.setPen(subColor);
	//drawAbscissa(painter, _diagramArea.top(), true, false);
	painter.drawLine(_diagramArea.left(), _diagramArea.top(), _diagramArea.right(), _diagramArea.top());

	// Draw ordinate
	painter.setPen(mainColor);
	drawOrdinate(painter, _diagramArea.left(), abscissaHeight, abscissaValue, false);
	painter.setPen(subColor);
	//drawOrdinate(painter, _diagramArea.right(), abscissaHeight, abscissaValue, true);

	painter.setPen(mainColor);
	painter.drawLine(_diagramArea.right(), _diagramArea.top(), _diagramArea.right(), _diagramArea.bottom());

	// Draw X text
	painter.drawText(_diagramArea.left(), _diagramArea.bottom()+_textHeight+_annotOffset,
	                 _diagramArea.width(), _clientArea.height(),
	                 Qt::AlignHCenter | Qt::AlignTop, _nameX);

	// Draw Y text
	painter.save();
	painter.translate(_clientArea.left()+_horMargin, _diagramArea.center().y());
	painter.rotate(-90);
	painter.drawText(-_clientArea.height()/2, 0, _clientArea.height(), _textHeight,
	                 Qt::AlignHCenter | Qt::AlignTop, _nameY);
	painter.restore();

	drawValues(painter);

	if ( _rubberBand.isValid() ) {
		QPoint min = (this->*project)(_rubberBand.bottomLeft());
		QPoint max = (this->*project)(_rubberBand.topRight());

		if ( _dragZoom ) {
			painter.setPen(QColor(96,64,32));
			painter.setBrush(QColor(255,224,192,160));
		}
		else {
			switch ( _rubberBandOperation ) {
				case SelectPlus:
					painter.setPen(QColor(32,96,64));
					painter.setBrush(QColor(192,255,224,160));
					break;
				case SelectMinus:
					painter.setPen(QColor(96,32,64));
					painter.setBrush(QColor(255,192,224,160));
					break;
				default:
					painter.setPen(QColor(32,64,96));
					painter.setBrush(QColor(192,224,255,160));
					break;
			}
		}

		painter.drawRect(min.x(), min.y(), max.x()-min.x()+1, max.y()-min.y()+1);
	}

	_diagramArea = bak;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::paintSphericalBackground(QPainter &painter) {
	QColor mainColor, subColor;

	int diameter = std::min(_diagramArea.width(), _diagramArea.height());
	int radius = diameter / 2;
	QPoint center = _diagramArea.center();

	int left = center.x() - radius;
	int top = center.y() - radius;

	painter.setPen(_background);
	painter.setBrush(_background);

	painter.setRenderHint(QPainter::Antialiasing);

	painter.drawEllipse(left-2, top-2, diameter+4, diameter+4);

	if ( _displayRect != _requestRect ) {
		mainColor = blend(Qt::black, Qt::red, 60);
		subColor = blend(Qt::gray, Qt::red, 60);
	}
	else {
		mainColor = Qt::black;
		subColor = Qt::gray;
	}

	QColor c = blend(Qt::gray, _background, 50);
	painter.setPen(c);
	painter.setBrush(c);

	painter.drawEllipse(left, top, diameter, diameter);

	{
		QColor c = blend(Qt::lightGray, _background, 50);
		painter.setPen(c);
		painter.setBrush(c);
		QPoint tmp = (this->*project)(QPointF(std::min(HIGH_BORDER, double(_displayRect.right())), 0));
		int w = tmp.y() - center.y();
		painter.drawEllipse(center.x()-w, center.y()-w, w*2, w*2);
	}

	{
		painter.setPen(_background);
		painter.setBrush(_background);
		QPoint tmp = (this->*project)(QPointF(std::min(LOW_BORDER , double(_displayRect.right())), 0));
		int w = tmp.y() - center.y();
		painter.drawEllipse(center.x()-w, center.y()-w, w*2, w*2);
	}

	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.setPen(blend(Qt::gray, _background, 25));

	if ( _displayRect.right() < 60 )
		painter.drawEllipse(left, top, diameter, diameter);

	// Draw coordinate system
	painter.drawLine(center.x()-radius, center.y(), center.x()+radius, center.y());
	painter.drawLine(center.x(), center.y()-radius, center.x(), center.y()+radius);

	painter.setPen(mainColor);
	drawHText(painter, center.x(), top-_annotOffset, _displayRect.right(), Qt::AlignHCenter, false);
	drawHText(painter, center.x(), top+diameter+_annotOffset, _displayRect.right(), Qt::AlignHCenter, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::paintSpherical(QPainter &painter) {
	paintSphericalBackground(painter);

	for ( int id = 0; id < _values.size(); ++id ) {
		const ValueItem& v = _values[id];

		if ( v.ptx(_xIndex) >= _displayRect.right() )
			continue;

		if ( !v.isVisible ) continue;

		QColor c;
		if ( v.isActive )
			c = v.cols[_yIndex].color;
		else
			c = _disabledColor;

		painter.setPen(QColor(c.red()/2, c.green()/2, c.blue()/2));
		if ( !v.isEnabled ) {
			if ( _hideDisabledValues ) continue;
			painter.setBrush(Qt::NoBrush);
		}
		else
			painter.setBrush(c);

		drawValue(id, painter, (this->*project)(v.pt(_xIndex,_yIndex)),
		          v.type, v.valid(_xIndex,_yIndex));
	}

	painter.setRenderHint(QPainter::Antialiasing);
	{

		QPoint p0 = (this->*project)(_rubberBand.topLeft());
		QPoint p2 = (this->*project)(_rubberBand.bottomRight());

		/*
		std::cout << "RB: " << _rubberBand.left() << ", "
		          << _rubberBand.top() << ", "
		          << _rubberBand.right() << ", "
		          << _rubberBand.bottom() << std::endl;
		*/

		if ( _dragZoom ) {
			painter.setPen(QColor(96,64,32));
			painter.setBrush(QColor(255,224,192,160));
		}
		else {
			switch ( _rubberBandOperation ) {
				case SelectPlus:
					painter.setPen(QColor(32,96,64));
					painter.setBrush(QColor(192,255,224,160));
					break;
				case SelectMinus:
					painter.setPen(QColor(96,32,64));
					painter.setBrush(QColor(255,192,224,160));
					break;
				default:
					painter.setPen(QColor(32,64,96));
					painter.setBrush(QColor(192,224,255,160));
					break;
			}
		}

		QPainterPath pp;

		QPoint center = _diagramArea.center();
		QPoint tmp = (this->*project)(QPointF(_rubberBand.left(), 0));
		int w = tmp.y() - center.y();
		float len = _rubberBand.height();
		if ( len < 0 ) len += 360;

		if ( len == 0 || len >= 360 ) {
			QPainterPath pp2;
			pp2.addEllipse(center.x() - w, center.y() - w, w*2, w*2);

			tmp = (this->*project)(QPointF(_rubberBand.right(), 0));
			w = tmp.y() - center.y();

			pp.addEllipse(center.x() - w, center.y() - w, w*2, w*2);
			pp.addPath(pp2.toReversed());
		}
		else {
			pp.moveTo(p0);
			pp.arcTo(center.x() - w, center.y() - w, w*2, w*2,
			         fmod(-(float)_rubberBand.top()-90.0f, 360.0f), -fmod(len, 360.0f));
			pp.lineTo(p2);

			tmp = (this->*project)(QPointF(_rubberBand.right(), 0));
			w = tmp.y() - center.y();

			pp.arcTo(center.x() - w, center.y() - w, w*2, w*2,
			         fmod(-_rubberBand.top()-90, 360)-fmod(len, 360), fmod(len, 360));
		}

		pp.closeSubpath();

		painter.drawPath(pp);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::mouseDoubleClickEvent(QMouseEvent * event) {
	if ( event->button() == Qt::LeftButton ) {
		if ( _hoverId != -1 ) {
			toggleState(_hoverId);
			update();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::mousePressEvent(QMouseEvent *event) {
	if ( _selecting )
		emit endSelection();

	_selecting = false;

	if ( event->button() == Qt::LeftButton ) {
		if ( !_diagramArea.contains(event->pos()) ) return;

		_rubberBandOperation = Select;

		if ( event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) ) {
			_dragZoom = true;
		}
		else if ( event->modifiers() == Qt::ShiftModifier ) {
			_rubberBandOperation = SelectPlus;
		}
		else if ( event->modifiers() == Qt::ControlModifier ) {
			_rubberBandOperation = SelectMinus;
		}
		else {
			_dragZoom = false;

			if ( _hoverId != -1 )
				emit clicked(_hoverId);
		}

		saveStates();

		_dragStart = true;
		_dragging = false;
		_draggingStart = event->pos();
	}
	/*
	else if ( event->button() == Qt::RightButton ) {
		_dragging = true;
		_draggingStart = event->pos();
		_rubberBand = QRectF(unProject(_draggingStart), QSizeF(0,0));
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::mouseReleaseEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		_dragStart = false;
		_dragging = false;

		if ( _dragZoom ) {
			if ( ( _rubberBand.width() > 0 || _rubberBand.height() > 0) ) {
				_displayRect = _rubberBand;
				(this->*adjustZoom)(_displayRect);
				adjustZoomRect(_displayRect);
			}
		}
		else {
			if ( _selecting )
				emit endSelection();
		}

		_rubberBand = QRectF();
		_dragZoom = false;
		_selecting = false;

		//updateSelection();
		update();
	}
	/*
	else if ( event->button() == Qt::RightButton ) {
		_dragging = false;
		_boundingRect = _rubberBand;
		_rubberBand = QRectF();
		update();
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::mouseMoveEvent(QMouseEvent *event) {
	if ( _dragStart ) {
		// Let the dragging effectively start only if the mouse
		// pointer has moved a certain distance. This is to prevent
		// accidental de-selection upon clicking and slightly moving
		// the mouse.
		QPoint _draggingEnd = event->pos();
		QRect rect(_draggingStart, _draggingEnd);
		// This criterion might be made configurable
		if (abs(rect.width()) + abs(rect.height()) < 20)
			return;

		_dragStart = false;
		_dragging  = true;
		_rubberBand = QRectF((this->*unProject)(_draggingStart), QSizeF(0,0));

		if ( !_dragZoom ) {
			_selecting = true;
			emit beginSelection();
			updateSelection();
		}
	}

	if ( _dragging ) {
		_rubberBand.setTopLeft((this->*unProject)(_draggingStart));
		_rubberBand.setBottomRight((this->*unProject)(event->pos()));

		_rubberBandOperation = Select;

		if ( event->modifiers() == Qt::ShiftModifier ) {
			_rubberBandOperation = SelectPlus;
		}
		else if ( event->modifiers() == Qt::ControlModifier ) {
			_rubberBandOperation = SelectMinus;
		}

		switch ( _type ) {
			case Rectangular:
				clipRectRectangular(_rubberBand);
				break;
			case Spherical:
				clipRectSpherical(_rubberBand);
				break;
			default:
				break;
		}

		if ( !_dragZoom )
			updateSelection();
		else {
			(this->*adjustZoom)(_rubberBand);
			emit adjustZoomRect(_rubberBand);
		}

		update();
	}
	else {
		int hoverId = findValue(event->pos());
		if ( hoverId != _hoverId ) {
			_hoverId = hoverId;
			update();
			emit hover(_hoverId);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::keyPressEvent(QKeyEvent *event) {
	if ( _dragging ) {
		// TODO: selection operation update
		if ( event->key() == Qt::Key_Escape ) {
			_dragStart = false;
			_dragging = false;
			_rubberBand = QRectF();
			_rubberBandOperation = SelectPlus;
			updateSelection();
			update();
		}
		else if ( event->key() == Qt::Key_Shift || event->key() == Qt::Key_Control ) {
			_rubberBandOperation = Select;

			if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
				_rubberBandOperation = SelectPlus;
			if ( QApplication::keyboardModifiers() == Qt::ControlModifier )
				_rubberBandOperation = SelectMinus;

			updateSelection();
			update();
		}

	}
	return QWidget::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::keyReleaseEvent(QKeyEvent *event) {
	if ( _dragging ) {
		if ( event->key() == Qt::Key_Shift || event->key() == Qt::Key_Control ) {
			_rubberBandOperation = Select;

			if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
				_rubberBandOperation = SelectPlus;
			if ( QApplication::keyboardModifiers() == Qt::ControlModifier )
				_rubberBandOperation = SelectMinus;

			updateSelection();
			update();
		}

	}
	return QWidget::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::updateContextMenu(QMenu &menu) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::handleContextMenuAction(QAction *) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::contextMenuEvent(QContextMenuEvent *event) {
	QMenu contextMenu(this);

	_zoomAction->setEnabled(false);

	for ( int i = 0; i < _values.count(); ++i ) {
		if ( _values[i].isActive ) {
			_zoomAction->setEnabled(true);
			break;
		}
	}

	_resetAction->setEnabled(_displayRect != _requestRect);

	contextMenu.addAction(_zoomAction);
	contextMenu.addAction(_resetAction);

	/*
	QAction *sep = new QAction(this);
	sep->setSeparator(true);
	contextMenu.addAction(sep);

	contextMenu.addAction(_actionActivate);
	contextMenu.addAction(_actionEnable);
	*/

	//contextMenu.addAction(_selectAbscissaInterval);
	//contextMenu.addAction(_selectOrdinateInterval);

	updateContextMenu(contextMenu);
	handleContextMenuAction(contextMenu.exec(event->globalPos()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::zoomIntoSelectedValues() {
	_displayRect = getSelectedValuesRect();
	(this->*adjustZoom)(_displayRect);
	emit adjustZoomRect(_displayRect);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::saveStates() {
	for ( int i = 0; i < _values.count(); ++i ) {
		_values[i].saveActiveState = _values[i].isActive;
		_values[i].saveEnableState = _values[i].isEnabled;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::resetZoom() {
	_displayRect = _requestRect;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::selectActiveState() {
	_mode = SelectActiveState;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::selectEnableState() {
	_mode = SelectEnableState;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueDisabledColor(const QColor& c) {
	_disabledColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setDisabledValuesHidden(bool f) {
	_hideDisabledValues = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setDrawGridLines(bool f) {
	_drawGridLines = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::clipRectRectangular(QRectF &rect) {
	rect = rect.normalized() & _displayRect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::clipRectSpherical(QRectF &rect) {
	if ( rect.left() > _displayRect.right() )
		rect.setLeft(_displayRect.right());
	if ( rect.right() > _displayRect.right() )
		rect.setRight(_displayRect.right());

	qreal dist = fabs(rect.top() - rect.bottom());
	// snap within 10 degrees
	if ( dist < 10 || dist > 350 )
		rect.setBottom(rect.top());

	if ( !_dragZoom && rect.right() < rect.left() ) {
		qreal tmp = rect.left();
		rect.setLeft(rect.right());
		rect.setRight(tmp);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::adjustZoomRectangular(QRectF &) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::adjustZoomSpherical(QRectF &zoomRect) {
	zoomRect.setTop(0);
	zoomRect.setBottom(360);

	zoomRect.setLeft(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::diagramAreaUpdated(const QRect &) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawValue(int id, QPainter& painter, const QPoint& p,
                              SymbolType type, bool valid) const {
	static QPolygon triangle = QPolygon() << QPoint(-3,3) << QPoint(3,3) << QPoint(0,-3);
	static QPolygon triangle2 = QPolygon() << QPoint(3,-3) << QPoint(-3,-3) << QPoint(0,3);
	static QPolygon diamond = QPolygon() << QPoint(-3,0) << QPoint(0,-3) << QPoint(3,0) << QPoint(0,3);

	switch ( type ) {
		case Circle:
			painter.drawEllipse(p.x()-3, p.y()-3, 6, 6);
			if ( !valid ) {
				painter.drawLine(p.x()-2,p.y()-2,p.x()+2,p.y()+2);
				painter.drawLine(p.x()+2,p.y()-2,p.x()-2,p.y()+2);
			}
			break;
		case Triangle:
			painter.translate(p.x(), p.y());
			painter.drawPolygon(triangle);
			painter.translate(-p.x(), -p.y());
			if ( !valid ) {
				painter.drawLine(p.x()-1,p.y(),p.x()+3,p.y()+3);
				painter.drawLine(p.x()+1,p.y(),p.x()-3,p.y()+3);
			}
			break;
		case TriangleUpsideDown:
			painter.translate(p.x(), p.y());
			painter.drawPolygon(triangle2);
			painter.translate(-p.x(), -p.y());
			if ( !valid ) {
				painter.drawLine(p.x()-1,p.y()+3,p.x()+3,p.y());
				painter.drawLine(p.x()+1,p.y()+3,p.x()-3,p.y());
			}
			break;
		case Rectangle:
			painter.drawRect(p.x()-3, p.y()-3, 6, 6);
			if ( !valid ) {
				painter.drawLine(p.x()-3,p.y()-3,p.x()+3,p.y()+3);
				painter.drawLine(p.x()+3,p.y()-3,p.x()-3,p.y()+3);
			}
			break;
		case Diamond:
			painter.translate(p.x(), p.y());
			painter.drawPolygon(diamond);
			painter.translate(-p.x(), -p.y());
			if ( !valid ) {
				painter.drawLine(p.x()-3,p.y()-3,p.x()+3,p.y()+3);
				painter.drawLine(p.x()+3,p.y()-3,p.x()-3,p.y()+3);
			}
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawHText(QPainter& painter, int x, int y, qreal value, Qt::Alignment al, bool topAligned) const {
	int flags = (topAligned ? Qt::AlignTop : Qt::AlignBottom) | al;
	int dy = 1; // vertical fine tuning
	int yy = topAligned ? (y+dy) : (y-_textHeight+dy);
	int xx = x;
	int w;

	if ( al & Qt::AlignHCenter ) {
		xx -= width();
		w = width()*2;
	}
	else if ( al & Qt::AlignRight ) {
		xx = 0;
		w = x;
	}
	else if ( al & Qt::AlignLeft ) {
		w = width() - x;
	}
	else
		w = width() - x;

	painter.drawText(xx, yy, w, _textHeight, flags, QString("%1").arg(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawVText(QPainter& painter, int x, int y, qreal value, Qt::Alignment al, bool leftAligned) const {
	int flags = (leftAligned ? Qt::AlignLeft : Qt::AlignRight) | al;
	int xx = leftAligned ? x : 0;
	int w = leftAligned ? width() - x : x;
	int yy = y;
	int h;

	if ( al & Qt::AlignVCenter ) {
		yy -= _textHeight/2;
		h = _textHeight;
	}
	else if ( al & Qt::AlignTop ) {
		h = height() - yy;
	}
	else if ( al & Qt::AlignBottom ) {
		yy = 0;
		h = y;
	}
	else
		h = height() - yy;

	painter.drawText(xx, yy, w, h, flags, QString("%1").arg(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawAbscissa(QPainter& painter, int y, bool drawMarker, bool topAligned) {
	painter.drawLine(_diagramArea.left(), y, _diagramArea.right(), y);
	if ( !drawMarker ) return;

	int textOffset = topAligned ? +_annotOffset : -_annotOffset;
	int textSpacing = painter.fontMetrics().width('0');
	double spacing = getSpacing(_displayRect.width(), 6);
	//double spacing = _markerDistance.x();

	int lastTextX = _diagramArea.left() + painter.fontMetrics().width(QString("%1").arg(_displayRect.left()));

	if ( spacing > 0 ) {
		qreal s0 = int(floor(_displayRect.left() / spacing)) * spacing;
		qreal scale = _diagramArea.width() / _displayRect.width();

		if ( s0 < _displayRect.left() ) s0 += spacing;

		while ( s0 < _displayRect.right() ) {
			if ( fabs(s0) < 1E-10 ) s0 = 0;
			int posX = _diagramArea.left() + (int)((s0 - _displayRect.left()) * scale);
			painter.drawLine(posX, y-_tickLength, posX, y+_tickLength);

			int tw = painter.fontMetrics().width(QString("%1").arg(s0));
			if ( (posX - tw/2 - lastTextX) >= textSpacing ) {
				drawHText(painter, posX, y+textOffset, s0, Qt::AlignHCenter, topAligned);
				lastTextX = posX + tw/2;
			}

			s0 += spacing;
		}
	}

	painter.drawLine(_diagramArea.left(), y-_tickLengthA, _diagramArea.left(), y+_tickLengthA);
	drawHText(painter, _diagramArea.left(), y+textOffset, _displayRect.left(), Qt::AlignLeft, topAligned);

	painter.drawLine(_diagramArea.right(), y-_tickLengthA, _diagramArea.right(), y+_tickLengthA);

	int tw = painter.fontMetrics().width(QString("%1").arg(_displayRect.right()));
	if ( (_diagramArea.right() - tw - lastTextX) >= textSpacing )
		drawHText(painter, _diagramArea.right(), y+textOffset, _displayRect.right(), Qt::AlignRight, topAligned);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::drawOrdinate(QPainter& painter, int x, int abscissaHeight, qreal abscissaValue, bool leftAligned) {
	//int textOffset = leftAligned ? +_annotOffset : -_annotOffset;

	double spacing = getSpacing(_displayRect.height(), 6);
	//double spacing = _markerDistance.y();

	if ( spacing > 0 ) {
		qreal s0 = int(floor(_displayRect.top() / spacing)) * spacing;
		qreal scale = _diagramArea.height() / _displayRect.height();

		if ( s0 < _displayRect.top() ) s0 += spacing;

		while ( s0 < _displayRect.bottom() ) {
			if ( fabs(s0) < 1E-10 ) s0 = 0;
			int posY = _diagramArea.bottom() - (int)((s0 - _displayRect.top()) * scale);
			painter.drawLine(x-_tickLength, posY, x+_tickLength, posY);
			drawVText(painter, x-_annotOffset, posY, s0, Qt::AlignRight | Qt::AlignVCenter, false);
			s0 += spacing;
		}
	}

	painter.drawLine(x, _diagramArea.top(), x, _diagramArea.bottom());
	painter.drawLine(x-_tickLengthA, _diagramArea.bottom(), x+_tickLengthA, _diagramArea.bottom());
	//drawVText(painter, x, _clientArea.bottom(), _displayRect.top(), Qt::AlignBottom, true);
	painter.drawLine(x-_tickLengthA, abscissaHeight, x+_tickLengthA, abscissaHeight);
	painter.drawLine(x-_tickLengthA, _diagramArea.top(), x+_tickLengthA, _diagramArea.top());
	//drawVText(painter, x, _clientArea.top(), _displayRect.bottom(), Qt::AlignTop, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPoint DiagramWidget::projectRectangular(const QPointF& p) const {
	return QPoint((int)((p.x() - _displayRect.left()) * _diagramArea.width() / _displayRect.width()) + _diagramArea.left(),
	              (int)((_displayRect.top() - p.y()) * _diagramArea.height() / _displayRect.height()) + _diagramArea.bottom());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF DiagramWidget::unProjectRectangular(const QPoint& p) const {
	return QPointF((p.x() - _diagramArea.left()) * _displayRect.width() / _diagramArea.width() + _displayRect.left(),
	               (_diagramArea.bottom() - p.y()) * _displayRect.height() / _diagramArea.height() + _displayRect.top());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPoint DiagramWidget::projectSpherical(const QPointF& p) const {
	int diameter = std::min(_diagramArea.width(), _diagramArea.height());
	int radius = diameter / 2;

	QPoint center = _diagramArea.center();

	float dist = (p.x() * radius / _displayRect.right());
	float deg = fmod(double(p.y()), 360.0);

	return QPoint(center.x() + (int)(dist*sin(deg2rad(deg))),
	              center.y() - (int)(dist*cos(deg2rad(deg))));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF DiagramWidget::unProjectSpherical(const QPoint& p) const {
	int diameter = std::min(_diagramArea.width(), _diagramArea.height());
	int radius = diameter / 2;

	QPoint center = _diagramArea.center();
	QPoint delta = p - center;

	float dist = (float)sqrt((float)(delta.x()*delta.x() + delta.y()*delta.y()));
	if ( dist <= 0.001 )
		return QPointF(0,0);

	float vx = dist * _displayRect.right() / radius;

	float rx = asin((p.x() - center.x()) / dist);
	float ry = acos((center.y() - p.y()) / dist);

	return QPointF(vx, rx < 0?360.0f - rad2deg(ry):rad2deg(ry));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::containsRectangular(const QRectF &r, const QPointF &p) const {
	return r.contains(p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::containsSpherical(const QRectF &r, const QPointF &p) const {
	if ( p.x() < r.left() || p.x() > r.right() ) return false;

	if ( r.top() < r.bottom() ) {
		if ( p.y() < r.top() || p.y() > r.bottom() ) return false;
	}
	else {
		if ( p.y() < r.top() && p.y() > r.bottom() ) return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setColumnCount(int c) {
	if ( _columns == c ) return;

	_columns = c;
	for ( int i = 0; i < _values.count(); ++i )
		_values[i].setColumns(_columns);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DiagramWidget::columnCount() const {
	return _columns;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setBackgroundColor(const QColor& c) {
	_background = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setMargin(int margin) {
	_horMargin = _verMargin = margin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setMarkerDistance(double xDist, double yDist) {
	_markerDistance = QPointF(xDist, yDist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setMarkerDistance(QPointF distances) {
	_markerDistance = distances;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::clear() {
	_hoverId = -1;
	_values.clear();
	_boundingRect = QRectF();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DiagramWidget::count() const {
	return _values.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::isEmpty() const {
	return _values.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(const QPointF& point) {
	addValue(point, Qt::green);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(const QPointF& point, const QColor& color) {
	_values.push_back(ValueItem(_columns, _xIndex, _yIndex, point, false, color));
	updateBoundingRect(point);
	checkSelection(_values.count()-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(qreal abscissa, qreal ordinate) {
	addValue(QPointF(abscissa, ordinate), Qt::green);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(int x, int y, qreal abzisse, qreal ordinate) {
	addValue(x, y, QPointF(abzisse, ordinate), Qt::green);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(int x, int y, const QPointF& point) {
	addValue(x,y, point, Qt::green);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::addValue(int x, int y, const QPointF& point, const QColor& color) {
	_values.push_back(ValueItem(_columns, x, y, point, false, color));
	updateBoundingRect(point);
	checkSelection(_values.count()-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVector<int> DiagramWidget::getValues(const QRectF& rect) const {
	QVector<int> selectionList;
	for ( int i = 0; i < _values.count(); ++i )
		if ( rect.contains(_values[i].pt(_xIndex,_yIndex)) )
			selectionList.push_back(i);

	return selectionList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF DiagramWidget::value(int id) const {
	return _values[id].pt(_xIndex,_yIndex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVector<int> DiagramWidget::getSelectedValues() const {
	QVector<int> selectionList;
	for ( int i = 0; i < _values.count(); ++i )
		if ( _values[i].isActive )
			selectionList.push_back(i);

	return selectionList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRectF DiagramWidget::getSelectedValuesRect() const {
	QRectF brect(0,0,-1,-1);
	bool first = true;
	for ( int i = 0; i < _values.count(); ++i )
		if ( _values[i].isActive && _values[i].isEnabled ) {
			QPointF p = _values[i].pt(_xIndex,_yIndex);
			if ( first )
				brect.setCoords(p.x(), p.y(), p.x(), p.y());
			else {
				if ( p.x() < brect.left() )
					brect.setLeft(p.x());
				else if ( p.x() > brect.right() )
					brect.setRight(p.x());

				if ( p.y() < brect.top() )
					brect.setTop(p.y());
				else if ( p.y() > brect.bottom() )
					brect.setBottom(p.y());
			}

			first = false;
		}

	return brect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::selectValues(const QRectF& targetRect) {
	QRectF tmp(_rubberBand);

	_rubberBand = targetRect;
	_rubberBandOperation = Select;

	updateSelection();
	update();

	_rubberBand = tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValue(int id, int x, qreal v) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].cols[x].value = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValue(int id, const QPointF& p) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].setPt(_xIndex,_yIndex, p);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValue(int id, int x, int y, const QPointF& p) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].setPt(x,y, p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueColor(int id, const QColor& c) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].cols[_yIndex].color = c;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueColor(int id, int x, const QColor& c) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].cols[x].color = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueValid(int id, int x, bool valid) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].cols[x].valid = valid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueSymbol(int id, SymbolType type) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueSelected(int id, bool selected) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].isActive = selected;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setValueEnabled(int id, bool enabled) {
	if ( id >= _values.count() || id < 0 ) {
		SEISCOMP_DEBUG("Index %d out of range", id);
		return;
	}

	_values[id].isEnabled = enabled;
	if ( id == _hoverId ) _hoverId = -1;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::isValueValid(int id, int x) const {
	return _values[id].cols[x].valid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::isValueSelected(int id) const {
	return _values[id].isActive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::isValueEnabled(int id) const {
	return _values[id].isEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DiagramWidget::isValueShown(int id) const {
	return _values[id].isVisible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::showValue(int id, bool f) {
	_values[id].isVisible = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::hideValue(int id, bool f) {
	_values[id].isVisible = !f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::setDisplayRect(const QRectF& rect) {
	_requestRect = rect.normalized();
	_displayRect = _requestRect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRectF DiagramWidget::displayRect() const {
	return _requestRect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRectF DiagramWidget::boundingRect() const {
	return _boundingRect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::updateBoundingRect(const QPointF& p) {
	if ( count() < 2 )
		_boundingRect.setCoords(floor(p.x()), floor(p.y()),
		                        ceil(p.x()), ceil(p.y()));
	else {
		if ( p.x() < _boundingRect.left() )
			_boundingRect.setLeft(floor(p.x()));
		else if ( p.x() > _boundingRect.right() )
			_boundingRect.setRight(ceil(p.x()));

		if ( p.y() < _boundingRect.top() )
			_boundingRect.setTop(floor(p.y()));
		else if ( p.y() > _boundingRect.bottom() )
			_boundingRect.setBottom(ceil(p.y()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::updateBoundingRect() {
	_boundingRect = QRectF();
	foreach(const ValueItem& v, _values) {
		if ( (!_hideDisabledValues || v.isEnabled) && v.isVisible )
			updateBoundingRect(v.pt(_xIndex,_yIndex));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::changeState(int id, bool state) {
	if ( _mode == SelectActiveState ) {
		if ( _values[id].isActive != state ) {
			_values[id].isActive = state;
			emit valueActiveStateChanged(id, state);
			if ( _values[id].isActive )
				emit valueActivated(id);
			else
				emit valueDeactivated(id);
		}
	}
	else if ( _mode == SelectEnableState ) {
		if ( _values[id].isEnabled != state ) {
			_values[id].isEnabled = state;
			emit valueEnableStateChanged(id, state);
			if ( _values[id].isEnabled )
				emit valueEnabled(id);
			else
				emit valueDisabled(id);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::toggleState(int id) {
	if ( _mode == SelectActiveState ) {
		setValueSelected(id, !_values[id].isActive);
		emit valueActiveStateChanged(id, _values[id].isActive);
		if ( _values[id].isActive )
			emit valueActivated(id);
		else
			emit valueDeactivated(id);
	}
	else if ( _mode == SelectEnableState ) {
		setValueEnabled(id, !_values[id].isEnabled);
		emit valueEnableStateChanged(id, _values[id].isActive);
		if ( _values[id].isEnabled )
			emit valueEnabled(id);
		else
			emit valueDisabled(id);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::restoreState(int id) {
	if ( _mode == SelectActiveState ) {
		if ( _values[id].isActive != _values[id].saveActiveState ) {
			_values[id].isActive = _values[id].saveActiveState;
			emit valueActiveStateChanged(id, _values[id].saveActiveState);
			if ( _values[id].isActive )
				emit valueActivated(id);
			else
				emit valueDeactivated(id);
		}
	}
	else if ( _mode == SelectEnableState ) {
		if ( _values[id].isEnabled != _values[id].saveEnableState ) {
			_values[id].isEnabled = _values[id].saveEnableState;
			emit valueEnableStateChanged(id, _values[id].saveEnableState);
			if ( _values[id].isEnabled )
				emit valueEnabled(id);
			else
				emit valueDisabled(id);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::checkSelection(int id) {
	if ( !_values[id].isVisible ) return;
	if ( _mode != SelectEnableState && !_values[id].isEnabled ) return;

	if ( (this->*contains)(_rubberBand, _values[id].pt(_xIndex,_yIndex)) ) {
		switch ( _rubberBandOperation ) {
			case Select:
			case SelectPlus:
				changeState(id, true);
				break;
			case SelectMinus:
				changeState(id, false);
				break;
		}
	}
	else {
		switch ( _rubberBandOperation ) {
			case Select:
				changeState(id, false);
				break;
			case SelectPlus:
			case SelectMinus:
				restoreState(id);
				break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramWidget::updateSelection() {
	if ( _dragZoom ) return;
	for ( int i = 0; i < _values.count(); ++i )
		checkSelection(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline int dist(const QPoint& p1, const QPoint& p2) {
	double diff_x = p2.x() - p1.x();
	double diff_y = p2.y() - p1.y();
	return (int)sqrt(diff_x*diff_x + diff_y*diff_y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DiagramWidget::findValue(const QPoint& p) const {
	int mindist = -1;
	int minid = -1;

	for ( int i = 0; i < _values.count(); ++i ) {
		if ( !_values[i].isVisible ||
		     !_values[i].cols[_xIndex].valid ||
		     !_values[i].cols[_yIndex].valid ) continue;

		if ( _mode != SelectEnableState && !_values[i].isEnabled ) continue;

		QPointF v = _values[i].pt(_xIndex,_yIndex);
		if ( !_displayRect.contains(v) ) continue;
		int d = dist((this->*project)(v),p);
		if ( d < mindist || mindist == -1 ) {
			mindist = d;
			minid = i;
		}
	}

	if ( mindist >= 0 && mindist < 5 )
		return minid;

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
