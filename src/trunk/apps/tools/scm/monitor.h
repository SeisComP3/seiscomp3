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

#ifndef __SEISCOMP_APPLICATIONS_MONITOR_H__
#define __SEISCOMP_APPLICATIONS_MONITOR_H__

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

#include <seiscomp3/client/application.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/communication/clientstatus.h>
#include <seiscomp3/plugins/monitor/monitoroutplugininterface.h>
#include <seiscomp3/plugins/monitor/monitorplugininterface.h>
#include <seiscomp3/plugins/monitor/types.h>


namespace Seiscomp {
namespace Applications {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Monitor : public Client::Application {

	private:
		typedef std::map<std::string, Core::Time>                       ResponseTimes;


	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		Monitor(int argc, char** argv);
		~Monitor();


	// ------------------------------------------------------------------
	// Public Interface
	// ------------------------------------------------------------------
	public:
		virtual bool init();
		virtual void handleNetworkMessage(const Communication::NetworkMessage* msg);
		virtual void handleDisconnect();

		// Console output specific member functions
		void update();


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private:
		virtual void createCommandLineDescription();
		virtual bool validateParameters();
		virtual void handleTimeout();

		void printClientInfoTags() const;
		void handleStatusMessage(const Communication::NetworkMessage* msg);


	// ------------------------------------------------------------------
	// Private Members
	// ------------------------------------------------------------------
	private:
		std::string                               _clientsToConsiderStr;
		std::vector<std::string>                  _clientsToConsider;
		ClientTable                               _clientTable;
		ResponseTimes                             _responseTimes;
		std::vector<MonitorPluginInterfacePtr>    _plugins;
		std::vector<MonitorOutPluginInterfacePtr> _outPlugins;
		unsigned int                              _refreshInterval;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Application
} // namespace Seiscomp

#endif

