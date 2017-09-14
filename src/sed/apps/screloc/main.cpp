/***************************************************************************
* Copyright (C) 2010 by Jan Becker, gempa GmbH                             *
* EMail: jabe@gempa.de                                                     *
*                                                                          *
* This code has been developed for the SED/ETH Zurich and is               *
* released under the SeisComP Public License.                              *
***************************************************************************/

#define SEISCOMP_COMPONENT screloc

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/seismology/locatorinterface.h>
#include <seiscomp3/io/archive/xmlarchive.h>


#include <iostream>
#include <iomanip>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Seismology;


class Reloc : public Client::Application {
	public:
		Reloc(int argc, char **argv) : Client::Application(argc, argv) {
			setMessagingEnabled(true);
			setDatabaseEnabled(true, true);
			setLoadStationsEnabled(true);
			setPrimaryMessagingGroup("LOCATION");

			// Listen for picks (caching) and origins
			addMessagingSubscription("PICK");
			addMessagingSubscription("LOCATION");

			// set up 1 hour of pick caching
			// otherwise the picks will be read from the database
			_cache.setTimeSpan(Core::TimeSpan(60*60));

			_useWeight = false;
			_originEvaluationMode = "AUTOMATIC";
		}


	protected:
		void createCommandLineDescription() {
			commandline().addGroup("Mode");
			commandline().addOption("Mode", "test", "test mode, do not send any message");
			commandline().addOption("Mode", "dump", "dump processed origins as XML to stdout");
			commandline().addGroup("Input");
			commandline().addOption("Input", "origin-id,O", "reprocess the origin and send a message", &_originIDs);
			commandline().addOption("Input", "locator", "the locator type to use", &_locatorType, false);
			commandline().addOption("Input", "profile", "the locator profile to use", &_locatorProfile, false);
			commandline().addOption("Input", "use-weight", "use current picks weight", &_useWeight, true);
			commandline().addOption("Input", "evaluation-mode", "set origin evaluation mode", &_originEvaluationMode, true);
			commandline().addOption("Input", "ep", "Event parameters XML file for offline processing of all contained origins. "
			                                       "This option should not be mixed with --dump.", &_epFile);
			commandline().addOption("Input", "replace", "Used in combination with --ep and defines if origins are to be replaced "
			                                            "by their relocated counterparts or just added to the output.");
		}


		bool initConfiguration() {
			// first call the application's configuration routine
			if ( !Client::Application::initConfiguration() )
				return false;

			_ignoreRejected = false;
			_allowPreliminary = false;

			try { _locatorType = configGetString("reloc.locator"); }
			catch ( ... ) {}

			try { _locatorProfile = configGetString("reloc.profile"); }
			catch ( ... ) {}

			try { _ignoreRejected = configGetBool("reloc.ignoreRejectedOrigins"); }
			catch ( ... ) {}

			try { _allowPreliminary = configGetBool("reloc.allowPreliminaryOrigins"); }
			catch ( ... ) {}

			try { _useWeight = configGetBool("reloc.useWeight"); }
			catch ( ... ) {}

			if ( !_epFile.empty() )
				setMessagingEnabled(false);

			if ( !isInventoryDatabaseEnabled() )
				setDatabaseEnabled(false, false);

			return true;
		}


		bool init() {
			if ( !Client::Application::init() )
				return false;

			_locator = LocatorInterfaceFactory::Create(_locatorType.c_str());
			if ( !_locator ) {
				SEISCOMP_ERROR("Locator %s not available -> abort", _locatorType.c_str());
				return false;
			}

			_inputOrgs = addInputObjectLog("origin");
			_outputOrgs = addOutputObjectLog("origin", primaryMessagingGroup());

			_cache.setDatabaseArchive(query());
			_locator->init(configuration());

			if ( !_locatorProfile.empty() )
				_locator->setProfile(_locatorProfile);

			if ( _originEvaluationMode != "AUTOMATIC" && _originEvaluationMode != "MANUAL") {
				SEISCOMP_ERROR("evaluation-mode must be (AUTOMATIC|MANUAL)");
				return false;
			}

			return true;
		}


