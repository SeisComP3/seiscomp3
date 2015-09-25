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


#include <seiscomp3/client/configdb.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/parameter.h>

#include <iostream>


namespace Seiscomp {
namespace Client {

ConfigDB *ConfigDB::_instance = NULL;


ConfigDB::ConfigDB() {
	_config = new Seiscomp::DataModel::Config();
}


ConfigDB* ConfigDB::Instance() {
	if ( _instance == NULL )
		_instance = new ConfigDB();

	return _instance;
}

#define _T(name) reader->driver()->convertColumnName(name)


Seiscomp::DataModel::DatabaseIterator ConfigDB::getConfigObjects(Seiscomp::DataModel::DatabaseReader* reader,
	const Seiscomp::Core::RTTI& classType,
	const OPT(std::string)& moduleName,
	const OPT(std::string)& networkCode,
	const OPT(std::string)& stationCode,
	const OPT(std::string)& setupName,
	const std::set<std::string>& parameterNames) {

	std::ostringstream query;

	if ( classType.isTypeOf(Seiscomp::DataModel::ConfigStation::TypeInfo()) ) {
		query << "SELECT DISTINCT PublicObject." << _T("publicID") << ",ConfigStation.* FROM ConfigStation LEFT JOIN PublicObject USING (_oid)";
		query << " LEFT JOIN Setup ON Setup._parent_oid=ConfigStation._oid";
		query << " LEFT JOIN PublicObject AS PParameterSet ON PParameterSet." << _T("publicID") << "=Setup." << _T("parameterSetID");
		query << " LEFT JOIN ConfigModule ON (ConfigModule._oid=ConfigStation._parent_oid OR ConfigModule." << _T("parameterSetID") << "=PParameterSet." << _T("publicID") << ")";
		if ( parameterNames.size() > 0) {
			query << " LEFT JOIN ParameterSet ON ParameterSet._oid=PParameterSet._oid";
			query << " LEFT JOIN Parameter ON Parameter._parent_oid=ParameterSet._oid";
		}

	}
	else if ( classType.isTypeOf(Seiscomp::DataModel::Setup::TypeInfo()) ) {
		query << "SELECT DISTINCT Setup.* FROM Setup ";
		query << " LEFT JOIN PublicObject AS PParameterSet ON PParameterSet." << _T("publicID") << "=Setup." << _T("parameterSetID");
		query << " LEFT JOIN ConfigStation ON ConfigStation._oid=Setup._parent_oid";
		query << " LEFT JOIN ConfigModule ON (ConfigModule._oid=ConfigStation._parent_oid OR ConfigModule." << _T("parameterSetID") << "=PParameterSet." << _T("publicID") << ")";
		if ( parameterNames.size() > 0) {
			query << " LEFT JOIN ParameterSet ON ParameterSet._oid=PParameterSet._oid";
			query << " LEFT JOIN Parameter ON Parameter._parent_oid=ParameterSet._oid";
		}
	}
	else if ( classType.isTypeOf(Seiscomp::DataModel::ParameterSet::TypeInfo()) ) {
		query << "SELECT DISTINCT PublicObject." << _T("publicID") << ",ParameterSet.* FROM ParameterSet LEFT JOIN PublicObject USING (_oid)";
		query << " LEFT JOIN Setup ON Setup." << _T("parameterSetID") << "=PublicObject." << _T("publicID");
		query << " LEFT JOIN ConfigStation ON ConfigStation._oid=Setup._parent_oid";
		query << " LEFT JOIN ConfigModule ON (ConfigModule._oid=ConfigStation._parent_oid OR ConfigModule." << _T("parameterSetID") << "=PublicObject." << _T("publicID") << ")";
		query << " LEFT JOIN Parameter ON Parameter._parent_oid=ParameterSet._oid";
	}
	else if ( classType.isTypeOf(Seiscomp::DataModel::Parameter::TypeInfo()) ) {
		query << "SELECT DISTINCT PublicObject." << _T("publicID") << ",Parameter.* FROM Parameter LEFT JOIN PublicObject USING (_oid)";
		query << " LEFT JOIN ParameterSet ON ParameterSet._oid=Parameter._parent_oid ";
		query << " LEFT JOIN PublicObject AS PParameterSet ON PParameterSet._oid=ParameterSet._oid ";
		query << " LEFT JOIN Setup ON Setup." << _T("parameterSetID") << "=PParameterSet." << _T("publicID");
		query << " LEFT JOIN ConfigStation ON ConfigStation._oid=Setup._parent_oid ";
		query << " LEFT JOIN ConfigModule ON (ConfigModule._oid=ConfigStation._parent_oid OR ConfigModule." << _T("parameterSetID") << "=PParameterSet." << _T("publicID") << ")";
	}

	bool first = true;

	if ( moduleName ) {
		if ( first ) { query << " WHERE "; first = false; }
		else query << " AND ";
		query << "ConfigModule." << _T("name") << "='" << *moduleName << "' ";
	}
	if ( networkCode ) {
		if ( first ) { query << " WHERE "; first = false; }
		else query << " AND ";
		query << "ConfigStation." << _T("networkCode") << "='" << *networkCode << "' ";
	}
	if ( stationCode ) {
		if ( first ) { query << " WHERE "; first = false; }
		else query << " AND ";
		query << "ConfigStation." << _T("stationCode") << "='" << *stationCode << "' ";
	}
	if ( setupName ) {
		if ( first ) { query << " WHERE "; first = false; }
		else query << " AND ";
		query << "Setup." << _T("name") << "='" << *setupName << "' ";
	}
	if ( parameterNames.size() != 0 ) {
		if ( first ) { query << " WHERE "; first = false; }
		else query << " AND ";
		std::set<std::string>::const_iterator it = parameterNames.begin();
		query << "(Parameter." << _T("name") << "='" << *it << "'";
		while(++it != parameterNames.end())
			query << " OR Parameter." << _T("name") << "='" << *it << "'";
		query << ") ";
	}

	return reader->getObjectIterator(query.str(), classType);
}

void ConfigDB::load(Seiscomp::DataModel::DatabaseReader* reader,
	const OPT(std::string)& moduleName,
	const OPT(std::string)& networkCode,
	const OPT(std::string)& stationCode,
	const OPT(std::string)& setupName,
	const std::set<std::string>& parameterNames) {

	if ( reader == NULL ) return;

	Seiscomp::DataModel::DatabaseIterator it;

	it = reader->getObjects(_config.get(), Seiscomp::DataModel::ConfigModule::TypeInfo());
	for ( Seiscomp::DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		Seiscomp::DataModel::ConfigModulePtr configModule = Seiscomp::DataModel::ConfigModule::Cast(obj);
		if ( configModule ) {
			if ( (moduleName && (configModule->name() == *moduleName)) || !moduleName ) {
//				std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << configModule->name() << std::endl;
				Seiscomp::DataModel::ConfigModulePtr existing = _config->findConfigModule(configModule->publicID());
				if (existing) {
					_configModules.insert(std::make_pair(it.oid(), existing));
				}
				else {
					_configModules.insert(std::make_pair(it.oid(), configModule));
					_config->add(configModule.get());
				}
			}
		}
	}

	it.close();

	if ( _configModules.empty() ) return;

	it = getConfigObjects(reader, Seiscomp::DataModel::ConfigStation::TypeInfo(), moduleName,
		networkCode, stationCode, setupName, parameterNames);

	for ( Seiscomp::DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		Seiscomp::DataModel::ConfigStationPtr configStation = Seiscomp::DataModel::ConfigStation::Cast(obj);
		if ( configStation ) {
//			std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << configStation->networkCode() << ", " << configStation->stationCode() << std::endl;
			std::map<int, Seiscomp::DataModel::ConfigModulePtr>::iterator p = _configModules.find(it.parentOid());
			if ( p != _configModules.end() ) {
				Seiscomp::DataModel::ConfigStationPtr existing = p->second->findConfigStation(configStation->publicID());
				if (existing) {
					_configStations.insert(std::make_pair(it.oid(), existing));
				}
				else {
					_configStations.insert(std::make_pair(it.oid(), configStation));
					p->second->add(configStation.get());
				}
			}
			else {
				std::cout << "cannot find parent object" << it.parentOid() << std::endl;
			}
		}
	}

	it.close();

	//if ( parameterNames.size() == 0 )
	//	return;

	it = getConfigObjects(reader, Seiscomp::DataModel::Setup::TypeInfo(), moduleName,
		networkCode, stationCode, setupName, parameterNames);

	for ( Seiscomp::DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		Seiscomp::DataModel::SetupPtr setup = Seiscomp::DataModel::Setup::Cast(obj);
		if ( setup ) {
//			std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << setup->name() << std::endl;
			std::map<int, Seiscomp::DataModel::ConfigStationPtr>::iterator p = _configStations.find(it.parentOid());
			if ( p != _configStations.end() ) {
				if ( !p->second->setup(setup->name()) )
					p->second->add(setup.get());
			}
			else {
				std::cout << "cannot find parent object" << it.parentOid() << std::endl;
			}
		}
	}

	it.close();

	it = getConfigObjects(reader, Seiscomp::DataModel::ParameterSet::TypeInfo(), moduleName,
		networkCode, stationCode, setupName, parameterNames);

	for ( Seiscomp::DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		Seiscomp::DataModel::ParameterSetPtr parameterSet = Seiscomp::DataModel::ParameterSet::Cast(obj);
		if ( parameterSet ) {
//			std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << parameterSet->publicID() << std::endl;
			Seiscomp::DataModel::ParameterSetPtr existing = _config->findParameterSet(parameterSet->publicID());
			if (existing) {
				_parameterSets.insert(std::make_pair(it.oid(), existing));
			}
			else {
				_parameterSets.insert(std::make_pair(it.oid(), parameterSet));
				_config->add(parameterSet.get());
			}
		}
	}

	it.close();

	it = getConfigObjects(reader, Seiscomp::DataModel::Parameter::TypeInfo(), moduleName,
		networkCode, stationCode, setupName, parameterNames);

	for ( Seiscomp::DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		Seiscomp::DataModel::ParameterPtr parameter = Seiscomp::DataModel::Parameter::Cast(obj);
		if ( parameter ) {
//			std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << parameter->name() << std::endl;
			std::map<int, Seiscomp::DataModel::ParameterSetPtr>::iterator p = _parameterSets.find(it.parentOid());
			if ( p != _parameterSets.end() )	{
				if ( !p->second->findParameter(parameter->publicID()) )
					p->second->add(parameter.get());
			}
			else {
				std::cout << "cannot find parent object" << it.parentOid() << std::endl;
			}
		}
	}

	it.close();
}


void ConfigDB::load(const char *xml) {
	IO::XMLArchive ar;

	if ( !ar.open(xml) )
		throw Core::GeneralException(std::string(xml) + " not found");

	ar >> _config;
	ar.close();
}


Seiscomp::DataModel::Config* ConfigDB::config() {
	return _config.get();
}


}
}

