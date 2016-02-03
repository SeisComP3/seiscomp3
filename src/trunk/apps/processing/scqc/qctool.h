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


#ifndef __SEISCOMP_APPLICATIONS_QCTOOL__
#define __SEISCOMP_APPLICATIONS_QCTOOL__

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/record.h>

#include <boost/version.hpp>
#include <boost/any.hpp>
#if (BOOST_VERSION < 104000)
#include <boost/signal.hpp>
namespace bsig = boost;
namespace boost_signals = boost::signals;
#else
#include <boost/signals2.hpp>
namespace bsig = boost::signals2;
namespace boost_signals = boost::signals2;
#endif

#include <string> 
#include <set> 
#include <map> 

#include <seiscomp3/plugins/qc/qcmessenger.h>
#include <seiscomp3/plugins/qc/qcplugin.h>
#include <seiscomp3/plugins/qc/qcconfig.h>

namespace Seiscomp {
namespace Applications {
namespace Qc {

class QcBuffer;
DEFINE_SMARTPOINTER(QcBuffer);


class QcTool : public QcApp, public boost_signals::trackable {
	public:
		QcTool(int argc, char **argv);
		~QcTool();

		QcMessenger* qcMessenger() const;
		bool exitRequested() const;

		typedef bsig::signal<void()> TimerSignal;
		void addTimeout(const TimerSignal::slot_type& onTimeout) const;
		bool archiveMode() const;
		std::string creatorID() const;


	protected:
		void createCommandLineDescription();
		bool validateParameters();
		bool initConfiguration();

		bool init();
		void done();

		void handleTimeout();

	private:
		void addStream(std::string net, std::string sta, std::string loc, std::string cha);
		Core::Time findLast(std::string net, std::string sta, std::string loc, std::string cha);

		void initQc(const std::string& networkCode,
		            const std::string& stationCode,
		            const std::string& locationCode,
		            const std::string& channelCode);
		
		void handleNewStream(const Record* rec);
		
		void processorFinished(const Record* rec, Processing::WaveformProcessor* wp);


	private:
		bool _archiveMode;
		bool _autoTime;
		Core::Time _beginTime;
		Core::Time _endTime;
		std::string _streamMask;

		bool _useConfiguredStreams;
		std::set<std::string> _streamIDs;

		std::string _creator;
		int _dbLookBack;
		std::map<std::string, QcConfigPtr> _plugins;
		std::set<std::string> _allParameterNames;
	
		double _maxGapLength;
		double _ringBufferSize;
		double _leadTime;
		
		QcMessenger* _qcMessenger;

		//! maps streamID's and associated qcPlugins
		typedef std::multimap<const std::string, QcPluginCPtr> QcPluginMap;
		QcPluginMap _qcPluginMap;

		mutable TimerSignal _emitTimeout;
		Util::StopWatch _timer;

};


}
}
}

#endif
