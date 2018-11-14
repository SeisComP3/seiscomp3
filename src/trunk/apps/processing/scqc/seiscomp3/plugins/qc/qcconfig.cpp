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



#define SEISCOMP_COMPONENT QCCONFIG
#include  <seiscomp3/logging/log.h>

#include<boost/tokenizer.hpp>

#include "qcconfig.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


using namespace std;


IMPLEMENT_SC_CLASS(QcConfig, "QcConfig");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcConfig::QcConfig()
: _app(0), _realtime(false), _buffer(4000), _archiveInterval(3600)
, _archiveBuffer(3600), _reportInterval(60), _reportBuffer(600)
, _reportTimeout(0), _alertInterval(0), _alertBuffer(60) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcConfig::QcConfig(QcApp *app, const string &pluginName)
: _app(app) {
	setQcConfig(pluginName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcConfig::~QcConfig() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string QcConfig::readConfig(const string& pluginName, const string& keyWord,
                            const string& defaultValue) const {
	if ( !_app )
		throw QcConfigException("No application instance given; can not retrieve config value");

	string config = "plugins." + pluginName + "." + keyWord;
	string value;

	SEISCOMP_DEBUG("     ***** qcConfig: %s *****", config.c_str());

	try {
		value = _app->configGetString(config);
		SEISCOMP_DEBUG("* reading qcConfig: %s = %s", config.c_str(), value.c_str());
	} catch (Config::Exception&) {
		config = "plugins.default." + keyWord;
		try {
			value = _app->configGetString(config);
			SEISCOMP_DEBUG("+ reading default qcConfig: %s = %s", config.c_str(), value.c_str());
		} catch (Config::Exception&) {
			value = defaultValue;
			SEISCOMP_DEBUG("0 using compiled-in default : %s = %s", config.c_str(), value.c_str());		
			}
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcConfig::setQcConfig(const string &pluginName) {
	string value;

	try {
		value = readConfig(pluginName,"realTimeOnly","false");
		_realtime = (value == "True" || value == "true") ? true : false;
	}
	catch ( QcConfigException& ) {
		throw;
	}

	value = readConfig(pluginName,"buffer","4000");
	_buffer = boost::lexical_cast<int>(value);


	value = readConfig(pluginName,"archive.interval","3600");
	_archiveInterval = boost::lexical_cast<int>(value);

	value = readConfig(pluginName,"archive.buffer","3600");
	_archiveBuffer = boost::lexical_cast<int>(value);


	value = readConfig(pluginName,"report.interval","60");
	_reportInterval = boost::lexical_cast<int>(value);

	value = readConfig(pluginName,"report.buffer","600");
	_reportBuffer =  boost::lexical_cast<int>(value);

	value = readConfig(pluginName,"report.timeout","0");
	_reportTimeout =  boost::lexical_cast<int>(value);


	value = readConfig(pluginName,"alert.interval","0");
	_alertInterval =  boost::lexical_cast<int>(value);

	value = readConfig(pluginName,"alert.buffer","60");
	_alertBuffer =  boost::lexical_cast<int>(value);


	value = readConfig(pluginName,"alert.thresholds","120");    
	boost::tokenizer<boost::char_separator<char> > ttok(value,boost::char_separator<char>(", "));
	for (boost::tokenizer<boost::char_separator<char> >::iterator it = ttok.begin(); it != ttok.end(); ++it)
		_alertThresholds.push_back(boost::lexical_cast<int>(*it));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcConfig::realtimeOnly() const {
	return _realtime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::buffer() const {
	return _buffer;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::archiveInterval() const {
	return _archiveInterval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::archiveBuffer() const {
	return (_archiveBuffer <= _buffer ? _archiveBuffer : _buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::reportInterval() const {
	return _reportInterval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::reportBuffer() const {
	return (_reportBuffer <= _buffer ? _reportBuffer : _buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::reportTimeout() const {
	if (!_app)
		throw QcConfigException("No application instance given; can not retrieve processing mode");

	if (_app->archiveMode())
		throw QcConfigException("Client runs in archive mode; report timeout only useable in real time mode!");

	return _reportTimeout;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::alertInterval() const {
	if (!_app)
		throw QcConfigException("No application instance given; can not retrieve processing mode");

	if (_app->archiveMode())
		throw QcConfigException("Client runs in archive mode; alert interval only useable in real time mode!");

	return _alertInterval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcConfig::alertBuffer() const {
	if (!_app)
		throw QcConfigException("No application instance given; can not retrieve processing mode");

	if (_app->archiveMode())
		throw QcConfigException("Client runs in archive mode; alert buffer only useable in real time mode!");

	return (_alertBuffer <= _buffer ? _alertBuffer : _buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<int> QcConfig::alertThresholds() const {
	if (!_app)
		throw QcConfigException("No application instance given; can not retrieve processing mode");

	if (_app->archiveMode())
		throw QcConfigException("Client runs in archive mode; alert thresholds only useable in real time mode!");

	return _alertThresholds;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcConfig::RealtimeOnly(const QcApp *app, const string &pluginName) {
	string value;
	string config = "plugins." + pluginName + ".realTimeOnly";

	try {
		value = app->configGetString(config);
	} catch (Config::Exception&) {
		config = "plugins.default.realTimeOnly";
		try {
			value = app->configGetString(config);
		} catch (Config::Exception&) {
			value = "false";
		}
	}

	return ((value == "True" || value == "true") ? true : false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
