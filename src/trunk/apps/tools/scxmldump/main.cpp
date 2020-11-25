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
#include <seiscomp3/datamodel/dataavailability.h>
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


static void removeAllArrivals(Seiscomp::DataModel::Origin *origin) {

	while ( origin->arrivalCount() > 0 )
		origin->removeArrival(0);
}


static void removeAllNetworkMagnitudes(Seiscomp::DataModel::Origin *origin) {

	while ( origin->magnitudeCount() > 0 )
		origin->removeMagnitude(0);
}


static void removeAllStationMagnitudes(Seiscomp::DataModel::Origin *origin) {

	while ( origin->stationMagnitudeCount() > 0 )
		origin->removeStationMagnitude(0);

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		Magnitude* netMag = origin->magnitude(i);
		while ( netMag->stationMagnitudeContributionCount() > 0 )
			netMag->removeStationMagnitudeContribution(0);
	}
}


static void removeAllStationContributions(Seiscomp::DataModel::MomentTensor *mt) {

	while ( mt->momentTensorStationContributionCount() > 0 )
		mt->removeMomentTensorStationContribution(0);
}



class EventDump : public Seiscomp::Client::Application {
	public:
		EventDump(int argc, char** argv) : Application(argc, argv) {
			setPrimaryMessagingGroup(Seiscomp::Communication::Protocol::LISTENER_GROUP);
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
			commandline().addOption("Dump", "availability,Y", "export data availability information");
			commandline().addOption("Dump", "with-segments", "export individual data availability segments");
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
				if ( !commandline().hasOption("event")
				  && !commandline().hasOption("origin")
				  && !commandline().hasOption("inventory")
				  && !commandline().hasOption("config")
				  && !commandline().hasOption("routing")
				  && !commandline().hasOption("availability") ) {
					cerr << "Require inventory, config, routing or "
					        "availability flag, or origin or event id" << endl;
					return false;
				}

				setMessagingEnabled(false);
				setLoggingToStdErr(true);
			}

			withPicks             = commandline().hasOption("with-picks");
			withAmplitudes        = commandline().hasOption("with-amplitudes");
			preferredOnly         = commandline().hasOption("preferred-only");
			withStationMagnitudes = commandline().hasOption("with-magnitudes");
			allMagnitudes         = commandline().hasOption("all-magnitudes");
			ignoreArrivals        = commandline().hasOption("ignore-arrivals");
			withFocalMechanisms   = commandline().hasOption("with-focal-mechanisms");

