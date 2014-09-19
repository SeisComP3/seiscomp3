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

#define SEISCOMP_COMPONENT ScImEx

#include <memory>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/file.hpp>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/utils/files.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/datamodel/exchange/trunk.h>


#include "imex.h"
#include "imexmessage.h"
#include "imeximpl.h"


namespace Seiscomp {
namespace Applications {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImEx::ImEx(int argc, char* argv[])
 : Client::Application(argc, argv),
   _mode(UNDEFINED),
   _cleanUpInterval(60*60,0) {

	setRecordStreamEnabled(false);
	setDatabaseEnabled(false, false);
	setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
	setMessagingUsername(Util::basename(argv[0]));

	_lastCleanUp = Core::Time::GMT();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImEx::~ImEx() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ImEx::PickList& ImEx::pickList() const
{
	return _pickList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ImEx::AmplitudeList& ImEx::amplitudeList() const
{
	return _amplitudeList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ImEx::OriginList& ImEx::originList() const
{
	return _originList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ImEx::Mode& ImEx::mode() const
{
	return _mode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImEx::init()
{
	if ( !Application::init() )
		return false;

	// get clean up interval
	try {
		_cleanUpInterval = configGetDouble("cleanupinterval");
	}
	catch (Config::Exception& e) {
		SEISCOMP_INFO("%s", e.what());
		SEISCOMP_INFO("Using interval of %ld sec", _cleanUpInterval.seconds());
	}

	try {
		std::string mode = configGetString("mode");
		if (Core::compareNoCase(mode, "EXPORT") == 0)
			_mode = EXPORT;
		else if (Core::compareNoCase(mode, "IMPORT") == 0)
			_mode = IMPORT;
		else
		{
			SEISCOMP_ERROR("Wrong mode specified in configuration file: %s - Choose IMPORT or EXPORT", mode.c_str());
			return false;
		}
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_ERROR("%s", e.what());
		SEISCOMP_ERROR("No mode specified in configuration file (Choose IMPORT or EXPORT)");
		return false;
	}

	std::vector<std::string> groupSubscriptions;
	try {
		groupSubscriptions = configGetStrings("subscriptions");

	} catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	if ( groupSubscriptions.empty() ) {
		if ( _mode == IMPORT ) {
			groupSubscriptions.push_back(Communication::Protocol::IMPORT_GROUP);
		}
		else if ( _mode == EXPORT ) {
			groupSubscriptions.push_back("AMPLITUDE");
			groupSubscriptions.push_back("PICK");
			groupSubscriptions.push_back("LOCATION");
			groupSubscriptions.push_back("MAGNITUDE");
			groupSubscriptions.push_back("EVENT");
		}
	}

	std::vector<std::string>::iterator groupIt = groupSubscriptions.begin();
	for ( ; groupIt != groupSubscriptions.end(); groupIt++ ) {
		connection()->subscribe(groupIt->c_str());
	}

	// Configure implementations here
	if ( _mode == IMPORT ) {
		try {
			_importMessageConversion = configGetString("conversion");
		}
		catch ( Config::Exception& e ) {
			SEISCOMP_DEBUG("%s", e.what());
		}

		std::string sinkName;
		try {
			sinkName = configGetString("importHosts");
		}
		catch (Config::Exception& e) {
			SEISCOMP_DEBUG("%s", e.what());
			return false;
		}

		boost::shared_ptr<ImExImpl> impl = boost::shared_ptr<ImExImpl>(ImExImpl::Create(this, sinkName));
		if ( !impl )
			return false;
		_imexImpls.push_back(impl);
	}
	else if ( _mode == EXPORT ) {
		std::vector<std::string> sinkNamesList;
		try {
			sinkNamesList = configGetStrings("exportHosts");
		}
		catch ( Config::Exception& e ) {
			SEISCOMP_DEBUG("%s", e.what());
			return false;
		}

		for ( size_t i = 0; i < sinkNamesList.size(); ++i ) {
			boost::shared_ptr<ImExImpl> impl = boost::shared_ptr<ImExImpl>(ImExImpl::Create(this, sinkNamesList[i]));
			if ( impl )
				_imexImpls.push_back(impl);
		}
	}

	if ( _imexImpls.size() == 0 ) {
		SEISCOMP_ERROR("No sink clients have been configured");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::handleNetworkMessage(const Communication::NetworkMessage* msg) {
	_lastNetworkMessage = msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::handleMessage(Core::Message* message) {
	if ( !_lastNetworkMessage ) {
		SEISCOMP_ERROR("lastNetworkMessage is NULL");
		return;
	}

	if ( !_importMessageConversion.empty() ) {
		if (  _lastNetworkMessage->type() > 0 ) {
			Core::MessagePtr convertedMessage = convertMessage(message);
			if ( convertedMessage )	dispatchMessage(convertedMessage.get());
		}
	}
	else {
		dispatchMessage(message);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::done() {
	for ( size_t i = 0; i <_imexImpls.size(); ++i )
		_imexImpls[i]->stop();

	for ( size_t i = 0; i <_imexImpls.size(); ++i )
		_imexImpls[i]->wait();

	Client::Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImEx::validateParameters() {
	if ( commandline().hasOption("print-default-routingtable") ) {
		ImExImpl::RoutingTable routingTable = ImExImpl::CreateDefaultRoutingTable();
		ImExImpl::RoutingTable::iterator it = routingTable.begin();

		for ( ; it != routingTable.end(); it++ ) {
			if ( it != routingTable.begin() )
				std::cout << ", ";
			std::cout << it->first << ":" << it->second;
		}
		std::cout << std::endl;

		return false;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::createCommandLineDescription() {
	Application::createCommandLineDescription();
	Client::CommandLine& cl = commandline();
	std::string clGroupName = "imex";
	cl.addGroup(clGroupName.c_str());
	cl.addOption(clGroupName.c_str(), "print-default-routingtable", "Print the default object routingtable");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::updateData(DataModel::NotifierMessage* notifierMessage)
{
	SEISCOMP_DEBUG("Updating data for %s", DataModel::NotifierMessage::ClassName());
	// fill data lists here, but update the data structures first
	DataModel::NotifierMessage::iterator notifierIt = notifierMessage->begin();

	while ( notifierIt != notifierMessage->end() ) {
		(*notifierIt)->apply();
		++notifierIt;
	}

	// Handle messages
	notifierIt = notifierMessage->begin();
	for ( ; notifierIt != notifierMessage->end(); ++notifierIt ) {
		Core::BaseObject* object = (*notifierIt)->object();
		if ( !object ) {
			SEISCOMP_WARNING("Ignoring empty notifier for %s",
			                 (*notifierIt)->parentID().data());
			continue;
		}

		std::string className = object->className();
		if ( className == DataModel::Pick::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
			DataModel::Pick* pick = DataModel::Pick::Cast(object);
			if ( pick )
				_pickList.push_back(pick);
		}
		else if ( className == DataModel::Amplitude::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
			DataModel::Amplitude* amplitude = DataModel::Amplitude::Cast(object);
			if ( amplitude )
				_amplitudeList.push_back(amplitude);
		}
		else if ( className == DataModel::Origin::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
			DataModel::Origin* origin = DataModel::Origin::Cast(object);
			if ( origin )
				_originList.push_back(origin);
		}
		else if ( className == DataModel::Arrival::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
		}
		else if ( className == DataModel::StationMagnitude::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
		}
		else if ( className == DataModel::StationMagnitudeContribution::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
		}
		else if ( className == DataModel::Magnitude::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
		}
		else if ( className == DataModel::Event::ClassName() ) {
			SEISCOMP_DEBUG("Handling object of type: %s", className.c_str());
			DataModel::Event* event = DataModel::Event::Cast(object);
			if ( event )
				updateEventData(event);
		}
		else {
			SEISCOMP_DEBUG("Received unhandled object: %s with notifier type: %s", className.c_str(), (*notifierIt)->operation().toString());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::updateData(Core::DataMessage* dataMessage)
{
	SEISCOMP_DEBUG("Updating data for incoming DataModel::DataMessage");
	Core::DataMessage::iterator it = dataMessage->begin();
	while ( it != dataMessage->end() ) {
		DataModel::Event* event = DataModel::Event::Cast(it->get());
		if ( event )
			updateEventData(event);
		++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::updateEventData(DataModel::Event* event)
{
	SEISCOMP_DEBUG("Updating event data for %s with id: %s",
			event->className(), event->publicID().c_str());
	EventList::iterator eventIt = _eventList.begin();
	bool found = false;
	for ( ; eventIt != _eventList.end(); ++eventIt ) {
		if ( (*eventIt)->publicID() == event->publicID() ) {
			*eventIt = event;
			found = true;
			break;
		}
	}
	if ( !found )
		_eventList.push_back(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::MessagePtr ImEx::convertMessage(Core::Message* message) {
	// The core::message might already contain properly constructed and registered objects of various
	// types. Now, if we are going to create our own message via an importer object, the
	// contained datamodel objects could not be registered because their ids are corresponding
	// with the ids of the core::message created objects.
	IMEXMessage* imexMsg = IMEXMessage::Cast(message);
	if ( imexMsg ) imexMsg->notifierMessage().clear();

	SEISCOMP_DEBUG("= Converting message = ");
	boost::iostreams::filtering_istreambuf filtered_buf;
	boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(
			_lastNetworkMessage->data().c_str(), _lastNetworkMessage->data().size()
	);
	filtered_buf.push(boost::iostreams::zlib_decompressor());
	filtered_buf.push(buf);

	std::auto_ptr<IO::Importer> importer = std::auto_ptr<IO::Importer>(IO::ImporterFactory::Create(_importMessageConversion.c_str()));
	if ( !importer.get() ) {
		SEISCOMP_ERROR("Could not create importer for type %s", _importMessageConversion.c_str());
		return NULL;
	}

	Core::BaseObjectPtr obj = importer->read(&filtered_buf);
	if ( !obj ) {
		SEISCOMP_ERROR("Could not convert message");
		return NULL;
	}

	if ( !importer->withoutErrors() ) {
		SEISCOMP_ERROR("Errors occurred while converting message");
		return NULL;
	}

	return Core::Message::Cast(obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::dispatchMessage(Core::Message* msg) {
	if ( Core::Time::GMT() - _lastCleanUp > _cleanUpInterval ) {
		SEISCOMP_DEBUG("Cleaning up");
		for ( size_t i = 0; i < _imexImpls.size(); ++i )
			_imexImpls[i]->cleanUp();

		cleanUp(_pickList);
		cleanUp(_amplitudeList);
		cleanUp(_eventList);
		cleanUp(_originList);

		_lastCleanUp = Core::Time::GMT();
	}

	if ( DataModel::NotifierMessagePtr notifierMessage = DataModel::NotifierMessage::Cast(msg) ) {
		if ( notifierMessage )
			updateData(notifierMessage.get());
	}
	else if ( IMEXMessagePtr imexMessage = IMEXMessage::Cast(msg) ) {
		if ( imexMessage )
			updateData(&imexMessage->notifierMessage());
	}
	else if ( Core::DataMessage* dataMessage = Core::DataMessage::Cast(msg) ) {
		if ( dataMessage ) {
			updateData(dataMessage);
		}
	}

	// Dispatch message to implementations
	for ( size_t i = 0; i < _imexImpls.size(); ++i ) {
		if ( !_imexImpls[i]->handleMessage(msg) )
			SEISCOMP_ERROR("Plugin %s could not process message", _imexImpls[i]->sinkName().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::cleanUp(EventList& container)
{
	cleanUpSpecial(container);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImEx::cleanUp(AmplitudeList& container)
{
	cleanUpSpecial(container);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::TimeSpan& ImEx::cleanUpInterval() const
{
	return _cleanUpInterval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ImEx::cleanUp(T& container)
{
	typename T::iterator it = container.begin();
	while ( it != container.end() ) {
		if ( Core::Time::GMT() - (*it)->time() > _cleanUpInterval ) {
			SEISCOMP_DEBUG("One element %s with id: %s removed",
					(*it)->className(), (*it)->publicID().c_str());
			it = container.erase(it);
		}
		else {
			++it;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ImEx::cleanUpSpecial(T& container)
{
	Core::Time now = Core::Time::GMT();
	typename T::iterator it = container.begin();
	while ( it != container.end() ) {
		try {
			if ( now - (*it)->creationInfo().creationTime() > _cleanUpInterval ) {
				SEISCOMP_DEBUG("GenericList: One element %s with id: %s removed",
				               (*it)->className(), (*it)->publicID().c_str());
				it = container.erase(it);
			}
			else {
				++it;
			}
		}
		catch ( Core::ValueException& e ) {
			SEISCOMP_ERROR("Creation time of object of type %s with id: %s not set. Removing object.",
			               (*it)->className(), (*it)->publicID().c_str());
			it = container.erase(it);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Applictions
} // namespace Seiscomp
