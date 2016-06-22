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
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/

#define SEISCOMP_COMPONENT QL2SC

#include "app.h"

#include <seiscomp3/logging/log.h>

int main(int argc, char **argv) {
	Seiscomp::QL2SC::App app(argc, argv);
	int retCode;
	retCode = app.exec();
	SEISCOMP_DEBUG("EXIT(%d)", retCode);
	return retCode;
}
