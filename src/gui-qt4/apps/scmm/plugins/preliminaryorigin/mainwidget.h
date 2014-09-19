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


#ifndef __SEISCOMP_GUI_PLUGIN_MAPVIEWWIDGET_H__
#define __SEISCOMP_GUI_PLUGIN_MAPVIEWWIDGET_H__

#include <QtGui>
#include <seiscomp3/core/message.h>
#include <seiscomp3/gui/map/mapwidget.h>

namespace Seiscomp {
namespace Gui {
namespace MessageMonitor {


class MainWidget : public Seiscomp::Gui::MapWidget {
	Q_OBJECT

	public:
		MainWidget(QWidget* parent, QObject* signaller,
		           const MapsDesc &maps);

	public slots:
		void onReceivedMessage(Seiscomp::Core::MessagePtr);
};


}
}
}

#endif

