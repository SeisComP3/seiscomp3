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

#ifndef __SEISCOMP_COMMUNICATION_MASTER_H__
#define __SEISCOMP_COMMUNICATION_MASTER_H__

#include <time.h>

#include <string>
#include <vector>
#include <set>
#include <queue>

#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include <seiscomp3/communication/masterplugininterface.h>
#include <seiscomp3/communication/networkinterface.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/datamodel/version.h>
#include <seiscomp3/client/queue.h>
#include <seiscomp3/client/queue.ipp>
#include <seiscomp3/utils/timer.h>

#include "clientdb.h"


namespace Seiscomp {
namespace Communication {


class ServiceMessage;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * \brief Master client who manages the messaging system / logic
 */
class Master: public boost::noncopyable
{

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	private:
		typedef std::vector<MasterPluginInterfacePtr> Plugins;

	// -----------------------------------------------------------------------
	// CONSTRUCTION - DESTRUCTION
	// -----------------------------------------------------------------------
public:
	static Master* Create(const std::string& name);
	~Master();


	// -----------------------------------------------------------------------
	// PUBLIC INTERFACE
	// -----------------------------------------------------------------------
public:

	//! Set alternative configuration file
	void setConfigFile(const std::string& fileName);

	bool init();

	/** Starts the master client.
	 * @param serverAddress address to connect to
	 * @return status code */
	void start(const std::string& serverAddress);

	/** Stops the server.
	 * @return status code */
	int stop();


	// -----------------------------------------------------------------------
	// PRIVATE INTERFACE
	// -----------------------------------------------------------------------
private:

	//! Reads configured pluginnames from the configuration file
	void readPluginNames(Config::Config& conf, const std::string& name);

	//!Does the actual processing of the arrived messages.
	void run();

	void listen();

	//! Handles networkmessages inside a thread
	void processNetworkMessages();

	//! Handles a service message
	void processServiceMessage(ServiceMessage* sm);

	/** Connects to the spread master deamon and creates protocol determined
	 * groups.
	 * @return status code */
	int connect(const std::string& serverAddress);

	/** Disconnect the master client from the messaging system. The clients
	 * notified by the master that he will be leaving.
	 * @return status code */
	int disconnect();

	/** Reconnects to the spread server. Returns on success otherwise the
	 * call will block.
	 */
	void reconnect();

	/** A factory that returns a new message. Initialization of a correct timestamp
	 * and sequence number is automatically handled by this method.
	 * @param type type of the message to be returned
	 * @param buffer buffer from which the message will be created.
	 * @return The new message. NULL in case of an error */
	NetworkMessage* createMsg(const int mesgType, const char* buf = NULL, int len = 0);

	/** Sends a message of the passe type to the given destination. This method is meant
	 * to be a convenience function.
	 * @param destination The destination this message should be sent to
	 * @param msgType The type of the message to be sent
	 * @return status code */
	int sendMsg(const std::string& destination, int msgType);

	/** Sends a message to a destination which is specified by the passed message.
	 * Therefore the client has to make sure that the message is properly initialized
	 * before the send method is called. It is recomended to use the createMsg factory.
	 * That way every message has a valid timestamp and sequence number.
	 * @param m the message to be send
	 * @return The status code */
	int send(NetworkMessage* m);

	/** Sends a message and archives it. */
	int sendAndArchive(NetworkMessage* m);

	/** Sends a message.
	 * In case of an error sending will be repeated continously.
	 */
	int sendRaw(NetworkMessage* m);

	//! Sets th sender field in the received messages
	// void setSender(NetworkMessage* msg);

	//! Sets a timestamp and a sequenze number.
	void tagMsg(NetworkMessage* msg);

	//! Archives passed messages
	void archiveMsg(NetworkMessage* msg);

	/** Sends the requested data to client. The messages which will be send comprise
	 * all data from the given sequence number to the latest archive index.
	 * @return true for success false for an error */
	void handleArchiveRequest(ServiceMessage* sm);

	//! Return the curent time
	time_t timeStamp();

	//! Returns a valid timstamp
	int sequenceNumber();


	// -----------------------------------------------------------------------
	// PRIVATE GETTER AND SETTER
	// -----------------------------------------------------------------------
private:
	Master(const std::string& name);

	void setRunning(bool running);

	bool isRunning() const;

	//! Returns a string containing the available groups:
	std::string msgGroups();


	// -----------------------------------------------------------------------
	// PRIVATE THREAD SPECIFIC DATA MEMBERS
	// -----------------------------------------------------------------------
private:
	bool _isRunning;


	// -----------------------------------------------------------------------
	// PRIVATE MASTER SPECIFIC DATA MEMBERS
	// -----------------------------------------------------------------------
private:
	//! Name of an alternative configuration file
	std::string _configFile;

	//! Holds the address of the spread server
	std::string _serverAddress;

	//! Holds the name of the administration client
	std::string _adminClientName;

	//! Specifies if an administration client is connected
	bool        _isAdminConnected;
	std::string _privateAdminGroup;

	//! Archive for the received messages
	std::auto_ptr<std::vector<NetworkMessage*> > _archive;

	//! Stores the data sent by the clients connect call
	ClientDB _clientDB;

	//! Holds the message groups which are currently available
	std::set<std::string> _msgGroups;

	//! Password needed to connect to the mediator
	std::string _password;

	Plugins _plugins;

	// -----------------------------------------------------------------------
	// PRIVATE COMMUNICATION SPECIFIC DATA MEMBERS
	// -----------------------------------------------------------------------
private:
	struct Slot {
		Slot() : msg(NULL), fromOutside(false) {}

		Slot(NetworkMessage *m, bool o)
		 : msg(m), fromOutside(o) {}

		NetworkMessage *msg;
		bool fromOutside;
	};

	NetworkInterfacePtr              _networkInterface;
	ConnectionInfo*                  _connectionInfo;
	std::auto_ptr<Util::StopWatch>   _uptime;

	Client::ThreadedQueue<Slot>               _messageQueue;
	Client::ThreadedQueue<Core::BaseObject*>  _networkMessageQueue;

	boost::mutex     _sendMutex;
	boost::mutex     _reconnectMutex;
	boost::mutex     _archiveMutex;

	int _seqNum;
	int _maxSeqNum;

	std::string _name;

	MessageStat _messageStat;
	Core::Version _schemaVersion;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp

#endif
