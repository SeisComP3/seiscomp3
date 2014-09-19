/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_WFPARAM_H__
#define __SEISCOMP_APPLICATIONS_WFPARAM_H__

#include <seiscomp3/client/streamapplication.h>
#include <seiscomp3/processing/amplitudeprocessor.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/journaling.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/utils/timer.h>

#define SEISCOMP_COMPONENT WfParam
#include <seiscomp3/logging/log.h>

#include "app.h"
#include "util.h"

#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <vector>


namespace Seiscomp {

class Record;

namespace DataModel {

class Pick;
class Origin;
class Event;

}


class WFParam : public Application {
	public:
		WFParam(int argc, char **argv);
		~WFParam();


	protected:
		void createCommandLineDescription();
		bool validateParameters();

		bool init();
		bool run();
		void done();

		bool storeRecord(Record *rec);
		void handleRecord(Record *rec);

		bool dispatchNotification(int type, Core::BaseObject *obj);

		void acquisitionFinished();

		void addObject(const std::string&, DataModel::Object* object);
		void updateObject(const std::string&, DataModel::Object* object);

		void handleTimeout();


	private:
		DEFINE_SMARTPOINTER(Process);

		bool addProcess(DataModel::Event *event);
		bool startProcess(Process *proc);
		void stopProcess(Process *proc);

		bool handle(DataModel::Event *event);
		bool handle(DataModel::Origin *origin);

		void process(DataModel::Origin *origin);

		void feed(DataModel::Pick *pick);

		int addProcessor(const DataModel::WaveformStreamID &streamID,
		                 DataModel::Stream *selectedStream,
		                 const Core::Time &time,
		                 Processing::WaveformProcessor::StreamComponent component
		                 = Processing::WaveformProcessor::Vertical);
		bool createProcessor(Record *rec);

		void removedFromCache(DataModel::PublicObject *);

		template <typename KEY, typename VALUE>
		bool getValue(VALUE &res, const std::map<KEY,VALUE> &map,
		              double ref) const;

		void setup(PGAVResult &res, Processing::PGAV *proc);
		void collectResults();
		void printReport();

		// Creates an event id from event. This id has the following format:
		// EventOriginTime_Mag_Lat_Lon_CreationDate
		// eg 20111210115715_12_46343_007519_20111210115740
		std::string generateEventID(const DataModel::Event *evt);


	private:
		struct Config {
			Config();

			std::vector<std::string> streamsWhiteList;
			std::vector<std::string> streamsBlackList;

			double totalTimeWindowLength;
			double preEventWindowLength;
			std::map<double,double> magnitudeTimeWindowTable;
			std::vector<std::string> vecMagnitudeTimeWindowTable;

			double maximumEpicentralDistance;
			std::map<double,double> magnitudeDistanceTable;
			std::vector<std::string> vecMagnitudeDistanceTable;

			std::map<double,FilterFreqs> magnitudeFilterTable;
			std::vector<std::string> vecMagnitudeFilterTable;

			double      saturationThreshold;
			double      STAlength;
			double      LTAlength;
			double      STALTAratio;
			double      STALTAmargin;

			std::vector<double> dampings;
			std::string naturalPeriodsStr;
			int         naturalPeriods;
			bool        naturalPeriodsLog;
			bool        naturalPeriodsFixed;
			double      Tmin;
			double      Tmax;

			bool        afterShockRemoval;
			bool        eventCutOff;

			double      fExpiry;
			std::string eventID;

			std::string eventParameterFile;

			bool        enableShortEventID;
			bool        enableShakeMapXMLOutput;
			std::string shakeMapOutputScript;
			std::string shakeMapOutputPath;
			bool        shakeMapOutputScriptWait;
			bool        shakeMapOutputSC3EventID;

			bool        enableMessagingOutput;

			std::string waveformOutputPath;
			bool        waveformOutputEventDirectory;

			std::string spectraOutputPath;
			bool        spectraOutputEventDirectory;

