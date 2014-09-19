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


#ifndef __SEISCOMP_CLIENT_STREAM_APPLICATION_H__
#define __SEISCOMP_CLIENT_STREAM_APPLICATION_H__


#include <seiscomp3/client/application.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/utils/mutex.h>


namespace Seiscomp {

namespace Client {


class SC_SYSTEM_CLIENT_API StreamApplication : public Application {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		StreamApplication(int argc, char **argv);
		~StreamApplication();


	public:
		bool openStream();
		void closeStream();

		IO::RecordStream* recordStream() const;

		bool addStation(const std::string& networkCode,
		                const std::string& stationCode);
		bool addStream(const std::string& networkCode,
		               const std::string& stationCode,
		               const std::string& locationCode,
		               const std::string& channelCode);

		void setStartTime(const Seiscomp::Core::Time&);
		void setEndTime(const Seiscomp::Core::Time&);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow&);

		//! Sets whether to start the acquisition automatically
		//! before the run loop or not. This method has to be called before run().
		//! The default is true. If set to false then the acquisition needs
		//! to be started with readRecords or startRecordThread and
		//! autoCloseOnAcquisitionFinished is also set to false.
		void setAutoAcquisitionStart(bool);

		//! Sets the application close flag when acquisition is finished.
		//! The default is true as auto start is true. If setAutoAcquisitionStart
		//! is changed this flag is set as well.
		void setAutoCloseOnAcquisitionFinished(bool);

		//! Request locking of record thread and send a sync request to the
		//! application after the last record has been flushed.
		void requestSync();

		//! Sets the storage hint of incoming records.
		//! The default is: DATA_ONLY
		void setRecordInputHint(Record::Hint hint);


		void startRecordThread();
		void waitForRecordThread();
		bool isRecordThreadActive() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		bool init();
		bool run();
		void done();
		void exit(int returnCode);

		bool dispatch(Core::BaseObject* obj);

		void readRecords(bool sendEndNotification);

		//! This method gets called when the acquisition is finished
		//! The default implementation closes the objects queue and
		//! finishes the application
		virtual void acquisitionFinished();

		//! This method gets called when a new record has been received
		//! The default implementation stores it in the threaded object
		//! queue which gets read by the main thread
		virtual bool storeRecord(Record *rec);

		virtual void handleRecord(Record *rec) = 0;

		//! Logs the received records for the last period
		virtual void handleMonitorLog(const Core::Time &timestamp);

		//! Unlocks record acquisiton.
		virtual void handleEndSync();


	private:
		bool                _startAcquisition;
		bool                _closeOnAcquisitionFinished;
		Record::Hint        _recordInputHint;
		IO::RecordStreamPtr _recordStream;
		boost::thread      *_recordThread;
		size_t              _receivedRecords;
		ObjectLog          *_logRecords;
		bool                _requestSync;
		Util::mutex         _recordLock;
};


}

}

#endif
