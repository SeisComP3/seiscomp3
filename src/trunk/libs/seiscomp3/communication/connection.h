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

// $Log$
// Revision 1.28  2006/10/20 15:16:01  jabe
// *** empty log message ***
//
// Revision 1.27  2006/10/20 13:20:14  thoms
// *** empty log message ***
//
// Revision 1.26  2006/10/17 09:54:41  jabe
// *** empty log message ***
//
// Revision 1.25  2006/10/16 06:41:07  jabe
// Removed <queue> include
//
// Revision 1.24  2006/10/14 12:29:52  jabe
// Made changes to use the new client interface
//
// Revision 1.23  2006/09/05 15:26:42  jabe
// *** empty log message ***
//
// Revision 1.22  2006/09/04 09:12:14  jabe
// Fixed disconnect deadlock
//
// Revision 1.21  2006/08/31 14:20:04  jabe
// *** empty log message ***
//
// Revision 1.20  2006/07/24 09:39:10  jabe
// removed message queue in Connection
// renamed Connection::dispatchMessage to Connection::readMessage
// Connection uses now the message queue from Com::Client by interface
//
// Revision 1.19  2006/07/24 07:35:42  jabe
// *** empty log message ***
//
// Revision 1.18  2006/07/19 13:13:12  jabe
// removed groupname from Connection::send
//
// Revision 1.16  2006/07/19 12:00:21  thoms
// 1) Some communication interfaces changed (Integer replaced by enums)
// 2) Archive request functionality added
// 3) Internal communication specific changes
//
// Revision 1.15  2006/07/17 10:23:04  jabe
// Renamed namespace Seiscomp to Seiscomp
// Added generated SWIG files to automake clean rules
//
// Revision 1.14  2006/07/11 06:32:40  jabe
// - dispatchMessage returns a C pointer instead a smartpointer now
// - Connection::create was renamed to Connection::Create to meet the codestyle guidelines for static methods
//
// Revision 1.13  2006/07/07 07:45:04  jabe
// *** empty log message ***
//
// Revision 1.12  2006/07/06 15:01:56  jabe
// added shutdown before returning NULL in connect if the master client does not answer the login request
//
// Revision 1.11  2006/07/06 09:58:07  jabe
// *** empty log message ***
//

#ifndef __SEISCOMP_COMMUNICATION_CONNECTION_H__
#define __SEISCOMP_COMMUNICATION_CONNECTION_H__

#include <seiscomp3/core/enumeration.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/communication/systemconnection.h>


namespace Seiscomp
{
namespace Communication
{


MAKEENUM(MessageEncoding,
	EVALUES(
		BINARY_ENCODING,
		XML_ENCODING
	),
	ENAMES(
		"binary",
		"xml"
	)
);


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(Connection);
/**
 * This class derives from SystemConnection but ads the functionality to handle
 * high level messages
 */
class SC_SYSTEM_CLIENT_API Connection : public SystemConnection
{
// ----------------------------------------------------------------------
// NESTED TYPES
// ----------------------------------------------------------------------
public:
	enum ReadMode {
		SKIP_UNKNOWN,
		READ_ALL,
		RM_QUANTITY
	};


// ----------------------------------------------------------------------
// X'STRUCTION
// ----------------------------------------------------------------------
public:
	~Connection();

private:
	Connection(NetworkInterface* networkInterface);

	
// ----------------------------------------------------------------------
// INTERFACE
// ----------------------------------------------------------------------
public:
	/**
	 * Sets the encoding for messages. Different encodings result in
	 * different message sizes and different decoding performance.
	 * Nevertheless sometimes it is more useful to use XML_ENCODING to
	 * preserve object compatibility while BINARY_ENCODING needs
	 * objects layouted exactly the same to communicate with another
	 * system.
	 * @param enc The encoding (default: BINARY_ENCODING)
	 */
	void setEncoding(MessageEncoding enc);

