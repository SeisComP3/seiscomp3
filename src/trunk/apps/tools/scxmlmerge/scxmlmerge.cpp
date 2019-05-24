/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   EMail: jabe@gempa.de                                                  *
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
#include <seiscomp3/datamodel/config.h>
#include <seiscomp3/datamodel/dataavailability.h>
#include <seiscomp3/datamodel/diff.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/datamodel/qualitycontrol.h>
#include <seiscomp3/datamodel/routing.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include <iostream>


class XMLMerge : public Seiscomp::Client::Application {

	public:
		XMLMerge(int argc, char **argv) : Seiscomp::Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(false, false);
			setDaemonEnabled(false);
		}

		bool init() {
			if ( !Seiscomp::Client::Application::init() )
				return false;

			_files = commandline().unrecognizedOptions();
			if ( _files.empty() ) {
				std::cerr << "No input files given" << std::endl;
				printUsage();
				return false;
			}

			return true;
		}

		void printUsage() const {
			std::cout << std::endl << "Description:" << std::endl;
			std::cout << "  Merge the content of multiple XML files. "
			             "Different root elements like EventParameters and "
			             "Inventory may be combined."
			          << std::endl
			          << std::endl << "Synopsis:" << std::endl
			          << "  scxmlmerge [options] inputFiles" << std::endl;

			Seiscomp::Client::Application::printUsage();

			std::cout << "Examples:" << std::endl;
			std::cout << "  scxmlmerge file1.xml file2.xml > file.xml"
			          << std::endl << std::endl
			          << "  Merges all SeisComP3 elements the from 2 XML files into a single XML file "
			          << std::endl << std::endl;
			std::cout << "  scxmlmerge -E -C file1.xml file2.xml > file.xml"
			          << std::endl << std::endl
			          << "  Merges the EventParameters and Config elements from 2 "
                         "XML files into a single XML file "
			          << std::endl;
		}

		void createCommandLineDescription() {
			commandline().addGroup("Dump");
			commandline().addOption("Dump", "event,E", "Include EventParameters");
			commandline().addOption("Dump", "inventory,I", "Include Inventory");
			commandline().addOption("Dump", "config,C", "Include Config");
			commandline().addOption("Dump", "routing,R", "Include Routing");
			commandline().addOption("Dump", "quality,Q", "Include QualityControl");
			commandline().addOption("Dump", "availability,Y", "Include DataAvailability");
		}

		bool run() {
			Seiscomp::DataModel::PublicObject::SetRegistrationEnabled(false);

			std::vector<Seiscomp::DataModel::ObjectPtr> storage;

			// set up root objects to collect
			bool collectEP      = commandline().hasOption("event");
			bool collectInv     = commandline().hasOption("inventory");
			bool collectCfg     = commandline().hasOption("config");
			bool collectRouting = commandline().hasOption("routing");
			bool collectQC      = commandline().hasOption("quality");
			bool collectDA      = commandline().hasOption("availability");

			bool collectAll = !collectEP && !collectInv && !collectCfg &&
			                  !collectRouting && !collectQC && !collectDA;

			if ( collectAll || collectEP )
				registerRootObject(Seiscomp::DataModel::EventParameters::ClassName());
			if ( collectAll || collectInv )
				registerRootObject(Seiscomp::DataModel::Inventory::ClassName());
			if ( collectAll || collectCfg )
				registerRootObject(Seiscomp::DataModel::Config::ClassName());
			if ( collectAll || collectRouting )
				registerRootObject(Seiscomp::DataModel::Routing::ClassName());
			if ( collectAll || collectQC )
				registerRootObject(Seiscomp::DataModel::QualityControl::ClassName());
			if ( collectAll || collectDA )
				registerRootObject(Seiscomp::DataModel::DataAvailability::ClassName());

			std::map<std::string, Objects>::iterator it;

			for ( size_t i = 0; i < _files.size(); ++i ) {
				Seiscomp::IO::XMLArchive ar;
				if ( !ar.open(_files[i].c_str()) ) {
					std::cerr << "Failed to open file: " << _files[i] << std::endl;
					return false;
				}

				std::cerr << "+ " << _files[i] << std::endl;
				Seiscomp::DataModel::ObjectPtr obj;
				bool first = true;
				while ( true ) {
					Seiscomp::Core::Generic::ObjectIterator
					        <Seiscomp::DataModel::ObjectPtr> objIt(obj, first);
					first = false;
					ar >> objIt;
					if ( !obj ) break;

					std::cerr << "    " << obj->className();
					it = _objectBins.find(obj->className());
					if ( it == _objectBins.end() ) {
						std::cerr << " (ignored)";
					}
					else {
						storage.push_back(obj.get());
						it->second.push_back(obj.get());
					}
					std::cerr << std::endl;
				}

				ar.close();
			}

			Seiscomp::DataModel::DiffMerge merger;
			merger.setLoggingLevel(4);

			Seiscomp::IO::XMLArchive ar;
			ar.create("-");
			ar.setFormattedOutput(true);

			for ( it = _objectBins.begin(); it != _objectBins.end(); ++it ) {
				const std::string &name = it->first;
				Objects &objs = it->second;
				if ( objs.size() == 1 ) {
					std::cerr << "writing " << name << " object" << std::endl;
					ar << objs.front();
				}
				else if ( objs.size() > 1 ) {
					std::cerr << "merging " << objs.size()
					          << " " << name << " objects" << std::endl;
					Seiscomp::DataModel::ObjectPtr obj =
					        dynamic_cast<Seiscomp::DataModel::Object*>(
					            Seiscomp::Core::ClassFactory::Create(it->first));
					merger.merge(obj.get(), objs);
					merger.showLog();
					ar << obj;
				}
			}

			ar.close();

			return true;
		}

	private:
		void registerRootObject(const std::string &name) {
			_objectBins[name] = std::vector<Seiscomp::DataModel::Object*>();
		}

	private:
		typedef std::vector<Seiscomp::DataModel::Object*> Objects;

		std::vector<std::string>            _files;
		std::map<std::string, Objects>      _objectBins;
};


int main(int argc, char **argv) {
	XMLMerge app(argc, argv);
	return app();
}

