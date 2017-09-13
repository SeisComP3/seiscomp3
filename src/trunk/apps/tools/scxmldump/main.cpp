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


#define SEISCOMP_COMPONENT SCEventDump

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/config.h>
#include <seiscomp3/datamodel/routing.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/momenttensor.h>

#include <set>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;


class EventDump : public Seiscomp::Client::Application {
	public:
		EventDump(int argc, char** argv) : Application(argc, argv) {
			setPrimaryMessagingGroup("LISTENER_GROUP");
			addMessagingSubscription("EVENT");
			setMessagingEnabled(true);
			setDatabaseEnabled(true, false);
			setAutoApplyNotifierEnabled(false);
			setInterpretNotifierEnabled(true);

			addMessagingSubscription("EVENT");
		}


	protected:
		void createCommandLineDescription() {
			commandline().addGroup("Dump");
			commandline().addOption("Dump", "listen", "listen to the message server for incoming events");
			commandline().addOption("Dump", "inventory,I", "export the inventory");
			commandline().addOption("Dump", "without-station-groups", "remove station groups from inventory");
			commandline().addOption("Dump", "stations", "if inventory is exported filter the stations to export where each item is in format net[.{sta|*}]", &_stationIDs);
			commandline().addOption("Dump", "config,C", "export the config");
			commandline().addOption("Dump", "routing,R", "export routing");
			commandline().addOption("Dump", "origin,O", "origin id(s)", &_originIDs, false);
			commandline().addOption("Dump", "event,E", "event id(s)", &_eventIDs, false);
			commandline().addOption("Dump", "with-picks,P", "export associated picks");
			commandline().addOption("Dump", "with-amplitudes,A", "export associated amplitudes");
			commandline().addOption("Dump", "with-magnitudes,M", "export station magnitudes");
			commandline().addOption("Dump", "with-focal-mechanisms,F", "export focal mechanisms");
			commandline().addOption("Dump", "ignore-arrivals,a", "do not export origin arrivals");
			commandline().addOption("Dump", "ignore-magnitudes", "ignores magnitudes of exported origins");
			commandline().addOption("Dump", "preferred-only,p", "when exporting events only the preferred origin and the preferred magnitude will be dumped");
			commandline().addOption("Dump", "all-magnitudes,m", "if only the preferred origin is exported, all magnitudes for this origin will be dumped");
			commandline().addOption("Dump", "formatted,f", "use formatted output");
			commandline().addOption("Dump", "prepend-datasize", "prepend a line with the length of the XML string");
			commandline().addOption("Dump", "output,o", "output file (default is stdout)", &_outputFile, false);
		}

		bool validateParameters() {
			if ( !commandline().hasOption("listen") ) {
				if ( !commandline().hasOption("event") && !commandline().hasOption("origin")
				     && !commandline().hasOption("inventory") && !commandline().hasOption("config")
				     && !commandline().hasOption("routing") ) {
					cerr << "Require inventory flag, origin id or event id" << endl;
					return false;
				}

				setMessagingEnabled(false);
				setLoggingToStdErr(true);
			}

			return true;
		}

		bool write(PublicObject *po) {
			XMLArchive ar;
			std::stringbuf buf;
			if ( !ar.create(&buf) ) {
				SEISCOMP_ERROR("Could not created output file '%s'", _outputFile.c_str());
				return false;
			}

			ar.setFormattedOutput(commandline().hasOption("formatted"));

			ar << po;
			ar.close();

			std::string content = buf.str();

			if ( !_outputFile.empty() && _outputFile != "-" ) {
				ofstream file(_outputFile.c_str(), ios::out | ios::trunc);

				if ( !file.is_open() ) {
					SEISCOMP_ERROR("Could not create file: %s", _outputFile.c_str());
					return false;
				}

				if ( commandline().hasOption("prepend-datasize") )
					file << content.size() << endl << content;
				else
					file << content;

				file.close();
			}
			else {
				if ( commandline().hasOption("prepend-datasize") )
					cout << content.size() << endl << content << flush;
				else
					cout << content << flush;
				SEISCOMP_INFO("Flushing %lu bytes", (unsigned long)content.size());
			}

			return true;
		}

