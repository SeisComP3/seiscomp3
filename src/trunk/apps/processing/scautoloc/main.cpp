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



#define SEISCOMP_COMPONENT Autoloc
#include <seiscomp3/logging/log.h>

#include "app.h"


int main(int argc, char **argv) {
	int retCode = EXIT_SUCCESS;

	// Create an own block to make sure the application object
	// is destroyed when printing the overall objectcount
	{
		Seiscomp::Applications::Autoloc::App app(argc, argv);
		retCode = app.exec();
	}

	SEISCOMP_DEBUG("EXIT(%d), remaining objects: %d", retCode, Seiscomp::Core::BaseObject::ObjectCount());

	return retCode;
}
