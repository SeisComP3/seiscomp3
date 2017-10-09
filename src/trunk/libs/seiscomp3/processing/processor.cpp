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


#define SEISCOMP_COMPONENT Processing

#include <seiscomp3/logging/log.h>
#include <seiscomp3/processing/processor.h>
#include <seiscomp3/config/config.h>


using namespace std;


namespace Seiscomp {

namespace Processing {

IMPLEMENT_SC_ABSTRACT_CLASS(Processor, "Processor");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Processor::Processor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Processor::~Processor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Processor::setup(const Settings &settings) {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Settings::Settings(const string &mod,
                   const string &network,
                   const string &station,
                   const string &location,
                   const string &stream,
                   const Config::Config *config,
                   const Util::KeyValues *keys)
: module(mod), networkCode(network), stationCode(station),
  locationCode(location), channelCode(stream),
  localConfiguration(config), keyParameters(keys) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define ROOT_CONFIG_KEY "module."
string Settings::getString(const string &parameter) const {
	std::string value;
	if ( !getValue(value, parameter) )
		throw Config::OptionNotFoundException(parameter);
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Settings::getInt(const string &parameter) const {
	int value;
	if ( !getValue(value, parameter) )
		throw Config::OptionNotFoundException(parameter);
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Settings::getDouble(const string &parameter) const {
	double value;
	if ( !getValue(value, parameter) )
		throw Config::OptionNotFoundException(parameter);
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Settings::getBool(const string &parameter) const {
	bool value;
	if ( !getValue(value, parameter) )
		throw Config::OptionNotFoundException(parameter);
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Settings::getValue(std::string &value, const std::string &parameter) const {
	if ( localConfiguration != NULL ) {
		if ( localConfiguration->getString(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + stationCode + "." + parameter) )
			return true;

		if ( localConfiguration->getString(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + parameter) )
			return true;

		if ( localConfiguration->getString(value, string(ROOT_CONFIG_KEY) + module + ".global." + parameter) )
			return true;
	}

	if ( keyParameters && keyParameters->getString(value, parameter) )
		return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Settings::getValue(int &value, const std::string &parameter) const {
	if ( localConfiguration != NULL ) {
		if ( localConfiguration->getInt(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + stationCode + "." + parameter) )
			return true;

		if ( localConfiguration->getInt(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + parameter) )
			return true;

		if ( localConfiguration->getInt(value, string(ROOT_CONFIG_KEY) + module + ".global." + parameter) )
			return true;
	}

	if ( keyParameters && keyParameters->getInt(value, parameter) )
		return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Settings::getValue(double &value, const std::string &parameter) const {
	if ( localConfiguration != NULL ) {
		if ( localConfiguration->getDouble(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + stationCode + "." + parameter) )
			return true;

		if ( localConfiguration->getDouble(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + parameter) )
			return true;

		if ( localConfiguration->getDouble(value, string(ROOT_CONFIG_KEY) + module + ".global." + parameter) )
			return true;
	}

	if ( keyParameters && keyParameters->getDouble(value, parameter) )
		return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Settings::getValue(bool &value, const std::string &parameter) const {
	if ( localConfiguration != NULL ) {
		if ( localConfiguration->getBool(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + stationCode + "." + parameter) )
			return true;

		if ( localConfiguration->getBool(value, string(ROOT_CONFIG_KEY) + module + "." + networkCode + "." + parameter) )
			return true;

		if ( localConfiguration->getBool(value, string(ROOT_CONFIG_KEY) + module + ".global." + parameter) )
			return true;
	}

	if ( keyParameters && keyParameters->getBool(value, parameter) )
		return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
