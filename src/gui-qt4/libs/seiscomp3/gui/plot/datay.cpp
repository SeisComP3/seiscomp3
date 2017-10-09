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


#include <seiscomp3/gui/plot/datay.h>
#include <seiscomp3/gui/plot/axis.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataY::DataY(QObject *parent)
: AbstractDataSet(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DataY::count() const {
	return y.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Range DataY::getXRange() const {
	return x;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Range DataY::getYRange() const {
	int cnt = y.count();
	Range r;

	if ( !cnt ) return r;

	r.lower = r.upper = y[0];
	for ( int i = 1; i < cnt; ++i ) {
		if ( y[i] < r.lower ) r.lower = y[i];
		else if ( y[i] > r.upper ) r.upper = y[i];
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataY::clear() {
	y.clear();
	x = Range();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataY::unproject(QPolygonF &poly, const Axis *keyAxis,
                      const Axis *valueAxis) const {
	if ( y.count() < 2 ) return;

	double dx = x.length() / (y.count()-1);
	double px = x.lower;

	int i = 0;

	// Find first "visible" data sample
	while ( i < y.count() ) {
		if ( px >= keyAxis->range().lower ) {
			if ( i ) --i;
			break;
		}

		px += dx;
		++i;
	}

	bool lastVisible = true;
	for ( ; i < y.count(); ++i, px += dx ) {
		if ( px > keyAxis->range().upper ) {
			if ( !lastVisible ) break;
			lastVisible = false;
		}

		double x0 = keyAxis->unproject(px);
		double y0 = -valueAxis->unproject(y[i]);
		poly.append(QPointF(x0, y0));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
