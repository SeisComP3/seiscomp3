/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_CUSTOM_REGIONS_NUTTLI_H__
#define __SEISCOMP_CUSTOM_REGIONS_NUTTLI_H__


#include <seiscomp3/core/version.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/geo/featureset.h>


namespace Seiscomp {
namespace Magnitudes {
namespace MN {


bool initialize(const Config::Config *config);

bool isInsideRegion(double lat0, double lon0,
                    double lat1, double lon1);

bool isInsideRegion(double lat, double lon);


}
}
}


#endif
