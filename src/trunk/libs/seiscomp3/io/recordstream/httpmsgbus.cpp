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


#define SEISCOMP_COMPONENT HMBConnection

#include <string>
#include <set>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/httpsocket.h>
#include <seiscomp3/io/httpsocket.ipp>

#include "httpmsgbus.h"


namespace Seiscomp {
namespace RecordStream {

typedef HMBConnection<IO::Socket> PlainHMBConnection;
typedef HMBConnection<IO::SSLSocket> SSLHMBConnection;

/* IMPLEMENT_SC_CLASS_DERIVED(PlainHMBConnection,
                           Seiscomp::IO::RecordStream,
                           "PlainHMBConnection"); */
REGISTER_RECORDSTREAM(PlainHMBConnection, "hmb");

/* IMPLEMENT_SC_CLASS_DERIVED(SSLHMBConnection,
                           Seiscomp::IO::RecordStream,
                           "SSLHMBConnection"); */
REGISTER_RECORDSTREAM(SSLHMBConnection, "hmbs");


const int BSON_SIZE_MAX = 16*1024*1024;
const int HEARTBEAT_INTERVAL = 30;
const int SOCKET_TIMEOUT = 60;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HMBQueue::HMBQueue(): _seq(-1) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HMBQueue::~HMBQueue() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HMBQueue::addStream(std::string loc, std::string cha,
	const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime)
{
	_topics.insert(loc + "_" + cha + "_D");

	if ( stime.valid() ) {
		if ( !_stime.valid() || stime < _stime ) {
			_stime = stime;
			_seq = 0;
		}
	}

	if ( etime.valid() ) {
		if ( !_etime.valid() || etime > _etime ) {
			_etime = etime;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HMBQueue::setSequenceNumber(int64_t seq) {
	_seq = seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HMBQueue::clear() {
	_topics.clear();
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bson_t* HMBQueue::toBSON() const
{
	bson_t* b = bson_new();
	bson_t t = BSON_INITIALIZER;
	bson_append_array_begin(b, "topics", -1, &t);

	int i = 0;
	for (std::set<std::string>::iterator it = _topics.begin(); it != _topics.end(); ++it)
		bson_append_utf8(&t, Core::toString(i++).c_str(), -1, it->c_str(), -1);

	bson_append_array_end(b, &t);
	bson_append_int64(b, "seq", -1, _seq);

	if ( _stime.valid() )
		bson_append_utf8(b, "starttime", -1, Core::toString(_stime).c_str(), -1);

	if ( _etime.valid() )
		bson_append_utf8(b, "endtime", -1, Core::toString(_etime).c_str(), -1);

	return b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HMBConnection<SocketType>::HMBConnection(): RecordStream(),
	_stream(std::istringstream::in|std::istringstream::binary), _readingData(false)
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HMBConnection<SocketType>::HMBConnection(std::string serverloc): RecordStream(),
	_stream(std::istringstream::in|std::istringstream::binary), _readingData(false)
{
	setSource(serverloc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HMBConnection<SocketType>::~HMBConnection() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setSource(std::string serverloc)
{
	std::string host;
	size_t pos = serverloc.find('@');

	if ( pos != std::string::npos ) {
		std::string login = serverloc.substr(0, pos);
		host = serverloc.substr(pos);
		pos = login.find(':');

		if ( pos != std::string::npos ) {
			_user = login.substr(0, pos);
			_password = login.substr(pos);
		}
		else {
			_user = login;
			_password = "";
		}
	}
	else {
		host = serverloc;
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

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setRecordType(const char* type)
{
	return !strcmp(type, "mseed");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::addStream(std::string net, std::string sta, std::string loc, std::string cha)
{
	std::pair<std::set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::addStream(std::string net, std::string sta, std::string loc, std::string cha,
	const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime)
{
	std::pair<std::set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha, stime, etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::removeStream(std::string net, std::string sta, std::string loc, std::string cha)
{
	bool deletedSomething = false;
	std::set<StreamIdx>::iterator it = _streams.begin();

	for ( ; it != _streams.end(); ) {
		if ( it->network()  == net &&
		     it->station()  == sta &&
		     it->location() == loc &&
		     it->channel()  == cha ) {
			_streams.erase(it++);
			deletedSomething = true;
		}
		else
			++it;
	}

	return deletedSomething;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setStartTime(const Seiscomp::Core::Time &stime)
{
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setEndTime(const Seiscomp::Core::Time &etime)
{
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setTimeWindow(const Seiscomp::Core::TimeWindow &w)
{
	return setStartTime(w.startTime()) && setEndTime(w.endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::setTimeout(int seconds)
{
	_sock.setTimeout(seconds);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::clear()
{
	this->~HMBConnection();
	new(this) HMBConnection(_serverHost + _serverPath);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Hopefully safe to be called from another thread
template <typename SocketType>
void HMBConnection<SocketType>::close()
{
	_sock.interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool HMBConnection<SocketType>::reconnect()
{
	if ( _sock.isOpen() )
		_sock.close();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HMBConnection<SocketType>::bsonGetString(const bson_t *bson, const char *key)
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
int64_t HMBConnection<SocketType>::bsonGetInt(const bson_t *bson, const char *key)
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
void HMBConnection<SocketType>::bsonGetBlob(const bson_t *bson, const char *key, const void **data, int *data_len)
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
void HMBConnection<SocketType>::initSession()
{
	if ( _sock.isOpen() )
		_sock.close();

	bson_t req = BSON_INITIALIZER;

	if ( _cid.length() > 0 )
		bson_append_utf8(&req, "cid", -1, _cid.c_str(), -1);

	bson_append_int32(&req, "heartbeat", -1, HEARTBEAT_INTERVAL);

	bson_t req_queue = BSON_INITIALIZER;
	bson_append_document_begin(&req, "queue", -1, &req_queue);

	std::map<std::string, HMBQueue>::iterator it;
	for ( it = _queues.begin(); it != _queues.end(); ++it ) {
		bson_t* b = it->second.toBSON();
		bson_append_document(&req_queue, it->first.c_str(), -1, b);
		bson_destroy(b);
	}

	bson_append_document_end(&req, &req_queue);

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

			std::string qname = bson_iter_key(&iQueueParam);
			std::string status;

			try {
				int64_t seq = bsonGetInt(&b, "seq");
				status += "seq=" + Core::toString<int64_t>(seq);
				_queues[qname].setSequenceNumber(seq);
			}
			catch(Core::GeneralException) {
			}

			try {
				std::string error = bsonGetString(&b, "error");
				status += error;
			}
			catch(Core::GeneralException) {
			}

			SEISCOMP_DEBUG("%s: %s", qname.c_str(), status.c_str());
		}
	}
	catch ( Core::GeneralException &e ) {
		SEISCOMP_ERROR("%s", e.what());

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
std::string HMBConnection<SocketType>::receive()
{
	while (true) {
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
			if ( _sock.isOpen() )
				_sock.close();

			_sid = "";
			throw;
		}

		try {
			bson_t bson = BSON_INITIALIZER;

			if ( !bson_init_static(&bson, (const uint8_t *) data.data(), data.length()) )
				throw Core::GeneralException("invalid BSON data");

			std::string msgtype = bsonGetString(&bson, "type");

			if ( !strcmp(msgtype.c_str(), "MSEED") ) {
				std::string qname = bsonGetString(&bson, "queue");
				int64_t seq = bsonGetInt(&bson, "seq");
				_queues[qname].setSequenceNumber(seq + 1);

				const void *cdata;
				int cdata_len;
				bsonGetBlob(&bson, "data", &cdata, &cdata_len);
				return std::string((const char *)cdata, cdata_len);
			}
			else if ( !strcmp(msgtype.c_str(), "EOF") ) {
				if ( _sock.isOpen() )
					_sock.close();

				_sid = "";
				return "";
			}
			else if ( !strcmp(msgtype.c_str(), "HEARTBEAT") ) {
				// do nothing
			}
			else {
				std::string qname = bsonGetString(&bson, "queue");
				int64_t seq = bsonGetInt(&bson, "seq");
				_queues[qname].setSequenceNumber(seq + 1);

				SEISCOMP_WARNING("ignoring non-MSEED data");
			}

		}
		catch ( Core::GeneralException &e ) {
			if ( _sock.isOpen() )
				_sock.close();

			throw Core::GeneralException("invalid message (" + std::string(e.what()) + ")");
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::istream &HMBConnection<SocketType>::stream()
{
	if ( !_readingData ) {
		_queues.clear();

		for ( std::set<StreamIdx>::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
			SEISCOMP_DEBUG("Request: %s", it->str(_stime, _etime).c_str());
			if ( !it->startTime().valid() && !_stime.valid() ) {
				/* invalid time window ignore stream */
				SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
				continue;
			}

			Core::Time stime = it->startTime().valid() ? it->startTime() : _stime;
			Core::Time etime = it->endTime().valid() ? it->endTime() : _etime;
			HMBQueue& q = _queues["WAVE_" + it->network() + "_" + it->station()];
			q.addStream(it->location(), it->channel(), stime, etime);
		}

		_readingData = true;
	}

	std::string data = receive();

	if ( data.empty() )
		_stream.clear(std::ios::eofbit);
	else
		_stream.str(data);

	return _stream;
}


}
}