		bool run() {
			if ( !query() ) {
				SEISCOMP_ERROR("No database connection");
				return false;
			}

			if ( commandline().hasOption("inventory") ) {
				typedef string NetworkID;
				typedef pair<NetworkID,string> StationID;
				typedef set<NetworkID> NetworkFilter;
				typedef set<StationID> StationFilter;
				NetworkFilter networkFilter;
				StationFilter stationFilter;
				set<string> usedSensors, usedDataloggers, usedDevices, usedResponses;
				vector<string> stationIDs;

				InventoryPtr inv = query()->loadInventory();
				if ( !inv ) {
					SEISCOMP_ERROR("Inventory has not been found");
					return false;
				}

				if ( !_stationIDs.empty() ) {
					Core::split(stationIDs, _stationIDs.c_str(), ",");

					for ( size_t i = 0; i < stationIDs.size(); ++i ) {
						size_t pos = stationIDs[i].find('.');

						if ( pos == string::npos ) {
							stationFilter.insert(StationID(stationIDs[i], "*"));
							networkFilter.insert(stationIDs[i]);
						}
						else {
							stationFilter.insert(
								StationID(
									stationIDs[i].substr(0,pos),
									stationIDs[i].substr(pos+1)
								)
							);
							networkFilter.insert(stationIDs[i].substr(0,pos));
						}
					}

					// Remove unwanted networks
					for ( size_t n = 0; n < inv->networkCount(); ) {
						Network *net = inv->network(n);

						bool passed;
						NetworkFilter::iterator nit;
						for ( nit = networkFilter.begin(), passed = false;
						      nit != networkFilter.end(); ++ nit ) {
							if ( Core::wildcmp(*nit, net->code()) ) {
								passed = true;
								break;
							}
						}

						if ( !passed ) {
							inv->removeNetwork(n);
							continue;
						}

						++n;

						// Remove unwanted stations
						for ( size_t s = 0; s < net->stationCount(); ) {
							Station *sta = net->station(s);

							StationFilter::iterator sit;
							for ( sit = stationFilter.begin(), passed = false;
							      sit != stationFilter.end(); ++ sit ) {
								if ( Core::wildcmp(sit->first, net->code()) &&
								     Core::wildcmp(sit->second, sta->code()) ) {
									passed = true;
									break;
								}
							}

							// Should this station be filtered
							if ( !passed ) {
								net->removeStation(s);
								continue;
							}

							++s;
						}
					}

					// Collect used sensors and dataloggers
					for ( size_t n = 0; n < inv->networkCount(); ++n ) {
						Network *net = inv->network(n);

						for ( size_t s = 0; s < net->stationCount(); ++s ) {
							Station *sta = net->station(s);

							// Collect all used sensors and dataloggers
							for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
								SensorLocation *loc = sta->sensorLocation(l);
								for ( size_t c = 0; c < loc->streamCount(); ++c ) {
									Stream *cha = loc->stream(c);
									usedSensors.insert(cha->sensor());
									usedDataloggers.insert(cha->datalogger());
								}

								for ( size_t a = 0; a < loc->auxStreamCount(); ++a ) {
									AuxStream *aux = loc->auxStream(a);
									usedDevices.insert(aux->device());
								}
							}
						}
					}

					// Removed unused dataloggers
					for ( size_t i = 0; i < inv->dataloggerCount(); ) {
						Datalogger *dl = inv->datalogger(i);
						if ( usedDataloggers.find(dl->publicID()) == usedDataloggers.end() ) {
							inv->removeDatalogger(i);
							continue;
						}

						++i;

						for ( size_t j = 0; j < dl->decimationCount(); ++j ) {
							Decimation *deci = dl->decimation(j);
							try {
								const string &c = deci->analogueFilterChain().content();
								if ( !c.empty() ) {
									vector<string> ids;
									Core::fromString(ids, c);
									usedResponses.insert(ids.begin(), ids.end());
								}
							}
							catch ( ... ) {}

							try {
								const string &c = deci->digitalFilterChain().content();
								if ( !c.empty() ) {
									vector<string> ids;
									Core::fromString(ids, c);
									usedResponses.insert(ids.begin(), ids.end());
								}
							}
							catch ( ... ) {}
						}
					}

					for ( size_t i = 0; i < inv->sensorCount(); ) {
						Sensor *sensor = inv->sensor(i);
						if ( usedSensors.find(sensor->publicID()) == usedSensors.end() ) {
							inv->removeSensor(i);
							continue;
						}

						++i;

						usedResponses.insert(sensor->response());
					}

					for ( size_t i = 0; i < inv->auxDeviceCount(); ) {
						AuxDevice *device = inv->auxDevice(i);
						if ( usedDevices.find(device->publicID()) == usedDevices.end() ) {
							inv->removeAuxDevice(i);
							continue;
						}

						++i;
					}

					// Go through all available responses and remove unused ones
					for ( size_t i = 0; i < inv->responseFIRCount(); ) {
						ResponseFIR *resp = inv->responseFIR(i);
						// Response not used -> remove it
						if ( usedResponses.find(resp->publicID()) == usedResponses.end() )
							inv->removeResponseFIR(i);
						else
							++i;
					}

					for ( size_t i = 0; i < inv->responsePAZCount(); ) {
						ResponsePAZ *resp = inv->responsePAZ(i);
						// Response not used -> remove it
						if ( usedResponses.find(resp->publicID()) == usedResponses.end() )
							inv->removeResponsePAZ(i);
						else
							++i;
					}

					for ( size_t i = 0; i < inv->responsePolynomialCount(); ) {
						ResponsePolynomial *resp = inv->responsePolynomial(i);
						// Response not used -> remove it
						if ( usedResponses.find(resp->publicID()) == usedResponses.end() )
							inv->removeResponsePolynomial(i);
						else
							++i;
					}

					for ( size_t i = 0; i < inv->responseFAPCount(); ) {
						ResponseFAP *resp = inv->responseFAP(i);
						// Response not used -> remove it
						if ( usedResponses.find(resp->publicID()) == usedResponses.end() )
							inv->removeResponseFAP(i);
						else
							++i;
					}
				}

				if ( commandline().hasOption("without-station-groups") ) {
					while ( inv->stationGroupCount() > 0 )
						inv->removeStationGroup(0);
				}

				if ( !write(inv.get()) ) return false;
			}

