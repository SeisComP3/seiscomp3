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



#include <QWheelEvent>
#include <seiscomp3/gui/datamodel/pickerzoomframe.h>

namespace Seiscomp {
namespace Gui {


ZoomRecordFrame::ZoomRecordFrame(QWidget* parent, Qt::WFlags f)
 : QFrame(parent, f) {}


void ZoomRecordFrame::wheelEvent(QWheelEvent* e) {
	if ( e->modifiers() != Qt::NoModifier ) {
		if ( e->modifiers() & Qt::ShiftModifier ) {
			if ( e->delta() < 0 )
				emit horizontalZoomOut();
			else
				emit horizontalZoomIn();
		}
	
		if ( e->modifiers() & Qt::ControlModifier ) {
			if ( e->delta() < 0 )
				emit verticalZoomOut();
			else
				emit verticalZoomIn();
		}
	}
	else {
		if ( e->delta() < 0 )
			emit lineDown();
		else
			emit lineUp();
	}
}


}
}
