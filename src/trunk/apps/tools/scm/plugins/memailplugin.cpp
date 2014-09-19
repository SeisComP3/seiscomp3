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

#define SEISCOMP_COMPONENT ScMonitor
#include <seiscomp3/logging/log.h>

#include "memailplugin.h"

#include <iostream>
#include <algorithm>

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/client/application.h>


namespace Seiscomp {
namespace Applications {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EmailMessage::EmailMessage() : _hasChanged(false) { }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setHeader(const std::string& header)
{
	_header.assign(header);
	_hasChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setHeaderFilteredClients(const std::string& header)
{
	_headerFilteredClients.assign(header);
	_hasChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setHeaderRequiredClients(const std::string& header)
{
	_headerRequiredClients.assign(header);
	_hasChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setHeaderSilentClients(const std::string& header)
{
	_headerSilentClients.assign(header);
	_hasChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setFilteredClients(const std::vector<std::string>& clients)
{
	_hasChanged = true;
	if ( !_filteredClients.empty() )
		_filteredClients.clear();
	for ( size_t i = 0; i < clients.size(); ++i ) {
		_filteredClients.append(clients[i] + "\n--\n");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setSilentClients(
		const std::vector<std::string>& silentClients,
		const std::vector<std::string>& recoveredClients)
{
	_hasChanged = true;
	if ( !_silentClients.empty() )
		_silentClients.clear();
	for ( size_t i = 0; i < silentClients.size(); ++i )
		_silentClients.append(silentClients[i] + "\n");
	for ( size_t i = 0; i < recoveredClients.size(); ++i ) {
		_silentClients.append(recoveredClients[i] + "\n");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setRequiredClients(
		const std::vector<std::string>& missing,
		const std::vector<std::string>& reappeared)
{
	_hasChanged = true;
	if ( !_requiredClients.empty() )
		_requiredClients.clear();
	for ( size_t i = 0; i < missing.size(); ++i ) {
		_requiredClients.append(missing[i] + "\n");
	}
	for ( size_t i = 0; i < reappeared.size(); ++i ) {
		_requiredClients.append(reappeared[i] + "\n");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::setUserData(const std::string& data)
{
	_hasChanged = true;
	_userData.assign(data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& EmailMessage::message()
{
	if ( _hasChanged ) {
		_hasChanged = !_hasChanged;
		_message.clear();
		std::string doubleSpacer = "\n\n";
		std::string singleSpacer = "\n";
		_message.append(_header);
		_message.append(Core::Time::GMT().toString("%a, %d %b %Y %H:%M:%S"));
		_message.append(doubleSpacer);

		if ( !_filteredClients.empty() ) {
			_message.append(_headerFilteredClients + doubleSpacer);
			_message.append(_filteredClients + singleSpacer);
		}
		if ( !_requiredClients.empty() ) {
			_message.append(_headerRequiredClients + doubleSpacer);
			_message.append(_requiredClients + singleSpacer);
		}
		if ( !_silentClients.empty() ) {
			_message.append(_headerSilentClients + doubleSpacer);
			_message.append(_silentClients + singleSpacer);
		}
		if ( !_userData.empty() )
			_message.append(_userData);
	}
	return _message;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EmailMessage::empty()
{
	bool empty =
		_filteredClients.empty() &&
		_silentClients.empty() &&
		_requiredClients.empty() &&
		_message.empty();
	return empty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EmailMessage::clear()
{
	if ( empty() ) return;

	_message.clear();
	_filteredClients.clear();
	_silentClients.clear();
	_requiredClients.clear();

	_hasChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EmailSender* EmailSender::Create()
{
	EmailSender* sender = new EmailSender;
	return sender;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EmailSender::sendEmail(const std::string& text, const std::string& recipient)
{
	SEISCOMP_DEBUG("Sending email to: %s", recipient.c_str());
	std::ostringstream command;
	command << "echo \'" << text << "\'  | mailx -s \'scm notification\' " << recipient << std::endl;
	SEISCOMP_DEBUG("%s", command.str().c_str());
	return Core::system(command.str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <Communication::EConnectionInfoTag tag>
inline const std::string printFilterMatch(const ClientInfoData& data)
{
	ClientInfoData::const_iterator clientIt = data.find(tag);
	std::string text;
	if ( clientIt != data.end() ) {
		text = std::string(Communication::ConnectionInfoTag(tag).toString())
		+ " = " + clientIt->second;
		SEISCOMP_DEBUG("%s", text.c_str());
	}
	return text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_CLASS_DERIVED(MEmailPlugin,
                           MonitorPluginInterface,
                           "MEmailPlugin");

REGISTER_MONITOR_PLUGIN_INTERFACE(MEmailPlugin, "memailplugin");
ADD_SC_PLUGIN(
		"monitor plugin for email notifications",
		"GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 1, 0, 0)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MEmailPlugin::MEmailPlugin() :
	MonitorPluginInterface("memailplugin"),
	_filterResponseInterval(60),
	_requiredClientsTimeMarker(Core::Time::GMT()),
	_reportRequiredClientsTimeSpan(5 * 60),
	_reportSilentClients(true),
	_reportSilentClientsTimeSpan(60), // 60 sec.
	_sendEmail(false)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MEmailPlugin::init(const Config::Config& cfg) {
	if ( !MonitorPluginInterface::init(cfg) ) return false;
	try {
		std::vector<std::string> recipients = cfg.getStrings(name() + ".recipients");
		addRecipients(recipients);
	}
	catch( Config::Exception& e ) {
		SEISCOMP_ERROR("MEmailPlugin could not be initialized due to missing recipients list: %s", e.what());
		setOperational(false);
	}

	try {
		_template = cfg.getString(name() + ".template");
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		std::vector<std::string> requiredClients = cfg.getStrings(name() + ".requiredClients");
		for (size_t i = 0; i < requiredClients.size(); ++i) {
			_requiredClients.insert(std::make_pair(requiredClients[i], false));
		}
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		_reportSilentClients = cfg.getBool(name() + ".reportSilentClients");
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		_reportSilentClientsTimeSpan =
			Core::TimeSpan(cfg.getDouble(name() + ".reportSilentClientsTimeSpan") * 60);
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		double filterMeanInterval = cfg.getDouble(name() + ".filterMeanInterval");
		setFilterMeanInterval(filterMeanInterval);
	}
	catch (Config::Exception& e) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		_reportRequiredClientsTimeSpan = cfg.getDouble(name() + ".reportRequiredClients") * 60;
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	try {
		_sendEmail = cfg.getBool(name() + ".sendEmail");
	}
	catch ( Config::Exception& e ) {
		SEISCOMP_DEBUG("%s", e.what());
	}

	_sender= std::auto_ptr<EmailSender>(EmailSender::Create());
	if ( !_sender.get() ) {
		SEISCOMP_ERROR("MEmailPlugin could not be initialized. Email service not available!");
		setOperational(false);
	}

	std::stringstream ss;
	ss << "This message has been automatically generated by scm on host: "
	<< Core::getHostname() << " for master: master@" << SCCoreApp->connection()->masterAddress() << std::endl;

	_message.setHeader(ss.str());
	ss.str(std::string());

	ss << "The following clients match the given filter condition:" << std::endl;
	ss << filterString();
	_message.setHeaderFilteredClients(ss.str());
	ss.str(std::string());

	ss << "Some of the connected have been silent for more than "
	   << _reportRequiredClientsTimeSpan << " seconds" << std::endl;
	ss << "'-' denotes a silent and '+' a recovered client.";
	_message.setHeaderSilentClients(ss.str());
	ss.str(std::string());

	ss << "Some required clients are disconnected (-) or reconnected (+)" << std::endl;
	ss << "Required clients: ";
	RequiredClients::iterator it = _requiredClients.begin();
	for ( ; it != _requiredClients.end(); ++it ) {
		if ( it != _requiredClients.begin() )
			ss << ", ";
		ss << it->first;
	}
	_message.setHeaderRequiredClients(ss.str());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MEmailPlugin::process(const ClientTable& clientTable)
{
	if ( !operational() )
		return;

	_message.clear();

	if ( isFilteringEnabled() ) {
		SEISCOMP_DEBUG("Processing client table with %d clients", (int)clientTable.size());
		for ( ClientTable::const_iterator it = clientTable.begin(); it != clientTable.end(); ++it ) {
			ClientInfoData::const_iterator clientIt = it->info.find(Communication::CLIENTNAME_TAG);
			if ( clientIt != it->info.end() ) {
				SEISCOMP_DEBUG("Applying filter on client: %s ", clientIt->second.c_str());
			}
		}

		const ClientTable* match = filterMean(clientTable);
		if ( match ) {
			SEISCOMP_DEBUG("Number of filter matches: %d", (int)match->size());
			std::vector<std::string> data;
			std::stringstream ss;
			for ( ClientTable::const_iterator it = match->begin(); it != match->end(); ++it ) {
				ss << printFilterMatch<Communication::PROGRAMNAME_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::CLIENTNAME_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::HOSTNAME_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::TOTAL_MEMORY_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::CLIENT_MEMORY_USAGE_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::MEMORY_USAGE_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::CPU_USAGE_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::MESSAGE_QUEUE_SIZE_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::UPTIME_TAG>(*it) << std::endl;
				ss << printFilterMatch<Communication::RESPONSE_TIME_TAG>(*it);
				data.push_back(ss.str());
				ss.str(std::string());
				SEISCOMP_DEBUG("--");
			}
			if ( data.size() > 0 )
				_message.setFilteredClients(data);

			if ( !_template.empty() )
				_message.setUserData(_template);
		}
	}

	if ( _reportSilentClients ) {
		std::vector<std::string> silentClients;
		std::vector<std::string> recoveredClients;

		ClientTable::const_iterator clientIt = clientTable.begin();
		for ( ; clientIt != clientTable.end(); ++clientIt ) {
			const ClientInfoData& clientInfoData = clientIt->info;

			// Check if the client is already in the list
			ClientInfoData::const_iterator clientNameIt =
				clientInfoData.find(Communication::CLIENTNAME_TAG);

			if ( clientNameIt == clientInfoData.end() ) {
				SEISCOMP_DEBUG("Could not find clientnametag in clientinfodata");
				continue;
			}

			// Ignore clients which are not listed in the required clients.
			if ( _requiredClients.find(clientNameIt->second) == _requiredClients.end() )
				continue;

			SilentClients::iterator silentClientIt = std::find(
					_silentClients.begin(),
					_silentClients.end(),
					clientNameIt->second
			);

			ClientInfoData::const_iterator responseTimeIt = clientInfoData.find(Communication::RESPONSE_TIME_TAG);
			if ( responseTimeIt == clientInfoData.end() )
				continue;

			// Get the hostname
			ClientInfoData::const_iterator hostNameIt =	clientInfoData.find(Communication::HOSTNAME_TAG);
			if ( hostNameIt == clientInfoData.end() ){
				SEISCOMP_DEBUG("Could not find HOSTNAME_TAG");
				continue;
			}
			std::string hostNameStr = " on " + hostNameIt->second;

			int responseTime = 0;
			Core::fromString(responseTime, responseTimeIt->second);
			if ( responseTime > _reportSilentClientsTimeSpan ) {
				if ( silentClientIt == _silentClients.end() ) {
					silentClients.push_back(std::string("- ") + clientNameIt->second + hostNameStr);
					_silentClients.push_back(clientNameIt->second);
				}
			}
			else {
				if ( silentClientIt != _silentClients.end() ) {
					recoveredClients.push_back("+ " + *silentClientIt + hostNameStr);
					_silentClients.erase(silentClientIt);
				}
			}
		}
		if ( silentClients.size() > 0 || recoveredClients.size() > 0 )
			_message.setSilentClients(silentClients, recoveredClients);
	}

	if ( Core::Time::GMT() - _requiredClientsTimeMarker > _reportRequiredClientsTimeSpan ) {
		_requiredClientsTimeMarker = Core::Time::GMT();
		std::vector<std::string> missingClients;
		std::vector<std::string> reconnectedClients;
		RequiredClients::iterator rcIt = _requiredClients.begin();
		for ( ; rcIt != _requiredClients.end(); ++rcIt ) {
			ClientTable::const_iterator found =
				std::find_if(
						clientTable.begin(),
						clientTable.end(),
						std::bind2nd(std::ptr_fun(findName), rcIt->first)
				);

			if ( found == clientTable.end() && !rcIt->second ) {
				missingClients.push_back(std::string("- ") + rcIt->first);
				rcIt->second = true;
			}
			else if ( found != clientTable.end() && rcIt->second ) {
				rcIt->second = false;
				ClientInfoData::const_iterator hostNameIt =	found->info.find(Communication::HOSTNAME_TAG);
				if ( hostNameIt == found->info.end() ) {
					SEISCOMP_DEBUG("Could not find HOSTNAME_TAG");
					continue;
				}
				std::string hostNameStr = " on " + hostNameIt->second;
				reconnectedClients.push_back(std::string("+ ") + rcIt->first + hostNameStr);
			}
		}
		if ( missingClients.size() > 0 || reconnectedClients.size() > 0 )
			_message.setRequiredClients(missingClients, reconnectedClients);
	}

	// Send email
	if ( !_message.empty() && _sendEmail ) {
		for ( size_t i = 0; i < _recipients.size(); ++i ) {
			if ( !_sender->sendEmail(_message.message(), _recipients[i]) )
				SEISCOMP_ERROR("MEmailPlugin: Sending notification to %s failed", _recipients[i].c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MEmailPlugin::addRecipient(const std::string& recipient)
{
	_recipients.push_back(recipient);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MEmailPlugin::addRecipients(const std::vector<std::string>& recipients)
{
	_recipients.assign(recipients.begin(), recipients.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector<std::string>& MEmailPlugin::recipients() const
{
	return _recipients;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Applications
} // namespace Seiscomp
