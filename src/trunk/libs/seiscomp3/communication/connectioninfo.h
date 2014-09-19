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

#ifndef __SEISCOMP_CONNECTION_CONNECTIONINFO_H__
#define __SEISCOMP_CONNECTION_CONNECTIONINFO_H__

#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

#include <seiscomp3/utils/timer.h>
#include <seiscomp3/communication/networkinterface.h>
#include <seiscomp3/core/enumeration.h>



namespace Seiscomp {
namespace Communication {


MAKEENUM(
	ConnectionInfoTag,
	EVALUES(
		TIME_TAG,
		PRIVATE_GROUP_TAG,
		HOSTNAME_TAG,
		CLIENTNAME_TAG,
		IPS_TAG,
		PROGRAMNAME_TAG,
		PID_TAG,
		CPU_USAGE_TAG,
		TOTAL_MEMORY_TAG,
		CLIENT_MEMORY_USAGE_TAG,
		MEMORY_USAGE_TAG,
		SENT_MESSAGES_TAG,
		RECEIVED_MESSAGES_TAG,
		MESSAGE_QUEUE_SIZE_TAG,
		SUMMED_MESSAGE_QUEUE_SIZE_TAG,
		AVERAGE_MESSAGE_QUEUE_SIZE_TAG,
		SUMMED_MESSAGE_SIZE_TAG,
		AVERAGE_MESSAGE_SIZE_TAG,
		OBJECT_COUNT_TAG,
		UPTIME_TAG,
		RESPONSE_TIME_TAG
	),
	ENAMES(
		"time",
		"privategroup",
		"hostname",
		"clientname",
		"ips",
		"programname",
		"pid",
		"cpuusage",
		"totalmemory",
		"clientmemoryusage",
		"memoryusage",
		"sentmessages",
		"receivedmessages",
		"messagequeuesize",
		"summedmessagequeuesize",
		"averagemessagequeuesize",
		"summedmessagesize",
		"averagemessagesize",
		"objectcount",
		"uptime",
		"responsetime"
	)
);


// Primary Template
template <EConnectionInfoTag T>
struct ConnectionInfoT;


// Specializations
#define SPECIALIZE_CONNECTIONINFOT(TAG, TYPE) \
	template <> struct ConnectionInfoT<TAG> { \
		typedef TYPE Type; \
	};


SPECIALIZE_CONNECTIONINFOT(TIME_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(PRIVATE_GROUP_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(HOSTNAME_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(CLIENTNAME_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(IPS_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(PROGRAMNAME_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(PID_TAG, int)
SPECIALIZE_CONNECTIONINFOT(CPU_USAGE_TAG, double)
SPECIALIZE_CONNECTIONINFOT(TOTAL_MEMORY_TAG, int)
SPECIALIZE_CONNECTIONINFOT(CLIENT_MEMORY_USAGE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(MEMORY_USAGE_TAG, double)
SPECIALIZE_CONNECTIONINFOT(SENT_MESSAGES_TAG, int)
SPECIALIZE_CONNECTIONINFOT(RECEIVED_MESSAGES_TAG, int)
SPECIALIZE_CONNECTIONINFOT(MESSAGE_QUEUE_SIZE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(SUMMED_MESSAGE_QUEUE_SIZE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(AVERAGE_MESSAGE_QUEUE_SIZE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(SUMMED_MESSAGE_SIZE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(AVERAGE_MESSAGE_SIZE_TAG, int)
SPECIALIZE_CONNECTIONINFOT(OBJECT_COUNT_TAG, int)
SPECIALIZE_CONNECTIONINFOT(UPTIME_TAG, std::string)
SPECIALIZE_CONNECTIONINFOT(RESPONSE_TIME_TAG, int)


struct SC_SYSTEM_CLIENT_API MessageStat {
	unsigned int totalSentMessages;
	unsigned int totalReceivedMessages;
	unsigned int summedMessageQueueSize;
	unsigned int summedMessageSize;

	MessageStat()
	 : totalSentMessages(0),
	   totalReceivedMessages(0),
	   summedMessageQueueSize(0), summedMessageSize(0) {
	}
};


class SystemConnection;


class SC_SYSTEM_CLIENT_API ConnectionInfo {
	// ------------------------------------------------------------------
	// Nested Types
	// ------------------------------------------------------------------
	private:
		typedef std::vector<SystemConnection*> SystemConnections;
		typedef std::vector<NetworkInterface*> NetworkInterfaces;
		typedef boost::function<
		void (const SystemConnection*,const Core::Time &,std::ostream&)
		> InfoCallback;

	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	private:
		ConnectionInfo();

	public:
		~ConnectionInfo();


	// ------------------------------------------------------------------
	// Public Interface
	// ------------------------------------------------------------------
	public:
		static ConnectionInfo* Instance();

		bool start();

		/** The return is a string containing the collected information. Each item is enclosed
		 * with an ? as a name&value pair. The names are:
		 * connectionname, hostname, ips, programname, pid, cpu (in %), totalhostmemory (in kB),
		 * clientmemoryusage (in kB), memory (in %), sentmessages, receivedmessages,
		 * messagequeuesize, averagemessagequeuesize, uptime (this will be added by the master)
		 * @return std::string conainting the collected information
		 */
		std::string info(const SystemConnection *con);

		/** The return is a string containing the collected information. Each item is enclosed
		 * with an ? as a name&value pair. The names are:
		 * connectionname, hostname, ips, programname, pid, cpu (in %), totalhostmemory (in kB),
		 * clientmemoryusage (in kB), memory (in %), uptime (this will be added by the master)
		 * @return std::string conainting the collected information
		 */
		std::string info(const NetworkInterface* ni);

		void registerConnection(SystemConnection* con, boost::mutex* mutex = NULL);
		bool unregisterConnection(SystemConnection* con);

		void registerConnection(NetworkInterface* ni, boost::mutex* mutex = NULL);
		bool unregisterConnection(NetworkInterface* ni);

		/**
		 * Registers a callback when a connection info is generated. This
		 * callback can be used to add more information to the output.
		 * If a callback is already registered and a valid callback is passed,
		 * false is returned, true otherwise.
		 */
		bool registerInfoCallback(InfoCallback);


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private:
		bool stop();
		unsigned long pid();
		const std::string& hostname();
		const std::string& programName();
		const std::string& ip();
		int totalMemory();
		int memoryUsage();
		void calculateCpuUsage();
		double cpuUsage() const;
		double calculateCurrentCpuUsage() const;

		std::string getLineFromFile(const std::string& fileName, const std::string& tag);


	// ------------------------------------------------------------------
	// Private Data Member
	// ------------------------------------------------------------------
	private:
		unsigned long        _pid;
		std::string          _hostname;
		std::string          _programName;
		std::string          _ips;
		int                  _totalMemory;
		int                  _usedMemory;
		int                  _delay;
		int                  _sendInterval; // multiple of _delay
		int                  _currentTimeout;
		long                 _clockTicks;
		double               _cpuUsage;
		bool                 _isRunning;
		bool                 _firstMeasurement;
		Core::Time           _lastLogTime;

		std::vector<double>  _cpuValues;

		SystemConnections          _systemConnections;
		std::vector<boost::mutex*> _systemConnectionsMutexes;
		NetworkInterfaces          _networkInterfaces;
		std::vector<boost::mutex*> _networkInterfacesMutexes;

		InfoCallback         _infoCallback;

		mutable boost::mutex _mutex;
		Util::Timer          _timer;

		static ConnectionInfo* _Instance;
};

} // namespace Communication
} // namespace Seiscomp

#endif