		bool run() {
			if ( !_originIDs.empty() ) {
				for ( size_t i = 0; i < _originIDs.size(); ++i ) {
					OriginPtr org = Origin::Cast(query()->getObject(Origin::TypeInfo(), _originIDs[i]));
					if ( !org ) {
						cerr << "ERROR: Origin with id '" << _originIDs[i] << "' has not been found" << endl;
						continue;
					}

					OriginPtr newOrg;
					try {
						newOrg = process(org.get());
						if ( !newOrg ) {
							std::cerr << "ERROR: processing failed" << std::endl;
							continue;
						}
					}
					catch ( std::exception &e ) {
						std::cerr << "ERROR: " << e.what() << std::endl;
						continue;
					}

					// Log warning messages
					string msgWarning = _locator->lastMessage(LocatorInterface::Warning);
					if ( !msgWarning.empty() )
						std::cerr << "WARNING: " << msgWarning << std::endl;

					bool sendOrigin = true;
					if ( _ignoreRejected ) {
						try {
							if ( newOrg->evaluationStatus() == REJECTED ) {
								std::cerr << "INFO: Origin status is REJECTED, skip sending" << std::endl;
								sendOrigin = false;
							}
						}
						catch ( ... ) {}
					}

					if ( sendOrigin && !send(newOrg.get()) ) {
						std::cerr << "ERROR: sending of processed origin failed" << std::endl;
						continue;
					}

					std::cerr << "INFO: new Origin created OriginID=" << newOrg.get()->publicID().c_str() << std::endl;
				}

				return true;
			}
			else if ( !_epFile.empty() ) {
				// Disable database
				setDatabase(NULL);

				IO::XMLArchive ar;
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

				int numberOfOrigins = (int)ep->originCount();
				bool replace = commandline().hasOption("replace");

				for ( int i = 0; i < numberOfOrigins; ++i ) {
					OriginPtr org = ep->origin(i);
					SEISCOMP_INFO("Processing origin %s", org->publicID().c_str());
					org = process(org.get());
					if ( org ) {
						if ( replace ) {
							ep->removeOrigin(i);
							--i;
							--numberOfOrigins;
						}

						ep->add(org.get());
					}
				}

				ar.create("-");
				ar.setFormattedOutput(true);
				ar << ep;
				ar.close();

				return true;
			}

			return Client::Application::run();
		}


	protected:
		void handleMessage(Core::Message* msg) {
			Application::handleMessage(msg);

			DataMessage *dm = DataMessage::Cast(msg);
			if ( dm == NULL ) return;

			for ( DataMessage::iterator it = dm->begin(); it != dm->end(); ++it ) {
				Origin *org = Origin::Cast(it->get());
				if ( org )
					addObject("", org);
			}
		}

		void addObject(const std::string &parentID, Object *obj) {
			Pick *pick = Pick::Cast(obj);
			if ( pick ) {
				_cache.feed(pick);
				return;
			}

			Origin *org = Origin::Cast(obj);
			if ( org ) {
				logObject(_inputOrgs, Core::Time::GMT());

				if ( isAgencyIDBlocked(objectAgencyID(org)) ) {
					SEISCOMP_DEBUG("%s: skipping: agencyID '%s' is blocked",
					               org->publicID().c_str(), objectAgencyID(org).c_str());
					return;
				}

				try {
					// ignore non automatic origins
					if ( org->evaluationMode() != AUTOMATIC ) {
						SEISCOMP_DEBUG("%s: skipping: mode is not 'automatic'", org->publicID().c_str());
						return;
					}
				}
				// origins without an evaluation mode are treated as
				// automatic origins
				catch ( ... ) {}

				// Skip confirmed or otherwise tagged solutions unless
				// preliminary origins are allowed
				try {
					EvaluationStatus stat = org->evaluationStatus();
					if ( stat != PRELIMINARY || !_allowPreliminary ) {
						SEISCOMP_DEBUG("%s: skipping due to valid evaluation status", org->publicID().c_str());
						return;
					}
				}
				catch ( ... ) {}

				OriginPtr newOrg;

				try {
					newOrg = process(org);
					if ( !newOrg ) {
						SEISCOMP_ERROR("processing of origin '%s' failed", org->publicID().c_str());
						return;
					}
				}
				catch ( std::exception &e ) {
					SEISCOMP_ERROR("%s: %s", org->publicID().c_str(), e.what());
					return;
				}

				string msgWarning = _locator->lastMessage(LocatorInterface::Warning);
				if ( !msgWarning.empty() )
					SEISCOMP_WARNING("%s: %s", org->publicID().c_str(), msgWarning.c_str());

				bool sendOrigin = true;
				if ( _ignoreRejected ) {
					try {
						if ( newOrg->evaluationStatus() == REJECTED ) {
							SEISCOMP_WARNING("%s: relocated origin has evaluation status REJECTED: result not sent",
							                 org->publicID().c_str());
							sendOrigin = false;
						}
					}
					catch ( ... ) {}
				}

				if ( sendOrigin && !send(newOrg.get()) ) {
					SEISCOMP_ERROR("%s: sending of derived origin failed", org->publicID().c_str());
					return;
				}

				return;
			}
		}


