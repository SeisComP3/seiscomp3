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

#include "monitor.h"

#include <sstream>
#include <iterator>

#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/communication/clientstatus.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/core/strings.h>


namespace Seiscomp {
namespace Applications {

using namespace Communication;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DEFINE_PLUGIN_ITERATOR(Interface) \
	PluginIterator<Interface, Interface##Ptr, Interface##Factory>

#define DEFINE_PLUGIN_ITERATOR_PARAM(Interface) \
	PluginIterator<Interface, Interface##Ptr, Interface##Factory>

#define DEFINE_PLUGIN_ITERATOR_ITERATOR(Interface) \
	PluginIterator<Interface, Interface##Ptr, Interface##Factory>::iterator

#define DEFINE_PLUGIN_ITERATOR_CITERATOR(Interface) \
	PluginIterator<Interface, Interface##Ptr, Interface##Factory>::const_iterator


template <typename Interface,
		  typename InterfacePtr,
		  typename InterfaceFactory
>
class PluginIterator {
	public:
		typedef std::vector<Interface*> Plugins;
		typedef typename Plugins::iterator iterator;
		typedef typename Plugins::const_iterator const_iterator;


	public:
		PluginIterator(const char* name)
		 : _name(name) {
			_operational = init();
		}

		PluginIterator(const char* name, const std::vector<std::string>& pluginNames)
		 : _name(name) {
			_operational = init(pluginNames.begin(), pluginNames.end());
		}

		bool isOperational() { return _operational; }

		iterator begin() { return _plugins.begin(); }
		iterator end() { return _plugins.end(); }
		const_iterator begin() const { return _plugins.begin(); }
		const_iterator end() const { return _plugins.end(); }

		std::vector<std::string>::iterator namesBegin() { return _pluginNames.begin(); }
		std::vector<std::string>::iterator namesEnd() { return _pluginNames.end(); }
		std::vector<std::string>::const_iterator namesBegin() const { return _pluginNames.begin(); }
		std::vector<std::string>::const_iterator namesEnd() const { return _pluginNames.end(); }


	private:
		bool init() {
			std::auto_ptr<typename InterfaceFactory::ServiceNames> tmpPluginNames;
			tmpPluginNames = std::auto_ptr<typename InterfaceFactory::ServiceNames>(InterfaceFactory::Services());
			if ( !tmpPluginNames.get() ) {
				SEISCOMP_INFO("%s hosts no services", _name);
				return false;
			}
			return init(tmpPluginNames->begin(), tmpPluginNames->end());
		}

		template <typename NameIterator>
		bool init(NameIterator begin, NameIterator end) {
			for ( ; begin != end; ++begin ) {
				_pluginNames.push_back(*begin);

				Interface* obj = InterfaceFactory::Create((*begin).c_str());
				if ( !obj ) {
					SEISCOMP_ERROR("Could not create plugin: %s", (*begin).c_str());
					return false;
				}
				_plugins.push_back(obj);
			}
			return true;
		}


