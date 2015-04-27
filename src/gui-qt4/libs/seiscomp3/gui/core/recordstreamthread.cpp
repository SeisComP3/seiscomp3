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


#define SEISCOMP_COMPONENT RecordStreamThread
#include <seiscomp3/logging/log.h>

#include <seiscomp3/gui/core/recordstreamthread.h>

#include <iostream>

#include <seiscomp3/core/status.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/strings.h>

Q_DECLARE_METATYPE(Seiscomp::RecordPtr)

namespace Seiscomp {
namespace Gui {


int RecordStreamThread::_numberOfThreads = 0;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamThread::RecordStreamThread(const std::string& recordStreamURL)
: QThread(), _id(_numberOfThreads), _recordStreamURL(recordStreamURL)
{
	_requestedClose = false;
	_readingStreams = false;
	_dataType = Array::FLOAT;

	qRegisterMetaType<Seiscomp::RecordPtr>("Seiscomp::RecordPtr");

	++_numberOfThreads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamThread::~RecordStreamThread() {
	--_numberOfThreads;
	stop(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordStreamThread::ID() const {
	return _id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::connect()
{
	_requestedClose = false;
	_readingStreams = false;

	SEISCOMP_DEBUG("[rthread %d] trying to open stream '%s'", ID(), _recordStreamURL.c_str());

	_recordStream = IO::RecordStream::Open(_recordStreamURL.c_str());

	if (_recordStream == NULL)
	{
		SEISCOMP_ERROR("[rthread %d] could not create stream from URL %s", ID(), _recordStreamURL.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setStartTime(const Seiscomp::Core::Time& t) {
	if ( _recordStream == NULL ) return;
	_recordStream->setStartTime(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setEndTime(const Seiscomp::Core::Time& t) {
	if ( _recordStream == NULL ) return;
	_recordStream->setEndTime(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	if ( _recordStream == NULL ) return;

	if ( tw.startTime() )
		_recordStream->setStartTime(tw.startTime());

	if ( tw.endTime() )
		_recordStream->setEndTime(tw.endTime());

	SEISCOMP_DEBUG("[rthread %d] setting time window: start = %s, end = %s",
	               ID(),
	               tw.startTime().toString("%T %F").c_str(),
	               tw.endTime().toString("%T %F").c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::setTimeout(int seconds) {
	if ( _recordStream == NULL ) return false;
	return _recordStream->setTimeout(seconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStation(const std::string& network, const std::string& station) {
	if ( _recordStream == NULL ) return false;

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.??.???", ID(), network.c_str(), station.c_str());
	return _recordStream->addStream(network, station, "??", "???");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStream(const std::string& network, const std::string& station,
                                   const std::string& location, const std::string& channel) {
	if ( _recordStream == NULL ) return false;

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.%s.%s", ID(), network.c_str(), station.c_str(), location.c_str(), channel.c_str());
	return _recordStream->addStream(network, station, location, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStream(const std::string& network, const std::string& station,
                                   const std::string& location, const std::string& channel,
                                   const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime) {
	if ( _recordStream == NULL ) return false;

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.%s.%s - %s~%s", ID(),
	               network.c_str(), station.c_str(), location.c_str(), channel.c_str(),
	               stime.iso().c_str(), etime.iso().c_str());
	return _recordStream->addStream(network, station, location, channel, stime, etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStream(const std::string& network, const std::string& station,
                                   const std::string& location, const std::string& channel, 
                                   double gain) {
	if ( addStream(network,station, location, channel) ) {
		std::string id = station+"."+location+"."+channel;
		_gainMap.insert(make_pair(id, gain));
		return true;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::run()
{
	if ( _recordStream == NULL ) {
		SEISCOMP_DEBUG("[rthread %d] no stream source set, running aborted", ID());
		return;
	}

	SEISCOMP_DEBUG("[rthread %d] running record acquisition", ID());

	_mutex.lock();
	_readingStreams = true;
	_requestedClose = false;
	_mutex.unlock();

	RecordStreamState::Instance().openedConnection(this);

	_mutex.lock();
	IO::RecordInput recInput(_recordStream.get(), _dataType, Record::DATA_ONLY);
	_mutex.unlock();
	try {
		for (IO::RecordIterator it = recInput.begin(); it != recInput.end(); ++it)
		{
			bool stopAcquisition;
			_mutex.lock();
			stopAcquisition = _requestedClose;
			_mutex.unlock();
			if ( stopAcquisition ) {
				SEISCOMP_DEBUG("[rthread %d] close request leads to breaking the acquisition loop", ID());
				break;
			}
			Record* rec = *it;
			if ( rec ) {
				
				if ( !_gainMap.empty() ) {
					std::string id = rec->stationCode()+"."+rec->locationCode()+"."+rec->channelCode();
					GainMap::const_iterator git = _gainMap.find(id);
	
					if ( git != _gainMap.end() ) {
						const Array* data = rec->data();
						if ( git->second != 0) {
							double gain = 1.0f/git->second;
		
		
							if ( data && data->dataType() == Array::FLOAT ) {
								FloatArray* array = const_cast<FloatArray*>(static_cast<const FloatArray*>(data));
								for ( int i = 0; i < array->size(); ++i )
									array->set(i, array->get(i)*gain);
							} 
						}
					}
				}

				try {
					rec->endTime();
					emit receivedRecord(rec);
				}
				catch ( ... ) {
					SEISCOMP_ERROR("[rthread %d] Skipping invalid record for %s.%s.%s.%s (fsamp: %0.2f, nsamp: %d)",
					               ID(),
					               rec->networkCode().c_str(), rec->stationCode().c_str(), rec->locationCode().c_str(),
					               rec->channelCode().c_str(), rec->samplingFrequency(), rec->sampleCount());
				}
			}
		}
	}
	catch ( Core::OperationInterrupted &e ) {
		SEISCOMP_INFO("[rthread %d] acquisition exception: %s", ID(), e.what());
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("[rthread %d] acquisition exception: %s", ID(), e.what());
		handleError(QString(e.what()));
	}

	SEISCOMP_DEBUG("[rthread %d] finished record acquisition", ID());

	RecordStreamState::Instance().closedConnection(this);

	_mutex.lock();
	_readingStreams = false;
	_mutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::stop(bool waitForTermination)
{
	_mutex.lock();
	_requestedClose = true;
	if ( !_readingStreams ) {
		SEISCOMP_DEBUG("[rthread %d] actually no stream are being read", ID());
	}

	if (_recordStream && _readingStreams) {
		SEISCOMP_DEBUG("[rthread %d] about to close record acquisition stream", ID());
		_recordStream->close();
		SEISCOMP_DEBUG("[rthread %d] closed record acquisition stream", ID());
	}
	_mutex.unlock();

	if ( !isRunning () ) {
		SEISCOMP_DEBUG("[rthread %d] not running now", ID());
		wait();
	}

	if ( waitForTermination ) {
		SEISCOMP_DEBUG("waiting for thread %d to finish", ID());
		wait(5000);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& RecordStreamThread::recordStreamURL() const {
	return _recordStreamURL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Array::DataType RecordStreamThread::dataType() const {
	return _dataType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setDataType(Array::DataType dataType) {
	_dataType = dataType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState RecordStreamState::_instance;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState::RecordStreamState() : QObject() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState& RecordStreamState::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordStreamState::connectionCount() const {
	return _connectionCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<RecordStreamThread*> RecordStreamState::connections() const {
	return _activeThreads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamState::openedConnection(RecordStreamThread* thread) {
	++_connectionCount;
	_activeThreads.removeAll(thread);
	_activeThreads.append(thread);

	emit connectionEstablished(thread);

	if ( _connectionCount == 1 ) {
		SEISCOMP_DEBUG("First connection established");
		emit firstConnectionEstablished();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamState::closedConnection(RecordStreamThread* thread) {
	--_connectionCount;
	_activeThreads.removeAll(thread);

	if ( _connectionCount < 0 ) {
		assert(false);
		_connectionCount = 0;
	}

	emit connectionClosed(thread);

	if ( !_connectionCount ) {
		SEISCOMP_DEBUG("Last connection closed");
		emit lastConnectionClosed();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Gui
} // namespace Seiscomp
