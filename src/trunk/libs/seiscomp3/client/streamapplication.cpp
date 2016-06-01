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


#define SEISCOMP_COMPONENT StreamApplication

#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/client/streamapplication.h>

#include <boost/bind.hpp>


using namespace Seiscomp;
using namespace Seiscomp::Client;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamApplication::StreamApplication(int argc, char **argv)
	: Client::Application(argc, argv), _recordThread(NULL) {
	setRecordStreamEnabled(true);
	_startAcquisition = true;
	_closeOnAcquisitionFinished = true;
	_recordInputHint = Record::DATA_ONLY;
	_recordDatatype = Array::FLOAT;
	_logRecords = NULL;
	_receivedRecords = 0;
	_requestSync = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamApplication::~StreamApplication() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::init() {
	if ( !Client::Application::init() )
		return false;

	_logRecords = addInputObjectLog("record");
	_receivedRecords = 0;

	try {
		std::string inputFile = commandline().option<std::string>("record-file");
		std::string type = "";

		try {
			type = commandline().option<std::string>("record-type");
		} catch ( ... ) {}


		_recordStream = IO::RecordStream::Create("file");
		if ( !_recordStream ) {
			SEISCOMP_ERROR("Failed to create recordstream 'file'");
			return false;
		}

		if ( !type.empty() ) {
			if ( !_recordStream->setRecordType(type.c_str()) ) {
				SEISCOMP_ERROR("Failed to set recordtype to '%s'", type.c_str());
				return false;
			}
		}

		if ( !_recordStream->setSource(inputFile) ) {
			SEISCOMP_ERROR("Failed to open recordfile %s", inputFile.c_str());
			return false;
		}
	}
	catch ( ... ) {
		openStream();
	}

	if ( !_recordStream ) {
		SEISCOMP_ERROR("Failed to open recordstream %s", recordStreamURL().c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::run() {
	if ( _startAcquisition )
		startRecordThread();
	return Client::Application::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::done() {
	Application::done();

	_receivedRecords = 0;

	if ( _recordStream )
		_recordStream->close();

	waitForRecordThread();

	_recordStream = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::exit(int returnCode) {
	Client::Application::exit(returnCode);
	if ( _recordStream )
		_recordStream->close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::dispatch(Core::BaseObject* obj) {
	if ( Client::Application::dispatch(obj) ) return true;
	Record *rec = Record::Cast(obj);
	if ( rec ) {
		handleRecord(rec);
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::handleMonitorLog(const Core::Time &timestamp) {
	if ( _logRecords ) logObject(_logRecords, timestamp, _receivedRecords);
	_receivedRecords = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setAutoAcquisitionStart(bool e) {
	_startAcquisition = e;
	_closeOnAcquisitionFinished = _startAcquisition;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setAutoCloseOnAcquisitionFinished(bool e) {
	_closeOnAcquisitionFinished = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setRecordInputHint(Record::Hint hint) {
	_recordInputHint = hint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setRecordDatatype(Array::DataType datatype) {
	_recordDatatype = datatype;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::startRecordThread() {
	_recordThread = new boost::thread(boost::bind(&StreamApplication::readRecords, this, true));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::waitForRecordThread() {
	if ( _recordThread ) {
		SEISCOMP_INFO("Waiting for record thread");
		if ( !_recordLock.try_lock() )
			SEISCOMP_DEBUG("Releasing acquisition lock obtained from acquisition thread");
		_recordLock.unlock();
		_recordThread->join();
		if ( !_recordLock.try_lock() )
			SEISCOMP_DEBUG("Releasing acquisition lock (2) obtained from acquisition thread");
		_recordLock.unlock();
		delete _recordThread;
		_recordThread = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::isRecordThreadActive() const {
	return _recordThread != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::RecordStream* StreamApplication::recordStream() const {
	return _recordStream.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::addStation(const std::string& networkCode,
                                   const std::string& stationCode) {
	if ( _recordStream )
		return _recordStream->addStream(networkCode, stationCode, "??", "???");

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setStartTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream )
		_recordStream->setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::setEndTime(const Seiscomp::Core::Time& time) {
	if ( _recordStream )
		_recordStream->setEndTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	if ( !_recordStream ) return false;

	return _recordStream->setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::addStream(const std::string& networkCode,
                                  const std::string& stationCode,
                                  const std::string& locationCode,
                                  const std::string& channelCode) {
	if ( _recordStream )
		return _recordStream->addStream(networkCode, stationCode, locationCode, channelCode);

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::openStream() {
	// If there is already an active recordstream return false
	if ( _recordStream ) return false;

	_recordStream = IO::RecordStream::Open(recordStreamURL().c_str());
	return _recordStream.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::closeStream() {
	if ( _recordStream )
		_recordStream->close();

	_recordStream = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::readRecords(bool sendEndNotification) {
	SEISCOMP_INFO("Starting record acquisition");

	IO::RecordInput recInput(_recordStream.get(), _recordDatatype, _recordInputHint);
	try {
		for ( IO::RecordIterator it = recInput.begin(); it != recInput.end(); ++it ) {
			Record* rec = *it;
			if ( rec ) {
				try {
					rec->endTime();
					if ( !storeRecord(rec) ) {
						delete rec;
						return;
					}
					++_receivedRecords;
				}
				catch ( ... ) {
					delete rec;
					SEISCOMP_ERROR("Skipping invalid record for %s.%s.%s.%s (fsamp: %0.2f, nsamp: %d)",
					               rec->networkCode().c_str(), rec->stationCode().c_str(), rec->locationCode().c_str(),
					               rec->channelCode().c_str(), rec->samplingFrequency(), rec->sampleCount());
				}
			}
		}
	}
	catch ( Core::OperationInterrupted& e ) {
		SEISCOMP_INFO("Interrupted acquisition, msg: '%s'", e.what());
	}
	catch ( std::exception& e ) {
		SEISCOMP_ERROR("Exception in acquisition: '%s'", e.what());
	}

	if ( sendEndNotification )
		sendNotification(Notification::AcquisitionFinished);

	SEISCOMP_INFO("Finished acquisition");
	acquisitionFinished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::acquisitionFinished() {
	if ( _closeOnAcquisitionFinished ) {
		SEISCOMP_INFO("Sending close event after finishing acquisition");
		sendNotification(Notification::Close);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StreamApplication::storeRecord(Record *rec) {
	_recordLock.lock();
	bool r = _queue.push(rec);
	if ( _requestSync ) {
		_requestSync = false;
		sendNotification(Notification::Sync);
	}
	else {
		_recordLock.unlock();
	}
	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::requestSync() {
	_requestSync = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamApplication::handleEndSync() {
	_requestSync = false;
	_recordLock.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
