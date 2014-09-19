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

#include "testclient.h"


using namespace Seiscomp::Applications;

/*
 * Usage: sctestclient -H <hostname> --component Application
 *        otherwise type: sctestclient --help
 */
int main(int argc, char* argv[])
{
	TestClient testClient(argc, argv);
	
	return testClient();
}
