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

#include <seiscomp3/gui/map/legend.h>

#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/canvas.h>

#include <QPainter>
#include <QFontMetrics>


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend::Legend(QObject *parent)
: QObject(parent)
, _margin(9)
, _spacing(4)
, _layer(NULL)
, _alignment(Qt::AlignLeft | Qt::AlignTop)
, _enabled(true)
, _visible(true) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend::Legend(const QString& title, QObject *parent)
: QObject(parent)
, _margin(9)
, _spacing(4)
, _layer(NULL)
, _title(title)
, _alignment(Qt::AlignLeft | Qt::AlignTop)
, _enabled(true)
,_visible(true) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::bringToFront() {
	emit bringToFrontRequested(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::setEnabled(bool e) {
	if ( _enabled == e ) return;
	_enabled = e;
	emit enabled(this, e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Legend::isEnabled() const {
	return _enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend &Legend::operator =(const Legend &other) {
	_alignment = other._alignment;
	_margin = other._margin;
	_spacing = other._spacing;
	_font = other._font;
	_titleFont = other._titleFont;
	_size = other._size;
	_layer = other._layer;
	_title = other._title;
	_enabled = other._enabled;
	_visible = other._visible;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegend::StandardLegend(QObject *parent) : Legend(parent) {
	_maxColumns = -1;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::addItem(const QColor &c, const QString &l) {
	_items.append(Item(c, l));
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::setMaximumColumns(int columns) {
	if ( _maxColumns == columns ) return;
	_maxColumns = columns;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::updateLayout(const QSize &size) {
	if ( !size.isValid() ) return;

	QFontMetrics fm(font());
	int ch = size.height();
	int fontHeight = fm.height();

	_columns = 1;
	_columnWidth = 0;

	for ( int i = 0; i < _items.count(); ++i ) {
		int itemWidth = fm.width(_items[i].label);
		if ( itemWidth > _columnWidth )
			_columnWidth = itemWidth;
	}

	_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
	_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));

	if ( ch <= 0 ) return;

	while ( (_size.width() < size.width()) && (_size.height() > ch - 30)
	    && ((_maxColumns <= 0) || (_columns < _maxColumns))
	    && (_columns < _items.size()) ) {
		++_columns;
		_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
		_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));
	}

	_layoutDirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::contextResizeEvent(const QSize &size) {
	updateLayout(size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::draw(const QRect &rect, QPainter &painter) {
	if ( _layoutDirty && layer() )
		updateLayout(layer()->size());

	QFontMetrics fm(font());
	int fontHeight = fm.height();
	int w = rect.width();
	int x = rect.left() + fontHeight/2;
	int idx = 0;

	for ( int c = 0; c < _columns; ++c ) {
		int y = rect.top() + fontHeight/2;
		int textOffset = fontHeight*3/2;
		int textWidth = w - fontHeight - textOffset;

		int itemsPerColumn = (_items.count() + _columns - 1) / _columns;
		int cnt = _items.count() - itemsPerColumn * c;
		if ( cnt > itemsPerColumn ) cnt = itemsPerColumn;

		for ( int i = 0; i < cnt; ++i, ++idx ) {
			painter.fillRect(x, y, fontHeight, fontHeight, _items[idx].color);
			painter.drawText(x + textOffset, y, textWidth, fontHeight,
			                 Qt::AlignLeft | Qt::AlignVCenter, _items[idx].label);
			y += fontHeight*3/2;
		}

		x += _columnWidth + fontHeight*4/2;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