	/**
	 * Returns the current encoding.
	 * @return The encoding. 
	 */
	MessageEncoding encoding() const;

	/**
	 * Reads and dispatches a message if there is one.
	 * The message will be removed from the message queue.
	 * @param waitForNew Defines the behaviour if the message queue is empty.
	 *                   If 'waitForNew' is true the call blocks until a new
	 *                   message arrives.
	 * @param readMode Defines how to handle unknown messages. When a network
	 *                 message arrives it can happen that it can't be decoded
	 *                 into a highlevel message.
	 *                 SKIP_UNKNOWN: Skips unknown network messages internally
	 *                               and returns only valid messages. When NULL
	 *                               is returned, the queue is empty or an error
	 *                               occured depending on the blocking mode.
	 *                 READ_ALL: Reads all messages. When an unknown message has
	 *                           been read, NULL is returned and lastNetworkMessage()
	 *                           gives back the read network message. The application
	 *                           has to call this method until NULL is returned and
	 *                           lastNetworkMessage() returns NULL, too.
	 * @return The decoded message or NULL
	 */
	Seiscomp::Core::Message* readMessage(bool waitForNew = false,
	                                     ReadMode readMode = SKIP_UNKNOWN,
	                                     NetworkMessage **networkMessage = NULL,
	                                     int *error = NULL);

	/**
	 * Reads and dispatches a message read from the local message queue.
	 * If the queue is empty, NULL will be returned
	 * @param readMode See method readMessage
	 * @return The decoded message or NULL if the queue is empty
	 */
	Seiscomp::Core::Message* readQueuedMessage(ReadMode readMode = SKIP_UNKNOWN,
	                                           NetworkMessage **networkMessage = NULL);

	/**
	 * Send a message.
	 * @param  msg The message
	 * @param  error The optional error value returned in case false is returned
	 * @retval true The message has been send or queued properly
	 * @retval false The message has been rejected
	 */
	bool send(Seiscomp::Core::Message* msg, int *error = NULL);

	/**
	 * Send a message to the primary group.
	 * @param  msg The message
	 * @param  error The optional error value returned in case false is returned
	 * @retval true The message has been send or queued properly
	 * @retval false The message has been rejected
	 */
	bool send(NetworkMessage* msg, int *error = NULL);

	/**
	 * Send a message to a given group.
	 * @param  groupname The name of the destination group
	 * @param  msg The message
	 * @param  error The optional error value returned in case false is returned
	 * @retval true The message has been send or queued properly
	 * @retval false The message has been rejected
	 */
	bool send(const std::string& groupname, Core::Message* msg, int *error = NULL);

	/**
	 * Sends a network message.
	 * @param groupname The target group
	 * @param  msg The network message
	 * @param  error The optional error value returned in case false is returned
	 * @return The result of the request
	 */
	bool send(const std::string& groupname, NetworkMessage* msg, int *error = NULL);

	/**
	 * Connects to the message system
	 * @param addr The name of the message net (port@host)
	 * @param user The user name
	 * @param group The primary target group for messages
	 * @param priority The client priority
	 * @return A connection. NULL if no connection has been established.
	 */
	static Connection* Create(const std::string& addr,
	                          const std::string& user,
	                          const std::string& group,
	                          Protocol::ClientPriority priority = Protocol::PRIORITY_DEFAULT,
	                          int timeout = 3000,
	                          int* status = NULL);

	Core::Message* dispatch(NetworkMessage*);

	/** Returns the number of bytes sent over this
	 * connection as payload 
	 * @return number of transmitted bytes
	 */
	int transmittedBytes() const;

	/** Returns the number of bytes received through
	 * this connection as payload
	 * @return number of received bytes
	 */
	int receivedBytes() const;

	
// ----------------------------------------------------------------------
// DATA MEMEBERS
// ----------------------------------------------------------------------
private:
	MessageEncoding _encoding;
	int _transmittedBytes;
	int _receivedBytes;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp


#endif
