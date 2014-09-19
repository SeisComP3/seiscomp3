/*==============================================================================
    Name:       converter.cpp

    Purpose:    convertion between stationdb and seiscomp xml

    Language:   C++, ANSI standard.

    Author:     Peter de Boer, KNMI, Netherlands
                Adopted by Jan Becker, gempa GmbH

    Revision:   2012-05-02

==============================================================================*/
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include "converter.h"
#include "dataless.h"

#define SEISCOMP_COMPONENT dlsv2inv
#include <seiscomp3/logging/log.h>

using namespace std;

namespace Seiscomp {
namespace Applications {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Converter::Converter(int argc, char **argv) : Client::Application(argc, argv) {
	setMessagingEnabled(false);
	setDatabaseEnabled(false, false);
	setLoggingToStdErr(true);

	_net_start_str = "1980-01-01"; // default
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Converter::~Converter() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Converter::createCommandLineDescription() {
	Client::Application::createCommandLineDescription();

	commandline().addGroup("ArcLink");
	commandline().addOption("ArcLink", "dcid", "datacenter/archive ID", &_dcid, false);
	commandline().addOption("ArcLink", "net-description", "override network description", &_net_description);
	commandline().addOption("ArcLink", "net-start", "set network start time", &_net_start_str);
	commandline().addOption("ArcLink", "net-end", "set network end time", &_net_end_str);
	commandline().addOption("ArcLink", "net-type", "set network type (VBB, SM, etc.)", &_net_type);
	commandline().addOption("ArcLink", "temporary", "set network as temporary");
	commandline().addOption("ArcLink", "restricted", "set network as restricted");
	commandline().addOption("ArcLink", "private", "set network as not shared");

	commandline().addGroup("Convert");
	commandline().addOption("Convert", "formatted,f", "Enables formatted output");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Converter::validateParameters() {
	Core::Time net_start;
	if( commandline().hasOption("net-start") ) {
		if ( !Core::fromString(net_start, _net_start_str) &&
		     !net_start.fromString(_net_start_str.c_str(), "%Y-%m-%d")) {
			SEISCOMP_ERROR("invalid time format: %s", _net_start_str.c_str());
			return false;
		}
		_net_start = net_start;
	}
	else {
		_net_start = Core::Time(1980, 1, 1);
	}

	Core::Time net_end;
	if ( commandline().hasOption("net-end") ) {
		if ( !Core::fromString(net_end, _net_end_str) &&
		     !net_end.fromString(_net_end_str.c_str(), "%Y-%m-%d") ) {
			SEISCOMP_ERROR("invalid time format: %s", _net_end_str.c_str());
			return false;
		}
		_net_end = net_end;
	}

	if( commandline().hasOption("temporary") && _net_start_str.empty() ) {
		SEISCOMP_ERROR("start time must be specified for temporary networks");
		exit(1);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Converter::initConfiguration() {
	if ( !Client::Application::initConfiguration() )
		return false;

	// force logging to stderr even if logging.file = 1
	setLoggingToStdErr(true);
	
	try { _dcid = configGetString("datacenterID"); } catch (...) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Converter::run() {
	vector<string> args = commandline().unrecognizedOptions();
	if ( args.size() < 1 ) {
		cerr << "Usage: dlsv2inv [options] input [output=stdout]" << endl;
		return false;
	}

	DataModel::InventoryPtr inv = new DataModel::Inventory;
	Dataless *dl = new Dataless(_dcid, _net_description, _net_type, _net_start, _net_end,
		commandline().hasOption("temporary"), commandline().hasOption("restricted"),
		!commandline().hasOption("private"));
	if ( !dl->SynchronizeDataless(inv.get(), args[0]) ) {
		cerr << "Error processing data" << endl;
		return false;
	}

	IO::XMLArchive ar;

	if ( args.size() > 1 ) {
		if ( !ar.create(args[1].c_str()) ) {
			cerr << "Cannot create " << args[1] << endl;
			return false;
		}
	}
	else {
		if ( !ar.create("-") ) {
			cerr << "Cannot open stdout for output" << endl;
			return false;
		}
	}

	ar.setFormattedOutput(commandline().hasOption("formatted"));
	ar << inv;
	ar.close();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Applications
} // namespace Seiscomp