			if ( commandline().hasOption("config") ) {
				ConfigPtr cfg = query()->loadConfig();
				if ( !cfg ) {
					SEISCOMP_ERROR("Config has not been found");
					return false;
				}

				if ( !write(cfg.get()) ) return false;
			}

			if ( commandline().hasOption("routing") ) {
				RoutingPtr routing = query()->loadRouting();
				if ( !routing ) {
					SEISCOMP_ERROR("Routing has not been found");
					return false;
				}

				if ( !write(routing.get()) ) return false;
			}

			if ( commandline().hasOption("event") ) {
				EventParametersPtr ep = new EventParameters;
				vector<string> eventIDs;
				Core::split(eventIDs, _eventIDs.c_str(), ",");
				for ( vector<string>::const_iterator it = eventIDs.begin();
				      it != eventIDs.end(); ++it ) {
					EventPtr event = Event::Cast(PublicObjectPtr(query()->getObject(Event::TypeInfo(), *it)));
					if ( event )
						addEvent(ep.get(), event.get());
					else
						SEISCOMP_ERROR("Event with id '%s' has not been found", it->c_str());
				}
				return ep->eventCount() > 0 && write(ep.get());
			}

			if ( commandline().hasOption("origin") ) {
				EventParametersPtr ep = new EventParameters;
				vector<string> originIDs;
				Core::split(originIDs, _originIDs.c_str(), ",");
				for ( vector<string>::const_iterator it = originIDs.begin();
				      it != originIDs.end(); ++it ) {
					OriginPtr org = Origin::Cast(PublicObjectPtr(query()->getObject(Origin::TypeInfo(), *it)));
					if ( org )
						addOrigin(ep.get(), org.get());
					else
						SEISCOMP_ERROR("Origin with id '%s' has not been found", it->c_str());
				}
				return ep->originCount() > 0 && write(ep.get());
			}

			if ( commandline().hasOption("listen") )
				return Application::run();