			bool        enableDeconvolution;
			bool        enableNonCausalFilters;
			double      taperLength;
			double      padLength;

			int         order;
			FilterFreqs filter;

			int         SCorder;
			FilterFreqs SCfilter;

			bool        useMaximumOfHorizontals;
			bool        offline;
			bool        testMode;
			bool        logCrontab;
			bool        saveProcessedWaveforms;
			bool        saveSpectraFiles;

			int         wakeupInterval;
			int         initialAcquisitionTimeout;
			int         runningAcquisitionTimeout;
			int         eventMaxIdleTime;

			double      magnitudeTolerance;
			bool        dumpRecords;

			// Cron options
			int         updateDelay;
			std::vector<int> delayTimes;
		};


		// Cronjob struct created per event
		DEFINE_SMARTPOINTER(Cronjob);
		struct Cronjob : public Core::BaseObject {
			std::list<Core::Time> runTimes;
		};

		typedef std::map<std::string, CronjobPtr> Crontab;

		struct CompareWaveformStreamID {
			bool operator()(const DataModel::WaveformStreamID &lhs,
			                const DataModel::WaveformStreamID &rhs) const;
		};

		typedef std::set<DataModel::WaveformStreamID,CompareWaveformStreamID> WaveformIDSet;
		struct StationRequest {
			Core::TimeWindow timeWindow;
			WaveformIDSet streams;
		};

		typedef std::vector<Processing::TimeWindowProcessorPtr>  ProcessorSlot;
		typedef std::map<std::string, ProcessorSlot>             ProcessorMap;
		typedef std::map<std::string, Processing::ParametersPtr> ParameterMap;
		typedef std::map<std::string, StationRequest>            RequestMap;

		typedef std::map<std::string, Processing::StreamPtr>     StreamMap;
		typedef DataModel::PublicObjectTimeSpanBuffer            Cache;

		void removeProcess(Crontab::iterator &, Process *proc);

		void dumpWaveforms(Process *p, PGAVResult &result,
		                   const Processing::PGAV *proc);

		void dumpSpectra(Process *p, const PGAVResult &result,
		                 const Processing::PGAV *proc);

		void writeShakeMapComponent(const PGAVResult *, bool &headerWritten,
		                            std::ostream *os, bool withComponent);

		typedef std::list<PGAVResult> PGAVResults;

		struct Process : Core::BaseObject {
			Core::Time          created;
			Core::Time          lastRun;
			Core::Time          referenceTime;
			DataModel::EventPtr event;
			PGAVResults         results;
			int                 remainingChannels;
			int                 newValidResults;

			OPT(double)         lastMagnitude;

			bool hasBeenProcessed(DataModel::Stream *) const;
		};

		typedef std::list<ProcessPtr>                            ProcessQueue;
		typedef std::map<std::string, ProcessPtr>                Processes;

		std::set<std::string>      _processedEvents;

		DataModel::EventParametersPtr _eventParameters;
		ProcessPtr                 _currentProcess;

		StreamMap                  _streams;
		Private::StringFirewall    _streamFirewall;

		TravelTimeTable            _travelTime;
		ProcessorMap               _processors;
		RequestMap                 _stationRequests;
		ParameterMap               _parameters;

		Crontab                    _crontab;
		ProcessQueue               _processQueue;
		Processes                  _processes;

		Cache                      _cache;

		bool                       _firstRecord;
		bool                       _receivedRecords;

		Config                     _config;

		Core::Time                 _originTime;
		double                     _latitude;
		double                     _longitude;
		double                     _depth;
		double                     _maximumEpicentralDistance;
		double                     _totalTimeWindowLength;
		FilterFreqs                _filter;
		int                        _cronCounter;
		int                        _acquisitionTimeout;

		Util::StopWatch            _acquisitionTimer;
		Util::StopWatch            _noDataTimer;

		Logging::Channel *_processingInfoChannel;
		Logging::Output  *_processingInfoOutput;

		std::stringstream _report;
		std::stringstream _result;

		std::ofstream     _recordDumpOutput;
};

}

#endif
