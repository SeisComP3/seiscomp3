/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/



#define SEISCOMP_COMPONENT Hypo71
#define HYPOINFODEBUG "OFF"
#define PICKINFODEBUG "OFF"

#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/vector3.h>
#include <seiscomp3/utils/files.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <math.h>


#include "configfile.h"
#include "hypo71.h"


#define IQ 2
#define IPUN 1
#define IPRN 1
#define KSORT ""
#define MSG_HEADER "[plugin] [Hypo71]"


ADD_SC_PLUGIN("HYPO71 seismic hypocenter locator plugin", "IPGP <www.ipgp.fr>", 0, 1, 1)


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Seismology;



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int stringLength = sizeof(alphanum) - 1;

char genRandom() {
	return alphanum[rand() % stringLength];
}

const string genRandomString(const size_t& length) {

	string alpha;
	for (size_t i = 0; i < length; ++i)
		alpha += alphanum[rand() % stringLength];

	return alpha;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



REGISTER_LOCATOR(Hypo71, "Hypo71");

Hypo71::IDList Hypo71::_allowedParameters;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Hypo71::Hypo71() {

	_name = "Hypo71";
	_publicIDPattern = "Hypo71.@time/%Y%m%d%H%M%S.%f@.@id@";
	_allowMissingStations = false;

	if ( _allowedParameters.empty() ) {
		_allowedParameters.push_back("TEST(01)");
		_allowedParameters.push_back("TEST(02)");
		_allowedParameters.push_back("TEST(03)");
		_allowedParameters.push_back("TEST(04)");
		_allowedParameters.push_back("TEST(05)");
		_allowedParameters.push_back("TEST(06)");
		_allowedParameters.push_back("TEST(10)");
		_allowedParameters.push_back("TEST(11)");
		_allowedParameters.push_back("TEST(12)");
		_allowedParameters.push_back("TEST(13)");
		_allowedParameters.push_back("TEST(15)");
		_allowedParameters.push_back("TEST(20)");
		_allowedParameters.push_back("CRUSTAL_VELOCITY_MODEL");
		_allowedParameters.push_back("CRUSTAL_DEPTH_MODEL");
		_allowedParameters.push_back("ZTR");
		_allowedParameters.push_back("XNEAR");
		_allowedParameters.push_back("XFAR");
		_allowedParameters.push_back("POS");
		_allowedParameters.push_back("KAZ");
		_allowedParameters.push_back("DISABLE_LAST_LOC");
		_allowedParameters.push_back("KNST");
		_allowedParameters.push_back("INST");
	}

	for (IDList::iterator it = _allowedParameters.begin();
	        it != _allowedParameters.end(); ++it)
		_parameters[*it] = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Hypo71::~Hypo71() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Hypo71::init(const Config::Config& config) {

	bool result = true;
	_currentProfile = NULL;

	SEISCOMP_DEBUG("%s -----------------------------------------------------------------", MSG_HEADER);
	SEISCOMP_DEBUG("%s |                    CONFIGURATION PARAMETERS                   |", MSG_HEADER);
	SEISCOMP_DEBUG("%s -----------------------------------------------------------------", MSG_HEADER);

	try {
		_publicIDPattern = config.getString("hypo71.publicID");
	}
	catch ( ... ) {
		_publicIDPattern = "Hypo71.@time/%Y%m%d%H%M%S.%f@.@id@";
	}
	try {
		_useHypo71PatternID = config.getBool("hypo71.useHypo71PublicID");
	}
	catch ( ... ) {
		_useHypo71PatternID = false;
	}

	try {
		_logFile = config.getString("hypo71.logFile");
		SEISCOMP_DEBUG("%s | logFile              | %s", MSG_HEADER, _logFile.c_str());
	}
	catch ( ... ) {
		_logFile = "HYPO71.LOG";
		SEISCOMP_ERROR("%s |   logFile            | DEFAULT value: %s",
		    MSG_HEADER, _logFile.c_str());
	}

	try {
		_h71inputFile = config.getString("hypo71.inputFile");
		SEISCOMP_DEBUG("%s | inputFile            | %s", MSG_HEADER, _h71inputFile.c_str());
	}
	catch ( ... ) {
		_h71inputFile = "HYPO71.INP";
		SEISCOMP_ERROR("%s | inputFile            | DEFAULT value: %s",
		    MSG_HEADER, _h71inputFile.c_str());
	}

	try {
		_h71outputFile = config.getString("hypo71.outputFile");
		SEISCOMP_DEBUG("%s | outputFile           | %s", MSG_HEADER, _h71outputFile.c_str());
	}
	catch ( ... ) {
		_h71outputFile = "HYPO71.PRT";
		SEISCOMP_ERROR("%s | outputFile           | DEFAULT value: %s",
		    MSG_HEADER, _h71outputFile.c_str());
	}

	try {
		_controlFilePath = config.getString("hypo71.defaultControlFile");
		SEISCOMP_DEBUG("%s | defaultControlFile   | %s", MSG_HEADER, _controlFilePath.c_str());
	}
	catch ( ... ) {
		_controlFilePath = "";
		SEISCOMP_ERROR("%s | defaultControlFile   | DEFAULT value: %s",
		    MSG_HEADER, _controlFilePath.c_str());
	}
	if ( !Util::fileExists(_controlFilePath) ) {
		SEISCOMP_ERROR("%s | defaultControlFile   | %s does not exist",
		    MSG_HEADER, _controlFilePath.c_str());
		result = false;
	}

	try {
		_hypo71ScriptFile = config.getString("hypo71.hypo71ScriptFile");
		SEISCOMP_DEBUG("%s | hypo71ScriptFile     | %s", MSG_HEADER,
		    _hypo71ScriptFile.c_str());
	}
	catch ( ... ) {
		SEISCOMP_ERROR("%s | hypo71ScriptFile     | can't read value", MSG_HEADER);
	}

	if ( !Util::fileExists(_hypo71ScriptFile) ) {
		SEISCOMP_ERROR("%s | hypo71ScriptFile     | %s does not exist",
		    MSG_HEADER, _hypo71ScriptFile.c_str());
		result = false;
	}


	_profileNames.clear();
	try {
		_profileNames = config.getStrings("hypo71.profiles");
	}
	catch ( ... ) {
		SEISCOMP_ERROR("%s CONFIGURATION FILE IS NOT CORRECTLY IMPLEMENTED!!", MSG_HEADER);
	}

	for ( IDList::iterator it = _profileNames.begin();
	      it != _profileNames.end(); ) {

		Profile prof;

		string prefix = string("hypo71.profile.") + *it + ".";

		prof.name = *it;
		SEISCOMP_DEBUG("%s | NEW PROFILE !!       | %s", MSG_HEADER, prof.name.c_str());
		try {
			prof.earthModelID = config.getString(prefix + "earthModelID");
			SEISCOMP_DEBUG("%s |   earthModelID       | %s", MSG_HEADER, prof.earthModelID.c_str());
		}
		catch ( ... ) {
			SEISCOMP_ERROR("%s |   earthModelID       | can't read value", MSG_HEADER);
		}

		try {
			prof.methodID = config.getString(prefix + "methodID");
			SEISCOMP_DEBUG("%s |   methodID           | %s", MSG_HEADER, prof.methodID.c_str());
		}
		catch ( ... ) {
			prof.methodID = "hypo71";
			SEISCOMP_ERROR("%s |   methodID           | DEFAULT value: %s",
			    MSG_HEADER, prof.methodID.c_str());
		}

		try {
			prof.controlFile = config.getString(prefix + "controlFile");
			SEISCOMP_DEBUG("%s |   configFile         | %s", MSG_HEADER, prof.controlFile.c_str());
		}
		catch ( ... ) {
			SEISCOMP_ERROR("%s |   configFile         | can't read value", MSG_HEADER);
		}

		try {
			prof.fixStartDepthOnly = config.getBool(prefix + "fixStartDepthOnly");
		}
		catch ( ... ) {
			prof.fixStartDepthOnly = false;
		}

		if ( prof.controlFile.empty() )
			prof.controlFile = _controlFilePath;

		if ( !Util::fileExists(prof.controlFile) ) {
			SEISCOMP_ERROR("%s |   configFile         | file %s does not exist",
			    MSG_HEADER, prof.controlFile.c_str());
			it = _profileNames.erase(it);
			result = false;
			continue;
		}

		_profiles.push_back(prof);
		++it;
	}

	_profileNames.insert(_profileNames.begin(), "SELECT PROFILE");

	SEISCOMP_DEBUG("%s -----------------------------------------------------------------", MSG_HEADER);
	try {
		updateProfile(config.getString("hypo71.defaultProfile"));
	}
	catch ( ... ) {
		updateProfile("");
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Hypo71::IDList Hypo71::parameters() const {
	return _allowedParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Hypo71::parameter(const string& name) const {

	ParameterMap::const_iterator it = _parameters.find(name);
	if ( it != _parameters.end() )
		return it->second;

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Hypo71::setParameter(const string& name, const string& value) {

	ParameterMap::iterator it = _parameters.find(name);
	if ( it == _parameters.end() )
		return false;

	it->second = value;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Hypo71::stripSpace(string& str) {

	for (size_t i = 0; i < str.length(); i++)
		if ( str[i] == ' ' ) {
			str.erase(i, 1);
			i--;
		}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool Hypo71::stringIsOfType(const string& str,
                                  const StringType& type) {

	int i;
	float f;
	switch ( type ) {
		case stInteger:
			if ( sscanf(str.c_str(), "%f", &f) != 0 )
				return true;
		break;
		case stDouble:
			if ( sscanf(str.c_str(), "%d", &i) != 0 )
				return true;
		break;
		default:
			return false;
		break;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int Hypo71::toInt(const string& str) {

	int value;
	istringstream iss(str);
	iss >> value;

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::getSituation(const double& value,
                                  const GeographicPosition& gp) {

	string pos;
	if ( value < 0 ) {
		switch ( gp ) {
			case gpLatitude:
				pos = "S";
			break;
			case gpLongitude:
				pos = "W";
			break;
			default:
				pos = "-";
			break;
		}
	}
	else {
		switch ( gp ) {
			case gpLatitude:
				pos = "N";
			break;
			case gpLongitude:
				pos = "E";
			break;
			default:
				pos = "-";
			break;
		}
	}

	return pos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::h71DecimalToSexagesimal(const double& value,
                                             const GeographicPosition& gp) {

	string output;
	char dec[5];
	char itaper[3];
	double i = floor(abs(value));
	double d = (abs(value) - i) * 60;
	d = floor(d * 100) / 100;

	switch ( gp ) {
		case gpLatitude:
			sprintf(itaper, "%02.0f", i);
		break;
		case gpLongitude:
			sprintf(itaper, "%03.0f", i);
		break;
		default:
			SEISCOMP_ERROR("%s %s failed - No geographic type specified",
			    MSG_HEADER, __func__);
		break;
	}

	sprintf(dec, "%#05.2f", d);
	output = toString(itaper) + toString(dec);

	return output;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::SexagesimalToDecimalHypo71(const double& deg,
                                                const double& min,
                                                const string& pol) {

	string value;
	double x = min / 60;
	double y = abs(deg) + x;

	if ( (pol == "S") or (pol == "W") or (pol == "s") or (pol == "w") )
		y = -y;

	value = toString(y);

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Hypo71::IDList Hypo71::profiles() const {
	return _profileNames;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Hypo71::setProfile(const string& name) {

	if ( find(_profileNames.begin(), _profileNames.end(), name) == _profileNames.end() )
		return;
	else
		updateProfile(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Hypo71::stringExplode(string str, string separator,
                           vector<string>* results) {

	int found;
	found = str.find_first_of(separator);
	while ( found != (signed) string::npos ) {
		if ( found > 0 )
			results->push_back(str.substr(0, found));
		str = str.substr(found + 1);
		found = str.find_first_of(separator);
	}

	if ( str.length() > 0 )
		results->push_back(str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Hypo71::capabilities() const {
	return FixedDepth | DistanceCutOff;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double Hypo71::toDouble(const string& s) {

	stringstream ss(s);
	double f;
	ss >> f;

	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double Hypo71::getTimeValue(const PickList& pickList,
                                  const string& networkCode,
                                  const string& stationCode,
                                  const string& phaseCode,
                                  unsigned int rtype) {

	double time = -1.;
	for (PickList::const_iterator i = pickList.begin();
	        i != pickList.end(); ++i) {

		PickPtr p = i->first;

		if ( p->phaseHint().code() != phaseCode )
			continue;

		if ( p->waveformID().networkCode() != networkCode )
			continue;

		if ( p->waveformID().stationCode() != stationCode )
			continue;

		switch ( rtype ) {
			case 0:
				time = p->time().value();
			break;
			case 1:
				time = toDouble(p->time().value().toString("%H"));
			break;
			case 2:
				time = toDouble(p->time().value().toString("%M"));
			break;
			case 3:
				time = toDouble(p->time().value().toString("%S.%f"));
			break;
			default:
				time = p->time().value();
			break;
		}
	}

	return time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::getPickPolarity(const PickList& pickList,
                                     const string& networkCode,
                                     const string& stationCode,
                                     const string& phaseCode) {

	string polarity = " ";
	for (PickList::const_iterator i = pickList.begin();
	        i != pickList.end(); ++i) {

		PickPtr p = i->first;

		if ( p->phaseHint().code() != phaseCode )
			continue;

		if ( p->waveformID().networkCode() != networkCode )
			continue;

		if ( p->waveformID().stationCode() != stationCode )
			continue;

		try {
			if ( p->polarity() == POSITIVE )
				polarity = "U";
			else if ( p->polarity() == NEGATIVE )
				polarity = "D";
		}
		catch ( ... ) {}
	}

	return polarity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string
Hypo71::formatString(string toFormat, const size_t& nb, const size_t& pos,
                     const string& sender) throw (Core::GeneralException) {

	if ( toFormat.size() > nb ) {
		SEISCOMP_ERROR("%s Can't format string %s : length(%d) > length(%d) [sender: %s]",
		    MSG_HEADER, toFormat.c_str(), (int ) toFormat.size(), (int ) nb, sender.c_str());
		throw LocatorException("SeisComP internal error\nTrying to fit " +
		        toString(toFormat.size()) + " chars into a " + toString(nb) + " chars variable");
	}

	size_t count;
	string blank;
	count = nb - toFormat.size();
	if ( count > 0 ) {

		while ( blank.size() < count )
			blank += " ";

		switch ( pos ) {
			case 0:
				// add blank space before
				toFormat = blank + toFormat;
			break;
			case 1:
				//! add blank space after
				toFormat += blank;
			break;
			default:
				SEISCOMP_ERROR("%s formatString >> wrong position passed as argument", MSG_HEADER);
			break;
		}
	}

	return toFormat;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int Hypo71::getH71Weight(const PickList& pickList,
                               const string& networkCode,
                               const string& stationCode,
                               const string& phaseCode,
                               const double& max) {

	int weight;
	double upper = 0, lower = 0;
	string pickID;

	for ( PickList::const_iterator it = pickList.begin();
	      it != pickList.end(); ++it ) {

		PickPtr pick = it->first;
		weight = it->second;

		if ( pick->phaseHint().code() != phaseCode )
			continue;

		if ( pick->waveformID().networkCode() != networkCode )
			continue;

		if ( pick->waveformID().stationCode() != stationCode )
			continue;

		pickID = pick->publicID();
		try {
			upper = pick->time().upperUncertainty();
		}
		catch ( ... ) {}
		try {
			lower = pick->time().lowerUncertainty();
		}
		catch ( ... ) {}

		// Pick found, break the loop
		break;
	}

	if ( pickID != "" ) {
		if ( weight != 1.0 )
			weight = 4;
		else
			weight = (int) round((3 / max) * (upper + lower));
	}

	return weight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Hypo71::locate(PickList& pickList) throw (Core::GeneralException) {

	ofstream log(_logFile.c_str(), ios::app);

	/* time log's buffer */
	time_t rawtime;
	struct tm* timeinfo;
	char head[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(head, 80, "%F %H:%M:%S [log] ", timeinfo);

	//! Reset trial hypocenter position
	_trialLatDeg = "";
	_trialLatMin = "";
	_trialLonDeg = "";
	_trialLonMin = "";

	string calculatedZTR;
	if ( _usingFixedDepth == true )
		calculatedZTR = toString(round(_fixedDepth));
	else
		calculatedZTR = getZTR(pickList);

	vector<string> Tvelocity;
	vector<string> Tdepth;

	//! Not really used but may be useful to store debug log
	_lastWarning = "";

	if ( pickList.empty() )
		SEISCOMP_ERROR("%s The phases list is empty", MSG_HEADER);

	if ( !_currentProfile )
		throw GeneralException("No profile selected! please do so !");

	if ( _usingFixedDepth )
		SEISCOMP_DEBUG("%s Fixed depth value; %s", MSG_HEADER, toString(round(_fixedDepth)).c_str());
	else
		SEISCOMP_DEBUG("%s Proceeding to localization without fixed depth", MSG_HEADER);


	// Cutting off stations which previous station-hypocenter distance
	// is higher than distanceCutOff parameter value
	if ( _enableDistanceCutOff )
		SEISCOMP_DEBUG("%s Cutoff distance value; %s", MSG_HEADER, toString(_distanceCutOff).c_str());
	else
		SEISCOMP_DEBUG("%s Proceeding to localization without distance cut off", MSG_HEADER);


	/*----------------------------------*
	 | DEFAULT CONFIGURATION PARAMETERS |
	 |            (default.hypo71.conf) |
	 *----------------------------------*/
	ResetList dRL;
	ControlCard dCC;
	InstructionCard dIC;
	ConfigFile dConfig(_controlFilePath);

	// Reset list values
	dConfig.readInto(dRL.test01, "TEST(01)");
	dConfig.readInto(dRL.test02, "TEST(02)");
	dConfig.readInto(dRL.test03, "TEST(03)");
	dConfig.readInto(dRL.test04, "TEST(04)");
	dConfig.readInto(dRL.test05, "TEST(05)");
	dConfig.readInto(dRL.test06, "TEST(06)");
	dConfig.readInto(dRL.test07, "TEST(07)");
	dConfig.readInto(dRL.test08, "TEST(08)");
	dConfig.readInto(dRL.test09, "TEST(09)");
	dConfig.readInto(dRL.test10, "TEST(10)");
	dConfig.readInto(dRL.test11, "TEST(11)");
	dConfig.readInto(dRL.test12, "TEST(12)");
	dConfig.readInto(dRL.test13, "TEST(13)");
	dConfig.readInto(dRL.test15, "TEST(15)");
	dConfig.readInto(dRL.test20, "TEST(20)");

	// Control card values
	dConfig.readInto(dCC.ztr, "ZTR");
	dConfig.readInto(dCC.xnear, "XNEAR");
	dConfig.readInto(dCC.xfar, "XFAR");
	dConfig.readInto(dCC.pos, "POS");
	dConfig.readInto(dCC.kfm, "KFM");
	dCC.kaz = "";
	dCC.kms = "0";
	dCC.imag = "";
	dCC.ir = "";
	dCC.ksel = "";
	string _iq = "2";
	dConfig.readInto(dCC.iq, "IQ", _iq);
	string _ipun = "1";
	dConfig.readInto(dCC.ipun, "IPUN", _ipun);
	string _iprn = "0";
	dConfig.readInto(dCC.iprn, "IPRN", _iprn);
	dCC.ksort = "";

	// Default instruction card values
	dConfig.readInto(dIC.ipro, "IPRO");
	dConfig.readInto(dIC.knst, "KNST");
	dConfig.readInto(dIC.inst, "INST");
	dConfig.readInto(dIC.zres, "ZRES");
	dIC.ipro = "";
	dIC.zres = "";


	/*---------------------------------*
	 | CUSTOM CONFIGURATION PARAMETERS |
	 |                (profile.*.conf) |
	 *---------------------------------*/
	ResetList cRL;
	FullControlCard cCC;
	InstructionCard cIC;
	ConfigFile pConfig(_currentProfile->controlFile);


	// Reset list values
	pConfig.readInto(cRL.test01, "TEST(01)", dRL.test01);
	pConfig.readInto(cRL.test02, "TEST(02)", dRL.test02);
	pConfig.readInto(cRL.test03, "TEST(03)", dRL.test03);
	pConfig.readInto(cRL.test04, "TEST(04)", dRL.test04);
	pConfig.readInto(cRL.test05, "TEST(05)", dRL.test05);
	pConfig.readInto(cRL.test06, "TEST(06)", dRL.test06);
	pConfig.readInto(cRL.test07, "TEST(07)", dRL.test07);
	pConfig.readInto(cRL.test08, "TEST(08)", dRL.test08);
	pConfig.readInto(cRL.test09, "TEST(09)", dRL.test09);
	pConfig.readInto(cRL.test10, "TEST(10)", dRL.test10);
	pConfig.readInto(cRL.test11, "TEST(11)", dRL.test11);
	pConfig.readInto(cRL.test12, "TEST(12)", dRL.test12);
	pConfig.readInto(cRL.test13, "TEST(13)", dRL.test13);
	pConfig.readInto(cRL.test15, "TEST(15)", dRL.test15);
	pConfig.readInto(cRL.test20, "TEST(20)", dRL.test20);


	// Crustal model list
	pConfig.readInto(velocityModel, "CRUSTAL_VELOCITY_MODEL");
	stringExplode(velocityModel, ",", &Tvelocity);

	pConfig.readInto(depthModel, "CRUSTAL_DEPTH_MODEL");
	stringExplode(depthModel, ",", &Tdepth);

	if ( Tvelocity.size() != Tdepth.size() ) {
		SEISCOMP_ERROR("%s Crustal velocity / depth doesn't match up, please correct that.", MSG_HEADER);
		throw LocatorException("ERROR! Velocity model and Depth model doesn't match up in configuration file");
	}

	// Control card parameters
	pConfig.readInto(cCC.ztr, "ZTR", dCC.ztr);
	if ( _usingFixedDepth )
		cCC.ztr = toString(round(_fixedDepth));
	pConfig.readInto(cCC.xnear, "XNEAR", dCC.xnear);

	if ( _enableDistanceCutOff )
		cCC.xfar = toString(_distanceCutOff);
	else
		pConfig.readInto(cCC.xfar, "XFAR", dCC.xfar);

	pConfig.readInto(cCC.pos, "POS", dCC.pos);
	pConfig.readInto(cCC.kfm, "KFM", dCC.kfm);
	pConfig.readInto(cCC.ktest, "KTEST", dCC.ktest);
	pConfig.readInto(cCC.kaz, "KAZ", dCC.kaz);
	pConfig.readInto(cCC.kms, "KMS", dCC.kms);
	pConfig.readInto(cCC.iprn, "IPRN", dCC.iprn);
	cCC.imag = "";
	cCC.ir = "";
	cCC.ksel = "";
	cCC.iq = dCC.iq;
	cCC.ipun = dCC.ipun;
	cCC.ksort = dCC.ksort;

	// Available only on custom configuration file...
	// Should we use the position obtained from the best ZTR value ?
	if ( pConfig.read("USE_TRIAL_POSITION", true) ) {
		cCC.lat1 = _trialLatDeg;
		cCC.lat2 = _trialLatMin;
		cCC.lon1 = _trialLonDeg;
		cCC.lon2 = _trialLonMin;
	}
	else {
		cCC.lat1 = "";
		cCC.lat2 = "";
		cCC.lon1 = "";
		cCC.lon2 = "";
	}

	// Instruction card parameters            !
	pConfig.readInto(cIC.knst, "KNST", dIC.knst);
	pConfig.readInto(cIC.inst, "INST", dIC.inst);
	if ( _usingFixedDepth && !_currentProfile->fixStartDepthOnly )
		cIC.inst = "1";

	cIC.ipro = dIC.ipro;
	cIC.zres = dIC.zres;


	ofstream h71in(_h71inputFile.c_str());

	/*-----------------------*/
	/*| HYPO71 HEADING CARD |*/
	/*-----------------------*/
	h71in << "HEAD" << endl;

	/*---------------------*/
	/*| HYPO71 RESET LIST |*/
	/*---------------------*/
	h71in << "RESET TEST(01)=" << cRL.test01 << endl;
	h71in << "RESET TEST(02)=" << cRL.test02 << endl;
	h71in << "RESET TEST(03)=" << cRL.test03 << endl;
	h71in << "RESET TEST(04)=" << cRL.test04 << endl;
	h71in << "RESET TEST(05)=" << cRL.test05 << endl;
	h71in << "RESET TEST(06)=" << cRL.test06 << endl;
	h71in << "RESET TEST(10)=" << cRL.test10 << endl;
	h71in << "RESET TEST(11)=" << cRL.test11 << endl;
	h71in << "RESET TEST(12)=" << cRL.test12 << endl;
	h71in << "RESET TEST(13)=" << cRL.test13 << endl;
	h71in << "RESET TEST(15)=" << cRL.test15 << endl;
	h71in << "RESET TEST(20)=" << cRL.test20 << endl;

	//! deactivated -> md plugin purpose
	//! h71in << "RESET TEST(07)=" << cRL.test07 <<endl;
	//! h71in << "RESET TEST(08)=" << cRL.test08 <<endl;
	//! h71in << "RESET TEST(09)=" << cRL.test09 <<endl;

	/*---------------------*/
	/*| HYPO71 BLANK CARD |*/
	/*---------------------*/
	h71in << formatString("", 1, 0, "blank card") << endl;


	PickList usedPicks;
	string lastStation;

	// Origin informations to re-use later
	string oYear;

	// Creates observation buffer
	for (PickList::iterator it = pickList.begin();
	        it != pickList.end(); ++it) {

		PickPtr pick = it->first;
		SensorLocationPtr sloc = getSensorLocation(pick.get());

		if ( !sloc ) {
			if ( _allowMissingStations ) {

				if ( !_lastWarning.empty() )
					_lastWarning += "\n";

				_lastWarning += "Sensor location ";
				_lastWarning += pick->waveformID().networkCode() + "." +
				        pick->waveformID().stationCode() + "." +
				        pick->waveformID().locationCode() +
				        " is not available and ignored";
				continue;
			}
			else {
				throw StationNotFoundException((string("Station ") +
				        pick->waveformID().networkCode() + "." +
				        pick->waveformID().stationCode() +
				        " not found.\nPlease deselect it to process localization.").c_str());
			}
		}

		usedPicks.push_back(*it);
		oYear = pick->time().value().toString("%Y");

		string stationMappedName = getStationMappedCode(pick->waveformID().networkCode(),
		    pick->waveformID().stationCode());

		if ( stationMappedName.empty() ) {
			SEISCOMP_ERROR("%s Couldn't found %s.%s alias", MSG_HEADER,
			    pick->waveformID().networkCode().c_str(),
			    pick->waveformID().stationCode().c_str());
			continue;
		}

		if ( string(PICKINFODEBUG) == "ON" ) {

			SEISCOMP_DEBUG("%s --------------------------------", MSG_HEADER);
			SEISCOMP_DEBUG("%s |    STATION PICK RECEPTION    |", MSG_HEADER);
			SEISCOMP_DEBUG("%s --------------------------------", MSG_HEADER);
			SEISCOMP_DEBUG("%s | Network code     | %s", MSG_HEADER, pick->waveformID().networkCode().c_str());
			SEISCOMP_DEBUG("%s | Station code     | %s", MSG_HEADER, pick->waveformID().stationCode().c_str());
			SEISCOMP_DEBUG("%s | Latitude         | %s", MSG_HEADER, toString(sloc->latitude()).c_str());
			SEISCOMP_DEBUG("%s | Longitude        | %s", MSG_HEADER, toString(sloc->longitude()).c_str());
			SEISCOMP_DEBUG("%s | Elevation        | %s", MSG_HEADER, toString(sloc->elevation()).c_str());
			SEISCOMP_DEBUG("%s | Time value       | %s", MSG_HEADER, pick->time().value().toString("%Y%m%d").c_str());
			SEISCOMP_DEBUG("%s | Phase hint code  | %s", MSG_HEADER, pick->phaseHint().code().c_str());
			SEISCOMP_DEBUG("%s --------------------------------", MSG_HEADER);
		}


		if ( lastStation != pick->waveformID().stationCode() ) {
			/*------------------------------*/
			/*| HYPO71 STATION DELAY MODEL |*/
			/*------------------------------*/
			h71in
			//! default blank
			<< formatString("", 2, 0)

			//!  station name
			<< formatString(stationMappedName, 4, 1, "station name")

			//! station latitude's degree + station latitude's minute //! float 7.2
			<< h71DecimalToSexagesimal(sloc->latitude(), gpLatitude)

			//! hemispheric station situation N/S //! Alphanumeric 1
			<< formatString(getSituation(sloc->latitude(), gpLatitude), 1, 0, "hemispheric situation")

			//! station longitude's degree + station longitude's minute //! float 8.2
			<< h71DecimalToSexagesimal(sloc->longitude(), gpLongitude)

			//! hemispheric station situation E/W //! Alphanumeric 1
			<< formatString(getSituation(sloc->longitude(), gpLongitude), 1, 0, "hemispheric situation")

			//! station elevation //! integer 4
			<< formatString(toString((int) sloc->elevation()).c_str(), 4, 0, "station elevation")

			//! blank space
			<< formatString("", 1, 0)

			//! station delay //! float 5.2
			<< formatString("0.00", 5, 0)

			//! station correction for FMAG //! float 5.2
			<< formatString("", 5, 0)

			//! station correction for XMAG //! float 5.2
			<< formatString("", 5, 0)

			//! system number assigned to station //! integer 1
			//! 0 - WoodAndersion
			//! 1 - NCER Standard
			//! 2 - EV-17 and Develco
			//! 3 - HS-10 and Teledyne
			//! 4 - HS-10 and Develco
			//! 5 - L-4C and Develco
			//! 6 - L-4C and Teledyne
			//! 7 - L-4C and replcing HS-10 and Develco
			//! 8 - 10-day recorders
			<< formatString("", 1, 0)

			//! XMAG period //! float 5.2
			<< formatString("", 5, 0)

			//! XMAG calibration //! float 4.2
			<< formatString("", 4, 0)

			//! calibration indicator //! integer 1
			<< formatString("", 1, 0)

			<< endl;

		}
		lastStation = pick->waveformID().stationCode();
	}


	/*---------------------*/
	/*| HYPO71 BLANK CARD |*/
	/*---------------------*/
	h71in << formatString("", 1, 0) << endl;

	/*-----------------------------*/
	/*| HYPO71 CRUSTAL MODEL LIST |*/
	/*-----------------------------*/
	for (size_t i = 0; i < Tvelocity.size(); ++i)
		h71in << formatString(stripSpace(Tvelocity.at(i)), 7, 0)
		        << formatString(stripSpace(Tdepth.at(i)), 7, 0) << endl;

	/*---------------------*/
	/*| HYPO71 BLANK CARD |*/
	/*---------------------*/
	h71in << formatString("", 1, 0) << endl;


	/*----------------*/
	/*| CONTROL CARD |*/
	/*----------------*/
	h71in

	//! blank space
	<< formatString("", 1, 0)

	//! Trial focal depth in km //! float 5.0
	<< formatString(calculatedZTR, 4, 0, "trial focal depth")

	//! distance in km from epicenter where the distance weighting is 1 //! float 5.0
	<< formatString(cCC.xnear, 5, 0, "xnear")

	//! distance in km from epicenter beyond which the distance weighting is 0 //! float 5.0
	<< formatString(cCC.xfar, 5, 0, "xfar")

	//! ratio of P-velocity to S-velocity //! float 5.2
	<< formatString(cCC.pos, 5, 0, "ratio P-S velocity")

	//! blank space
	<< formatString("", 4, 0)

	//! quality class of earthquake to be included in the summary of residuals //! integer 1
	<< formatString(cCC.iq, 1, 0, "quality of earthquake")

	//! blank space
	<< formatString("", 4, 0)

	//! indicator to check missing data //! integer 1
	<< formatString(cCC.kms, 1, 0, "kms")

	//! blank space
	<< formatString("", 3, 0)

	//! minimum number of first motion reading required before it is plotted integer 2
	<< formatString(cCC.kfm, 2, 0, "kfm")

	//! blank space
	<< formatString("", 4, 0)

	//! indicator for punched card //!  integer 1
	<< formatString(cCC.ipun, 1, 0, "ipun")

	//! blank space
	<< formatString("", 4, 0)

	//! method of selecting earthquake magnitude //! integer 1
	<< formatString(cCC.imag, 1, 0, "imag")

	//! blank space
	<< formatString("", 4, 0)

	//! number of new system response curves to be read in //! integer 1
	<< formatString(cCC.ir, 1, 0, "response curve")

	//! blank space
	<< formatString("", 4, 0)

	//! indicator for printed output //! integer 1
	<< formatString(cCC.iprn, 1, 0, "iprn")

	//! blank space
	<< formatString("", 1, 0)

	//! helps to determine if the solution is at the minimum RMS //! integer 1
	<< formatString(cCC.ktest, 1, 0, "ktest")

	//! helps to apply the azimuthal weight of stations //! integer 1
	<< formatString(cCC.kaz, 1, 0, "kaz")

	//! to sort the station by distance in the output file //! integer 1
	<< formatString(cCC.ksort, 1, 0, "ksort")

	//! printed output will start at a new page ? //! integer 1
	<< formatString(cCC.ksel, 1, 0, "ksel")

	//! blank space
	<< formatString("", 2, 0)

	//! Latitudes and longitudes used here are recovered from the getZTR()
	//! method, this will enhance the locator's precision

	//! degree portion of the trial hypocenter latitude //! integer 2
	<< formatString(cCC.lat1, 2, 0, "lat1")
	//	<< formatString(_trialLatDeg, 2, 0, "tlat1deg")

	//! blank space
	<< formatString("", 1, 0)

	//! minute portion of the trial hypocenter latitude //! float 5.2
	<< formatString(cCC.lat2, 5, 0, "lat2")
	//	<< formatString(_trialLatMin, 5, 0, "tlatmin")

	//! blank space
	<< formatString("", 1, 0)

	//! degree portion of the trial longitude //! integer 3
	<< formatString(cCC.lon1, 3, 0, "lon1")
	//	<< formatString(_trialLonDeg, 3, 0, "tlondeg")

	//! blank space
	<< formatString("", 1, 0)

	//! minute portion of the trial longitude //! float 5.2
	<< formatString(cCC.lon2, 5, 0, "lon2")
	//	<< formatString(_trialLonMin, 5, 0, "tlonmin")

	<< endl;


	// Phases list
	bool isPPhase = false;
	bool isSPhase = false;

	// Searching for First Arrival Station (FAS)
	bool foundFAS = false;
	double refTime = .0, refTimeSec, prevRefTime, prevRefTimeSec;
	int refTimeMin, prevRefTimeMin;
	string refTimeYear, refTimeMonth, refTimeDay, refTimeHour, refStation,
	        refNetwork;
	string prevRefTimeYear, prevRefTimeMonth, prevRefTimeDay,
	        prevRefTimeHour, prevRefStation, prevRefNetwork;

	// Uncertainty values
	double maxUncertainty = -1, minUncertainty = 100;
	string maxWeight = "0";
	string minWeight = "4";


	for (PickList::iterator i = pickList.begin();
	        i != pickList.end(); ++i) {

		PickPtr p = i->first;

		double ctime = (double) p->time().value();

		if ( refTime == .0 )
			refTime = ctime;

		if ( p->phaseHint().code().find("P") != string::npos ) {

			if ( ctime <= refTime ) {
				refTime = ctime;
				refTimeYear = p->time().value().toString("%Y");
				refTimeMonth = p->time().value().toString("%m");
				refTimeDay = p->time().value().toString("%d");
				refTimeHour = p->time().value().toString("%H");
				refTimeMin = toInt(p->time().value().toString("%M"));
				refTimeSec = toDouble(p->time().value().toString("%S.%f"));
				refStation = p->waveformID().stationCode();
				refNetwork = p->waveformID().networkCode();
				foundFAS = true;
			}
			else {
				prevRefTime = ctime;
				prevRefTimeYear = p->time().value().toString("%Y");
				prevRefTimeMonth = p->time().value().toString("%m");
				prevRefTimeDay = p->time().value().toString("%d");
				prevRefTimeHour = p->time().value().toString("%H");
				prevRefTimeMin = toInt(p->time().value().toString("%M"));
				prevRefTimeSec = toDouble(p->time().value().toString("%S.%f"));
				prevRefStation = p->waveformID().stationCode();
				prevRefNetwork = p->waveformID().networkCode();
			}
		}

		double upper = .0;
		double lower = .0;
		try {
			if ( p->time().upperUncertainty() != .0 ) {
				upper = p->time().upperUncertainty();
			}
			if ( p->time().lowerUncertainty() != .0 ) {
				lower = p->time().lowerUncertainty();
			}
			if ( (lower + upper) > maxUncertainty )
				maxUncertainty = lower + upper;
			if ( (lower + upper) < minUncertainty )
				minUncertainty = lower + upper;
		} catch ( ... ) {}
	}


	if ( !foundFAS ) {
		refTime = prevRefTime;
		refTimeYear = prevRefTimeYear;
		refTimeMonth = prevRefTimeMonth;
		refTimeDay = prevRefTimeDay;
		refTimeHour = prevRefTimeHour;
		refTimeMin = prevRefTimeMin;
		refTimeSec = prevRefTimeSec;
		refStation = prevRefStation;
		refNetwork = prevRefNetwork;
	}

	if ( !refStation.empty() ) {
		SEISCOMP_DEBUG("%s First arrival station detected: %s.%s - %s-%s-%s %s:%d:%f",
		    MSG_HEADER, refNetwork.c_str(), refStation.c_str(), refTimeYear.c_str(),
		    refTimeMonth.c_str(), refTimeDay.c_str(), refTimeHour.c_str(), refTimeMin,
		    refTimeSec);
	}
	else
		throw LocatorException("SeisComP internal error\nFAS station detection failed");

	int sharedTime = ((int) (refTime / 3600) * 3600);
	string oDate = refTimeYear.substr(2, 2) + refTimeMonth + refTimeDay;
	string h71PWeight;
	string h71SWeight;
	vector<string> Tstation;

	// Retrieve phases from iterator
	for (PickList::iterator it = pickList.begin();
	        it != pickList.end(); ++it) {

		PickPtr pick = it->first;

		string pMinute;
		string pSec;
		string sSec;
		string staName = pick->waveformID().stationCode();
		string pPolarity;
		string sPolarity;
		string stationMappedName = getStationMappedCode(pick->waveformID().networkCode(),
		    pick->waveformID().stationCode());

		char buffer[10];
		double pmin, psec, ssec;

		if ( pick->phaseHint().code().find("P") != string::npos ) {

			bool isIntegrated = false;
			for (size_t x = 0; x < Tstation.size(); ++x)
				if ( Tstation.at(x) == stationMappedName )
					isIntegrated = true;

			if ( !isIntegrated ) {

				Tstation.push_back(stationMappedName);
				isPPhase = true;
				pmin = sharedTime + ((int) (((double) pick->time().value() - sharedTime) / 60) * 60);
				double newmin = (pmin / 60) - (int) (sharedTime / 3600) * 60;
				pMinute = toString((int) newmin);

				psec = getTimeValue(pickList, pick->waveformID().networkCode(), staName, "P", 0) - pmin;
				if ( psec > 99.99 )
					sprintf(buffer, "%#03.1f", ssec);
				else
					sprintf(buffer, "%#02.2f", psec);
				pSec = toString(buffer);

				pPolarity = getPickPolarity(pickList, pick->waveformID().networkCode(),
				    pick->waveformID().stationCode(), "P");

				try {
					h71PWeight = toString(getH71Weight(pickList, pick->waveformID().networkCode(), staName, "P", maxUncertainty));
				}
				catch ( ... ) {
					h71PWeight = maxWeight;
				}

				ssec = getTimeValue(pickList, pick->waveformID().networkCode(), staName, "S", 0) - pmin;
				if ( ssec > 0. ) {
					//! if ssec > 99.99 then it won't fit into a F5.2 so we convert it into a F5.1
					if ( ssec > 99.99 )
						sprintf(buffer, "%#03.1f", ssec);
					else
						sprintf(buffer, "%#02.2f", ssec);
					sSec = toString(buffer);

					sPolarity = getPickPolarity(pickList, pick->waveformID().networkCode(),
					    pick->waveformID().stationCode(), "S");

					isSPhase = true;
					try {
						h71SWeight = toString(getH71Weight(pickList, pick->waveformID().networkCode(), staName, "S", maxUncertainty));
					}
					catch ( ... ) {
						h71SWeight = maxWeight;
					}
				}
			}
		}


		// Writing down P-phase with S-phase
		if ( isPPhase && isSPhase ) {

			SEISCOMP_DEBUG("%s applying %s's P phase weight of %s", MSG_HEADER,
			    staName.c_str(), h71PWeight.c_str());
			SEISCOMP_DEBUG("%s applying %s's S phase weight of %s", MSG_HEADER,
			    staName.c_str(), h71SWeight.c_str());

			h71in
			//! station name //! alphanumeric 4
			<< formatString(stationMappedName, 4, 1)

			//! description of onset of P-arrival //! alphanumeric 1
			<< formatString("E", 1, 0)

			//! 'P' to denote P-arrival //! alphanumeric 1
			<< "P"

			//! first motion direction of P-arrival //! alphanumeric 1
			<< formatString(pPolarity, 1, 0)

			//! weight assigned to P-arrival //! float 1.0
			<< formatString(h71PWeight, 1, 0)

			//! blank space between 8-10
			<< formatString("", 1, 0)

			//! year, month and day of P-arrival //! integer 6
			<< formatString(oDate, 6, 0)

			//! hour of P-arrival //! integer 2
			<< formatString(refTimeHour, 2, 0)

			//! minute of P-arrival //! integer 2
			<< formatString(pMinute, 2, 0)

			//! second of P-arrival //! float 5.2
			<< formatString(pSec, 5, 0)

			//! blank space between 24-32
			<< formatString("", 7, 0)

			//! second of S-arrival //! float of 5.2
			<< formatString(sSec, 5, 0)

			//! description of onset S-arrival //! alphanumeric 1
			<< formatString("E", 1, 0)

			//! 'S' to denote S-arrival //! alphanumeric 1
			<< "S"

			//! first motion direction //! alphanumeric 1
			<< formatString(sPolarity, 1, 0)

			//! weight assigned to S-arrival //! float 1.0
			<< formatString(h71SWeight, 1, 0)

			//! maximum peak-to-peak amplitude in mm //! float 4.0
			<< formatString("", 4, 0)

			//! period of the maximum amplitude in sec //! float 3.2
			<< formatString("", 3, 0)

			//! normally not used except as note in next item //! float 4.1
			<< formatString("", 4, 0)

			//! blank space between 54-59
			<< formatString("", 5, 0)

			//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
			<< formatString("", 4, 0)

			//! remark for this phase card //! alphanumeric 3
			<< formatString("", 3, 0)

			//! time correction in sec //! float 5.2
			<< formatString("", 5, 0)

			<< endl;

			isSPhase = false;
			isPPhase = false;
			staName = "";
		}


		// Writing down P-phase without S-phase
		if ( isPPhase && !isSPhase ) {

			SEISCOMP_DEBUG("%s applying %s's P phase weight of %s", MSG_HEADER,
			    staName.c_str(), h71PWeight.c_str());

			h71in
			//! station name //! alphanumeric 4
			<< formatString(stationMappedName, 4, 1)

			//! description of onset of P-arrival //! alphanumeric 1
			<< formatString("E", 1, 0)

			//! 'P' to denote P-arrival //! alphanumeric 1
			<< "P"

			//! first motion direction of P-arrival //! alphanumeric 1
			<< formatString("", 1, 0)

			//! weight assigned to P-arrival //! float 1.0
			<< formatString(h71PWeight, 1, 0)

			//! blank space between 8-10
			<< formatString("", 1, 0)

			//! year, month and day of P-arrival //! integer 6
			<< formatString(oDate, 6, 0)

			//! hour of P-arrival //! integer 2
			<< formatString(refTimeHour, 2, 0)

			//! minute of P-arrival //! integer 2
			<< formatString(pMinute, 2, 0)

			//! second of P-arrival //! float 5.2
			<< formatString(pSec, 5, 0)

			//! blank space between 24-32
			<< formatString("", 7, 0)

			//! second of S-arrival //! float of 5.2
			<< formatString(sSec, 5, 0)

			//! description of onset S-arrival //! alphanumeric 1
			<< formatString("", 1, 0)

			//! 'S' to denote S-arrival //! alphanumeric 1
			<< formatString("", 1, 0)

			//! first motion direction //! alphanumeric 1
			<< formatString("", 1, 0)

			//! weight assigned to S-arrival //! float 1.0
			<< formatString("", 1, 0)

			//! maximum peak-to-peak amplitude in mm //! float 4.0
			<< formatString("", 4, 0)

			//! period of the maximum amplitude in sec //! float 3.2
			<< formatString("", 3, 0)

			//! normally not used except as note in next item //! float 4.1
			<< formatString("", 4, 0)

			//! blank space between 54-59
			<< formatString("", 5, 0)

			//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
			<< formatString("", 4, 0)

			//! remark for this phase card //! alphanumeric 3
			<< formatString("", 3, 0)

			//! time correction in sec //! float 5.2
			<< formatString("", 5, 0)

			<< endl;

			isSPhase = false;
			isPPhase = false;
			staName = "";
		}

	}

	// Instruction card
	h71in
	//! blank space between 1-4
	<< formatString("", 4, 0)

	//! IPRO //! alphanumeric 4
	<< formatString(cIC.ipro, 4, 0)

	//! blank space between 8-18
	<< formatString("", 9, 0)

	//! Use S Data ? //! integer 1
	<< formatString(cIC.knst, 1, 0)

	//! Fix depth ? //! integer 1
	<< formatString(cIC.inst, 1, 0)

	//! trial focal-depth //! float 5.2
	<< formatString(cIC.zres, 5, 0) << endl;

	//! let's close the generated HYPO71.INP
	h71in.close();


	// Execute script so the localization process gets properly run thru
	// Hypo71 binary... This should take less than a sec...
	system(_hypo71ScriptFile.c_str());

	// Read back *.prt file, that's were all we need is stored
	if ( _h71outputFile.empty() )
		throw LocatorException("ERROR! Processing stream failed\nOutput file is empty");


	string originLine = "DATEORIGINLAT";
	string stationListStart = "STNDISTAZMAINPRMKHRMNP-SECTPOBSTPCALDLY/H1P-RESP-WTAMXPRXCALXKXMAGRMKFMPFMAGSRMKS-SECTSOBSS-RESS-WTDT";
	string stationListEnd = "1*****CLASS:ABCDTOTAL*****";
	string lineContent;
	string line;
	string latSit;
	string lonSit;

	int loop = 0;
	int lineNumber = 1;
	int event = -1;
	int staStart;
	int staEnd;

	// We need to locate the part of interest in which objects shall inherit
	// data from, therefore we identify the key lines of the file by using
	// defined originLine, stationListStart and stationListEnd variables.
	ifstream ifile(_h71outputFile.c_str());
	while ( ifile.good() ) {
		getline(ifile, line);

		string trimmedLine = stripSpace(line);

		// If trimmed line and origineLine match, we've got our origin line
		if ( trimmedLine.substr(0, 13) == originLine ) {
			event = lineNumber;
			latSit = line.substr(13, 1);
			lonSit = line.substr(18, 1);
		}

		// If trimmed line and stationListStart match, we've got our station list start point
		if ( trimmedLine == stationListStart )
			staStart = lineNumber;

		// If trimmed line and stationListEnd match, we've got our station list end point
		if ( trimmedLine == stationListEnd )
			staEnd = lineNumber - 1;

		lineNumber++;
	}
	ifile.close();


	// If event variable value is equal to her initialization value,
	// this file is not a proper Hypo71 prt file, therefore we must exit this
	// process, preventing bad or incorrect value reading.
	if ( event == -1 )
		throw LocatorException("ERROR! The generated output file is not correct\n"
			"Please review your configuration");

	// New Origin creation
	Origin* origin;
	if ( !_publicIDPattern.empty() && _useHypo71PatternID ) {
		origin = Origin::Create("");
		PublicObject::GenerateId(origin, _publicIDPattern);
	}
	else
		origin = Origin::Create();

	Time ot;
	CreationInfo ci;
	OriginQuality oq;

	// Pick indexer
	int idx = 1;
	//int phaseAssocCount = 0;
	int usedAssocCount = 0;
	int depthPhaseCount = 0;
	double rms = 0;
	double hrms;
	vector<double> Tdist;
	vector<double> Tazi;
	string gap;

	// We now know were informations we need to create Origin, Picks, Arrivals
	// are, so we parse the lines properly and create them.
	ifstream file(_h71outputFile.c_str());
	while ( file.good() ) {
		getline(file, lineContent);

		if ( loop == event ) {

			string year, month, day, tHour, tMin, tSec, latDeg, latMin,
			        lonDeg, lonMin, depth, orms, erh, erz, nbStations, tLat,
			        tLon, quality;

			try {
				year = lineContent.substr(1, 2);
				year = stripSpace(year);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("Year exception: %s", e.what());
			}
			try {
				month = lineContent.substr(3, 2);
				month = stripSpace(month);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("Month exception: %s", e.what());
			}
			try {
				day = lineContent.substr(5, 2);
				day = stripSpace(day);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("day exception: %s", e.what());
			}
			try {
				tHour = lineContent.substr(8, 2);
				tHour = stripSpace(tHour);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("thour exception: %s", e.what());
			}
			try {
				tMin = lineContent.substr(10, 2);
				tMin = stripSpace(tMin);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("tmin exception: %s", e.what());
			}
			try {
				tSec = lineContent.substr(13, 5);
				tSec = stripSpace(tSec);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("tsec exception: %s", e.what());
			}
			try {
				latDeg = lineContent.substr(18, 3);
				latDeg = stripSpace(latDeg);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("latdeg exception: %s", e.what());
			}
			try {
				latMin = lineContent.substr(22, 5);
				latMin = stripSpace(latMin);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("latmin exception: %s", e.what());
			}
			try {
				lonDeg = lineContent.substr(28, 3);
				lonDeg = stripSpace(lonDeg);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("londeg exception: %s", e.what());
			}
			try {
				lonMin = lineContent.substr(32, 5);
				lonMin = stripSpace(lonMin);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("lonmin exception: %s", e.what());
			}
			try {
				depth = lineContent.substr(37, 7);
				depth = stripSpace(depth);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("depth exception: %s", e.what());
			}
			try {
				gap = lineContent.substr(57, 4);
				gap = stripSpace(gap);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("gap exception: %s", e.what());
			}
			try {
				orms = lineContent.substr(63, 5);
				orms = stripSpace(orms);
				hrms = toDouble(orms);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("orms exception: %s", e.what());
			}
			try {
				erh = lineContent.substr(68, 5);
				erh = stripSpace(erh);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("erh exception: %s", e.what());
			}
			try {
				erz = lineContent.substr(73, 5);
				erz = stripSpace(erz);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("erz exception: %s", e.what());
			}

			try {
				quality = lineContent.substr(78, 2);
				quality = stripSpace(quality);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("quality exception: %s", e.what());
			}
			try {
				nbStations = lineContent.substr(51, 3);
				nbStations = stripSpace(nbStations);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("nbStations exception: %s", e.what());
			}

			tLat = SexagesimalToDecimalHypo71(toDouble(latDeg), toDouble(latMin), latSit);
			tLon = SexagesimalToDecimalHypo71(toDouble(lonDeg), toDouble(lonMin), lonSit);

			// Setting origin time
			ot.set(toInt(oYear), toInt(month), toInt(day), toInt(tHour), toInt(tMin),
			    (int) floor(toDouble(tSec)), (int) ((toDouble(tSec) - floor(toDouble(tSec))) * 1.0E6));
			origin->setTime(ot);

			// Setting origin creation time
			ci.setCreationTime(Core::Time().gmt());
			origin->setCreationInfo(ci);

			// Dealing with ERH
			// if ERH has been calculated, we convert it into SC3 correction
			// otherwise we put it to NONE
			if ( erh.compare("") != 0 ) {
				double corr = toDouble(erh) / sqrt(2);
				origin->setLatitude(RealQuantity(toDouble(tLat), corr, None, None, None));
				origin->setLongitude(RealQuantity(toDouble(tLon), corr, None, None, None));
			}
			else {
				origin->setLatitude(RealQuantity(toDouble(tLat), None, None, None, None));
				origin->setLongitude(RealQuantity(toDouble(tLon), None, None, None, None));
				SEISCOMP_DEBUG("%s No ERH calculated by Hypo71, putting default NONE value", MSG_HEADER);
			}

			// Dealing with ERZ
			// if ERZ has been calculated, we convert it into SC3 correction
			// otherwise we put it to NONE
			if ( erz.compare("") != 0 ) {
				origin->setDepth(RealQuantity(toDouble(depth), toDouble(erz), None, None, None));
			}
			else {
				origin->setDepth(RealQuantity(toDouble(depth), None, None, None, None));
				SEISCOMP_DEBUG("%s No ERZ calculated by Hypo71, putting default NONE value", MSG_HEADER);
			}
			_usingFixedDepth = false;

			// Set origin quality
			oq.setUsedStationCount(toInt(nbStations));
			oq.setAzimuthalGap(toDouble(gap));
			oq.setGroundTruthLevel(quality);

			if ( string(HYPOINFODEBUG) == "ON" ) {

				SEISCOMP_DEBUG("%s --------------------------------------------", MSG_HEADER);
				SEISCOMP_DEBUG("%s |     HYPO71 NEW HYPOCENTER ESTIMATION     |", MSG_HEADER);
				SEISCOMP_DEBUG("%s --------------------------------------------", MSG_HEADER);
				SEISCOMP_DEBUG("%s | Date              | %s-%s-%s", MSG_HEADER, year.c_str(), month.c_str(), day.c_str());
				SEISCOMP_DEBUG("%s | Time              | %s:%s:%s", MSG_HEADER, tHour.c_str(), tMin.c_str(), tSec.c_str());
				SEISCOMP_DEBUG("%s | Decimal latitude  | %s°", MSG_HEADER, tLat.c_str());
				SEISCOMP_DEBUG("%s | Decimal longitude | %s°", MSG_HEADER, tLon.c_str());
				SEISCOMP_DEBUG("%s | Depth             | %skm", MSG_HEADER, depth.c_str());
				SEISCOMP_DEBUG("%s | RMS               | %s", MSG_HEADER, orms.c_str());
				SEISCOMP_DEBUG("%s | Azimuthal GAP     | %s°", MSG_HEADER, gap.c_str());
				SEISCOMP_DEBUG("%s | Phases            | %s", MSG_HEADER, nbStations.c_str());
				if ( stringIsOfType(erh, stInteger) )
					SEISCOMP_DEBUG("%s | ERH               | %s", MSG_HEADER, erh.c_str());
				if ( stringIsOfType(erz, stInteger) )
					SEISCOMP_DEBUG("%s | ERZ               | %s", MSG_HEADER, erz.c_str());
				SEISCOMP_DEBUG("%s --------------------------------------------", MSG_HEADER);
			}

			//log << head << "*---------------------------------------*" <<endl;
			log << head << "|           FINAL RUN RESULTS           |" << endl;
			log << head << "*---------------------------------------*" << endl;
			log << head << "|       DATE: " << year << "-" << month << "-" << day << endl;
			log << head << "|       TIME: " << tHour << ":" << tMin << ":" << tSec << endl;
			log << head << "|   LATITUDE: " << tLat << endl;
			log << head << "|  LONGITUDE: " << tLon << endl;
			log << head << "|      DEPTH: " << depth << endl;
			log << head << "|        RMS: " << orms << endl;
			log << head << "|    AZ. GAP: " << gap << endl;
			if ( stringIsOfType(erh, stInteger) )
				log << head << "|        ERH: " << erh << endl;
			if ( stringIsOfType(erz, stInteger) )
				log << head << "|        ERZ: " << erz << endl;
			log << head << "*---------------------------------------*" << endl;


		} // loop==event



		if ( (loop >= staStart) && (loop < staEnd) ) {

			string networkCode, staName, dist, azimuth, takeOffAngle, hour,
			        minute, psec, pres, pwt, ssec, sres, swt;
			try {
				//! string 4 - station name
				string str = lineContent.substr(1, 4);
				str = stripSpace(str);

				vector<string> tstr;
				str = getOriginalStationCode(str);
				stringExplode(str, ".", &tstr);

				if ( tstr.size() == 2 ) {
					networkCode = tstr.at(0);
					staName = tstr.at(1);
				}
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("staname: %s", e.what());
			}
			try {
				//! float 6 - station distance to epicenter
				dist = lineContent.substr(5, 6);
				dist = stripSpace(dist);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("dist; %s", e.what());
			}
			try {
				//! integer 4 - station azimuth
				azimuth = lineContent.substr(11, 4);
				azimuth = stripSpace(azimuth);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("azimuth: %s", e.what());
			}
			try {
				//! int 3 - AIN (Take Off Angle)
				takeOffAngle = lineContent.substr(15, 4);
				takeOffAngle = stripSpace(takeOffAngle);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("takeoffangle: %s", e.what());
			}
			try {
				//! integer 2 - Arrival Hour
				hour = lineContent.substr(25, 2);
				hour = stripSpace(hour);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("hour: %s", e.what());
			}
			try {
				//! integer 2 - Arrival Minute
				minute = lineContent.substr(27, 2);
				minute = stripSpace(minute);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("minute: %s", e.what());
			}
			try {
				//! float 5 - P arrival seconds
				psec = lineContent.substr(30, 5);
				psec = stripSpace(psec);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("psec: %s", e.what());
			}
			try {
				//! float 6 - P residuals
				pres = lineContent.substr(53, 6);
				pres = stripSpace(pres);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("pres: %s", e.what());
			}
			try {
				//! float 5 - P weight
				pwt = lineContent.substr(61, 4);
				pwt = stripSpace(pwt);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("pwt: %s", e.what());
			}
			try {
				//! float 6 - S arrival seconds
				ssec = lineContent.substr(103, 6);
				ssec = stripSpace(ssec);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("ssec: %s", e.what());
			}
			try {
				//! float 5 - S residuals
				sres = lineContent.substr(115, 6);
				sres = stripSpace(sres);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("sres: %s", e.what());
			}
			try {
				//! float 4 - S weight
				swt = lineContent.substr(123, 4);
				swt = stripSpace(swt);
			}
			catch ( exception& e ) {
				SEISCOMP_ERROR("swt: %s", e.what());
			}

			CreationInfo aci;
			aci.setCreationTime(Core::Time::GMT());
			aci.setAuthor("Hypo71");
			aci.setModificationTime(Core::Time::GMT());

			// Associating new arrival with new origin.
			// P.S.: the weight actually stays the same as the one in picklist
			// since it is not the real weight but just intel about whether or not
			// to use arrival (rms too high or some like that) [act as boolean 1 or 0]
			for (PickList::iterator it = pickList.begin();
			        it != pickList.end(); ++it) {

				PickPtr pick = it->first;
				double weight = it->second;

				if ( (pick->phaseHint().code() == "P")
				        && (pick->waveformID().stationCode() == staName)
				        && (pick->waveformID().networkCode() == networkCode) ) {

					ArrivalPtr pArrival = new Arrival;
					pArrival->setPickID(pick->publicID());
					pArrival->setCreationInfo(aci);
					pArrival->setWeight(weight);
					pArrival->setDistance(Math::Geo::km2deg(toDouble(dist)));
					pArrival->setAzimuth(toInt(azimuth));
					pArrival->setPhase(Phase("P"));
					pArrival->setTimeResidual(toDouble(pres));
					pArrival->setTakeOffAngle(toDouble(takeOffAngle));

					if ( !origin->add(pArrival.get()) ) {
						origin->removeArrival(pArrival->index());
						origin->add(pArrival.get());
					}
				}

				if ( (pick->phaseHint().code() == "S")
				        && (pick->waveformID().stationCode() == staName)
				        && (pick->waveformID().networkCode() == networkCode) ) {

					ArrivalPtr sArrival = new Arrival;
					sArrival->setPickID(pick->publicID());
					sArrival->setCreationInfo(aci);
					sArrival->setWeight(weight);
					sArrival->setDistance(Math::Geo::km2deg(toDouble(dist)));
					sArrival->setAzimuth(toInt(azimuth));
					sArrival->setPhase(Phase("S"));
					sArrival->setTimeResidual(toDouble(sres));
					sArrival->setTakeOffAngle(toDouble(takeOffAngle));

					if ( !origin->add(sArrival.get()) ) {
						origin->removeArrival(sArrival->index());
						origin->add(sArrival.get());
					}
				}
			}

			if ( toDouble(pwt) > 0.5 )
				depthPhaseCount++;

			idx++;

			if ( ssec != "" ) {

				if ( toDouble(swt) > 0.5 )
					depthPhaseCount++;
				idx++;
			}

			// Saving station arrival info
			Tdist.push_back(toDouble(dist));
			Tazi.push_back(toDouble(azimuth));
			rms += toDouble(pres) * toDouble(sres);
			++usedAssocCount;

			if ( string(HYPOINFODEBUG) == "ON" ) {
				SEISCOMP_DEBUG("%s -----------------------------------", MSG_HEADER);
				SEISCOMP_DEBUG("%s |   HYPO71 STATION INFORMATIONS   |", MSG_HEADER);
				SEISCOMP_DEBUG("%s -----------------------------------", MSG_HEADER);
				SEISCOMP_DEBUG("%s | Network code | %s", MSG_HEADER, networkCode.c_str());
				SEISCOMP_DEBUG("%s | Station code | %s", MSG_HEADER, staName.c_str());
				SEISCOMP_DEBUG("%s | Ep. distance | %skm", MSG_HEADER, dist.c_str());
				SEISCOMP_DEBUG("%s | Azimuth      | %s°", MSG_HEADER, azimuth.c_str());
				SEISCOMP_DEBUG("%s | Hour         | %s:%s", MSG_HEADER, hour.c_str(), minute.c_str());
				SEISCOMP_DEBUG("%s | P-Sec        | %s", MSG_HEADER, psec.c_str());
				SEISCOMP_DEBUG("%s | P residuals  | %s", MSG_HEADER, pres.c_str());
				SEISCOMP_DEBUG("%s | P weight     | %s", MSG_HEADER, pwt.c_str());
				if ( ssec != "" )
					SEISCOMP_DEBUG("%s | S-Sec        | %s", MSG_HEADER, ssec.c_str());
				if ( sres != "" )
					SEISCOMP_DEBUG("%s | S residuals  | %s", MSG_HEADER, sres.c_str());
				if ( swt != "" )
					SEISCOMP_DEBUG("%s | S weight     | %s", MSG_HEADER, swt.c_str());
				SEISCOMP_DEBUG("%s -----------------------------------", MSG_HEADER);
			}

		} //  (loop >= staStart) && (loop < staEnd)

		loop++;
	} //  end of second file itereation
	file.close();

	origin->setEarthModelID(_currentProfile->earthModelID);
	origin->setMethodID("Hypo71");

	oq.setAssociatedPhaseCount(idx);
	oq.setUsedPhaseCount(idx);
	oq.setDepthPhaseCount(0);
	oq.setStandardError(hrms);

	sort(Tazi.begin(), Tazi.end());
	Tazi.push_back(Tazi.front() + 360.);

	double azGap = 0.;
	if ( Tazi.size() > 2 )
		for (size_t i = 0; i < Tazi.size() - 1; ++i)
			azGap = (Tazi[i + 1] - Tazi[i]) > azGap ? (Tazi[i + 1] - Tazi[i]) : azGap;

	sort(Tdist.begin(), Tdist.end());
	oq.setMinimumDistance(Math::Geo::km2deg(Tdist.front()));
	oq.setMaximumDistance(Math::Geo::km2deg(Tdist.back()));
	oq.setMedianDistance(Tdist[Tdist.size() / 2]);
	origin->setQuality(oq);

	log.close();

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string
Hypo71::getZTR(const PickList& pickList) throw (Core::GeneralException) {

	vector<string> Tvelocity;
	vector<string> Tdepth;
	vector<string> Tztr;
	double minRMS = 10.;
	double minER = 10000.;
	string minDepth;
	char buf[10];
	ofstream log(_logFile.c_str(), ios::app);

	/* time log's buffer */
	time_t rawtime;
	struct tm* timeinfo;
	char head[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(head, 80, "%F %H:%M:%S [log] ", timeinfo);

	log << endl;
	log << endl;
	log << head << "*---------------------------------------*" << endl;
	log << head << "|           NEW LOCALIZATION            |" << endl;
	log << head << "*---------------------------------------*" << endl;


	/*----------------------------------*
	 | DEFAULT CONFIGURATION PARAMETERS |
	 |            (default.hypo71.conf) |
	 *----------------------------------*/
	ResetList dRL;
	ControlCard dCC;
	InstructionCard dIC;
	ConfigFile dConfig(_controlFilePath);

	// Reset list values
	dConfig.readInto(dRL.test01, "TEST(01)");
	dConfig.readInto(dRL.test02, "TEST(02)");
	dConfig.readInto(dRL.test03, "TEST(03)");
	dConfig.readInto(dRL.test04, "TEST(04)");
	dConfig.readInto(dRL.test05, "TEST(05)");
	dConfig.readInto(dRL.test06, "TEST(06)");
	dConfig.readInto(dRL.test07, "TEST(07)");
	dConfig.readInto(dRL.test08, "TEST(08)");
	dConfig.readInto(dRL.test09, "TEST(09)");
	dConfig.readInto(dRL.test10, "TEST(10)");
	dConfig.readInto(dRL.test11, "TEST(11)");
	dConfig.readInto(dRL.test12, "TEST(12)");
	dConfig.readInto(dRL.test13, "TEST(13)");
	dConfig.readInto(dRL.test15, "TEST(15)");
	dConfig.readInto(dRL.test20, "TEST(20)");

	// Control card values
	dConfig.readInto(dCC.ztr, "ZTR");
	dConfig.readInto(dCC.xnear, "XNEAR");
	dConfig.readInto(dCC.xfar, "XFAR");
	dConfig.readInto(dCC.pos, "POS");
	dConfig.readInto(dCC.kfm, "KFM");
	dCC.kaz = "";
	dCC.kms = "0";
	dCC.imag = "";
	dCC.ir = "";
	dCC.ksel = "";
	string _iq = "2";
	dConfig.readInto(dCC.iq, "IQ", _iq);
	string _ipun = "1";
	dConfig.readInto(dCC.ipun, "IPUN", _ipun);
	string _iprn = "0";
	dConfig.readInto(dCC.iprn, "IPRN", _iprn);
	dCC.ksort = "";

	// Default instruction card values !
	dConfig.readInto(dIC.ipro, "IPRO");
	dConfig.readInto(dIC.knst, "KNST");
	dConfig.readInto(dIC.inst, "INST");
	dConfig.readInto(dIC.zres, "ZRES");
	dIC.ipro = "";
	dIC.zres = "";


	/*---------------------------------*
	 | CUSTOM CONFIGURATION PARAMETERS |
	 |                (profile.*.conf) |
	 *---------------------------------*/
	ResetList cRL;
	FullControlCard cCC;
	InstructionCard cIC;
	ConfigFile pConfig(_currentProfile->controlFile);


	// Reset list values
	pConfig.readInto(cRL.test01, "TEST(01)", dRL.test01);
	pConfig.readInto(cRL.test02, "TEST(02)", dRL.test02);
	pConfig.readInto(cRL.test03, "TEST(03)", dRL.test03);
	pConfig.readInto(cRL.test04, "TEST(04)", dRL.test04);
	pConfig.readInto(cRL.test05, "TEST(05)", dRL.test05);
	pConfig.readInto(cRL.test06, "TEST(06)", dRL.test06);
	pConfig.readInto(cRL.test07, "TEST(07)", dRL.test07);
	pConfig.readInto(cRL.test08, "TEST(08)", dRL.test08);
	pConfig.readInto(cRL.test09, "TEST(09)", dRL.test09);
	pConfig.readInto(cRL.test10, "TEST(10)", dRL.test10);
	pConfig.readInto(cRL.test11, "TEST(11)", dRL.test11);
	pConfig.readInto(cRL.test12, "TEST(12)", dRL.test12);
	pConfig.readInto(cRL.test13, "TEST(13)", dRL.test13);
	pConfig.readInto(cRL.test15, "TEST(15)", dRL.test15);
	pConfig.readInto(cRL.test20, "TEST(20)", dRL.test20);


	// Crustal model list
	pConfig.readInto(velocityModel, "CRUSTAL_VELOCITY_MODEL");
	stringExplode(velocityModel, ",", &Tvelocity);

	pConfig.readInto(depthModel, "CRUSTAL_DEPTH_MODEL");
	stringExplode(depthModel, ",", &Tdepth);

	if ( Tvelocity.size() != Tdepth.size() )
		throw LocatorException("ERROR! Velocity model and Depth model doesn't match up in configuration file");

	// Control card parameters
	pConfig.readInto(cCC.ztr, "ZTR", dCC.ztr);
	stringExplode(cCC.ztr, ",", &Tztr);

	// Do not force ztr under 2 values to exit, just inform the user in log
	SEISCOMP_DEBUG("%s ZTR process will be using %d value(s)", MSG_HEADER, (int )Tztr.size());
//		if ( Tztr.size() < 2 )
//		throw LocatorException("ERROR! Only one value has been set for ZTR trial depth\n"
//				"Iterations process needs at least two");


	if ( _usingFixedDepth )
		cCC.ztr = toString(round(_fixedDepth));
	pConfig.readInto(cCC.xnear, "XNEAR", dCC.xnear);

	if ( _enableDistanceCutOff )
		cCC.xfar = toString(_distanceCutOff);
	else
		pConfig.readInto(cCC.xfar, "XFAR", dCC.xfar);

	pConfig.readInto(cCC.pos, "POS", dCC.pos);
	pConfig.readInto(cCC.kfm, "KFM", dCC.kfm);
	pConfig.readInto(cCC.ktest, "KTEST", dCC.ktest);
	pConfig.readInto(cCC.kaz, "KAZ", dCC.kaz);
	pConfig.readInto(cCC.kms, "KMS", dCC.kms);
	pConfig.readInto(cCC.iprn, "IPRN", dCC.iprn);
	cCC.imag = "";
	cCC.ir = "";
	cCC.ksel = "";
	cCC.iq = dCC.iq;
	cCC.ipun = dCC.ipun;
	cCC.ksort = dCC.ksort;

	//! Start iterations without any known position
	cCC.lat1 = "";
	cCC.lat2 = "";
	cCC.lon1 = "";
	cCC.lon2 = "";

	// Instruction card parameters            !
	pConfig.readInto(cIC.knst, "KNST", dIC.knst);
	pConfig.readInto(cIC.inst, "INST", dIC.inst);
	cIC.ipro = dIC.ipro;
	cIC.zres = dIC.zres;


	log << head << "| Picklist content" << endl;
	for (PickList::const_iterator i = pickList.begin();
	        i != pickList.end(); ++i) {
		PickPtr p = i->first;
		log << head << "|  " << p->phaseHint().code() << " "
		    << p->waveformID().stationCode() << " a.k.a. "
		    << getStationMappedCode(p->waveformID().networkCode(), p->waveformID().stationCode())
		    << " " << p->time().value().toString("%H:%M:%S.%f")
		    << endl;
	}


	// Here start ZTR iterations
	// NB: we process several localizations using vector's Tztr values
	// and select the min(rms*sqrt(erh²+erz²)) as reference. This value will
	// become the depth starting point.
	for (size_t j = 0; j < Tztr.size(); ++j) {

		ofstream h71in(_h71inputFile.c_str());

		log << head << "*---------------------------------------*" << endl;
		log << head << "| " << "ITERATION " << j << " with ZTR fixed at "
		        << stripSpace(Tztr.at(j)) << "km" << endl;

		/*-----------------------*/
		/*| HYPO71 HEADING CARD |*/
		/*-----------------------*/
		h71in << "HEAD" << endl;

		/*---------------------*/
		/*| HYPO71 RESET LIST |*/
		/*---------------------*/
		h71in << "RESET TEST(01)=" << cRL.test01 << endl;
		h71in << "RESET TEST(02)=" << cRL.test02 << endl;
		h71in << "RESET TEST(03)=" << cRL.test03 << endl;
		h71in << "RESET TEST(04)=" << cRL.test04 << endl;
		h71in << "RESET TEST(05)=" << cRL.test05 << endl;
		h71in << "RESET TEST(06)=" << cRL.test06 << endl;
		h71in << "RESET TEST(10)=" << cRL.test10 << endl;
		h71in << "RESET TEST(11)=" << cRL.test11 << endl;
		h71in << "RESET TEST(12)=" << cRL.test12 << endl;
		h71in << "RESET TEST(13)=" << cRL.test13 << endl;
		h71in << "RESET TEST(15)=" << cRL.test15 << endl;
		h71in << "RESET TEST(20)=" << cRL.test20 << endl;


		/*---------------------*/
		/*| HYPO71 BLANK CARD |*/
		/*---------------------*/
		h71in << formatString("", 1, 0) << endl;

		PickList usedPicks;
		string lastStation;

		// Origin information to re-use later
		string oYear;

		// Creates observation buffer
		for (PickList::const_iterator it = pickList.begin();
		        it != pickList.end(); ++it) {

			PickPtr pick = it->first;
			SensorLocationPtr sloc = getSensorLocation(pick.get());
			usedPicks.push_back(*it);

			oYear = pick->time().value().toString("%Y");

			string stationName = getStationMappedCode(pick->waveformID().networkCode(),
			    pick->waveformID().stationCode());

			if ( stationName.empty() ) {
				SEISCOMP_ERROR("%s Couldn't find %s.%s alias", MSG_HEADER,
				    pick->waveformID().networkCode().c_str(),
				    pick->waveformID().stationCode().c_str());
				continue;
			}

			if ( lastStation != pick->waveformID().stationCode() ) {

				/*------------------------------*/
				/*| HYPO71 STATION DELAY MODEL |*/
				/*------------------------------*/
				h71in
				//! default blank
				<< formatString("", 2, 0)
				//!  station name
				<< formatString(stationName, 4, 1, "station name")
				//! station latitude's degree + station latitude's minute //! float 7.2
				<< h71DecimalToSexagesimal(sloc->latitude(), gpLatitude)
				//! hemispheric station situation N/S //! Alphanumeric 1
				<< formatString(getSituation(sloc->latitude(), gpLatitude), 1, 0, "hemispheric situation")
				//! station longitude's degree + station longitude's minute //! float 7.2
				<< h71DecimalToSexagesimal(sloc->longitude(), gpLongitude)
				//! hemispheric station situation E/W //! Alphanumeric 1
				<< formatString(getSituation(sloc->longitude(), gpLongitude), 1, 0, "hemispheric situation")
				//! station elevation //! integer 4
				<< formatString(toString((int) sloc->elevation()).c_str(), 4, 0, "station elevation")
				//! blank space
				<< formatString("", 1, 0)
				//! station delay //! float 5.2
				<< formatString("0.00", 5, 0)
				//! station correction for FMAG //! float 5.2
				<< formatString("", 5, 0)
				//! station correction for XMAG //! float 5.2
				<< formatString("", 5, 0)
				//! system number assigned to station //! integer 1
				<< formatString("", 1, 0)
				//! XMAG period //! float 5.2
				<< formatString("", 5, 0)
				//! XMAG calibration //! float 4.2
				<< formatString("", 4, 0)
				//! calibration indicator //! integer 1
				<< formatString("", 1, 0)
				<< endl;

			}
			lastStation = pick->waveformID().stationCode();
		}    //! end for

		/*---------------------*/
		/*| HYPO71 BLANK CARD |*/
		/*---------------------*/
		h71in << formatString("", 1, 0) << endl;

		for (size_t i = 0; i < Tvelocity.size(); ++i)
			h71in << formatString(stripSpace(Tvelocity.at(i)), 7, 0, "HYPO BLANK CARD")
			        << formatString(stripSpace(Tdepth.at(i)), 7, 0, "HYPO BLANK CARD")
			        << endl;


		/*---------------------*/
		/*| HYPO71 BLANK CARD |*/
		/*---------------------*/
		h71in << formatString("", 1, 0) << endl;


		//? ##------------##
		//? # control card #
		//? ##------------##
		h71in
		        //! blank card
		        << formatString("", 1, 0)
		        //! Trial focal depth in km //! float 5.0
		        << formatString(stripSpace(Tztr.at(j)), 4, 0, "trial focal depth")
		        //! distance in km from epicenter where the distance weighting is 1 //! float 5.0
		        << formatString(cCC.xnear, 5, 0, "xnear")
		        //! distance in km from epicenter beyond which the distance weighting is 0 //! float 5.0
		        << formatString(cCC.xfar, 5, 0, "xfar")
		        //! ratio of P-velocity to S-velocity //! float 5.2
		        << formatString(cCC.pos, 5, 0, "ratio P-velocity to S-velocity")
		        //! blank space
		        << formatString("", 4, 0)
		        //! quality class of earthquake to be included in the summary of residuals //! integer 1
		        << formatString(cCC.iq, 1, 0, "class or earthquake")
		        //! blank space
		        << formatString("", 4, 0)
		        //! indicator to check missing data //! integer 1
		        << formatString(cCC.kms, 1, 0, "kms")
		        //! blank space
		        << formatString("", 3, 0)
		        //! minimum number of first motion reading required before it is plotted integer 2
		        << formatString(cCC.kfm, 2, 0, "kfm")
		        //! blank space
		        << formatString("", 4, 0)
		        //! indicator for punched card //!  integer 1
		        << formatString(cCC.ipun, 1, 0, "ipun")
		        //! blank space
		        << formatString("", 4, 0)
		        //! method of selecting earthquake magnitude //! integer 1
		        << formatString(cCC.imag, 1, 0, "imag")
		        //! blank space
		        << formatString("", 4, 0)
		        //! number of new system response curves to be read in //! integer 1
		        << formatString(cCC.ir, 1, 0, "response curve")
		        //! blank space
		        << formatString("", 4, 0)
		        //! indicator for printed output //! integer 1
		        << formatString(cCC.iprn, 1, 0, "iprn")
		        //! blank space
		        << formatString("", 1, 0)
		        //! helps to determine if the solution is at the minimum RMS //! integer 1
		        << formatString(cCC.ktest, 1, 0, "ktest")
		        //! helps to apply the azimuthal weight of stations //! integer 1
		        << formatString(cCC.kaz, 1, 0, "kaz")
		        //! to sort the station by distance in the output file //! integer 1
		        << formatString(cCC.ksort, 1, 0, "ksort")
		        //! printed output will start at a new page ? //! integer 1
		        << formatString(cCC.ksel, 1, 0, "ksel")
		        //! blank space
		        << formatString("", 2, 0)
		        //! degree portion of the trial hypocenter latitude //! integer 2
		        << formatString(cCC.lat1, 2, 0, "lat1")
		        //! blank space
		        << formatString("", 1, 0)
		        //! minute portion of the trial hypocenter latitude //! float 5.2
		        << formatString(cCC.lat2, 5, 0, "lat2")
		        //! blank space
		        << formatString("", 1, 0)
		        //! degree portion of the trial longitude //! integer 3
		        << formatString(cCC.lon1, 3, 0, "lon1")
		        //! blank space
		        << formatString("", 1, 0)
		        //! minute portion of the trial longitude //! float 5.2
		        << formatString(cCC.lon2, 5, 0, "lon2")

		        << endl;


		// Phases list
		string prevStaName;
		bool isPPhase = false;
		bool isSPhase = false;

		// Searching for First Arrival Station (FAS) variables
		bool foundFAS = false;
		double refTime = .0, prevRefTime;
		string refTimeYear, refTimeMonth, refTimeDay, refTimeHour, refStation,
		        refNetwork;
		string prevRefTimeYear, prevRefTimeMonth, prevRefTimeDay,
		        prevRefTimeHour, prevRefStation, prevRefNetwork;

		// Uncertainty values
		double maxUncertainty = -1, minUncertainty = 100;
		string maxWeight = "0";
		string minWeight = "4";


		for (PickList::const_iterator i = pickList.begin();
		        i != pickList.end(); ++i) {

			PickPtr p = i->first;
			double ctime = (double) p->time().value();

			if ( refTime == .0 )
				refTime = ctime;

			if ( p->phaseHint().code().find("P") != string::npos ) {
				if ( ctime <= refTime ) {
					refTime = ctime;
					refTimeYear = p->time().value().toString("%Y");
					refTimeMonth = p->time().value().toString("%m");
					refTimeDay = p->time().value().toString("%d");
					refTimeHour = p->time().value().toString("%H");
					refStation = p->waveformID().stationCode();
					refNetwork = p->waveformID().networkCode();
					foundFAS = true;
				}
				else {
					prevRefTime = ctime;
					prevRefTimeYear = p->time().value().toString("%Y");
					prevRefTimeMonth = p->time().value().toString("%m");
					prevRefTimeDay = p->time().value().toString("%d");
					prevRefTimeHour = p->time().value().toString("%H");
					prevRefStation = p->waveformID().stationCode();
					prevRefNetwork = p->waveformID().networkCode();
				}
			}

			double upper = .0;
			double lower = .0;
			try {
				if ( p->time().upperUncertainty() != .0 )
					upper = p->time().upperUncertainty();
				if ( p->time().lowerUncertainty() != .0 )
					lower = p->time().lowerUncertainty();
				if ( (lower + upper) > maxUncertainty )
					maxUncertainty = lower + upper;
				if ( (lower + upper) < minUncertainty )
					minUncertainty = lower + upper;
			}
			catch ( ... ) {}
		}

		if ( !foundFAS ) {
			refTime = prevRefTime;
			refTimeYear = prevRefTimeYear;
			refTimeMonth = prevRefTimeMonth;
			refTimeDay = prevRefTimeDay;
			refTimeHour = prevRefTimeHour;
			refStation = prevRefStation;
			refNetwork = prevRefNetwork;
		}


		if ( refStation.empty() )
			throw LocatorException("ERROR! getZTR was unable to identify the FirstArrivalStation (FAS)");

		int sharedTime = ((int) (refTime / 3600) * 3600);
		string oDate = refTimeYear.substr(2, 2) + refTimeMonth + refTimeDay;
		string h71PWeight;
		string h71SWeight;
		vector<string> Tstation;


		// Retrieves phases from iterator
		for (PickList::const_iterator it = pickList.begin();
		        it != pickList.end(); ++it) {

			PickPtr pick = it->first;

			string pMinute;
			string pSec;
			string sSec;
			string staName = pick->waveformID().stationCode();
			string pPolarity;
			string sPolarity;
			string stationMappedName = getStationMappedCode(pick->waveformID().networkCode(),
			    pick->waveformID().stationCode());
			char buffer[10];
			double pmin, psec, ssec;

			string tmpDate = pick->time().value().toString("%Y").substr(2, 2)
			        + pick->time().value().toString("%m") + pick->time().value().toString("%d");
			string tmpHour = pick->time().value().toString("%H");

			if ( pick->phaseHint().code().find("P") != string::npos ) {

				bool isIntegrated = false;
				for (size_t x = 0; x < Tstation.size(); ++x)
					if ( Tstation.at(x) == stationMappedName )
						isIntegrated = true;

				if ( !isIntegrated ) {

					Tstation.push_back(stationMappedName);
					isPPhase = true;
					pmin = sharedTime + ((int) (((double) pick->time().value() - sharedTime) / 60)) * 60;
					double newmin = pmin / 60 - (int) (sharedTime / 3600) * 60;
					pMinute = toString((int) newmin);

					psec = getTimeValue(pickList, pick->waveformID().networkCode(), staName, "P", 0) - pmin;
					pPolarity = getPickPolarity(pickList, pick->waveformID().networkCode(), staName, "P");
					if ( psec > 99.99 )
						sprintf(buffer, "%#03.1f", ssec);
					else
						sprintf(buffer, "%#02.2f", psec);
					pSec = toString(buffer);

					try {
						h71PWeight = toString(getH71Weight(pickList, pick->waveformID().networkCode(), staName, "P", maxUncertainty));
					}
					catch ( ... ) {
						h71PWeight = maxWeight;
					}

					ssec = getTimeValue(pickList, pick->waveformID().networkCode(), staName, "S", 0) - pmin;

					if ( ssec > 0. ) {
						// If ssec > 99.99 then it won't fit into a F5.2
						// so we convert it into a F5.1
						if ( ssec > 99.99 )
							sprintf(buffer, "%#03.1f", ssec);
						else
							sprintf(buffer, "%#02.2f", ssec);
						sSec = toString(buffer);
						sPolarity = getPickPolarity(pickList, pick->waveformID().networkCode(), staName, "S");
						isSPhase = true;
						try {
							h71SWeight = toString(getH71Weight(pickList, pick->waveformID().networkCode(), staName, "S", maxUncertainty));
						}
						catch ( ... ) {
							h71SWeight = maxWeight;
						}
					}
				}
			}


			// Writing down P-phase with S-phase
			if ( isPPhase && isSPhase ) {

				if ( sSec.size() > 5 )
					throw LocatorException("Hypo71 error!\n" + staName +
					        " S Phase is wrongly picked.\nHypo71 does not allow to fit " +
					        toString(sSec.size()) + " chars into 5 chars variable");

				h71in
				//! station name //! alphanumeric 4
				<< formatString(stationMappedName, 4, 1)
				//! description of onset of P-arrival //! alphanumeric 1
				<< formatString("E", 1, 0)
				//! 'P' to denote P-arrival //! alphanumeric 1
				<< "P"
				//! first motion direction of P-arrival //! alphanumeric 1
				<< formatString(pPolarity, 1, 0)
				//! weight assigned to P-arrival //! float 1.0
				<< formatString(h71PWeight, 1, 0)
				//! blank space between 8-10
				<< formatString("", 1, 0)
				//! year, month and day of P-arrival //! integer 6
				<< formatString(oDate, 6, 0, "ODATE")
				//! hour of P-arrival //! integer 2
				<< formatString(refTimeHour, 2, 0)
				//! minute of P-arrival //! integer 2
				<< formatString(pMinute, 2, 0)
				//! second of P-arrival //! float 5.2
				<< formatString(pSec, 5, 0, "PSEC")
				//! blank space between 24-32
				<< formatString("", 7, 0)
				//! second of S-arrival //! float of 5.2
				<< formatString(sSec, 5, 0, "SSEC")
				//! description of onset S-arrival //! alphanumeric 1
				<< formatString("E", 1, 0)
				//! 'S' to denote S-arrival //! alphanumeric 1
				<< "S"
				//! first motion direction //! alphanumeric 1
				<< formatString(sPolarity, 1, 0)
				//! weight assigned to S-arrival //! float 1.0
				<< formatString(h71SWeight, 1, 0)
				//! maximum peak-to-peak amplitude in mm //! float 4.0
				<< formatString("", 4, 0)
				//! period of the maximum amplitude in sec //! float 3.2
				<< formatString("", 3, 0)
				//! normally not used except as note in next item //! float 4.1
				<< formatString("", 4, 0)
				//! blank space between 54-59
				<< formatString("", 5, 0)
				//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
				<< formatString("", 4, 0)
				//! remark for this phase card //! alphanumeric 3
				<< formatString("", 3, 0)
				//! time correction in sec //! float 5.2
				<< formatString("", 5, 0)
				<< endl;

				isSPhase = false;
				isPPhase = false;
				staName = "";
			}


			// Writing down P-phase without S-phase
			if ( isPPhase && !isSPhase ) {

				h71in
				//! station name //! alphanumeric 4
				<< formatString(stationMappedName, 4, 1)
				//! description of onset of P-arrival //! alphanumeric 1
				<< formatString("E", 1, 0)
				//! 'P' to denote P-arrival //! alphanumeric 1
				<< "P"
				//! first motion direction of P-arrival //! alphanumeric 1
				<< formatString("", 1, 0)
				//! weight assigned to P-arrival //! float 1.0
				<< formatString(h71PWeight, 1, 0)
				//! blank space between 8-10
				<< formatString("", 1, 0)
				//! year, month and day of P-arrival //! integer 6
				<< formatString(oDate, 6, 0, "ODATE")
				//! hour of P-arrival //! integer 2
				<< formatString(refTimeHour, 2, 0)
				//! minute of P-arrival //! integer 2
				<< formatString(pMinute, 2, 0)
				//! second of P-arrival //! float 5.2
				<< formatString(pSec, 5, 0, "PSEC")
				//! blank space between 24-32
				<< formatString("", 7, 0)
				//! second of S-arrival //! float of 5.2
				<< formatString(sSec, 5, 0, "SSEC")
				//! description of onset S-arrival //! alphanumeric 1
				<< formatString("", 1, 0)
				//! 'S' to denote S-arrival //! alphanumeric 1
				<< formatString("", 1, 0)
				//! first motion direction //! alphanumeric 1
				<< formatString("", 1, 0)
				//! weight assigned to S-arrival //! float 1.0
				<< formatString("", 1, 0)
				//! maximum peak-to-peak amplitude in mm //! float 4.0
				<< formatString("", 4, 0)
				//! period of the maximum amplitude in sec //! float 3.2
				<< formatString("", 3, 0)
				//! normally not used except as note in next item //! float 4.1
				<< formatString("", 4, 0)
				//! blank space between 54-59
				<< formatString("", 5, 0)
				//! peak-to-peak amplitude of 10 microvolts calibration signal in mm //! float 4.1
				<< formatString("", 4, 0)
				//! remark for this phase card //! alphanumeric 3
				<< formatString("", 3, 0)
				//! time correction in sec //! float 5.2
				<< formatString("", 5, 0)
				<< endl;

				isSPhase = false;
				isPPhase = false;
				staName = "";
			}

		} // end for

		// Instruction card
		h71in
		//! blank space between 1-4
		<< formatString("", 4, 0)
		//! IPRO //! alphanumeric 4
		<< formatString(cIC.ipro, 4, 0, "ipro")
		//! blank space between 8-18
		<< formatString("", 9, 0)
		//! Use S Data ? //! integer 1
		<< formatString(cIC.knst, 1, 0, "knst")
		//! Fix depth ? //! integer 1
		<< formatString(cIC.inst, 1, 0, "inst")
		//! trial focal-depth //! float 5.2
		<< formatString(cIC.zres, 5, 0, "zres")

		<< endl;

		//! let's close the generated HYPO71.INP
		h71in.close();


		// Now we need to call the HYPO71 binary file.
		// TODO IDEA: try and generate the script file instead of calling it...
		// Though we might get some trouble and weird stuffs happening if the
		// the system terminal is quite exotic (bash, shell, bourne shell, ksh, csh, etc)
		// Food for thought!
		system(_hypo71ScriptFile.c_str());

		// READING BACK GENERATED HYPO71.PRT
		if ( _h71outputFile.empty() ) {
			throw LocatorException("HYPO71 computation error...");
		}
		else {

			string originLine = "DATEORIGINLAT";
			string lineContent;
			string line;
			string cutoff = "*****INSUFFICIENTDATAFORLOCATINGTHISQUAKE:";
			string latSit;
			string lonSit;

			int loop = 0;
			int lineNumber = 1;
			int event = -1;

			ifstream ifile(_h71outputFile.c_str());
			while ( ifile.good() ) {
				getline(ifile, line);
				string trimmedLine = stripSpace(line);
				// If trimmed line and origineLine match we've got our origin line
				if ( trimmedLine.substr(0, 13) == originLine ) {
					event = lineNumber;
					latSit = line.substr(13, 1);
					lonSit = line.substr(18, 1);
				}

				if ( trimmedLine.substr(0, 42) == cutoff )
					throw LocatorException("ERROR! Epicentral distance out of XFAR range\nSet distance cutoff ON");

				lineNumber++;
			}
			ifile.close();

			if ( event == -1 ) {
				SEISCOMP_ERROR("%s Failed to determine ZTR value", MSG_HEADER);
				throw LocatorException("ERROR! Can't properly read output file\nPlease verify the output file");
			}

			double ER;

			ifstream file(_h71outputFile.c_str());
			while ( file.good() ) {
				getline(file, lineContent);
				if ( loop == event ) {
					try {
						string rms = lineContent.substr(63, 5);
						rms = stripSpace(rms);
						string erh = lineContent.substr(68, 5);
						erh = stripSpace(erh);
						string erz = lineContent.substr(73, 5);
						erz = stripSpace(erz);
						string depth = lineContent.substr(37, 7);
						depth = stripSpace(depth);

						string latDeg = lineContent.substr(18, 3);
						latDeg = stripSpace(latDeg);

						string latMin = lineContent.substr(22, 5);
						latMin = stripSpace(latMin);

						string lonDeg = lineContent.substr(28, 3);
						lonDeg = stripSpace(lonDeg);

						string lonMin = lineContent.substr(32, 5);
						lonMin = stripSpace(lonMin);

						string tLat = SexagesimalToDecimalHypo71(toDouble(latDeg), toDouble(latMin), latSit);
						string tLon = SexagesimalToDecimalHypo71(toDouble(lonDeg), toDouble(lonMin), lonSit);

						if ( stringIsOfType(erh, stInteger) && stringIsOfType(erz, stInteger) )
							ER = sqrt(pow(toDouble(erh), 2) + pow(toDouble(erz), 2));

						log << head << "|  RMS: " << formatString(rms, 10, 1) << "  LAT: " << tLat << endl;
						log << head << "|  ERH: " << formatString(erh, 10, 1) << "  LON: " << tLon << endl;
						log << head << "|  ERZ: " << formatString(erz, 10, 1) << "DEPTH: " << depth << "km" << endl;
						log << head << "|   ER: " << ER << endl;

						if ( (toDouble(rms) < minRMS) or (toDouble(rms) == minRMS) ) {

							if ( minDepth == "" )
								log << head << "|  ZTR set to " << depth << "km" << endl;
							else
								log << head << "|  " << minDepth << "km old ZTR is replaced by " << depth << "km" << endl;
							minDepth = depth;
							minRMS = toDouble(rms);

							if ( ER < minER ) {
								minER = ER;
								_trialLatDeg = latDeg;
								_trialLatMin = latMin;
								_trialLonDeg = lonDeg;
								_trialLonMin = lonMin;
							}
						}

					} catch ( ... ) {}
				}

				loop++;
			}
			file.close();
		}
	}

	sprintf(buf, "%#04.0f", toDouble(minDepth));
	string fdepth = toString(buf);

	log << head << "*---------------------------------------*" << endl;
	log << head << "|           ITERATIONS RESULTS          |" << endl;
	log << head << "*---------------------------------------*" << endl;
	log << head << "| MIN RMS VALUE: " << minRMS << endl;
	log << head << "|  MIN ER VALUE: " << minER << endl;
	log << head << "|  ZTR TO APPLY: " << fdepth << endl;
	log << head << "*---------------------------------------*" << endl;

	log.close();

	return fdepth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Hypo71::locate(PickList& pickList, double initLat, double initLon,
                       double initDepth, Time& initTime) throw (Core::GeneralException) {
	return locate(pickList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Hypo71::relocate(const Origin* origin) throw (Core::GeneralException) {

	if ( !origin )
		throw LocatorException("Initial origin is a NULL object. Nothing to do.");

	_currentOriginID = origin->publicID();
	_oLastLatitude = origin->latitude().value();
	_oLastLongitude = origin->longitude().value();
	_lastWarning = "";
	_stationMap.clear();

	// Make sure everything is properly randomized each time this method
	// gets instantiated...
	srand(time(0));

	bool emptyProfile = false;

	if ( !_currentProfile ) {
		emptyProfile = true;
		throw LocatorException(string("Please select a profile down the list !"));
	}

	PickList picks;
	for (size_t i = 0; i < origin->arrivalCount(); ++i) {

		double weight = 1.0;

		PickPtr pick = getPick(origin->arrival(i));

		if ( !pick )
			continue;

		SensorLocationPtr sloc = getSensorLocation(pick.get());

		//! Passing on it if station's sensor location isn't found
		//! Note: prevent this to happen by having fully implemented datalesses
		if ( !sloc )
			throw StationNotFoundException("Station '" + pick->waveformID().networkCode()
			        + "." + pick->waveformID().stationCode() + "' has not been found in database");

		try {
			if ( origin->arrival(i)->weight() <= 0 )
				weight = .0;
		} catch ( ... ) {}


		picks.push_back(WeightedPick(pick, weight));
		addNewStation(pick->waveformID().networkCode(), pick->waveformID().stationCode());
	}

	Origin* org;
	try {

		SEISCOMP_INFO("%s Proceeding to localization...", MSG_HEADER);
		for (StationMap::iterator i = _stationMap.begin();
		        i != _stationMap.end(); ++i)
			SEISCOMP_INFO("%s %s is now known as %s", MSG_HEADER, i->first.c_str(),
			    i->second.c_str());

		org = locate(picks);
	}
	catch ( GeneralException& exc ) {

		if ( emptyProfile )
			_currentProfile = NULL;
		throw exc;
	}

	return org;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Hypo71::addNewStation(const string& networkCode,
                           const string& stationCode) {

	string name = networkCode + "." + stationCode;

	if ( !getStationMappedCode(networkCode, stationCode).empty() ) {
		SEISCOMP_INFO("%s Ignored adding %s.%s to list, station alias is already registered",
		    MSG_HEADER, networkCode.c_str(), stationCode.c_str());
		return;
	}

	while ( getStationMappedCode(networkCode, stationCode).empty() ) {

		string id = genRandomString(4);
		if ( getOriginalStationCode(id).empty() )
			_stationMap.insert(make_pair(name, id));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::getStationMappedCode(const string& networkCode,
                                          const string& stationCode) {

	string name = networkCode + "." + stationCode;
	for (StationMap::iterator i = _stationMap.begin();
	        i != _stationMap.end(); ++i)
		if ( i->first == name )
			return i->second;

	return string("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Hypo71::getOriginalStationCode(const string& mappedCode) {

	for (StationMap::iterator i = _stationMap.begin();
	        i != _stationMap.end(); ++i)
		if ( i->second == mappedCode )
			return i->first;

	return string("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Hypo71::lastMessage(MessageType type) const {

	if ( type == Warning )
		return _lastWarning;
	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Hypo71::updateProfile(const string& name) {

	SEISCOMP_DEBUG("Update profile");

	_currentProfile = NULL;
	Profile* prof = NULL;
	for (Profiles::iterator it = _profiles.begin();
	        it != _profiles.end(); ++it) {

		if ( it->name != name )
			continue;

		prof = &(*it);
		break;
	}

	if ( prof == _currentProfile )
		return;

	_currentProfile = prof;
	_controlFile.clear();

	if ( !_currentProfile )
		return;


	// Unset all parameters
	for (ParameterMap::iterator it = _parameters.begin();
	        it != _parameters.end(); ++it)
		it->second = "";

	string controlFile;
	if ( !_currentProfile->controlFile.empty() )
		controlFile = _currentProfile->controlFile;
	else if ( !_controlFilePath.empty() )
		controlFile = _controlFilePath;

	if ( !controlFile.empty() ) {

		ConfigFile config(controlFile);
		string blank;

		// Reset list parameters
		string test01, test02, test03, test04, test05, test06, test07,
		        test08, test09, test10, test11, test12, test13, test15,
		        test20;
		config.readInto(test01, "TEST(01)", blank);
		setParameter("TEST(01)", test01);
		config.readInto(test02, "TEST(02)", blank);
		setParameter("TEST(02)", test02);
		config.readInto(test03, "TEST(03)", blank);
		setParameter("TEST(03)", test03);
		config.readInto(test04, "TEST(04)", blank);
		setParameter("TEST(04)", test04);
		config.readInto(test05, "TEST(05)", blank);
		setParameter("TEST(05)", test05);
		config.readInto(test06, "TEST(06)", blank);
		setParameter("TEST(06)", test06);
		config.readInto(test07, "TEST(07)", blank);
		setParameter("TEST(07)", test07);
		config.readInto(test08, "TEST(08)", blank);
		setParameter("TEST(08)", test08);
		config.readInto(test09, "TEST(09)", blank);
		setParameter("TEST(09)", test09);
		config.readInto(test10, "TEST(10)", blank);
		setParameter("TEST(10)", test10);
		config.readInto(test11, "TEST(11)", blank);
		setParameter("TEST(11)", test11);
		config.readInto(test12, "TEST(12)", blank);
		setParameter("TEST(12)", test12);
		config.readInto(test13, "TEST(13)", blank);
		setParameter("TEST(13)", test13);
		config.readInto(test15, "TEST(15)", blank);
		setParameter("TEST(15)", test15);
		config.readInto(test20, "TEST(20)", blank);
		setParameter("TEST(20)", test20);

		// Crustal Model list paramaters
		string cvm, cdm;
		config.readInto(cvm, "CRUSTAL_VELOCITY_MODEL", blank);
		setParameter("CRUSTAL_VELOCITY_MODEL", cvm);
		config.readInto(cdm, "CRUSTAL_DEPTH_MODEL", blank);
		setParameter("CRUSTAL_DEPTH_MODEL", cdm);

		// Control card parameters
		string ztr, xnear, xfar, pos, kms, kfm, imag, kaz, disableLastLoc;
		config.readInto(ztr, "ZTR", blank);
		setParameter("ZTR", ztr);
		config.readInto(xnear, "XNEAR", blank);
		setParameter("XNEAR", xnear);
		config.readInto(xfar, "XFAR", blank);
		setParameter("XFAR", xfar);
		config.readInto(pos, "POS", blank);
		setParameter("POS", pos);
		config.readInto(kms, "KMS", blank);
		setParameter("KMS", kms);
		config.readInto(kfm, "KFM", blank);
		setParameter("KFM", kfm);
		config.readInto(imag, "IMAG", blank);
		setParameter("IMAG", imag);
		config.readInto(kaz, "KAZ", blank);
		setParameter("KAZ", kaz);
		config.readInto(disableLastLoc, "DISABLE_LAST_LOC", blank);
		setParameter("DISABLE_LAST_LOC", disableLastLoc);

		// Instruction card parameters
		string knst, inst;
		config.readInto(knst, "KNST", blank);
		setParameter("KNST", knst);
		config.readInto(inst, "INST", blank);
		setParameter("INST", inst);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



