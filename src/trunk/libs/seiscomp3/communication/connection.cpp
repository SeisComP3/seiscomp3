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
#include <seiscomp3/communication/networkinterface.h>

#include "connection.h"


namespace Seiscomp
{
namespace Communication
{
namespace
{

Protocol::MSG_CONTENT_TYPES encodingLUT[MessageEncoding::Quantity] =
{
	Protocol::CONTENT_BINARY,
	Protocol::CONTENT_XML
};


NetworkMessage* encode(Core::Message *msg, const MessageEncoding &enc,
                       int schemaVersion)
{
	return NetworkMessage::Encode(msg, encodingLUT[enc], schemaVersion);
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection::Connection(NetworkInterface* networkInterface) :
		SystemConnection(networkInterface),
		_encoding(BINARY_ENCODING),
		_transmittedBytes(0),
		_receivedBytes(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection::~Connection()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection* Connection::Create(const std::string& netName,
                               const std::string& user,
                               const std::string& group,
                               Protocol::ClientPriority priority,
                               int timeout,
                               int* status)
{
	if (netName.empty())
		return NULL;

	std::string protocol = "spread";
	std::string server = netName;
	size_t pos = netName.find("://");

	if ( pos != std::string::npos ) {
		protocol = netName.substr(0,pos);
		server = netName.substr(pos+3);
	}

	NetworkInterfacePtr ni = NetworkInterface::Create(protocol.c_str());
	if (ni == NULL)
	{
		SEISCOMP_DEBUG("Networkinterface \"%s\" not found", protocol.c_str());
		return NULL;
	}

	Connection* con = new Connection(ni.get());

	int ret = con->connect(server, user, group, Protocol::TYPE_DEFAULT,
	                       priority, timeout);
	if (status != NULL)
		*status = ret;
	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		delete con;
		return NULL;
	}

	SEISCOMP_INFO("user \"%s\" connected successfully to %s", user.c_str(), netName.c_str());

	return con;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::setEncoding(MessageEncoding enc)
{
	_encoding = enc;
	if ( _encoding < 0 || _encoding >= MessageEncoding::Quantity )
		_encoding = BINARY_ENCODING;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MessageEncoding Connection::encoding() const {
	return _encoding;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Connection::send(Seiscomp::Core::Message* msg)
{
	if (!isConnected())
	{
		SEISCOMP_ERROR("send::error: not connected");
		return false;
	}

	if (msg->empty())
	{
		SEISCOMP_DEBUG("send::error: empty message rejected");
		return false;
	}

	NetworkMessage *clientMsg = encode(msg, _encoding, _schemaVersion.packed);
	if ( clientMsg == NULL ) return false;

	_transmittedBytes += msg->dataSize();

	int ret = SystemConnection::send(clientMsg);

	delete clientMsg;

	if (ret !=  Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("send::error = %d", ret);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Connection::send(const std::string& groupname, Seiscomp::Core::Message* msg)
{
	if (!isConnected())
	{
		SEISCOMP_ERROR("send::error: not connected");
		return false;
	}

	if (groupname.empty())
	{
		SEISCOMP_ERROR("send::invalid groupname");
		return false;
	}

	if (msg->empty())
	{
		SEISCOMP_DEBUG("send::error: empty message rejected");
		return false;
	}

	NetworkMessage* clientMsg = encode(msg, _encoding, _schemaVersion.packed);
	if ( clientMsg == NULL ) return false;

	_transmittedBytes += msg->dataSize();

	int ret = SystemConnection::send(groupname, clientMsg);

	delete clientMsg;

	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("send::error = %d", ret);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Connection::send(NetworkMessage* msg)
{
	if (!isConnected())
	{
		SEISCOMP_ERROR("send::error: not connected");
		return false;
	}

	_transmittedBytes += msg->dataSize();

	int ret = SystemConnection::send(msg);

	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("send::error = %d", ret);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Connection::send(const std::string& groupname, NetworkMessage* msg)
{
	if (!isConnected())
	{
		SEISCOMP_ERROR("send::error: not connected");
		return false;
	}

	_transmittedBytes += msg->dataSize();

	int ret = SystemConnection::send(groupname, msg);

	if (ret != Core::Status::SEISCOMP_SUCCESS)
	{
		SEISCOMP_ERROR("send::error = %d", ret);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Message* Connection::readMessage(bool waitForNew,
                                                 ReadMode readMode,
                                                 NetworkMessage **networkMessage,
                                                 int *error)
{
	NetworkMessage *tmpMessage = NULL;
	while ( (tmpMessage = receive(waitForNew, error)) )
	{
		//SEISCOMP_DEBUG("fetched message (left in queue: %d)", queuedMessageCount());
		int type = tmpMessage->type();
		Seiscomp::Core::Message* msg = type < 0 ? NULL : dispatch(tmpMessage);
		if (networkMessage != NULL)
			*networkMessage = tmpMessage;
		else
			delete tmpMessage;

		if (msg == NULL)
		{
			if (readMode == READ_ALL)
				break;
			SEISCOMP_DEBUG("skipping unknown network message");
		}
		else
			return msg;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Message* Connection::readQueuedMessage(ReadMode readMode,
                                                       NetworkMessage **networkMessage) {
	NetworkMessage *tmpMessage = NULL;
	while ( (tmpMessage = readLocalMessage()) )
	{
		//SEISCOMP_DEBUG("fetched message (left in queue: %d)", queuedMessageCount());
		int type = tmpMessage->type();
		Seiscomp::Core::Message* msg = type < 0 ? NULL : dispatch(tmpMessage);
		if (networkMessage != NULL)
			*networkMessage = tmpMessage;
		else
			delete tmpMessage;

		if (msg == NULL)
		{
			if (readMode == READ_ALL)
				break;
			SEISCOMP_DEBUG("skipping unknown network message");
		}
		else
			return msg;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Message* Connection::dispatch(NetworkMessage* clientMsg)
{
	Seiscomp::Core::Message* msg = clientMsg->decode();
	if (msg != NULL)
		_receivedBytes += msg->dataSize();
	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Connection::transmittedBytes() const
{
	return _transmittedBytes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Connection::receivedBytes() const
{
	return _receivedBytes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Communication
} // namespace Seiscomp
