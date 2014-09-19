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




#include <string>
#include <vector>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/origin.h>

#include "datamodel.h"

namespace Autoloc {

void delazi(double lat1, double lon1, double lat2, double lon2, double &delta, double &az1, double &az2);
void delazi(const Hypocenter*, const Station*, double &delta, double &az1, double &az2);

Seiscomp::Core::Time sc3time(const Time &time);

Seiscomp::DataModel::Origin *convertToSC3(const Origin* origin, bool allPhases=true);
// Origin *convertFromSC3(const Seiscomp::DataModel::Origin* sc3origin);

// Pick *convertFromSC3(const Seiscomp::DataModel::Pick *sc3pick);

} // namespace Autoloc

