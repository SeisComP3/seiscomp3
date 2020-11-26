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





#define SEISCOMP_COMPONENT Autopick

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/processing/processor.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ctype.h>

#include "stationconfig.h"


#ifdef _MSC_VER
#define isWhiteSpace isspace
#else
#define isWhiteSpace std::isspace
#endif


namespace {


void parseToks(std::vector<std::string> &toks, const std::string &line) {
	size_t lit = 0;
	size_t lastit = lit;
	std::string tok;
	bool skipWhitespaces = false;

	while ( lit < line.size() ) {
		if ( isWhiteSpace(line[lit]) && !skipWhitespaces ) {
			if ( !tok.empty() ) {
				toks.push_back(tok);
				tok.clear();
			}

			++lit;
			while ( lit < line.size() && isWhiteSpace(line[lit]) )
				++lit;
			lastit = lit;
		}
		else if ( line[lit] == '\"' ) {
			if ( skipWhitespaces ) {
				skipWhitespaces = false;
				if ( !tok.empty() ) {
					toks.push_back(tok);
					tok.clear();
				}

				++lit;
				while ( lit < line.size() && isWhiteSpace(line[lit]) )
					++lit;
				lastit = lit;
			}
			else {
				skipWhitespaces = true;
				++lit;
				lastit = lit;
			}
		}
		else {
			tok += line[lit];
			++lit;
		}
	}

	if ( lastit != lit ) toks.push_back(line.substr(lastit, lit-lastit));
}


}


namespace Seiscomp {
namespace Applications {
namespace Picker {


StreamConfig::StreamConfig() : sensitivityCorrection(false), enabled(true) {}


StreamConfig::StreamConfig(double on, double off, double tcorr, const std::string &f)
: triggerOn(on)
, triggerOff(off)
, timeCorrection(tcorr)
, sensitivityCorrection(false)
, enabled(true)
, filter(f) {}


StationConfig::StationConfig() {}


void StationConfig::setDefault(const StreamConfig &entry) {
	_default = entry;
}


const StreamConfig *
StationConfig::read(const Seiscomp::Config::Config *config, const std::string &mod,
                    DataModel::ParameterSet *params,
                    const std::string &net, const std::string &sta) {
	std::string loc, cha, filter = _default.filter;
	double trigOn = *_default.triggerOn, trigOff = *_default.triggerOff;
	double tcorr = *_default.timeCorrection;
	bool sensitivityCorrection = false;
	bool enabled = true;
	Util::KeyValuesPtr keys;

	if ( params ) {
		keys = new Util::KeyValues;
		keys->init(params);
	}

	Processing::Settings settings(mod, net, sta, "", "", config, keys.get());

	settings.getValue(loc, "detecLocid");
	settings.getValue(cha, "detecStream");
	settings.getValue(trigOn, "trigOn");
	settings.getValue(trigOff, "trigOff");
	settings.getValue(tcorr, "timeCorr");
	if ( settings.getValue(filter, "detecFilter") ) {
		if ( filter.empty() )
			filter = _default.filter;
	}
	settings.getValue(sensitivityCorrection, "sensitivityCorrection");
	settings.getValue(enabled, "detecEnable");

	StreamConfig &sc = _stationConfigs[Key(net,sta)];
	sc.updatable = true;
	sc.locationCode = loc;
	sc.channel = cha;
	sc.triggerOn = trigOn;
	sc.triggerOff = trigOff;
	sc.timeCorrection = tcorr;
	sc.filter = filter;
	sc.enabled = enabled;
	sc.sensitivityCorrection = sensitivityCorrection;
	sc.parameters = keys;
	return &sc;
}


void StationConfig::read(const Seiscomp::Config::Config *localConfig,
                         const DataModel::ConfigModule *module,
                         const std::string &setupName) {
	if ( module == NULL ) return;

	for ( size_t j = 0; j < module->configStationCount(); ++j ) {
		DataModel::ConfigStation *station = module->configStation(j);
		DataModel::Setup *configSetup = DataModel::findSetup(station, setupName, false);

		if ( configSetup ) {
			DataModel::ParameterSet* ps = NULL;
			try {
				ps = DataModel::ParameterSet::Find(configSetup->parameterSetID());
			}
			catch ( Core::ValueException & ) {
				continue;
			}

			if ( !ps ) {
				SEISCOMP_ERROR("Cannot find parameter set %s",
				               configSetup->parameterSetID().c_str());
				continue;
			}
	
			read(localConfig, module->name(), ps,
			     station->networkCode(), station->stationCode());
		}
	}
}


const StreamConfig *
StationConfig::get(const Seiscomp::Config::Config *config, const std::string &mod,
                   const std::string &net, const std::string &sta) {
	ConfigMap::const_iterator it;
	it = _stationConfigs.find(Key(net,sta));
	if ( it != _stationConfigs.end() )
		return &it->second;

	return NULL;
}


StationConfig::const_iterator StationConfig::begin() const {
	return _stationConfigs.begin();
}


StationConfig::const_iterator StationConfig::end() const {
	return _stationConfigs.end();
}


void StationConfig::dump() const {
	printf("Stream configuration:\n");
	ConfigMap::const_iterator it = _stationConfigs.begin();
	for ( ; it != _stationConfigs.end(); ++it ) {
		if ( it->first.first == "*" || it->first.second == "*" ) continue;
		if ( !it->second.enabled ) printf("#");
		std::string streamID;
		if ( it->second.channel.empty() )
			streamID = it->first.first + "." + it->first.second;
		else
			streamID = it->first.first + "." + it->first.second + "." + it->second.locationCode + "." + it->second.channel;
		printf("%-16s", streamID.c_str());
		if ( it->second.triggerOn )
			printf("%.2f ", *it->second.triggerOn);
		else
			printf("? ");

		if ( it->second.triggerOff )
			printf("%.2f ", *it->second.triggerOff);
		else
			printf("? ");

		if ( it->second.timeCorrection )
			printf("%.2f ", *it->second.timeCorrection);
		else
			printf("? ");

		if ( !it->second.filter.empty() )
			printf("%s", it->second.filter.c_str());
		else
			printf("?");

		printf("\n");
	}
}


}
}
}