			return true;
		}


		void addObject(const std::string& parentID,
		               Seiscomp::DataModel::Object* object) {
			updateObject(parentID, object);
		}


		void updateObject(const std::string&, Seiscomp::DataModel::Object* object) {
			Event* e = Event::Cast(object);
			if ( !e ) return;

			EventParametersPtr ep = new EventParameters;
			addEvent(ep.get(), e);
			write(ep.get());
		}


		void addOrigin(EventParameters *ep, Origin *org) {
			SEISCOMP_INFO("Dumping Origin '%s'", org->publicID().c_str());
			ep->add(org);

			bool staMags = commandline().hasOption("with-magnitudes");
			bool ignoreArrivals = commandline().hasOption("ignore-arrivals");

			query()->load(org);

			if ( commandline().hasOption("ignore-magnitudes") ) {
				while ( org->magnitudeCount() > 0 )
					org->removeMagnitude(0);
			}

			if ( !staMags ) {
				while ( org->stationMagnitudeCount() > 0 )
					org->removeStationMagnitude(0);

				for ( size_t i = 0; i < org->magnitudeCount(); ++i ) {
					Magnitude* netMag = org->magnitude(i);
					while ( netMag->stationMagnitudeContributionCount() > 0 )
						netMag->removeStationMagnitudeContribution(0);
				}
			}

			if ( ignoreArrivals ) {
				while ( org->arrivalCount() > 0 )
					org->removeArrival(0);
			}

			if ( commandline().hasOption("with-picks") ) {
				for ( size_t a = 0; a < org->arrivalCount(); ++a ) {
					PickPtr pick = Pick::Cast(PublicObjectPtr(query()->getObject(Pick::TypeInfo(), org->arrival(a)->pickID())));
					if ( !pick ) {
						SEISCOMP_WARNING("Pick with id '%s' not found", org->arrival(a)->pickID().c_str());
						continue;
					}

					query()->load(pick.get());

					if ( !pick->eventParameters() )
						ep->add(pick.get());
				}
			}

			if ( commandline().hasOption("with-amplitudes") ) {
				for ( size_t m = 0; m < org->magnitudeCount(); ++m ) {
					Magnitude* netmag = org->magnitude(m);
					for ( size_t s = 0; s < netmag->stationMagnitudeContributionCount(); ++s ) {
						StationMagnitude* stamag = StationMagnitude::Find(netmag->stationMagnitudeContribution(s)->stationMagnitudeID());
						if ( !stamag ) {
							SEISCOMP_WARNING("StationMagnitude with id '%s' not found", netmag->stationMagnitudeContribution(s)->stationMagnitudeID().c_str());
							continue;
						}

						AmplitudePtr staamp = Amplitude::Cast(PublicObjectPtr(query()->getObject(Amplitude::TypeInfo(), stamag->amplitudeID())));

						if ( !staamp ) {
							SEISCOMP_WARNING("Amplitude with id '%s' not found", stamag->amplitudeID().c_str());
							continue;
						}

						if ( !staamp->eventParameters() )
							ep->add(staamp.get());
					}
				}
			}
		}


		void addEvent(EventParameters *ep, Event *event) {
			SEISCOMP_INFO("Dumping Event '%s'", event->publicID().c_str());
			ep->add(event);

			bool preferredOnly = commandline().hasOption("preferred-only");
			bool staMags = commandline().hasOption("with-magnitudes");
			bool allMags = commandline().hasOption("all-magnitudes");
			bool ignoreArrivals = commandline().hasOption("ignore-arrivals");
			bool withFocMechs = commandline().hasOption("with-focal-mechanisms");

			if ( !preferredOnly )
				query()->load(event);
			else {
				query()->loadComments(event);
				query()->loadEventDescriptions(event);
				event->add(OriginReferencePtr(new OriginReference(event->preferredOriginID())).get());

				if ( withFocMechs && !event->preferredFocalMechanismID().empty() )
					event->add(FocalMechanismReferencePtr(new FocalMechanismReference(event->preferredFocalMechanismID())).get());
			}

			bool foundPreferredMag = false;

			// No need to search for it
			if ( event->preferredMagnitudeID().empty() )
				foundPreferredMag = true;

			for ( size_t i = 0; i < event->originReferenceCount(); ++i ) {
				OriginPtr origin = Origin::Cast(PublicObjectPtr(query()->getObject(Origin::TypeInfo(), event->originReference(i)->originID())));
				if ( !origin ) {
					SEISCOMP_WARNING("Origin with id '%s' not found", event->originReference(i)->originID().c_str());
					continue;
				}

				query()->load(origin.get());

				if ( preferredOnly && !allMags ) {
					MagnitudePtr netMag;
					while ( origin->magnitudeCount() > 0 ) {
						if ( origin->magnitude(0)->publicID() == event->preferredMagnitudeID() )
							netMag = origin->magnitude(0);

						origin->removeMagnitude(0);
					}

					if ( netMag ) {
						foundPreferredMag = true;
						origin->add(netMag.get());
					}
				}
				else if ( !foundPreferredMag ){
					for ( size_t m = 0; m < origin->magnitudeCount(); ++m ) {
						if ( origin->magnitude(m)->publicID() == event->preferredMagnitudeID() ) {
							foundPreferredMag = true;
							break;
						}
					}
				}

				if ( !staMags ) {
					while ( origin->stationMagnitudeCount() > 0 )
						origin->removeStationMagnitude(0);

					for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
						Magnitude* netMag = origin->magnitude(i);
						while ( netMag->stationMagnitudeContributionCount() > 0 )
							netMag->removeStationMagnitudeContribution(0);
					}
				}

				if ( ignoreArrivals ) {
					while ( origin->arrivalCount() > 0 )
						origin->removeArrival(0);
				}

				ep->add(origin.get());

				if ( commandline().hasOption("with-picks") ) {
					for ( size_t a = 0; a < origin->arrivalCount(); ++a ) {
						PickPtr pick = Pick::Cast(PublicObjectPtr(query()->getObject(Pick::TypeInfo(), origin->arrival(a)->pickID())));
						if ( !pick ) {
							SEISCOMP_WARNING("Pick with id '%s' not found", origin->arrival(a)->pickID().c_str());
							continue;
						}

						query()->load(pick.get());

						if ( !pick->eventParameters() )
							ep->add(pick.get());
					}
				}

				if ( commandline().hasOption("with-amplitudes") ) {
					for ( size_t m = 0; m < origin->magnitudeCount(); ++m ) {
						Magnitude* netmag = origin->magnitude(m);
						for ( size_t s = 0; s < netmag->stationMagnitudeContributionCount(); ++s ) {
							StationMagnitude* stamag = StationMagnitude::Find(netmag->stationMagnitudeContribution(s)->stationMagnitudeID());
							if ( !stamag ) {
								SEISCOMP_WARNING("StationMagnitude with id '%s' not found", netmag->stationMagnitudeContribution(s)->stationMagnitudeID().c_str());
								continue;
							}

							AmplitudePtr staamp = Amplitude::Cast(PublicObjectPtr(query()->getObject(Amplitude::TypeInfo(), stamag->amplitudeID())));

							if ( !staamp ) {
								SEISCOMP_WARNING("Amplitude with id '%s' not found", stamag->amplitudeID().c_str());
								continue;
							}

							if ( !staamp->eventParameters() )
								ep->add(staamp.get());
						}
					}
				}
			}

			if ( !withFocMechs ) {
				while ( event->focalMechanismReferenceCount() > 0 )
					event->removeFocalMechanismReference(0);
			}

			for ( size_t i = 0; i < event->focalMechanismReferenceCount(); ++i ) {
				FocalMechanismPtr fm = FocalMechanism::Cast(PublicObjectPtr(query()->getObject(FocalMechanism::TypeInfo(), event->focalMechanismReference(i)->focalMechanismID())));
				if ( !fm ) {
					SEISCOMP_WARNING("FocalMechanism with id '%s' not found", event->focalMechanismReference(i)->focalMechanismID().c_str());
					continue;
				}

				query()->load(fm.get());
				ep->add(fm.get());

				for ( size_t m = 0; m < fm->momentTensorCount(); ++m ) {
					MomentTensor *mt = fm->momentTensor(m);
					if ( mt->derivedOriginID().empty() ) continue;

					OriginPtr derivedOrigin = ep->findOrigin(mt->derivedOriginID());
					if ( derivedOrigin != NULL ) continue;

					derivedOrigin = Origin::Cast(PublicObjectPtr(query()->getObject(Origin::TypeInfo(), mt->derivedOriginID())));
					if ( !derivedOrigin ) {
						SEISCOMP_WARNING("Derived MT origin with id '%s' not found", mt->derivedOriginID().c_str());
						continue;
					}

					query()->load(derivedOrigin.get());
					ep->add(derivedOrigin.get());

					if ( !foundPreferredMag ) {
						for ( size_t m = 0; m < derivedOrigin->magnitudeCount(); ++m ) {
							if ( derivedOrigin->magnitude(m)->publicID() == event->preferredMagnitudeID() ) {
								foundPreferredMag = true;
								break;
							}
						}
					}
				}
			}

			// Find the preferred magnitude
			if ( !foundPreferredMag ) {
				OriginPtr org = query()->getOriginByMagnitude(event->preferredMagnitudeID());
				if ( org ) {
					query()->load(org.get());

					if ( !staMags ) {
						while ( org->stationMagnitudeCount() > 0 )
							org->removeStationMagnitude(0);

						for ( size_t i = 0; i < org->magnitudeCount(); ++i ) {
							Magnitude* netMag = org->magnitude(i);
							while ( netMag->stationMagnitudeContributionCount() > 0 )
								netMag->removeStationMagnitudeContribution(0);
						}
					}

					if ( ignoreArrivals ) {
						while ( org->arrivalCount() > 0 )
							org->removeArrival(0);
					}

					if ( preferredOnly && !allMags ) {
						MagnitudePtr netMag;
						while ( org->magnitudeCount() > 0 ) {
							if ( org->magnitude(0)->publicID() == event->preferredMagnitudeID() )
								netMag = org->magnitude(0);

							org->removeMagnitude(0);
						}

						if ( netMag )
							org->add(netMag.get());
					}

					ep->add(org.get());
				}
			}
		}


	private:
		string _outputFile;
		string _originIDs;
		string _eventIDs;
		string _stationIDs;
};


int main(int argc, char** argv) {
	EventDump app(argc, argv);
	return app.exec();
}
