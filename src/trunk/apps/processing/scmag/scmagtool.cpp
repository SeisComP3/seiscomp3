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


#include "component.h"
#include "magtool.h"
#include "dmutil.h"

#include <seiscomp3/core/strings.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/logging/log.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Magnitudes;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;


class MagToolApp : public Seiscomp::Client::Application {
	public:
		MagToolApp(int argc, char **argv)
		: Application(argc, argv), _expiry(3*3600.) {
			_fExpiry = 1.0;
			_interval = 1;

			_magTypes.insert("MLv");
			_magTypes.insert("mb");
			_magTypes.insert("mB");
			_magTypes.insert("Mwp");

			setAutoApplyNotifierEnabled(true);
			setInterpretNotifierEnabled(true);

			setLoadStationsEnabled(true);
			setLoadConfigModuleEnabled(true);

			setPrimaryMessagingGroup("MAGNITUDE");
			addMessagingSubscription("LOCATION");
			addMessagingSubscription("PICK");
			addMessagingSubscription("AMPLITUDE");
		}

		void createCommandLineDescription() {
			commandline().addOption("Messaging", "test", "Test mode, no messages are sent");
			commandline().addOption("Messaging", "interval", "Sets the message send interval,"
			                                                 " 0 sends messages immediatly", &_interval, true);
			commandline().addOption("Generic", "expiry,x", "Time span in hours after which objects expire", &_fExpiry, true);

			commandline().addGroup("Input");
			commandline().addOption("Input", "ep", "Event parameters XML file for offline processing of all contained origins. This option implies the computation of all station and network magnitudes not having an evaluation status set.",
			                        &_epFile);
			commandline().addOption("Input", "reprocess", "Reprocess also station and network magnitudes with an evaluation status. Only used with --ep.");

			commandline().addGroup("Reprocess");
			commandline().addOption("Reprocess", "static", "Do not create new station or new network magnitudes just update them. Considers the associated amplitudes. Weights of station magnitudes will be changed according to the accumulation function of the network magnitude.");
		}

		bool validateParameters() {
			if ( !isInventoryDatabaseEnabled() && !isConfigDatabaseEnabled() )
				setDatabaseEnabled(false, false);

			if ( !_epFile.empty() ) {
				setMessagingEnabled(false);
				setLoggingToStdErr(true);
			}

			return Client::Application::validateParameters();
		}

		bool getAverageMethod(MagTool::AverageDescription &desc, std::string &value, const char *type) {
			Core::trim(value);
			std::string param;
			size_t pos = value.find('(');
			if ( pos != std::string::npos ) {
				param = value.substr(pos+1);
				value.erase(pos);
				pos = param.find(')');
				if ( pos == std::string::npos ) {
					SEISCOMP_ERROR("magnitudes.average(%s): syntax error: expected closing bracket at end", type);
					return false;
				}

				if ( pos < param.size()-1 ) {
					SEISCOMP_ERROR("magnitudes.average(%s): syntax error: unexpected input after brackets", type);
					return false;
				}

				param.erase(pos);
			}

			if ( value == "default" ) {
				if ( !param.empty() ) {
					SEISCOMP_ERROR("magnitudes.average(%s): method 'default' does not take a parameter", type);
					return false;
				}

				desc.type = MagTool::Default;
			}
			else if ( value == "mean" ) {
				if ( !param.empty() ) {
					SEISCOMP_ERROR("magnitudes.average(%s): method 'mean' does not take a parameter", type);
					return false;
				}

				desc.type = MagTool::Mean;
			}
			else if ( value == "trimmedMean" ) {
				double percent;
				if ( !Core::fromString(percent, param) ) {
					SEISCOMP_ERROR("magnitudes.average(%s): 'trimmedMean' parameter is not a number", type);
					return false;
				}

				if ( percent < 0 || percent > 100 ) {
					SEISCOMP_ERROR("magnitudes.average(%s): 'trimmedMean' parameter with %.2f is out of bounds", type, percent);
					return false;
				}

				desc.type = MagTool::TrimmedMean;
				desc.parameter = percent;
			}
			else if ( value == "median" ) {
				if ( !param.empty() ) {
					SEISCOMP_ERROR("magnitudes.average(%s): method 'median' does not take a parameter", type);
					return false;
				}

				desc.type = MagTool::Median;
			}
			else if ( value == "trimmedMedian" ) {
				double percent;
				if ( !Core::fromString(percent, param) ) {
					SEISCOMP_ERROR("magnitudes.average(%s): 'trimmedMedian' parameter is not a number", type);
					return false;
				}

				if ( percent < 0 || percent > 100 ) {
					SEISCOMP_ERROR("magnitudes.average(%s): 'trimmedMedian' parameter with %.2f is out of bounds", type, percent);
					return false;
				}

				desc.type = MagTool::TrimmedMedian;
				desc.parameter = percent;
			}
			else {
				SEISCOMP_ERROR("magnitudes.average(%s): unknown average method '%s'", type, value.c_str());
				return false;
			}

			return true;
		}

