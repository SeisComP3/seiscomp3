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

#ifndef __SEISCOMP_APPLICATIONS_SCVSMAG_UTIL_H__
#define __SEISCOMP_APPLICATIONS_SCVSMAG_UTIL_H__

#include <seiscomp3/datamodel/waveformstreamid.h>
#include <string>

namespace Seiscomp {
namespace Private {

std::string
toStreamID(const DataModel::WaveformStreamID &wfid);

}
}

#endif

