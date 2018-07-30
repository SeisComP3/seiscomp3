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


#include "scardac.h"


int main(int argc, char **argv) {
	Seiscomp::Applications::SCARDAC app(argc, argv);
	return app();
}
