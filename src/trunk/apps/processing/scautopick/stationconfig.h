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


#ifndef __APPS_PICKER_STATIONCONFIG_H__
#define __APPS_PICKER_STATIONCONFIG_H__

#include <seiscomp3/datamodel/configmodule.h>
#include <seiscomp3/utils/keyvalues.h>

#include <string>
#include <map>


namespace Seiscomp {
namespace Applications {
namespace Picker {


struct StreamConfig {
	StreamConfig();
	StreamConfig(double on, double off, double tcorr,
	             const std::string &f);

	std::string  locCode;
	std::string  channel;

	OPT(double)  triggerOn;
	OPT(double)  triggerOff;
	OPT(double)  timeCorrection;
	bool         enabled;

	std::string  filter;

	bool         updatable;

	Util::KeyValuesPtr parameters;
};


class StationConfig {
	public:
		typedef std::pair<std::string, std::string> Key;
		typedef std::map<Key, StreamConfig> ConfigMap;
		typedef ConfigMap::const_iterator const_iterator;


	public:
		StationConfig();

		void setDefault(const StreamConfig &entry);

		const StreamConfig *read(const Seiscomp::Config::Config *config, const std::string &mod,
		                         DataModel::ParameterSet *params,
		                         const std::string &net, const std::string &sta);

		void read(const Seiscomp::Config::Config *config, const DataModel::ConfigModule *module,
		          const std::string &setup);

		const StreamConfig *get(const Seiscomp::Config::Config *config, const std::string &mod,
		                        const std::string &net, const std::string &sta);

		const_iterator begin() const;
		const_iterator end() const;

		void dump() const;


	private:
		StreamConfig _default;
		ConfigMap _stationConfigs;
};


}
}
}


#endif
