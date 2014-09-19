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


#ifndef __SEISCOMP_PROCESSING_STREAMPROCESSOR_APPLICATION_H__
#define __SEISCOMP_PROCESSING_STREAMPROCESSOR_APPLICATION_H__


#include <seiscomp3/client/streamapplication.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/processing/waveformprocessor.h>
#include <seiscomp3/processing/timewindowprocessor.h>
#include <seiscomp3/processing/streambuffer.h>


namespace Seiscomp {

namespace Processing {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Application class for stream processing commandline applications.
  */
class SC_SYSTEM_CLIENT_API Application : public Client::StreamApplication {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Application(int argc, char **argv);

		//! D'tor
		~Application();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		StreamBuffer& streamBuffer();
		const StreamBuffer& streamBuffer() const;

		void addProcessor(const std::string& networkCode,
		                  const std::string& stationCode,
		                  const std::string& locationCode,
		                  const std::string& channelCode,
		                  WaveformProcessor *wp);

		void addProcessor(const std::string& networkCode,
		                  const std::string& stationCode,
		                  const std::string& locationCode,
		                  const std::string& channelCode,
		                  TimeWindowProcessor *twp);

		void removeProcessors(const std::string& networkCode,
		                      const std::string& stationCode,
		                      const std::string& locationCode,
		                      const std::string& channelCode);

		void removeProcessor(WaveformProcessor *wp);

		size_t processorCount() const;


	// ----------------------------------------------------------------------
	//  Protected methods
	// ----------------------------------------------------------------------
	protected:
		void addObject(const std::string& parentID, DataModel::Object* o);
		void removeObject(const std::string& parentID, DataModel::Object* o);
		void updateObject(const std::string& parentID, DataModel::Object* o);

		void handleRecord(Record *rec);

		void enableStation(const std::string& code, bool enabled);
		void enableStream(const std::string& code, bool enabled);

		virtual void handleNewStream(const Record *rec) {}
		virtual void processorFinished(const Record *rec, WaveformProcessor *wp) {}

		void done();


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void registerProcessor(const std::string& networkCode,
		                       const std::string& stationCode,
		                       const std::string& locationCode,
		                       const std::string& channelCode,
		                       WaveformProcessor *wp);

		void registerProcessor(const std::string& networkCode,
		                       const std::string& stationCode,
		                       const std::string& locationCode,
		                       const std::string& channelCode,
		                       TimeWindowProcessor *twp);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::multimap<std::string, WaveformProcessorPtr> StationProcessors;
		typedef std::multimap<std::string, WaveformProcessorPtr> ProcessorMap;
		typedef DataModel::WaveformStreamID                      WID;
		typedef std::pair<WID, WaveformProcessorPtr>             WaveformProcessorItem;
		typedef std::pair<WID, TimeWindowProcessorPtr>           TimeWindowProcessorItem;
		typedef std::list<WaveformProcessorItem>                 WaveformProcessorQueue;
		typedef std::list<TimeWindowProcessorItem>               TimeWindowProcessorQueue;

		ProcessorMap _processors;
		StationProcessors _stationProcessors;

		StreamBuffer _waveformBuffer;

		WaveformProcessorQueue _waveformProcessorQueue;
		TimeWindowProcessorQueue _timeWindowProcessorQueue;
		bool _registrationBlocked;
};


}

}

#endif