			return true;
		}

		bool write(PublicObject *po) {
			XMLArchive ar;
			stringbuf buf;
			if ( !ar.create(&buf) ) {
				SEISCOMP_ERROR("Could not create output file '%s'", _outputFile.c_str());
				return false;
			}

			ar.setFormattedOutput(commandline().hasOption("formatted"));

			ar << po;
			ar.close();

			string content = buf.str();

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

					for ( size_t i = 0; i < inv->responseIIRCount(); ) {
						ResponseIIR *resp = inv->responseIIR(i);
						// Response not used -> remove it
						if ( usedResponses.find(resp->publicID()) == usedResponses.end() )
							inv->removeResponseIIR(i);
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

			if ( commandline().hasOption("availability") ) {
				DataAvailabilityPtr avail = new DataAvailability();
				query()->loadDataExtents(avail.get());
				if ( avail->dataExtentCount() == 0 ) {
					SEISCOMP_ERROR("No data availability extents found");
					return false;
				}

				if ( commandline().hasOption("with-segments") ) {
					for ( size_t i = 0; i < avail->dataExtentCount(); ++i ) {
						query()->load(avail->dataExtent(i));
					}
				}
				else {
					for ( size_t i = 0; i < avail->dataExtentCount(); ++i ) {
						query()->loadDataAttributeExtents(avail->dataExtent(i));
					}
				}

				if ( !write(avail.get()) ) return false;
			}

			if ( commandline().hasOption("event") ) {
				EventParametersPtr ep = new EventParameters;
				vector<string> eventIDs;
				Core::split(eventIDs, _eventIDs.c_str(), ",");
				for ( vector<string>::const_iterator it = eventIDs.begin();
				      it != eventIDs.end(); ++it ) {
					EventPtr event = Event::Cast(PublicObjectPtr(
						query()->getObject(Event::TypeInfo(), *it)));
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
					OriginPtr origin = Origin::Cast(PublicObjectPtr(
						query()->getObject(Origin::TypeInfo(), *it)));
					if ( origin )
						addOrigin(ep.get(), origin.get());
					else
						SEISCOMP_ERROR("Origin with id '%s' has not been found", it->c_str());
				}
				return ep->originCount() > 0 && write(ep.get());
			}

			if ( commandline().hasOption("listen") )
				return Application::run();

			return true;
		}


		void addObject(const string& parentID,
		               Seiscomp::DataModel::Object* object) {
			updateObject(parentID, object);
		}


		void updateObject(const string&, Seiscomp::DataModel::Object* object) {
			Event* e = Event::Cast(object);
			if ( !e ) return;

			EventParametersPtr ep = new EventParameters;
			addEvent(ep.get(), e);
			write(ep.get());
		}


		void addOrigin(EventParameters *ep, Origin *origin) {
			SEISCOMP_INFO("Dumping Origin '%s'", origin->publicID().c_str());
			ep->add(origin);

			query()->load(origin);

			if ( commandline().hasOption("ignore-magnitudes") )
				removeAllNetworkMagnitudes(origin);

			if ( !withStationMagnitudes )
				removeAllStationMagnitudes(origin);

			if ( ignoreArrivals )
				removeAllArrivals(origin);

			if ( withPicks ) {
				for ( size_t a = 0; a < origin->arrivalCount(); ++a ) {
					const string &pickID = origin->arrival(a)->pickID();
					if ( _pickIDs.find(pickID) != _pickIDs.end() )
						continue;

					PickPtr pick = Pick::Cast(PublicObjectPtr(
						query()->getObject(Pick::TypeInfo(), pickID)));
					if ( !pick ) {
						SEISCOMP_WARNING("Pick with id '%s' not found", pickID.c_str());
						continue;
					}

					query()->load(pick.get());

					if ( !pick->eventParameters() )
						ep->add(pick.get());
					_pickIDs.insert(pickID);
				}
			}

			if ( withAmplitudes && withStationMagnitudes ) {
				// Extract amplitudes corresponding to station magnitudes
				for ( size_t m = 0; m < origin->magnitudeCount(); ++m ) {
					Magnitude* netMag = origin->magnitude(m);
					for ( size_t s = 0; s < netMag->stationMagnitudeContributionCount(); ++s ) {
						const string &stationMagnitudeID =
							netMag->stationMagnitudeContribution(s)->stationMagnitudeID();
						StationMagnitude* staMag = StationMagnitude::Find(stationMagnitudeID);
						if ( !staMag ) {
							SEISCOMP_WARNING("StationMagnitude with id '%s' not found",
									 stationMagnitudeID.c_str());
							continue;
						}

						const string &amplitudeID = staMag->amplitudeID();
						if ( _amplitudeIDs.find(amplitudeID) != _amplitudeIDs.end() )
							continue;

						AmplitudePtr amplitude = Amplitude::Cast(PublicObjectPtr(
							query()->getObject(Amplitude::TypeInfo(), amplitudeID)));

						if ( !amplitude ) {
							SEISCOMP_WARNING("Amplitude with id '%s' not found",
									 amplitudeID.c_str());
							continue;
						}

						if ( !amplitude->eventParameters() )
							ep->add(amplitude.get());

						_amplitudeIDs.insert(amplitudeID);
					}
				}
			}

			if ( withAmplitudes && !withStationMagnitudes ) {
				// Extract all amplitudes for all picks
				for ( size_t p = 0; p < ep->pickCount(); ++p ) {
					Pick *pick = ep->pick(p);
					DatabaseIterator it = query()->getAmplitudesForPick(pick->publicID());
					for ( ; *it; ++it ) {
						Amplitude *amplitude = Amplitude::Cast(*it);
						if ( _amplitudeIDs.find(amplitude->publicID()) != _amplitudeIDs.end() )
							continue;

						if ( amplitude )
							ep->add(amplitude);

						_amplitudeIDs.insert(amplitude->publicID());
					}
				}
			}
		}


		void addEvent(EventParameters *ep, Event *event) {
			SEISCOMP_INFO("Dumping Event '%s'", event->publicID().c_str());
			ep->add(event);

			if ( !preferredOnly )
				query()->load(event);
			else {
				query()->loadComments(event);
				query()->loadEventDescriptions(event);
				event->add(
					OriginReferencePtr(
						new OriginReference(event->preferredOriginID())
					).get()
				);

				if ( withFocalMechanisms
				  && !event->preferredFocalMechanismID().empty() )
					event->add(
						FocalMechanismReferencePtr(
							new FocalMechanismReference(
								event->preferredFocalMechanismID()
							)
						).get()
					);
			}

			bool foundPreferredMag = false;

			// No need to search for it
			if ( event->preferredMagnitudeID().empty() )
				foundPreferredMag = true;


			// loop over origins referenced by event
			for ( size_t i = 0; i < event->originReferenceCount(); ++i ) {
				const string originID = event->originReference(i)->originID();
				OriginPtr origin = Origin::Cast(PublicObjectPtr(
					query()->getObject(Origin::TypeInfo(), originID)));
				if ( !origin ) {
					SEISCOMP_WARNING("Origin with id '%s' not found", originID.c_str());
					continue;
				}

				query()->load(origin.get());

				if ( preferredOnly && !allMagnitudes ) {
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
				else if ( !foundPreferredMag ) {
					for ( size_t m = 0; m < origin->magnitudeCount(); ++m ) {
						if ( origin->magnitude(m)->publicID() == event->preferredMagnitudeID() ) {
							foundPreferredMag = true;
							break;
						}
					}
				}

				if ( !withStationMagnitudes )
					removeAllStationMagnitudes(origin.get());

				if ( ignoreArrivals )
					removeAllArrivals(origin.get());

				ep->add(origin.get());

				if ( withPicks ) {
					for ( size_t a = 0; a < origin->arrivalCount(); ++a ) {
						const string &pickID = origin->arrival(a)->pickID();

						if (_pickIDs.find(pickID) != _pickIDs.end())
							continue;

						PickPtr pick = Pick::Cast(PublicObjectPtr(
							query()->getObject(Pick::TypeInfo(), pickID)));
						if ( !pick ) {
							SEISCOMP_WARNING("Pick with id '%s' not found",
							                 pickID.c_str());
							continue;
						}

						query()->load(pick.get());

						if ( !pick->eventParameters() )
							ep->add(pick.get());
						_pickIDs.insert(pickID);
					}
				}

				if ( withAmplitudes && withStationMagnitudes ) {
					for ( size_t m = 0; m < origin->magnitudeCount(); ++m ) {
						Magnitude* netMag = origin->magnitude(m);
						for ( size_t s = 0; s < netMag->stationMagnitudeContributionCount(); ++s ) {
							const string &staMagID
								= netMag->stationMagnitudeContribution(s)->stationMagnitudeID();
							StationMagnitude* staMag = StationMagnitude::Find(staMagID);
							if ( !staMag ) {
								SEISCOMP_WARNING("StationMagnitude with id '%s' not found",
								                 staMagID.c_str());
								continue;
							}

							const string &amplitudeID = staMag->amplitudeID();
							if ( _amplitudeIDs.find(amplitudeID) != _amplitudeIDs.end() )
								continue;

							AmplitudePtr amplitude = Amplitude::Cast(PublicObjectPtr(
								query()->getObject(Amplitude::TypeInfo(), amplitudeID)));

							if ( !amplitude ) {
								SEISCOMP_WARNING("Amplitude with id '%s' not found",
								                 amplitudeID.c_str());
								continue;
							}

							if ( !amplitude->eventParameters() )
								ep->add(amplitude.get());

							_amplitudeIDs.insert(amplitudeID);
						}
					}
				}
			}
			// end of loop over origins referenced by event


			if ( withAmplitudes && !withStationMagnitudes ) {
				// Extract all amplitudes for all picks
				for ( size_t p = 0; p < ep->pickCount(); ++p ) {
					Pick *pick = ep->pick(p);
					DatabaseIterator it = query()->getAmplitudesForPick(pick->publicID());
					for ( ; *it; ++it ) {
						Amplitude *amplitude = Amplitude::Cast(*it);
						if ( _amplitudeIDs.find(amplitude->publicID()) != _amplitudeIDs.end() )
							continue;

						if ( amplitude )
							ep->add(amplitude);

						_amplitudeIDs.insert(amplitude->publicID());
					}
				}
			}

			if ( !withFocalMechanisms ) {
				while ( event->focalMechanismReferenceCount() > 0 )
					event->removeFocalMechanismReference(0);
			}

			for ( size_t i = 0; i < event->focalMechanismReferenceCount(); ++i ) {
				const string &fmID =
					event->focalMechanismReference(i)->focalMechanismID();
				FocalMechanismPtr fm = FocalMechanism::Cast(PublicObjectPtr(
					query()->getObject(FocalMechanism::TypeInfo(), fmID)));
				if ( !fm ) {
					SEISCOMP_WARNING("FocalMechanism with id '%s' not found", fmID.c_str());
					continue;
				}

				query()->load(fm.get());
				for ( size_t m = 0; m < fm->momentTensorCount(); ++m ) {
					MomentTensor *mt = fm->momentTensor(m);
					if ( ignoreArrivals ) // TODO: review!
						removeAllStationContributions(mt);
				}

				if ( !fm->triggeringOriginID().empty()
				  && fm->triggeringOriginID() != event->preferredOriginID() ) {

					OriginPtr triggeringOrigin = ep->findOrigin(fm->triggeringOriginID());
					if ( !triggeringOrigin ) {
						triggeringOrigin = Origin::Cast(PublicObjectPtr(
							query()->getObject(Origin::TypeInfo(), fm->triggeringOriginID())));
						if ( !triggeringOrigin ) {
							SEISCOMP_WARNING("Triggering origin with id '%s' not found",
							                 fm->triggeringOriginID().c_str());
						}
						else {
							query()->load(triggeringOrigin.get());

							if ( !withStationMagnitudes )
								removeAllStationMagnitudes(triggeringOrigin.get());

							if ( ignoreArrivals )
								removeAllArrivals(triggeringOrigin.get());

							if ( preferredOnly && !allMagnitudes ) {
								MagnitudePtr netMag;
								while ( triggeringOrigin->magnitudeCount() > 0 ) {
									if ( triggeringOrigin->magnitude(0)->publicID() == event->preferredMagnitudeID() )
										netMag = triggeringOrigin->magnitude(0);

									triggeringOrigin->removeMagnitude(0);
								}

								if ( netMag )
									triggeringOrigin->add(netMag.get());
							}

							ep->add(triggeringOrigin.get());
						}
					}
				}

				ep->add(fm.get());

				for ( size_t m = 0; m < fm->momentTensorCount(); ++m ) {
					MomentTensor *mt = fm->momentTensor(m);
					if ( mt->derivedOriginID().empty() ) continue;

					OriginPtr derivedOrigin = ep->findOrigin(mt->derivedOriginID());
					if ( derivedOrigin ) continue;

					derivedOrigin = Origin::Cast(PublicObjectPtr(
						query()->getObject(Origin::TypeInfo(), mt->derivedOriginID())));
					if ( !derivedOrigin ) {
						SEISCOMP_WARNING("Derived MT origin with id '%s' not found",
						                 mt->derivedOriginID().c_str());
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
				OriginPtr origin = query()->getOriginByMagnitude(event->preferredMagnitudeID());
				if ( origin ) {
					query()->load(origin.get());

					if ( !withStationMagnitudes )
						removeAllStationMagnitudes(origin.get());

					if ( ignoreArrivals )
						removeAllArrivals(origin.get());

					if ( preferredOnly && !allMagnitudes ) {
						MagnitudePtr netMag;
						while ( origin->magnitudeCount() > 0 ) {
							if ( origin->magnitude(0)->publicID() == event->preferredMagnitudeID() )
								netMag = origin->magnitude(0);

							origin->removeMagnitude(0);
						}

						if ( netMag )
							origin->add(netMag.get());
					}

					ep->add(origin.get());
				}
			}
		}


	private:
		bool preferredOnly;
		bool allMagnitudes;
		bool ignoreArrivals;
		bool withPicks;
		bool withAmplitudes;
		bool withStationMagnitudes;
		bool withFocalMechanisms;

		string _outputFile;
		string _originIDs;
		string _eventIDs;

		// picks and amplitudes are often shared between origins
		// and we therefore keep track of the objects already added
		// to the event parameters. This avoids redundant database
		// queries and even error messages.
		set<string> _pickIDs, _amplitudeIDs;

		string _stationIDs;
};


int main(int argc, char** argv) {
	EventDump app(argc, argv);
	return app.exec();
}
