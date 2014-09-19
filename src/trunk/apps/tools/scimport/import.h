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



#ifndef __SEISCOMP_COMMUNICATION_IMPORT_H__
#define __SEISCOMP_COMMUNICATION_IMPORT_H__


#include <string>
#include <map>
#include <boost/thread/thread.hpp>

#include <seiscomp3/client/application.h>


namespace Seiscomp {
namespace Applications {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Import : public Client::Application {

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
private:
	enum Mode {
	    RELAY = 0x0,
	    IMPORT
	};

	
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
public:
	Import(int argc, char* argv[]);
	~Import();

	
	// ------------------------------------------------------------------
	// Private interface
	// ------------------------------------------------------------------
private:
	virtual bool init();
	virtual void createCommandLineDescription();
	virtual void handleNetworkMessage(const Communication::NetworkMessage* msg);
	virtual void done();
	
	int connectToSink(const std::string& sink);
	bool filterObject(Core::BaseObject* obj);
	bool buildImportRoutingtable();
	void buildRelayRoutingtable(bool routeUnknownGroup = false);
	void readSinkMessages();

	// ------------------------------------------------------------------
	// Private implementation
	// ------------------------------------------------------------------
private:
	typedef std::map<std::string, std::string> RoutingTable;
	Communication::SystemConnectionPtr _sink;
	RoutingTable _routingTable;
	boost::thread *_sinkMessageThread;

	
	bool _filter;
	bool _routeUnknownGroup;
	Mode _mode;
	bool _useSpecifiedGroups;
	bool _test;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namepsace Applications
} // namespace Seiscomp

#endif
