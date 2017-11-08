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

#ifndef __SEISCOMP_COMMUNICATION_COMMUNICATION_H__
#define __SEISCOMP_COMMUNICATION_COMMUNICATION_H__


#include <seiscomp3/core/message.h>
#include <seiscomp3/communication/protocol.h>


namespace Seiscomp {
namespace Communication {


DEFINE_SMARTPOINTER(NetworkMessage);


/**
 * \brief Network message class used by the communication interface
 */
class SC_SYSTEM_CLIENT_API NetworkMessage : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(NetworkMessage);
	DECLARE_SERIALIZATION;

	// -----------------------------------------------------------------------
	// X'struction
	// -----------------------------------------------------------------------
public:
	//! C'tor
	NetworkMessage();
	NetworkMessage(const NetworkMessage&);

	//! D'tor
	virtual ~NetworkMessage();


protected:
	NetworkMessage(int msgType);


	// ------------------------------------------------------------------------
	// Getter / Setter
	// ------------------------------------------------------------------------
public:
	//! returns the name of the client
	const std::string& privateSenderGroup() const;

	//! Sets the name of the client
	void setPrivateSenderGroup(const std::string& privateGroup);

	//! Returns the name of the client
	std::string clientName() const;

	//! The senders hostname
	std::string hostname() const;

	//! Returns the type flag of the message
	int type() const;

	//! Sets the type flag of the message
	void setType(int type);

	//! Returns the message type
	Protocol::MSG_TYPES messageType() const;

	//! Sets the message type of the message
	void setMessageType(Protocol::MSG_TYPES type);

	//! Returns the content type
	Protocol::MSG_CONTENT_TYPES contentType() const;

	//! Sets the content type of the message
	void setContentType(Protocol::MSG_CONTENT_TYPES type);

	//! Returns the group this message will be sent to.
	const std::string& destination() const;

	//! Sets the group this message will be sent to.
	void setDestination(const std::string& destinationGroup);

	//! Assigns a sequence number and a timestamp to  message instance.
	//! Note: This will be done by the master
	bool tag(int seqNum, time_t timeStamp);

	bool tagged() const;

	//! Returns the sequence number of the message
	int seqNum() const;

	//! Returns the timestamp
	time_t timestamp() const;

	//! Returns the data carried by this message
	std::string& data();

	//! Returns the data carried by this message
	const std::string& data() const;

	//! Returns the ascii data size carried by the message
	int dataSize() const;

	//! Sets the ascii data this message should be carrying
	void setData(const std::string& data);

	//! Sets binary data this message should be carrying
	void setData(const void* data, int size);

	//! Sets the size of the message
	void setSize(unsigned int size);

	//! Returns the message size in bytes
	unsigned int size() const;

	//! Deserializes the class
	virtual bool read(const char* buf, int size);

	//! Serializes the class and returns the number of
	//! written bytes
	virtual int write(char* buf, int size);

	/** Creates a copy of the current message.
	 *  NOTE: The user is responsible to delete the received copy.
	 *  @return a copy of the current msg;
	 */
	virtual NetworkMessage* copy() const;

	static NetworkMessage* Encode(Seiscomp::Core::Message*,
	                              Protocol::MSG_CONTENT_TYPES type,
	                              int schemaVersion = -1);
	Seiscomp::Core::Message* decode() const;


	// -----------------------------------------------------------------------
	// Data members
	// -----------------------------------------------------------------------
private:
	int          _type;
	std::string  _destination;
	int          _seqNum;
	unsigned int _size;
	time_t       _timeStamp;
	std::string  _data;
	std::string  _privateSenderGroup;
	//! Determines if a sequence number and a timestamp has been assigned the a
	//! articula message instance.
	int          _tagged;
};



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(ServiceMessage);

/** @brief Implementation of a service message
 *
 * Extends the base (data) message class with some data members
 * needed for service operations
 */
class SC_SYSTEM_CLIENT_API ServiceMessage : public NetworkMessage
{

	DECLARE_SC_CLASS(ServiceMessage);
	DECLARE_SERIALIZATION;


	// -----------------------------------------------------------------------
	// X'struction
	// -----------------------------------------------------------------------
public:
	ServiceMessage();

	/**
	 * Msg constructor.
	 * \param msgType Type of the message
	 */
	ServiceMessage(int msgType);

	/**
	 * Msg constructor
	 * @param msgType Type of the message
	 * @param clientType Type of the client
	 * @param clientPriority priority of the client
	 */
	ServiceMessage(int msgType,
	               Protocol::ClientType clientType,
	               Protocol::ClientPriority clientPriority);

	/**
	 * Msg constructor
	 * @param msgType Type of the message
	 * @param clientType Type of the client
	 * @param clientPriority priority of the client
	 * @param clientName Name of the client
	 */
	ServiceMessage(int msgType,
	               Protocol::ClientType clientType,
	               Protocol::ClientPriority clientPriority,
	               const std::string& clientName);

	virtual ~ServiceMessage();


	// -----------------------------------------------------------------------
	// Getter and setter
	// -----------------------------------------------------------------------
public:
	//! Returns the protocol version
	const std::string& protocolVersion() const;

	//! Sets the protocol version to be used
	void setProtocolVersion(const std::string &version);

	//! Returns the type of the sender
	Protocol::ClientType clientType() const;

	//! Sets the type of the sender
	void setClientType(Protocol::ClientType clientType);

	//! Returns the priority of the sender
	Protocol::ClientPriority clientPriority() const;

	//! Sets the priority of the sender
	void setClientPriority(Protocol::ClientPriority clientPriority);

	/** Creates a copy of the current message. NOTE: The user is responsible to
	 * delete the received copy.
	 * @return a copy of the current msg;
	 */
	virtual NetworkMessage* copy() const;

	int archiveSeqNum() const;
	void setArchiveSeqNum(int seqNum);

	time_t archiveTimestamp() const;
	void setArchiveTimestamp(time_t timestamp);

	void setPassword(const std::string& password);
	const std::string& password() const;

	void setPeerGroup(const std::string& peerGroup);
	const std::string& peerGroup() const;


	// -----------------------------------------------------------------------
	// Private data members
	// -----------------------------------------------------------------------
private:
	std::string              _protocolVersion;
	Protocol::ClientType     _clientType;
	Protocol::ClientPriority _clientPriority;
	int                      _archiveSeqNum;
	time_t                   _archiveTimestamp;
	std::string              _password;
	std::string              _peerGroup;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp

#endif

