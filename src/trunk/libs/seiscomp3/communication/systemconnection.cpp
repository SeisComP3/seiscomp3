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
#define SEISCOMP_COMPONENT Communication
#include <seiscomp3/logging/log.h>

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/version.hpp>

#include <seiscomp3/communication/systemconnection.h>
#include <seiscomp3/communication/networkinterface.h>
#include <seiscomp3/communication/connectioninfo.h>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/datamodel/version.h>


namespace Seiscomp {
namespace Communication {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SystemConnection::SystemConnection(NetworkInterface* networkInterface) :
		_networkInterface(networkInterface),
		_schemaVersion(0),
		_type(Protocol::TYPE_DEFAULT),
		_priority(Protocol::PRIORITY_DEFAULT),
		_archiveRequested(false),
		_archiveMsgLen(0),
		_subscribedArchiveGroups(),
		_groups(),
		_subscriptions(),
		_stopRequested(false),
		_isConnected(false)
{
	_messageStat = std::auto_ptr<MessageStat>(new MessageStat);
	_connectionInfo = ConnectionInfo::Instance();
	if (_connectionInfo)
		_connectionInfo->registerConnection(this);

	if (networkInterface == NULL)
		_networkInterface = NetworkInterface::Create("spread");

	if (_networkInterface.get() == NULL)
		SEISCOMP_DEBUG("communication interface is NULL");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SystemConnection::~SystemConnection()
{
	if (_connectionInfo)
		_connectionInfo->unregisterConnection(this);

	disconnect();

	const Seiscomp::Environment* env = Seiscomp::Environment::Instance();
	std::string archiveFilePath = env->archiveFileName(_clientName.c_str());
	std::fstream archiveFile(archiveFilePath.c_str(),
	                         std::ios::trunc | std::ios::in | std::ios::out);
	if (_archiveMsgLen > 0)
		archiveFile.write(_archiveMsg, _archiveMsgLen);
	archiveFile.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::handshake(const std::string &protocolVersion,
                                std::string *supportedVersion) {
	// Ok, send request to the master client.
	SEISCOMP_INFO("Sending connect message to server: %s", _masterAddress.c_str());
	ServiceMessage synMsg(Protocol::CONNECT_GROUP_MSG, _type, _priority);
	synMsg.setPeerGroup(_peerGroup);
	synMsg.setPrivateSenderGroup(_networkInterface->privateGroup());
	synMsg.setProtocolVersion(protocolVersion);
	synMsg.setDestination(Protocol::MASTER_GROUP);
	synMsg.setPassword(password());
	synMsg.setData(_connectionInfo->info(this));

	int ret = send(Protocol::MASTER_GROUP.c_str(), Protocol::CONNECT_GROUP_MSG, &synMsg);
	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		SEISCOMP_ERROR("Could not send connect message to server: %s due to error %i",
					   _masterAddress.c_str(), ret);
		SEISCOMP_ERROR("%s", Protocol::MsgTypeToString(ret));
		return ret;
	}

	Util::StopWatch timer;
	double fTimeOut = (double)_timeOut * 0.001;

	NetworkMessagePtr ackMessage = NULL;
	int error = 0;
	// Read the response from the server on the private channel
	do {
		if ( _networkInterface->poll() ) {
			ackMessage = _networkInterface->receive(&error);
			if ( ackMessage ) {
				if ( ackMessage->destination() != _networkInterface->privateGroup() ) {
					ackMessage = NULL;
					continue;
				}

				if ( error < 0 ) {
					SEISCOMP_ERROR("Could not read acknowledgment message");
					return error;
				}
			}
		}
		else {
			if ( (double)timer.elapsed() >= fTimeOut ) {
				SEISCOMP_ERROR( "Timeout while waiting for acknowledgment message" );
				return Core::Status::SEISCOMP_TIMEOUT_ERROR;
			}

			// Sleep 200 milliseconds
			Core::msleep(200);
		}
	} while ( !ackMessage.get() );

	if ( ackMessage->type() == Protocol::CONNECT_GROUP_OK_MSG ) {
		//Copy the private group name of the master
		_privateMasterGroup = _networkInterface->groupOfLastSender();

		_groups.clear();

		if ( static_cast<ServiceMessage*>(ackMessage.get())->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_0 ) {
			Core::split(_groups, ackMessage->data().c_str(), ",");
			_schemaVersion = 0;
		}
		else if ( static_cast<ServiceMessage*>(ackMessage.get())->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_1 ) {
			size_t pos;
			std::vector<std::string> lines;
			Core::split(lines, ackMessage->data().c_str(), "\n");
			for ( size_t i = 0; i < lines.size(); ++i ) {
				pos = lines[i].find(':');
				if ( pos == std::string::npos ) continue;
				if ( lines[i].compare(0, pos, Protocol::HEADER_GROUP_TAG) == 0 ) {
					lines[i].erase(0,pos+1);
					Core::split(_groups, Core::trim(lines[i]).c_str(), ",");
				}
				else if ( lines[i].compare(0, pos, Protocol::HEADER_SCHEMA_VERSION_TAG) == 0 ) {
					lines[i].erase(0,pos+1);
					if ( !_schemaVersion.fromString(lines[i]) ) {
						SEISCOMP_WARNING("Invalid Schema-Version content: %s", lines[i].c_str());
						continue;
					}
				}
				else if ( lines[i].compare(0, pos, Protocol::HEADER_SERVER_VERSION_TAG) == 0 ) {
					pos = lines[i].find_first_not_of(' ', pos+1);
					SEISCOMP_INFO("Server version is '%s'", lines[i].c_str() + pos);
				}
			}
		}
		else
			return Core::Status::SEISCOMP_WRONG_SERVER_VERSION;

		if ( std::find(_groups.begin(), _groups.end(), Protocol::IMPORT_GROUP) == _groups.end() )
			_groups.insert(_groups.begin(), Protocol::IMPORT_GROUP);

		if ( std::find(_groups.begin(), _groups.end(), Protocol::STATUS_GROUP) == _groups.end() )
			_groups.insert(_groups.begin(), Protocol::STATUS_GROUP);
	}
	else if ( ackMessage->type() == Protocol::INVALID_PROTOCOL_MSG ) {
		SEISCOMP_INFO("The master protocol v%s is not compatible with client v%s",
		              static_cast<ServiceMessage*>(ackMessage.get())->protocolVersion().c_str(),
		              Protocol::PROTOCOL_VERSION_V1_1);
		if ( supportedVersion ) *supportedVersion = static_cast<ServiceMessage*>(ackMessage.get())->protocolVersion();
		return Core::Status::SEISCOMP_WRONG_SERVER_VERSION;
	}
	else {
		SEISCOMP_INFO("Could not connect to the master due to message: %s",
		              Protocol::MsgTypeToString(ackMessage->type()));
		return Core::Status::SEISCOMP_CONNECT_ERROR;
	}

	Core::Version maxVersion = Core::Version(DataModel::Version::Major, DataModel::Version::Minor);
	if ( _schemaVersion > maxVersion ) {
		SEISCOMP_INFO("Outgoing messages are encoded to match schema version %d.%d, "
		              "although server supports %d.%d",
		              maxVersion.majorTag(), maxVersion.minorTag(),
		              _schemaVersion.majorTag(), _schemaVersion.minorTag());
		_schemaVersion = maxVersion;
	}
	else
		SEISCOMP_INFO("Outgoing messages are encoded to match schema version %d.%d",
		              _schemaVersion.majorTag(), _schemaVersion.minorTag());

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::connect(const std::string &masterAddress,
                              const std::string &clientName,
                              const std::string &peerGroup,
                              Protocol::ClientType type,
                              Protocol::ClientPriority priority,
                              int timeOut)
{
	boost::try_mutex::scoped_try_lock l(_readBufferMutex);

	_timeOut = timeOut;
	_schemaVersion = 0;

	if ( isConnected() )
		return Core::Status::SEISCOMP_SUCCESS;

	if ( peerGroup == Protocol::MASTER_GROUP ) {
		SEISCOMP_ERROR("You cannot be a regular member of group: %s", peerGroup.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}
	_peerGroup = peerGroup;

	if (!clientName.empty())
		_clientName = clientName;

	_masterAddress = masterAddress;

	_type = type;
	_priority = priority;

	// Connect to the server
	SEISCOMP_INFO("Connecting to server: %s", _masterAddress.c_str());

	int ret = _networkInterface->connect(_masterAddress, _clientName);
	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		SEISCOMP_ERROR("Could not connect to server: %s, internal error: %d", _masterAddress.c_str(), ret);
		return ret;
	}

	SEISCOMP_INFO("Connected to message server: %s", _masterAddress.c_str());

	// Clear previously requested archive groups
	_subscribedArchiveGroups.clear();

	// IMPORTANT: Once we are connected to the messaging system (currently spread)
	// we subscribe to the messaging group. On account of this the master receives
	// membership message for this client because he is also a member of this group.
	// Otherwise, in case of an error, the client activities will not reported to
	// the master.
	SEISCOMP_INFO("Joining %s group", Protocol::MASTER_GROUP.c_str());
	ret = _networkInterface->subscribe(Protocol::MASTER_GROUP);
	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		SEISCOMP_ERROR("Could not join %s", Protocol::MASTER_GROUP.c_str());
		shutdown();
		return ret;
	}

	std::string masterProtocolVersion;
	ret = handshake(Protocol::PROTOCOL_VERSION_V1_1, &masterProtocolVersion);
	if ( ret == Core::Status::SEISCOMP_WRONG_SERVER_VERSION ) {
		if ( masterProtocolVersion == Protocol::PROTOCOL_VERSION_V1_0 ) {
			SEISCOMP_INFO("Falling back to master protocol v%s",
			              masterProtocolVersion.c_str());
			ret = handshake(Protocol::PROTOCOL_VERSION_V1_0);
		}
		else {
			SEISCOMP_ERROR("Unsupported master protocol v%s",
			               masterProtocolVersion.c_str());
		}
	}

	if ( ret != Core::Status::SEISCOMP_SUCCESS ) {
		shutdown();
		return ret;
	}

	_isConnected = true;

	if ( _connectionInfo ) _connectionInfo->start();

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::reconnect()
{
	if (isConnected())
		disconnect();

	int ret = connect(_masterAddress, _clientName, _peerGroup,
	                  _type, _priority, _timeOut);
	if (ret == Core::Status::SEISCOMP_SUCCESS )
	{
		for (std::set<std::string>::iterator it = _subscriptions.begin(); it != _subscriptions.end(); it++)
				subscribe(*it);

		SEISCOMP_INFO("Client is reconnected to master client");
	}
	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::disconnect()
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	// Ok, send disconnect request to the master client.
	ServiceMessage disconnectMsg(Protocol::CLIENT_DISCONNECTED_MSG, _type, _priority,
	                             _networkInterface->privateGroup());
	if (send(_privateMasterGroup, Protocol::CLIENT_DISCONNECTED_MSG, &disconnectMsg) < 0)
		SEISCOMP_ERROR("Could not send disconnect message to server");

	_groups.clear();

	// Clear the message queue and delete all remaining messages
	while (!_messageQueue.empty())
	{
		delete _messageQueue.front();
		_messageQueue.pop();
	}

	return shutdown();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SystemConnection::isConnected()
{
	return _networkInterface->isConnected() && _isConnected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::subscribe(const std::string& group)
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	_archiveRequested = true;
	int ret = 0;

	if (!isGroupAvailable(group))
	{
		SEISCOMP_ERROR("Group: %s does not exits!", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}
	if (group == Protocol::MASTER_GROUP)
	{
		SEISCOMP_INFO("Group is solely for private communication: %s", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	_subscriptions.insert(group);

	SEISCOMP_INFO("Joining group: %s", group.c_str());
	ret = _networkInterface->subscribe(group);
	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("Could not subscribe to group: %s", group.c_str());
		return ret;
	}

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::unsubscribe(const std::string& group)
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	int ret = 0;
	if (!isGroupAvailable(group))
	{
		SEISCOMP_ERROR("Group: %s does not exits!", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	if (group == Protocol::MASTER_GROUP)
	{
		SEISCOMP_INFO("Group is solely for private communication: %s", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	_subscriptions.erase(group);

	ret = _networkInterface->unsubscribe(group);
	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("Could not unsubscribe group: %s", group.c_str());
		return ret;
	}
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::subscribeArchive(const std::string& group)
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	if (!isGroupAvailable(group))
	{
		SEISCOMP_ERROR("Group: %s does not exits!", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	if (group == Protocol::MASTER_GROUP)
	{
		SEISCOMP_INFO("Group is solely for private communication: %s", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	_subscribedArchiveGroups.insert(group);
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::unsubscribeArchive(const std::string& group)
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	if (!isGroupAvailable(group))
	{
		SEISCOMP_ERROR("Group: %s does not exits!", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	if (group == Protocol::MASTER_GROUP)
	{
		SEISCOMP_INFO("Group is solely for private communication: %s", group.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	_subscribedArchiveGroups.erase(group);
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SystemConnection::poll() const
{
	return _networkInterface->poll();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& SystemConnection::peerGroup() const
{
	return _peerGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& SystemConnection::masterAddress() const
{
	return _masterAddress;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string SystemConnection::privateGroup() const
{
	return _networkInterface->privateGroup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Communication::Protocol::ClientType SystemConnection::type() const
{
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Communication::Protocol::ClientPriority SystemConnection::priority() const
{
	return _priority;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::readNetworkMessage(bool blocking)
{
	while (true)
	{
		std::auto_ptr<NetworkMessage> message;
		int error = 0;

		{

#if (BOOST_VERSION >= 103500)
			boost::try_mutex::scoped_try_lock l(_readBufferMutex, boost::defer_lock);
#else
			boost::try_mutex::scoped_try_lock l(_readBufferMutex, false);
#endif

			if ( !blocking ) {
				if ( !l.try_lock() || !poll())
					return Core::Status::SEISCOMP_FAILURE;
			}
			else
				l.lock();

			message = std::auto_ptr<NetworkMessage>(_networkInterface->receive(&error));
		}

		if (message.get() == NULL)
		{
			if (error == Core::Status::SEISCOMP_NOT_CONNECTED_ERROR)
			{
				SEISCOMP_DEBUG("Connection has been closed while reading from network");
				return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;
			}
			else if (error == Core::Status::SEISCOMP_NETWORKING_ERROR)
			{
				if ( _networkInterface->isConnected() ) {
					SEISCOMP_ERROR("Error during reading attempt: (%i)", error);
					shutdown();
				}
				return Core::Status::SEISCOMP_NETWORKING_ERROR;
			}
			else
				continue;
		}

		//SEISCOMP_DEBUG("= seiscomp message =");

		// Data message
		if (message->type() > 0)
		{
			// NOTE: This can be done automatically by a flag of the the connect call
			if (message->privateSenderGroup() == _networkInterface->privateGroup())
			{
				/*
				SEISCOMP_INFO("Sender is myself: %s -> Skipping message!",
				              message->privateSenderGroup().c_str());
				*/
			}
			else if (message->type() == Protocol::ARCHIVE_MSG)
			{
				std::string dest = message->destination();
				if (_subscribedArchiveGroups.find(dest) != _subscribedArchiveGroups.end())
				{
					//boost::mutex::scoped_lock l(messageQueueMutex);
					queueMessage(message);
					return Core::Status::SEISCOMP_SUCCESS;
				}
			}
			else
			{
				//boost::mutex::scoped_lock l(messageQueueMutex);
				queueMessage(message);
				return Core::Status::SEISCOMP_SUCCESS;
			}
		}
		// Service message
		else if (message->type() < 0)
		{
			ServiceMessage *sm = static_cast<ServiceMessage*>(message.get());
			switch (sm->type())
			{
			case Protocol::MASTER_DISCONNECTED_MSG:
				SEISCOMP_INFO("Received %s msg - Shutting down connection", Protocol::MsgTypeToString(sm->type()));
				shutdown();
				return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

			case Protocol::CLIENT_DISCONNECTED_MSG:
			case Protocol::JOIN_GROUP_MSG:
				queueMessage(message);
				return Core::Status::SEISCOMP_SUCCESS;

			case Protocol::LEAVE_GROUP_MSG:
				queueMessage(message);
				return Core::Status::SEISCOMP_SUCCESS;

			case Protocol::INVAlID_ARCHIVE_REQUEST_MSG:
				break;

			case Protocol::CLIENT_DISCONNECT_CMD_MSG:
				SEISCOMP_INFO("Received %s Closing application...", Protocol::MsgTypeToString(sm->type()));
				disconnect();
				exit(Core::Status::SEISCOMP_SUCCESS);

			case Protocol::STATE_OF_HEALTH_CMD_MSG:
				// sm->setData(sm->privateSenderGroup());
				sm->setData(ConnectionInfo::Instance()->info(this));
				sm->setType(Protocol::STATE_OF_HEALTH_RESPONSE_MSG);
				SEISCOMP_DEBUG("Sending SOH response message to %s", sm->privateSenderGroup().c_str());
				send(sm->privateSenderGroup(), sm);
				break;

			case Protocol::STATE_OF_HEALTH_RESPONSE_MSG:
				{
					queueMessage(message);
					return Core::Status::SEISCOMP_SUCCESS;
				}

			case Protocol::LIST_CONNECTED_CLIENTS_RESPONSE_MSG:
				{
					queueMessage(message);
					return Core::Status::SEISCOMP_SUCCESS;
				}

			case Protocol::REJECTED_CMD_MSG:
				{
					queueMessage(message);
					return Core::Status::SEISCOMP_SUCCESS;
				}

			default:
				break;
			}
		}
		else
			SEISCOMP_INFO("Undefined message received!");
	}
	return Core::Status::SEISCOMP_FAILURE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::send(NetworkMessage* msg)
{
	return send(_peerGroup, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::send(const std::string& groupname, NetworkMessage* msg)
{
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	if (groupname == Protocol::MASTER_GROUP)
	{
		SEISCOMP_ERROR("You cannot be a regular member of group: %s", groupname.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	if ( (isGroupAvailable(groupname) == false &&
	      groupname != _privateMasterGroup) ||
	     (groupname == Protocol::ADMIN_GROUP && _peerGroup != Protocol::ADMIN_GROUP) )
	{
		if (groupname == Protocol::LISTENER_GROUP)
			SEISCOMP_ERROR("You cannot send to %s!", Protocol::LISTENER_GROUP.c_str());
		else
			SEISCOMP_ERROR("Group: %s does not exist!", groupname.c_str());
		return Core::Status::SEISCOMP_INVALID_GROUP_ERROR;
	}

	// Before we send this message, read (nonblocking) all available messages from
	// the queue to make sure not to miss a service message which we should react upon.
	while (readNetworkMessage(false) == Core::Status::SEISCOMP_SUCCESS);
	if (!isConnected())
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

	msg->setPrivateSenderGroup(_networkInterface->privateGroup());
	msg->setDestination(groupname);
	return (int)send(_privateMasterGroup, msg->type(), msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::send(const std::string& group, int type, NetworkMessage* msg)
{
	boost::mutex::scoped_lock l(_writeBufferMutex);
	int ret = _networkInterface->send(group, type, msg);
	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("Could not send message to server: %s, due to error: %d", _masterAddress.c_str(), ret);
		return ret;
	}
	++_messageStat->totalSentMessages;
	_messageStat->summedMessageSize += msg->size();

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::queuedMessageCount() const
{
	boost::mutex::scoped_lock l(_messageQueueMutex);
    return _messageQueue.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::shutdown()
{
	_archiveRequested = false;
	_isConnected = false;
	if (_networkInterface->isConnected())
	{
		int ret = _networkInterface->disconnect();
		if (ret != Core::Status::SEISCOMP_SUCCESS)
		{
			SEISCOMP_ERROR("Could not properly disconnect!");
			return Core::Status::SEISCOMP_NETWORKING_ERROR;
		}
	}
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SystemConnection::isGroupAvailable(const std::string& group)
{
	std::vector<std::string>::iterator found;
	found = find(_groups.begin(), _groups.end(), group);

	return (found != _groups.end() ||
			group == Protocol::STATUS_GROUP ||
			group == Protocol::ADMIN_GROUP);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemConnection::queueMessage(NetworkMessage *message)
{
	boost::mutex::scoped_lock l(_messageQueueMutex);
	_messageQueue.push(message);
	++_messageStat->totalReceivedMessages;
	_messageStat->summedMessageQueueSize += _messageQueue.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemConnection::queueMessage(std::auto_ptr<NetworkMessage>& message)
{
	queueMessage(message.release());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* SystemConnection::receive(bool blocking, int *error)
{
	if (error)
		*error = Core::Status::SEISCOMP_SUCCESS;

	if (/*blocking == true && */queuedMessageCount() == 0)
	{
		int ret = readNetworkMessage(blocking);
		if (error) *error = ret;
		if (ret != Core::Status::SEISCOMP_SUCCESS)
			return NULL;
	}

	NetworkMessage* message = readLocalMessage();

	// Archive message to have a possibility to start an archive request
	// later on.
	if (message != NULL)
		_archiveMsgLen = message->write(_archiveMsg, Protocol::STD_MSG_LEN);
	return message;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* SystemConnection::readLocalMessage()
{
	boost::mutex::scoped_lock l(_messageQueueMutex);
	NetworkMessage *msg = NULL;

	if (!_messageQueue.empty())
	{
		msg = _messageQueue.front();
		_messageQueue.pop();
	}

	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::archiveRequest()
{
	if (_archiveRequested)
	{
		SEISCOMP_INFO( "Archived messages have been requested before or\n\
		               this call has not been placed directly after connect!\n\
		               To set another request connect again to the server!" );
		return Core::Status::SEISCOMP_ARCHIVE_REQUEST_ERROR;

	}
	_archiveRequested = true;

	// Read backup message (Last message read from the queue)
	const Seiscomp::Environment* env = Seiscomp::Environment::Instance();
	std::string archiveFilePath = env->archiveFileName( _clientName.c_str() );
	std::fstream archiveFile( archiveFilePath.c_str(), std::ios::in );

	std::string line;
	if (archiveFile.is_open())
		getline( archiveFile, line, '\0' );
	else
	{
		SEISCOMP_ERROR( "Could not open file: %s", archiveFilePath.c_str( ) );
		return Core::Status::SEISCOMP_ARCHIVE_REQUEST_ERROR;
	}

	if (line.empty())
	{
		SEISCOMP_INFO("Archive file was empty: %s", archiveFilePath.c_str());
		return Core::Status::SEISCOMP_ARCHIVE_REQUEST_ERROR;
	}

	archiveFile.close();

	NetworkMessage msg;
	msg.read(line.c_str(), line.size());
	ServiceMessage sm(Protocol::ARCHIVE_REQUEST_MSG);

	sm.setArchiveTimestamp(msg.timestamp());
	sm.setArchiveSeqNum(msg.seqNum());

	SEISCOMP_DEBUG("Message: %s  MessageID: %d seqNum: %d timestamp: %d",
	               Protocol::MsgTypeToString(sm.type()),
	               sm.type(),
	               sm.archiveSeqNum(),
	               (int) sm.archiveTimestamp());

	if (poll() || queuedMessageCount() > 0)
		std::cout << "There are messages in the queue!" << std::endl;

	send(_privateMasterGroup, &sm);
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::groupCount() const
{
	return _groups.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* SystemConnection::group(int i) const
{
	if ( i >= groupCount() || i < 0 )
		return NULL;
	return _groups[i].c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<std::string> SystemConnection::groups() const
{
	return _groups;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& SystemConnection::password() const
{
	return _password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemConnection::setPassword(const std::string& password)
{
	_password = password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SystemConnection::listen(ListenMode mode)
{
	int ret = 0;

	if (isListening())
	{
		SEISCOMP_DEBUG("Already listening");
		return 0;
	}

	if (!isConnected())
		return 0;

	if (mode == THREADED)
	{
		SEISCOMP_DEBUG("Creating worker thread");
		boost::thread thrd(boost::bind(&SystemConnection::listen, this, NON_THREADED));
		boost::thread::yield();
		return 0;
	}

	SEISCOMP_DEBUG("Entering listening state");
	_isListening = true;
	_stopRequested = false;
	while (!_stopRequested)
		ret += readNetworkMessage(true);
	_isListening = false;
	SEISCOMP_DEBUG("Leaving listening state");

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemConnection::stopListening()
{
	_stopRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SystemConnection::isListening() const
{
	return _isListening;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MessageStat& SystemConnection::messageStat() const
{
	return *_messageStat;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const NetworkInterface* SystemConnection::networkInterface() const
{
	return _networkInterface.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemConnection::setSequenceNumber(int64_t seq)
{
	_networkInterface->setSequenceNumber(seq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t SystemConnection::getSequenceNumber() const
{
	return _networkInterface->getSequenceNumber();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Version SystemConnection::schemaVersion() const
{
	return _schemaVersion;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Communication
} // namespace Seiscomp
