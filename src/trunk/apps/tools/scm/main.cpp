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

#include <iostream>
#include <memory>

#include <seiscomp3/core/platform/platform.h>

#include "monitor.h"


using namespace Seiscomp::Applications;


int main(int argc, char* argv[]) {
	Monitor app(argc, argv);
	return app.exec();
}
