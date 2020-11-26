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



#ifndef __SEISCOMP_GUI_PICKERZOOMFRAME_H__
#define __SEISCOMP_GUI_PICKERZOOMFRAME_H__

#include <QFrame>
#include <seiscomp3/gui/qt4.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API ZoomRecordFrame : public QFrame {
	Q_OBJECT

	public:
		ZoomRecordFrame(QWidget* parent = 0, Qt::WindowFlags f = 0);

	protected:
		void wheelEvent(QWheelEvent* e);

	signals:
		void lineDown();
		void lineUp();

		void verticalZoomIn();
		void verticalZoomOut();

		void horizontalZoomIn();
		void horizontalZoomOut();
};


}
}


#endif


