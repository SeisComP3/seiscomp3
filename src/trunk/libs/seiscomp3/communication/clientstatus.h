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

#ifndef __SEISCOMP_UTILS_COMMUNICATION__
#define __SEISCOMP_UTILS_COMMUNICATION__


#include <string>
#include <map>

#include <boost/lexical_cast.hpp>

#include <seiscomp3/communication/connectioninfo.h>



namespace Seiscomp {
namespace Communication {



class SC_SYSTEM_CLIENT_API ClientStatus {

	private:
		/** Takes the data of a TATE_OF_HEALTH_RESPONSE_MSG. For information
		 * about the contents of the data string \see ConnectionInfo
		 */
		ClientStatus(const std::string& data);

	public:
		/**
		 * Takes the data of a TATE_OF_HEALTH_RESPONSE_MSG
		 * @return ClientStatus on success or NULL on  failure
		 */
		static ClientStatus* CreateClientStatus(const std::string& data);

	public:
		std::string getValue(const std::string& data, const std::string& name);

		template <Communication::EConnectionInfoTag Tag>
		typename Communication::ConnectionInfoT<Tag>::Type getValue(const std::string& data);

		const std::map<Communication::ConnectionInfoTag, std::string>& clientInfoData() const;

		std::string privateGroup() const { return _privateGroup; }
		std::string clientName() const { return _clientName; }
		std::string hostname() const { return _hostname; }
		std::string ips() const { return _ips; }
		std::string programName() const { return _programName; }
		std::string pid() const { return _pid; }
		std::string cpuUsage() const { return _cpuUsage; }
		std::string totalMemory() const { return _totalMemory; }
		std::string clientMemoryUsage() const { return _clientMemoryUsage; }
		std::string memoryUsage() const { return _memoryUsage; }
		std::string sentMessages() const { return _sentMessages; }
		std::string receivedMessages() const { return _receivedMessages; }
		std::string messageQueueSize() const { return _messageQueueSize; }
		std::string summedMessageQueueSize() const { return _summedMessageQueueSize; }
		std::string averageMessageQueueSize() const { return _averageMessageQueueSize; }
		std::string summedMessageSize() const { return _averageMessageSize; }
		std::string averageMessageSize() const { return _averageMessageSize; }
		std::string objectCount() const { return _objectCount; }
		std::string uptime() const { return _uptime; }


	private:
		void parseData(const std::string& data);
		std::string createTag(const std::string& name) {
			return "&" + name + "=";
		}

	private:
		std::string _privateGroup;
		std::string _clientName;
		std::string _hostname;
		std::string _ips;
		std::string _programName;
		std::string _pid;
		std::string _cpuUsage;
		std::string _totalMemory;
		std::string _clientMemoryUsage;
		std::string _memoryUsage;
		std::string _sentMessages;
		std::string _receivedMessages;
		std::string _messageQueueSize;
		std::string _summedMessageQueueSize;
		std::string _averageMessageQueueSize;
		std::string _summedMessageSize;
		std::string _averageMessageSize;
		std::string _objectCount;
		std::string _uptime;

		std::map<Communication::ConnectionInfoTag, std::string> _clientInfoData;
};




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <Communication::EConnectionInfoTag Tag>
typename Communication::ConnectionInfoT<Tag>::Type ClientStatus::getValue(const std::string& data)
{
	typename Communication::ConnectionInfoT<Tag>::Type val;
	std::string::size_type pos0 = data.find(Communication::ConnectionInfoTag(Tag).toString());
	if (pos0 == std::string::npos)
		return val;

	pos0 = data.find("=", pos0);
	if (pos0 == std::string::npos || pos0 + 1 == std::string::npos)
		return val;

	++pos0;
	std::string::size_type pos1 = data.find("&", pos0);

	try
	{
		val = boost::lexical_cast<typename Communication::ConnectionInfoT<Tag>::Type >(data.substr(pos0, pos1 - pos0));
	}
	catch ( boost::bad_lexical_cast& )
	{
		return val;
	}
	return val;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Communication
} // namespace Seiscomp

#endif
