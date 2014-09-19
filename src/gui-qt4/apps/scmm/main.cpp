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




#include <seiscomp3/gui/core/application.h>
#include "mainframe.h"

using namespace Seiscomp::Gui;


class MMApp : public Kicker<MessageMonitor::MainFrame> {
	public:
		MMApp(int& argc, char** argv, int flags = DEFAULT) :
		  Kicker<MessageMonitor::MainFrame>(argc, argv, flags) {}

	protected:
		bool init() {
			if ( !Kicker<MessageMonitor::MainFrame>::init() )
				return false;

			connection()->subscribe("STATUS_GROUP");
			return true;
		}
};


int main(int argc, char *argv[])
{
	int retCode;

	{
		MMApp app(argc, argv, Application::SHOW_SPLASH | Application::WANT_MESSAGING | Application::OPEN_CONNECTION_DIALOG);
		app.setPrimaryMessagingGroup("LISTENER_GROUP");
		app.setMessagingUsername("");
		app.addMessagingSubscription("*");
		retCode = app();
		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	}

	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	return retCode;
}
