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


#define SEISCOMP_COMPONENT CommandLine

#include <seiscomp3/client/commandline.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/logging/log.h>

#include <boost/program_options/parsers.hpp>
#include <iostream>
#include <fstream>


using namespace std;
using namespace boost::program_options;


namespace Seiscomp {
namespace Client {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommandLine::CommandLine() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommandLine::options_description*
CommandLine::findGroup(const char* group, const char* option) const {
	if ( group == NULL ) return NULL;

	program_options_map::const_iterator it = _groupsMap.find(group);
	if ( it == _groupsMap.end() ) {
		if ( option )
			SEISCOMP_WARNING("Commandline group '%s' not found -> parameter '%s' ignored",
			                 group, option);
		return NULL;
	}

	return (*it).second.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::addGroup(const char* name) {
	if ( _groupsMap.find(name) != _groupsMap.end() ) return;
	program_options_ptr po = program_options_ptr(new options_description(name));
	_groupsMap[name] = po;
	_groups.push_back(po);
	//_commandLineOptions.add(po);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description) {
	options_description* o = findGroup(group, option);

	if ( o )
		(options_description_easy_init(o))(option, description);
		//o->add_options()(option, description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommandLine::parse(int argc, char** argv) {
	if ( !argc ) return true;

	_options = program_options_ptr(new options_description());

	for ( program_options_list::const_iterator it = _groups.begin();
	      it != _groups.end(); ++it )
		_options->add(*(*it));

	try {
		parsed_options parsed = command_line_parser(argc, argv).options(*_options).allow_unregistered().run();
		store(parsed, _variableMap, false);

		// NOTE: To enable a configuration file for commandline options uncomment the following lines
		//       of code
		/*
		ifstream ifs((Environment::Instance()->configDir() + "/seiscomp3.cfg").c_str());
     	if (ifs) {
			std::cerr << "Using " << Environment::Instance()->configDir() << "/seiscomp3.cfg" << std::endl;
			store(parse_config_file(ifs, *_options), _variableMap);
		}
		*/

		notify(_variableMap);

		_unrecognizedOptions = collect_unrecognized(parsed.options, include_positional);
	}
	catch ( std::exception& e ) {
		cout << "Error: " << e.what() <<  endl;
		cout << *_options << endl;
    	return false;
	}

	notify(_variableMap);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommandLine::hasOption(const std::string& option) const {
	std::map<std::string, boost::program_options::variable_value>::
	const_iterator it = _variableMap.find(option);
	if ( it == _variableMap.end() ) return false;
	return !it->second.defaulted();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<std::string> CommandLine::unrecognizedOptions() const {
	return _unrecognizedOptions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::printOptions() const {
	if ( _options )
		cout << *_options << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
