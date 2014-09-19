/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/datamodel/vs/databasereader.h>


ADD_SC_PLUGIN("Data model extension for Virtual Seismologist",
              "ETHZ/SED, gempa GmbH <jabe@gempa.de>", 0, 1, 0)

// Dummy method to force linkage of libseiscomp3_datamodel_sm
Seiscomp::DataModel::VS::VSReader *
createReader() {
	return new Seiscomp::DataModel::VS::VSReader(NULL);
}

