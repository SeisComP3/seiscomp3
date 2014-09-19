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




#define SEISCOMP_COMPONENT OriginLocatorView
#include <seiscomp3/logging/log.h>
#include <seiscomp3/gui/core/application.h>

#include "mainframe.h"

using namespace Seiscomp::Gui;


class OLVApp : public Kicker<MainFrame> {
	public:
		OLVApp(int& argc, char** argv, int flags = DEFAULT) :
		  Kicker<MainFrame>(argc, argv, flags) {
			setLoadRegionsEnabled(true);
			_preloadDays = 1;
		}

	protected:
		bool initConfiguration() {
			if ( !Kicker<MainFrame>::initConfiguration() )
				return false;

			try { _preloadDays = configGetDouble("loadEventDB"); }
			catch ( ... ) {}

			return true;
		}

		void createCommandLineDescription() {
			Kicker<MainFrame>::createCommandLineDescription();

			commandline().addGroup("Options");
			commandline().addOption("Options", "origin,O", "Preload origin", &_originID);
			commandline().addOption("Options", "event,E", "Preload event",  &_eventID);
			commandline().addOption("Options", "offline", "Switch to offline mode");
			commandline().addOption("Options", "load-event-db", "Number of days to load from database", &_preloadDays);
		}

		bool validateParameters() {
			if ( !Kicker<MainFrame>::validateParameters() ) return false;

			if ( commandline().hasOption("offline") ) {
				setMessagingEnabled(false);
				if ( !isInventoryDatabaseEnabled() )
					setDatabaseEnabled(false, false);
			}

			return true;
		}

		void setupUi(MainFrame* w) {
			if ( commandline().hasOption("offline") )
				w->setOffline(true);
			else if ( !_eventID.empty() )
				w->setEventID(_eventID);
			else if ( !_originID.empty() )
				w->setOriginID(_originID);
			else
				w->loadEvents(_preloadDays);
		}

	private:
		std::string _originID;
		std::string _eventID;
		float _preloadDays;
};



int main(int argc, char** argv) {
	int retCode;

	{
		OLVApp app(argc, argv, Application::DEFAULT | Application::LOAD_INVENTORY | Application::LOAD_CONFIGMODULE);
		app.setPrimaryMessagingGroup("LOCATION");
		app.addMessagingSubscription("EVENT");
		app.addMessagingSubscription("LOCATION");
		app.addMessagingSubscription("FOCMECH");
		app.addMessagingSubscription("MAGNITUDE");
		app.addMessagingSubscription("PICK");
		app.addMessagingSubscription("CONFIG");
		app.addMessagingSubscription("GUI");
		retCode = app();
		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	}

	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	return retCode;
}
