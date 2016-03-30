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

#ifndef __SEISCOMP_COMMUNICATION_SYSTEMCONNECTION_H__
#define __SEISCOMP_COMMUNICATION_SYSTEMCONNECTION_H__

#include <string>
#include <vector>
#include <queue>
#include <set>

#include <boost/thread/thread.hpp>

#include <seiscomp3/core/status.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/core/version.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/communication/systemmessages.h>


namespace Seiscomp {
namespace Communication {

class ConnectionInfo;
class NetworkInterface;
struct MessageStat;

DEFINE_SMARTPOINTER(NetworkInterface);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(SystemConnection);
/** This class offers a general interface to the network layer on system
 * level. In particular, NetworkMessages will be used for the message
 * handling.
 */
class SC_SYSTEM_CLIENT_API SystemConnection : public Seiscomp::Core::BaseObject {
	
	// ----------------------------------------------------------------------
	// NESTED TYPES
	// ----------------------------------------------------------------------
	public:
		enum ListenMode {
			NON_THREADED,
			THREADED,
			LM_QUANTITY
		};
	
	// ----------------------------------------------------------------------
	// X'STRUCTION
	// ----------------------------------------------------------------------
	public:
		/** Protected Constructor
		 * @param networInterface Pointer to a specific network implementation. If
		 *                        NULL is passed, a default implementation will be
		 *                        used.
		 */
		SystemConnection(NetworkInterface* networkInterface = NULL);

		/** Destructor
		 */
		~SystemConnection();


	// -----------------------------------------------------------------------
	// PUBLIC COMMUNICATION API
	// -------------------------- --------------------------------------------
	public:
		/** Connect to the messaging system. This includes a connnection to spread
		 * as well as to the master client.
		 * @param masterAddress Address of the server
		 * @param clientName Name of the client
		 * @param peerGroup Name of the group this client is a member of
		 * @param type The type of the client
		 * @param priority The priority of the client
		 * @param timeOut The timespan this client tries to connect to the server
		 * @return SEISCOMP_SUCCESS on success
		 */
		int connect(const std::string& masterAddress,
					const std::string& clientName,
					const std::string& peerGroup,
					Protocol::ClientType type = Protocol::TYPE_DEFAULT,
					Protocol::ClientPriority priority = Protocol::PRIORITY_DEFAULT,
					int timeOut = 3000);

		/** Reconnects to the master client in case of a connection interruption.
		 * @return SEISCOMP_SUCCESS on success
		 */
		int reconnect();

		/** Notfies all other clients with a CLIENT_DISCONNECTED_MSG message and
		 * disconnects from the messaging system.
		 */
		int disconnect();

		/** Queries the connection state
		 * @return connection state
		 */
		bool isConnected();

		/** Subscibes to a message group
		 * @param group The message group to be subscribed
		 * @return SEISCOMP_SUCCESS on success
		 */
		int subscribe(const std::string& group);

		/** Unsubscibes to a message group
		 * @param group The message group to be unsubscribed
		 * @return SEISCOMP_SUCCESS on success
		 */
		int unsubscribe(const std::string& group);

		/** Subscribes to an achrive message group
		 * @param group Group to be subscribed
		 * @return SEISCOMP_SUCCESS on success
		 */
		int subscribeArchive(const std::string& group);

		/** Unsubscribes to an achrive message group
		 * @param group Group to be unsubscribed
		 * @return SEISCOMP_SUCCESS on success
		 */
		int unsubscribeArchive(const std::string& group);

		/** Sends an archive request to the messaging system.
		 * NOTE: This can be done only after the connect call
		 *       and only once.
		 * @return SEISCOMP_SUCESS on success
		 */
		int archiveRequest();

		/** Reads the next available message either from the local message queue or
		 * from the messaging system.
		 * @param blocking If no message is available and blocking == true the call
		 *                 will block until a new message arrives
		 *                 else it returns NULL. In case NULL is returened make sure
		 *                 that the connection is still established. Otherwise call
		 *                 reconnect.
		 * @return A new message or NULL
		 */
		NetworkMessage* receive(bool blocking = true, int *error = NULL);

		/** Reads a message from the network into the local message queue.
		 * @param blocking specifies if the call is blocking
		 * @return SEISCOMP_SUCCESS on success
		 */
		int readNetworkMessage(bool blocking = true);

		/**
		 * Reads the and returns the first message in the message queue
		 * @return The message or NULL.
		 */
		NetworkMessage* readLocalMessage();

		/** Sends a message. The destination etc. has to be specified in the message.
		 * @param msg Message to be sent
		 * @return SEISCOMP_SUCCESS on success
		 */
		int send(NetworkMessage* msg);

		/** Sends a message
		 * @param groupname Destination of the message
		 * @param msg Message to be sent
		 * @return SEISCOMP_SUCCESS on success
		 */
		int send(const std::string& groupname, NetworkMessage* msg);

		/** Determines whether a new message is ready to be read from the network
		 * @return true if a new message arrived
		 */
		bool poll() const;

