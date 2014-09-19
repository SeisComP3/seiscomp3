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


#ifndef __SEISCOMP_GUI_MESSAGEBROWSER_PLUGIN_H__
#define __SEISCOMP_GUI_MESSAGEBROWSER_PLUGIN_H__

#include <QtGui>

namespace Seiscomp {

namespace Client {

class Message;

}

namespace Gui {
namespace MessageMonitor {

class Plugin {
	public:
		virtual ~Plugin() {}

	public:
		virtual const char* name() const = 0;
	
		virtual QWidget* create(QWidget* parent, QObject* signaller) const = 0;
};

}
}
}

Q_DECLARE_INTERFACE(Seiscomp::Gui::MessageMonitor::Plugin,
                    "de.gfz.seiscomp3.MessageMonitor.Plugin/1.0")

#endif
