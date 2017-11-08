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

#define SEISCOMP_COMPONENT Communication

#include "systemmessages.h"

#include <exception>
#include <string>
#include <iosfwd>
#include <iostream>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/archive/bsonarchive.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>



namespace Seiscomp
{
namespace Communication
{

namespace
{

template<typename Ch>
class buffer_sink
{
public:
	typedef Ch char_type;
	typedef boost::iostreams::sink_tag category;

	buffer_sink(char_type* buf, std::streamsize length, std::streamsize* pos = NULL)
			: _begin(buf), _end(buf + length), _pos(pos)
	{
		if ( _pos )
			*_pos = 0;
	}

	std::streamsize write(const char_type* s, std::streamsize n)
	{
		std::streamsize left = _end-_begin;
		if ( n > left )
		{
			if ( _pos )
				*_pos = -1;
			throw Seiscomp::Core::OverflowException("message does not fit into the buffer");
		}

		memcpy(_begin, s, n*sizeof(char_type));
		_begin += n;
		if ( _pos )
			*_pos += n;
		return n;
	}

private:
	char_type* _begin;
	char_type* _end;
	std::streamsize* _pos;
};

}




IMPLEMENT_SC_CLASS(NetworkMessage, "NetworkMessage");
IMPLEMENT_SC_CLASS(ServiceMessage, "ServiceMessage");




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage::NetworkMessage()
		: _type(Protocol::DATA_MSG),
		_seqNum(0),
		_size(0),
		_timeStamp(0),
		_data(""),
		_privateSenderGroup(""),
		_tagged(false)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage::NetworkMessage(const NetworkMessage& msg)
{
	*this = msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage::~NetworkMessage()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage::NetworkMessage(int msgType) :
		_type(msgType),
		_seqNum(0),
		_size(0),
		_timeStamp(0),
		_data(""),
		_privateSenderGroup(""),
		_tagged(false)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& NetworkMessage::privateSenderGroup() const
{
	return _privateSenderGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setPrivateSenderGroup(const std::string& privateGroup)
{
	_privateSenderGroup = privateGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string NetworkMessage::clientName() const
{
	std::vector<std::string> tokens;
	int ret = Core::split(tokens, _privateSenderGroup.c_str(), "#");
	return (ret > 1) ? tokens[1] : tokens[0];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string NetworkMessage::hostname() const {
	std::vector<std::string> tokens;
	int ret = Core::split(tokens, _privateSenderGroup.c_str(), "#");
	return ret > 0 ? tokens.back(): "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NetworkMessage::type() const
{
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setType(int type)
{
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& NetworkMessage::destination() const
{
	return _destination;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setDestination(const std::string& destinationGroup)
{
	_destination = destinationGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NetworkMessage::tag(int seqNum, time_t timeStamp)
{
	if ( !_tagged  ) {
		_seqNum = seqNum;
		_timeStamp = timeStamp;
		_tagged = !_tagged;
	}
	return _tagged;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NetworkMessage::tagged() const
{
	return _tagged;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NetworkMessage::seqNum() const
{
	return _seqNum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
time_t NetworkMessage::timestamp() const
{
	return _timeStamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string& NetworkMessage::data()
{
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& NetworkMessage::data() const
{
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NetworkMessage::dataSize() const
{
	return _data.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setData(const std::string& data)
{
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setData(const void* data, int size)
{
	setSize(size);
	_data.resize(size);

	const char* d = (const char*)data;
	for ( int i = 0; i < size; ++i )
		_data[i] = d[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setSize(unsigned int size)
{
	_size = size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned int NetworkMessage::size() const
{
	return _size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NetworkMessage::read(const char* buf, int size)
{
	try
	{
		boost::iostreams::stream_buffer<boost::iostreams::array_source> sb(buf, size);
		IO::BinaryArchive ar(&sb, true);
		serialize(ar);
		return true;
	}
	catch ( std::exception& e )
	{
		SEISCOMP_ERROR("Decoding message: %s, datasize = %d", e.what(), size);
		return false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NetworkMessage::write(char* buf, int size)
{
	std::streamsize pos;
	boost::iostreams::stream_buffer< buffer_sink<char> > sb(buf, size, &pos);
	IO::BinaryArchive ar(&sb, false);
	serialize(ar);
	sb.close();
	return (int)pos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* NetworkMessage::copy() const
{
	return new NetworkMessage(*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::MSG_TYPES NetworkMessage::messageType() const {
	if ( _type < 0 ) throw Core::GeneralException("ServiceMessages has no MSG_TYPES entry");
	return static_cast<Protocol::MSG_TYPES>(_type & 0x7F);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setMessageType(Protocol::MSG_TYPES type) {
	_type = (_type & ~0x7F) | type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::MSG_CONTENT_TYPES NetworkMessage::contentType() const {
	if ( _type < 0 ) throw Core::GeneralException("ServiceMessages have no content type");
	return static_cast<Protocol::MSG_CONTENT_TYPES>(_type >> 0x08);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::setContentType(Protocol::MSG_CONTENT_TYPES type) {
	if ( _type < 0 ) throw Core::GeneralException("ServiceMessages cannot have a content type");
	_type = (_type & 0x7F) | (type << 0x08);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* NetworkMessage::Encode(Seiscomp::Core::Message* msg,
                                       Protocol::MSG_CONTENT_TYPES type,
                                       int schemaVersion)
{
	NetworkMessage *nm = new NetworkMessage(Protocol::DATA_MSG);
	std::string &data = nm->data();

	try
	{
		nm->setContentType(type);

		boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(data);
		boost::iostreams::filtering_ostreambuf filtered_buf;
		filtered_buf.push(boost::iostreams::zlib_compressor());
		filtered_buf.push(buf);

		switch ( type )
		{
			case Protocol::CONTENT_BINARY:
			{
				IO::VBinaryArchive ar(&filtered_buf, false, schemaVersion);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_XML:
			{
				IO::XMLArchive ar(&filtered_buf, false, schemaVersion);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_XML:
			{
				boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(data);

				IO::XMLArchive ar(&buf, false, schemaVersion);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_IMPORTED_XML:
			{
				boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(data);

				IO::XMLArchive ar;
				ar.setRootName("");
				if ( ar.create(&buf) ) {
					ar << msg;
					if ( !ar.success() )
						throw Core::GeneralException("failed to serialize archive");
				}
				else
					throw Core::GeneralException("encode: unable to create imported XML stream");
			}
				break;

			case Protocol::CONTENT_BSON:
			{
				IO::BSONArchive ar(&filtered_buf, false, schemaVersion);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_BSON:
			{
				boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(data);

				IO::BSONArchive ar(&buf, false, schemaVersion);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_JSON:
			{
				IO::BSONArchive ar(&filtered_buf, false, schemaVersion);
				ar.setJSON(true);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_JSON:
			{
				boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(data);

				IO::BSONArchive ar(&buf, false, schemaVersion);
				ar.setJSON(true);
				ar << msg;
				if ( !ar.success() )
					throw Core::GeneralException("failed to serialize archive");
			}
				break;

			default:
				throw Core::GeneralException("encode: unknown message content type");
		};

		msg->setDataSize(data.size());
	}
	catch ( std::exception& e )
	{
		SEISCOMP_ERROR("%s", e.what());
		delete nm;
		nm = NULL;
		msg->setDataSize(0);
	}

	return nm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Message* NetworkMessage::decode() const
{
	Seiscomp::Core::Message* msg = NULL;
	Protocol::MSG_CONTENT_TYPES cType = contentType();

	try {

		boost::iostreams::filtering_istreambuf filtered_buf;
		boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(data().c_str(), data().size());
		filtered_buf.push(boost::iostreams::zlib_decompressor());
		filtered_buf.push(buf);

		switch ( cType )
		{
			case Protocol::CONTENT_BINARY:
			{
				IO::VBinaryArchive ar(&filtered_buf, true);
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_XML:
			{
				IO::XMLArchive ar(&filtered_buf, true);
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_XML:
			{
				boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(data().c_str(), data().size());

				IO::XMLArchive ar(&buf, true);
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_IMPORTED_XML:
			{
				boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(data().c_str(), data().size());

				IO::XMLArchive ar;
				ar.setRootName("");
				if ( !ar.open(&buf) )
					throw Core::GeneralException("decode: invalid imported XML stream");
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_BSON:
			{
				IO::BSONArchive ar;
				if ( !ar.open(&filtered_buf) )
					throw Core::GeneralException("decode: invalid compressed BSON content");
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_BSON:
			{
				boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(data().c_str(), data().size());

				IO::BSONArchive ar;
				if ( !ar.open(&buf) )
					throw Core::GeneralException("decode: invalid BSON content");
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_JSON:
			{
				IO::BSONArchive ar;
				ar.setJSON(true);
				if ( !ar.open(&filtered_buf) )
					throw Core::GeneralException("decode: invalid compressed JSON content");
				ar >> msg;
			}
				break;

			case Protocol::CONTENT_UNCOMPRESSED_JSON:
			{
				boost::iostreams::stream_buffer<boost::iostreams::array_source> buf(data().c_str(), data().size());

				IO::BSONArchive ar;
				ar.setJSON(true);
				if ( !ar.open(&buf) )
					throw Core::GeneralException("decode: invalid JSON content");
				ar >> msg;
			}
				break;

			default:
				throw Core::GeneralException("decode: unknown message content type");
		};

		if ( msg != NULL )
			msg->setDataSize(data().size());

	}
	catch ( std::exception& e ) {
		SEISCOMP_ERROR("message (%s -> %s): decoding failed: %s",
		               _privateSenderGroup.c_str(),
		               _destination.c_str(), e.what());
		if ( msg ) {
			delete msg;
			msg = NULL;
		}
	}

	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NetworkMessage::serialize(Archive& ar)
{
	ar & TAGGED_MEMBER(type);
	ar & TAGGED_MEMBER(destination);
	ar & TAGGED_MEMBER(seqNum);
	ar & TAGGED_MEMBER(timeStamp);
	ar & TAGGED_MEMBER(data);
	ar & TAGGED_MEMBER(privateSenderGroup);
	ar & TAGGED_MEMBER(tagged);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServiceMessage::ServiceMessage() :
		NetworkMessage(Protocol::UNDEFINED_MSG),
		_protocolVersion(Protocol::PROTOCOL_VERSION_V1_0),
		_clientType(Protocol::TYPE_ONE),
		_clientPriority(Protocol::PRIORITY_HIGH),
		_archiveSeqNum(0), _archiveTimestamp(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServiceMessage::ServiceMessage(int msgType) :
		NetworkMessage(msgType),
		_protocolVersion(Protocol::PROTOCOL_VERSION_V1_0),
		_clientType(Protocol::TYPE_ONE),
		_clientPriority(Protocol::PRIORITY_HIGH),
		_archiveSeqNum(0), _archiveTimestamp(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServiceMessage::ServiceMessage(int msgType,
               Protocol::ClientType clientType,
               Protocol::ClientPriority clientPriority) :
		NetworkMessage(msgType),
		_protocolVersion(Protocol::PROTOCOL_VERSION_V1_0),
		_clientType(clientType),
		_clientPriority(clientPriority),
		_archiveSeqNum(0), _archiveTimestamp(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServiceMessage::ServiceMessage(int msgType,
               Protocol::ClientType clientType,
               Protocol::ClientPriority clientPriority,
               const std::string& clientName) :
		NetworkMessage(msgType),
		_protocolVersion(Protocol::PROTOCOL_VERSION_V1_0),
		_clientType(clientType),
		_clientPriority(clientPriority),
		_archiveSeqNum(0), _archiveTimestamp(0)
{
	setPrivateSenderGroup(clientName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServiceMessage::~ServiceMessage() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ServiceMessage::protocolVersion() const
{
	return _protocolVersion;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setProtocolVersion(const std::string &version)
{
	_protocolVersion = version;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::ClientType ServiceMessage::clientType() const
{
	return _clientType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setClientType(Protocol::ClientType clientType)
{
	_clientType = clientType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::ClientPriority ServiceMessage::clientPriority() const
{
	return _clientPriority;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setClientPriority(Protocol::ClientPriority clientPriority)
{
	_clientPriority	= clientPriority;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* ServiceMessage::copy() const
{
	ServiceMessage* copy = new ServiceMessage();

	// Use elementwise copying of the compiler generated assignement operator
	*copy = *this;

	return copy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ServiceMessage::archiveSeqNum() const
{
	return _archiveSeqNum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setArchiveSeqNum(int seqNum)
{
	_archiveSeqNum = seqNum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
time_t ServiceMessage::archiveTimestamp() const
{
	return _archiveTimestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setArchiveTimestamp(time_t timestamp)
{
	_archiveTimestamp = timestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setPassword(const std::string& password)
{
	_password = password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ServiceMessage::password() const
{
	return _password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::setPeerGroup(const std::string& peerGroup)
{
	_peerGroup = peerGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ServiceMessage::peerGroup() const
{
	return _peerGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ServiceMessage::serialize(Archive& ar)
{
	// serialize base class information
	NetworkMessage::serialize(ar);

	int prio = _clientPriority;
	int type = _clientType;

	ar & TAGGED_MEMBER(protocolVersion);
	ar & NAMED_OBJECT("clientType", type);
	ar & NAMED_OBJECT("clientPriority", prio);
	ar & TAGGED_MEMBER(archiveSeqNum);
	ar & TAGGED_MEMBER(archiveTimestamp);
	ar & TAGGED_MEMBER(password);
	ar & TAGGED_MEMBER(peerGroup);

	_clientPriority = (Protocol::ClientPriority)prio;
	_clientType = (Protocol::ClientType)type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Communication
} // namespace Seiscomp
