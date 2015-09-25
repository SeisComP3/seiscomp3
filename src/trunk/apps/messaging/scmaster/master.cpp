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

// logging
#define SEISCOMP_COMPONENT MASTER_COM_MODULE
#include <seiscomp3/logging/log.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <functional>

#include <boost/bind.hpp>

#include <seiscomp3/client/pluginregistry.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/version.h>
#include <seiscomp3/utils/files.h>

#include "master.h"

namespace Seiscomp {
namespace Communication {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Master::Master(const std::string& name)
 : _seqNum(0),
   _maxSeqNum(Protocol::MAX_SEQ_NUM),
   _name(Util::basename(name)) {
	_schemaVersion = Core::Version(DataModel::Version::Major, DataModel::Version::Minor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Master* Master::Create(const std::string& name) {
	Master* m = new Master(name);
	if ( !m ) return NULL;
	return m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Master::~Master() {
	if ( _archive.get() ) {
		std::vector<NetworkMessage*>::iterator it = _archive->begin();
		for ( ; it != _archive->end(); ++it ) {
			if ( *it != NULL ) {
				delete *it;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::setConfigFile(const std::string& fileName) {
	_configFile.assign(fileName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::start(const std::string& serverAddress) {
	std::vector<std::string> tokens;
	Core::split(tokens, serverAddress.c_str(), ":");
	if ( tokens.size() > 2 || tokens.size() == 0 ) {
		SEISCOMP_ERROR("Invalid host address: %s", serverAddress.c_str());
		exit(Core::Status::SEISCOMP_FAILURE);
	}

	if ( tokens.size() > 1 )
		_serverAddress = tokens[1];
	else
		_serverAddress = "4803";

	_serverAddress += "@";
	_serverAddress += tokens[0];

	// connect to server
	if ( connect(_serverAddress) == Core::Status::SEISCOMP_SUCCESS ) {
		setRunning(true);
		_uptime = std::auto_ptr<Util::StopWatch>(new Util::StopWatch);

		_connectionInfo = ConnectionInfo::Instance();
		if ( _connectionInfo ) {
			_connectionInfo->registerConnection(_networkInterface.get(), &_sendMutex);
			_connectionInfo->start();
		}

		// Start message handling
		run();
	}
	else {
		SEISCOMP_ERROR("Master could not connect to messaging server!");
		exit(Core::Status::SEISCOMP_FAILURE);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::stop() {
	setRunning(false);
//	if ( _pluginRegistry.get() )
//		_pluginRegistry->setDisabled(true);

	// Stop all plugins
	for ( Plugins::iterator it = _plugins.begin(); it != _plugins.end(); ++it ) {
		(*it)->close();
		(*it)->setOperational(false);
	}

	if ( _connectionInfo )
		_connectionInfo->unregisterConnection(_networkInterface.get());
	_messageQueue.close();

	return disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Master::init() {
	_networkInterface = NetworkInterface::Create("spread");
	if ( _networkInterface == NULL ) {
		SEISCOMP_ERROR("Could not create NetworkInteface - Exiting");
		exit(Core::Status::SEISCOMP_FAILURE);
	}

	_archive = std::auto_ptr<std::vector<NetworkMessage*> >
	           (new std::vector<NetworkMessage*>(Protocol::MASTER_ARCHIVE_SIZE));

	std::vector<NetworkMessage*>::iterator elem = _archive->begin();
	for ( ; elem != _archive->end(); ++elem )
		*elem = NULL;

	//To be consistend with the mod archive index the maximal sequence number
	//to be a mutiple of MASTER_ARCHIVE_SIZE.
	_maxSeqNum -= Protocol::MAX_SEQ_NUM % Protocol::MASTER_ARCHIVE_SIZE;

	// Initialize available message groups
	Config::Config conf;
	if ( _configFile.empty() ) {
		Environment::Instance()->initConfig(&conf, _name);
	}
	else {
		_configFile = Environment::Instance()->absolutePath(_configFile);
		if ( !Util::fileExists(_configFile) ) {
			SEISCOMP_ERROR("Could not find alternative configuration file %s, abort", _configFile.c_str());
			return false;
		}
		if ( !conf.readConfig(_configFile) ) {
			SEISCOMP_ERROR("Error found in alternative configuration file %s, abort", _configFile.c_str());
			return false;
		}
	}

	try {
		std::string version = conf.getString("schemaVersionOverride");
		Core::Version schemaVersion;
		if ( !schemaVersion.fromString(version) ) {
			SEISCOMP_ERROR("Wrong format in schemaVersionOverride: %s", version.data());
			return false;
		}

		if ( schemaVersion > _schemaVersion ) {
			SEISCOMP_ERROR("schemaVersionOverride %s is higher than version %s "
			               "compiled in", schemaVersion.toString().data(),
			               _schemaVersion.toString().data());
			return false;
		}

		_schemaVersion = schemaVersion;
	}
	catch ( ... ) {}
	SEISCOMP_INFO("Reporting schema version %s to clients", _schemaVersion.toString().data());

	std::vector<std::string> groups;
	try {
		groups = conf.getStrings("msgGroups");
	}
	catch ( ... )
	{
		SEISCOMP_INFO("No msgGroups entry found, using default values");
		groups.push_back("AMPLITUDE");
		groups.push_back("PICK");
		groups.push_back("LOCATION");
		groups.push_back("MAGNITUDE");
		groups.push_back("EVENT");
		groups.push_back("QC");
		groups.push_back("PUBLICATION");
		groups.push_back("GUI");
		groups.push_back("INVENTORY");
		groups.push_back("CONFIG");
		groups.push_back("LOGGING");
		groups.push_back("SERVICE_REQUEST");
		groups.push_back("SERVICE_PROVIDE");
	}

	for ( std::vector<std::string>::iterator it = groups.begin();
			it != groups.end(); ++it ) {
		std::cout << "group: " << static_cast<std::string>(*it) << std::endl;
		_msgGroups.insert(static_cast<std::string>(*it));
	}

	_isAdminConnected = false;
	try {
		_adminClientName = conf.getString("admin.adminname");
		SEISCOMP_INFO("Set admin name to %s", _adminClientName.c_str());
	}
	catch ( const Config::Exception &e ) {
		SEISCOMP_WARNING("Could not find option: %s", e.what());
	}

	if ( _adminClientName.empty() ) {
		_adminClientName = Protocol::DEFAULT_ADMIN_CLIENT_NAME;
	}

	try {
		_password = conf.getString("admin.password");
	}
	catch ( const Config::Exception &e ) {
		SEISCOMP_WARNING("Could not find option: %s", e.what());
		_password = "";
	}

	readPluginNames(conf, "core.plugins");
	readPluginNames(conf, "plugins");

	Client::PluginRegistry::Instance()->addPackagePath("scmaster");
	Client::PluginRegistry::Instance()->loadPlugins();
	if ( Client::PluginRegistry::Instance()->pluginCount() > 0 ) {
		std::cerr << "Plugins:" << std::endl;
		std::cerr << "--------" << std::endl;
		int idx = 1;
		for ( Client::PluginRegistry::iterator it = Client::PluginRegistry::Instance()->begin();
				it != Client::PluginRegistry::Instance()->end(); ++it ) {
			std::cerr << " [" << idx << "]" << std::endl;
			std::cerr << "  description: " << (*it)->description().description << std::endl;
			std::cerr << "       author: " << (*it)->description().author << std::endl;
			std::cerr << "      version: " << (*it)->description().version.major
			                        << "." << (*it)->description().version.minor
			                        << "." << (*it)->description().version.revision
			                        << std::endl;
			++idx;
		}
	}
	else
		SEISCOMP_INFO("No plugins loaded");


	std::auto_ptr<MasterPluginInterfaceFactory::ServiceNames> pluginNames =
		std::auto_ptr<MasterPluginInterfaceFactory::ServiceNames>(MasterPluginInterfaceFactory::Services());

	if ( pluginNames.get() ) {
		for ( MasterPluginInterfaceFactory::ServiceNames::iterator pluginIt = pluginNames->begin();
				pluginIt != pluginNames->end(); ++pluginIt ) {
			MasterPluginInterfacePtr ptr = MasterPluginInterfaceFactory::Create((*pluginIt).c_str());
			if ( ptr ) {
				if ( ptr->init(conf, "plugins.") ) {
					SEISCOMP_DEBUG("Plugin %s loaded", pluginIt->c_str());
					_plugins.push_back(ptr);
				}
				else {
					SEISCOMP_ERROR("Plugin %s not operational. Bailing out", pluginIt->c_str());
					return false;
				}
			}
			else
				SEISCOMP_ERROR("Plugin %s could not be loaded", pluginIt->c_str());
		}
	}

	_networkMessageQueue.resize(1000);
	_messageQueue.resize(10);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::readPluginNames(Config::Config& conf, const std::string& name) {
	try {
		std::vector<std::string> pluginNames = conf.getStrings(name);
		std::vector<std::string>::iterator it = pluginNames.begin();
		for ( ; it != pluginNames.end(); ++it )
			Client::PluginRegistry::Instance()->addPluginName(*it);
	}
	catch ( ... ) {
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::run() {
	boost::thread networkMessageThread(boost::bind(&Master::processNetworkMessages, this));
	networkMessageThread.yield();

	boost::thread listenThread(boost::bind(&Master::listen, this));
	listenThread.yield();

	while ( isRunning() ) {
		Slot slot;

		try {
			slot = _messageQueue.pop();
		}
		// The message queue has been closed => stop master
		catch ( Core::GeneralException &ex ) {
			SEISCOMP_INFO("Exception (messageQueue.pop): %s", ex.what());
			// Close the networkMessageQueue to end the plugin thread
			_networkMessageQueue.close();
			// leave run loop
			break;
		}

		++_messageStat.totalReceivedMessages;

		NetworkMessage *message = slot.msg;
		if ( message == NULL ) {
			SEISCOMP_WARNING("Received a NULL message?");
			continue;
		}

		// Network message
		if ( message->type() > 0 ) {
			// Has the message been read from the network?
			if ( slot.fromOutside ) {
				// Queue it for plugin handling
				if ( !_networkMessageQueue.push(message) ) {
					// Queue closed because of TERM signal => leave run loop
					delete message;
					break;
				}

				_messageStat.summedMessageQueueSize += _networkMessageQueue.size();
			}
			else {
				// Handle the specific message type
				switch (message->type()) {
					case Protocol::DATA_MSG: {
						sendAndArchive(message);
						break;
					}

					default:
						SEISCOMP_WARNING("Unknown data message: %d", message->type());
						SEISCOMP_DEBUG("Destination: %s ", message->destination().c_str());
						SEISCOMP_DEBUG("Sender: %s ", message->destination().c_str());
						delete message;
						break;
				}
			}
		}
		// Service message
		else if ( message->type() < 0 )
			processServiceMessage(static_cast<ServiceMessage*>(message));
		else
			SEISCOMP_WARNING("Received undefined message from: %s", message->privateSenderGroup().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::listen() {
	int error;

	while ( isRunning() ) {
		error = 0;
		NetworkMessage* message = _networkInterface->receive(&error);
		if ( message == NULL ) {
			if ( error != Core::Status::SEISCOMP_SUCCESS ) {
				SEISCOMP_ERROR("Received error code %d while reading message", error);
				if ( !_networkInterface->isConnected() ) {
					SEISCOMP_ERROR("Master is disconnected. Trying to reconnect ...");
					reconnect();
				}
			}
			continue;
		}

		// Process messages
		SEISCOMP_DEBUG(
				"Received Seiscomp message with type: %s (%d) from sender: %s",
				Protocol::MsgTypeToString(message->type()),
				message->type(),
				message->privateSenderGroup().c_str()
		);

		if ( !_messageQueue.push(Slot(message, true)) ) {
			delete message;
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::processNetworkMessages() {
	SEISCOMP_INFO("= Starting processNetworkMessages =");

	// Disable object registration
	DataModel::PublicObject::SetRegistrationEnabled(false);

	while ( isRunning() ) {
		Core::BaseObject *obj;
		try {
			obj = _networkMessageQueue.pop();
		}
		// Queue has been closed => end thread
		catch ( Core::GeneralException &ex ) {
			SEISCOMP_INFO("Exception (networkMessageQueue.pop): %s", ex.what());
			break;
		}

		NetworkMessage *message = NetworkMessage::Cast(obj);
		if ( message == NULL ) {
			SEISCOMP_WARNING("Received a NULL message (plugin thread)?");
			continue;
		}

		Core::MessagePtr msg(message->decode());

		for ( Plugins::iterator it = _plugins.begin(); it != _plugins.end(); ++it ) {
			if ( !(*it)->process(message, msg.get()) ) {
				SEISCOMP_ERROR("Stopping master due to plugin error");
				stop();
			}
		}

		if ( msg ) {
			// Handle sync request messages natively
			SyncRequestMessage *sync_req = SyncRequestMessage::Cast(msg);
			if ( sync_req ) {
				SEISCOMP_DEBUG("Received sync request from %s with ID %s",
				               message->clientName().c_str(), sync_req->ID());
				SyncResponseMessage sync_resp(sync_req->ID());
				NetworkMessagePtr sync_resp_net =
					NetworkMessage::Encode(&sync_resp, Protocol::CONTENT_BINARY);
				sync_resp_net->setDestination(message->privateSenderGroup());
				send(sync_resp_net.get());
				SEISCOMP_DEBUG("Sent sync response to group %s",
				               sync_resp_net->destination().c_str());

				// Do not relay a sync message
				delete message;
				message = NULL;
			}
		}

		if ( message ) sendAndArchive(message);
	}

	SEISCOMP_INFO("= Stopping processNetworkMessages =");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::processServiceMessage(ServiceMessage* sm) {
	switch ( sm->type() ) {
		case Protocol::CONNECT_GROUP_MSG:
			if ( sm->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_0 ||
			     sm->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_1 ) {
				std::string pm = sm->peerGroup();
				bool val = _msgGroups.find(pm.c_str()) != _msgGroups.end() ||
						   pm == Protocol::ADMIN_GROUP ||
						   pm == Protocol::LISTENER_GROUP ||
						   pm == Protocol::IMPORT_GROUP ||
						   pm == Protocol::STATUS_GROUP;

				if ( val ) {
					SEISCOMP_DEBUG("Connection request: %s to %s", sm->clientName().c_str(), pm.c_str());
					if ( pm == Protocol::ADMIN_GROUP && _adminClientName == sm->clientName() ) {
						if ( _password.empty() || _password == sm->password() ) {
							_isAdminConnected = true;
							_privateAdminGroup.assign(sm->privateSenderGroup());
						}
						else {
							SEISCOMP_INFO("Rejected %s because of wrong password", sm->clientName().c_str());
							sendMsg(sm->privateSenderGroup(), Protocol::ADMIN_REJECT_MSG);
							break;
						}
					}
					else if ( pm == Protocol::ADMIN_GROUP && _adminClientName != sm->clientName() ) {
						SEISCOMP_INFO("Rejected %s because of wrong clientname, expected %s", sm->clientName().c_str(), _adminClientName.c_str());
						sendMsg(sm->privateSenderGroup(), Protocol::ADMIN_REJECT_MSG);
						break;
					}
					else if ( pm != Protocol::ADMIN_GROUP && _adminClientName == sm->clientName() ) {
						SEISCOMP_INFO("Rejected %s because of wrong group, expected ADMIN_GROUP", sm->clientName().c_str());
						sendMsg(sm->privateSenderGroup(), Protocol::ADMIN_REJECT_MSG);
						break;
					}

					std::string groups = msgGroups();
					NetworkMessagePtr tmpMsg(createMsg(Protocol::CONNECT_GROUP_OK_MSG));
					tmpMsg->setDestination(sm->privateSenderGroup());
					static_cast<ServiceMessage*>(tmpMsg.get())->setProtocolVersion(sm->protocolVersion());
					if ( sm->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_0 )
						tmpMsg->setData(groups.c_str());
					else {
						tmpMsg->data() = Protocol::HEADER_SERVER_VERSION_TAG;
						tmpMsg->data() += ": ";
						tmpMsg->data() += Core::CurrentVersion.toString() + "\n";
						tmpMsg->data() += Protocol::HEADER_GROUP_TAG;
						tmpMsg->data() += ": ";
						tmpMsg->data() += groups + "\n";
						tmpMsg->data() += Protocol::HEADER_SCHEMA_VERSION_TAG;
						tmpMsg->data() += ": ";
						tmpMsg->data() += Core::toString(_schemaVersion.majorTag());
						tmpMsg->data() += ".";
						tmpMsg->data() += Core::toString(_schemaVersion.minorTag());
					}

					send(tmpMsg.get());

					// Send service response messages from the registered plugins
					// to the newly connected client.
					for ( Plugins::const_iterator it = _plugins.begin(); it != _plugins.end(); ++it ) {
						std::auto_ptr<NetworkMessage> serviceProvideMsg((*it)->service());
						if ( serviceProvideMsg.get() ) {
							serviceProvideMsg->setDestination(sm->privateSenderGroup());
							send(serviceProvideMsg.get());
						}
					}

					const ClientInfo* ci = _clientDB.addClientToDB(*sm);
					SEISCOMP_INFO(
							"New client connected: %s %s %s %s",
							ci->clientStatus()->privateGroup().c_str(),
							ci->clientStatus()->programName().c_str(),
							ci->clientStatus()->hostname().c_str(),
							ci->clientStatus()->ips().c_str()
					);
				}
				else
					sendMsg(sm->privateSenderGroup(), Protocol::CONNECT_GROUP_REJECT_MSG);
			}
			else
				sendMsg(sm->privateSenderGroup(), Protocol::INVALID_PROTOCOL_MSG);
			break;

		case Protocol::CONNECT_GROUP_REJECT_MSG:
			break;

		case Protocol::CONNECT_GROUP_OK_MSG:
			break;

		case Protocol::CLIENT_DISCONNECTED_MSG:
			if ( _adminClientName == sm->clientName() ) {
				_isAdminConnected = false;
				_privateAdminGroup.clear();
			}
			_clientDB.removeClientFromDB(sm->privateSenderGroup());
			SEISCOMP_DEBUG("Received %s. Removing client %s from database",
			               Protocol::MsgTypeToString(sm->type()), sm->privateSenderGroup().c_str());
			break;

		case Protocol::MASTER_DISCONNECTED_MSG:
			break;

		case Protocol::JOIN_GROUP_MSG:
			break;

		case Protocol::LEAVE_GROUP_MSG:
			break;

		case Protocol::ARCHIVE_REQUEST_MSG:
			handleArchiveRequest(sm);
			break;

		case Protocol::LIST_CONNECTED_CLIENTS_CMD_MSG: {
			if ( _adminClientName != sm->clientName() ) {
				sendMsg(sm->privateSenderGroup(), Protocol::REJECTED_CMD_MSG);
				break;
			}
			NetworkMessagePtr tmpMsg = createMsg(Protocol::LIST_CONNECTED_CLIENTS_RESPONSE_MSG);
			std::ostringstream ostream;
			for ( ClientDB::const_iterator it = _clientDB.begin(); it != _clientDB.end(); ++it ) {
				if ( it != _clientDB.begin() )
					ostream << "&";

				int days = 0, hours = 0, minutes = 0, seconds = 0;
				(*it)->uptime().elapsedTime(&days, &hours, &minutes, &seconds);

				ostream << (*it)->privateGroup() << "?"
				        << Protocol::ClientTypeToString((*it)->clientType()) << "?"
				        << Protocol::ClientPriorityToString((*it)->clientPriority()) << "?"
				        << (*it)->clientStatus()->programName() << "?"
				        << (*it)->clientStatus()->hostname() << "?" << (*it)->clientStatus()->ips() << "?"
				        << days << ":" << hours << ":" << minutes << ":" << seconds << "?";
			}

			tmpMsg->setData(ostream.str());
			tmpMsg->setDestination(sm->privateSenderGroup());
			send(tmpMsg.get());
			break;
		}

		case Protocol::CLIENT_DISCONNECT_CMD_MSG: {
			if ( _adminClientName != sm->clientName() ) {
				sendMsg(sm->privateSenderGroup(), Protocol::REJECTED_CMD_MSG);
				break;
			}

			const ClientInfo* tmp = _clientDB.getClientFromDB(sm->data().c_str());
			if ( tmp == NULL ) {
				SEISCOMP_WARNING("Unknown client: %s - Rejecting command: %s",
				                 sm->data().c_str(), Protocol::MsgTypeToString(sm->type()));
				sendMsg(sm->privateSenderGroup(), Protocol::REJECTED_CMD_MSG);
				break;
			}
			else {
				SEISCOMP_DEBUG("Sending %s to client: %s", Protocol::MsgTypeToString(sm->type()), sm->data().c_str());
				sendMsg(tmp->privateGroup().c_str(), Protocol::CLIENT_DISCONNECT_CMD_MSG);
				_clientDB.removeClientFromDB(tmp->privateGroup());
			}
			break;
		}

		case Protocol::STATE_OF_HEALTH_CMD_MSG: {
			if ( _adminClientName != sm->clientName() ) {
				sendMsg(sm->privateSenderGroup(), Protocol::REJECTED_CMD_MSG);
				break;
			}
			const ClientInfo* tmp = _clientDB.getClientFromDB(sm->data().c_str());
			if ( tmp == NULL ) {
				if ( _msgGroups.find(sm->data()) != _msgGroups.end() ||
				     sm->data() == Protocol::ADMIN_GROUP ||
				     sm->data() == Protocol::LISTENER_GROUP ||
				     sm->data() == Protocol::IMPORT_GROUP ||
				     sm->data() == Protocol::STATUS_GROUP ) {
					NetworkMessagePtr tmpMsg = createMsg(Protocol::STATE_OF_HEALTH_CMD_MSG);
					tmpMsg->setPrivateSenderGroup(sm->privateSenderGroup());
					tmpMsg->setDestination(sm->data());
					SEISCOMP_DEBUG("Sending %s to group: %s",
					               Protocol::MsgTypeToString(tmpMsg->type()), tmpMsg->destination().c_str());
					send(tmpMsg.get());
				}
				else {
					SEISCOMP_WARNING("Unknown client or group: %s - Rejecting command: %s",
					                 sm->data().c_str(), Protocol::MsgTypeToString(sm->type()));
					sendMsg(sm->privateSenderGroup(), Protocol::REJECTED_CMD_MSG);
				}
				break;
			}
			else {
				NetworkMessagePtr tmpMsg = createMsg(Protocol::STATE_OF_HEALTH_CMD_MSG);
				tmpMsg->setPrivateSenderGroup(sm->privateSenderGroup());
				tmpMsg->setDestination(sm->data());
				SEISCOMP_DEBUG("Sending %s to client: %s",
				               Protocol::MsgTypeToString(tmpMsg->type()), tmpMsg->destination().c_str());

				// SEISCOMP_DEBUG("sender: %s", tmpMsg->privateSenderGroup().c_str());
				// SEISCOMP_DEBUG("destination: %s", tmpMsg->destination().c_str());
				// SEISCOMP_DEBUG("data: %s", tmpMsg->data().c_str());
				// SEISCOMP_DEBUG("message string: %s", tmpMsg->toString());

				send(tmpMsg.get());
			}
			break;
		}

		case Protocol::STATE_OF_HEALTH_RESPONSE_MSG: {
			//NetworkMessage nm;
			NetworkMessagePtr tmpMsg = createMsg(Protocol::STATE_OF_HEALTH_RESPONSE_MSG);

			// tmpMsg->setDestination(sm->data());
			if ( sm->destination() == "STATUS_GROUP" )
				tmpMsg->setDestination("STATUS_GROUP");
			else
				tmpMsg->setDestination(_privateAdminGroup);
			tmpMsg->setPrivateSenderGroup(sm->privateSenderGroup());
			//tmpMsg->setData(Protocol::MsgTypeToString(sm->type()));
			tmpMsg->setData(sm->data());

			int days = 0, hours = 0, minutes = 0, seconds = 0;
			if ( sm->clientName() == Protocol::MASTER_CLIENT_NAME ) {
				_uptime->elapsed().elapsedTime(&days, &hours, &minutes, &seconds);
				std::ostringstream ss;
				ss << ConnectionInfoTag(MESSAGE_QUEUE_SIZE_TAG).toString() << "=" << _networkMessageQueue.size() << "&"
				   << ConnectionInfoTag(SUMMED_MESSAGE_QUEUE_SIZE_TAG).toString() << "=" << _messageStat.summedMessageQueueSize << "&"
				   << ConnectionInfoTag(SENT_MESSAGES_TAG).toString() << "=" << _messageStat.totalSentMessages << "&"
				   << ConnectionInfoTag(RECEIVED_MESSAGES_TAG).toString() << "=" << _messageStat.totalReceivedMessages << "&";

				for ( Plugins::const_iterator it = _plugins.begin(); it != _plugins.end(); ++it )
					(*it)->printStateOfHealthInformation(ss);

				tmpMsg->setData(tmpMsg->data() + ss.str());
			}
			else {
				const ClientInfo* clientInfo = _clientDB.getClientFromDB(tmpMsg->privateSenderGroup());
				if ( !clientInfo ) {
					SEISCOMP_INFO("NOTE: This client (%s) is not in the database. This should rarely happen!\n"
					              "If this occurs frequently, make sure the client orchestration (protocol) works properly.",
					              tmpMsg->privateSenderGroup().c_str());
					break;
				}
				else /* HACK: Fix crash */
					clientInfo->uptime().elapsedTime(&days, &hours, &minutes, &seconds);
			}

			std::ostringstream os;
			os << days
			   << ":"
			   << std::setw(2) << std::setfill('0') << hours
			   << ":"
			   << std::setw(2) << std::setfill('0') << minutes
			   << ":"
			   << std::setw(2) << std::setfill('0') << seconds;
			tmpMsg->setData(tmpMsg->data() + ConnectionInfoTag(UPTIME_TAG).toString() + "=" + os.str() + "&");

			// std::cout << "data: " << tmpMsg->data() << std::endl;

			send(tmpMsg.get());
			break;
		}

		default:
			break;
	}

	delete sm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* Master::createMsg(const int msgType, const char* buf, int len) {
	NetworkMessage* msg = NULL;

	// Check message type
	if ( msgType == 0 )
		return NULL;
	else if ( msgType > 0 )
		msg = new NetworkMessage();
	else if ( msgType < 0 )
		msg = new ServiceMessage(msgType);

	// Default value
	msg->setPrivateSenderGroup(_networkInterface->privateGroup());

	if ( buf != NULL )
		msg->read(buf, len);

	// Set correct timestamp and sequence number
	tagMsg(msg);

	return msg;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::send(NetworkMessage* msg) {
	if ( msg == NULL ) {
		SEISCOMP_ERROR("Message is NULL");
		return Core::Status::SEISCOMP_FAILURE;
	}

	return sendRaw(msg);
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::sendAndArchive(NetworkMessage* msg) {
	if ( msg == NULL ) {
		SEISCOMP_ERROR("Message is NULL");
		return Core::Status::SEISCOMP_FAILURE;
	}

	// Releasing the message is handled by the message archive
	archiveMsg(msg);

	SEISCOMP_DEBUG("Forwarding message to group: %s ", msg->destination().c_str());

	int ret = sendRaw(msg);

	if ( ret != Core::Status::SEISCOMP_SUCCESS )
		SEISCOMP_WARNING("Error: Could not forward message to %s", msg->destination().c_str());

	return ret;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::sendRaw(NetworkMessage* msg) {
	boost::mutex::scoped_lock lock(_sendMutex);

	// Set sequence number and timestamp
	tagMsg(msg);

	int ret = 0;
	while ( (ret = _networkInterface->send(msg->destination(), msg->type(), msg)) !=
	        Core::Status::SEISCOMP_SUCCESS && isRunning() ) {
		SEISCOMP_ERROR(
				"Could not send message [src: %s -> dest: %s seqNum: %d] due to error: %s (%d)",
				msg->privateSenderGroup().c_str(),
				msg->destination().c_str(),
				msg->seqNum(),
				Core::Status::StatusToStr(ret), ret
		);
		if ( !_networkInterface->isConnected() ) {
			SEISCOMP_ERROR("Master is disconnected. Trying to reconnect ...");
			reconnect();
		}
	}

	if ( ret == Core::Status::SEISCOMP_SUCCESS ) {
		_messageStat.summedMessageSize      += msg->size();
		++_messageStat.totalSentMessages;
	}

	return ret;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::tagMsg(NetworkMessage* msg) {
	if ( !msg->tagged() )
		msg->tag(sequenceNumber(), timeStamp());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::sendMsg(const std::string& destination, int msgType) {
	NetworkMessagePtr tmpMsg = createMsg(msgType);
	tmpMsg->setDestination(destination);
	return send(tmpMsg.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::connect(const std::string& serverAddress) {
	// Connect to the server
	int ret = _networkInterface->connect(serverAddress, Protocol::MASTER_CLIENT_NAME);
	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		SEISCOMP_ERROR("Could not connect to server: %s", serverAddress.c_str());
		return ret;
	}

	SEISCOMP_INFO("Subscribing group: %s", Protocol::MASTER_GROUP.c_str());
	ret = _networkInterface->subscribe(Protocol::MASTER_GROUP);
	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		SEISCOMP_ERROR("Could not subscribe group: %s", Protocol::MASTER_GROUP.c_str());
		return ret;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::disconnect() {
	SEISCOMP_INFO("Master is Disconnecting from spread deamon");

	NetworkMessagePtr tmpMsg = createMsg(Protocol::MASTER_DISCONNECTED_MSG);
	tmpMsg->setDestination(Protocol::MASTER_GROUP);
	int ret = send(tmpMsg.get());
	if ( ret != Core::Status::SEISCOMP_SUCCESS )
		SEISCOMP_ERROR("Could not send disconnect message to clients");

	SEISCOMP_INFO("Unsubscribing group: %s", Protocol::MASTER_GROUP.c_str());
	ret = _networkInterface->unsubscribe(Protocol::MASTER_GROUP);
	if ( ret != Core::Status::SEISCOMP_SUCCESS )
		SEISCOMP_ERROR("Could not unsubscribe group: %s", Protocol::MASTER_GROUP.c_str());

	ret = _networkInterface->disconnect();
	if ( ret != Core::Status::SEISCOMP_SUCCESS )
		SEISCOMP_ERROR("Could not disconnect from the spread deamon: ");

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::reconnect() {
	boost::mutex::scoped_lock lock(_reconnectMutex);
	if ( _networkInterface->isConnected() ) return;

	SEISCOMP_DEBUG("Reconnecting ...");
	_networkInterface->disconnect();
	_clientDB.clear();
	while ( connect(_serverAddress) != Core::Status::SEISCOMP_SUCCESS && isRunning() ) {
		SEISCOMP_INFO("Trying to reconnect to server: %s", _serverAddress.c_str());
		Core::sleep(1);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::archiveMsg(NetworkMessage* msg) {
	boost::mutex::scoped_lock lock(_archiveMutex);

	int idx = (msg->seqNum() % Protocol::MASTER_ARCHIVE_SIZE);
	if ( (*_archive) [idx] != NULL )
		delete (*_archive) [idx];

	(*_archive) [idx] = msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::handleArchiveRequest(ServiceMessage* msg) {
	boost::mutex::scoped_lock lock(_archiveMutex);

	int seqNum = msg->archiveSeqNum();
	time_t timestamp = msg->archiveTimestamp();

	int idx = seqNum % Protocol::MASTER_ARCHIVE_SIZE;

	NetworkMessage* archiveMsg = (*_archive) [idx];

	if ( archiveMsg == NULL ) {
		SEISCOMP_INFO("Archive message with index: %d not available", idx);
		NetworkMessagePtr tmpMsg = createMsg(Protocol::INVAlID_ARCHIVE_REQUEST_MSG);
		tmpMsg->setDestination(msg->privateSenderGroup());
		send(tmpMsg.get());
	}
	else {
		if ( archiveMsg->timestamp() != timestamp ) {
			SEISCOMP_INFO("Archive message with index: %d is too old", idx);
			SEISCOMP_INFO(
					"Archive timestamp (%d) minus Request timestamp (%d): %d",
					(int) archiveMsg->timestamp(), (int) timestamp,
					(int) (archiveMsg->timestamp() - timestamp)
			);
			NetworkMessagePtr tmpMsg = createMsg(Protocol::INVAlID_ARCHIVE_REQUEST_MSG);
			tmpMsg->setDestination(msg->privateSenderGroup());
			send(tmpMsg.get());
		}
		else {
			NetworkMessage* tmpMsg = NULL;
			time_t tmpTimestamp = timestamp;
			for ( int i = idx;; ++i ) {
				tmpMsg = (*_archive) [i % Protocol::MASTER_ARCHIVE_SIZE];
				if ( tmpMsg == NULL ) break;
				// Ignore service messages
				if ( tmpMsg->type() < 0 ) continue;
				if ( (tmpMsg->timestamp() - tmpTimestamp) < 0 ) break;

				tmpMsg->setType(Protocol::ARCHIVE_MSG);
				send(tmpMsg);
				tmpTimestamp = tmpMsg->timestamp();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
time_t Master::timeStamp() {
	time_t timeStamp;
	time(&timeStamp);

	return timeStamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Master::sequenceNumber() {
	if ( _seqNum == _maxSeqNum )
		_seqNum = 0;
	else
		++_seqNum;

	return _seqNum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Master::setRunning(bool b) {
	_isRunning = b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Master::isRunning() const {
	return _isRunning;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Master::msgGroups() {
	std::string msgGroups;
	std::set<std::string>::iterator it = _msgGroups.begin();
	for ( ; it != _msgGroups.end(); ++it ) {
		if ( *it != *_msgGroups.rbegin() )
			msgGroups += *it + ",";
		else
			msgGroups += *it;
	}

	return msgGroups;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Communincation
} // namespace Seiscomp
