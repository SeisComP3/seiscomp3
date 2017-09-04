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



#include <seiscomp3/gui/datamodel/tensorsymbol.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/math/conversions.h>
#include <QPainter>


namespace Seiscomp {
namespace Gui {


TensorSymbol::TensorSymbol(const Math::Tensor2Sd &t,
                           Map::Decorator* decorator)
: Symbol(decorator) {
	_tensor = t;
	Math::tensor2matrix(_tensor, _rotation);
	_renderer.setShadingEnabled(false);
	_lastSize = QSize(-1, -1);
	_drawLocationConnector = true;
	
}


TensorSymbol::~TensorSymbol() {}


void TensorSymbol::setShadingEnabled(bool e) {
	_renderer.setShadingEnabled(e);
}


void TensorSymbol::setDrawConnectorEnabled(bool e) {
	_drawLocationConnector = e;
}


void TensorSymbol::setBorderColor(QColor c) {
	_renderer.setBorderColor(c);
}


void TensorSymbol::setTColor(QColor c) {
	_renderer.setTColor(c);
}


void TensorSymbol::setPColor(QColor c) {
	_renderer.setPColor(c);
}


void TensorSymbol::setPosition(QPointF geoPosition) {
	setLocation(geoPosition);
}


void TensorSymbol::setOffset(QPoint offset) {
	_offset = offset;
}


void TensorSymbol::resize(int w, int h) {
	setSize(QSize(w,h));
	_buffer = QImage(size(), QImage::Format_ARGB32);
	_renderer.render(_buffer, _tensor, _rotation);
}


bool TensorSymbol::isInside(int x, int y) const {
	return false;
}


void TensorSymbol::customDraw(const Map::Canvas *, QPainter &p) {
	if ( size() != _lastSize ) {
		_lastSize = size();
		resize(_lastSize.width(), _lastSize.height());
	}

	if ( _drawLocationConnector ) {
		p.setPen(QPen(Qt::black, 1, Qt::DashLine));
		p.drawLine(pos(), pos() + _offset);
	}

	p.drawImage(pos() + _offset - QPoint(_size.width()/2, _size.height()/2), _buffer);
}


}
}
