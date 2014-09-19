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




#define SEISCOMP_COMPONENT EventSummaryView

#include "mainframe.h"
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/eventsummaryview.h>
#include <seiscomp3/logging/log.h>

typedef Seiscomp::Applications::EventSummaryView::MainFrame MainWindow;
typedef Seiscomp::Gui::Kicker<MainWindow> Kicker;

class ESVApp : public Kicker {
	public:
		ESVApp(int& argc, char** argv, int flags = DEFAULT) :
		  Kicker(argc, argv, flags) {
			setRecordStreamEnabled(false);
			setLoadRegionsEnabled(true);
		}

	protected:
		void createCommandLineDescription() {
			Kicker::createCommandLineDescription();

			commandline().addOption(
				"Generic", "script0",
				"path to the script called when configurable button0 is pressed; "
				"EventID, arrival count, magnitude and the additional location information string are passed as parameters $1, $2, $3 and $4",
				&_script0);

			commandline().addOption(
				"Generic", "script1",
				"path to the script called when configurable button1 is pressed; "
				"EventID, arrival count, magnitude and the additional location information string are passed as parameters $1, $2, $3 and $4",
				&_script1);
		}

		bool initConfiguration() {
			if ( !Kicker::initConfiguration() ) return false;

			try {
				_script0 = Seiscomp::Environment::Instance()->absolutePath(configGetString("scripts.script0"));
			}
			catch ( ... ) {}

			try {
				_scriptStyle0 = configGetBool("scripts.script0.oldStyle");
			}
			catch ( ... ) {
				_scriptStyle0 = true;
			}

			try {
				_script1 = Seiscomp::Environment::Instance()->absolutePath(configGetString("scripts.script1"));
			}
			catch ( ... ) {}

			try {
				_scriptStyle1 = configGetBool("scripts.script1.oldStyle");
			}
			catch ( ... ) {
				_scriptStyle1 = true;
			}

			return true;
		}

		void setupUi(MainWindow *mw) {
			mw->eventSummaryView()->setScript0(_script0, _scriptStyle0);
			mw->eventSummaryView()->setScript1(_script1, _scriptStyle1);
		}

	public:
		std::string _script0;
		std::string _script1;
		bool        _scriptStyle0;
		bool        _scriptStyle1;
};


int main(int argc, char** argv) {
	int retCode;

	{
		ESVApp app(argc, argv, Seiscomp::Gui::Application::DEFAULT | Seiscomp::Gui::Application::LOAD_STATIONS);
		app.setPrimaryMessagingGroup("GUI");
		app.addMessagingSubscription("EVENT");
		app.addMessagingSubscription("MAGNITUDE");
		app.addMessagingSubscription("LOCATION");
		app.addMessagingSubscription("FOCMECH");
		retCode = app();
		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	}

	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Seiscomp::Core::BaseObject::ObjectCount());
	return retCode;
}
