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


#include "plugin.h"
#include "mainwidget.h"
#include <seiscomp3/gui/core/application.h>


namespace Seiscomp {
namespace Gui {
namespace MessageMonitor {


Q_EXPORT_PLUGIN2(mm_preliminaryorigin, PreliminaryOrigin)


PreliminaryOrigin::PreliminaryOrigin() {
}


const char* PreliminaryOrigin::name() const {
	return "preliminary origin";
}


QWidget* PreliminaryOrigin::create(QWidget* parent, QObject* signaller) const {
	return new MainWidget(parent, signaller, SCApp->mapsDesc());
}


}
}
}
