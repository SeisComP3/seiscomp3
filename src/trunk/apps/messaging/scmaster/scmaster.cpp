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


#define SEISCOMP_COMPONENT MASTER_COM_MODULE
#include <seiscomp3/logging/log.h>

#include <iostream>
#include <string>


#include "master.h"
#include <seiscomp3/system/environment.h>
#include <seiscomp3/client/daemon.h>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <signal.h>
#include <sys/stat.h>

using namespace Seiscomp;

Communication::Master* master = NULL;


void signalHandler(int signal) {
	switch ( signal ) {
		case SIGTERM:
			SEISCOMP_INFO("Received SIGTERM");
			break;
		case SIGINT:
			SEISCOMP_INFO("Received SIGINT");
			break;
		case SIGSEGV:
			SEISCOMP_ERROR("segfault");
			exit(1);
			return;
		default:
			SEISCOMP_INFO("Received signal %d, ignoring", signal);
			return;
	}
	master->stop();
	exit(1);
}


int main( int argc, char *argv[] )
{
	// stdLog.subscribeTo(Seiscomp::Logging::GetComponentWarnings("COM_MODULE"));
	// stdLog.subscribeTo(Seiscomp::Logging::GetComponentAll("COM_MODULE"));
	// subscribe to all messages globally
	// stdLog.subscribeTo(Seiscomp::Logging::GetAll());

	// Command line parsing
	unsigned int logLevel = 2;
	unsigned int logLevelAdd = 0;
	std::string lockfile;
	std::string serverAddress;
	std::string configFile;
	boost::program_options::options_description desc("Allowed options");
	Utils::addCommonOptions(desc, logLevel, logLevelAdd, lockfile);
	desc.add_options()("host,H",
	                   boost::program_options::value<std::string>(&serverAddress)
						   ->default_value("localhost:4803"),
	                   "spread server address");
	desc.add_options()("config-file",
	                   boost::program_options::value<std::string>(&configFile),
	                   "Alternative configuration file");

	boost::program_options::variables_map vm;

	try {
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
	}
	catch ( std::exception& e ) {
		std::cout << "Error: " << e.what() <<  std::endl;
		std::cout << desc << std::endl;
    	return EXIT_FAILURE;
	}
	boost::program_options::notify( vm );

	if ( vm.count( "help" ) )
	{
		std::cout << desc << std::endl;
		return EXIT_SUCCESS;
	}

	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGSEGV, signalHandler);
	std::cout << "Master client will connect to spread server: " << serverAddress << std::endl;

	logLevel += logLevelAdd;

	if ( Utils::initLogging(argc, argv, logLevel, vm.count("syslog"), vm.count("debug")) < 0 )
		return EXIT_FAILURE;

	if ( vm.count("daemon") && !vm.count("debug") && Utils::initDaemon() < 0 )
		return EXIT_FAILURE;

	if ( lockfile.length() > 0 ) {
		int r = Utils::acquireLockfile(lockfile.c_str());
		if ( r < 0 ) {
			// Error should have been reported by acquireLockfile()
			return EXIT_FAILURE;
		}
		else if ( r == 0 ) {
			SEISCOMP_WARNING("Already running");
			return EXIT_FAILURE;
		}
	}

	master = Communication::Master::Create(argv[0]);
	if ( !master ) return EXIT_FAILURE;

	if ( vm.count("config-file") )
		master->setConfigFile(configFile);

	if ( master->init() )
		master->start(serverAddress);

	delete master;

	return EXIT_SUCCESS;
}
