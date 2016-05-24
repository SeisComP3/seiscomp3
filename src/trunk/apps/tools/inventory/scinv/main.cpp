/***************************************************************************
 * Copyright (C) 2009 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/

#define SEISCOMP_COMPONENT INVMGR

#include <boost/version.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/utils/files.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>

#include "merge.h"
#include "sync.h"
#include "check.h"


using namespace std;
using namespace Seiscomp;

namespace fs = boost::filesystem;


template <typename T>
bool lessID(const T *v1, const T *v2) {
	if ( v1->code() < v2->code() ) return true;
	if ( v1->code() > v2->code() ) return false;

	return v1->start() < v2->start();
}


string timeToStr(const Core::Time &time) {
	if ( time.microseconds() == 0 && time.seconds() % 86400 == 0 ) {
		return time.toString("%F");
	}

	return time.toString("%F %T");
}


template <typename T>
string epochToStr(const T *obj) {
	string str;

	str = timeToStr(obj->start());
	try {
		Core::Time end = obj->end();
		str += " - ";
		str += timeToStr(end);
	}
	catch ( ... ) {}

	return str;
}


void compareObjects(const DataModel::Object *o1, const DataModel::Object *o2,
                    std::ostream &out) {
	DataModel::DiffMerge diff;
	diff.setLoggingLevel(1);
	diff.compareObjects(o1, o2);
	diff.showLog(out, 3, 1);
}


template <typename T>
struct Out {
	Out(const T *src, int indentation) : obj(src), indent(indentation) {}

	const T   *obj;
	int        indent;
};

template <typename R, typename T>
struct Out2 : Out<T> {
	Out2(const R *reg, const T *src, int indentation) : Out<T>(src, indentation), registry(reg) {}

	const R   *registry;
};


struct Fill {
	Fill(int number_of_copies, char ch_ = ' ') : n(number_of_copies), ch(ch_) {}
	int  n;
	char ch;
};


ostream &operator<<(ostream &os, const Fill &fill) {
	for ( int i = 0; i < fill.n; ++i )
		os.write(&fill.ch, 1);
	return os;
}


template <typename T>
Out<T> tabular(const T *src, int indentation) {
	return Out<T>(src, indentation);
}


template <typename R, typename T>
Out2<R,T> tabular(const R *reg, const T *src, int indentation) {
	return Out2<R,T>(reg, src, indentation);
}


ostream &operator<<(ostream &os, const Out<DataModel::ResponsePAZ> &out) {
	const DataModel::ResponsePAZ *paz = out.obj;
	os << Fill(out.indent) << "norm freq    ";
	try { os << paz->normalizationFrequency() << "Hz"; }
	catch (...) { os << "-"; }
	os << endl;
	os << Fill(out.indent) << "norm factor  ";
	try { os << paz->normalizationFactor(); }
	catch (...) { os << "-"; }
	os << endl;
	os << Fill(out.indent) << "poles        ";
	try {
		os << Core::toString(paz->poles().content());
	}
	catch ( ... ) {
		os << "-";
	}
	os << endl;

	os << Fill(out.indent) << "zeros        ";
	try {
		os << Core::toString(paz->zeros().content());
	}
	catch ( ... ) {
		os << "-";
	}

	return os;
}


ostream &operator<<(ostream &os, const Out<DataModel::ResponsePolynomial> &out) {
	os << Fill(out.indent) << "<to be implemented>";
	return os;
}


ostream &operator<<(ostream &os, const Out<DataModel::ResponseFAP> &out) {
	const DataModel::ResponseFAP *fap = out.obj;
	os << Fill(out.indent) << "tuples       ";
	try {
		const vector<double> &tuples = fap->tuples().content();
		for ( size_t i = 0; i < tuples.size(); i += 3 ) {
			if ( i > 0 ) os << "," << endl << Fill(out.indent) << "             ";
			os << scientific;
			os << "(" << setw(13) << tuples[i] << ", " << tuples[i+1] << ", " << tuples[i+2] << ")";
		}
	}
	catch ( ... ) {
		os << "-";
	}

	os << endl;
	return os;
}


ostream &operator<<(ostream &os, const Out<DataModel::ResponseFIR> &out) {
	const DataModel::ResponseFIR *fir = out.obj;

	os << Fill(out.indent) << "gain         ";
	try { os << fir->gain(); }
	catch ( ... ) { os << "-"; }
	os << endl;

	os << Fill(out.indent) << "factor       ";
	try { os << fir->decimationFactor(); }
	catch ( ... ) { os << "-"; }
	os << endl;

	os << Fill(out.indent) << "symmetrie    ";
	if ( fir->symmetry().empty() )
		os << "-";
	else
		os << fir->symmetry();
	os << endl;

	os << Fill(out.indent) << "coefficients ";
	try {
		const vector<double> &coeffs = fir->coefficients().content();
		for ( size_t i = 0; i < coeffs.size(); ++i ) {
			if ( i ) {
				if ( i % 5 == 0 ) os << endl << Fill(out.indent+13);
				else os << " ";
			}
			os << showpos << scientific << coeffs[i];
		}
		os << noshowpos;
	}
	catch ( ... ) {
		os << "-";
	}

	return os;
}


template <typename R>
ostream &operator<<(ostream &os, const Out2<R, DataModel::Decimation> &out) {
	try {
		vector<string> ids;
		Core::split(ids, out.obj->analogueFilterChain().content().c_str(), " ");
		for ( size_t i = 0; i < ids.size(); ++i ) {
			os << endl << Fill(out.indent) << "[analogue stage #" << (i+1) << "]" << endl;

			const DataModel::ResponsePAZ *paz = out.registry->findPAZ(ids[i]);
			if ( paz == NULL ) {
				const DataModel::ResponsePolynomial *poly = out.registry->findPoly(ids[i]);
				if ( poly ) {
					os << Fill(out.indent) << "POLY" << endl;
					os << tabular(poly, out.indent);
				}
				else {
					const DataModel::ResponseFAP *fap = out.registry->findFAP(ids[i]);
					if ( fap == NULL )
						os << Fill(out.indent) << "UNKNOWN" << endl;
					else {
						os << Fill(out.indent) << "FAP" << endl;
						os << tabular(fap, out.indent);
					}
				}
			}
			else {
				os << Fill(out.indent) << "PAZ" << endl;
				os << tabular(paz, out.indent);
			}

			if ( i < ids.size()-1 ) os << endl;
		}
	}
	catch ( ... ) {}

	try {
		vector<string> ids;
		Core::split(ids, out.obj->digitalFilterChain().content().c_str(), " ");
		for ( size_t i = 0; i < ids.size(); ++i ) {
			os << endl << Fill(out.indent) << "[digital stage #" << (i+1) << "]" << endl;

			const DataModel::ResponseFIR *fir = out.registry->findFIR(ids[i]);
			if ( fir == NULL ) {
				const DataModel::ResponsePAZ *paz = out.registry->findPAZ(ids[i]);
				if ( paz == NULL ) {
					const DataModel::ResponsePolynomial *poly = out.registry->findPoly(ids[i]);
					if ( poly ) {
						os << Fill(out.indent) << "POLY" << endl;
						os << tabular(poly, out.indent);
					}
					else {
						const DataModel::ResponseFAP *fap = out.registry->findFAP(ids[i]);
						if ( fap == NULL )
							os << Fill(out.indent) << "UNKNOWN" << endl;
						else {
							os << Fill(out.indent) << "FAP" << endl;
							os << tabular(fap, out.indent);
						}
					}
				}
				else {
					os << Fill(out.indent) << "PAZ" << endl;
					os << tabular(paz, out.indent);
				}
			}
			else {
				os << Fill(out.indent) << "FIR" << endl;
				os << tabular(fir, out.indent);
			}

			if ( i < ids.size()-1 ) os << endl;
		}
	}
	catch ( ... ) {}

	return os;
}


class InventoryManager : public Client::Application,
                         private LogHandler {
	public:
		InventoryManager(int argc, char **argv)
		: Client::Application(argc, argv) {
			setMessagingEnabled(true);
			setDatabaseEnabled(true, true);
			setLoadInventoryEnabled(true);
			setPrimaryMessagingGroup("INVENTORY");
			setAutoApplyNotifierEnabled(false);
			setInterpretNotifierEnabled(false);
			setLoggingToStdErr(true);
			setConnectionRetries(3);

			_currentTask = NULL;

			if ( argc > 1 ) _operation = argv[1];
		}


		void createCommandLineDescription() {
			Client::Application::createCommandLineDescription();
			commandline().addGroup("Manager");
			commandline().addOption("Manager", "filebase",
			                        "Filebase to check for XML files. If not "
			                        "given, all XML files passed are checked.",
			                        &_filebase);
			commandline().addOption("Manager", "rc-dir",
			                        "If given rc files will be created in this directory for each "
			                        "station which contains the station description "
			                        "of the last epoch.",
			                        &_rcdir);
			commandline().addOption("Manager", "key-dir",
			                        "If given this directory is used to synchronise "
			                        "key files.",
			                        &_keydir);
			commandline().addOption("Manager", "output,o",
			                        "Output file.",
			                        &_output);
			commandline().addOption("Manager", "level",
			                        "Information level (net, sta, cha or resp) used by ls.",
			                        &_level);
			commandline().addOption("Manager", "compact",
			                        "Enabled compact output for ls.");
			commandline().addOption("Manager", "no-purge-keys",
			                        "Do not delete key files if a station does not exist in inventory");
			commandline().addOption("Manager", "purge-keys",
			                        "(default) Delete key files if a station does not exist in inventory");
			commandline().addGroup("Merge");
			commandline().addOption("Merge", "strip",
			                        "Remove unreferenced objects (dataloggers, "
			                        "sensors, ...).");
			commandline().addGroup("Sync");
			commandline().addOption("Sync", "create-notifier",
			                        "If an output file is given then all "
			                        "notifiers will be saved and not the "
			                        "result set itself.");
			commandline().addOption("Sync", "no-keys",
			                        "Do not synchronise key files.");
			commandline().addOption("Sync", "no-rc",
			                        "Do not synchronise rc files.");
			commandline().addOption("Sync", "test",
			                        "Does not send any notifiers and just outputs "
			                        "resulting operations and conflicts.");
		}


		bool validateParameters() {
			if ( !Client::Application::validateParameters() ) return false;
			vector<string> opts = commandline().unrecognizedOptions();

			if ( _operation.empty() ) {
				cerr << "No operation given." << endl;
				cerr << "Usage: " << name() << " [sync|merge|apply|ls|keys|check] [opts]" << endl;
				return false;
			}

			if ( _operation != "sync" &&
			     _operation != "merge" &&
			     _operation != "apply" &&
			     _operation != "ls" &&
			     _operation != "keys" &&
			     _operation != "check" ) {
				cerr << "Invalid operation: " << _operation << endl;
				return false;
			}

			if ( !_level.empty() && _level != "net" && _level != "sta" &&
			     _level != "cha" && _level != "resp" ) {
				cerr << "Invalid level: " << _level << endl;
				return false;
			}

			if ( _operation == "sync" || _operation == "apply" ) {
				if ( !isInventoryDatabaseEnabled() )
					setDatabaseEnabled(false, false);

				if ( !_output.empty() )
					setMessagingEnabled(false);
			}
			else {
				setDatabaseEnabled(false, false);
				setLoadInventoryEnabled(false);
				setMessagingEnabled(false);
			}

			if ( _keydir.empty() )
				_keydir = Environment::Instance()->appConfigDir() + "/key";

			if ( _rcdir.empty() )
				_rcdir = Environment::Instance()->installDir() + "/var/lib/rc";

			return true;
		}


		void printUsage() const {
			cout << "Usage: " << name() << " [COMMAND] [FILES] [OPTIONS]" << endl;
			cout << endl << "Commands:" << endl;
			cout << "  sync  Synchronises an applications inventory with a given source" << endl
			     << "        given as file(s). The applications inventory is either read from the" << endl
			     << "        database or given with --inventory-db. As a result all information" << endl
			     << "        in the source is written to target and target does not contain any" << endl
			     << "        additional information. The source must hold all information. This" << endl
			     << "        works different to merge. If an output file is specified with -o" << endl
			     << "        no notifiers are generated and sent via messaging." << endl;
			cout << "  merge Merges two or more inventories into one inventory. This command" << endl
			     << "        is useful to merge existing subtrees into a final inventory before" << endl
			     << "        synchronization." << endl;
			cout << "  apply Applies stored notifiers created with sync and option" << endl
			     << "        --create-notifer which is saved in a file (-o). Source is the" << endl
			     << "        applications inventory read from the database or given with --inventory-db." << endl
			     << "        If -o is passed no messages are sent but the result is stored in a file." << endl
			     << "        Useful to test/debug or prepare an inventory for offline processing." << endl;
			cout << "  keys  Synchronise station key files with current inventory pool." << endl;
			cout << "  ls    List contained items up to channel level." << endl;
			cout << "  check Check consistency of given inventory files. If no files are given," << endl;
			cout << "        all files in filebase are merged and checked." << endl;

			Client::Application::printUsage();
		}


		void exit(int returnCode) {
			Client::Application::exit(returnCode);

			if ( _currentTask )
				_currentTask->interrupt();
		}


		bool run() {
			if ( _operation == "merge" )
				return mergeInventory();
			else if ( _operation == "sync" )
				return syncInventory();
			else if ( _operation == "apply" )
				return applyNotifier();
			else if ( _operation == "keys" )
				return syncKeys();
			else if ( _operation == "ls" )
				return listNetworks();
			else if ( _operation == "check" )
				return checkInventory();

			return false;
		}


		void collectFiles(std::vector<std::string> &files) {
			if ( _filebase.empty() ) {
				files = commandline().unrecognizedOptions();
				// Remove the first entry which is the command used
				if ( !files.empty() ) files.erase(files.begin());

				// No files passed and filebase not given? Try to access
				// SEISCOMP_ROOT/etc/inventory
				if ( files.empty() )
					_filebase = Environment::Instance()->appConfigDir() + "/inventory";
				else
					return;
			}

			try {
				fs::path directory = SC_FS_PATH(_filebase);
				fs::directory_iterator it(directory);
				fs::directory_iterator dirEnd;

				for ( ; it != dirEnd; ++it ) {
					if ( fs::is_directory(SC_FS_IT_PATH(it)) ) continue;
					string name = SC_FS_IT_STR(it);
					if ( name.size() > 4 && name.compare(name.size()-4, 4, ".xml") != 0 )
						cerr << "WARNING: " << name
						     << " ignored: wrong extension"
						     << endl;
					else
						files.push_back(name);
				}
			}
			catch ( ... ) {}
		}


		bool mergeInventory() {
			bool stripUnreferenced = commandline().hasOption("strip");

			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( files.empty() ) {
				cerr << "Nothing to merge, no files given" << endl;
				return false;
			}

			DataModel::InventoryPtr finalInventory = new DataModel::Inventory();
			Merge merger(finalInventory.get());
			merger.setLogHandler(this);
			_continueOperation = true;

			_currentTask = &merger;

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				DataModel::InventoryPtr inv;
				cerr << "Parsing " << files[i] << " ... " << flush;
				ar >> inv;
				cerr << "done" << endl;
				if ( !inv ) {
					cerr << "No inventory found (ignored): " << files[i] << endl;
					continue;
				}

				// Pushing the inventory into the merger cleans it
				// completely. The ownership of all childs went to
				// the merger
				merger.push(inv.get());
			}

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			cerr << "Merging inventory ... " << flush;
			merger.merge(stripUnreferenced);
			cerr << "done" << endl;

			printLogs();

			if ( !_continueOperation ) {
				cerr << "Unresolvable errors ... aborting" << endl;
				return false;
			}

			if ( _output.empty() ) _output = "-";

			IO::XMLArchive ar;
			if ( !ar.create(_output.c_str()) ) {
				cerr << "Failed to create output file: " << _output << endl;
				return false;
			}

			cerr << "Generating output ... "  << flush;
			ar.setFormattedOutput(true);
			ar << finalInventory;
			ar.close();
			cerr << "done" << endl;

			if ( !_rcdir.empty() ) {
				if ( !syncRCFiles(finalInventory.get()) ) return false;
			}

			return true;
		}


		bool syncInventory() {
			DataModel::Inventory *targetInv = Client::Inventory::Instance()->inventory();
			bool createNotifier = commandline().hasOption("create-notifier");
			bool testMode = commandline().hasOption("test");

			if ( testMode || _output.empty() ) createNotifier = true;

			if ( targetInv == NULL ) {
				cerr << "No inventory to sync" << endl;
				return false;
			}

			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);
			// Disable notifier check
			DataModel::Notifier::SetCheckEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( _filebase.empty() && files.empty() ) {
				cerr << "Nothing to merge, no files given" << endl;
				return false;
			}

			DataModel::InventoryPtr mergedInventory = new DataModel::Inventory();
			Merge merger(mergedInventory.get());
			merger.setLogHandler(this);
			_continueOperation = true;
			_currentTask = &merger;

			DataModel::Notifier::SetEnabled(false);

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				DataModel::InventoryPtr inv;
				cerr << "Parsing " << files[i] << " ... " << flush;
				ar >> inv;
				cerr << "done" << endl;
				if ( !inv ) {
					cerr << "No inventory found (ignored): " << files[i] << endl;
					continue;
				}

				// Pushing the inventory into the merger cleans it
				// completely. The ownership of all childs goes to
				// the merger
				merger.push(inv.get());
			}

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			cerr << "Merging inventory ... " << flush;
			if ( merger.merge(false) )
				cerr << "done" << endl;
			else
				cerr << "failed" << endl;

			printLogs();

			if ( !_continueOperation ) {
				cerr << "Unresolvable errors ... aborting" << endl;
				return false;
			}

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			// Activate registration again
			DataModel::PublicObject::SetRegistrationEnabled(true);

			Sync syncTask(targetInv);
			_currentTask = &syncTask;

			if ( createNotifier ) DataModel::Notifier::SetEnabled(true);

			cerr << "Synchronising inventory ... " << flush;
			if ( syncTask.push(mergedInventory.get()) )
				cerr << "done";
			else
				cerr << "failed";
			cerr << endl;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			if ( createNotifier ) DataModel::Notifier::SetEnabled(true);

			cerr << "Removing remaining objects ... " << flush;
			syncTask.cleanUp();
			cerr << "done" << endl;

			// --- Check key files
			// Collect all station key files
			map<string,string> keyFiles;
			try {
				fs::path directory = SC_FS_PATH(_keydir);
				fs::directory_iterator it(directory);
				fs::directory_iterator dirEnd;

				for ( ; it != dirEnd; ++it ) {
					if ( fs::is_directory(SC_FS_IT_PATH(it)) ) continue;
					string name = SC_FS_IT_LEAF(it);
					if ( name.compare(0, 8, "station_") == 0 )
						keyFiles[SC_FS_IT_LEAF(it)] = SC_FS_IT_STR(it);
				}
			}
			catch ( ... ) {}

			for ( size_t n = 0; n < mergedInventory->networkCount(); ++n ) {
				DataModel::Network *net = mergedInventory->network(n);
				for ( size_t s = 0; s < net->stationCount(); ++s ) {
					DataModel::Station *sta = net->station(s);
					string id = net->code() + "_" + sta->code();
					string filename = "station_" + id;

					// Remove filename from keyFiles
					map<string,string>::iterator it = keyFiles.find(filename);
					if ( it != keyFiles.end() ) keyFiles.erase(it);
				}
			}

			// Warn about existing key files without a corresponding station
			// in inventory
			if ( !keyFiles.empty() ) {
				SEISCOMP_WARNING("Found %d key file%s without a corresponding "
				                 "station in inventory:",
				                 (int)keyFiles.size(), keyFiles.size() > 1?"s":"");
				for ( map<string,string>::iterator it = keyFiles.begin();
					  it != keyFiles.end(); ++it )
					SEISCOMP_WARNING("  %s", it->second.c_str());
			}

			DataModel::Notifier::SetEnabled(false);

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			bool doSyncKeys = false;

			if ( createNotifier ) {
				DataModel::NotifierMessagePtr nmsg;
				size_t notifierCount = DataModel::Notifier::Size();

				if ( notifierCount > 0 ) {
					cerr << notifierCount << " notifiers available" << endl;

					if ( !_output.empty() ) {
						IO::XMLArchive ar;
						if ( !ar.create(_output.c_str()) ) {
							cerr << "Failed to create output file: " << _output << endl;
							DataModel::Notifier::Clear();
							return false;
						}

						cerr << "Generating output ... "  << flush;
						ar.setFormattedOutput(true);

						nmsg = DataModel::Notifier::GetMessage(true);
						ar << nmsg;

						ar.close();
						cerr << "done" << endl;
					}
					else if ( !testMode ) {
						// Send an inital sync command to also wake-up the messaging
						sync();

						// Send notifier
						DataModel::NotifierMessagePtr tmp = new DataModel::NotifierMessage();
						DataModel::NotifierMessage::iterator it;
						int count = 0;

						// Fetch each single notifier message. Fetching all notifiers
						// in one message can take a long time if a huge amount of
						// notifiers is in the queue. Due to memory fragmentation
						// most of the time spent is in malloc.
						while ( (nmsg = DataModel::Notifier::GetMessage(false)) != NULL ) {
							if ( _exitRequested ) break;
							for ( it = nmsg->begin(); it != nmsg->end(); ++it ) {
								DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
								if ( !n ) continue;

								tmp->attach(n);
								++count;

								if ( count % 100 == 0 ) {
									cerr << "\rSending notifiers: " << (int)(count*100/notifierCount) << "%" << flush;

									connection()->send(tmp.get());

									tmp->clear();
									sync();
								}
							}
						}

						if ( !_exitRequested && !tmp->empty() ) {
							connection()->send(tmp.get());
							cerr << "\rSending notifiers: " << (int)(count*100/notifierCount) << "%" << flush;
						}

						cerr << endl;
						sync();

						doSyncKeys = true;
					}
					else {
						/*
						DataModel::NotifierMessage::iterator it;

						while ( (nmsg = DataModel::Notifier::GetMessage(false)) != NULL ) {
							for ( it = nmsg->begin(); it != nmsg->end(); ++it ) {
								DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
								if ( !n ) continue;

								switch ( n->operation() ) {
									case DataModel::OP_ADD:
										cout << "+";
										break;
									case DataModel::OP_REMOVE:
										cout << "-";
										break;
									case DataModel::OP_UPDATE:
										cout << "M";
										break;
									default:
										cout << "?";
										break;
								}

								cout << " " << n->object()->className() << endl;
							}
						}
						*/

						DataModel::Notifier::Clear();
						cout << "OK - synchronization test passed" << endl;
					}
				}
				else
					cerr << "Inventory is synchronised already, nothing to do" << endl;

				DataModel::Notifier::Clear();
			}
			else {
				if ( _output.empty() ) _output = "-";

				IO::XMLArchive ar;
				if ( !ar.create(_output.c_str()) ) {
					cerr << "Failed to create output file: " << _output << endl;
					return false;
				}

				cerr << "Generating output ... "  << flush;
				ar.setFormattedOutput(true);

				ar << targetInv;

				ar.close();
				cerr << "done" << endl;
			}

			if ( doSyncKeys ) {
				if ( !commandline().hasOption("no-rc") ) {
					if ( !syncRCFiles(targetInv) ) return false;
				}

				if ( !commandline().hasOption("no-keys") ) {
					bool syncKeys = true;
					try { syncKeys = configGetBool("syncKeys"); } catch (...) {}
					if ( syncKeys && !syncKeyFiles(targetInv) ) return false;
				}
			}

			return true;
		}


		bool checkInventory() {
			_conflicts = _errors = _warnings = _unresolved = 0;

			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( files.empty() ) {
				cerr << "Nothing to merge, no files given" << endl;
				return false;
			}

			DataModel::InventoryPtr finalInventory = new DataModel::Inventory();
			Merge merger(finalInventory.get());
			merger.setLogHandler(this);
			_continueOperation = true;

			_currentTask = &merger;

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				DataModel::InventoryPtr inv;
				cerr << "Parsing " << files[i] << " ... " << flush;
				ar >> inv;
				cerr << "done" << endl;
				if ( !inv ) {
					cerr << "No inventory found (ignored): " << files[i] << endl;
					continue;
				}

				// Pushing the inventory into the merger cleans it
				// completely. The ownership of all childs went to
				// the merger
				merger.push(inv.get());
			}

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			cerr << "Merging inventory ... " << flush;
			merger.merge(false);
			cerr << "done" << endl;

			Check checker(finalInventory.get());
			checker.setLogHandler(this);
			cerr << "Checking inventory ... " << flush;
			checker.check();
			cerr << "done" << endl;

			printLogs(cout);

			if ( _conflicts > 0 )
				cerr << _conflicts << " conflict" << (_conflicts == 1?"":"s") << endl;

			if ( _unresolved > 0 )
				cerr << _unresolved << " unresolved reference" << (_unresolved == 1?"":"s") << endl;

			if ( _errors > 0 )
				cerr << _errors << " error" << (_errors == 1?"":"s") << endl;

			if ( _warnings > 0 )
				cerr << _warnings << " warning" << (_warnings == 1?"":"s") << endl;

			return _conflicts == 0 && _errors == 0;
		}


		bool syncKeys() {
			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);
			// Disable notifier check
			DataModel::Notifier::SetCheckEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( _filebase.empty() && files.empty() ) {
				cerr << "Nothing to merge, no files given" << endl;
				return false;
			}

			DataModel::InventoryPtr mergedInventory = new DataModel::Inventory();
			Merge merger(mergedInventory.get());
			merger.setLogHandler(this);
			_continueOperation = true;
			_currentTask = &merger;

			DataModel::Notifier::SetEnabled(false);

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				DataModel::InventoryPtr inv;
				cerr << "Parsing " << files[i] << " ... " << flush;
				ar >> inv;
				cerr << "done" << endl;
				if ( !inv ) {
					cerr << "No inventory found (ignored): " << files[i] << endl;
					continue;
				}

				// Pushing the inventory into the merger cleans it
				// completely. The ownership of all childs goes to
				// the merger
				merger.push(inv.get());
			}

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			cerr << "Merging inventory ... " << flush;
			merger.merge(false);
			cerr << "done" << endl;

			printLogs();

			if ( !_continueOperation ) {
				cerr << "Unresolvable errors ... aborting" << endl;
				return false;
			}

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			return syncKeyFiles(mergedInventory.get());
		}


		bool syncRCFiles(DataModel::Inventory *inv) {
			if ( !Util::pathExists(_rcdir) ) {
				if ( !Util::createPath(_rcdir) ) {
					cerr << "ERROR: Unable to create rc output path "
					     << _rcdir << endl;
					return false;
				}
			}

			typedef pair<Core::Time, string> Item;
			typedef map<string, Item> DescMap;
			DescMap descs;

			for ( size_t n = 0; n < inv->networkCount(); ++n ) {
				DataModel::Network *net = inv->network(n);
				for ( size_t s = 0; s < net->stationCount(); ++s ) {
					DataModel::Station *sta = net->station(s);
					string id = net->code() + "_" + sta->code();
					Core::Time endTime;

					try { endTime = sta->end(); }
					catch ( ... ) {}

					DescMap::iterator it = descs.find(id);
					if ( it == descs.end() )
						descs[id] = Item(endTime, sta->description());
					else {
						if ( (it->second.first.valid() && it->second.first < endTime) ||
						     !endTime.valid() )
							it->second.second = sta->description();
					}
				}
			}

			// Collect all station files
			vector<string> oldFiles;
			try {
				fs::path directory = SC_FS_PATH(_rcdir);
				fs::directory_iterator it(directory);
				fs::directory_iterator dirEnd;

				for ( ; it != dirEnd; ++it ) {
					if ( fs::is_directory(*it) ) continue;
					string name = SC_FS_IT_LEAF(it);
					if ( name.compare(0, 8, "station_") == 0 )
						oldFiles.push_back(SC_FS_IT_STR(it));
				}
			}
			catch ( ... ) {}

			// Delete them
			for ( size_t i = 0; i < oldFiles.size(); ++i )
				unlink(oldFiles[i].c_str());

			for ( DescMap::iterator it = descs.begin(); it != descs.end(); ++it ) {
				fs::path fp = SC_FS_PATH(_rcdir) / SC_FS_PATH((string("station_") + it->first));
				Config::Config cfg;
				cfg.setString("description", it->second.second);
				if (! cfg.writeConfig(fp.string()) ) {
					cerr << "ERROR: " << fp.string() << ": unable to write file" << endl;
					return false;
				}
			}

			return true;
		}


		bool syncKeyFiles(DataModel::Inventory *inv) {
			if ( !Util::pathExists(_keydir) ) {
				if ( !Util::createPath(_keydir) ) {
					cerr << "ERROR: Unable to create key output path "
					     << _keydir << endl;
					return false;
				}
			}

			bool allowPurge = !commandline().hasOption("no-purge-keys");
			if ( !commandline().hasOption("no-purge-keys") && !commandline().hasOption("purge-keys") ) {
				try { allowPurge = configGetBool("purgeKeys"); } catch (...) {}
			}

			// Collect all station key files
			map<string, string> oldFiles;
			try {
				fs::path dir = SC_FS_PATH(_keydir);
				fs::directory_iterator it(dir);
				fs::directory_iterator dirEnd;

				for ( ; it != dirEnd; ++it ) {
					if ( fs::is_directory(*it) ) continue;
					string name = SC_FS_IT_LEAF(it);
					if ( name.compare(0, 8, "station_") == 0 )
						oldFiles[SC_FS_IT_LEAF(it)] = SC_FS_IT_STR(it);
				}
			}
			catch ( ... ) {}

			int added = 0, removed = 0;

			// Create station key files
			for ( size_t n = 0; n < inv->networkCount(); ++n ) {
				DataModel::Network *net = inv->network(n);
				for ( size_t s = 0; s < net->stationCount(); ++s ) {
					DataModel::Station *sta = net->station(s);
					string id = net->code() + "_" + sta->code();
					string filename = "station_" + id;

					// Remove filename from oldFiles and prevent deletion
					map<string, string>::iterator it = oldFiles.find(filename);
					if ( it != oldFiles.end() ) oldFiles.erase(it);

					fs::path fp = SC_FS_PATH(_keydir) / SC_FS_PATH(filename);
					// Ignore existing files
					if ( fs::exists(fp) ) continue;

					ofstream ofs(fp.string().c_str());
					++added;
				}
			}

			if ( allowPurge ) {
				// Delete remaining files
				map<string, string>::iterator it;
				for ( it = oldFiles.begin(); it != oldFiles.end(); ++it ) {
					unlink(it->second.c_str());
					++removed;
				}
			}

			if ( added > 0 )
				cerr << "Added " << added << " new key file(s)";
			else
				cerr << "No new key file added";
			cerr << " and ";
			if ( removed > 0 )
				cerr << "removed " << removed << " old key file(s)";
			else
				cerr << "no old key file removed";
			cerr << endl;

			return true;
		}


		bool applyNotifier() {
			DataModel::Inventory *targetInv = Client::Inventory::Instance()->inventory();

			if ( targetInv == NULL ) {
				cerr << "No inventory to apply" << endl;
				return false;
			}

			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( files.empty() ) {
				cerr << "Nothing to apply, no files given" << endl;
				return false;
			}

			Sync syncTask(targetInv);
			_currentTask = &syncTask;

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				cerr << "Parsing " << files[i] << " ... " << flush;
				DataModel::NotifierMessagePtr msg;
				ar >> msg;
				cerr << "done" << endl;

				if ( !msg ) {
					cerr << "No notifier message found (ignored): " << files[i] << endl;
					continue;
				}

				size_t notifierCount = msg->size();
				cerr << notifierCount << " notifiers available" << endl;

				if ( !_output.empty() ) {
					cerr << "Applying notifier ... " << flush;

					// Apply all notifier
					DataModel::NotifierMessage::iterator it;
					for ( it = msg->begin(); it != msg->end(); ++it ) {
						DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
						if ( !n ) continue;
						n->apply();
					}

					cerr << "done" << endl;
				}
				else {
					// Send all notifier

					// Send an inital sync command to also wake-up the messaging
					sync();

					// Send notifier
					DataModel::NotifierMessagePtr tmp = new DataModel::NotifierMessage();
					DataModel::NotifierMessage::iterator it;
					int count = 0;

					for ( it = msg->begin(); it != msg->end(); ++it ) {
						DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
						if ( !n ) continue;

						tmp->attach(n);
						++count;

						if ( count % 100 == 0 ) {
							cerr << "\rSending notifiers: " << (int)(count*100/notifierCount) << "%" << flush;

							connection()->send(tmp.get());

							tmp->clear();
							sync();
						}
					}

					sync();

					if ( !tmp->empty() ) {
						connection()->send(tmp.get());
						cerr << "\rSending notifiers: " << (int)(count*100/notifierCount) << "%" << flush;
					}
				}
			}

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			if ( !_output.empty() ) {
				IO::XMLArchive ar;
				if ( !ar.create(_output.c_str()) ) {
					cerr << "Failed to create output file: " << _output << endl;
					return false;
				}

				cerr << "Generating output ... "  << flush;
				ar.setFormattedOutput(true);

				ar << targetInv;

				ar.close();
				cerr << "done" << endl;
			}

			return true;
		}


		bool listNetworks() {
			// Disable object registration
			DataModel::PublicObject::SetRegistrationEnabled(false);

			vector<string> files;
			collectFiles(files);

			if ( files.empty() ) {
				cerr << "Nothing to merge, no files given" << endl;
				return false;
			}

			DataModel::InventoryPtr finalInventory = new DataModel::Inventory();
			Merge merger(finalInventory.get());

			_currentTask = &merger;

			for ( size_t i = 0; i < files.size(); ++i ) {
				if ( _exitRequested ) break;

				IO::XMLArchive ar;
				if ( !ar.open(files[i].c_str()) ) {
					cerr << "Could not open file (ignored): " << files[i] << endl;
					continue;
				}

				DataModel::InventoryPtr inv;
				cerr << "Parsing " << files[i] << " ... " << flush;
				ar >> inv;
				cerr << "done" << endl;
				if ( !inv ) {
					cerr << "No inventory found (ignored): " << files[i] << endl;
					continue;
				}

				// Pushing the inventory into the merger cleans it
				// completely. The ownership of all childs went to
				// the merger
				merger.push(inv.get());
			}

			_currentTask = NULL;

			if ( _exitRequested ) {
				cerr << "Exit requested: abort" << endl;
				return false;
			}

			cerr << "Merging inventory ... " << flush;
			merger.merge(false);
			cerr << "done" << endl;

			bool compact = commandline().hasOption("compact");
			int level = 2;
			if ( _level == "net" )
				level = 0;
			else if ( _level == "sta" )
				level = 1;
			else if ( _level == "cha" )
				level = 2;
			else if ( _level == "resp" )
				level = 3;

			std::vector<DataModel::Network*> nets;

			for ( size_t n = 0; n < finalInventory->networkCount(); ++n )
				nets.push_back(finalInventory->network(n));

			sort(nets.begin(), nets.end(), lessID<DataModel::Network>);

			for ( size_t n = 0; n < nets.size(); ++n ) {
				DataModel::Network *net = nets[n];
				if ( compact )
					cout << net->code() << "\t" << epochToStr(net) << endl;
				else {
					cout << "  network " << net->code();
					if ( !net->description().empty() ) {
						cout << setfill(' ') << setw(8-net->code().size()) << ' ';
						cout << " " << net->description();
					}
					cout << endl;
					cout << "    epoch " << epochToStr(net) << endl;
				}

				std::vector<DataModel::Station*> stas;

				if ( level > 0 ) {
					for ( size_t s = 0; s < net->stationCount(); ++s )
						stas.push_back(net->station(s));
				}

				sort(stas.begin(), stas.end(), lessID<DataModel::Station>);

				for ( size_t s = 0; s < stas.size(); ++s ) {
					DataModel::Station *sta = stas[s];
					if ( compact )
						cout << " " << sta->code() << "\t" << epochToStr(sta) << endl;
					else {
						cout << "    station " << sta->code();
						if ( !sta->description().empty() ) {
							cout << setfill(' ') << setw(6-sta->code().size()) << ' ';
							cout << " " << sta->description();
						}
						cout << endl;
						cout << "      epoch " << epochToStr(sta) << endl;
					}

					std::vector<DataModel::SensorLocation*> locs;

					if ( level > 1 ) {
						for ( size_t l = 0; l < sta->sensorLocationCount(); ++l )
							locs.push_back(sta->sensorLocation(l));
					}

					sort(locs.begin(), locs.end(), lessID<DataModel::SensorLocation>);

					for ( size_t l = 0; l < locs.size(); ++l ) {
						DataModel::SensorLocation *loc = locs[l];
						if ( compact ) {
							cout << "  ";
							if ( loc->code().empty() )
								cout << "__";
							else
								cout << loc->code();
							cout << "\t" << epochToStr(loc) << endl;
						}
						else {
							cout << "      location ";
							if ( loc->code().empty() )
								cout << "__";
							else
								cout << loc->code();
							cout << endl;

							cout << "        epoch " << epochToStr(loc) << endl;
						}

						std::vector<DataModel::Stream*> streams;

						for ( size_t s = 0; s < loc->streamCount(); ++s )
							streams.push_back(loc->stream(s));

						sort(streams.begin(), streams.end(), lessID<DataModel::Stream>);

						for ( size_t s = 0; s < streams.size(); ++s ) {
							DataModel::Stream *str = streams[s];
							if ( compact )
								cout << "   " << str->code() << "\t" << epochToStr(str) << endl;
							else {
								cout << "        channel ";
								cout << str->code() << endl;

								cout << "          epoch " << epochToStr(str) << endl;
							}

							if ( level >= 3 ) {
								const DataModel::Sensor *sens;
								const DataModel::Datalogger *dl;
								const DataModel::ResponsePAZ *paz;
								const DataModel::ResponsePolynomial *poly;
								const DataModel::ResponseFAP *fap;

								try {
									int sr_num = str->sampleRateNumerator();
									int sr_den = str->sampleRateDenominator();
									cout << "          rate  " << sr_num << "/" << sr_den << " sps" << endl;
								}
								catch ( ... ) {}

								try {
									double gain = str->gain();
									cout << "          gain  " << gain << endl;
								}
								catch ( ... ) {}
								try {
									double freq = str->gainFrequency();
									cout << "          freq  " << freq << "Hz" << endl;
								}
								catch ( ... ) {}

								if ( !str->gainUnit().empty() )
									cout << "          unit  " << str->gainUnit() << endl;

								sens = merger.findSensor(str->sensor());
								if ( sens ) {
									cout << "          sens  " << sens->description() << endl;
									paz = merger.findPAZ(sens->response());
									if ( paz ) {
										cout << "          resp  PAZ" << endl;
										cout << tabular(paz, 16) << endl;
									}
									else {
										poly = merger.findPoly(sens->response());
										if ( poly ) {
											cout << "          resp  polynomial" << endl;
											cout << tabular(poly, 16) << endl;
										}
										else {
											fap = merger.findFAP(sens->response());
											if ( fap ) {
												cout << "          resp  fap" << endl;
												cout << tabular(fap, 16) << endl;
											}
										}
									}
								}

								dl = merger.findDatalogger(str->datalogger());
								if ( dl ) {
									if ( !dl->description().empty() )
										cout << "          dl    " << dl->description() << endl;
									try {
										DataModel::Decimation *deci = dl->decimation(DataModel::DecimationIndex(str->sampleRateNumerator(), str->sampleRateDenominator()));
										if ( deci ) {
											cout << "          dec   " << str->sampleRateNumerator() << "/" << str->sampleRateDenominator() << " sps" << endl;
											cout << tabular(&merger, deci, 16) << endl;
										}
									}
									catch ( ... ) {}
								}
							}
						}
					}
				}
			}

			return true;
		}


		void publish(LogHandler::Level level, const char *message,
		             const DataModel::Object *obj1,
		             const DataModel::Object *obj2) {
			if ( level == LogHandler::Conflict ) ++_conflicts;
			else if ( level == LogHandler::Error ) ++_errors;
			else if ( level == LogHandler::Warning ) ++_warnings;
			else if ( level == LogHandler::Unresolved ) ++_unresolved;

			if ( level == LogHandler::Conflict ) {
				_logs << "C " << message << endl;
				_continueOperation = false;

				if ( obj1 != NULL && obj2 != NULL )
					compareObjects(obj1, obj2, _logs);
			}
			else if ( level == LogHandler::Error ) {
				_logs << "! " << message << endl;
				_continueOperation = false;
			}
			else if ( level == LogHandler::Warning ) {
				_logs << "W " << message << endl;
			}
			else if ( level == LogHandler::Information ) {
				_logs << "I " << message << endl;
			}
			else if ( level == LogHandler::Debug ) {
				_logs << "D " << message << endl;
			}
			else if ( level == LogHandler::Unresolved ) {
				_logs << "R " << message << endl;
			}
			else {
				_logs << "? " << message << endl;
			}
		}


		void printLogs(ostream &os = cerr) {
			string content = _logs.str();
			if ( !content.empty() ) os << content;
			_logs.str(string());
			_logs.clear();
		}


	private:
		Task   *_currentTask;
		string  _operation;
		string  _filebase;
		string  _rcdir;
		string  _keydir;
		string  _output;
		string  _level;
		bool    _continueOperation;
		std::stringstream  _logs;
		int     _conflicts;
		int     _errors;
		int     _warnings;
		int     _unresolved;
};


int main(int argc, char **argv) {
	InventoryManager mgr(argc, argv);
	return mgr();
}
