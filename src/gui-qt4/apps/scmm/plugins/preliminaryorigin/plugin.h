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


#ifndef __SEISCOMP_GUI_MESSAGEMONITOR_PLUGIN_PRELIMINARYORIGIN_H__
#define __SEISCOMP_GUI_MESSAGEMONITOR_PLUGIN_PRELIMINARYORIGIN_H__

#include "../../plugin.h"

namespace Seiscomp {
namespace Gui {
namespace MessageMonitor {


class PreliminaryOrigin : public QObject,
                          public Plugin {

	Q_OBJECT
	Q_INTERFACES(Seiscomp::Gui::MessageMonitor::Plugin)

	public:
		PreliminaryOrigin();

	public:
		const char* name() const;

		QWidget* create(QWidget* parent, QObject* signaller) const;
};

}
}
}

#endif