	private:
		OriginPtr process(Origin *org) {
			if ( org->arrivalCount() == 0 )
				query()->loadArrivals(org);

			LocatorInterface::PickList picks;

			// Load all referenced picks and store them locally. Through the
			// global PublicObject pool they can then be found by the locator.
			for ( size_t i = 0; i < org->arrivalCount(); ++i ) {
				Arrival *ar = org->arrival(i);
				PickPtr pick = _cache.get<Pick>(ar->pickID());
				if ( !pick ) continue;

				if ( !_useWeight ) {
					// Set weight to 1
					ar->setWeight(1.0);
				}

				// Use all picks regardless of weight
				picks.push_back(LocatorInterface::WeightedPick(pick,1));
			}

			OriginPtr newOrg = _locator->relocate(org);
			if ( newOrg ) {
				if ( _originEvaluationMode == "AUTOMATIC" )
					newOrg->setEvaluationMode(EvaluationMode(AUTOMATIC));
				else
					newOrg->setEvaluationMode(EvaluationMode(MANUAL));

				CreationInfo *ci;

				try {
					ci = &newOrg->creationInfo();
				}
				catch ( ... ) {
					newOrg->setCreationInfo(CreationInfo());
					newOrg->creationInfo().setCreationTime(Core::Time::GMT());
					ci = &newOrg->creationInfo();
				}

				ci->setAgencyID(agencyID());
				ci->setAuthor(author());
			}

			if ( commandline().hasOption("dump") ) {
				EventParametersPtr ep = new EventParameters;
				ep->add(newOrg.get());

				for ( LocatorInterface::PickList::iterator it = picks.begin();
				      it != picks.end(); ++it ) {
					ep->add(it->first.get());
				}

				IO::XMLArchive ar;
				ar.setFormattedOutput(true);
				ar.create("-");
				ar << ep;
				ar.close();
			}

			return newOrg;
		}


		bool send(Origin *org) {
			if ( org == NULL ) return false;

			logObject(_outputOrgs, Core::Time::GMT());

			if ( commandline().hasOption("test") ) return true;

			EventParametersPtr ep = new EventParameters;

			bool wasEnabled = Notifier::IsEnabled();
			Notifier::Enable();

			// Insert origin to event parameters
			ep->add(org);

			NotifierMessagePtr msg = Notifier::GetMessage();

			bool result = false;
			if ( connection() )
				result = connection()->send(msg.get());

			Notifier::SetEnabled(wasEnabled);

			return result;
		}


	private:
		std::vector<std::string>   _originIDs;
		std::string                _locatorType;
		std::string                _locatorProfile;
		bool                       _ignoreRejected;
		bool                       _allowPreliminary;
		LocatorInterfacePtr        _locator;
		PublicObjectTimeSpanBuffer _cache;
		ObjectLog                 *_inputOrgs;
		ObjectLog                 *_outputOrgs;
		bool                       _useWeight;
		std::string                _originEvaluationMode;
		std::string                _epFile;
};


int main(int argc, char **argv) {
	int retCode = EXIT_SUCCESS;

	// Create an own block to make sure the application object
	// is destroyed when printing the overall objectcount
	{
		Reloc app(argc, argv);
		retCode = app.exec();
	}

	return retCode;
}