		/**
		 * Sets the connection to listening state. If no thread is used to
		 * listen to and dispatch new messages the application will block
		 * until stop() or disconnect() is called.
		 * stop() works only after receiving a message. If no message
		 * arrives after the stop has been requested the method will block.
		 * disconnect() will leave the method (nearly) immediately.
		 * @return The number of bytes read
		 */
		int listen(ListenMode mode = NON_THREADED);

		/** Stops listening */
		void stopListening();

		/** Returns whether the connection is listening or not
		 * @return the listening state
		 */
		bool isListening() const;



	// -----------------------------------------------------------------------
	// GETTER AND SETTER
	// -----------------------------------------------------------------------
	public:
		/** Returns the address of the master client
		 * @return address of the master
		 */
		const std::string& masterAddress() const;

		/** Returns the name of the group this client sends its messages to
		 * @return name of the peergroup
		 */
		const std::string& peerGroup() const;

		/** Returns the name of the clients private group
		 * @return name of the private groupname
		 */
		std::string privateGroup() const;

		/** Returns the type of the client
		 * @return type of the client
		 */
		Protocol::ClientType type() const;

		/** Returns the priority of the client
		 * @return priority of the client
		 */
		Protocol::ClientPriority priority() const;

		/** Returns the number of available groups
		 * @return number o groups
		 */
		int groupCount() const;

		/** Returns the group at index i, i = [0..groupCount()-1]
		 * @param index
		 * @return name of the group
		 */
		const char* group(int i) const;

		/** Returns the names of the available message groups
		 * @return message groups
		 */
		std::vector<std::string> groups() const;

		/** Returns the password for the current session
		 * @return password
		 */
		const std::string& password() const;

		/** Sets a password that might be necessary to connect to the mediator.
		 * @param password
		 */
		void setPassword(const std::string& password);

		/** Returns the number of queued messages in the message queue
		 * @return Number of queued messages
		 */
		int queuedMessageCount() const;

		/** Returns a structure to some basic message statistics
		 * @see MessageStat
		 * @Return MessageStat
		 */
		const MessageStat& messageStat() const;

		const NetworkInterface* networkInterface() const;

		/** Sets the sequence number of the next message
		 * if implemented by the network interface
		 */
		void setSequenceNumber(int64_t seq);

		/** Gets the sequence number of the next message
		 * if implemented by the network interface
		 * @return sequence number
		 */
		int64_t getSequenceNumber() const;

		Core::Version schemaVersion() const;


	// -----------------------------------------------------------------------
	// PRIVATE COMMUNICATION API
	// -------------------------- --------------------------------------------
	private:
		/** Sends a message
		 * @param group destination of the message
		 * @param type type of the message
		 * @param msg the message to be sent
		 * @return SEISCOMP_SUCCESS ons sucess
		 */
		int send(const std::string& group, int type, Seiscomp::Communication::NetworkMessage* msg);

		/** Closes the connection to the spread server without notifying the
		 * other clients.
		 * @return SEISCOMP_SUCCESS ons sucess
		 */
		int shutdown();

		/** Returns true if the passed groupname is available on the messaging system
		 * @param group name of the group
		 * @return true if the group is available
		 */
		bool isGroupAvailable(const std::string& group);

		/** Push a message in the queue
		 * @param NetworkMessage*
		 */
		void queueMessage(NetworkMessage *message);

		/** Push a message in the queue and releases the ownership of the
		 *  passed auto_ptr
		 * @param auto_ptr<NetworkMessage>
		 */
		void queueMessage(std::auto_ptr<NetworkMessage>& message);

		int handshake(const std::string &protocolVersion,
					  std::string *supportedVersion = NULL);


		// -----------------------------------------------------------------------
		// DATA MEMBERS
		// -----------------------------------------------------------------------
	protected:
		NetworkInterfacePtr      _networkInterface;
		Core::Version            _schemaVersion;

	private:
		std::string              _clientName;
		std::string              _masterAddress;
		std::string              _peerGroup;
		std::string              _privateMasterGroup;
		Protocol::ClientType     _type;
		Protocol::ClientPriority _priority;
		int                      _timeOut;

		bool                     _archiveRequested;
		char                     _archiveMsg[Protocol::STD_MSG_LEN];
		int                      _archiveMsgLen;
		std::set<std::string>    _subscribedArchiveGroups;

		//! Holds the message groups which are currently available
		std::vector<std::string> _groups;

		//! Holds the joined message groups
		std::set<std::string>    _subscriptions;

		//! Holds a password that might be necessary to connect to a master client
		std::string              _password;

		bool                       _stopRequested;
		volatile bool              _isListening;
		std::auto_ptr<MessageStat> _messageStat;
		ConnectionInfo*            _connectionInfo;

		//! Holds the messages that have been read from the network
		std::queue<NetworkMessage*> _messageQueue;

		bool                     _isConnected;

		mutable boost::mutex     _messageQueueMutex;
		boost::mutex             _writeBufferMutex;
		boost::try_mutex         _readBufferMutex;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Communication
} // namespace Seiscomp


#endif
