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




#define SEISCOMP_COMPONENT Gui::QcView
#include <seiscomp3/logging/log.h>

#include "qcviewconfig.h"


namespace Seiscomp {
namespace Applications {
namespace Qc {

typedef std::vector<std::string> Strings;
typedef std::vector<double> Doubles;
typedef std::vector<int> Ints;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcViewConfig::QcViewConfig() {}

QcViewConfig::QcViewConfig(Gui::Application *app)
	: _app(app) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
bool QcViewConfig::init() {

	Range range_bad;
	range_bad.min = -999E99;
	range_bad.max = 0.0;
	range_bad.count = -1;
	range_bad.color = "red";
	range_bad.action = "";
	
	Range range_sane;
	range_sane.min = 0.0;
	range_sane.max = 999E99;
	range_sane.count = 0;
	range_sane.color = "green";
	range_sane.action = "";
	
	Config config;
	config.rangeMap.insert("bad", readRange("sane", range_sane));
	config.rangeMap.insert("sane", readRange("bad", range_bad));
	config.expire = 0;
	config.useAbsoluteValue = false;
	config.format = "float";
	config.color = "white";

	_configMap.insert("default", readConfig("default", config));

	Score score_default;
	score_default.rangeMap.insert("default", range_sane);

	_scoreMap.insert("default", score_default);
	readScore("default");

	_colorMap.insert("red", QColor(255,0,0,32));
	_colorMap.insert("green", QColor(0,255,0,32));
	_colorMap.insert("white", QColor(255,255,255,255));

	// create parameter map & set config
	try {
		Strings parameter = _app->configGetStrings("parameter");
		for (Strings::iterator it = parameter.begin(); it != parameter.end(); ++it) {
			QStringList sl = QString(it->c_str()).split(QRegExp(":"));
			QString parameter = sl.first().simplified();
			QString cfgName = sl.last().simplified();
			_parameterList.append(parameter);
			_parameterMap.insert(parameter, cfgName);
			_configMap.insert(cfgName, readConfig(cfgName.toAscii().data(), _configMap["default"]));
		}
	}
	catch (Seiscomp::Config::Exception &) {
		SEISCOMP_ERROR("QcViewConfig: no 'parameter' defined in cfg file!");
		return false;
	}

	try { _streamWidgetLength = _app->configGetDouble("streamWidget.length");}
	catch (Seiscomp::Config::Exception &) {_streamWidgetLength = 600.0;}

	try { _cumulative = _app->configGetBool("streams.cumulative");}
	catch (Seiscomp::Config::Exception &) {_cumulative = true;}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QcViewConfig::Config QcViewConfig::readConfig(const std::string& name, QcViewConfig::Config config) {

	try { config.expire = _app->configGetInt(name+".expire");}
	catch (Seiscomp::Config::Exception &) {}

	try { config.useAbsoluteValue = _app->configGetBool(name+".useAbsoluteValue");}
	catch (Seiscomp::Config::Exception &) {}

	try { config.format = QString(_app->configGetString(name+".format").c_str());}
	catch (Seiscomp::Config::Exception &) {}

	try { config.color = QString(_app->configGetString(name+".color").c_str());}
	catch (Seiscomp::Config::Exception &) {}

	try {
		Strings ranges = _app->configGetStrings(name+".ranges");
		config.rangeMap.clear();
		for (Strings::iterator it = ranges.begin(); it != ranges.end(); ++it) {
			config.rangeMap.insert(QString(it->c_str()), readRange(*it, Range()));
		}
	}
	catch (Seiscomp::Config::Exception &) {}
	
	RangeMap::iterator it;
	for (it = config.rangeMap.begin(); it != config.rangeMap.end(); ++it) {
		it.value() = readRange(it.key().toAscii().data(), it.value(), name+".");
	}

	return config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QcViewConfig::Range QcViewConfig::readRange(const std::string& name, QcViewConfig::Range range, std::string pre) {

	try { range.count = _app->configGetInt(pre+"range."+name+".count");}
	catch (Seiscomp::Config::Exception &) {}
	
	try { range.action = QString( _app->configGetString(pre+"range."+name+".action").c_str());}
	catch (Seiscomp::Config::Exception &) {}
	
	try { range.color = QString(_app->configGetString(pre+"range."+name+".color").c_str());
		readColor(range.color.toAscii().data());
	}
	catch (Seiscomp::Config::Exception &) {}

	try {
		Doubles dv = _app->configGetDoubles(pre+"range."+name);
		range.min = dv[0];
		range.max = dv[1];
	}
	catch (Seiscomp::Config::Exception &) {}

	return range;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void QcViewConfig::readColor(const std::string& name) {

	std::vector<int> cv;

	try { cv = _app->configGetInts("color."+name);}
	catch (Seiscomp::Config::Exception &) {}

	_colorMap.insert(QString(name.c_str()), QColor(cv[0], cv[1], cv[2], cv[3]));


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void QcViewConfig::readScore(const std::string& name) {

	Score score;

	Strings ranges;
	try { ranges = _app->configGetStrings("score."+name+".ranges");}
	catch ( Seiscomp::Config::Exception & ) {}

	for (Strings::iterator it = ranges.begin(); it != ranges.end(); ++it) {
		score.rangeMap.insert(QString(it->c_str()), readRange(*it, Range()));
	}

	_scoreMap.insert(QString(name.c_str()), score);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QStringList QcViewConfig::parameter() const {

	return _parameterList;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
bool QcViewConfig::cumulative() const {
	return _cumulative;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int QcViewConfig::streamWidgetLength() const {
	return _streamWidgetLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int QcViewConfig::column(const QString& parameterName) const {

	return _parameterMap.keys().indexOf(parameterName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
const QString& QcViewConfig::parameterName(int column) const {

	return _parameterList.at(column);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QColor QcViewConfig::color(const QString& parameterName, double value) const {

	QColor color = _colorMap[_configMap["default"].color];

	RangeMap rm = _configMap[_parameterMap[parameterName]].rangeMap;
	foreach (Range range, rm.values()) {
		if (value >= range.min && value <= range.max) {
			color = _colorMap[range.color];
		}
	}

	return color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int QcViewConfig::count(const QString& parameterName, double value) const {

	int count(0);

	RangeMap rm = _configMap[_parameterMap[parameterName]].rangeMap;
	foreach (Range range, rm.values()) {
		if (value >= range.min && value <= range.max) {
			count = range.count;
		}
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QString QcViewConfig::format(const QString& parameterName, double value) const {

	QString format = _configMap[_parameterMap[parameterName]].format;
	
	// format according to given format string
	if (format == "float") {
		return QString().setNum(value, 'f', 2);
	}
	
	if (format == "int") {
		return QString().setNum(int(value));
	}

	if (format == "timeSpan") {
		return formatTimeSpan(value);
	}

	if (format == "percent") {
		return QString().setNum(int(value)) + "%";
	}


	// raw data
	return QString().number(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
bool QcViewConfig::useAbsoluteValue(const QString& parameterName) const {

	return _configMap[_parameterMap[parameterName]].useAbsoluteValue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
bool QcViewConfig::expired(const QString& parameterName, double dt) const {

	double expire = _configMap[_parameterMap[parameterName]].expire;

	if (expire > 0.0)
		return dt>=expire?true:false;
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QColor QcViewConfig::sumColor(const QString& scoreName, int score) const {

	QColor color;

	RangeMap rm = _scoreMap[scoreName].rangeMap;
	foreach (Range range, rm.values()) {
		if (score >= range.min && score <= range.max) {
			color = _colorMap[range.color];
		}
	}

	return color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString QcViewConfig::formatTimeSpan(double rs) const {
	char buffer[256];
	
	double s = fabs(rs);

	char sign = rs>=0?' ':'-';

	if (s < 60.0) {
		sprintf(buffer, "%c%.1f s", sign, s);
		return QString(buffer);
	}

	if (s >= 60.0 && s < 3600.0) {
		int min = (int)(s/60);
		double sec = s-min*60;
		sprintf(buffer, "%c%d m %.1f s", sign, min, sec); 
		return QString(buffer);
	}

	if (s >= 3600.0 && s < 86400.0) {
		int hour = (int)(s/3600);
		int min = (int)(s-hour*3600)/60;
		sprintf(buffer, "%c%d h %d m", sign, hour, min); 
		return QString(buffer);
	}

	if (s >= 3600.0 && s < 7*86400.0) {
		int day = (int)(s/86400);
		int hour = (int)(s-day*86400)/3600;
		sprintf(buffer, "%c%d %s %d h", sign, day, day==1?"day":"days", hour); 
		return QString(buffer);
	}

	if (s >= 7*86400.0 && s < 30.5*86400.0) {
		int week = (int)(s/(7*86400));
		int day = (int)(s-week*7*86400)/86400;
		sprintf(buffer, "%c%d %s %d d", sign, week, week==1?"week":"weeks", day); 
		return QString(buffer);
	}

	if (s >= 30.5*86400.0 && s < 365.0*86400.0) {
		int month = (int)(s/(30.5*86400));
		sprintf(buffer, "%c%d %s", sign, month, month==1?"month":"months"); 
		return QString(buffer);
	}

	int year = (int)(s/(365.0*86400));
	sprintf(buffer, "%c%d %s", sign, year, year==1?"year":"years"); 
	return QString(buffer);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}
}
}
