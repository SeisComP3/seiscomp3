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


#define SEISCOMP_COMPONENT MapView
#include <seiscomp3/logging/log.h>
#include <seiscomp3/gui/core/application.h>

#include "mvmainwindow.h"

using namespace Seiscomp;

class MvKicker : public Gui::Kicker<MvMainWindow> {
	public:
		MvKicker(int& argc, char **argv, int flags = DEFAULT)
		 : Gui::Kicker<MvMainWindow>(argc, argv, flags) {
			setLoadRegionsEnabled(true);
		}

	protected:
		virtual void createCommandLineDescription() {
			Gui::Kicker<MvMainWindow>::createCommandLineDescription();

			commandline().addGroup("Mapview");
			commandline().addOption("Mapview",
			                        "displaymode",
			                        "Start mapview as walldisplay. Modes: groundmotion, qualitycontrol",
			                        (std::string*)0,
			                        false);
			commandline().addOption("Mapview",
			                        "with-legend",
			                        "Shows the map legend if started as walldisplay");
		}

		virtual bool initUi(MvMainWindow* mvw) {
			return mvw->init();
		}

};




int main(int argc, char* argv[]) {
	int retCode;

	{
		Client::Application::HandleSignals(false, false);
		MvKicker app(argc, argv,
			Gui::Application::DEFAULT |
			Gui::Application::LOAD_STATIONS |
			Gui::Application::LOAD_CONFIGMODULE);
		app.setPrimaryMessagingGroup("GUI");
		app.addMessagingSubscription("AMPLITUDE");
		app.addMessagingSubscription("PICK");
		app.addMessagingSubscription("EVENT");
		app.addMessagingSubscription("LOCATION");
		app.addMessagingSubscription("MAGNITUDE");
		app.addMessagingSubscription("QC");
		app.addMessagingSubscription("CONFIG");
		app.addPluginPackagePath("qc");
		retCode = app();
		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	}
	
	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	return retCode;
}
