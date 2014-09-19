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


#ifndef __SEISCOMP_CLIENT_CONFIG_H__
#define __SEISCOMP_CLIENT_CONFIG_H__


#include <seiscomp3/datamodel/config.h>
#include <seiscomp3/datamodel/configmodule.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/databasereader.h>
#include <seiscomp3/client.h>

#include <map>
#include <set>


namespace Seiscomp {
namespace Client {


class SC_SYSTEM_CLIENT_API ConfigDB {
	private:
		ConfigDB();

	public:
		static ConfigDB* Instance();

		void load(Seiscomp::DataModel::DatabaseReader* reader,
			const OPT(std::string)& moduleName = Seiscomp::Core::None,
			const OPT(std::string)& networkCode = Seiscomp::Core::None,
			const OPT(std::string)& stationCode = Seiscomp::Core::None,
			const OPT(std::string)& setupName = Seiscomp::Core::None,
			const std::set<std::string>& parameterNames = std::set<std::string>());

		void load(const char *xml);

		Seiscomp::DataModel::Config* config();

	private:
		std::map<int, Seiscomp::DataModel::ConfigModulePtr> _configModules;
		std::map<int, Seiscomp::DataModel::ConfigStationPtr> _configStations;
		std::map<int, Seiscomp::DataModel::ParameterSetPtr> _parameterSets;
		Seiscomp::DataModel::ConfigPtr _config;
		static ConfigDB *_instance;

	Seiscomp::DataModel::DatabaseIterator getConfigObjects(Seiscomp::DataModel::DatabaseReader* reader,
		const Seiscomp::Core::RTTI& classType,
   		const OPT(std::string)& moduleName,
		const OPT(std::string)& networkCode,
		const OPT(std::string)& stationCode,
		const OPT(std::string)& setupName,
		const std::set<std::string>& parameterNames);
};


}
}


#endif

