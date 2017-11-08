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


#include <seiscomp3/gui/plot/legend.h>
#include <seiscomp3/gui/plot/graph.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend::Legend(Qt::Alignment align, QObject *parent)
: AbstractLegend(parent), _alignment(align) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::draw(QPainter &p, const QRect &plotRect,
                  const QList<Graph*> &graphs) {

	int width = 0;
	int height = 0;

	QFontMetrics fm = p.fontMetrics();
	int fontHeight = fm.height();
	int halfFontHeight = fontHeight / 2;

	foreach ( Graph *g, graphs ) {
		if ( !g->isVisible() ) continue;
		if ( g->isEmpty() ) continue;
		if ( g->name().isEmpty() ) continue;

		width = qMax(width, fm.width(g->name()));
		height += fontHeight;
	}

	QRect r(0,0,width+5*fontHeight/2,height+fontHeight);
	if ( _alignment & Qt::AlignLeft )
		r.moveLeft(plotRect.left());
	else
		r.moveRight(plotRect.right()-1);

	if ( _alignment & Qt::AlignBottom )
		r.moveBottom(plotRect.bottom()-1);
	else
		r.moveTop(plotRect.top());

	p.setRenderHint(QPainter::Antialiasing, false);
	p.setPen(QColor(192,192,192));
	p.setBrush(QColor(255,255,255,192));
	p.drawRect(r);

	int lx = r.left() + halfFontHeight;
	int tx = r.left() + 2*fontHeight;
	int y = r.top() + halfFontHeight;

	QRect symbolRect(lx, y, fontHeight, fontHeight);

	foreach ( Graph *g, graphs ) {
		if ( !g->isVisible() ) continue;
		if ( g->isEmpty() ) continue;
		if ( g->name().isEmpty() ) continue;

		g->drawSymbol(p, symbolRect);
		p.setPen(Qt::black);
		p.drawText(tx, y, width, fontHeight, Qt::AlignLeft | Qt::AlignVCenter, g->name());

		y += fontHeight;
		symbolRect.moveTop(y);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
