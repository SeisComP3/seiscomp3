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



#ifndef __SEISCOMP_GUI_CORE_GRADIENT_H__
#define __SEISCOMP_GUI_CORE_GRADIENT_H__

#include <seiscomp3/gui/qt4.h>

#include <QColor>
#include <QMap>
#include <QPair>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API Gradient : public QMap<qreal, QPair<QColor, QString> > {
	public:
		typedef QPair<QColor, QString> ValueType;

	public:
		Gradient();

	public:
		//! Sets the color and text at a specified position
		void setColorAt(qreal position, const QColor &color,
		                const QString& text = "");

		//! Returns the color at position. If position falls between
		//! two color positions, the resulting color will be
		//! interpolated linearly between both colors. When the discrete flag
		//! is set, the interpolation will be disabled
		QColor colorAt(qreal position, bool discrete = false) const;
};


}
}


# endif
