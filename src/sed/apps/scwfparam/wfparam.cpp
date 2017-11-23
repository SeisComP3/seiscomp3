/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#include "wfparam.h"
#include "util.h"
#include "msg.h"

#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>

#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/system.h>

#include <seiscomp3/client/inventory.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/records/mseedrecord.h>

#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/filter/butterworth.h>

#include <seiscomp3/utils/files.h>

#include <boost/bind.hpp>
#include <sys/wait.h>


using namespace std;
using namespace Seiscomp::Processing;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Private;


#define JOURNAL_ACTION           "WfParam"
#define JOURNAL_ACTION_COMPLETED "completed"
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {

namespace {


struct FreqOption : Application::Option {
	FreqOption(double *var,
	           const char *cfgname,
	           const char *cligroup = NULL,
	           const char *cliparam = NULL,
	           const char *clidesc = NULL,
	           bool clidefault = false)
	: Option(cfgname, cligroup, cliparam, clidesc, clidefault, false),
	  storage(var) {
		PGAV::Config::freqToString(_strValue, *storage);
	}

	void bind(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return;
		if ( cliGroup != NULL ) cli->addGroup(cliGroup);
		if ( cliSwitch )
			cli->addOption(cliGroup?cliGroup:"Generic",
			               cliParam, cliDesc);
		else
			cli->addOption(cliGroup?cliGroup:"Generic",
			               cliParam, cliDesc, &_strValue, cliDefault);
	}

	bool get(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return true;
		if ( cli->hasOption(cliParam) ) {
			int res = PGAV::Config::freqFromString(*storage, _strValue);
			if ( res ) {
				if ( res == 2 )
					cerr << "Error: in option '" << cliParam << "': "
					     << "invalid option value '" << _strValue << "': "
					     << "negative values are not allowed" << endl;
				else
					cerr << "Error: in option '" << cliParam << "': "
					     << "invalid option value '" << _strValue << "'" << endl;
				return false;
			}
		}
		return true;
	}

	bool get(const Seiscomp::Client::Application *app) {
		try {
			_strValue = app->configGetString(cfgName);
			int res = PGAV::Config::freqFromString(*storage, _strValue);
			if ( res ) {
				if ( res == 2 )
					SEISCOMP_ERROR("%s: invalid value '%s': negative values are not allowed",
					               cfgName, _strValue.c_str());
				else
					SEISCOMP_ERROR("%s: invalid value '%s'",
					               cfgName, _strValue.c_str());
				return false;
			}
		}
		catch ( ... ) {}
		return true;
	}

	void printStorage(std::ostream &os) {
		os << _strValue;
	}

	double *storage;
	string _strValue;
};


string toXML(const string &input) {
	string output;

	for ( size_t i = 0; i < input.size(); ++i ) {
		if ( input[i] == '&' )
			output += "&amp;";
		else if ( input[i] == '\"')
			output += "&quot;";
		else if ( input[i] == '\'')
			output += "&apos;";
		else if ( input[i] == '<')
			output += "&lt;";
		else if ( input[i] == '>')
			output += "&gt;";
		else
			output += input[i];
	}

	return output;
}


}

#define NEW_OPT(var, ...) addOption(&var, __VA_ARGS__)
#define NEW_OPT_FREQ(var, ...) addOption(new FreqOption(&var, __VA_ARGS__))
#define NEW_OPT_CLI(var, ...) addOption(&var, NULL, __VA_ARGS__)


enum MyNotifications {
	AcquisitionFinished  = -1
};


namespace {


Core::Time now;


pid_t startExternalProcess(const vector<string> &cmdparams) {
	pid_t pid;
	string cmdline;

	pid = fork();

	if ( pid < 0 )
		return pid;
	// Forked process
	else if ( pid == 0 ) {
		vector<char *> params(cmdparams.size());
		for ( size_t i = 0; i < cmdparams.size(); ++i ) {
			//cerr << toks[i] << endl;
			params[i] = (char*)cmdparams[i].c_str();
			if ( i > 0 ) cmdline += " ";
			cmdline += cmdparams[i];
		}
		params.push_back(NULL);

		SEISCOMP_DEBUG("$ %s", cmdline.c_str());
		execv(params[0], &params[0]);
		exit(1);
	}

	return pid;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WFParam::Config::Config() {
	totalTimeWindowLength = 360;
	preEventWindowLength = 60;

	maximumEpicentralDistance = 400;

	STAlength = 1;
	LTAlength = 60;
	STALTAratio = 3;
	STALTAmargin = 5;

	durationScale = 1.5;

	dampings.push_back(5);
	naturalPeriods = 100;
	naturalPeriodsLog = false;
	naturalPeriodsFixed = false;
	Tmin = 0;
	Tmax = 5;
	clipTmax = true;

	afterShockRemoval = true;
	eventCutOff = true;

	fExpiry = 1.0;

	// Second stage filter settings
	order = 4;
	filter.first = 0.025;
	filter.second = 40;

	// Post deconvolution filter, which is disabled by default
	PDorder = 4;
	PDfilter.first = 0;
	PDfilter.second = 0;

	enableDeconvolution = true;
	enableNonCausalFilters = false;
	taperLength = -1;
	padLength = -1;

	enableShortEventID = false;
	enableShakeMapXMLOutput = true;
	shakeMapOutputPath = "@LOGDIR@/shakemaps";
	shakeMapOutputScriptWait = true;
	shakeMapOutputSC3EventID = false;
	shakeMapOutputRegionName = false;
	shakeMapXMLEncoding = "UTF-8";

	waveformOutputPath = "@LOGDIR@/shakemaps/waveforms";
	waveformOutputEventDirectory = false;

	spectraOutputPath = "@LOGDIR@/shakemaps/spectra";
	spectraOutputEventDirectory = false;

	wakeupInterval = 10;

	initialAcquisitionTimeout = 30;
	runningAcquisitionTimeout = 2;

	eventMaxIdleTime = 3600;

	useMaximumOfHorizontals = false;

	testMode = false;
	offline = false;
	force = false;
	logCrontab = true;
	saveProcessedWaveforms = false;
	saveSpectraFiles = false;
	enableMessagingOutput = false;

	saturationThreshold = 80;

	updateDelay = 60;

	magnitudeTolerance = 0.5;

	dumpRecords = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::CompareWaveformStreamID::operator()(
	const Seiscomp::DataModel::WaveformStreamID &one,
	const Seiscomp::DataModel::WaveformStreamID &other
) const
{
	if ( one.networkCode() < other.networkCode() ) return true;
	if ( one.networkCode() > other.networkCode() ) return false;

	if ( one.stationCode() < other.stationCode() ) return true;
	if ( one.stationCode() > other.stationCode() ) return false;

	if ( one.locationCode() < other.locationCode() ) return true;
	if ( one.locationCode() > other.locationCode() ) return false;

	return one.channelCode() < other.channelCode();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::Process::hasBeenProcessed(DataModel::Stream *stream) const {
	PGAVResults::const_iterator it;
	for ( it = results.begin(); it != results.end(); ++it ) {
		if ( it->streamID.networkCode() != stream->sensorLocation()->station()->network()->code() )
			continue;

		if ( it->streamID.stationCode() != stream->sensorLocation()->station()->code() )
			continue;

		if ( it->streamID.locationCode() != stream->sensorLocation()->code() )
			continue;

		//if ( it->streamID.channelCode().compare(0,2,stream->code(),0,2) == 0 )
		if ( it->streamID.channelCode() == stream->code() )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WFParam::WFParam(int argc, char **argv) : Application(argc, argv) {
	setAutoApplyNotifierEnabled(true);
	setInterpretNotifierEnabled(true);

	setLoadInventoryEnabled(true);
	setLoadConfigModuleEnabled(true);

	setPrimaryMessagingGroup("AMPLITUDE");

	addMessagingSubscription("PICK");
	addMessagingSubscription("AMPLITUDE");
	addMessagingSubscription("MAGNITUDE");
	addMessagingSubscription("LOCATION");
	addMessagingSubscription("EVENT");

	setAutoAcquisitionStart(false);

	_cache.setPopCallback(boost::bind(&WFParam::removedFromCache, this, _1));

	_processingInfoChannel = NULL;
	_processingInfoOutput = NULL;

	_acquisitionTimeout = 0;

	NEW_OPT(_config.streamsWhiteList, "wfparam.streams.whitelist");
	NEW_OPT(_config.streamsBlackList, "wfparam.streams.blacklist");
	NEW_OPT(_config.totalTimeWindowLength, "wfparam.totalTimeWindowLength");
	NEW_OPT(_config.vecMagnitudeTimeWindowTable, "wfparam.magnitudeTimeWindowTable");
	NEW_OPT(_config.vecMagnitudeFilterTable, "wfparam.magnitudeFilterTable");
	NEW_OPT(_config.preEventWindowLength, "wfparam.preEventWindowLength");
	NEW_OPT(_config.vecMagnitudeDistanceTable, "wfparam.magnitudeDistanceTable");
	NEW_OPT(_config.maximumEpicentralDistance, "wfparam.maximumEpicentralDistance");
	NEW_OPT(_config.saturationThreshold, "wfparam.saturationThreshold");
	NEW_OPT(_config.STAlength, "wfparam.STAlength");
	NEW_OPT(_config.LTAlength, "wfparam.LTAlength");
	NEW_OPT(_config.STALTAratio, "wfparam.STALTAratio");
	NEW_OPT(_config.STALTAmargin, "wfparam.STALTAmargin");
	NEW_OPT(_config.durationScale, "wfparam.durationScale");
	NEW_OPT(_config.dampings, "wfparam.dampings");
	NEW_OPT(_config.naturalPeriodsStr, "wfparam.naturalPeriods");
	NEW_OPT(_config.naturalPeriodsLog, "wfparam.naturalPeriods.log");
	NEW_OPT(_config.Tmin, "wfparam.Tmin");
	NEW_OPT(_config.Tmax, "wfparam.Tmax");
	NEW_OPT(_config.clipTmax, "wfparam.clipTmax");
	NEW_OPT(_config.afterShockRemoval, "wfparam.afterShockRemoval");
	NEW_OPT(_config.eventCutOff, "wfparam.eventCutOff");
	NEW_OPT(_config.order, "wfparam.filter.order",
	        "Mode", "order", "filter order");
	NEW_OPT_FREQ(_config.filter.first, "wfparam.filter.loFreq",
	             "Mode", "lo-filter", "high-pass filter frequency");
	NEW_OPT_FREQ(_config.filter.second, "wfparam.filter.hiFreq",
	             "Mode", "hi-filter", "low-pass filter frequency");
	NEW_OPT(_config.PDorder, "wfparam.pd.order",
	        "Mode", "sc-order", "sensitivity correction filter order");
	NEW_OPT_FREQ(_config.PDfilter.first, "wfparam.pd.loFreq",
	             "Mode", "pd-lo-filter", "post deconvolution high-pass filter frequency");
	NEW_OPT_FREQ(_config.PDfilter.second, "wfparam.pd.hiFreq",
	             "Mode", "pd-hi-filter", "post deconvolution low-pass filter frequency");
	NEW_OPT(_config.enableDeconvolution, "wfparam.deconvolution");
	NEW_OPT(_config.enableNonCausalFilters, "wfparam.filtering.noncausal");
	NEW_OPT(_config.taperLength, "wfparam.filtering.taperLength");
	NEW_OPT(_config.padLength, "wfparam.filtering.padLength");
	NEW_OPT(_config.wakeupInterval, "wfparam.cron.wakeupInterval");
	NEW_OPT(_config.eventMaxIdleTime, "wfparam.cron.eventMaxIdleTime");
	NEW_OPT(_config.logCrontab, "wfparam.cron.logging");
	NEW_OPT(_config.updateDelay, "wfparam.cron.updateDelay");
	NEW_OPT(_config.delayTimes, "wfparam.cron.delayTimes");
	NEW_OPT(_config.initialAcquisitionTimeout, "wfparam.acquisition.initialTimeout");
	NEW_OPT(_config.runningAcquisitionTimeout, "wfparam.acquisition.runningTimeout");
	NEW_OPT(_config.enableMessagingOutput, "wfparam.output.messaging");
	NEW_OPT(_config.saveProcessedWaveforms, "wfparam.output.waveforms.enable");
	NEW_OPT(_config.waveformOutputPath, "wfparam.output.waveforms.path");
	NEW_OPT(_config.waveformOutputEventDirectory, "wfparam.output.waveforms.withEventDirectory");
	NEW_OPT(_config.saveSpectraFiles, "wfparam.output.spectra.enable");
	NEW_OPT(_config.spectraOutputPath, "wfparam.output.spectra.path");
	NEW_OPT(_config.spectraOutputEventDirectory, "wfparam.output.spectra.withEventDirectory");
	NEW_OPT(_config.enableShortEventID, "wfparam.output.shortEventID");
	NEW_OPT(_config.enableShakeMapXMLOutput, "wfparam.output.shakeMap.enable");
	NEW_OPT(_config.useMaximumOfHorizontals, "wfparam.output.shakeMap.maximumOfHorizontals");
	NEW_OPT(_config.shakeMapOutputPath, "wfparam.output.shakeMap.path");
	NEW_OPT(_config.shakeMapOutputScript, "wfparam.output.shakeMap.script");
	NEW_OPT(_config.shakeMapOutputScriptWait, "wfparam.output.shakeMap.synchronous");
	NEW_OPT(_config.shakeMapOutputSC3EventID, "wfparam.output.shakeMap.SC3EventID");
	NEW_OPT(_config.shakeMapOutputRegionName, "wfparam.output.shakeMap.regionName");
	NEW_OPT(_config.shakeMapXMLEncoding, "wfparam.output.shakeMap.encoding");
	NEW_OPT(_config.magnitudeTolerance, "wfparam.magnitudeTolerance");
	NEW_OPT_CLI(_config.fExpiry, "Generic", "expiry,x",
	            "Time span in hours after which objects expire", true);
	NEW_OPT_CLI(_config.eventID, "Generic", "event-id,E",
	            "EventID to calculate amplitudes for", true);
	NEW_OPT_CLI(_config.eventParameterFile, "Generic", "ep",
	            "EventParameters (XML) to load", false);
	NEW_OPT_CLI(_config.offline, "Mode", "offline",
	            "Do not connect to the messaging and to the database",
	            false, true);
	NEW_OPT_CLI(_config.force, "Mode", "force",
	            "Force event processing even if a journal entry exists that processing has completed",
	            false, true);
	NEW_OPT_CLI(_config.testMode, "Messaging", "test",
	            "Test mode, no messages are sent", false, true);
	NEW_OPT_CLI(_config.dumpRecords, "Mode", "dump-records",
	            "Dumps all received records (binary) to [eventid].recs", false, true);

	/*
	cout << "<configuration-options>" << endl;
	cout << "  <cfg>" << endl;
	for ( Options::const_iterator it = options().begin(); it != options().end(); ++it ) {
		if ( (*it)->cfgName == NULL ) continue;
		cout << " {{{";
		cout << (*it)->cfgName << " [";
		(*it)->printStorage(cout);
		cout << "]}}}::" << endl;
		cout << "   Doku..." << endl;
	}
	cout << "  </cfg>" << endl;

	cout << "  <cli>" << endl;
	for ( Options::const_iterator it = options().begin(); it != options().end(); ++it ) {
		if ( (*it)->cliParam == NULL ) continue;
		cout << " {{{--";
		cout << (*it)->cliParam;
		if ( (*it)->cfgName == NULL ) {
			cout << " [";
			(*it)->printStorage(cout);
			cout << "]}}}::" << endl;
			if ( (*it)->cliDesc )
				cout << "   " << (*it)->cliDesc << endl;
			else
				cout << "   Doku..." << endl;
		}
		else {
			cout << "}}}::" << endl;
			cout << "   Overrides param {{{" << (*it)->cfgName << "}}}" << endl;
		}
	}
	cout << "  </cli>" << endl;
	cout << "</configuration-options>" << endl;
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WFParam::~WFParam() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::createCommandLineDescription() {
	Application::createCommandLineDescription();
	commandline().addOption("Mode", "dump-config", "Dump the configuration and exit");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::validateParameters() {
	if ( !Application::validateParameters() ) return false;

	if ( !_config.eventID.empty() && !_config.enableMessagingOutput )
		setMessagingEnabled(false);

	if ( _config.naturalPeriodsStr == "fixed" )
		_config.naturalPeriodsFixed = true;
	else {
		if ( !Core::fromString(_config.naturalPeriods, _config.naturalPeriodsStr) ) {
			SEISCOMP_ERROR("wfparam.naturalPeriods: "
			               "neither valid int value '%s' nor 'fixed' found",
			               _config.naturalPeriodsStr.c_str());
		}

		_config.naturalPeriodsFixed = false;
	}

	if ( _config.offline )
		// If the inventory is provided by an XML file and
		// an event XML is provided, disable the database
		if ( !isInventoryDatabaseEnabled() && !_config.eventParameterFile.empty() )
			setDatabaseEnabled(false, false);

	for ( size_t i = 0; i < _config.vecMagnitudeTimeWindowTable.size(); ++i ) {
		string &item = _config.vecMagnitudeTimeWindowTable[i];
		size_t pos = item.find(':');
		if ( pos == string::npos ) {
			SEISCOMP_ERROR("wfparam.magnitudeTimeWindowTable:%d: missing ':'",
			               (int)(i+1));
			return false;
		}

		double mag, value;
		if ( !Core::fromString(mag, item.substr(0, pos)) ) {
			SEISCOMP_ERROR("wfparam.magnitudeTimeWindowTable:%d: "
			               "invalid double value '%s'",
			               (int)(i+1), item.substr(0, pos).c_str());
			return false;
		}

		if ( !Core::fromString(value, item.substr(pos+1)) ) {
			SEISCOMP_ERROR("wfparam.magnitudeTimeWindowTable:%d: "
			               "invalid double value '%s'",
			               (int)(i+1), item.substr(pos+1).c_str());
			return false;
		}

		_config.magnitudeTimeWindowTable[mag] = value;
	}

	for ( size_t i = 0; i < _config.vecMagnitudeDistanceTable.size(); ++i ) {
		string &item = _config.vecMagnitudeDistanceTable[i];
		size_t pos = item.find(':');
		if ( pos == string::npos ) {
			SEISCOMP_ERROR("wfparam.magnitudeDistanceTable:%d: missing ':'",
			               (int)(i+1));
			return false;
		}

		double mag, dist;
		if ( !Core::fromString(mag, item.substr(0, pos)) ) {
			SEISCOMP_ERROR("wfparam.magnitudeDistanceTable:%d: "
			               "invalid double value '%s'",
			               (int)(i+1), item.substr(0, pos).c_str());
			return false;
		}

		if ( !Core::fromString(dist, item.substr(pos+1)) ) {
			SEISCOMP_ERROR("wfparam.magnitudeDistanceTable:%d: "
			               "invalid double value '%s'",
			               (int)(i+1), item.substr(pos+1).c_str());
			return false;
		}

		_config.magnitudeDistanceTable[mag] = dist;
	}

	for ( size_t i = 0; i < _config.vecMagnitudeFilterTable.size(); ++i ) {
		string &item = _config.vecMagnitudeFilterTable[i];
                if ( item.empty() ) continue;
		size_t pos = item.find(':');
		if ( pos == string::npos ) {
			SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: missing ':'",
			               (int)(i+1));
			return false;
		}

		FilterFreqs freqs;
		double mag;

		if ( !Core::fromString(mag, item.substr(0, pos)) ) {
			SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: "
			               "invalid double value '%s'",
			               (int)(i+1), item.substr(0, pos).c_str());
			return false;
		}

		string strFreqs = item.substr(pos+1);
		pos = strFreqs.find(';');
		if ( pos == string::npos ) {
			SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: missing ';' as frequency separator",
			               (int)(i+1));
			return false;
		}

		string strFMin = strFreqs.substr(0,pos);
		Core::trim(strFMin);
		int r = PGAV::Config::freqFromString(freqs.first, strFMin);
		if ( r ) {
			if ( r == 2 )
				SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: "
				               "negative value for fmin is not allowed '%s'",
				               (int)(i+1), strFMin.c_str());
			else
				SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: "
				               "invalid value for fmin '%s'",
				               (int)(i+1), strFMin.c_str());
			return false;
		}

		string strFMax = strFreqs.substr(pos+1);
		Core::trim(strFMax);

		r = PGAV::Config::freqFromString(freqs.second, strFMax);
		if ( r ) {
			if ( r == 2 )
				SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: "
				               "negative value for fmax is not allowed '%s'",
				               (int)(i+1), strFMax.c_str());
			else
				SEISCOMP_ERROR("wfparam.magnitudeFilterTable:%d: "
				               "invalid value for fmax '%s'",
				               (int)(i+1), strFMax.c_str());
			return false;
		}

		_config.magnitudeFilterTable[mag] = freqs;
	}

	// Resolve placeholders
	_config.shakeMapOutputScript = Environment::Instance()->absolutePath(_config.shakeMapOutputScript);
	_config.shakeMapOutputPath = Environment::Instance()->absolutePath(_config.shakeMapOutputPath);
	if ( !_config.shakeMapOutputPath.empty() && *_config.shakeMapOutputPath.rbegin() != '/' )
		_config.shakeMapOutputPath += '/';

	_config.waveformOutputPath = Environment::Instance()->absolutePath(_config.waveformOutputPath);
	if ( !_config.waveformOutputPath.empty() && *_config.waveformOutputPath.rbegin() != '/' )
		_config.waveformOutputPath += '/';

	_config.spectraOutputPath = Environment::Instance()->absolutePath(_config.spectraOutputPath);
	if ( !_config.spectraOutputPath.empty() && *_config.spectraOutputPath.rbegin() != '/' )
		_config.spectraOutputPath += '/';

	if ( commandline().hasOption("dump-config") ) {
		for ( Options::const_iterator it = options().begin(); it != options().end(); ++it ) {
			if ( (*it)->cfgName )
				cout << (*it)->cfgName;
			else if ( (*it)->cliParam)
				cout << "--" << (*it)->cliParam;
			else
				continue;

			cout << ": ";
			(*it)->printStorage(cout);
			cout << endl;
		}

		return false;
	}

	if ( _config.enableShakeMapXMLOutput ) {
		if ( _config.shakeMapOutputPath != "-" &&
		     !Util::pathExists(_config.shakeMapOutputPath) ) {
			if ( !Util::createPath(_config.shakeMapOutputPath) ) {
				SEISCOMP_ERROR("Unable to create shakeMap output path: %s",
				               _config.shakeMapOutputPath.c_str());
				return false;
			}
		}
	}

	if ( _config.saveProcessedWaveforms ) {
		if ( !Util::pathExists(_config.waveformOutputPath) ) {
			if ( !Util::createPath(_config.waveformOutputPath) ) {
				SEISCOMP_ERROR("Unable to create waveform output directory: %s",
				               _config.waveformOutputPath.c_str());
				return false;
			}
		}
	}

	if ( _config.saveSpectraFiles ) {
		if ( !Util::pathExists(_config.spectraOutputPath) ) {
			if ( !Util::createPath(_config.spectraOutputPath) ) {
				SEISCOMP_ERROR("Unable to create spectra output directory: %s",
				               _config.spectraOutputPath.c_str());
				return false;
			}
		}
	}

	// Check and add 5% damping if shakemap output is enabled
	if ( _config.enableShakeMapXMLOutput ) {
		std::vector<double>::iterator it =
			std::find(_config.dampings.begin(), _config.dampings.end(), 5);
		if ( it == _config.dampings.end() )
			_config.dampings.push_back(5);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::init() {
	if ( !Application::init() )
		return false;

	// Construct stream firewall
	for ( size_t i = 0; i < _config.streamsWhiteList.size(); ++i ) {
		Core::trim(_config.streamsWhiteList[i]);
		if ( !_config.streamsWhiteList[i].empty() ) {
			SEISCOMP_DEBUG("Adding pattern to stream whitelist: %s",
			               _config.streamsWhiteList[i].c_str());
			_streamFirewall.allow.insert(_config.streamsWhiteList[i]);
		}
	}

	for ( size_t i = 0; i < _config.streamsBlackList.size(); ++i ) {
		Core::trim(_config.streamsBlackList[i]);
		if ( !_config.streamsBlackList[i].empty() ) {
			SEISCOMP_DEBUG("Adding pattern to stream blacklist: %s",
			               _config.streamsBlackList[i].c_str());
			_streamFirewall.deny.insert(_config.streamsBlackList[i]);
		}
	}

	if ( _config.dumpRecords )
		setRecordInputHint(Record::Hint(Record::SAVE_RAW));

	if ( !_config.eventParameterFile.empty() ) {
		IO::XMLArchive ar;
		if ( !ar.open(_config.eventParameterFile.c_str()) ) {
			cerr << "Unable to open " << _config.eventParameterFile << endl;
			return false;
		}

		ar >> _eventParameters;
		if ( !_eventParameters ) {
			cerr << "No event parameters found in " << _config.eventParameterFile << endl;
			return false;
		}
	}

	// Log into processing/info to avoid logging the same information into the global info channel
	_processingInfoChannel = SEISCOMP_DEF_LOGCHANNEL("processing/info", Logging::LL_INFO);
	_processingInfoOutput = new Logging::FileRotatorOutput(Environment::Instance()->logFile("scwfparam-processing-info").c_str(),
	                                                       60*60*24, 30);

	_processingInfoOutput->subscribe(_processingInfoChannel);

	_cache.setTimeSpan(Core::TimeSpan(_config.fExpiry*3600.));
	_cache.setDatabaseArchive(query());

	// Check each 10 seconds if a new job needs to be started
	enableTimer(1);
	_cronCounter = _config.wakeupInterval;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::run() {
	if ( !_config.eventID.empty() ) {
		EventPtr evt = _cache.get<Event>(_config.eventID);
		if ( !evt ) {
			cerr << "Event " << _config.eventID << " not found." << endl;
			return false;
		}

		// In offline mode start processing immediately
		if ( _config.offline ) {
			_config.delayTimes.clear();
			_config.updateDelay = 0;
		}

		if ( !addProcess(evt.get()) )
			return false;
	}

	return Application::run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::done() {
	Application::done();

	// Remove crontab log file if exists
	unlink((Environment::Instance()->logDir() + "/" + name() + ".sched").c_str());

	if ( _processingInfoChannel ) delete _processingInfoChannel;
	if ( _processingInfoOutput ) delete _processingInfoOutput;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::handleMessage(Core::Message *msg) {
	// Each message is taken as an transaction.
	_todos.clear();

	Application::handleMessage(msg);

	Todos::iterator it;
	for ( it = _todos.begin(); it != _todos.end(); ++it )
		addProcess(it->get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::addObject(const string& parentID, DataModel::Object* object) {
	updateObject(parentID, object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::updateObject(const string &parentID, Object* object) {
	Pick *pick = Pick::Cast(object);
	if ( pick ) {
		feed(pick);
		return;
	}

	Origin *origin = Origin::Cast(object);
	if ( origin ) {
		_cache.feed(origin);
		return;
	}

	Event *event = Event::Cast(object);
	if ( event ) {
		if ( !event->registered() ) {
			EventPtr cached = Event::Find(event->publicID());
			if ( cached ) {
				_todos.insert(cached.get());
				return;
			}
		}

		_todos.insert(event);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::handleTimeout() {
	--_cronCounter;

	if ( _cronCounter <= 0 ) {
		// Reset counter
		_cronCounter = _config.wakeupInterval;

		Crontab::iterator it;
		now = Core::Time::GMT();

		// Update crontab
		for ( it = _crontab.begin(); it != _crontab.end(); ) {
			Cronjob *job = it->second.get();

			Processes::iterator pit = _processes.find(it->first);
			ProcessPtr proc = (pit == _processes.end()?NULL:pit->second);

			if ( proc == NULL ) {
				SEISCOMP_WARNING("No processor for cronjob %s", it->first.c_str());
				++it;
				continue;
			}

			// Skip processes where nextRun is not set
			if ( job->runTimes.empty() ) {
				// Jobs stopped for more than a day now?
				if ( (now - proc->lastRun).seconds() >= _config.eventMaxIdleTime ) {
					SEISCOMP_DEBUG("Process %s idle time expired, removing",
					               proc->event->publicID().c_str());
					removeProcess(it, proc.get());
				}
				else
					++it;

				continue;
			}

			Core::Time nextRun = job->runTimes.front();

			// Time of next run in future?
			if ( nextRun > now ) {
				++it;
				continue;
			}

			// Remove all times in the past
			while ( !job->runTimes.empty() && (job->runTimes.front() <= now) )
				job->runTimes.pop_front();

			// Add eventID to processQueue if not already inserted
			if ( find(_processQueue.begin(), _processQueue.end(), proc) ==
				 _processQueue.end() ) {
				SEISCOMP_DEBUG("Pushing %s to process queue",
				               proc->event->publicID().c_str());
				_processQueue.push_back(proc);
			}

			/*
			// No more jobs to start later?
			if ( !job->nextRun.valid() ) {
				_crontab.erase(it++);
				continue;
			}
			*/

			++it;
		}

		// Process event queue if no acquisition thread is currently running
		if ( !isRecordThreadActive() && !_processQueue.empty() ) {
			ProcessPtr proc = _processQueue.front();
			_processQueue.pop_front();
			startProcess(proc.get());
		}
		else if ( isRecordThreadActive() && !_processQueue.empty() )
			SEISCOMP_DEBUG("Acquistion active, starting next process deferred");

		// Dump crontab if activated
		if ( _config.logCrontab ) {
			ofstream of((Environment::Instance()->logDir() + "/" + name() + ".sched").c_str());
			of << "Now: " << now.toString("%F %T") << endl;
			of << "------------------------" << endl;
			of << "[Schedule]" << endl;
			for ( it = _crontab.begin(); it != _crontab.end(); ++it ) {
				if ( !it->second->runTimes.empty() )
					of << it->second->runTimes.front().toString("%F %T") << "\t" << it->first
					   << "\t" << (it->second->runTimes.front()-now).seconds() << endl;
				else
					of << "STOPPED            \t" << it->first << endl;
			}

			// Dump process queue if not empty
			if ( !_processQueue.empty() || _currentProcess ) {
				of << endl << "[Queue]" << endl;

				ProcessQueue::iterator it;
				for ( it = _processQueue.begin(); it != _processQueue.end(); ++it )
					of << "WAITING            \t" << (*it)->event->publicID() << endl;
				if ( _currentProcess )
					of << "RUNNING            \t" << _currentProcess->event->publicID() << endl;
			}
		}
	}

	// Check acquisition timeout
	if ( recordStream() && _acquisitionTimeout > 0 && !_receivedRecords ) {
		if ( _acquisitionTimer.elapsed().seconds() >= _acquisitionTimeout )
			recordStream()->close();
	}

	_receivedRecords = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::addProcess(DataModel::Event *evt) {
	_cache.feed(evt);

	if ( !_config.eventID.empty() && (evt->publicID() != _config.eventID) ) {
		SEISCOMP_NOTICE("%s: event ignored: only event %s is allowed for processing",
		                evt->publicID().c_str(), _config.eventID.c_str());
		return false;
	}

	if ( _processedEvents.find(evt->publicID()) != _processedEvents.end() ) {
		SEISCOMP_DEBUG("%s: event has been completely processed already",
		               evt->publicID().c_str());
		return false;
	}

	now = Core::Time::GMT();

	OriginPtr org = _cache.get<Origin>(evt->preferredOriginID());
	if ( !org ) {
		SEISCOMP_WARNING("%s: preferred origin %s not found",
		                 evt->publicID().c_str(), evt->preferredOriginID().c_str());
		return false;
	}

	if ( isAgencyIDBlocked(objectAgencyID(org.get())) ) {
		SEISCOMP_INFO("%s: preferred origin's agencyID '%s' is blocked",
		              evt->publicID().c_str(), objectAgencyID(org.get()).c_str());
		return false;
	}

	MagnitudePtr mag = _cache.get<Magnitude>(evt->preferredMagnitudeID());
	if ( !mag ) {
		if ( evt->preferredMagnitudeID().empty() )
			SEISCOMP_WARNING("%s: no preferred magnitude",
			                 evt->publicID().c_str());
		else
			SEISCOMP_WARNING("%s: preferred magnitude %s not found",
			                 evt->publicID().c_str(), evt->preferredMagnitudeID().c_str());
		return false;
	}

	// New process?
	ProcessPtr proc;
	Processes::iterator pit = _processes.find(evt->publicID());
	if ( pit == _processes.end() ) {
		if ( !_config.force ) {
			if ( query() ) {
				DatabaseIterator it;
				JournalEntryPtr entry;
				it = query()->getJournalAction(evt->publicID(), JOURNAL_ACTION);
				while ( (entry = static_cast<JournalEntry*>(*it)) != NULL ) {
					if ( entry->parameters() == JOURNAL_ACTION_COMPLETED ) {
						SEISCOMP_INFO("%s: found journal entry \"completely processed\", ignoring event",
						              evt->publicID().c_str());
						it.close();
						return false;
					}
					++it;
				}
				it.close();
				SEISCOMP_DEBUG("No journal entry \"completely processed\" found, go ahead");
			}
		}
		else
			SEISCOMP_DEBUG("Force processing, journal ignored");

		SEISCOMP_DEBUG("Adding process [%s]", evt->publicID().c_str());
		proc = new Process;
		proc->created = now;
		proc->event = evt;
		_processes[evt->publicID()] = proc;
	}
	else
		proc = pit->second;

	Core::Time nextRun = now + Core::TimeSpan(_config.updateDelay);

	Crontab::iterator it = _crontab.find(evt->publicID());
	if ( it != _crontab.end() ) {
		// Update reference time
		try {
			proc->referenceTime = org->time().value();
		}
		catch ( ... ) {
			proc->referenceTime = Core::Time();
		}

		// Process currently stopped?
		if ( it->second->runTimes.empty() ) {
			it->second->runTimes.push_back(nextRun);
			SEISCOMP_DEBUG("Update delay = %ds, next run at %s",
			               _config.updateDelay, nextRun.toString("%FT%T").c_str());
		}
		else {
			// Insert next run into queue
			Core::Time first = it->second->runTimes.front();
			if ( (first-nextRun).seconds() >= _config.updateDelay )
				it->second->runTimes.push_front(nextRun);
		}

		return true;
	}

	CronjobPtr job = new Cronjob;
	try {
		proc->referenceTime = org->time().value();
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Could not read origin time of event %s", evt->publicID().c_str());
		return false;
	}

	// Debug to test next run in 20 seconds
	//job->delayTimes.push_back(now-job->referenceTime+Core::TimeSpan(20));

	if ( _config.delayTimes.empty() )
		job->runTimes.push_back(nextRun);
	else {
		for ( size_t i = 0; i < _config.delayTimes.size(); ++i )
			job->runTimes.push_back(proc->referenceTime + Core::TimeSpan(_config.delayTimes[i]));
	}

	SEISCOMP_DEBUG("%s: adding new cronjob", evt->publicID().c_str());
	_crontab[evt->publicID()] = job;
	handleTimeout();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::startProcess(Process *proc) {
	SEISCOMP_DEBUG("Starting process [%s]", proc->event->publicID().c_str());
	_currentProcess = proc;
	_currentProcess->newValidResults = 0;
	_currentProcess->lastRun = now;

	MagnitudePtr mag = _cache.get<Magnitude>(proc->event->preferredMagnitudeID());
	if ( mag ) {
		double mval = mag->magnitude().value();
		if ( !proc->lastMagnitude ||
		     (fabs(*proc->lastMagnitude-mval) > _config.magnitudeTolerance) ) {
			SEISCOMP_DEBUG("Reprocess event, magnitude = %.2f", mval);
			proc->results.clear();
			proc->lastMagnitude = mval;
		}
		else {
			SEISCOMP_DEBUG("Processing remaining channels, process magnitude = %.2f, current magnitude = %.2f",
			               *proc->lastMagnitude, mval);
		}
	}
	else
		return false;

	if ( !handle(proc->event.get()) ) {
		_currentProcess = NULL;
		return false;
	}

	if ( !isRecordThreadActive() ) {
		_currentProcess = NULL;
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::stopProcess(Process *proc) {
	Crontab::iterator cit = _crontab.find(proc->event->publicID());
	if ( cit != _crontab.end() ) cit->second->runTimes.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::removeProcess(WFParam::Crontab::iterator &it, Process *proc) {
	bool doExit = !_config.eventID.empty() && proc->event->publicID() == _config.eventID;

	// Remove process from process map
	Processes::iterator pit = _processes.find(proc->event->publicID());
	if ( pit != _processes.end() ) _processes.erase(pit);

	// Remove process from queue
	ProcessQueue::iterator qit = find(_processQueue.begin(), _processQueue.end(),
	                                  proc);
	if ( qit != _processQueue.end() ) _processQueue.erase(qit);

	// Remove cronjob
	_crontab.erase(it++);

	if ( doExit ) quit();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::handle(Seiscomp::DataModel::Event *evt) {
	OriginPtr org = _cache.get<Origin>(evt->preferredOriginID());
	if ( !org ) {
		cerr << "Preferred origin " << evt->preferredOriginID() << " not found." << endl;
		return false;
	}

	// Copy default values
	_maximumEpicentralDistance = _config.maximumEpicentralDistance;
	_totalTimeWindowLength = _config.totalTimeWindowLength;
	_filter = _config.filter;

	MagnitudePtr mag = _cache.get<Magnitude>(evt->preferredMagnitudeID());
	if ( mag ) {
		try {
			// Load magnitude dependent maximum distance
			getValue(_maximumEpicentralDistance,
			         _config.magnitudeDistanceTable,
			         mag->magnitude().value());

			// Load magnitude dependent maximum time window
			getValue(_totalTimeWindowLength,
			         _config.magnitudeTimeWindowTable,
			         mag->magnitude().value());

			// Load magnitude dependent filter settings
			getValue(_filter, _config.magnitudeFilterTable,
			         mag->magnitude().value());
		}
		catch ( ... ) {}
	}

	process(org.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::handle(Seiscomp::DataModel::Origin *org) {
	// Copy default values
	_maximumEpicentralDistance = _config.maximumEpicentralDistance;
	_totalTimeWindowLength = _config.totalTimeWindowLength;
	_currentProcess = NULL;

	process(org);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::process(Origin *origin) {
	if ( !origin ) return;

	if ( origin->arrivalCount() == 0 && query() )
		query()->loadArrivals(origin);

	if ( Private::status(origin) == REJECTED ) {
		SEISCOMP_INFO("Ignoring origin %s with status = REJECTED",
		              origin->publicID().c_str());
		return;
	}

	Client::Inventory *inv = Client::Inventory::Instance();
	DataModel::Inventory *inventory = inv->inventory();
	if ( inventory == NULL ) {
		SEISCOMP_ERROR("Inventory not available");
		return;
	}

	// Clear all processors
	_processors.clear();

	// Clear all station time windows
	_stationRequests.clear();

	if ( recordStream() == NULL ) {
		if ( !openStream() ) {
			SEISCOMP_ERROR("%s: unable to open stream", recordStreamURL().c_str());
			return;
		}
	}

	// Typedef a pickmap entry containing the pick and
	// the distance of the station from the origin
	typedef pair<PickCPtr, double> PickStreamEntry;

	// Typedef a pickmap that maps a streamcode to a pick
	typedef map<string, PickStreamEntry> PickStreamMap;

	// This map is needed to find the earliest P pick of
	// a certain stream
	PickStreamMap pickStreamMap;

	try {
		_originTime = origin->time().value();
		_latitude = origin->latitude().value();
		_longitude = origin->longitude().value();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("Ignoring origin %s with unset lat/lon or time",
		                 origin->publicID().c_str());
		return;
	}

	try { _depth = origin->depth().value(); }
	catch ( ... ) { _depth = 10; }

	_report << endl;
	_report << "Processing report for event: " << _currentProcess->event->publicID() << endl;
	_report << "-----------------------------------------------------------------" << endl;
	_report << " + Hypocenter" << endl;
	_report << "   + origin " << origin->publicID() << endl;
	_report << " + Parameters" << endl;
	if ( _currentProcess->lastMagnitude )
		_report << "   + magnitude = " << *_currentProcess->lastMagnitude << endl;
	else
		_report << "   + magnitude is none" << endl;
	_report << "   + saturation threshold = " << _config.saturationThreshold << "% of 2**23" << endl;
	_report << "   + maximum epicentral distance = " << _maximumEpicentralDistance << "km" << endl;
	_report << "   + pre event window length = " << _config.preEventWindowLength << "s" << endl;
	_report << "   + total time window length = " << _totalTimeWindowLength << "s" << endl;
	_report << "   + sta/lta/ratio = " << _config.STAlength << "/"
	                             << _config.LTAlength << "/"
	                             << _config.STALTAratio << endl;
	_report << "   + aftershock removal = " << (_config.afterShockRemoval?"on":"off") << endl;
	_report << "   + pre event cut off = " << (_config.eventCutOff?"on":"off") << endl;

	_report << " + Stations" << endl;

	// Reset remaining channels
	_currentProcess->remainingChannels = 0;

	set<string> usedStations;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Arrival *arr = origin->arrival(i);
		const string &pickID = arr->pickID();

		double weight = Private::arrivalWeight(arr);

		if ( Private::shortPhaseName(arr->phase().code()) != 'P' || weight < 0.5 ) {
			SEISCOMP_INFO("Ignoring pick '%s' weight=%.1f phase=%s",
			              pickID.c_str(), weight, arr->phase().code().c_str());
			continue;
		}

		PickPtr pick = _cache.get<Pick>(pickID);
		if ( !pick ) {
			_report << "   - " << pickID << " [pick not found]" << endl;
			continue;
		}

		DataModel::WaveformStreamID wfid = pick->waveformID();
		// Strip the component code because every AmplitudeProcessor
		// will use its own component to pick the amplitude on
		wfid.setChannelCode(wfid.channelCode().substr(0,2));

		string stationID = Private::toStationID(wfid);
		PickStreamEntry &e = pickStreamMap[stationID];

		// When there is already a pick registered for this stream which has
		// been picked earlier, ignore the current pick
		if ( e.first && e.first->time().value() < pick->time().value() )
			continue;

		e.first = pick;
		e.second = Private::arrivalDistance(arr);

		usedStations.insert(stationID);
	}

	for ( size_t n = 0; n < inventory->networkCount(); ++n ) {
		DataModel::Network *net = inventory->network(n);
		if ( net->start() > _originTime ) continue;
		try { if ( net->end() < _originTime ) continue; }
		catch ( ... ) {}

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			DataModel::Station *sta = net->station(s);
			if ( sta->start() > _originTime ) continue;
			try { if ( sta->end() < _originTime ) continue; }
			catch ( ... ) {}

			double distance, az, baz;
			Math::Geo::delazi(_latitude, _longitude,
			                  sta->latitude(), sta->longitude(),
			                  &distance, &az, &baz);
			distance = Math::Geo::deg2km(distance);

			string stationID = net->code() + "." + sta->code();

			if ( distance > _maximumEpicentralDistance ) {
				_report << "   - " << stationID << " [distance out of range]" << endl;
				continue;
			}

			_report << "   + " << stationID << endl;
			_report << "     + distance = " << distance << "km" << endl;

			PickStreamEntry &e = pickStreamMap[stationID];
			Core::Time triggerTime;

			if ( e.first ) {
				triggerTime = e.first->time().value();
				_report << "     + trigger time = " << triggerTime.iso()
				        << " [pick: " << e.first->publicID() << "]" << endl;
			}
			else {
				try {
					TravelTime tt = _travelTime.computeFirst(_latitude, _longitude, _depth,
					                                         sta->latitude(), sta->longitude());
					triggerTime = _originTime + Core::TimeSpan(tt.time);
				}
				catch ( ... ) {
					_report << "     - PGAV [no travel time available]" << endl;
					continue;
				}

				_report << "     + trigger time = " << triggerTime.iso() << " [predicted arrival time]" << endl;
			}

			// Find velocity and strong-motion streams
			DataModel::WaveformStreamID tmp(net->code(), sta->code(), "", "", "");

			DataModel::Stream *maxVel, *maxAcc;
			maxVel = Private::findStreamMaxSR(sta, triggerTime,
			                                  WaveformProcessor::MeterPerSecond,
			                                  &_streamFirewall);
			maxAcc = Private::findStreamMaxSR(sta, triggerTime,
			                                  WaveformProcessor::MeterPerSecondSquared,
			                                  &_streamFirewall);

			/*
			if ( maxVel && _currentProcess->hasBeenProcessed(maxVel) ) {
				_report << "     - vel " << maxVel->sensorLocation()->code()
				        << "." << maxVel->code().substr(0,2) << " [processed already]" << endl;
				maxVel = NULL;
			}

			if ( maxAcc && _currentProcess->hasBeenProcessed(maxAcc) ) {
				_report << "     - acc " << maxAcc->sensorLocation()->code()
				        << "." << maxAcc->code().substr(0,2) << " [processed already]" << endl;
				maxAcc = NULL;
			}
			*/

			if ( !maxAcc && !maxVel ) {
				_report << "     - PGAV [no usable channel found]" << endl;
				continue;
			}

			// Add strong-motion data if available
			if ( maxAcc ) {
				tmp.setLocationCode(maxAcc->sensorLocation()->code());
				// Fixing the component would bind this channel to the processor
				// regardless of the orientation of the requested component.
				// This will be useful, if processors are created whenever a
				// new data channel arrives.
				//tmp.setChannelCode(maxAcc->code() + "E");
				tmp.setChannelCode(maxAcc->code().substr(0,2));

				DataModel::ThreeComponents tc;
				try {
					DataModel::getThreeComponents(
						tc, maxAcc->sensorLocation(),
						tmp.channelCode().c_str(), triggerTime
					);
				}
				catch ( exception &e ) {
					cout << Private::toStreamID(tmp) << ": " << e.what() << endl;
					_report << "     - acc " << maxAcc->sensorLocation()->code()
					        << "." << maxAcc->code().substr(0,2) << " [" << e.what() << "]" << endl;
				}

				for ( int i = 0; i < 3; ++i ) {
					if ( tc.comps[i] == NULL ) continue;
					tmp.setChannelCode(tc.comps[i]->code());
					if ( _currentProcess->hasBeenProcessed(tc.comps[i]) ) {
						_report << "     - acc " << tmp.locationCode()
						        << "." << tmp.channelCode() << " [processed already]" << endl;
					}
					else if ( _streamFirewall.isAllowed(toStreamID(tmp)) ) {
						_report << "     + acc " << tmp.networkCode() << "."
						        << tmp.stationCode() << "." << tmp.locationCode() << "."
						        << tmp.channelCode() << endl;
						if ( addProcessor(tmp, tc.comps[i], triggerTime,
						                  (WaveformProcessor::StreamComponent)i) == -3 )
							++_currentProcess->remainingChannels;
					}
				}
			}

			// Add velocity data if available
			if ( maxVel ) {
				tmp.setLocationCode(maxVel->sensorLocation()->code());
				tmp.setChannelCode(maxVel->code().substr(0,2));

				DataModel::ThreeComponents tc;
				try {
					DataModel::getThreeComponents(
						tc, maxVel->sensorLocation(),
						tmp.channelCode().c_str(), triggerTime
					);
				}
				catch ( exception &e ) {
					cout << Private::toStreamID(tmp) << ": " << e.what() << endl;
					_report << "     - vel " << maxAcc->sensorLocation()->code()
					        << "." << maxAcc->code().substr(0,2) << " [" << e.what() << "]" << endl;
				}

				for ( int i = 0; i < 3; ++i ) {
					if ( tc.comps[i] == NULL ) continue;
					tmp.setChannelCode(tc.comps[i]->code());
					if ( _currentProcess->hasBeenProcessed(tc.comps[i]) ) {
						_report << "     - vel " << tmp.locationCode()
						        << "." << tmp.channelCode() << " [processed already]" << endl;
					}
					else if ( _streamFirewall.isAllowed(toStreamID(tmp)) ) {
						_report << "     + vel " << tmp.networkCode() << "."
						        << tmp.stationCode() << "." << tmp.locationCode() << "."
						        << tmp.channelCode() << endl;
						if ( addProcessor(tmp, tc.comps[i], triggerTime,
						                  (WaveformProcessor::StreamComponent)i) == -3 )
							++_currentProcess->remainingChannels;
					}
				}
			}

			// Eventually all results are grouped by station and the
			// results from the sensor with the highest raw value is
			// used.
		}
	}

	//_currentProcess->results.clear();
	_acquisitionTimer.restart();

	if ( _processors.empty() ) {
		_report << " + No processors added" << endl;
		printReport();
		//return;
	}
	else {
		_report << " + Requested time windows" << endl;
		for ( RequestMap::iterator it = _stationRequests.begin(); it != _stationRequests.end(); ++it ) {
			StationRequest &req = it->second;
			for ( WaveformIDSet::iterator wit = req.streams.begin(); wit != req.streams.end(); ++wit ) {
				const WaveformStreamID &wsid = *wit;
				recordStream()->addStream(wsid.networkCode(), wsid.stationCode(),
				                          wsid.locationCode(), wsid.channelCode(),
				                          req.timeWindow.startTime(),
				                          req.timeWindow.endTime());
			}

			_report << "   + " << it->first << ": " << req.timeWindow.startTime().toString("%F %T")
			        << ", " << req.timeWindow.endTime().toString("%F %T") << endl;
		}

		_result << " + Processing" << endl;
	}

	_firstRecord = true;
	_noDataTimer.restart();
	_receivedRecords = false;
	_acquisitionTimeout = _config.initialAcquisitionTimeout;
	if ( _acquisitionTimeout > 0 )
		SEISCOMP_INFO("set stream timeout to %d seconds", _acquisitionTimeout);

	if ( _config.dumpRecords ) {
		if ( _currentProcess->event )
			_recordDumpOutput.open((_currentProcess->event->publicID() + ".recs").c_str());
		else
			_recordDumpOutput.open("dump.recs");
	}

	startRecordThread();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int WFParam::addProcessor(const DataModel::WaveformStreamID &waveformID,
                          DataModel::Stream *selectedStream,
                          const Core::Time &time,
                          WaveformProcessor::StreamComponent component) {
	WaveformProcessor::Component components[3];
	int componentCount = 0;

	PGAVPtr proc = new PGAV(time);
	proc->setEventWindow(_config.preEventWindowLength, _totalTimeWindowLength);
	proc->setSTALTAParameters(_config.STAlength, _config.LTAlength, _config.STALTAratio, _config.STALTAmargin);
	if ( _config.naturalPeriodsFixed )
		proc->setResponseSpectrumParameters(_config.dampings);
	else
		proc->setResponseSpectrumParameters(_config.dampings, _config.naturalPeriods, _config.Tmin, _config.Tmax, _config.naturalPeriodsLog);
	proc->setAftershockRemovalEnabled(_config.afterShockRemoval);
	proc->setPreEventCutOffEnabled(_config.eventCutOff);
	proc->setSaturationThreshold((_config.saturationThreshold * 0.01) * (1 << 23));
	proc->setNonCausalFiltering(_config.enableNonCausalFilters, _config.taperLength);
	proc->setPadLength(_config.padLength);
	// -1 as hifreq: let the algorithm define the best frequency
	proc->setPostDeconvolutionFilterParams(_config.PDorder, _config.PDfilter.first, _config.PDfilter.second);
	proc->setFilterParams(_config.order, _filter.first, _filter.second);
	proc->setDeconvolutionEnabled(_config.enableDeconvolution);
	proc->setDurationScale(_config.durationScale);
	proc->setClipTmaxToLowestFilterFrequency(_config.clipTmax);

	// Override used component
	proc->setUsedComponent(component);

	// Lookup station parameters of config module
	Util::KeyValues *params = NULL;
	string stationID = waveformID.networkCode() + "." +
	                   waveformID.stationCode();
	KeyMap::iterator it = _keys.find(stationID);
	if ( it != _keys.end() )
		params = it->second.get();
	else if ( configModule() != NULL ) {
		for ( size_t i = 0; i < configModule()->configStationCount(); ++i ) {
			ConfigStation *station = configModule()->configStation(i);

			if ( station->networkCode() != waveformID.networkCode() ) continue;
			if ( station->stationCode() != waveformID.stationCode() ) continue;

			Setup *setup = findSetup(station, name());
			if ( setup ) {
				ParameterSet* ps = ParameterSet::Find(setup->parameterSetID());

				if ( !ps ) {
					SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
					continue;
				}

				Util::KeyValuesPtr keys = new Util::KeyValues;
				keys->init(ps);
				_keys[stationID] = keys;
				params = keys.get();
			}
		}
	}

	switch ( proc->usedComponent() ) {
		case WaveformProcessor::Vertical:
			components[0] = WaveformProcessor::VerticalComponent;
			componentCount = 1;
			break;
		case WaveformProcessor::FirstHorizontal:
			components[0] = WaveformProcessor::FirstHorizontalComponent;
			componentCount = 1;
			break;
		case WaveformProcessor::SecondHorizontal:
			components[0] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 1;
			break;
		case WaveformProcessor::Horizontal:
			components[0] = WaveformProcessor::FirstHorizontalComponent;
			components[1] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 2;
			break;
		case WaveformProcessor::Any:
			components[0] = WaveformProcessor::VerticalComponent;
			components[1] = WaveformProcessor::FirstHorizontalComponent;
			components[2] = WaveformProcessor::SecondHorizontalComponent;
			componentCount = 3;
			break;
		default:
			_report << "       - PGAV [unsupported component " << proc->usedComponent() << "]"
			        << endl;
			return -1;
	}

	if ( waveformID.channelCode().size() == 3 && componentCount > 1 ) {
		SEISCOMP_ERROR("Processor needs %d components, but only one channel "
		               "passed", componentCount);
		return -1;
	}

	WaveformStreamID tmp = waveformID;
	string streamIDs[3];
	WaveformStreamID cwids[3];
	DataModel::ThreeComponents tc;

	// Use the given stream
	if ( selectedStream ) {
		cwids[0] = waveformID;
		streamIDs[0] = Private::toStreamID(cwids[0]);
		tc.comps[proc->usedComponent()] = selectedStream;
	}
	// Don't try to find a component, use exactly the given one
	else if ( waveformID.channelCode().size() == 3 ) {
		cwids[0] = waveformID;
		streamIDs[0] = Private::toStreamID(cwids[0]);

		Client::Inventory *inv = Client::Inventory::Instance();
		tc.comps[0] = inv->getStream(tmp.networkCode(), tmp.stationCode(),
		                             tmp.locationCode(), tmp.channelCode(),
		                             time);
	}
	else {
		// Cut component code
		tmp.setChannelCode(tmp.channelCode().substr(0,2));

		try {
			Client::Inventory *inv = Client::Inventory::Instance();
			tc = inv->getThreeComponents(tmp.networkCode(), tmp.stationCode(),
			                             tmp.locationCode(), tmp.channelCode(),
			                             time);
		}
		catch ( ... ) {}
	}

	for ( int i = 0; i < componentCount; ++i ) {
		cwids[i] = tmp;
		if ( tc.comps[components[i]] == NULL ) {
			_report << "       - PGAV [components not found]" << endl;
			return -1;
		}

		cwids[i].setChannelCode(tc.comps[components[i]]->code());
		streamIDs[i] = Private::toStreamID(cwids[i]);

		if ( cwids[i].channelCode().empty() ) {
			_report << "       - PGAV [invalid channel code]" << endl;
			return -1;
		}

		StreamMap::iterator it = _streams.find(streamIDs[i]);
		if ( it != _streams.end() )
			proc->streamConfig(components[i]) = *it->second;
		else {
			Processing::StreamPtr stream = new Processing::Stream;
			stream->init(cwids[i].networkCode(),
			             cwids[i].stationCode(),
			             cwids[i].locationCode(),
			             cwids[i].channelCode(),
			             time);
			_streams[streamIDs[i]] = stream;

			proc->streamConfig(components[i]) = *stream;
		}

		if ( proc->streamConfig(components[i]).gain == 0.0 ) {
			_report << "       - PGAV [gain not found for "
					<< proc->streamConfig(components[i]).code() << "]"
					<< endl;
			return -1;
		}
	}


	// If initialization fails, abort
	if ( !proc->setup(
		Settings(
			configModuleName(),
			tmp.networkCode(), tmp.stationCode(),
			tmp.locationCode(), tmp.channelCode(),
			&configuration(), params)) )
		return -1;

	SEISCOMP_DEBUG("setup processor on %s.%s.%s.%s",
	               tmp.networkCode().c_str(), tmp.stationCode().c_str(),
	               tmp.locationCode().c_str(), tmp.channelCode().c_str());

	proc->computeTimeWindow();

	// Check: end-time in future?
	if ( now.valid() && (proc->safetyTimeWindow().endTime() > now) ) {
		_report << "       - PGAV [end of time window in future]" << endl;
		return -3;
	}

	if ( proc->isFinished() ) {
		_report << "       - PGAV [" << proc->status().toString() << " (" << proc->statusValue() << ")]" << endl;
		return -1;
	}
	else {
		/*
		if ( proc->safetyTimeWindow().endTime() > Core::Time::GMT() + Core::TimeSpan(30.) ) {
			_report << "     - " << proc->type() << " [timewindow end is too far in the future]" << endl;
			return false;
		}
		*/
		_report << "       + PGAV" << endl;
	}

	for ( int i = 0; i < componentCount; ++i ) {
		pair<ProcessorMap::iterator, bool> handle =
			_processors.insert(ProcessorMap::value_type(streamIDs[i], ProcessorSlot()));

		// Update processors station time window
		StationRequest &req = _stationRequests[stationID];
		if ( (bool)req.timeWindow == true )
			req.timeWindow = req.timeWindow | proc->safetyTimeWindow();
		else
			req.timeWindow = proc->safetyTimeWindow();

		// The second value of the pair describes whether a new entry has been inserted or not
		if ( handle.second ) {
			req.streams.insert(cwids[i]);
			//addStream(cwids[i].networkCode(), cwids[i].stationCode(), cwids[i].locationCode(), cwids[i].channelCode());
			_report << "         + " << streamIDs[i] << endl;
		}

		handle.first->second.push_back(proc);
	}

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename KEY, typename VALUE>
bool WFParam::getValue(VALUE &res, const map<KEY,VALUE> &values,
                       double ref) const {
	typename map<KEY,VALUE>::const_iterator it;

	if ( values.empty() ) return false;

	it = values.begin();
	res = it->second;
	++it;

	for ( ; it != values.end(); ++it )
		if ( ref >= it->first ) res = it->second;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::printReport() {
	SEISCOMP_LOG(_processingInfoChannel, "%s%s", _report.str().c_str(),
	             _result.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string WFParam::generateEventID(const DataModel::Event *evt) {
	char buf[20];

	// EventOriginTime_Mag_Lat_Lon_CreationDate
	if ( evt == NULL ) return "";

	OriginPtr org = _cache.get<Origin>(evt->preferredOriginID());
	if ( org == NULL ) return "";

	if ( _config.enableShortEventID )
		return org->time().value().toString("%Y%m%d%H%M%S");

	MagnitudePtr mag = _cache.get<Magnitude>(evt->preferredMagnitudeID());

	if ( !mag && org->magnitudeCount() > 0 )
		mag = org->magnitude(0);

	string id;
	id = org->time().value().toString("%Y%m%d%H%M%S");

	id += "_";

	// Magnitude
	if ( mag )
		id += Core::toString(int(mag->magnitude().value()*10));

	id += "_";

	// Latitude
	sprintf(buf, "%05d", int(org->latitude().value()*1000));
	id += buf;

	id += "_";

	// Longitude
	sprintf(buf, "%06d", int(org->longitude().value()*1000));
	id += buf;

	id += "_";

	Core::Time t;
	try {
		t = org->creationInfo().creationTime();
	}
	catch ( ... ) {
		t = Core::Time::GMT();
	}

	id += t.toString("%Y%m%d%H%M%S");

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::feed(Seiscomp::DataModel::Pick *pick) {
	if ( isAgencyIDAllowed(objectAgencyID(pick)) )
		_cache.feed(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::removedFromCache(Seiscomp::DataModel::PublicObject *po) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::storeRecord(Record *rec) {
	if ( _firstRecord ) {
		if ( _config.runningAcquisitionTimeout > 0 ) {
			SEISCOMP_INFO("Data request: got first record, set timeout to %d seconds",
			              _config.runningAcquisitionTimeout);

			_acquisitionTimeout = _config.runningAcquisitionTimeout;
			_noDataTimer.restart();
		}

		_firstRecord = false;
	}

	return Application::storeRecord(rec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::handleRecord(Record *rec) {
	_receivedRecords = true;

	RecordPtr tmp(rec);

	if ( _config.dumpRecords ) {
		if ( rec->raw() ) {
			size_t bytes = rec->raw()->bytes()*rec->raw()->size();
			_recordDumpOutput.write((const char*)rec->raw()->data(), bytes);
		}
	}

	string streamID = rec->streamID();

	ProcessorMap::iterator slot_it = _processors.find(streamID);
	if ( slot_it == _processors.end() ) {
		/*
		// Add new processor?
		if ( !createProcessor(rec) ) return;

		slot_it = _processors.find(streamID);
		if ( slot_it == _processors.end() ) return;
		*/
		return;
	}

	for ( ProcessorSlot::iterator it = slot_it->second.begin(); it != slot_it->second.end(); ) {
		(*it)->feed(rec);
		if ( (*it)->status() == WaveformProcessor::InProgress ) {
			// processor still needs some time (progress = (*it)->statusValue())
			++it;
		}
		else if ( (*it)->status() == WaveformProcessor::Finished ) {
			_result << "   + PGAV, " << slot_it->first.c_str() << endl;
			PGAV *pgav = static_cast<PGAV*>(it->get());
			_currentProcess->results.resize(_currentProcess->results.size()+1);
			++_currentProcess->newValidResults;
			PGAVResult &res = _currentProcess->results.back();
			res.valid = true;
			res.processed = pgav->processed();
			res.streamID.setNetworkCode(rec->networkCode());
			res.streamID.setStationCode(rec->stationCode());
			res.streamID.setLocationCode(rec->locationCode());
			res.streamID.setChannelCode(rec->channelCode());

			setup(res, pgav);

			if ( _config.saveProcessedWaveforms )
				dumpWaveforms(_currentProcess.get(), res, pgav);

			if ( _config.saveSpectraFiles )
				dumpSpectra(_currentProcess.get(), res, pgav);

			// processor finished successfully
			it = slot_it->second.erase(it);
		}
		else if ( (*it)->isFinished() ) {
			_result << "   - PGAV, " << slot_it->first.c_str() << " ("
			        << (*it)->status().toString()
			        << ")" << endl;
			PGAV *pgav = static_cast<PGAV*>(it->get());
			_currentProcess->results.resize(_currentProcess->results.size()+1);
			PGAVResult &res = _currentProcess->results.back();
			res.valid = false;
			res.processed = pgav->processed();
			res.streamID.setNetworkCode(rec->networkCode());
			res.streamID.setStationCode(rec->stationCode());
			res.streamID.setLocationCode(rec->locationCode());
			res.streamID.setChannelCode(rec->channelCode());

			if ( res.processed ) {
				setup(res, pgav);

				if ( _config.saveProcessedWaveforms )
					dumpWaveforms(_currentProcess.get(), res, pgav);

				if ( _config.saveSpectraFiles )
					dumpSpectra(_currentProcess.get(), res, pgav);
			}

			it = slot_it->second.erase(it);
		}
		else
			++it;
	}

	if ( slot_it->second.empty() )
		_processors.erase(slot_it);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::createProcessor(Record *rec) {
	DataModel::WaveformStreamID tmp;
	tmp.setNetworkCode(rec->networkCode());
	tmp.setStationCode(rec->stationCode());
	tmp.setLocationCode(rec->locationCode());
	tmp.setChannelCode(rec->channelCode());

	Client::Inventory *inv = Client::Inventory::Instance();

	DataModel::Stream *stream =
		inv->getStream(tmp.networkCode(), tmp.stationCode(),
		               tmp.locationCode(), tmp.channelCode(), _originTime);

	if ( stream == NULL ) {
		_report << "   - " << rec->streamID() << " [no inventory information]" << endl;
		return false;
	}

	Core::Time triggerTime;

	double distance, az, baz;
	Math::Geo::delazi(_latitude, _longitude,
	                  stream->sensorLocation()->latitude(),
	                  stream->sensorLocation()->longitude(),
	                  &distance, &az, &baz);
	distance = Math::Geo::deg2km(distance);

	if ( distance > _maximumEpicentralDistance ) {
		_report << "   - " << rec->streamID() << " [distance out of range]" << endl;
		return false;
	}
	else {
		_report << "   + " << rec->streamID() << endl;
		_report << "     + distance = " << distance << "km" << endl;
	}

	try {
		TravelTime tt = _travelTime.computeFirst(_latitude, _longitude, _depth,
		                                         stream->sensorLocation()->latitude(),
		                                         stream->sensorLocation()->longitude());
		triggerTime = _originTime + Core::TimeSpan(tt.time);
		_report << "     + trigger time = " << triggerTime.iso() << " [predicted arrival time]" << endl;
	}
	catch ( ... ) {
		_report << "     - PGAV [no travel time available]" << endl;
		return false;
	}

	Processing::WaveformProcessor::SignalUnit unit;
	if ( !unit.fromString(stream->gainUnit().c_str()) ) {
		_report << "     - PGAV [unknown sensor unit:" << stream->gainUnit()
		        << "]" << endl;
		return false;
	}

	if ( unit == WaveformProcessor::MeterPerSecond )
		_report << "     + type = vel" << endl;
	else if ( unit == WaveformProcessor::MeterPerSecondSquared )
		_report << "     + type = acc" << endl;
	else
		_report << "     + type = other (" << unit.toString() << ")" << endl;

	/*
	_report << "     + acc " << tmp.networkCode() << "."
	        << tmp.stationCode() << "." << tmp.locationCode() << "."
	        << tmp.channelCode().substr(0,2) << endl;
	*/

	addProcessor(tmp, NULL, triggerTime);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WFParam::dispatchNotification(int type, Core::BaseObject *obj) {
	switch ( type ) {
		case AcquisitionFinished:
		{
			waitForRecordThread();
			closeStream();
			collectResults();

			if ( _currentProcess->remainingChannels == 0 ) {
				SEISCOMP_INFO("All available channels for event %s have been "
				              "processed, stop process",
				              _currentProcess->event->publicID().c_str());
				stopProcess(_currentProcess.get());

				if ( connection() ) {
					DataModel::Journaling journal;
					JournalEntryPtr entry = new JournalEntry;
					entry->setObjectID(_currentProcess->event->publicID());
					entry->setAction(JOURNAL_ACTION);
					entry->setParameters(JOURNAL_ACTION_COMPLETED);
					entry->setSender(name() + "@" + Core::getHostname());
					entry->setCreated(Core::Time::GMT());
					Notifier::Enable();
					Notifier::Create(journal.publicID(), OP_ADD, entry.get());
					Notifier::Disable();

					Core::MessagePtr msg = Notifier::GetMessage();
					if ( msg ) connection()->send("EVENT", msg.get());
				}
			}

			_acquisitionTimeout = 0;
			_currentProcess = NULL;
			handleTimeout();

			if ( (!_config.eventID.empty() && _crontab.empty()) ||
			     _config.offline )
				quit();
			break;
		}
		default:
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::acquisitionFinished() {
	if ( _config.dumpRecords ) _recordDumpOutput.close();
	sendNotification(Client::Notification(AcquisitionFinished, NULL));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::setup(PGAVResult &res, Processing::PGAV *pgav) {
	res.isVertical = pgav->usedComponent() == WaveformProcessor::Vertical;
	res.isVelocity = pgav->isVelocity();
	res.isAcausal = pgav->config().noncausal;
	res.maxRawAmplitude = pgav->maximumRawValue();
	res.pga = pgav->PGA();
	res.pgv = pgav->PGV();
	res.duration = pgav->duration();
	res.pdFilterOrder = pgav->config().PDorder;
	res.pdFilter.first = pgav->loPDFilterUsed();
	res.pdFilter.second = pgav->hiPDFilterUsed();
	res.filterOrder = pgav->config().filterOrder;
	res.filter.first = pgav->loFilterUsed();
	res.filter.second = pgav->hiFilterUsed();
	res.trigger = pgav->trigger();
	res.startTime = pgav->dataTimeWindow().startTime();
	res.endTime = pgav->dataTimeWindow().endTime();
	res.responseSpectra = pgav->responseSpectra();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::collectResults() {
	_report << " + Data request: finished" << endl;

	for ( ProcessorMap::iterator slot_it = _processors.begin();
	      slot_it != _processors.end(); ++slot_it ) {
		for ( ProcessorSlot::iterator it = slot_it->second.begin();
		      it != slot_it->second.end(); ++it ) {
			if ( _config.offline )
				static_cast<PGAV*>((it->get()))->finish();

			if ( (*it)->status() == WaveformProcessor::Finished ) {
				const Record *rec = (*it)->lastRecord();
				_result << "   + PGAV, " << slot_it->first.c_str() << endl;
				PGAV *pgav = static_cast<PGAV*>(it->get());
				_currentProcess->results.resize(_currentProcess->results.size()+1);
				++_currentProcess->newValidResults;
				PGAVResult &res = _currentProcess->results.back();
				res.valid = true;
				res.processed = pgav->processed();
				res.streamID.setNetworkCode(rec->networkCode());
				res.streamID.setStationCode(rec->stationCode());
				res.streamID.setLocationCode(rec->locationCode());
				res.streamID.setChannelCode(rec->channelCode());

				setup(res, pgav);

				if ( _config.saveProcessedWaveforms )
					dumpWaveforms(_currentProcess.get(), res, pgav);

				if ( _config.saveSpectraFiles )
					dumpSpectra(_currentProcess.get(), res, pgav);

				(*it)->close();
				continue;
			}
			else if ( (*it)->isFinished() ) {
				const Record *rec = (*it)->lastRecord();
				PGAV *pgav = static_cast<PGAV*>(it->get());
				_currentProcess->results.resize(_currentProcess->results.size()+1);
				PGAVResult &res = _currentProcess->results.back();
				res.valid = false;
				res.processed = pgav->processed();
				res.streamID.setNetworkCode(rec->networkCode());
				res.streamID.setStationCode(rec->stationCode());
				res.streamID.setLocationCode(rec->locationCode());
				res.streamID.setChannelCode(rec->channelCode());

				if ( res.processed ) {
					setup(res, pgav);

					if ( _config.saveProcessedWaveforms )
						dumpWaveforms(_currentProcess.get(), res, pgav);

					if ( _config.saveSpectraFiles )
						dumpSpectra(_currentProcess.get(), res, pgav);
				}

				(*it)->close();
			}
			else {
				// Processor did not receive enough data, lets try later
				++_currentProcess->remainingChannels;
			}

			_result << "   - PGAV, " << slot_it->first.c_str()
			        << " (" << (*it)->status().toString()
			        << ", " << (*it)->statusValue() << ")" << endl;

			_result << "     - TimeWindow: " << (*it)->safetyTimeWindow().startTime().toString("%F %T") << ", "
			        << (*it)->safetyTimeWindow().endTime().toString("%F %T") << endl;

			(*it)->close();
		}
	}

	_processors.clear();

	double seconds = (double)_acquisitionTimer.elapsed();
	SEISCOMP_INFO("Acquisition took %.2f seconds", seconds);

	printReport();

	_report.str(string());
	_result.str(string());

	StationMap stationMap;
	StationMap::iterator sit;

	// Check all processed but invalid results if there are channels on that
	// stream that are valid. If so, then set the invalid stream also to
	// valid
	for ( PGAVResults::iterator it = _currentProcess->results.begin();
	      it != _currentProcess->results.end(); ++it ) {
		if ( it->valid ) continue;
		if ( !it->processed ) continue;

		// The key is the station and the type (velocity)
		for ( PGAVResults::iterator it2 = _currentProcess->results.begin();
		      it2 != _currentProcess->results.end(); ++it2 ) {
			if ( !it2->valid ) continue;
			if ( it2->isVelocity != it->isVelocity ) continue;
			if ( it2->streamID.networkCode() != it->streamID.networkCode() ) continue;
			if ( it2->streamID.stationCode() != it->streamID.stationCode() ) continue;

			// Network matches, station matches, type matches, set valid to true
			it->valid = true;
			break;
		}
	}

	// Collect all results per station and select the correct data set
	// (velocity is always preferred over strong-motion)
	for ( PGAVResults::iterator it = _currentProcess->results.begin();
	      it != _currentProcess->results.end(); ++it ) {
		string stationID = Private::toStationID(it->streamID);

		// Skip invalid results
		if ( !it->valid ) {
			SEISCOMP_DEBUG("Skipping invalid result of %s", stationID.c_str());
			continue;
		}

		sit = stationMap.find(stationID);

		StationResults &res = stationMap[stationID];

		if ( !res.empty() ) {
			if ( res.back()->isVelocity ) {
				// Skip acceleration results if velocity has been stored already
				if ( !it->isVelocity ) continue;
			}
			else {
				// Remove all acceleration results if a velocity result exists
				if ( it->isVelocity ) res.clear();
			}
		}

		// Update psa03, psa10 and psa30 values
		it->psa03 = it->psa10 = it->psa30 = -1.0;

		const PGAV::ResponseSpectrum *spectrum = NULL;
		const PGAV::ResponseSpectra &spec = it->responseSpectra;
		PGAV::ResponseSpectra::const_iterator it2;
		for ( it2 = spec.begin(); it2 != spec.end(); ++it2 ) {
			if ( it2->first == 5 ) {
				spectrum = &it2->second;
				break;
			}
		}

		bool got03 = false, got10 = false, got30 = false;
		int missing = 3;

		// Find psa03, psa10 and psa30 values
		if ( spectrum ) {
			for ( size_t i = 0; i < spectrum->size(); ++i ) {
				if ( !got03 && (*spectrum)[i].period == 0.3 ) {
					it->psa03 = (*spectrum)[i].psa;
					got03 = true;
					--missing;
					if ( !missing ) break;
				}
				else if ( !got10 && (*spectrum)[i].period== 1.0 ) {
					it->psa10 = (*spectrum)[i].psa;
					got10 = true;
					--missing;
					if ( !missing ) break;
				}
				else if ( !got30 && (*spectrum)[i].period == 3.0 ) {
					it->psa30 = (*spectrum)[i].psa;
					got30 = true;
					--missing;
					if ( !missing ) break;
				}
			}
		}

		res.push_back(&(*it));
	}

	ostream *os;

	EventPtr evt;
	OriginPtr org;
	MagnitudePtr mag;
	bool newResultsAvailable;

	if ( _currentProcess )
		newResultsAvailable = _currentProcess->newValidResults > 0;
	else
		newResultsAvailable = true;

	if ( _currentProcess && _currentProcess->event ) {
		evt = _currentProcess->event;
		org = _cache.get<Origin>(evt->preferredOriginID());
		mag = _cache.get<Magnitude>(evt->preferredMagnitudeID());
	}

	if ( !newResultsAvailable )
		SEISCOMP_DEBUG("There aren't any new station results, skip further processing (messaging, shakemap, ...)");

	if ( _config.enableMessagingOutput && newResultsAvailable ) {
		if ( !sendMessages(connection(), evt.get(), org.get(),
		                   mag.get(), stationMap) )
			SEISCOMP_ERROR("Sending result messages failed");
	}

	if ( _config.enableShakeMapXMLOutput && newResultsAvailable ) {
		ofstream of;
		Core::Time timestamp = Core::Time::GMT();
		string eventPath, path;
		string eventID, shakeMapEventID, locstring;
		bool writeToFile = _config.shakeMapOutputPath != "-";

		eventID = generateEventID(evt.get());
		if ( eventID.empty() )
			eventID = timestamp.toString("%Y%m%d%H%M%S");

		if ( _config.shakeMapOutputSC3EventID && evt )
			shakeMapEventID = evt->publicID();
		else
			shakeMapEventID = eventID;

		if ( _config.shakeMapOutputRegionName ) {
			// Load event descriptions if not already there
			if ( query() && evt->eventDescriptionCount() == 0 )
				query()->loadEventDescriptions(evt.get());

			EventDescriptionPtr ed = evt->eventDescription(EventDescriptionIndex(REGION_NAME));
			if ( ed )
				locstring = toXML(ed->text());
		}
		else {
			locstring = evt->publicID() + " / " +
			            Core::toString(org->latitude().value()) +" / " +
			            Core::toString(org->longitude().value());
		}

		if ( writeToFile ) {
			eventPath = _config.shakeMapOutputPath + shakeMapEventID + "/";
			path = eventPath + "input";
			if ( !Util::pathExists(path) ) {
				if ( !Util::createPath(path) ) {
					SEISCOMP_ERROR("Unable to create shakeMap event path: %s",
					               path.c_str());
					return;
				}
			}
			path += "/";
		}

		if ( evt && org && mag ) {
			if ( !writeToFile )
				os = &cout;
			else {
				of.open((path + "event.xml").c_str());
				os = &of;
			}

			try {
				int year, mon, day, hour, min, sec;
				org->time().value().get(&year, &mon, &day, &hour, &min, &sec);
				*os << "<?xml version=\"1.0\" encoding=\"" << _config.shakeMapXMLEncoding << "\" standalone=\"yes\"?>" << endl;
				*os << "<!DOCTYPE earthquake SYSTEM \"earthquake.dtd\">" << endl;
				*os << "<earthquake id=\"" << shakeMapEventID << "\""
				    << " lat=\"" << org->latitude().value() << "\""
				    << " lon=\"" << org->longitude().value() << "\""
				    << " depth=\"" << org->depth().value() << "\""
				    << " mag=\"" << mag->magnitude().value() << "\""
				    << " year=\"" << year << "\" month=\"" << mon << "\" day=\"" << day << "\""
				    << " hour=\"" << hour << "\" minute=\"" << min << "\" second=\"" << sec << "\" timezone=\"GMT\""
				    << " locstring=\"" << locstring << "\""
				    << " created=\"" << Core::Time::GMT().seconds() << "\"/>"
				    << endl;
			}
			catch ( exception &e ) {
				SEISCOMP_ERROR("creating event.xml failed: %s", e.what());
			}

			if ( os == &of ) of.close();
		}

		if ( !writeToFile )
			os = &cout;
		else {
			of.open((path + "event_dat.xml").c_str());
			os = &of;
		}

		*os << "<?xml version=\"1.0\" encoding=\"" << _config.shakeMapXMLEncoding << "\" standalone=\"yes\"?>" << endl;
		*os << "<!DOCTYPE earthquake SYSTEM \"stationlist.dtd\">" << endl;
		*os << "<stationlist created=\"\" xmlns=\"ch.ethz.sed.shakemap.usgs.xml\">" << endl;

		for ( sit = stationMap.begin(); sit != stationMap.end(); ++sit ) {
			bool openStationTag = false;

			if ( _config.useMaximumOfHorizontals ) {
				bool foundHorizontals = false;
				PGAVResult res;

				StationResults::iterator rit;
				for ( rit = sit->second.begin(); rit != sit->second.end(); ++rit ) {
					if ( (*rit)->isVertical ) continue;

					if ( !foundHorizontals ) {
						foundHorizontals = true;
						res = **rit;
						continue;
					}

					if ( res.pga < (*rit)->pga ) res.pga = (*rit)->pga;
					if ( res.pgv < (*rit)->pgv ) res.pgv = (*rit)->pgv;
					if ( res.psa03 < (*rit)->psa03 ) res.psa03 = (*rit)->psa03;
					if ( res.psa10 < (*rit)->psa10 ) res.psa10 = (*rit)->psa10;
					if ( res.psa30 < (*rit)->psa30 ) res.psa30 = (*rit)->psa30;
				}

				if ( foundHorizontals )
					writeShakeMapComponent(&res, openStationTag, os, false);
			}
			else {
				StationResults::iterator rit;
				for ( rit = sit->second.begin(); rit != sit->second.end(); ++rit )
					writeShakeMapComponent(*rit, openStationTag, os, true);
			}

			if ( openStationTag )
				*os << "  </station>" << endl;
		}

		*os << "</stationlist>" << endl;

		if ( os == &of ) of.close();

		if ( !_config.shakeMapOutputScript.empty() && writeToFile ) {
			// Call script
			vector<string> params;
			params.push_back(_config.shakeMapOutputScript);
			params.push_back(_currentProcess && _currentProcess->event?
			                 _currentProcess->event->publicID():string("-"));
			params.push_back(eventID.empty()?string("-"):eventID);
			params.push_back(eventPath);
			pid_t pid = startExternalProcess(params);
			if ( pid < 0 )
				SEISCOMP_ERROR("%s: execution failed", _config.shakeMapOutputScript.c_str());
			else if ( _config.shakeMapOutputScriptWait ) {
				int status;
				waitpid(pid, &status, 0);
				SEISCOMP_DEBUG("%s: execution finished", _config.shakeMapOutputScript.c_str());
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::writeShakeMapComponent(const PGAVResult *res, bool &stationTag,
                                     std::ostream *os, bool withComponent) {
	Client::Inventory *inv = Client::Inventory::Instance();

	DataModel::Stream *stream = inv->getStream(
		res->streamID.networkCode(),
		res->streamID.stationCode(),
		res->streamID.locationCode(),
		res->streamID.channelCode(),
		_originTime
	);

	if ( !stream ) {
		SEISCOMP_WARNING("%s.%s.%s.%s: missing stream",
			res->streamID.networkCode().c_str(),
			res->streamID.stationCode().c_str(),
			res->streamID.locationCode().c_str(),
			res->streamID.channelCode().c_str()
		);
	}

	DataModel::Sensor *sensor = stream?DataModel::Sensor::Find(stream->sensor()):NULL;
	if ( !sensor ) {
		SEISCOMP_WARNING("%s.%s.%s.%s: sensor not found or not defined",
			res->streamID.networkCode().c_str(),
			res->streamID.stationCode().c_str(),
			res->streamID.locationCode().c_str(),
			res->streamID.channelCode().c_str()
		);
	}

	DataModel::SensorLocation *loc = stream?stream->sensorLocation():NULL;

	if ( !loc ) {
		SEISCOMP_WARNING("%s.%s.%s: missing sensor location",
			res->streamID.networkCode().c_str(),
			res->streamID.stationCode().c_str(),
			res->streamID.locationCode().c_str()
		);

		return;
	}

	if ( !stationTag ) {
		*os << "  <station code=\"" << res->streamID.stationCode() << "\""
			<< " name=\"" << res->streamID.stationCode() << "\""
			<< " netid=\"" << res->streamID.networkCode() << "\"";
		if ( loc && !loc->station()->network()->archive().empty() )
			*os << " source=\"" << loc->station()->network()->archive() << "\"";
		if ( sensor )
			*os << " insttype=\"" << sensor->model() << "\"";
		*os << " lat=\"" << loc->latitude() << "\""
			<< " lon=\"" << loc->longitude() << "\""
			<< ">" << endl;
		stationTag = true;
	}

	*os << "    <comp name=\"";
	if ( loc && !loc->code().empty() )
		*os << loc->code();
	else
		*os << "--";
	*os << ".";

	if ( withComponent )
		*os << res->streamID.channelCode();
	else
		*os << res->streamID.channelCode().substr(0,2);
	*os << "\">" << endl;

	ios::fmtflags tmpf(os->flags());
	streamsize tmpp(os->precision());

	os->precision(10);
	os->setf(ios::fixed,ios::floatfield);
	*os << "      <acc value=\"" << (res->pga/9.806)*100.0 << "\" flag=\"0\"/>" << endl;
	*os << "      <vel value=\"" << res->pgv*100.0 << "\" flag=\"0\"/>" << endl;

	if ( res->psa03 >= 0.0 )
		*os << "      <psa03 value=\"" << (res->psa03/9.806)*100.0 << "\" flag=\"0\"/>" << endl;
	if ( res->psa10 >= 0.0 )
		*os << "      <psa10 value=\"" << (res->psa10/9.806)*100.0 << "\" flag=\"0\"/>" << endl;
	if ( res->psa30 >= 0.0 )
		*os << "      <psa30 value=\"" << (res->psa30/9.806)*100.0 << "\" flag=\"0\"/>" << endl;

	os->flags(tmpf);
	os->precision(tmpp);
	*os << "    </comp>" << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::dumpWaveforms(Process *p, PGAVResult &result,
                            const Processing::PGAV *proc) {
	DataModel::Event *event = p->event.get();

	if ( event == NULL ) return;

	string filename = _config.waveformOutputPath;

	if ( _config.waveformOutputEventDirectory ) {
		filename += generateEventID(event);

		if ( !Util::pathExists(filename) ) {
			if ( !Util::createPath(filename) ) {
				SEISCOMP_ERROR("Unable to create waveform event directory: %s",
				               filename.c_str());
				return;
			}
		}

		filename += "/";
	}

	filename += p->referenceTime.toString("%Y%m%d%H%M%S") + "_";
	filename += result.streamID.networkCode() + "_";
	filename += result.streamID.stationCode() + "_";
	filename += result.streamID.locationCode() + result.streamID.channelCode();

	const PGAV::Config &cfg = proc->config();
	if ( result.filter.first > 0 && result.filter.second > 0 ) {
		filename += "_BP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.first);
		filename += "_";
		filename += Core::toString(result.filter.second);
	}
	else if ( result.filter.first > 0 ) {
		filename += "_HP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.first);
	}
	else if ( result.filter.second > 0 ) {
		filename += "_LP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.second);
	}

	filename += ".mseed";

	ofstream of(filename.c_str(), ios_base::binary | ios_base::out);
	if ( !of.is_open() ) {
		SEISCOMP_ERROR("Unable to create waveform file: %s",
		               filename.c_str());
		return;
	}

	proc->continuousData();
	proc->dataTimeWindow().startTime();

	GenericRecord rec(
		result.streamID.networkCode(),
		result.streamID.stationCode(),
		result.streamID.locationCode(),
		result.streamID.channelCode(),
		proc->dataTimeWindow().startTime(),
		proc->samplingFrequency(), -1,
		Array::FLOAT
	);

	// Convert data to float
	ArrayPtr data = proc->continuousData().copy(Array::FLOAT);

	rec.setData(data.get());

	// Convert to MiniSEED
	IO::MSeedRecord mseed(rec);
	mseed.setOutputRecordLength(4096);
	mseed.write(of);

	result.filename = filename;

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WFParam::dumpSpectra(Process *p, const PGAVResult &result,
                          const Processing::PGAV *proc) {
	DataModel::Event *event = p->event.get();

	if ( event == NULL ) return;

	string filename = _config.spectraOutputPath;

	if ( _config.spectraOutputEventDirectory ) {
		filename += generateEventID(event);

		if ( !Util::pathExists(filename) ) {
			if ( !Util::createPath(filename) ) {
				SEISCOMP_ERROR("Unable to create waveform event directory: %s",
				               filename.c_str());
				return;
			}
		}

		filename += "/";
	}

	filename += p->referenceTime.toString("%Y%m%d%H%M%S") + "_";
	filename += result.streamID.networkCode() + "_";
	filename += result.streamID.stationCode() + "_";
	filename += result.streamID.locationCode() + result.streamID.channelCode();

	const PGAV::Config &cfg = proc->config();
	if ( result.filter.first > 0 && result.filter.second > 0 ) {
		filename += "_BP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.first);
		filename += "_";
		filename += Core::toString(result.filter.second);
	}
	else if ( result.filter.first > 0 ) {
		filename += "_HP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.first);
	}
	else if ( result.filter.second > 0 ) {
		filename += "_LP";
		filename += Core::toString(cfg.filterOrder);
		filename += "_";
		filename += Core::toString(result.filter.second);
	}

	filename += ".";

	PGAV::ResponseSpectra::const_iterator it;
	for ( it = result.responseSpectra.begin(); it != result.responseSpectra.end(); ++it ) {
		SEISCOMP_DEBUG(">  saving spectrum for damping %f", it->first);

		{ // PSA output
			string fn = filename + "psa" + Core::toString(it->first);
			ofstream of(fn.c_str(), ios_base::out);
			SEISCOMP_DEBUG(">   %s", fn.c_str());
			for ( size_t i = 0; i < it->second.size(); ++i )
				of << it->second[i].period << "\t" << it->second[i].psa << endl;
		}

		{ // DRS output
			string fn = filename + "drs" + Core::toString(it->first);
			ofstream of(fn.c_str(), ios_base::out);
			SEISCOMP_DEBUG(">   %s", fn.c_str());
			for ( size_t i = 0; i < it->second.size(); ++i )
				of << it->second[i].period << "\t" << it->second[i].sd << endl;
		}
	}

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
