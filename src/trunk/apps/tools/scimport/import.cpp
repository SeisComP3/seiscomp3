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



#define SEISCOMP_COMPONENT ScImport
#include "import.h"

#include <vector>

#include <boost/bind.hpp>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/utils/files.h>
#include <seiscomp3/communication/networkinterface.h>

#include "filter.h"

namespace Seiscomp {
namespace Applications {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Import::Import(int argc, char* argv[]) :
	Client::Application(argc, argv),
		_sinkMessageThread(NULL),
		_filter(true),
		_routeUnknownGroup(false),
		_mode(RELAY),
		_useSpecifiedGroups(true),
		_test(false)
{
	setRecordStreamEnabled(false);
	setDatabaseEnabled(false, false);
	setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
	setMessagingUsername(Util::basename(argv[0]));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Import::~Import() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Import::init() {
	if (!Application::init())
		return false;

	if ( !Filter::Init(*this) ) return false;

	if (commandline().hasOption("test"))
		_test = true;

	if (commandline().hasOption("no-filter"))
		_filter = false;

	if (commandline().hasOption("import"))
		_mode = IMPORT;

	if (commandline().hasOption("routeunknowngroup"))
		_routeUnknownGroup = true;

	if (commandline().hasOption("ignore-groups"))
		_useSpecifiedGroups = false;

	try {
		configGetString("routingtable");
		_mode = IMPORT;
	}
	catch ( ... ) {}

	std::string sinkName = "localhost";
	if (commandline().hasOption("sink")) {
		sinkName = commandline().option<std::string>("sink");
	}
	else {
		try {
		sinkName = configGetString("sink");
		}
		catch (Config::Exception& ) {}
	}

	if (connectToSink(sinkName) != Core::Status::SEISCOMP_SUCCESS)
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Import::done() {
	if ( _sink ) _sink->disconnect();

	Client::Application::done();

	if ( _sinkMessageThread ) {
		SEISCOMP_DEBUG("Waiting for sink message thread");
		_sinkMessageThread->join();
		delete _sinkMessageThread;
		_sinkMessageThread = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Import::createCommandLineDescription() {
	Application::createCommandLineDescription();
	Client::CommandLine& cl = commandline();
	std::string clGroupName = "scimport";
	cl.addGroup(clGroupName.c_str());
	cl.addOption(clGroupName.c_str(), "sink,o", "Sink master", static_cast<std::string*>(NULL));
	cl.addOption(clGroupName.c_str(), "import,i", "Switch to import mode (default is relay)\n"
		             "import mode: You have your own routing table specified\n"
		             "relay mode: The routing table will be calculated automatically");
	cl.addOption(clGroupName.c_str(), "no-filter", "Don't filter messages");
	cl.addOption(clGroupName.c_str(), "routeunknowngroup", "Route unknown groups to the default group IMPORT_GROUP");
	cl.addOption(clGroupName.c_str(), "ignore-groups", "Ignore user specified groups");
	cl.addOption(clGroupName.c_str(), "test", "Do not send any messages");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Import::connectToSink(const std::string& sink) {
	std::string protocol = "spread";
	std::string server = sink;
	size_t pos = sink.find("://");

	if ( pos != std::string::npos ) {
		protocol = sink.substr(0,pos);
		server = sink.substr(pos+3);
	}

	Communication::NetworkInterfacePtr ni = Communication::NetworkInterface::Create(protocol.c_str());
	if (ni == NULL)
	{
		SEISCOMP_ERROR("Networkinterface \"%s\" not found", protocol.c_str());
		return Core::Status::SEISCOMP_FAILURE;
	}

	_sink = new Communication::SystemConnection(ni.get());

	// Connect to the sink master and use a default name
	int ret;
	bool first = true;
	while ( (ret = _sink->connect(server, "", Communication::Protocol::IMPORT_GROUP)) !=
			Core::Status::SEISCOMP_SUCCESS && !_exitRequested ) {
		if ( first ) {
			SEISCOMP_WARNING("Could not connect to the sink master %s : %s, trying again every 2s",
			                 sink.c_str(), Core::Status::StatusToStr(ret));
			first = false;
		}

		Core::sleep(2);
	}

	if (ret != Core::Status::SEISCOMP_SUCCESS)
		return ret;

	// Get rid of data messages and read commands that are may send.
	SEISCOMP_INFO("Successfully connected to sink master: %s", sink.c_str());

	// Build routing table
	if (_mode == RELAY) {
		buildRelayRoutingtable(_routeUnknownGroup);
	}
	else if (_mode == IMPORT) {
		if (!buildImportRoutingtable())
		{
			SEISCOMP_ERROR("Could not built routing table for IMPORT mode.\nThere are no routing entries in specified in the configuration file");
			return 0;
		}
	}
	else {
		SEISCOMP_ERROR("Unknown import mode: %i", _mode);
		return Core::Status::SEISCOMP_FAILURE;
	}

	_sinkMessageThread = new boost::thread(boost::bind(&Import::readSinkMessages, this));

	// Print routing table
	for (std::map<std::string,
		 std::string>::iterator it = _routingTable.begin();
		 it != _routingTable.end(); ++it)
		SEISCOMP_INFO("%s@%s -> %s@%s", it->first.c_str(), connection()->masterAddress().c_str(), it->second.c_str(), sink.c_str());

	// Subscribe to source message groups
	for (std::map<std::string, std::string>::iterator it = _routingTable.begin();
	        it != _routingTable.end(); ++it) {
		if (connection()->subscribe(it->first) != Core::Status::SEISCOMP_SUCCESS)
			SEISCOMP_INFO("Could subscribe to group: %s", it->first.c_str());
	}

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Import::handleNetworkMessage(const Communication::NetworkMessage* networkMessage)
{
	if (networkMessage->type() > 0 )
	{
		RoutingTable::iterator it = _routingTable.find(networkMessage->destination());
		if ( it == _routingTable.end() ) return;

		Communication::NetworkMessagePtr newNetworkMessagePtr;
		DataModel::PublicObject::SetRegistrationEnabled(false);
		Seiscomp::Core::MessagePtr messagePtr = networkMessage->decode();
		DataModel::PublicObject::SetRegistrationEnabled(true);

		if (_filter)
		{
			if (Core::DataMessage* dataMessage = Core::DataMessage::Cast(messagePtr))
			{
				Core::DataMessage::iterator it = dataMessage->begin();
				while (it != dataMessage->end())
				{
					if ( filterObject((*it).get()) )
						it = dataMessage->detach(it);
					else
						++it;
				}
				newNetworkMessagePtr = Communication::NetworkMessage::Encode(
						messagePtr.get(), networkMessage->contentType(),
						_sink->schemaVersion().packed);
			}
			else if (DataModel::NotifierMessage* notifierMessage = DataModel::NotifierMessage::Cast(messagePtr))
			{
				// Process Notifier Message
				DataModel::NotifierMessage::iterator it = notifierMessage->begin();
				while ( it != notifierMessage->end() ) {
					if (filterObject((*it)->object()))
						it = notifierMessage->detach(it);
					else
						++it;
				}
				newNetworkMessagePtr = Communication::NetworkMessage::Encode(
						messagePtr.get(), networkMessage->contentType(),
						_sink->schemaVersion().packed);
			}
			else {
				// Unknown message
				return;
			}

			if (messagePtr->empty()) return;
		}
		else {
			newNetworkMessagePtr = Communication::NetworkMessage::Encode(
					messagePtr.get(), networkMessage->contentType(),
					_sink->schemaVersion().packed);
		}

		/*
		SEISCOMP_INFO(
			"Message (type: %d) from source %s with destination %s relayed to group %s on sink %s",
			newNetworkMessagePtr->type(), networkMessage->clientName().c_str(), networkMessage->destination().c_str(),
			it->second.c_str(), _sink->masterAddress().c_str());
		*/

		if ( !_test ) {
			bool first = true;
			int res;
			while ( (res = _sink->send(it->second, newNetworkMessagePtr.get())) !=
					Core::Status::SEISCOMP_SUCCESS && !_exitRequested ) {
				switch ( res ) {
					case Core::Status::SEISCOMP_INVALID_GROUP_ERROR:
						SEISCOMP_WARNING("Sink group %s does not exist, message ignored",
						                 it->second.c_str());
						return;

					case Core::Status::SEISCOMP_MESSAGE_SIZE_ERROR:
						SEISCOMP_WARNING("Sink says: message is too big, message ignored");
						return;
					default:
						break;
				}

				if ( first ) {
					SEISCOMP_WARNING("Sending message to %s failed, waiting for reconnect",
					                 _sink->masterAddress().c_str());
					first = false;
				}

				Core::sleep(2);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Import::filterObject(Core::BaseObject* obj)
{
	Filter* filter = Filter::GetFilter(obj->className());
	if ( filter ) {
		if ( !filter->filter(obj) ) return true;
	}
	else {
		SEISCOMP_DEBUG("Filter for class: %s not available", obj->className());
		return true;
	}
	return false;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Import::buildImportRoutingtable()
{
	// Build routing table
	SEISCOMP_INFO("Calculating routing table ...");
	try	{
		std::vector<std::string> tmpRoutingTable = configGetStrings("routingtable");

		for ( std::vector<std::string>::iterator it = tmpRoutingTable.begin();
		        it != tmpRoutingTable.end(); ++it )
		{
			std::vector<std::string> tokens;
			Core::split(tokens, it->c_str(), ":");
			if ( tokens.size() != 2 )
				SEISCOMP_INFO("Malformed routing table entry: %s", it->c_str());
			else
				_routingTable[tokens[0]] = tokens[1];
		}
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_ERROR("%s", e.what());
		return false;
	}

	std::vector<std::string> sourceGroups;
	if ( _useSpecifiedGroups ) {
		try {
			sourceGroups = configGetStrings("msggroups");
		}
		catch ( Config::Exception& ) {
			exit(0);
		}
	}
	else
	{
		Communication::Connection *con = connection();
		if (con)
			sourceGroups = con->groups();
	}

	std::map<std::string, std::string>::iterator it, tmp;
	for ( it = _routingTable.begin(); it != _routingTable.end(); ) {
		if ( std::find(sourceGroups.begin(), sourceGroups.end(), it->first) == sourceGroups.end() ) {
			SEISCOMP_ERROR("Group %s is not a member of available source groups. Deleting routing entry", it->first.c_str());
			tmp = it;
			++it;
			_routingTable.erase(tmp);
		}
		else
			++it;
	}

	/*
	std::vector<std::string> sinkGroups = _sink->groups();
	for ( it = _routingTable.begin(); it != _routingTable.end(); ) {
		if ( std::find(sinkGroups.begin(), sinkGroups.end(), it->second) == sinkGroups.end() ) {
			SEISCOMP_ERROR("Group %s is not a member of available sink groups. Deleting routing entry", it->second.c_str());
			tmp = it;
			++it;
			_routingTable.erase(tmp);
		}
		else
			++it;
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Import::buildRelayRoutingtable(bool routeUnknownGroup)
{
	SEISCOMP_INFO("Calculating default routing table ...");
	std::vector<std::string> sourceGroups;
	if ( _useSpecifiedGroups ) {
		try {
			sourceGroups = configGetStrings("msggroups");
		}
		catch ( Config::Exception& ) {
			exit(0);
		}
	}
	else
	{
		Communication::Connection* con = connection();
		if (con)
			sourceGroups = connection()->groups();
	}

	std::vector<std::string> sinkGroups = _sink->groups();

	for( std::vector<std::string>::iterator it = sourceGroups.begin();
	        it != sourceGroups.end(); ++it )
	{
		std::vector<std::string>::iterator found =
		    std::find(sinkGroups.begin(), sinkGroups.end(), *it);
		if ( found != sinkGroups.end() )
			_routingTable[*it] = *found;
		else if( routeUnknownGroup )
			_routingTable[*it] = Communication::Protocol::IMPORT_GROUP;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Import::readSinkMessages()
{
	while ( !_exitRequested ) {
		int error;
		Communication::NetworkMessagePtr networkMsgPtr;
		networkMsgPtr = _sink->receive(true, &error);
		if ( error != Core::Status::SEISCOMP_SUCCESS ) {
			bool first = true;

			while ( !_exitRequested ) {
				if ( first )
					SEISCOMP_WARNING("Connection lost to sink, trying to reconnect");

				_sink->reconnect();
				if ( _sink->isConnected() ) {
					SEISCOMP_INFO("Reconnected successfully to sink");
					break;
				}
				else {
					if ( first ) {
						first = false;
						SEISCOMP_INFO("Reconnecting to sink failed, trying again every 2 seconds");
					}
					Core::sleep(2);
				}
			}
		}
	}

	SEISCOMP_INFO("Leaving sink message thread");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Applictions
} // namespace Seiscomp
