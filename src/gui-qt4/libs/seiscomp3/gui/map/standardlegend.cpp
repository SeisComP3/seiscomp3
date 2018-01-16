/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#include <seiscomp3/gui/map/standardlegend.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/canvas.h>

#include <QPainter>
#include <QFontMetrics>


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(StandardLegend *legend) {
	if ( legend )
		legend->addItem(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(const QPen &p, const QString &l)
: pen(p), label(l), size(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(const QPen &p, const QString &l, int s)
: pen(p), label(l), size(s) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(const QPen &p, const QBrush &b, const QString &l)
: pen(p), brush(b), label(l), size(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(const QPen &p, const QBrush &b, const QString &l, int s)
: pen(p), brush(b), label(l), size(s) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::~StandardLegendItem() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::draw(QPainter *painter, const QRect &symbolRect,
                              const QRect &textRect) {
	drawSymbol(painter, symbolRect);
	drawText(painter, textRect);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::drawSymbol(QPainter *painter, const QRect &rect) {
	if ( brush != Qt::NoBrush ) {
		int x = rect.left();
		int y = rect.top();
		int w = rect.width();
		int h = rect.height();

		if ( size > 0 ) {
			int s = qMin(size, w);
			s = qMin(s, h);
			x += (w-s)/2;
			y += (h-s)/2;
			w = h = s;
		}

		painter->setPen(pen);
		painter->setBrush(brush);
		painter->drawRect(x, y, w, h);
	}
	else {
		QPen p(pen);
		int h = qMin(4, rect.height());
		p.setWidth(h);
		painter->setPen(p);
		painter->drawLine(rect.left(), (rect.top()+rect.bottom()+h)/2,
		                  rect.right(), (rect.top()+rect.bottom()+h)/2);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::drawText(QPainter *painter, const QRect &rect) {
	painter->setPen(SCScheme.colors.legend.text);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, label);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegend::StandardLegend(QObject *parent) : Legend(parent) {
	_orientation = Qt::Vertical;
	_maxColumns = -1;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegend::~StandardLegend() {
	for ( int i = 0; i < _items.count(); ++i )
		delete _items[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::addItem(StandardLegendItem *item) {
	_items.append(item);
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
void StandardLegend::setOrientation(Qt::Orientation orientation) {
	if ( _orientation == orientation ) return;
	_orientation = orientation;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::updateLayout(const QSize &size) {
	if ( !size.isValid() ) return;

	QFontMetrics fm(font());
	int ch = size.height();
	int fontHeight = fm.height();
	int rows;

	_columnWidth = 0;

	for ( int i = 0; i < _items.count(); ++i ) {
		int itemWidth = fm.width(_items[i]->label);
		if ( itemWidth > _columnWidth )
			_columnWidth = itemWidth;
	}

	switch ( _orientation ) {
		case Qt::Vertical:
			_columns = 1;

			_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
			_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));

			if ( ch <= 0 ) return;

			while ( (_size.width() < size.width()) && (_size.height() > ch-30)
			     && ((_maxColumns <= 0) || (_columns < _maxColumns))
			     && (_columns < _items.size()) ) {
				++_columns;
				_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
				_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));
			}

			break;

		case Qt::Horizontal:
			_columns = _items.count();

			_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
			_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));

			if ( ch <= 0 ) return;

			rows = 1;

			while ( (_size.width() > size.width()-2*_margin) && (_size.height() < ch-30)
			     && ((_maxColumns <= 0) || (rows < _maxColumns))
			     && (_columns > 1) ) {
				++rows;
				_columns = (_items.count()+rows-1) / rows;
				_size.setWidth((_columnWidth + fontHeight*3/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
				_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*fontHeight*3/2+fontHeight/2, 0));
			}

			break;

		default:
			break;
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

	painter.save();
	painter.setFont(font());

	switch ( _orientation ) {
		case Qt::Vertical:
		{
			int idx = 0;
			int textWidth = rect.width() - fontHeight - fontHeight*3/2;

			QRect symbolRect(rect.left() + fontHeight/2, 0, 0, 0);
			QRect textRect(rect.left() + fontHeight*2, 0, 0, 0);

			for ( int c = 0; c < _columns; ++c ) {
				symbolRect.setTop(rect.top() + fontHeight/2);
				textRect.setTop(symbolRect.top());

				int itemsPerColumn = (_items.count() + _columns-1) / _columns;
				int cnt = _items.count() - itemsPerColumn * c;
				if ( cnt > itemsPerColumn ) cnt = itemsPerColumn;

				for ( int i = 0; i < cnt; ++i, ++idx ) {
					symbolRect.setWidth(fontHeight); symbolRect.setHeight(fontHeight);
					textRect.setWidth(textWidth); textRect.setHeight(fontHeight);

					_items[idx]->draw(&painter, symbolRect, textRect);

					symbolRect.setTop(symbolRect.top() + fontHeight*3/2);
					textRect.setTop(textRect.top() + fontHeight*3/2);
				}

				symbolRect.setLeft(symbolRect.left() + _columnWidth + fontHeight*4/2);
				textRect.setLeft(textRect.left() + _columnWidth + fontHeight*4/2);
			}
			break;
		}

		case Qt::Horizontal:
		{
			QRect symbolRect(rect.left() + fontHeight/2, 0, 0, 0);
			QRect textRect(rect.left() + fontHeight/2 + fontHeight + fontHeight/2, 0, 0, 0);

			for ( int c = 0; c < _columns; ++c ) {
				symbolRect.setTop(rect.top() + fontHeight/2);
				textRect.setTop(symbolRect.top());

				int itemsPerColumn = (_items.count() + _columns-1) / _columns;
				int cnt = _items.count() - itemsPerColumn * c;
				if ( cnt > itemsPerColumn ) cnt = itemsPerColumn;

				for ( int i = 0; i < cnt; ++i ) {
					int idx = i*_columns+c;
					symbolRect.setWidth(fontHeight); symbolRect.setHeight(fontHeight);
					textRect.setWidth(_columnWidth); textRect.setHeight(fontHeight);

					_items[idx]->draw(&painter, symbolRect, textRect);

					symbolRect.setTop(symbolRect.top() + fontHeight*3/2);
					textRect.setTop(textRect.top() + fontHeight*3/2);
				}

				symbolRect.setLeft(symbolRect.left() + _columnWidth + fontHeight/2 + fontHeight + fontHeight/2);
				textRect.setLeft(textRect.left() + _columnWidth + fontHeight/2 + fontHeight + fontHeight/2);
			}
			break;
		}

		default:
			break;
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
