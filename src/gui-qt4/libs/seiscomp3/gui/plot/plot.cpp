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
#include <seiscomp3/gui/plot/graph.h>
#include <seiscomp3/gui/plot/plot.h>
#include <seiscomp3/gui/plot/abstractlegend.h>

#include <QPainter>
#include <iostream>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Plot::Plot(QObject *parent) : QObject(parent), _legend(NULL) {
	xAxis = new Axis(this);
	xAxis->setPosition(Axis::Bottom);
	xAxis->setGrid(true);

	yAxis = new Axis(this);
	yAxis->setPosition(Axis::Left);
	yAxis->setGrid(true);

	xAxis2 = new Axis(this);
	xAxis2->setPosition(Axis::Top);
	xAxis2->setVisible(false);

	yAxis2 = new Axis(this);
	yAxis2->setPosition(Axis::Right);
	yAxis2->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Graph *Plot::addGraph(Axis *keyAxis, Axis *valueAxis) {
	Graph *graph = new Graph(keyAxis == NULL ? xAxis : keyAxis,
	                         valueAxis == NULL ? yAxis : valueAxis,
	                         this);
	_graphs.append(graph);
	return graph;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::addGraph(Graph *graph) {
	graph->setParent(this);
	_graphs.append(graph);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::setLegend(AbstractLegend *legend) {
	if ( _legend == legend ) return;

	if ( _legend != NULL )
		delete _legend;

	_legend = legend;

	if ( _legend != NULL )
		_legend->setParent(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::updateRanges() {
	// Reset ranges
	xAxis->setRange(Range());
	yAxis->setRange(Range());
	xAxis2->setRange(Range());
	yAxis2->setRange(Range());

	foreach ( Graph *graph, _graphs ) {
		Range key, value;

		if ( graph->isEmpty() ) continue;
		graph->getBounds(key, value);

		graph->keyAxis()->extendRange(key);
		graph->valueAxis()->extendRange(value);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::draw(QPainter &p, const QRect &rect) {
	int xAxisHeight = xAxis->isVisible() ? xAxis->sizeHint(p) : 0;
	int xAxis2Height = xAxis2->isVisible() ? xAxis2->sizeHint(p) : 0;

	_plotRect = rect;
	_plotRect.adjust(0,xAxis2Height,0,-xAxisHeight);

	p.save();

	// Draw axis
	if ( yAxis->isVisible() ) {
		QRect yAxisRect(_plotRect.left(),_plotRect.top(),0,_plotRect.height());
		yAxis->updateLayout(p, yAxisRect);
		yAxis->draw(p, yAxisRect);
		_plotRect.adjust(yAxisRect.width()-1,0,0,0);
	}

	if ( yAxis2->isVisible() ) {
		QRect yAxis2Rect(_plotRect.right(),_plotRect.top(),0,_plotRect.height());
		yAxis2->updateLayout(p, yAxis2Rect);
		yAxis2->draw(p, yAxis2Rect);
		_plotRect.adjust(0,0,-(yAxis2Rect.width()+1),0);
	}

	if ( xAxis->isVisible() ) {
		QRect xAxisRect(_plotRect.left(),rect.bottom(),_plotRect.width(),0);
		xAxis->updateLayout(p, xAxisRect);
		xAxis->draw(p, xAxisRect);
	}

	if ( xAxis2->isVisible() ) {
		QRect xAxis2Rect(_plotRect.left(),rect.top(),_plotRect.width(),0);
		xAxis2->updateLayout(p, xAxis2Rect);
		xAxis2->draw(p, xAxis2Rect);
	}

	// Setup clipping
	if ( yAxis->isVisible() ) _plotRect.adjust( yAxis->pen().width(), 0, 0, 0);
	if ( xAxis2->isVisible() ) _plotRect.adjust( 0, xAxis2->pen().width(), 0, 0);
	if ( yAxis2->isVisible() ) _plotRect.adjust( 0, 0,-yAxis2->pen().width(), 0);
	if ( xAxis->isVisible() ) _plotRect.adjust( 0, 0, 0, -xAxis->pen().width());

	p.setClipRect(_plotRect);

	if ( xAxis->hasGrid() ) xAxis->drawGrid(p, _plotRect, true, false);
	if ( yAxis->hasGrid() ) yAxis->drawGrid(p, _plotRect, true, false);
	if ( xAxis2->hasGrid() ) xAxis2->drawGrid(p, _plotRect, true, false);
	if ( yAxis2->hasGrid() ) yAxis2->drawGrid(p, _plotRect, true, false);

	if ( xAxis->hasGrid() ) xAxis->drawGrid(p, _plotRect, false, true);
	if ( yAxis->hasGrid() ) yAxis->drawGrid(p, _plotRect, false, true);
	if ( xAxis2->hasGrid() ) xAxis2->drawGrid(p, _plotRect, false, true);
	if ( yAxis2->hasGrid() ) yAxis2->drawGrid(p, _plotRect, false, true);

	p.translate(_plotRect.left(), _plotRect.bottom()+1);
	foreach ( Graph *graph, _graphs ) {
		if ( graph->isEmpty() || !graph->isVisible() ) continue;
		graph->draw(p);
	}

	if ( (_legend != NULL) && _legend->isVisible() ) {
		p.translate(-_plotRect.left(), -_plotRect.bottom()-1);
		_legend->draw(p, _plotRect, _graphs);
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
