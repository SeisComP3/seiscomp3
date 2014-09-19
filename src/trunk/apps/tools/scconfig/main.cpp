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


#define SEISCOMP_COMPONENT Configurator

#include <iostream>
#include <seiscomp3/utils/files.h>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/system/schema.h>
#include <seiscomp3/system/model.h>
#include <seiscomp3/logging/log.h>

#include "gui.h"

#include <QtGui>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Logging;


namespace {


const char *module_desc = "/etc/descriptions";

string filebase;
string outdir;


}


int main(int argc, char **argv) {
	filebase = Seiscomp::Environment::Instance()->installDir();
	if ( filebase.empty() )
		filebase = ".";
	else if ( *filebase.rbegin() == '/' )
		filebase.resize(filebase.size()-1);

	//outdir = filebase + ".out";
	outdir = filebase;

	// Activate error and warning logs
	enableConsoleLogging(_SCErrorChannel);
	enableConsoleLogging(_SCWarningChannel);

	for ( int i = 1; i < argc; ++i ) {
		if ( !strcmp("--debug", argv[i]) )
			Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getAll());
	}

	System::SchemaDefinitions defs;
	cerr << "Loading definitions from: " << filebase << module_desc << endl;
	if ( !defs.load((filebase + module_desc).c_str()) ) {
		cerr << "read error: not a directory" << endl;
		return 1;
	}

	if ( defs.moduleCount() == 0 ) {
		cerr << "read error: no module" << endl;
		return 2;
	}

	System::Model model;
	model.create(&defs);

	cerr << "Loading stations from: " << model.stationConfigDir(true) << endl;

	QApplication app(argc, argv);

	Configurator c(Seiscomp::Environment::CS_CONFIG_APP);
	c.resize(800,600);
	if ( !c.setModel(&model) )
		return 1;

	c.show();

	app.exec();

	return 0;
}
