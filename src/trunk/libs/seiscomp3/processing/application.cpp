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


#define SEISCOMP_COMPONENT ProcessingApplication

#include <seiscomp3/processing/application.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {

namespace Processing {


Application::Application(int argc, char **argv)
: Client::StreamApplication(argc, argv), _waveformBuffer(30.*60.) {
	_registrationBlocked = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamBuffer& Application::streamBuffer() {
	return _waveformBuffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const StreamBuffer& Application::streamBuffer() const {
	return _waveformBuffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addProcessor(const std::string& networkCode,
                               const std::string& stationCode,
                               const std::string& locationCode,
                               const std::string& channelCode,
                               WaveformProcessor *wp) {
	if ( _registrationBlocked ) {
		_waveformProcessorQueue.push_back(
			WaveformProcessorItem(WID(networkCode, stationCode,
			                          locationCode, channelCode, ""), wp))
		;
		return;
	}

	registerProcessor(networkCode, stationCode,
	                  locationCode, channelCode, wp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addProcessor(const DataModel::WaveformStreamID &wfid,
			       WaveformProcessor *proc) {
	addProcessor(wfid.networkCode(),
		     wfid.stationCode(),
		     wfid.locationCode(),
		     wfid.channelCode(),
		     proc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addProcessor(const std::string& networkCode,
                               const std::string& stationCode,
                               const std::string& locationCode,
                               const std::string& channelCode,
                               TimeWindowProcessor *twp) {
	if ( _registrationBlocked ) {
		_timeWindowProcessorQueue.push_back(
			TimeWindowProcessorItem(WID(networkCode, stationCode,
			                            locationCode, channelCode, ""), twp))
		;
		return;
	}

	registerProcessor(networkCode, stationCode,
	                  locationCode, channelCode, twp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addProcessor(const DataModel::WaveformStreamID &wfid,
			       TimeWindowProcessor *proc) {
	addProcessor(wfid.networkCode(),
		     wfid.stationCode(),
		     wfid.locationCode(),
		     wfid.channelCode(),
		     proc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::registerProcessor(const std::string& networkCode,
                                    const std::string& stationCode,
                                    const std::string& locationCode,
                                    const std::string& channelCode,
                                    WaveformProcessor *wp) {
	_processors.insert(ProcessorMap::value_type(networkCode + "." + stationCode + "." + locationCode + "." + channelCode, wp));

	// Because we are dealing with a multimap we need to check if the pointer
	// is already registered for this station. Otherwise the remove method will
	// keep the additional instance because it stops after the first hit.
	std::string staID = networkCode + "." + stationCode;
	std::pair<StationProcessors::iterator, StationProcessors::iterator> itq =
		_stationProcessors.equal_range(staID);
	bool foundWP = false;
	for ( StationProcessors::iterator it = itq.first; it != itq.second; ++it ) {
		if ( it->second == wp ) {
			foundWP = true;
			break;
		}
	}

	if ( !foundWP )
		_stationProcessors.insert(StationProcessors::value_type(staID, wp));

	wp->setEnabled(isStationEnabled(networkCode, stationCode));

	SEISCOMP_DEBUG("Added processor on stream %s.%s.%s.%s    addr=0x%lx",
                       networkCode.c_str(), stationCode.c_str(),
	               locationCode.c_str(), channelCode.c_str(),
                       (long)wp);
	SEISCOMP_DEBUG("Current processor count: %lu/%lu, object count: %d",
		      (unsigned long)_processors.size(),
	              (unsigned long)_stationProcessors.size(),
		      Core::BaseObject::ObjectCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::registerProcessor(const DataModel::WaveformStreamID &wfid,
			            WaveformProcessor *proc) {
	registerProcessor(wfid.networkCode(),
		          wfid.stationCode(),
		          wfid.locationCode(),
		          wfid.channelCode(),
		          proc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::registerProcessor(const std::string& networkCode,
                                    const std::string& stationCode,
                                    const std::string& locationCode,
                                    const std::string& channelCode,
                                    TimeWindowProcessor *twp) {
	registerProcessor(networkCode, stationCode, locationCode, channelCode, (WaveformProcessor*)twp);

	twp->computeTimeWindow();

	RecordSequence* seq = _waveformBuffer.sequence(
		StreamBuffer::WaveformID(networkCode, stationCode, locationCode, channelCode));
	if ( !seq ) return;

	Core::Time startTime = twp->timeWindow().startTime() - twp->margin();
	Core::Time endTime = twp->timeWindow().endTime() +  twp->margin();

	if ( startTime < seq->timeWindow().startTime() ) {
		// TODO: Fetch historical data
		// Actually feed as much data as possible
		TimeWindowProcessorPtr twp_ptr = twp;

		for ( RecordSequence::iterator it = seq->begin(); it != seq->end(); ++it ) {
			if ( (*it)->startTime() > endTime )
				break;
			twp->feed((*it).get());
		}
	}
	else {
		// find the position in the recordsequence to fill the requested timewindow
		RecordSequence::reverse_iterator rit;
		for ( rit = seq->rbegin(); rit != seq->rend(); ++rit ) {
			if ( (*rit)->endTime() < startTime )
				break;
		}

		RecordSequence::iterator it;
		if ( rit == seq->rend() )
			it = seq->begin();
		else
			it = --rit.base();

		while ( it != seq->end() && (*it)->startTime() <= endTime ) {
			twp->feed((*it).get());
			++it;
		}
	}

	if ( twp->isFinished() ) {
		processorFinished(twp->lastRecord(), twp);
		removeProcessor(twp);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::registerProcessor(const DataModel::WaveformStreamID &wfid,
			            TimeWindowProcessor *proc) {
	registerProcessor(wfid.networkCode(),
		          wfid.stationCode(),
		          wfid.locationCode(),
		          wfid.channelCode(),
		          proc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::removeProcessors(const std::string& networkCode,
                                   const std::string& stationCode,
                                   const std::string& locationCode,
                                   const std::string& channelCode) {

	bool checkPendingQueue;
	std::pair<ProcessorMap::iterator, ProcessorMap::iterator> itq =
		_processors.equal_range(networkCode + "." +
		                        stationCode + "." +
		                        locationCode + "." +
		                        channelCode);

	checkPendingQueue = itq.first == itq.second;

	// Remove stations - processor association
	for ( ProcessorMap::iterator it = itq.first; it != itq.second; ++it ) {
		for ( StationProcessors::iterator its = _stationProcessors.begin();
		      its != _stationProcessors.end(); )
		{
			if ( its->second == it->second ) {
				SEISCOMP_DEBUG("Removed processor from station %s", its->first.c_str());
				_stationProcessors.erase(its++);
				break;
			}
		}
	}

	_processors.erase(itq.first, itq.second);

	if ( !checkPendingQueue ) return;

	// Remove from pending queue (if exists)
	for ( WaveformProcessorQueue::iterator it = _waveformProcessorQueue.begin();
	      it != _waveformProcessorQueue.end(); ) {
		if ( it->first.networkCode() != networkCode ) { ++it; continue; }
		if ( it->first.stationCode() != stationCode ) { ++it; continue; }
		if ( it->first.locationCode() != locationCode ) { ++it; continue; }
		if ( it->first.channelCode() != channelCode ) { ++it; continue; }
		it = _waveformProcessorQueue.erase(it);
	}

	for ( TimeWindowProcessorQueue::iterator it = _timeWindowProcessorQueue.begin();
	      it != _timeWindowProcessorQueue.end(); ) {
		if ( it->first.networkCode() != networkCode ) { ++it; continue; }
		if ( it->first.stationCode() != stationCode ) { ++it; continue; }
		if ( it->first.locationCode() != locationCode ) { ++it; continue; }
		if ( it->first.channelCode() != channelCode ) { ++it; continue; }
		it = _timeWindowProcessorQueue.erase(it);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::removeProcessors(const DataModel::WaveformStreamID &wfid) {
	removeProcessors(wfid.networkCode(),
		         wfid.stationCode(),
		         wfid.locationCode(),
		         wfid.channelCode());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::removeProcessor(Processing::WaveformProcessor *wp) {
	if ( _registrationBlocked ) {
		_waveformProcessorRemovalQueue.push_back(wp);
		return;
	}

	for ( ProcessorMap::iterator it = _processors.begin();
	      it != _processors.end(); )
	{
		if ( it->second.get() == wp ) {
			SEISCOMP_DEBUG("Removed processor from stream %s    addr=0x%lx",
				       it->first.c_str(), (long)wp);
			_processors.erase(it++);
		}
		else
			++it;
	}

	for ( StationProcessors::iterator it = _stationProcessors.begin();
	      it != _stationProcessors.end(); ++it )
	{
		if ( it->second.get() == wp ) {
			SEISCOMP_DEBUG("Removed processor from station %s", it->first.c_str());
			_stationProcessors.erase(it);
			break;
		}
	}

	// Remove from pending queue (if exists)
	for ( WaveformProcessorQueue::iterator it = _waveformProcessorQueue.begin();
	      it != _waveformProcessorQueue.end(); ) {
		if ( it->second.get() == wp )
			it = _waveformProcessorQueue.erase(it);
		else
			++it;
	}

	for ( TimeWindowProcessorQueue::iterator it = _timeWindowProcessorQueue.begin();
	      it != _timeWindowProcessorQueue.end(); ) {
		if ( it->second.get() == wp )
			it = _timeWindowProcessorQueue.erase(it);
		else
			++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Application::processorCount() const {
	return _processors.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addObject(const std::string& parentID, DataModel::Object* o) {
	Client::StreamApplication::addObject(parentID, o);

	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		if ( configModule() && parentID == configModule()->publicID() )
			enableStation(cs->networkCode() + "." + cs->stationCode(), cs->enabled());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::removeObject(const std::string& parentID, DataModel::Object* o) {
	Client::StreamApplication::removeObject(parentID, o);

	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		if ( configModule() && parentID == configModule()->publicID() )
			enableStation(cs->networkCode() + "." + cs->stationCode(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::updateObject(const std::string& parentID, DataModel::Object* o) {
	Client::StreamApplication::updateObject(parentID, o);

	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		if ( configModule() && parentID == configModule()->publicID() )
			enableStation(cs->networkCode() + "." + cs->stationCode(), cs->enabled());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleRecord(Record *rec) {
	std::string streamID = rec->streamID();
	std::list<WaveformProcessor*> trashList;

	RecordPtr tmp(rec);

	if ( rec->data() == NULL ) return;

	if ( !_waveformBuffer.feed(rec) ) return;

	if ( _waveformBuffer.addedNewStream() )
		handleNewStream(rec);

	_registrationBlocked = true;

	std::pair<ProcessorMap::iterator, ProcessorMap::iterator> itq = _processors.equal_range(streamID);
	for ( ProcessorMap::iterator it = itq.first; it != itq.second; ++it ) {
		// The proc must not be already on the removal list
		if ( std::find(_waveformProcessorRemovalQueue.begin(),
		               _waveformProcessorRemovalQueue.end(),
		               it->second) != _waveformProcessorRemovalQueue.end() )
			continue;

		// Schedule the processor for deletion when finished
		if ( it->second->isFinished() )
			trashList.push_back(it->second.get());
		else {
			it->second->feed(rec);
			if ( it->second->isFinished() )
				trashList.push_back(it->second.get());
		}
	}

	// Delete finished processors
	for ( std::list<WaveformProcessor*>::iterator itt = trashList.begin();
	      itt != trashList.end(); ++itt ) {
		processorFinished(rec, *itt);
		removeProcessor(*itt);
	}

	trashList.clear();

	_registrationBlocked = false;

	// Remove outdated processors if not already on the trash list
	while ( !_waveformProcessorRemovalQueue.empty() ) {
		WaveformProcessorPtr wp = _waveformProcessorRemovalQueue.front();
		_waveformProcessorRemovalQueue.pop_front();
		removeProcessor(wp.get());
	}

	// Register pending processors
	while ( !_waveformProcessorQueue.empty() ) {
		WID wid = _waveformProcessorQueue.front().first;
		WaveformProcessorPtr wp = _waveformProcessorQueue.front().second;
		_waveformProcessorQueue.pop_front();

		registerProcessor(wid.networkCode(), wid.stationCode(),
		                  wid.locationCode(), wid.channelCode(), wp.get());
	}

	while ( !_timeWindowProcessorQueue.empty() ) {
		WID wid = _timeWindowProcessorQueue.front().first;
		TimeWindowProcessorPtr twp = _timeWindowProcessorQueue.front().second;
		_timeWindowProcessorQueue.pop_front();

		registerProcessor(wid.networkCode(), wid.stationCode(),
		                  wid.locationCode(), wid.channelCode(), twp.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	Client::StreamApplication::done();
	//_waveformBuffer.printStreams();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::enableStation(const std::string& code, bool enabled) {
	std::pair<StationProcessors::iterator, StationProcessors::iterator> itq = _stationProcessors.equal_range(code);
	for (StationProcessors::iterator it = itq.first; it != itq.second; ++it) {
		SEISCOMP_INFO("%s station %s", enabled?"Enabling":"Disabling", code.c_str());
		it->second->setEnabled(enabled);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::enableStream(const std::string& code, bool enabled) {
	std::pair<StationProcessors::iterator, StationProcessors::iterator> itq = _processors.equal_range(code);
	for (StationProcessors::iterator it = itq.first; it != itq.second; ++it) {
		SEISCOMP_INFO("%s stream %s", enabled?"Enabling":"Disabling", code.c_str());
		it->second->setEnabled(enabled);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
