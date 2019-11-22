/***************************************************************************
 *   Copyright (C) gempa GmbH                                              *
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


#ifndef __SEISCOMP_QL2SC_CONFIG_H__
#define __SEISCOMP_QL2SC_CONFIG_H__

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace Seiscomp {
namespace QL2SC {

typedef std::map<std::string, std::string> RoutingTable;

struct HostConfig {
	std::string  host;
	std::string  url;
	bool         gzip;
	bool         native;
	int          options;
	std::string  filter;
	RoutingTable routingTable;
	bool         syncEventAttributes;
	bool         syncPreferred;
};

typedef std::vector<HostConfig> HostConfigs;

struct Config {
	bool init();
	void format(std::stringstream &ss, const RoutingTable &table) const;

	int          batchSize;
	size_t       backLog;
	int          maxWaitForEventIDTimeout;
	HostConfigs  hosts;
};

} // ns QL2SC
} // ns Seiscomp

#endif // __SEISCOMP_QL2SC_CONFIG_H__
