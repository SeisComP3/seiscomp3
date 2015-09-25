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




#ifndef __SEISCOMP_APPLICATIONS_LOCATOR__
#define __SEISCOMP_APPLICATIONS_LOCATOR__

#include <queue>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/client/application.h>
#include "autoloc.h"


namespace Seiscomp {

namespace Applications {

namespace Autoloc {


class App : public Client::Application,
            protected ::Autoloc::Autoloc3 {
	public:
		App(int argc, char **argv);
		~App();


	public:
		bool feed(DataModel::Pick*);
		bool feed(DataModel::Amplitude*);
		bool feed(DataModel::Origin*);


	protected:
		void createCommandLineDescription();
		bool validateParameters();
		bool initConfiguration();
		bool initStations();

		void readHistoricEvents();

		bool init();
		bool run();
		void done();

		void handleMessage(Core::Message* msg);
		void handleTimeout();
		void handleAutoShutdown();

		void addObject(const std::string& parentID, DataModel::Object *o);
		void removeObject(const std::string& parentID, DataModel::Object *o);
		void updateObject(const std::string& parentID, DataModel::Object *o);

		virtual bool _report(const ::Autoloc::Origin *origin);
//		bool runFromPickFile();
		bool runFromXMLFile(const char *fname);
		bool runFromEPFile(const char *fname);


	protected:
//		DataModel::Origin *convertToSC3  (const ::Autoloc::Origin* origin, bool allPhases=true);
		::Autoloc::Origin *convertFromSC3(const DataModel::Origin* sc3origin);
		::Autoloc::Pick   *convertFromSC3(const DataModel::Pick*   sc3pick);

	private:
		std::string _inputFileXML; // for XML playback
		std::string _inputEPFile; // for offline processing
		std::string _stationLocationFile;
		std::string _gridConfigFile;
		std::string _amplTypeAbs, _amplTypeSNR;

		std::queue<DataModel::PublicObjectPtr> _objects; // for XML playback
		double _playbackSpeed;
		Core::Time playbackStartTime;
		Core::Time  objectsStartTime;
		unsigned int objectCount;

		DataModel::EventParametersPtr _ep;

		::Autoloc::Autoloc3::Config _config;
		int _keepEventsTimeSpan;
		int _wakeUpTimout;

		std::map<std::string, DataModel::AmplitudePtr> ampmap;

		ObjectLog   *_inputPicks;
		ObjectLog   *_inputAmps;
		ObjectLog   *_inputOrgs;
		ObjectLog   *_outputOrgs;
};


}

}

}

#endif
