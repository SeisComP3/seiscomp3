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

#define SEISCOMP_COMPONENT admin

#include "admin.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Admin::~Admin()
{
	if (_client.get())
		_client->disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::connect(const std::string& serverAddress,
				   const std::string& clientName,
				   const std::string& password)
{
	_client = new Communication::SystemConnection;
	_client->setPassword(password);
	int ret = _client->connect(serverAddress.c_str(),
	                           clientName.c_str(),
	                           Communication::Protocol::ADMIN_GROUP.c_str(),
	                           Communication::Protocol::TYPE_ONE,
	                           Communication::Protocol::PRIORITY_HIGH);
	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		std::cout << Core::Status::StatusToStr(ret) << std::endl;
		return false;
	}

	_client->subscribe(Communication::Protocol::ADMIN_GROUP);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::disconnect()
{
	int ret = _client->disconnect();
	if (ret != Core::Status::SEISCOMP_SUCCESS)
		std::cout << Core::Status::StatusToStr(ret) << std::endl;
	return ret == Core::Status::SEISCOMP_SUCCESS;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::showGroupMembers(const std::string& group, std::vector<std::string>& members)
{
	if (_client->subscribe(group) != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("Could not subscribe to group: %s", group.c_str());
		return false;
	}

	Communication::NetworkMessagePtr msg;
	while (true)
	{
		msg = readResponse();
		if (msg->type() != Communication::Protocol::JOIN_GROUP_MSG)
			continue;

		std::string data = msg->data();
		std::string::size_type pos0 = data.find("?");
		pos0 += 1;
		std::string::size_type pos1 = data.find("?", pos0);

		std::vector<std::string> tokens;
		Core::split(tokens, data.substr(pos0, pos1 - pos0).c_str(), "&");
		// std::cout << "group: " << group << " == " << tokens[0] << std::endl;
		// std::cout << "client name: " << tokens[1] << " == " <<  _client->privateGroup() << std::endl;
		if (tokens[0] == group && tokens[1] == _client->privateGroup())
			break;
	}

	std::string data = msg->data();
	std::string::size_type pos = data.find_last_of("?");

	std::vector<std::string> tokens;
	data = data.substr(pos+1);
	if (data.empty())
		return false;
	Core::split(tokens, data.c_str(), "&");
	for (size_t i = 0; i < tokens.size(); ++i)
		members.push_back(extractClientName(tokens[i]));

	_client->unsubscribe(group);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::showAvailableGroups(std::vector<std::string>& groups)
{
	if (_client.get())
	{
		groups = _client->groups();
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::showConnectedClients(std::vector<std::string>& clients)
{
	if (sendCommand(Communication::Protocol::LIST_CONNECTED_CLIENTS_CMD_MSG) != Core::Status::SEISCOMP_SUCCESS)
		return false;

	Communication::NetworkMessagePtr msg;
	while ( (msg = readResponse())->type() !=
		Communication::Protocol::LIST_CONNECTED_CLIENTS_RESPONSE_MSG );
	if (msg.get() != NULL) {
		Core::split(clients, msg->data().c_str(), "&", false);
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::disconnectClient(const std::string& clientName)
{
	int ret = sendCommand(Communication::Protocol::CLIENT_DISCONNECT_CMD_MSG, clientName);
	if (ret != Core::Status::SEISCOMP_SUCCESS)
		std::cout << Core::Status::StatusToStr(ret) << std::endl;
	return ret == Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Admin::requestHealthStatus(const std::string& groupOrClient)
{
	if (sendCommand(Communication::Protocol::STATE_OF_HEALTH_CMD_MSG, groupOrClient) != Core::Status::SEISCOMP_SUCCESS)
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Communication::ClientStatus* Admin::showHealthStatus()
{
	Communication::NetworkMessagePtr msg;
	while ( (msg = readResponse())->type() !=
			Communication::Protocol::STATE_OF_HEALTH_RESPONSE_MSG );
	if ( msg.get() == NULL  )
		return NULL;

	return Communication::ClientStatus::CreateClientStatus(msg->data());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Admin::sendCommand(Communication::Protocol::SERVICE_MSG_TYPES type, const std::string& data)
{
	// std::auto_ptr<NetworkMessage> sm(new Communication::ServiceMessage(type));
	Communication::NetworkMessagePtr sm = new Communication::ServiceMessage(type);
	sm->setData(data);
	int ret = _client->send(sm.get());

	if (ret != 0)
	{
		if (ret == Core::Status::SEISCOMP_NOT_CONNECTED_ERROR)
		{
			std::cout << "Master disconnected from the system!" << std::endl;
			std::cout << "Trying to reconnect ..." << std::endl;
			while ((ret = _client->reconnect()) != Core::Status::SEISCOMP_SUCCESS)
				Core::sleep(1);
			std::cout << "Reconnected" << std::endl;
		}
	}
	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Communication::NetworkMessage* Admin::readResponse()
{
	Communication::NetworkMessage* msg = _client->receive();
	if (msg == NULL && _client->isConnected() == false) {
		std::cout << "Reconnecting to server ..." << std::endl;
		while (_client->reconnect()) {
			Core::sleep(1);
		}
	}
	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Admin::extractClientName(const std::string& str)
{
	std::string::size_type pos0 = str.find("#");
	pos0 += 1;
	std::string::size_type pos1 = str.find("#", pos0);
	return str.substr(pos0, pos1 - pos0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Seiscomp
