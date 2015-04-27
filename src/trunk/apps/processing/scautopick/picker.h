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




#ifndef __SEISCOMP_APPLICATIONS_PICKER__
#define __SEISCOMP_APPLICATIONS_PICKER__

#include <seiscomp3/processing/application.h>
#include <seiscomp3/processing/detector.h>
#include <seiscomp3/processing/picker.h>
#include <seiscomp3/processing/secondarypicker.h>
#include <seiscomp3/processing/amplitudeprocessor.h>

#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/stationmagnitude.h>

#include <list>

#include "config.h"
#include "stationconfig.h"


namespace Seiscomp {

namespace Applications {

namespace Picker {


class App : public Processing::Application {
	public:
		App(int argc, char **argv);
		~App();


	protected:
		void createCommandLineDescription();
		bool validateParameters();
		bool initConfiguration();

		bool init();
		bool run();
		void done();

		void addObject(const std::string& parentID, DataModel::Object* o);
		void removeObject(const std::string& parentID, DataModel::Object* o);
		void updateObject(const std::string& parentID, DataModel::Object* o);


	private:
		// Initializes a single component of a processor.
		bool initComponent(Processing::WaveformProcessor *proc,
		                   Processing::WaveformProcessor::Component comp,
		                   const Core::Time &time,
		                   const std::string &streamID,
		                   const std::string &networkCode,
		                   const std::string &stationCode,
		                   const std::string &locationCode,
		                   const std::string &channelCode,
		                   bool metaDataRequired);

		// Initializes a processor which can use multiple components. This
		// method calls initComponent for each requested component.
		bool initProcessor(Processing::WaveformProcessor *proc,
		                   Processing::WaveformProcessor::StreamComponent comp,
		                   const Core::Time &time,
		                   const std::string &streamID,
		                   const std::string &networkCode,
		                   const std::string &stationCode,
		                   const std::string &locationCode,
		                   const std::string &channelCode,
		                   bool metaDataRequired);

		bool initDetector(const std::string &streamID,
		                  const std::string &networkCode,
		                  const std::string &stationCode,
		                  const std::string &locationCode,
		                  const std::string &channelCode,
		                  const Core::Time &time);

		void addSecondaryPicker(const Core::Time &onset, const Record *rec,
		                        const std::string& pickID);
		void addAmplitudeProcessor(Processing::AmplitudeProcessorPtr proc,
		                           const Record *rec,
		                           const std::string& pickID);

		void handleNewStream(const Record *rec);
		void processorFinished(const Record *rec, Processing::WaveformProcessor *wp);

		void emitTrigger(const Processing::Detector *pickProc,
		                 const Record *rec, const Core::Time& time);

		void emitDetection(const Processing::Detector *pickProc,
		                   const Record *rec, const Core::Time& time);

		void emitPPick(const Processing::Picker *,
		               const Processing::Picker::Result &);

		void emitSPick(const Processing::SecondaryPicker *,
		               const Processing::SecondaryPicker::Result &);

		void emitAmplitude(const Processing::AmplitudeProcessor *ampProc,
		                   const Processing::AmplitudeProcessor::Result &res);


	private:
		typedef std::map<std::string, Processing::StreamPtr> StreamMap;
		typedef std::map<std::string, DataModel::PickPtr> PickMap;

		typedef Processing::WaveformProcessor TWProc;

		struct ProcEntry {
			ProcEntry(const Core::Time &t, TWProc *p)
			: dataEndTime(t), proc(p) {}

			Core::Time  dataEndTime;
			TWProc     *proc;
		};

		typedef std::list<ProcEntry>  ProcList;
		typedef std::map<std::string, ProcList> ProcMap;
		typedef std::map<TWProc*, std::string> ProcReverseMap;
		typedef DataModel::EventParametersPtr EP;

		int            _sentMessages;
		StreamMap      _streams;
		Config         _config;
		PickMap        _lastPicks;

		ProcMap        _runningStreamProcs;
		ProcReverseMap _procLookup;

		StringSet      _streamIDs;

		StationConfig  _stationConfig;
		EP             _ep;

		ObjectLog    *_logPicks;
		ObjectLog    *_logAmps;
};


}

}

}

#endif