		bool initConfiguration() {
			if ( !Application::initConfiguration() )
				return false;

			try { _interval = configGetInt("connection.sendInterval"); } catch ( ... ) {}
			try {
				std::vector<std::string> magTypes = configGetStrings("magnitudes");
				_magTypes.clear();
				_magTypes.insert(magTypes.begin(), magTypes.end());
			}
			catch ( ... ) {}

			try {
				_magtool.setSummaryMagnitudeEnabled(configGetBool("summaryMagnitude.enabled"));
			} catch ( ... ) {}

			try {
				_magtool.setSummaryMagnitudeType(configGetString("summaryMagnitude.type"));
			} catch ( ... ) {}

			try {
				_magtool.setSummaryMagnitudeMinStationCount(configGetInt("summaryMagnitude.minStationCount"));
			} catch ( ... ) {}

			try {
				_magtool.setSummaryMagnitudeBlacklist(configGetStrings("summaryMagnitude.blacklist"));
			} catch ( ... ) {}

			try {
				_magtool.setSummaryMagnitudeWhitelist(configGetStrings("summaryMagnitude.whitelist"));
			} catch ( ... ) {}

			try {
				_magtool.setMinimumArrivalWeight(configGetDouble("minimumArrivalWeight"));
			} catch ( ... ) {}


			try {
				std::vector<std::string> averages = configGetStrings("magnitudes.average");

				MagTool::AverageMethods averageMethods;
				MagTool::AverageDescription defaultAverageMethod;

				defaultAverageMethod.type = MagTool::Default;

				int defaultCount = 0;

				for ( size_t i = 0; i < averages.size(); ++i ) {
					std::string type = averages[i];
					// Found global average
					if ( type.find(':') == std::string::npos ) {
						++defaultCount;

						// Unable to parse the default?
						if ( !getAverageMethod(defaultAverageMethod, type, "default") )
							return false;
					}
				}

				if ( defaultCount > 1 ) {
					SEISCOMP_ERROR("magnitudes.average: multiple default values used: only one or none allowed");
					return false;
				}

				// Prefill the typed averages with default
				for ( MagTool::MagnitudeTypes::iterator it = _magTypes.begin(); it != _magTypes.end(); ++it )
					averageMethods[*it] = defaultAverageMethod;

				for ( size_t i = 0; i < averages.size(); ++i ) {
					std::string type = averages[i];
					size_t pos = type.find(':');
					// Found global average
					if ( pos == std::string::npos ) continue;

					std::string averageStr = type.substr(pos+1);
					type.erase(pos);
					Core::trim(type);

					// Ignore unconfigured magnitude types
					if ( _magTypes.find(type) == _magTypes.end() ) continue;

					MagTool::AverageDescription magTypeAverage;
					if ( !getAverageMethod(magTypeAverage, averageStr, type.c_str()) )
						return false;

					averageMethods[type] = magTypeAverage;
				}


				_magtool.setAverageMethods(averageMethods);
			}
			catch ( ... ) {}


			MagTool::SummaryMagnitudeCoefficients defaultCoefficients;
			std::map<std::string, MagTool::SummaryMagnitudeCoefficients> coefficients;

			std::vector<std::string> coeff[2];

			try {
				coeff[0] = configGetStrings("summaryMagnitude.coefficients.a");
			} catch ( ... ) {}

			try {
				coeff[1] = configGetStrings("summaryMagnitude.coefficients.b");
			} catch ( ... ) {}

			for ( int c = 0; c < 2; ++c ) {
				for ( size_t i = 0; i < coeff[c].size(); ++i ) {
					std::vector<std::string> toks;
					split(toks, coeff[c][i].c_str(), ":");

					if ( toks.size() == 2 ) {
						double v;
						if ( !fromString(v, toks[1]) ) {
							SEISCOMP_ERROR("summaryMagnitude.coefficients.a invalid double value: '%s' at '%s' -> using default values",
							               toks[1].c_str(), coeff[c][i].c_str());
							return true;
						}

						trim(toks[0]);

						if ( c == 0 )
							coefficients[toks[0]].a = v;
						else
							coefficients[toks[0]].b = v;
					}
					else if ( toks.size() == 1 ) {
						double v;
						if ( !fromString(v, toks[0]) ) {
							SEISCOMP_ERROR("summaryMagnitude.coefficients.a invalid double value: '%s' at '%s' -> using default values",
							               toks[0].c_str(), coeff[c][i].c_str());
							return true;
						}

						if ( c == 0 )
							defaultCoefficients.a = v;
						else
							defaultCoefficients.b = v;
					}
					else {
						SEISCOMP_ERROR("summaryMagnitude.coefficients.a syntax error: '%s' -> using default values",
						               coeff[c][i].c_str());

						return true;
					}
				}
			}

			if ( defaultCoefficients.a || defaultCoefficients.b )
				_magtool.setSummaryMagnitudeDefaultCoefficients(defaultCoefficients);

			if ( !coefficients.empty() )
				_magtool.setSummaryMagnitudeCoefficients(coefficients);

			return true;
		}