	private:
		 std::vector<std::string> _pluginNames;
		 Plugins _plugins;
		 const char* _name;
		 bool _operational;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Monitor::Monitor(int argc, char** argv)
 : Client::Application(argc, argv),
   _refreshInterval(3) {

	setDatabaseEnabled(false, false);
	// addLoggingComponentSubscription("ScMonitor");
	setLoadConfigModuleEnabled(false);
	setMessagingUsername("");
	setPrimaryMessagingGroup(Protocol::LISTENER_GROUP);
	addMessagingSubscription("STATUS_GROUP");
	addPluginPackagePath("monitor");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Monitor::~Monitor() {
	for ( size_t i = 0; i < _outPlugins.size(); ++i )
		_outPlugins[i]->deinitOut();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::createCommandLineDescription() {
	Application::createCommandLineDescription();
	Client::CommandLine& cl = commandline();
	std::string clGroupName = "monitor";
	cl.addGroup(clGroupName.c_str());
	cl.addOption(clGroupName.c_str(), "clients,c", "Just monitor given clients", &_clientsToConsiderStr);
	cl.addOption(clGroupName.c_str(), "print-tags", "Print available keys for accessing client info data");
	cl.addOption(clGroupName.c_str(), "no-output-plugins", "Do not use output plugins");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Monitor::validateParameters() {
	if ( commandline().hasOption("print-tags") ) {
		printClientInfoTags();
		return false;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::printClientInfoTags() const {
	std::cout << "= Client info tags =" << std::endl;
	for ( size_t i = 0; i < Communication::EConnectionInfoTagQuantity; ++i ) {
		std::cout << Communication::EConnectionInfoTagNames::name(i) << std::endl;
	}
	std::cout << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Monitor::init() {
	if ( !Application::init() ) return false;

	if ( !_clientsToConsiderStr.empty() ) {
		Core::split(_clientsToConsider, _clientsToConsiderStr.c_str(), ",");
		std::vector<std::string>::iterator it = _clientsToConsider.begin();
		for ( ; it != _clientsToConsider.end(); ++it )
			Core::trim(*it);
	}

	// Initialize output plugins
	if ( !commandline().hasOption("no-output-plugins") ) {
		DEFINE_PLUGIN_ITERATOR(MonitorOutPluginInterface) outPluginFactory("MonitorOutPluginInterfaceFactory");
		if ( outPluginFactory.isOperational() ) {
			DEFINE_PLUGIN_ITERATOR_ITERATOR(MonitorOutPluginInterface) it = outPluginFactory.begin();
			for ( ; it != outPluginFactory.end(); ++it )
				_outPlugins.push_back(*it);
		}
	}

	DEFINE_PLUGIN_ITERATOR(MonitorPluginInterface) pluginFactory("MonitorPluginInterfaceFactory");
	if ( pluginFactory.isOperational() ) {
		DEFINE_PLUGIN_ITERATOR_ITERATOR(MonitorPluginInterface) it;
		it = pluginFactory.begin();
		for ( ; it != pluginFactory.end(); ++it ) {
			if ( (*it)->init(configuration()) )
				_plugins.push_back(*it);
			else
				SEISCOMP_ERROR("Plugin %s not operational, skipped", (*it)->name().c_str());
		}
	}

	for ( size_t i = 0; i < _outPlugins.size(); ++i )
		_outPlugins[i]->initOut(configuration());

	update();
	enableTimer(_refreshInterval);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::handleNetworkMessage(const NetworkMessage* msg) {
	if ( !_clientsToConsider.empty() ) {
		std::vector<std::string>::iterator found;
		found = std::find(_clientsToConsider.begin(), _clientsToConsider.end(), msg->clientName());
		if ( found == _clientsToConsider.end() ) return;
	}

	if ( msg->type() == Protocol::STATE_OF_HEALTH_RESPONSE_MSG ) {
		handleStatusMessage(msg);

		for ( size_t i = 0; i < _plugins.size(); ++i )
			_plugins[i]->process(_clientTable);

		for (ClientTable::iterator it = _clientTable.begin();
		     it != _clientTable.end(); ++it )
			it->processed = true;
	}
	else if ( msg->type() == Protocol::CLIENT_DISCONNECTED_MSG ) {
		ClientTable::iterator clientsIt = _clientTable.begin();
		for ( ; clientsIt != _clientTable.end(); ++clientsIt)
			if (clientsIt->info[CLIENTNAME_TAG] == msg->clientName())
				break;

		if ( clientsIt != _clientTable.end() ) {
			ResponseTimes::iterator responseTimesIt = _responseTimes.find(msg->clientName());
			if ( responseTimesIt != _responseTimes.end() )
				_responseTimes.erase(responseTimesIt);
			_clientTable.erase(clientsIt);
		}
	}
	else if ( msg->type() == Protocol::MASTER_DISCONNECTED_MSG ) {
		_clientTable.clear();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::handleDisconnect() {
	_clientTable.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::handleStatusMessage(const NetworkMessage* msg) {
	std::auto_ptr<Communication::ClientStatus> val(Communication::ClientStatus::CreateClientStatus(msg->data()));
	if ( !val.get() ) return;


	ClientInfoData tmpClientInfoData = val->clientInfoData();
	ClientTable::iterator clientIt = _clientTable.begin();
	for ( ; clientIt != _clientTable.end(); ++clientIt ) {
		if ( clientIt->info[CLIENTNAME_TAG] == msg->clientName() ) {
			*clientIt = tmpClientInfoData;
			break;
		}
	}

	if ( clientIt == _clientTable.end() )
		_clientTable.push_back(tmpClientInfoData);

	_responseTimes[val->clientName()] = Core::Time::GMT();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::handleTimeout() {
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Monitor::update() {
	// Update response time
	for ( ClientTable::iterator it = _clientTable.begin(); it != _clientTable.end(); ++it )
		it->info[RESPONSE_TIME_TAG] =
			Core::toString<int>((Core::Time::GMT() - _responseTimes[it->info[CLIENTNAME_TAG]]).seconds());

	for (size_t i = 0; i < _outPlugins.size(); ++i)
		_outPlugins[i]->print(_clientTable);

	for (ClientTable::iterator it = _clientTable.begin();
	     it != _clientTable.end(); ++it )
		it->printed = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





} // namespace Applications
} // namespace Seiscomp

