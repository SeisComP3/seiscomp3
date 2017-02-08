/***************************************************************************
 *   Copyright (C) by ASGSR
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#define SEISCOMP_COMPONENT SendEvent



#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/messages.h>
#include <seiscomp3/logging/log.h>

#include "sendevent.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Gui;


SendEvent::SendEvent(int& argc, char **argv, Seiscomp::Gui::Application::Type type)
: Application(argc, argv, 0, type) {
	setMessagingEnabled(true);
	setMessagingUsername("scsendevt");
	setDatabaseEnabled(true,false);
	setRecordStreamEnabled(false);
	setLoadRegionsEnabled(false);
	setLoadCitiesEnabled(false);
	addLoggingComponentSubscription("Application");
	setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
}


bool SendEvent::run() {

        if ( !_eventID.empty() ) {

        	PublicObjectPtr po = SCApp->query()->loadObject(Event::TypeInfo(), _eventID);
        	EventPtr e = Event::Cast(po);
        	if ( !e ) {
			SEISCOMP_WARNING("Event %s has not been found.\n", _eventID.c_str());
			cerr << "Warning: EventID " << _eventID.c_str() << " has not been found.\n";
                	return false;
        	}

		// Workaround for not to open window QMessageBox in the Application::sendCommand
		// when application is commandline
		if ( commandTarget().empty() && SCApp->type() == QApplication::Tty ) {
                	cerr << "WARNING: \n"
                            "\tVariable <commands.target> is not set. To disable sending commands \n"
                            "\tto all connected clients, set a proper target. You can use \n"
                            "\tregular expressions to specify a group of clients (HINT: all = '.*$').\n\n";
                	return false;
		}

        	SCApp->sendCommand(Gui::CM_SHOW_ORIGIN, e->preferredOriginID());
		return true;
	} 

	cerr << "must specify event using '-E eventID'\n";
	return false;
}


void SendEvent::createCommandLineDescription() {
	commandline().addGroup("Options");
	commandline().addOption("Options", "event-id,E", "eventID to show details", &_eventID, false);
}

