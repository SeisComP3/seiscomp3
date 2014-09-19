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



#include <seiscomp3/gui/core/gradient.h>

namespace Seiscomp {
namespace Gui {


namespace {

QColor blend(const QColor& c1, const QColor& c2, qreal ratio) {
	qreal invRatio = 1-ratio;
	return QColor((int)(c1.red()*invRatio + c2.red()*ratio),
	              (int)(c1.green()*invRatio + c2.green()*ratio),
	              (int)(c1.blue()*invRatio + c2.blue()*ratio),
	              (int)(c1.alpha()*invRatio + c2.alpha()*ratio));
}


}


Gradient::Gradient() {}


void Gradient::setColorAt(qreal position, const QColor &color, const QString& text) {
	insert(position, qMakePair(color, text));
}


QColor Gradient::colorAt(qreal position, bool discrete) const {
	const_iterator last = end();
	for ( const_iterator it = begin(); it != end(); ++it ) {
		if ( it.key() == position )
			return it.value().first;
		else if ( it.key() > position ) {
			if ( last != end() ) {
				if ( discrete ) {
					return (last.key()>=0)?last.value().first: it.value().first;
				}
				else {
					qreal v1 = last.key();
					qreal v2 = it.key();
					return blend(last.value().first, it.value().first, (position-v1)/(v2-v1));
				}
			}
			else
				return it.value().first;
		}

		last = it;
	}

	if ( last != end() )
		return last.value().first;

	return QColor();
}


}
}
