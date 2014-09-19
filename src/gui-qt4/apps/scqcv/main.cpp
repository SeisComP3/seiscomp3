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




#define SEISCOMP_COMPONENT QcView

#include <seiscomp3/client/pluginregistry.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/logging/log.h>
#include "mainframe.h"

typedef Seiscomp::Gui::MainFrame MainWindow;
typedef Seiscomp::Gui::Kicker<MainWindow> Kicker;


int main(int argc, char** argv) {
	int retCode;

	{
		Kicker app(argc, argv, Seiscomp::Gui::Application::DEFAULT|Seiscomp::Gui::Application::LOAD_INVENTORY|Seiscomp::Gui::Application::LOAD_CONFIGMODULE);

		app.setPrimaryMessagingGroup("GUI");
		app.addMessagingSubscription("QC");


		retCode = app();

		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	}

	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	return retCode;
}
