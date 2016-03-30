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
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/datamodel/version.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/httpsocket.h>
#include <seiscomp3/io/httpsocket.ipp>

#include "httpdriver.h"

#include <memory.h>


namespace Seiscomp {
namespace Communication {


typedef HttpDriver<IO::Socket> PlainHttpDriver;
typedef HttpDriver<IO::SSLSocket> SSLHttpDriver;

REGISTER_NETWORK_INTERFACE(PlainHttpDriver, "hmb");
REGISTER_NETWORK_INTERFACE(SSLHttpDriver, "hmbs");

const int BSON_SIZE_MAX = 16*1024*1024;
const int HEARTBEAT_INTERVAL = 30;
const int SOCKET_TIMEOUT = 60;
const std::string GROUPS = "AMPLITUDE,PICK,LOCATION,MAGNITUDE,EVENT,QC,PUBLICATION,GUI,INVENTORY,CONFIG,LOGGING";


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HttpDriver<SocketType>::HttpDriver(): _seq(-1), _isConnected(false)
{
	_sock.setTimeout(SOCKET_TIMEOUT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HttpDriver<SocketType>::~HttpDriver()
{
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpDriver<SocketType>::bsonGetString(const bson_t *bson, const char *key)
{
	bson_iter_t iter;
	if ( bson_iter_init_find(&iter, bson, key) ) {
		if ( bson_iter_type(&iter) == BSON_TYPE_UTF8 ) {
			uint32_t value_len;
			const char *value = bson_iter_utf8(&iter, &value_len);
			return std::string(value, value_len);
		}

		throw Core::GeneralException((std::string("invalid ") + key).c_str());
	}

	throw Core::GeneralException((std::string("missing ") + key).c_str());

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int64_t HttpDriver<SocketType>::bsonGetInt(const bson_t *bson, const char *key)
{
	bson_iter_t iter;

	if ( bson_iter_init_find(&iter, bson, key) ) {
		switch ( bson_iter_type(&iter) ) {
			case BSON_TYPE_INT32:
				return bson_iter_int32(&iter);

			case BSON_TYPE_INT64:
				 return bson_iter_int64(&iter);

			default:
				throw Core::GeneralException((std::string("invalid ") + key).c_str());
		}
	}

	throw Core::GeneralException((std::string("missing ") + key).c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpDriver<SocketType>::bsonGetBlob(const bson_t *bson, const char *key, const void **data, int *data_len)
{
	bson_iter_t iter;

	if ( bson_iter_init_find(&iter, bson, key) ) {
		uint32_t value_len;
		const uint8_t *value;

		switch ( bson_iter_type(&iter) ) {
			case BSON_TYPE_DOCUMENT:
				bson_iter_document(&iter, &value_len, &value);
				break;

			case BSON_TYPE_BINARY:
				bson_iter_binary(&iter, NULL, &value_len, &value);
				break;

			default:
				throw Core::GeneralException((std::string("invalid ") + key).c_str());
		}

		*data = (const void *) value;
		*data_len = (int) value_len;
		return;
	}

	throw Core::GeneralException((std::string("missing ") + key).c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int HttpDriver<SocketType>::connect(
		const std::string& serverAddress,
		const std::string& clientName) {

	std::string host;
	size_t pos = serverAddress.find('@');

	if ( pos != std::string::npos ) {
		std::string login = serverAddress.substr(0, pos);
		host = serverAddress.substr(pos + 1);
		pos = login.find(':');

		if ( pos != std::string::npos ) {
			_user = login.substr(0, pos);
			_password = login.substr(pos + 1);
		}
		else {
			_user = login;
			_password = "";
		}
	}
	else {
		host = serverAddress;
		_user = "";
		_password = "";
	}

	pos = host.find('/');

	if ( pos != std::string::npos ) {
		_serverHost = host.substr(0, pos);
		_serverPath = host.substr(pos);

		if ( *_serverPath.rbegin() != '/' )
			_serverPath += '/';
	}
	else {
		_serverHost = host;
		_serverPath = "/";
	}

	_cid = clientName;
	_isConnected = true;

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpDriver<SocketType>::initSession()
{
	if ( _sock.isOpen() )
		_sock.close();

	bson_t req = BSON_INITIALIZER;

	if ( _cid.length() > 0 )
		bson_append_utf8(&req, "cid", -1, _cid.c_str(), -1);

	bson_append_int32(&req, "heartbeat", -1, HEARTBEAT_INTERVAL);

	bson_t queue = BSON_INITIALIZER;
	bson_t queueParam = BSON_INITIALIZER;
	bson_t topics = BSON_INITIALIZER;
	bson_append_document_begin(&req, "queue", -1, &queue);
	bson_append_array_begin(&queueParam, "topics", -1, &topics);

	int i = 0;
	for (std::set<std::string>::iterator it = _msgGroups.begin(); it != _msgGroups.end(); ++it)
		bson_append_utf8(&topics, Core::toString(i++).c_str(), -1, it->c_str(), -1);

	bson_append_array_end(&queueParam, &topics);
	bson_append_int64(&queueParam, "seq", -1, _seq);
	bson_append_document(&queue, "SC3MSG", -1, &queueParam);
	bson_destroy(&queueParam);
	bson_append_document_end(&req, &queue);

	bson_t ack = BSON_INITIALIZER;
	IO::HttpSocket<SocketType> sock;

	try {
		sock.setTimeout(SOCKET_TIMEOUT);
		sock.startTimer();
		sock.open(_serverHost, _user, _password);
		sock.httpPost(_serverPath + "open", std::string((char *) bson_get_data(&req), req.len));
		sock.startTimer();
		std::string data = sock.httpRead(4);
		int size;
		memcpy(&size, data.c_str(), 4);
		size = BSON_UINT32_FROM_LE(size);

		SEISCOMP_DEBUG("BSON size (ack): %d", size);

		if ( size > BSON_SIZE_MAX )
			throw Core::GeneralException("invalid BSON size (ack)");

		sock.startTimer();
		data += sock.httpRead(size - 4);

		if ( !bson_init_static(&ack, (const uint8_t *) data.data(), data.length()) )
			throw Core::GeneralException("invalid BSON data (ack)");

		_sid = bsonGetString(&ack, "sid");
		_cid = bsonGetString(&ack, "cid");

		SEISCOMP_INFO("HMB session opened with sid=%s, cid=%s", _sid.c_str(), _cid.c_str());

		bson_iter_t iQueue;
		bson_iter_t iQueueParam;

		if ( !bson_iter_init_find(&iQueue, &ack, "queue") ||
				!bson_iter_recurse(&iQueue, &iQueueParam) )
			throw Core::GeneralException("invalid ack");

		while ( bson_iter_next(&iQueueParam) ) {
			if ( bson_iter_type(&iQueueParam) != BSON_TYPE_DOCUMENT )
				throw Core::GeneralException("invalid ack");

			bson_t b;
			uint32_t len;
			const uint8_t *data;

			bson_iter_document(&iQueueParam, &len, &data);

			if ( !bson_init_static (&b, data, len) )
				throw Core::GeneralException("invalid ack");

			std::string queue = bson_iter_key(&iQueueParam);
			std::string status;

			try {
				int64_t seq = bsonGetInt(&b, "seq");
				status += "seq=" + Core::toString<int64_t>(seq);
				_seq = seq;
			}
			catch(Core::GeneralException) {
			}

			try {
				std::string error = bsonGetString(&b, "error");
				status += error;
			}
			catch(Core::GeneralException) {
			}

			SEISCOMP_INFO("%s: %s", queue.c_str(), status.c_str());
		}
	}
	catch ( Core::GeneralException &e ) {
		bson_destroy(&req);

		if ( sock.isOpen() )
			sock.close();

		throw;
	}

	bson_destroy(&req);
	sock.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int HttpDriver<SocketType>::disconnect()
{
	_sid = "";
	_isConnected = false;

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
NetworkMessage* HttpDriver<SocketType>::receive(int* error)
{
	if ( !_isConnected ) {
		SEISCOMP_ERROR("not connected");

		if ( error )
			*error = Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

		return NULL;
	}

	if ( _fakemsgs.size() > 0 ) {
		NetworkMessage* msg = _fakemsgs.front();
		_fakemsgs.pop_front();
		return msg;
	}

	std::string data;

	try {
		if ( _sid.length() == 0 )
			initSession();

		if ( !_sock.isOpen() ) {
			_sock.startTimer();
			_sock.open(_serverHost, _user, _password);
			_sock.httpGet(_serverPath + "stream/" + _sid);
		}

		_sock.startTimer();
		data = _sock.httpRead(4);
		int size;
		memcpy(&size, data.c_str(), 4);
		size = BSON_UINT32_FROM_LE(size);

		SEISCOMP_DEBUG("BSON size: %d", size);

		if ( size > BSON_SIZE_MAX )
			throw Core::GeneralException("invalid BSON size");

		_sock.startTimer();
		data += _sock.httpRead(size - 4);
	}
	catch ( Core::GeneralException &e ) {
		SEISCOMP_ERROR("%s", e.what());

		if ( error )
			*error = Core::Status::SEISCOMP_NETWORKING_ERROR;

		if ( _sock.isOpen() )
			_sock.close();

		_sid = "";
		return NULL;
	}

	try {
		bson_t bson = BSON_INITIALIZER;

		if ( !bson_init_static(&bson, (const uint8_t *) data.data(), data.length()) ) {
			SEISCOMP_ERROR("invalid BSON data");

			if ( error )
				*error = Core::Status::SEISCOMP_NETWORKING_ERROR;

			if ( _sock.isOpen() )
				_sock.close();

			return NULL;
		}

		std::string msgtype = bsonGetString(&bson, "type");

		if ( !strcmp(msgtype.c_str(), "HEARTBEAT") || !strcmp(msgtype.c_str(), "EOF") ) {
			if ( error )
				*error = Core::Status::SEISCOMP_SUCCESS;

			return NULL;
		}

		int64_t seq = bsonGetInt(&bson, "seq");

		// message successfully received, so point seq to the next message
		_seq = seq + 1;

		if ( strcmp(msgtype.c_str(), "SC3") ) {
			SEISCOMP_WARNING("ignoring non-SC3 message");

			if ( error )
				*error = Core::Status::SEISCOMP_SUCCESS;

			return NULL;
		}

		std::string topic = bsonGetString(&bson, "topic");
		std::string sender = bsonGetString(&bson, "sender");
		Protocol::MSG_TYPES scMessageType = Protocol::MSG_TYPES(bsonGetInt(&bson, "scMessageType"));
		Protocol::MSG_CONTENT_TYPES scContentType = Protocol::MSG_CONTENT_TYPES(bsonGetInt(&bson, "scContentType"));

		const void *cdata;
		int cdata_len;
		bsonGetBlob(&bson, "data", &cdata, &cdata_len);

		NetworkMessage* msg = new NetworkMessage();
		msg->setDestination(topic);
		msg->setPrivateSenderGroup(sender);
		msg->setMessageType(scMessageType);
		msg->setContentType(scContentType);
		msg->setData(cdata, cdata_len);

		_lastSender = sender;

		return msg;
	}
	catch ( Core::GeneralException &e ) {
		SEISCOMP_ERROR("invalid message (%s)", e.what());

		if ( error )
			*error = Core::Status::SEISCOMP_NETWORKING_ERROR;

		if ( _sock.isOpen() )
			_sock.close();

		return NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int HttpDriver<SocketType>::send(const std::string& group, int type, NetworkMessage* msg, bool selfDiscard)
{
	if ( !_isConnected ) {
		SEISCOMP_ERROR("not connected");
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;
	}

	if ( type == Protocol::CONNECT_GROUP_MSG ) {  // generate fake response
		ServiceMessage* sm = static_cast<ServiceMessage*>(msg);
		ServiceMessage* ack = new ServiceMessage(Protocol::CONNECT_GROUP_OK_MSG);
		ack->setDestination(sm->privateSenderGroup());
		ack->setProtocolVersion(sm->protocolVersion());

		if ( sm->protocolVersion() == Protocol::PROTOCOL_VERSION_V1_0 )
			ack->setData(GROUPS);
		else
			ack->setData(std::string(Protocol::HEADER_SERVER_VERSION_TAG) + ": " +
					Core::CurrentVersion.toString() + "\n" +
					Protocol::HEADER_GROUP_TAG + ": " +
					GROUPS + "\n" +
					Protocol::HEADER_SCHEMA_VERSION_TAG + ": " +
					Core::toString(DataModel::Version::Major) + "." +
					Core::toString(DataModel::Version::Minor));

		_fakemsgs.push_back(ack);

		return Core::Status::SEISCOMP_SUCCESS;
	}
	else if ( type == Protocol::CLIENT_DISCONNECTED_MSG ) {
		disconnect();

		if ( _sock.isOpen() )
			_sock.interrupt();

		return Core::Status::SEISCOMP_SUCCESS;
	}
	else if ( type <= 0 ) {
		SEISCOMP_DEBUG("discarding %s", Protocol::MsgTypeToString(type));
		return Core::Status::SEISCOMP_SUCCESS;
	}

	const std::string& destination = msg->destination();
	bool converted = false;

	// Force uncompressed BSON -- compression should be done on the HTTP level
	if ( msg->contentType() != Protocol::CONTENT_UNCOMPRESSED_BSON ) {
		try {
			bool reg = DataModel::PublicObject::IsRegistrationEnabled();
			DataModel::PublicObject::SetRegistrationEnabled(false);
			Core::MessagePtr messagePtr = msg->decode();
			DataModel::PublicObject::SetRegistrationEnabled(reg);

			if ( messagePtr == NULL ) {
				SEISCOMP_ERROR("message decoding failed");
				return Core::Status::SEISCOMP_FAILURE;
			}

			msg = Communication::NetworkMessage::Encode(
					messagePtr.get(), Communication::Protocol::CONTENT_UNCOMPRESSED_BSON);

			converted = true;
		}
		catch ( Core::GeneralException &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return Core::Status::SEISCOMP_FAILURE;
		}
	}

	bson_t bson = BSON_INITIALIZER;
	bson_append_utf8(&bson, "type", -1, "SC3", -1);
	bson_append_utf8(&bson, "queue", -1, "SC3MSG", -1);
	bson_append_utf8(&bson, "topic", -1, destination.c_str(), -1);
	bson_append_int32(&bson, "scMessageType", -1, msg->messageType());
	bson_append_int32(&bson, "scContentType", -1, msg->contentType());

	uint32_t size = BSON_UINT32_FROM_LE(*((u_int32_t *)msg->data().data()));

	if ( size > msg->dataSize() ) {
		SEISCOMP_ERROR("invalid BSON message");
		return Core::Status::SEISCOMP_FAILURE;
	}
	else if ( size != msg->dataSize() ) {
		SEISCOMP_DEBUG("NetworkMessage has trailing garbage: msg->dataSize() = %d, BSON size = %d", (int) msg->dataSize(), (int) size);
	}

	bson_t bmsg = BSON_INITIALIZER;

	if ( !bson_init_static(&bmsg, (const uint8_t *) msg->data().data(), size) ) {
		SEISCOMP_ERROR("invalid BSON message");
		bson_destroy(&bson);

		if ( converted )
			delete msg;

		return Core::Status::SEISCOMP_FAILURE;
	}

	bson_append_document(&bson, "data", -1, &bmsg);

	if ( converted )
		delete msg;

	std::string data((char *) bson_get_data(&bson), bson.len);
	bson_destroy(&bson);

	IO::HttpSocket<SocketType> sock;

	try {
		if ( _sid.length() == 0 )
			initSession();

		sock.setTimeout(SOCKET_TIMEOUT);
		sock.startTimer();
		sock.open(_serverHost, _user, _password);
		sock.httpPost(_serverPath + "send/" + _sid, data);
		sock.httpRead(1024);
		sock.close();
	}
	catch ( Core::GeneralException &e ) {
		SEISCOMP_ERROR("%s", e.what());

		if ( sock.isOpen() )
			sock.close();

		_sid = "";

		return Core::Status::SEISCOMP_NETWORKING_ERROR;
	}

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int HttpDriver<SocketType>::subscribe(const std::string& group)
{
	if ( !_isConnected ) {
		SEISCOMP_ERROR("not connected");
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;
	}

	_msgGroups.insert(group);

	_sid = "";

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int HttpDriver<SocketType>::unsubscribe(const std::string& group)
{
	if ( !_isConnected ) {
		SEISCOMP_ERROR("not connected");
		return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;
	}

	_msgGroups.erase(group);

	_sid = "";

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HttpDriver<SocketType>::poll(int* error)
{
	if ( !_isConnected ) {
		SEISCOMP_ERROR("not connected");

		if ( error )
			*error = Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

		return false;
	}

	if ( _fakemsgs.size() > 0 )
		return true;

	try {
		if ( _sid.length() == 0 )
			initSession();

		if ( !_sock.isOpen() ) {
			_sock.startTimer();
			_sock.open(_serverHost, _user, _password);
			_sock.httpGet(_serverPath + "stream/" + _sid);
		}

		if ( error )
			*error = Core::Status::SEISCOMP_SUCCESS;

		return (_sock.poll() > 0);
	}
	catch ( Core::GeneralException &e ) {
		SEISCOMP_ERROR("%s", e.what());

		if ( error )
			*error = Core::Status::SEISCOMP_NETWORKING_ERROR;

		if ( _sock.isOpen() )
			_sock.close();

		_sid = "";

		return false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HttpDriver<SocketType>::isConnected()
{
	return _isConnected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpDriver<SocketType>::privateGroup() const
{
	return _cid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpDriver<SocketType>::groupOfLastSender() const
{
	return _lastSender;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpDriver<SocketType>::setSequenceNumber(int64_t seq)
{
	_sid = "";
	_seq = seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
int64_t HttpDriver<SocketType>::getSequenceNumber() const
{
	return _seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp
