/***************************************************************************
 *   Copyright (C) by ASGSR
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

#include "sendevent.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Gui;

/*
 * Usage: scsendevent -E <eventID> 
 *        otherwise type: sctestclient --help
 */
int main(int argc, char* argv[])
{
	Application::Type type = Application::Tty;

	SendEvent app(argc, argv, type);
	
	return app();
}



