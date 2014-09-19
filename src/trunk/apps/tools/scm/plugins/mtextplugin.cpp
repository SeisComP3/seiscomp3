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

#define SEISCOMP_COMPONENT MTextPlugin
#include <seiscomp3/logging/log.h>

#include "mtextplugin.h"

#include <fstream>
#include <memory>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/client/application.h>



namespace Seiscomp {
namespace Applications {

IMPLEMENT_SC_CLASS_DERIVED(MTextPlugin,
                           MonitorOutPluginInterface,
                           "MTextPlugin");

REGISTER_MONITOR_OUT_PLUGIN_INTERFACE(MTextPlugin, "mtextplugin");
ADD_SC_PLUGIN(
		"Plugin for text output",
		"GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 1, 0, 0)



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MTextPlugin::MTextPlugin() :
	MonitorOutPluginInterface("mtextplugin"),
	_init(false),
	_descrFileName("description.txt")
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MTextPlugin::print(const ClientTable& table)
{
	// Write the order of the tags into a description file
	if ( !_init ) {
		if ( table.size() > 0 ) {
			std::string fileName = _outputDir + _descrFileName;
			std::ofstream of(fileName.c_str(), std::ios_base::trunc);
			if ( !of.fail() && of.is_open() ) {
				const ClientInfoData& info = table.begin()->info;
				ClientInfoData::const_iterator infoIt = info.begin();
				for ( ; infoIt != info.end(); ++infoIt ) {
					if ( infoIt != info.begin() )
						of << "\t";
					of << infoIt->first.toString();
				}
				_init = !_init;
			}
			else {
				SEISCOMP_ERROR("MTextPlugin: Could not write description file");
			}
		}
	}

	// Check if all clients are present otherwise remove file
	Clients::iterator it = _clients.begin();
	for ( ; it != _clients.end(); ++it ) {
		bool found = false;
		ClientTable::const_iterator clientIt = table.begin();
		for ( ; clientIt != table.end(); ++clientIt ) {
			ClientInfoData::const_iterator infoIt = clientIt->info.find(Communication::CLIENTNAME_TAG);
			if ( infoIt != clientIt->info.end() ) {
				if ( infoIt->second == it->first ) {
					found = !found;
					break;
				}
			}
			else {
				SEISCOMP_DEBUG("MTextPlugin: Could not find CLIENT_NAME_TAG");
			}
		}
		if ( !found ) {
			SEISCOMP_DEBUG("MTextPlugin: Removing file: %s", it->second.c_str());
			boost::filesystem::remove(it->second);
			_clients.erase(it);
		}
	}

	// Find the recently updated clients (response == 0) and append them
	// to a status file.
	ClientTable::const_iterator clientIt = table.begin();
	for ( ; clientIt != table.end(); ++clientIt ) {
		const ClientInfoData& info = clientIt->info;
		ClientInfoData::const_iterator infoIt = info.find(Communication::RESPONSE_TIME_TAG);
		if ( infoIt == info.end() ) {
			SEISCOMP_ERROR("MTextPlugin: could not find RESPONSE_TIME_TAG");
			continue;
		}

		infoIt = info.find(Communication::CLIENTNAME_TAG);
		if ( infoIt == info.end() ) {
			SEISCOMP_ERROR("MTextPlugin: Could not find CLIENT_NAME_TAG");
			continue;
		}

		std::string fileName = _outputDir + infoIt->second + ".txt";
		if ( _clients.find(infoIt->second) == _clients.end() ) {
			_clients.insert(std::make_pair(infoIt->second, fileName));
			SEISCOMP_DEBUG("MTextPlugin: Writing status to file: %s", fileName.c_str());
		}

		std::ofstream of(fileName.c_str(), std::ios_base::trunc);
		if ( of.fail() || !of.is_open() ) {
			SEISCOMP_ERROR("MTextPlugin: Could not open file: %s", fileName.c_str());
			continue;
		}

		infoIt = clientIt->info.begin();
		for ( ; infoIt != clientIt->info.end(); ++infoIt) {
			if ( infoIt != info.begin() )
				of << "\t";
			of << infoIt->second;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MTextPlugin::initOut(const Config::Config& cfg)
{
	try {
		_outputDir = Environment::Instance()->absolutePath(SCCoreApp->configGetString(name() + ".outputDir"));
		if ( *_outputDir.rbegin() != '/' )
			_outputDir += "/";
	}
	catch ( Config::Exception& e ) {
		Environment* env = Environment::Instance();
		_outputDir = env->logDir() + "/" + SCCoreApp->name() + "/";
		SEISCOMP_DEBUG("Basepath: %s", _outputDir.c_str());

		SC_FS_DECLARE_PATH(p, _outputDir);

		if ( boost::filesystem::exists(p) )
			boost::filesystem::remove_all(p);

		if ( !boost::filesystem::create_directory(p) ) {
			SEISCOMP_ERROR("MTextPlugin: Could not create log directory: %s", _outputDir.c_str());
			return false;
		}
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MTextPlugin::deinitOut()
{
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MTextPlugin::refreshOut()
{
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MTextPlugin::clearOut()
{
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Applications
} // namespace Sesicomp