		bool init() {
			if ( !Application::init() )
				return false;

			_magtool.inputPickLog = addInputObjectLog("pick");
			_magtool.inputAmpLog = addInputObjectLog("amplitude");
			_magtool.inputOrgLog = addInputObjectLog("origin");
			_magtool.outputMagLog = addOutputObjectLog("magnitude", primaryMessagingGroup());

			_expiry = _fExpiry * 3600.;

			_magtool.init(_magTypes, _expiry,
			              commandline().hasOption("reprocess"),
			              commandline().hasOption("static"));

			if ( _interval > 0 )
				enableTimer(_interval);

			return true;
		}

		bool run() {
			if ( !_epFile.empty() ) {
				_sendImmediately = true;

				// Disable database
				setDatabase(NULL);

				XMLArchive ar;
				if ( !ar.open(_epFile.c_str()) ) {
					SEISCOMP_ERROR("Failed to open %s", _epFile.c_str());
					return false;
				}

				EventParametersPtr ep;
				ar >> ep;
				ar.close();

				if ( !ep ) {
					SEISCOMP_ERROR("No event parameters found in %s", _epFile.c_str());
					return false;
				}

				for ( size_t i = 0; i < ep->pickCount(); ++i )
					_magtool.feed(ep->pick(i));

				for ( size_t i = 0; i < ep->amplitudeCount(); ++i )
					_magtool.feed(ep->amplitude(i), false);

				for ( size_t i = 0; i < ep->originCount(); ++i ) {
					OriginPtr org = ep->origin(i);
					SEISCOMP_INFO("Processing origin %s", org->publicID().c_str());
					_magtool.feed(org.get());
				}

				ar.create("-");
				ar.setFormattedOutput(true);
				ar << ep;
				ar.close();

				return true;
			}

			return Seiscomp::Client::Application::run();
		}

		void done() {
			Application::done();
			_magtool.done();
		}

		void handleTimeout() {
			// Send out all available notifiers
			NotifierMessagePtr xmsg = Notifier::GetMessage();
			if (xmsg) {
				if ( !commandline().hasOption("test") )
					connection()->send(xmsg.get());
			}
		}

		void handleMessage(Core::Message* msg) {
			_sendImmediately = false;

			// Call the original method to make sure that the
			// interpret callbacks (addObject, updateObject -> see below)
			// will be called
			Application::handleMessage(msg);

			// All message handling is done so lets continue
			if ( !_interval || _sendImmediately ) {
				NotifierMessagePtr xmsg = Notifier::GetMessage();
				if (xmsg) {
					if ( !commandline().hasOption("test") )
						connection()->send(xmsg.get());
				}
			}

			_sendImmediately = false;
		}

		void addObject(const std::string& parentID, DataModel::Object* object) {
			Pick *pick = Pick::Cast(object);
			if ( pick != NULL ) {
				logObject(_magtool.inputPickLog, Time::GMT());
				_magtool.feed(pick);
				return;
			}

			Amplitude *ampl = Amplitude::Cast(object);
			if ( ampl != NULL ) {
				logObject(_magtool.inputAmpLog, Time::GMT());
				Notifier::Enable();
				_magtool.feed(ampl, false);
				Notifier::Disable();
				return;
			}

			Origin *origin = Origin::Cast(object);
			if ( origin != NULL ) {
				logObject(_magtool.inputOrgLog, Time::GMT());
				// When an origins arrives the initial magnitudes
				// have to be sent out immediately
				_sendImmediately = true;
				Notifier::Enable();
				_magtool.feed(origin);
				Notifier::Disable();
				return;
			}
		}

		void updateObject(const std::string&, DataModel::Object* object) {
			Amplitude *ampl = Amplitude::Cast(object);
			if ( ampl != NULL ) {
				logObject(_magtool.inputAmpLog, Time::GMT());
				Notifier::Enable();
				_magtool.feed(ampl, true);
				Notifier::Disable();
				return;
			}

			Origin *origin = Origin::Cast(object);
			if ( origin != NULL ) {
				logObject(_magtool.inputOrgLog, Time::GMT());
				// When an origins arrives the initial magnitudes
				// have to be sent out immediately
				_sendImmediately = true;
				Notifier::Enable();
				_magtool.feed(origin);
				Notifier::Disable();
				return;
			}
		}


	private:
		bool _sendImmediately;

		unsigned int _interval;
		double _fExpiry;
		TimeSpan _expiry;

		MagTool::MagnitudeTypes _magTypes;
		MagTool _magtool;

		std::string _epFile;
};


int main(int argc, char *argv[]) {
	int retCode = EXIT_SUCCESS;

	// Create an own block to make sure the application object
	// is destroyed when printing the overall objectcount
	{
		MagToolApp app(argc, argv);
		retCode = app.exec();
	}

	SEISCOMP_DEBUG("EXIT(%d), remaining objects: %d", retCode, BaseObject::ObjectCount());

	return retCode;
}
