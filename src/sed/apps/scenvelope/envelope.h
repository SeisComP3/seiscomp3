/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
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


#ifndef __SEISCOMP_APPLICATIONS_SCENVELOPE_H__
#define __SEISCOMP_APPLICATIONS_SCENVELOPE_H__

#include <seiscomp3/client/streamapplication.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/utils/timer.h>

#include "app.h"
#include "processor.h"
#include "util.h"

#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>


// Comment this definition if a SC3 version is not used with correct implemented
// record acquisition synchronization.
#define SC3_SYNC_VERSION


namespace Seiscomp {


class Envelope : public Application {
	public:
		Envelope(int argc, char **argv);
		~Envelope();


	protected:
		void createCommandLineDescription();
		bool validateParameters();

		bool init();
		bool run();
		void done();

		void handleRecord(Record *rec);

	private:
		void addProcessor(DataModel::SensorLocation *loc,
		                  const DataModel::WaveformStreamID &id,
		                  const Core::Time &timestamp,
		                  const char *type, const char *short_type);

		void emitResult(const Processor *proc,
		                double acc, double vel, double disp,
		                const Core::Time &timestamp,
		                bool clipped);

#ifndef SC3_SYNC_VERSION
		// This method is called from a thread every second
		void resetMPSCount();
#endif

	private:
		struct Config {
			Config();

			std::vector<std::string> streamsWhiteList;
			std::vector<std::string> streamsBlackList;

			double      saturationThreshold;
			int         baselineCorrectionBufferLength;
			bool        useSC3Filter;

			std::string strTs;
			std::string strTe;

			Core::Time  ts;
			Core::Time  te;

#ifndef SC3_SYNC_VERSION
			int         maxMessageCountPerSecond;
#endif
		};

		typedef std::map<std::string, ProcessorPtr> Processors;

		Core::Time                       _appStartTime;
		Config                           _config;
		Private::StringFirewall          _streamFirewall;
		Processors                       _processors;
		DataModel::CreationInfo          _creationInfo;
		int                              _sentMessages;
		size_t                           _sentMessagesTotal;
#ifndef SC3_SYNC_VERSION
		Util::Timer                      _mpsReset;
#endif
};


}

#endif
