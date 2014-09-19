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


#define SEISCOMP_COMPONENT log
#include <seiscomp3/logging/log.h>
#include <seiscomp3/logging/init.h>

#include <list>

using namespace std;


namespace Seiscomp {
namespace Logging {


Module::~Module()
{
}

void Module::init( int &argc, char **argv )
{
    (void)argc;
    (void)argv;
}



static std::list<Module *> moduleList;
static int *gArgc =0;
static char **gArgv = 0;

void init(int &argc, char **argv) {
	gArgc = &argc;
	gArgv = argv;

	list<Module*>::const_iterator it;
	for ( it = moduleList.begin(); it != moduleList.end(); ++it )
		(*it)->init( argc, argv );
}

Module *RegisterModule(Module *module) {
	moduleList.push_back( module );
	if ( gArgc )
		module->init(*gArgc, gArgv);

	return module;
}


}
}
