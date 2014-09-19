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

#include <seiscomp3/communication/clientstatus.h>

#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <seiscomp3/core/strings.h>



namespace Seiscomp {
namespace Communication {


ClientStatus::ClientStatus(const std::string& data)
{
	parseData(data);
}




void ClientStatus::parseData(const std::string& data)
{
	_clientInfoData.clear();

	_clientInfoData[PRIVATE_GROUP_TAG]             = _privateGroup           = getValue(data, createTag(ConnectionInfoTag(PRIVATE_GROUP_TAG).toString()));
	_clientInfoData[HOSTNAME_TAG]                  = _hostname               = getValue(data, createTag(ConnectionInfoTag(HOSTNAME_TAG).toString()));
	_clientInfoData[IPS_TAG]                       = _ips                    = getValue(data, createTag(ConnectionInfoTag(IPS_TAG).toString()));
	_clientInfoData[PROGRAMNAME_TAG]               = _programName            = getValue(data, createTag(ConnectionInfoTag(PROGRAMNAME_TAG).toString()));
	_clientInfoData[PID_TAG]                       = _pid                    = getValue(data, createTag(ConnectionInfoTag(PID_TAG).toString()));
	_clientInfoData[CPU_USAGE_TAG]                 = _cpuUsage               = getValue(data, createTag(ConnectionInfoTag(CPU_USAGE_TAG).toString()));
	_clientInfoData[TOTAL_MEMORY_TAG]              = _totalMemory            = getValue(data, createTag(ConnectionInfoTag(TOTAL_MEMORY_TAG).toString()));
	_clientInfoData[CLIENT_MEMORY_USAGE_TAG]       = _clientMemoryUsage      = getValue(data, createTag(ConnectionInfoTag(CLIENT_MEMORY_USAGE_TAG).toString()));
	_clientInfoData[SENT_MESSAGES_TAG]             = _sentMessages           = getValue(data, createTag(ConnectionInfoTag(SENT_MESSAGES_TAG).toString()));
	_clientInfoData[RECEIVED_MESSAGES_TAG]         = _receivedMessages       = getValue(data, createTag(ConnectionInfoTag(RECEIVED_MESSAGES_TAG).toString()));
	_clientInfoData[MESSAGE_QUEUE_SIZE_TAG]        = _messageQueueSize       = getValue(data, createTag(ConnectionInfoTag(MESSAGE_QUEUE_SIZE_TAG).toString()));
	_clientInfoData[SUMMED_MESSAGE_QUEUE_SIZE_TAG] = _summedMessageQueueSize = getValue(data, createTag(ConnectionInfoTag(SUMMED_MESSAGE_QUEUE_SIZE_TAG).toString()));
	_clientInfoData[SUMMED_MESSAGE_SIZE_TAG]       = _summedMessageSize      = getValue(data, createTag(ConnectionInfoTag(SUMMED_MESSAGE_SIZE_TAG).toString()));
	_clientInfoData[OBJECT_COUNT_TAG]              = _objectCount            = getValue(data, createTag(ConnectionInfoTag(OBJECT_COUNT_TAG).toString()));
	_clientInfoData[UPTIME_TAG]                    = _uptime                 = getValue(data, createTag(ConnectionInfoTag(UPTIME_TAG).toString()));
	_clientInfoData[RESPONSE_TIME_TAG]             = "0";


	std::vector<std::string> tokens;
	Core::split(tokens, _privateGroup.c_str(), "#");
	_clientName = tokens[1];
	_clientInfoData[CLIENTNAME_TAG] = _clientName;

	_memoryUsage = "0";
	try {
		if ( boost::lexical_cast<int>(_totalMemory) > 0 )
			_memoryUsage = boost::lexical_cast<std::string>(
				boost::lexical_cast<double>(_clientMemoryUsage) / boost::lexical_cast<double>(_totalMemory) * 100 );
	}
	catch ( boost::bad_lexical_cast& ) {}
	_clientInfoData[MEMORY_USAGE_TAG] = _memoryUsage;

	_averageMessageQueueSize = "0";
	try {
		if (boost::lexical_cast<int>(_receivedMessages) > 0)
			_averageMessageQueueSize = boost::lexical_cast<std::string>(
					boost::lexical_cast<int>(_summedMessageQueueSize) / boost::lexical_cast<int>(_receivedMessages));
	}
	catch ( boost::bad_lexical_cast& ) {}
	_clientInfoData[AVERAGE_MESSAGE_QUEUE_SIZE_TAG] = _averageMessageQueueSize;

	_averageMessageSize = "0";
	try {
		if (boost::lexical_cast<int>(_receivedMessages) > 0)
			_averageMessageSize = boost::lexical_cast<std::string>(
					boost::lexical_cast<int>(_summedMessageSize) / boost::lexical_cast<int>(_receivedMessages));
	}
	catch ( boost::bad_lexical_cast& ) {}
	_clientInfoData[AVERAGE_MESSAGE_SIZE_TAG] = _averageMessageSize;

	try {
		std::ostringstream os;
		double cpu = boost::lexical_cast<double>(_cpuUsage);
		if (cpu < 0.1)
			cpu = 0;
		os << std::fixed << std::setprecision(1) << cpu;
		_cpuUsage = os.str();
	}
	catch ( boost::bad_lexical_cast& ) {}
	_clientInfoData[CPU_USAGE_TAG] = _cpuUsage;

	try {
		std::ostringstream os;
		os << std::fixed << std::setprecision(1) << boost::lexical_cast<double>(_memoryUsage);
		_memoryUsage = os.str();
	}
	catch ( boost::bad_lexical_cast& ) {}
	_clientInfoData[MEMORY_USAGE_TAG] = _memoryUsage;
}


ClientStatus* ClientStatus::CreateClientStatus(const std::string& data)
{
	return new ClientStatus(data);
}


std::string ClientStatus::getValue(const std::string& data, const std::string& name)
{
	std::string::size_type pos0 = data.find(name);
	if (pos0 == std::string::npos)
		return "";

	pos0 = data.find("=", pos0);
	if (pos0 == std::string::npos || pos0 + 1 == std::string::npos)
		return "";

	++pos0;
	std::string::size_type pos1 = data.find("&", pos0);

	return data.substr(pos0, pos1 - pos0);
}


const std::map<Communication::ConnectionInfoTag, std::string>& ClientStatus::clientInfoData() const
{
	return _clientInfoData;
}


} // namespace Utils
} // namespace Seiscomp
