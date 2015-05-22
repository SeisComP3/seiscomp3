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
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#define SEISCOMP_COMPONENT QL2SC

#include "config.h"
#include "quakelink.h"

#include <seiscomp3/client/application.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/reading.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/logging/log.h>

using namespace std;

namespace Seiscomp {
namespace QL2SC {

bool Config::init() {
	Client::Application *app = SCCoreApp;
	if ( app == NULL ) return false;

	SEISCOMP_INFO("reading configuration");

	// back log seconds
	try {
		int i = app->configGetInt("backLog");
		backLog = i < 0 ? 0 : (size_t)i;
	}
	catch ( ... ) { backLog = 1800; }

	// number of cached objects
	try {
		int i = app->configGetInt("cacheSize");
		cacheSize = i < 0 ? 0 : (size_t)i;
	}
	catch ( ... ) { cacheSize = 5000; }

	// maximum number of notifiers per message
	try { batchSize = app->configGetInt("batchSize"); }
	catch ( ... ) { batchSize = 200; }

	// host configurations
	hosts.clear();
	vector<string> hostNames, routings;

	SEISCOMP_INFO("reading host configuration");
	try { hostNames = app->configGetStrings("hosts"); }
	catch ( ... ) {}
	if ( hostNames.empty() ) {
		SEISCOMP_ERROR("could not read host list");
		return false;
	}

	for ( vector<string>::const_iterator it = hostNames.begin();
	      it != hostNames.end(); ++it ) {
		HostConfig cfg;
		string prefix = "host." + *it + ".";

		// host
		cfg.host = *it;

		// URL
		try { cfg.url = app->configGetString(prefix + "url"); }
		catch ( ... ) { cfg.url = "ql://localhost:18010"; }

		// gzip
		try { cfg.gzip = app->configGetBool(prefix + "gzip"); }
		catch ( ... ) { cfg.gzip = false; }

		// data options
		bool isSet;
		string dataPrefix = prefix + "data.";
		cfg.options = IO::QuakeLink::opIgnore;

		try { isSet = app->configGetBool(dataPrefix + "picks"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataPicks;

		try { isSet = app->configGetBool(dataPrefix + "amplitudes"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataAmplitudes;

		try { isSet = app->configGetBool(dataPrefix + "arrivals"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataArrivals;

		try { isSet = app->configGetBool(dataPrefix + "staMags"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataStaMags;

		try { isSet = app->configGetBool(dataPrefix + "staMts"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataStaMts;

		try { isSet = app->configGetBool(dataPrefix + "preferred"); }
		catch ( ... ) { isSet = true; }
		if ( isSet ) cfg.options |= IO::QuakeLink::opDataPreferred;

		// keep alive messages
		try { isSet = app->configGetBool(prefix + "keepAlive"); }
		catch ( ... ) { isSet = false; }
		if ( isSet ) cfg.options |= Seiscomp::IO::QuakeLink::opKeepAlive;

		// filter
		try { cfg.filter = app->configGetString(prefix + "filter"); }
		catch ( ... ) {}

		// routing table
		try {
			routings = app->configGetStrings(prefix + "routingTable");
			vector<string> toks;
			for ( vector<string>::iterator it = routings.begin();
			      it != routings.end(); ++it ) {
				Core::split(toks, it->c_str(), ":");
				if ( toks.size() != 2 ) {
					SEISCOMP_ERROR("Malformed routing table entry: %s", it->c_str());
					return false;
				}
				cfg.routingTable[toks[0]] = (toks[1] == "NULL" ? "" : toks[1]);
			}
		}
		catch ( ... ) {
			cfg.routingTable[DataModel::Pick::TypeInfo().className()] = Communication::Protocol::IMPORT_GROUP;
			cfg.routingTable[DataModel::Amplitude::TypeInfo().className()] = Communication::Protocol::IMPORT_GROUP;
			cfg.routingTable[DataModel::Origin::TypeInfo().className()] = "LOCATION";
			cfg.routingTable[DataModel::StationMagnitude::TypeInfo().className()] = "MAGNITUDE";
			cfg.routingTable[DataModel::Magnitude::TypeInfo().className()] = "MAGNITUDE";
		}

		// create explicit routing entries for top-level EventParameters
		// children
		RoutingTable::const_iterator rit = cfg.routingTable.find(DataModel::EventParameters::TypeInfo().className());
		if ( rit != cfg.routingTable.end() && !rit->second.empty() ) {
			if ( cfg.routingTable[DataModel::Pick::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::Pick::TypeInfo().className()] = rit->second;
			if ( cfg.routingTable[DataModel::Amplitude::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::Amplitude::TypeInfo().className()] = rit->second;
			if ( cfg.routingTable[DataModel::Reading::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::Reading::TypeInfo().className()] = rit->second;
			if ( cfg.routingTable[DataModel::Origin::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::Origin::TypeInfo().className()] = rit->second;
			if ( cfg.routingTable[DataModel::FocalMechanism::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::FocalMechanism::TypeInfo().className()] = rit->second;
			if ( cfg.routingTable[DataModel::Event::TypeInfo().className()].empty() )
				cfg.routingTable[DataModel::Event::TypeInfo().className()] = rit->second;
		}

		hosts.push_back(cfg);

		stringstream ss;
		format(ss, cfg.routingTable);
		SEISCOMP_DEBUG("Read host configuration '%s':\n"
		               "  url         : %s\n"
		               "  gzip        : %s\n"
		               "  data\n"
		               "    picks     : %s\n"
		               "    amplitudes: %s\n"
		               "    arrivals  : %s\n"
		               "    staMags   : %s\n"
		               "    staMts    : %s\n"
		               "    preferred : %s\n"
		               "  keepAlive   : %s\n"
		               "  filter      : %s\n"
		               "  routing     : %s\n",
		              it->c_str(),
		               cfg.url.c_str(),
		               cfg.gzip                                      ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataPicks      ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataAmplitudes ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataArrivals   ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataStaMags    ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataStaMts     ? "true" : "false",
		               cfg.options & IO::QuakeLink::opDataPreferred  ? "true" : "false",
		               cfg.options & IO::QuakeLink::opKeepAlive      ? "true" : "false",
		               cfg.filter.c_str(),
		               ss.str().c_str());
	}

	return true;
}

void Config::format(stringstream &ss, const RoutingTable &table) const {
	bool first = true;
	for ( RoutingTable::const_iterator it = table.begin();
	      it != table.end(); ++it, first = false) {
		if ( !first ) ss << ", ";
		ss << it->first << ":" << it->second;
	}
}

} // ns QL2SC
} // ns Seiscomp
