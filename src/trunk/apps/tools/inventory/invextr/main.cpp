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


#include <seiscomp3/client/application.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>


using namespace std;
using namespace Seiscomp;


bool filter(const std::vector<string> &whitelist, const string &id) {
	if ( whitelist.empty() ) return false;

	for ( size_t i = 0; i < whitelist.size(); ++i ) {
		if ( Core::wildcmp(whitelist[i], id) )
			return false;
	}

	return true;
}


class InventoryExtractor : public Client::Application {
	public:
		InventoryExtractor(int argc, char **argv)
		: Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(false, false);
			setLoggingToStdErr(true);
			_remove = false;
		}


		void createCommandLineDescription() {
			Client::Application::createCommandLineDescription();
			commandline().addGroup("Extract");
			commandline().addOption("Extract", "chans",
			                        "A comma separated list of channel id's to "
			                        "extract which can contain wildcards.",
			                        &_chanIDs);
			commandline().addOption("Extract", "rm",
			                        "Removes the given channels instead of "
			                        "extracting them.");
			commandline().addOption("Extract", "formatted,f", "Enables formatted XML output");
		}


		bool validateParameters() {
			if ( !Client::Application::validateParameters() ) return false;
			_remove = commandline().hasOption("rm");
			return true;
		}


		void printUsage() const {
			cout << "Usage: " << name() << " [OPTIONS] [input=stdin] [output=stdout]" << endl;
			Client::Application::printUsage();
		}


		bool run() {
			vector<string> chanIDs;
			Core::split(chanIDs, _chanIDs.c_str(), ",");

			vector<string> opts = commandline().unrecognizedOptions();
			string input("-"), output("-");

			if ( opts.size() > 0 )
				input = opts[0];
			if ( opts.size() > 1 )
				output = opts[1];

			DataModel::InventoryPtr inv;

			IO::XMLArchive ar;
			if ( !ar.open(input.c_str()) ) {
				cerr << "Unable to open " << input << endl;
				return false;
			}

			ar >> inv;
			if ( inv == NULL ) {
				cerr << "No inventory found in " << input << endl;
				return false;
			}

			ar.close();

			string id0;
			set<string> usedSensors, usedDataloggers;

			for ( size_t n = 0; n < inv->networkCount(); ) {
				DataModel::Network *net = inv->network(n);
				id0 = net->code();

				for ( size_t s = 0; s < net->stationCount(); ) {
					DataModel::Station *sta = net->station(s);
					string id1 = id0 + "." + sta->code();

					for ( size_t l = 0; l < sta->sensorLocationCount(); ) {
						DataModel::SensorLocation *loc = sta->sensorLocation(l);
						string id2 = id1 + "." + loc->code();

						for ( size_t c = 0; c < loc->streamCount(); ) {
							DataModel::Stream *cha = loc->stream(c);
							string id3 = id2 + "." + cha->code();

							if ( _remove != filter(chanIDs, id3) ) {
								loc->removeStream(c);
								continue;
							}

							// Keep track of used sensors and dataloggers
							if ( !cha->sensor().empty() )
								usedSensors.insert(cha->sensor());

							if ( !cha->datalogger().empty() )
								usedDataloggers.insert(cha->datalogger());

							++c;
						}

						if ( loc->streamCount() == 0 )
							sta->removeSensorLocation(l);
						else
							++l;
					}

					if ( sta->sensorLocationCount() == 0 )
						net->removeStation(s);
					else
						++s;
				}

				if ( net->stationCount() == 0 )
					inv->removeNetwork(n);
				else
					++n;
			}

			set<string> usedResponses;

			// Remove unused sensors
			for ( size_t s = 0; s < inv->sensorCount(); ) {
				if ( usedSensors.find(inv->sensor(s)->publicID()) == usedSensors.end() )
					inv->removeSensor(s);
				else {
					usedResponses.insert(inv->sensor(s)->response());
					++s;
				}
			}

			// Remove unused dataloggers
			for ( size_t d = 0; d < inv->dataloggerCount(); ) {
				DataModel::Datalogger *dl = inv->datalogger(d);
				if ( usedDataloggers.find(dl->publicID()) == usedDataloggers.end() )
					inv->removeDatalogger(d);
				else {
					for ( size_t i = 0; i < dl->decimationCount(); ++i ) {
						DataModel::Decimation *deci = dl->decimation(i);
						try {
							vector<string> filters;
							Core::split(filters, deci->analogueFilterChain().content().c_str(), " ");

							for ( size_t j = 0; j < filters.size(); ++j ) {
								if ( filters[j].empty() ) continue;
								usedResponses.insert(filters[j]);
							}
						}
						catch ( ... ) {}

						try {
							vector<string> filters;
							Core::split(filters, deci->digitalFilterChain().content().c_str(), " ");

							for ( size_t j = 0; j < filters.size(); ++j ) {
								if ( filters[j].empty() ) continue;
								usedResponses.insert(filters[j]);
							}
						}
						catch ( ... ) {}
					}

					++d;
				}
			}


			// Remove unused responses
			for ( size_t i = 0; i < inv->responsePAZCount(); ) {
				if ( usedResponses.find(inv->responsePAZ(i)->publicID()) == usedResponses.end() )
					inv->removeResponsePAZ(i);
				else
					++i;
			}

			for ( size_t i = 0; i < inv->responsePolynomialCount(); ) {
				if ( usedResponses.find(inv->responsePolynomial(i)->publicID()) == usedResponses.end() )
					inv->removeResponsePolynomial(i);
				else
					++i;
			}

			for ( size_t i = 0; i < inv->responseFIRCount(); ) {
				if ( usedResponses.find(inv->responseFIR(i)->publicID()) == usedResponses.end() )
					inv->removeResponseFIR(i);
				else
					++i;
			}

			if ( !ar.create(output.c_str()) ) {
				cerr << "Unable to create output " << output << endl;
				return false;
			}

			ar.setFormattedOutput(commandline().hasOption("formatted"));
			ar << inv;
			ar.close();

			return true;
		}


	private:
		string  _chanIDs;
		bool    _remove;
};


int main(int argc, char **argv) {
	return InventoryExtractor(argc, argv)();
}
